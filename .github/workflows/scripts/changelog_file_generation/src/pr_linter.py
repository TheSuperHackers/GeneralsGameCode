"""
PR Format Linter - Validates GitHub Pull Requests against configuration templates.
"""

import argparse
import re
import sys
from pathlib import Path
from typing import Dict, List
import yaml
from dotenv import load_dotenv
import os
load_dotenv()

from github import Github, GithubException

class PRLinter:
    """Validates GitHub PR formatting against configuration templates using PR number."""
    
    def __init__(self, config_path: str = "pr_changelog_config.yaml") -> None:
        """
        Initialize linter with validation rules.
        
        Args:
            config_path: Path to YAML config file
        """
        with open(config_path) as f:
            self.config = yaml.safe_load(f)
        self.errors: List[str] = []

    def lint(self, pr_title: str, pr_body: str) -> bool:
        """
        Validate PR title and body against configured rules.
        
        Args:
            pr_title: Pull request title
            pr_body: Pull request body content
            
        Returns:
            True if valid, False if errors exist
        """
        self._validate_title(pr_title)
        self._validate_body(pr_body)
        return len(self.errors) == 0

    def _validate_title(self, title: str) -> None:
        """Validate PR title structure and scopes.
        
        Checks:
        - Title starts with valid scope brackets
        - Scopes match allowed values from config
        """
        scope_pattern = self.config["title_processing"].get("scope_pattern", r"^\[([\w\/]+)\]")
        allowed_scopes = self.config["title_processing"].get("allowed_scopes", [])

        # Check scope prefix exists
        if not re.search(scope_pattern, title):
            self.errors.append("Title missing required scope prefix (e.g. [GEN])")
            return

        # Extract and validate scopes
        scopes = []
        remaining = title
        while match := re.match(scope_pattern, remaining):
            scopes.extend(match.group(1).upper().split("/"))
            remaining = remaining[match.end():].strip()

        if allowed_scopes:
            invalid = [s for s in scopes if s not in allowed_scopes]
            if invalid:
                self.errors.append(
                    f"Invalid title scopes: {', '.join(invalid)}. "
                    f"Allowed values: {', '.join(allowed_scopes)}"
                )

    def _parse_sections(self, body: str) -> Dict[str, List[str]]:
        """Parse PR body into sections using config header patterns"""
        sections = {}
        current_section = None
        body = re.sub(r"<!--.*?-->", "", body, flags=re.DOTALL)
        
        # Build header patterns from config
        header_patterns = {
            name: re.compile(config["header"], re.IGNORECASE)
            for name, config in self.config["sections"].items()
        }

        for line in body.split("\n"):
            line = line.strip()
            if line.startswith("## "):
                header_text = line[3:].split("<!--")[0].strip()
                current_section = None
                
                # Match against all configured headers
                for section_name, pattern in header_patterns.items():
                    if pattern.search(header_text):
                        current_section = section_name
                        break
                
                if current_section:
                    sections.setdefault(current_section, [])
                continue
                
            if current_section:
                sections[current_section].append(line)

        return sections

    def _validate_body(self, body: str) -> None:
        """Validate sections using config-defined headers"""
        sections = self._parse_sections(body)
        
        # Check required sections
        required = [
            name for name, config in self.config["sections"].items()
            if config.get("required")
        ]
        for section in required:
            if section not in sections:
                self.errors.append(f"Missing required section: {self.config['sections'][section]['header']}")

        # Validate existing sections
        for name, content in sections.items():
            config = self.config["sections"][name]
            self._validate_section_content(
                section_name=name,
                header_name=config["header"],  # Use display name for errors
                lines=content,
                pattern=config["pattern"],
                allowed_keywords=config.get("allowed_keywords"),
                processor=config.get("processor")
            )


    def _validate_section_content(
        self,
        section_name: str,
        header_name: str,
        lines: List[str],
        pattern: str,
        allowed_keywords: List[str],
        processor: str
    ) -> None:
        """Validate section content with detailed error messages"""
        section_errors = []
        expected_format = self.config['sections'][section_name].get('example', '')

        for line_num, raw_line in enumerate(lines, 1):
            line = raw_line.strip()
            if not line:
                continue

            # Validate list item formatting
            if not line.startswith('- ') and not processor == "flat_list":
                section_errors.append(
                    f"Line {line_num}: Item must be a list element starting with '- '"
                )
                continue

            clean_line = line[2:].strip()
            
            # Validate colon presence for key-value sections
            if processor == "key_value" and ':' not in clean_line:
                parts = clean_line.split(' ', 1)
                if len(parts) > 0 and parts[0].lower() in allowed_keywords:
                    section_errors.append(
                        f"Line {line_num}: Missing colon after keyword. "
                        f"Should be '{parts[0]}: {parts[1]}'"
                    )
                else:
                    section_errors.append(
                        f"Line {line_num}: Invalid format. Expected: {expected_format}"
                    )
                continue

            # Validate pattern match
            match = re.fullmatch(pattern, clean_line)
            if not match:
                section_errors.append(
                    f"Line {line_num}: Invalid format. Expected: {expected_format}"
                )
                continue

            # Validate keywords
            if allowed_keywords:
                keyword = match.group(1).lower()
                if keyword not in allowed_keywords:
                    section_errors.append(
                        f"Line {line_num}: Invalid keyword '{keyword}'. "
                        f"Allowed: {', '.join(allowed_keywords)}"
                    )

        if section_errors:
            self.errors.append(
                f"**{header_name} errors**:\n" + "\n".join(section_errors)
            )

    def print_errors(self) -> None:
        """Print formatted validation errors to stderr."""
        if self.errors:
            print("\nPR Validation Errors Found:", file=sys.stderr)
            for error in self.errors:
                print(f"  • {error}", file=sys.stderr)
            print("\nPlease refer to the PR template guidelines.", file=sys.stderr)
        else:
            print("PR format is valid!")

