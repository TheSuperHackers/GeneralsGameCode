import argparse
import re
import yaml
from pathlib import Path
from github import Github, Auth, PullRequest, Repository
from datetime import datetime
from ruamel.yaml import YAML, CommentedMap
from typing import Any, Dict, List, Pattern, Tuple, Union

import os
from dotenv import load_dotenv

load_dotenv()

# Type aliases for better readability
ConfigDict = Dict[str, Any]
SectionConfig = Dict[str, Union[str, List[str], bool]]


class PRProcessor:
    """Main processor class for converting PR data to structured YAML"""

    def __init__(self, config_path: str) -> None:
        """
        Initialize processor with configuration file

        Args:
            config_path: Path to YAML configuration file
        """
        with open(config_path) as f:
            self.config: ConfigDict = yaml.safe_load(f)

        self.parsed_data: Dict[str, Any] = {}
        self.repo_context: Dict[str, Union[str, int]] = {}

    def process_pr(
        self, pr: PullRequest.PullRequest, repo: Repository.Repository
    ) -> Dict[str, Any]:
        """
        Process a GitHub PR and generate structured data

        Args:
            pr: GitHub PullRequest object
            repo: GitHub Repository object

        Returns:
            Dictionary containing processed data and output filename
        """
        self._setup_repo_context(pr, repo)
        self._process_title(pr.title)
        self._process_body(pr.body, repo.html_url)
        self._process_github_metadata(pr)
        return self._prepare_output()

    def _setup_repo_context(self, pr: Any, repo: Any) -> None:
        """Store repository context including API object"""
        self.repo_context = {
            "repo_url": repo.html_url,
            "pr_creator": pr.user.login,
            "pr_number": pr.number,
            "repo_object": repo,  # Store actual repo object for API access
        }

    def _process_title(self, title: str) -> None:
        """
        Extract scopes and clean title based on configuration

        Args:
            title: Original PR title from GitHub
        """
        title_config = self.config.get("title_processing", {})
        scope_pattern: str = title_config.get("scope_pattern", r"^\[([\w\/]+)\]")
        scopes: List[str] = []
        remaining_title = title

        # Extract scopes using configured pattern
        while True:
            match = re.match(scope_pattern, remaining_title)
            if not match:
                break
            scopes.extend(match.group(1).upper().split("/"))
            remaining_title = remaining_title[match.end() :].strip()

        # Validate allowed scopes
        allowed_scopes = self.config["title_processing"].get("allowed_scopes", [])
        if allowed_scopes:
            invalid_scopes = [s for s in scopes if s.upper() not in allowed_scopes]
            if invalid_scopes:
                print(
                    f"Warning: Disallowed scopes detected - {', '.join(invalid_scopes)}"
                )
            scopes = [s for s in scopes if s.upper() in allowed_scopes]

        # Store processed data
        if scope_key := title_config.get("scope_key"):
            self.parsed_data[scope_key] = scopes

        self.parsed_data["title"] = (
            remaining_title if title_config.get("clean_title", True) else title
        )

    def _process_body(self, pr_body: Union[str, None], repo_url: str) -> None:
        """
        Process PR body content using configured section definitions

        Args:
            pr_body: Raw PR body content from GitHub
            repo_url: Base URL of the repository
        """
        sections = self._parse_markdown_sections(pr_body or "")

        for section_name, config in self.config.get("sections", {}).items():
            if processed := self._process_section(sections, config, repo_url):
                self._merge_data(
                    yaml_key=config["yaml_key"],
                    new_data=processed,
                    merge_keys=config.get("merge_with", []),
                )

    def _parse_markdown_sections(self, pr_body: str) -> Dict[str, List[str]]:
        """
        Parse PR body into markdown sections

        Args:
            pr_body: Raw PR body content

        Returns:
            Dictionary of section names to their content lines
        """
        sections: Dict[str, List[str]] = {}
        current_section = None
        cleaned_body = re.sub(r"<!--.*?-->", "", pr_body, flags=re.DOTALL)

        for line in cleaned_body.split("\n"):
            line = line.strip()
            if line.startswith("## "):
                current_section = line[3:].split("<!--")[0].strip().lower()
                sections[current_section] = []
            elif current_section:
                sections[current_section].append(line)

        return sections

    def _process_section(
        self, sections: Dict[str, List[str]], config: SectionConfig, repo_url: str
    ) -> List[str]:
        """
        Process a single section based on configuration

        Args:
            sections: All parsed markdown sections
            config: Section processing configuration
            repo_url: Base repository URL for link resolution

        Returns:
            List of processed values from this section
        """
        target_header = config["header"].lower()
        section_content: List[str] = []

        # Find matching section header
        for section_name in sections:
            if re.fullmatch(config["header"], section_name, re.IGNORECASE):
                section_content = sections[section_name]
                break

        # Process section content
        processed: List[str] = []
        processor = config.get("processor")
        pattern: Pattern = re.compile(config["pattern"])

        for line in section_content:
            # Clean line and remove list markers
            clean_line = re.sub(r"^-\s*", "", line.strip())
            matches = pattern.findall(clean_line)

            if not matches:
                continue

            if processor == "key_value":
                processed.extend(
                    [
                        {match[0].strip(): match[1].strip()}
                        for match in matches
                        if len(match) >= 2
                    ]
                )
            elif processor == "structured_links":
                processed.extend(
                    [
                        {
                            "action": m[0].lower(),
                            "link": self._resolve_reference(m[1], config["link_type"]),
                        }
                        for m in matches
                        if len(m) >= 2
                    ]
                )
            elif processor == "flat_list":
                processed.extend([m[0] if isinstance(m, tuple) else m for m in matches])
            else:
                processed.extend(matches)

        # Handle default values if no matches found
        if not processed and "default" in config:
            default = self._resolve_context_value(config["default"])
            processed = default if isinstance(default, list) else [default]

        if config.get("allowed_keywords"):
            processed = [
                item
                for item in processed
                if self._validate_keyword(item, config["allowed_keywords"])
            ]

        return processed

    def _validate_keyword(self, item: Any, allowed: List[str]) -> bool:
        """Validate change type keywords against allowed list"""
        if isinstance(item, dict):
            key = next(iter(item.keys()))
            if key not in allowed:
                print(f"Warning: Disallowed keyword '{key}' found, skipping entry")
                return False
        return True

    def _apply_processor(
        self, matches: List[Union[str, Tuple]], processor: str
    ) -> List[Any]:
        """Handle different processor types safely"""
        if processor == "structured_links":
            return [self._process_structured_link(m) for m in matches]
        elif processor == "flat_list":
            return [m[0] if isinstance(m, tuple) else m for m in matches]
        return matches

    def _process_structured_link(self, match: Union[str, Tuple]) -> Dict[str, str]:
        """Convert tuple matches to structured links"""
        if isinstance(match, tuple) and len(match) >= 2:
            return {"action": match[0].lower(), "link": match[1]}
        return {"action": "unknown", "link": str(match)}

    def _resolve_reference(self, ref: str, ref_type: str) -> str:
        """Resolve references using GitHub API with type validation"""
        try:
            # Extract numeric ID from any format (#123 or 123)
            ref_num = int(ref.lstrip("#"))
            repo = self.repo_context["repo_object"]

            if ref_type == "issue":
                issue = repo.get_issue(ref_num)
                return issue.html_url
            elif ref_type == "pr":
                pr = repo.get_pull(ref_num)
                return pr.html_url

        except Exception as e:
            print(f"⚠️ Failed to resolve {ref} as {ref_type}: {str(e)}")
            return f"{self.repo_context['repo_url']}/{ref_type}s/{ref_num}"

    def _merge_data(
        self, yaml_key: str, new_data: List[Any], merge_keys: List[str]
    ) -> None:
        """Merge processed data with existing values and context"""
        current = self.parsed_data.get(yaml_key, [])

        # Add context values (e.g., PR creator)
        for key in merge_keys:
            if context_value := self._resolve_context_value(key):
                current.extend(
                    context_value
                    if isinstance(context_value, list)
                    else [context_value]
                )

        # Add new data and deduplicate
        current.extend(new_data)

        # Handle duplicate removal for both hashable and unhashable types
        seen = set()
        deduped = []
        for item in current:
            # Create a hashable representation for dictionaries
            if isinstance(item, dict):
                item_repr = frozenset(item.items())
            else:
                item_repr = item

            if item_repr not in seen:
                seen.add(item_repr)
                deduped.append(item)

        self.parsed_data[yaml_key] = deduped

    def _resolve_context_value(self, value: Union[str, Any]) -> Any:
        """Resolve $ prefixed context variables"""
        if isinstance(value, str) and value.startswith("$"):
            return self.repo_context.get(value[1:], "")
        return value

    def _process_github_metadata(self, pr: Any) -> None:
        """Add GitHub-specific metadata to parsed data"""
        self.parsed_data.setdefault(
            "date",
            (
                pr.merged_at.strftime("%Y-%m-%d")
                if pr.merged_at
                else datetime.now().strftime("%Y-%m-%d")
            ),
        )
        self.parsed_data.setdefault("labels", sorted(label.name for label in pr.labels))
        self.parsed_data.setdefault("links", []).insert(0, pr.html_url)

    def _prepare_output(self) -> Dict[str, Any]:
        """Validate and prepare final output structure"""
        # Validate required keys
        for key in self.config.get("output", {}).get("required_keys", []):
            if key not in self.parsed_data:
                raise ValueError(f"Missing required key in output: {key}")

        # Generate filename components
        pr_number = self.repo_context["pr_number"]
        clean_title = re.sub(r"[^\w-]", "_", self.parsed_data["title"].lower()[:50])
        filename = (
            self.config["output"]["filename_template"]
            .replace("$pr_number", str(pr_number))
            .replace("$clean_title", clean_title)
        )
        
        output_dir = Path(self.config["output"]["directory"])
        
        # Delete existing files for this PR number
        existing_files = output_dir.glob(f"{pr_number}_*.yaml")
        for file in existing_files:
            try:
                file.unlink()
                print(f"Removed previous changelog: {file}")
            except Exception as e:
                print(f"Error removing {file}: {str(e)}")

        # Create ordered output structure
        ordered_data = CommentedMap()
        output_order = self.config["output"].get("order", [])

        # Add keys in specified order
        for key in output_order:
            if key in self.parsed_data:
                ordered_data[key] = self.parsed_data[key]

        # Add any remaining keys not in order list
        for key in self.parsed_data:
            if key not in ordered_data:
                ordered_data[key] = self.parsed_data[key]

        return {
            "data": ordered_data,
            "filename": output_dir / filename
        }


