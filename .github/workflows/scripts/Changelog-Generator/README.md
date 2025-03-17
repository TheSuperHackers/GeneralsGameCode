# Changelog-Generator

This script is intended to work with GitHub actions in order to create a changelog based on the commits and/or PRs made to a repository.

## Requirements


* A [Personalized Access Token](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/managing-your-personal-access-tokens) to your repository. With the following permissions:
    * Contents: Read and write
    * Metadata: Read-only
    * Pull requests: Read-only
* A repository secret defining that token with the name **GH_TOKEN**.  
* A way to host GitHub actions.

## Usage
There are 2 main ways to use this tool. Howecer [GitHub Actions](#github-actions) is the intended way.

### GitHub Actions
The out of the box way to use the script is by manually dispatching a GitHub action.
1. Go to the Actions tab
2. Click `Changelog Generator` on the left side.
3. Press `Run Workflow` on the right side.
4. If the repository has multiple branches with this action, select the desired branch to run the action on.
5. Fill out the input fields.
6. Run the action by pressing `Run Workflow`.

### CommandLine
1. Clone the repository.
2. Install the package defined in `requirements.txt`.
3. Run the `main.py` file under `src`.
4. Run the script with `-h` for argument listing.

## RoadMap
* Have commit/PR SHAs link in changelog entry
* Pytests to ensure functionality before script.
* Support other date formats like YYYY-DD-MM (freedom units)

## Contact
- Email: vitexftw@gmail.com
- Discord: ViTeXFTW#6644