def create_or_update_comment(
        pr, 
        message: str, 
        bot_marker: str = "<!-- LinterBot -->"
    ) -> None:
        """Create or update existing linter comment on PR"""
        existing_comment = None
        for comment in pr.get_issue_comments():
            if bot_marker in comment.body:
                existing_comment = comment
                break

        full_message = f"{message}\n{bot_marker}"
        
        if existing_comment:
            existing_comment.edit(full_message)
        else:
            pr.create_issue_comment(full_message)

def format_comment(errors: List[str], pr_number: int) -> str:
    """Format validation results as a GitHub comment"""
    if not errors:
        return f"## ✅ PR #{pr_number} Format Validation\nNo linting errors found!"

    error_list = "\n".join(f"❌ {error}\n" for error in errors)
    return f"## ❌ PR #{pr_number} Format Issues\nThe following formatting issues were found:\n\n{error_list}\nPlease check the PR template guidelines."

def main():
    """CLI for validating PRs using GitHub PR number."""
    parser = argparse.ArgumentParser(
        description="Validate PR format and report via GitHub comments",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        "--pr-number",
        type=int,
        required=True,
        help="Pull request number to validate"
    )
    parser.add_argument(
        "--repo",
        default=os.getenv("GH_REPOSITORY"),
        type=str,
        help="Repository in owner/repo format"
    )
    parser.add_argument(
        "--token",
        default=os.getenv("GH_TOKEN"),
        type=str,
        help="GitHub token (default: GITHUB_TOKEN env var)"
    )
    parser.add_argument(
        "--config",
        default="pr_changelog_config.yaml",
        help="Path to validation config YAML"
    )
    parser.add_argument(
        "-q", "--quiet",
        action="store_true",
        help="Suppress output except exit status"
    )

    args = parser.parse_args()

    try:
        # GitHub connection setup
        g = Github(args.token)
        repo = g.get_repo(args.repo)
        pr = repo.get_pull(args.pr_number)

        # Run validation
        linter = PRLinter(Path(__file__).parent.parent / args.config)
        is_valid = linter.lint(pr.title, pr.body or "")
        
        # Format and post comment
        comment_body = format_comment(linter.errors, args.pr_number)
        create_or_update_comment(pr, comment_body)

        # CLI output unless quiet
        if not args.quiet:
            linter.print_errors()
            print(f"\nPosted validation results to PR #{args.pr_number}")

        sys.exit(0 if is_valid else 1)

    except GithubException as e:
        print(f"GitHub API Error: {str(e)}", file=sys.stderr)
        sys.exit(2)
    except Exception as e:
        print(f"Runtime Error: {str(e)}", file=sys.stderr)
        sys.exit(3)

if __name__ == "__main__":
    main()