def main():
    parser = argparse.ArgumentParser(
        description="Generate YAML changelog from merged GitHub PR."
    )
    parser.add_argument("--pr-number", type=int, required=True, help="Merged PR number")
    parser.add_argument(
        "--repo",
        help="GitHub repository (owner/repo)",
        default=os.getenv("GH_REPOSITORY"),
    )
    parser.add_argument(
        "--token",
        help="GitHub personal access token",
        default=os.getenv("GH_TOKEN"),
    )
    parser.add_argument(
        "--output-dir", default="changelogs", help="Output directory for YAML files"
    )
    parser.add_argument("--config", default="pr_changelog_config.yaml")
    parser.add_argument(
        "--force", action="store_true", help="Run even if PR is not merged"
    )

    args = parser.parse_args()

    # Authenticate with GitHub
    auth = Auth.Token(args.token)
    github = Github(auth=auth)
    repo = github.get_repo(args.repo)
    pr = repo.get_pull(args.pr_number)

    if not pr.merged and not args.force:
        print(f"PR #{args.pr_number} is not merged. Exiting.")
        return

    # Process PR and write output
    processor = PRProcessor(Path(__file__).parent.parent / args.config)
    result = processor.process_pr(pr, repo)

    yaml_handler = YAML(typ="rt")
    yaml_handler.indent(mapping=2, sequence=4, offset=2)
    result["filename"].parent.mkdir(parents=True, exist_ok=True)

    with open(result["filename"], "w") as f:
        yaml_handler.dump(result["data"], f)

    print(f"Successfully generated: {result['filename']}")


if __name__ == "__main__":
    main()
