# How to Write a Pull Request
When submitting a pull request, it is required that the pull request templated is followed. To do this requires some knowledge of markdown, and the format in which the different fields should be filled out. 

This document will provide a brief overview of the different fields, and what keywords should be used to fill them out.

## Table of Contents
- [How to Write a Pull Request](#how-to-write-a-pull-request)
  - [Table of Contents](#table-of-contents)
  - [Required Fields](#required-fields)
    - [Title](#title)
    - [Change list](#change-list)
  - [Optional Fields](#optional-fields)
    - [Author(s)](#authors)
    - [Fixes Issue(s)](#fixes-issues)
    - [Updates PR(s)](#updates-prs)
    - [Additional Notes](#additional-notes)


## Required Fields
The following fields are required to be filled out. This is to help the maintainers and reviews understand what the pull request is about, what changes are being made and what the pull request affects.


### Title
The title needs to follow the folling format:
```
[<Type>] <Short Description>
```
Where `<Type>` is the area the pull request is affecting, and the `<Short Description>` is a title for the pull request. The following `<Type>`'s are available:
- [GEN] = `Generals`
- [ZH] = `Zero Hour`
- [GITHUB] = `GitHub`
- [DOCS] = `Documentation`
- [BUILD] = `Build`

Multiple types can be used if the pull request affects multiple areas. Types need to be separated by `/`.

**Example Title:**
```
[GEN/ZH] Improving AI pathfinding
```
```
[BUILD] Updated Cmake to version 3.20
```
```
[GITHUB/DOCS] Updated pull request template
```


### Change list
Changes made in the pull request should be in list form, and follow the following format:
```
- <Type>: <Description>
```
Where `<Type>` is the type of change being made, and `<Description>` is a short description of the change. The following `<Type>`'s are available:
- fix = `Fixes a bug`
- feat = `Adds a new feature`
- breaking = `Introduces a breaking change, breaks backwards compatibility`
- build = `Changes to the build system`
- ci = `Changes to the CI/CD pipeline`
- docs = `Changes to the documentation`
- performance = `Improves performance`
- test = `Adds or modifies tests`

If there are multiple changes, they should all be listed, each in their own bullet point.

**Exmaple**
```
Change list
- fix: Fixed a bug where the game would crash when loading a map
- test: Added related tests for the bug fix
```
```
Change list
- breaking: Updated mismatch CRC calculation
- performance: Reduced GPU calls by 20%
- test: Added tests for the new CRC calculation
```

## Optional Fields
These next fields are optional, and can be removed if not needed. They are used to provide additional information about the pull request, and if one is applicable, it should be filled out.

### Author(s)
If there are multiple authors for the pull request, they should be listed here. The format is as follows:
```
- @<Author>
```
Where `<Author>` is the GitHub username of the author. If there are multiple authors, they should be listed in their own bullet point.

**Example**
```
* Removed because no additional authors
```
```
Author(s)
- @<PR Author>
```
```
Author(s)
- @<PR Author>
- @<Additional Author>
```

### Fixes Issue(s)
If the pull request fixes an issue, it should be listed here. The format is as follows:
```
- <Task> #<Issue Number>
```
Where `<Task>` is the GitHub keyword for closing an issue. The following keywords are available:
- `Closes` = `Closes an issue`
- `Fixes` = `Fixes an issue`
- `Resolves` = `Resolves an issue`

And `<Issue Number>` is the number of the issue that is being fixed. If there are multiple issues, they should be listed in their own bullet point.

**Example**
```
Fixes Issue(s)
- Fixes #123
```
```
Fixes Issue(s)
- Closes #123
- Closes #124
```

### Updates PR(s)
If a pull request is updating another pull request, or is a direct continuation of another, it should be listed here. The format is as follows:
```
- <Task> #<PR Number>
```
Where `<Task>` is the GitHub keyword for updating a PR. The following keywords are available:
- `Updates` = `Updates an issue`
- `Continues` = `Continues an issue`

And `<PR Number>` is the number of the PR that is being updated. If there are multiple PRs, they should be listed in their own bullet point.

**Example**
```
Updates PR(s)
- Updates #123
```
```
Updates PR(s)
- Continues #123
```

### Additional Notes
Lastly in the additional notes section, any additional information that is not related to the changelog can be added. This can be anything from additional information about the pull request, to notes about the changes made.