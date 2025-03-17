from changelogger import Changelogger
from dotenv import load_dotenv
from github import Github, Repository
import argparse
import os
from changelogger import ChangelogEntryPosition

# Load environment variables from a .env file
load_dotenv()

def authenticate(repo: str) -> tuple[Github, Repository.Repository]:
    """
    Authenticate with the GitHub API.

    :param token: The GitHub token.
    :return: A tuple containing the GitHub object and the repository object.
    """

    global ghub, repository
    try:
        ghub = Github(os.getenv("GH_TOKEN"))
        repository = ghub.get_repo(repo)
        return ghub, repository
    except:
        print("Authentication failed.")
        return False


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Generate a changelog for a GitHub repository.")

    parser.add_argument("--repo", type=str, help="The repository to generate the changelog for.", default="")
    parser.add_argument("-b", "--branch", type=str, help="The branch to generate the changelog from.", default="main")
    parser.add_argument('-f', "--file-name", type=str, help="The name of the changelog file.", default="CHANGELOG.md")
    parser.add_argument("-pr", "--use-pull-requests", type=str, help="Use pull requests to generate the changelog.", default="true")
    parser.add_argument("-c", "--use-commits", type=str, help="Use commit messages to generate the changelog.", default="true")
    parser.add_argument("--insert-position", type=str, help="The position to insert the changelog entry.", default="above_previous", choices=[position.value for position in ChangelogEntryPosition])
    parser.add_argument("--commit-message", type=str, help="The commit message to use when updating the changelog.", default=Changelogger.COMMIT_MESSAGE)

    parser.add_argument("-r", "--release", type=str, help="Generate a release changelog.", default="false")
    parser.add_argument("--draft", type=str, help="Mark the release as a draft. Requires release is true", default="false")
    parser.add_argument("--prerelease", type=str, help="Mark the release as a prerelease. Requires release is true", default="false")

    parser.add_argument("--dry-run", type=str, help="Perform a dry run.", default="false")

    args = parser.parse_args()

    arg_release = args.release.lower() == "true"
    arg_draft = args.draft.lower() == "true"
    arg_prerelease = args.prerelease.lower() == "true"
    arg_dry_run = args.dry_run.lower() == "true"

    if not arg_release and (arg_draft or arg_prerelease):
        parser.error("--draft and --prerelease require --release to be set to 'true'.")

    ghub, repo = authenticate(args.repo)
    
    changelogger = Changelogger(repo,
                                args.branch,
                                args.file_name,
                                args.use_pull_requests,
                                args.use_commits,
                                arg_release,
                                arg_draft,
                                arg_prerelease,
                                ChangelogEntryPosition(args.insert_position),
                                args.commit_message,
                                dry_run=arg_dry_run)
    
    if changelogger.create_new_changelog_entry():
        print("Changelog entry created.")
    else:
        exit(1)