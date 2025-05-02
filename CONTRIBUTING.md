# How to contribute as a developer

To contribute, fork this repository to create your own copy that you can clone locally and push back to. You can use your fork to create pull requests for your code to be merged into this repository.

## Code guidelines

### Scope of code changes

Code edits only touch the lines of code that serve the intended goal of the change. Big refactors should not be combined with logical changes, because these can become very difficult to review. If a change requires a refactor, create a commit for the refactor before (or after) creating a commit for the change. A Pull Request can contain multiple commits and can be merged with **Rebase and Merge** if these commits are meant to be preserved on the main branch. Otherwise, method of merging will be **Squash and Merge**.

### Style of code changes

Code edits should fit the nearby code in ways that the code style reads consistent, unless the original code style is bad. The original game code uses c++98, or a deviation thereof, and is simple to read. Prefer not to use newer language features unless required to implement the desired change. Prefer to use newer language features when they are considerably more robust or make the code easier to understand or maintain.

### Language style guide

*Work in progress. Needs a maintainer. Can be built upon existing Code guidelines, such as the "Google C++ Style Guide".*

### Precedence of code changes

Changes to Zero Hour take precedence over Generals, if applicable. When the changed code is not shared by both titles, then the change needs to be created for Zero Hour first, and then recreated for Generals. The implementation of a change for both titles needs to be identical or as close as possible. Preferably the Generals replica of a change comes with the same Pull Request. The Generals replica can be created after the Zero Hour code review has finished.


## Change documentation

User facing changes need to be documented in code, Pull Requests and change logs. All documentation ideally is written in the present tense, and not the past.

Good:

> Fixes particle effect of USA Missile Defender

Bad:

> Fixed particle effect of USA Missile Defender

When a text refers to a faction unit, structure, upgrade or similar, then the unit should be worded without any abbrevations and should be prefixed with the faction name. Valid faction names are USA, China, GLA, Boss, Civilian. Subfaction names can be appended too, for example GLA Stealth.

Good:

> Fixes particle effect of USA Missile Defender

Bad:

> Fixes particle effect of MD


### Code documentation

User facing changes need to be accompanied by comment(s) where the change is made. Maintenance related changes, such as compilation fixes, typically do not need commenting, unless the next reader can benefit from a special explanation. The comment can be put at the begin of the changed file, class, function or block. It must be clear from the change description what has changed.

The expected comment format is

```
// TheSuperHackers @keyword author DD/MM/YYYY A meaningful description for this change.
```

The `TheSuperHackers` word and `@keyword` are mandatory. `author` and date can be omitted when preferred.

| Keyword          | Use-case                                                    |
|------------------|-------------------------------------------------------------|
| @bugfix          | Fixes a bug                                                 |
| @fix             | Fixes something, but is not a user facing bug               |
| @compile         | Addresses a compile warning or error                        |
| @feature         | Adds something new                                          |
| @performance     | Improves performance                                        |
| @refactor        | Moves or rewrites code, but does not change the behaviour   |
| @tweak           | Changes values or settings                                  |
| @info            | Writes useful information for the next reader               |
| @todo            | Adds a note for something left to do if really necessary    |

Block comment sample

```
    // TheSuperHackers @bugfix JAJames 17/03/2025 Fix uninitialized memory access and add more Windows versions.
    memset(&os_info,0,sizeof(os_info));
```

Optionally, the pull request number can be appended to the comment. This can only be done after the pull request has been created.

```
// TheSuperHackers @bugfix JAJames 17/03/2025 Fix uninitialized memory access and add more Windows versions. (#123)
```

### Pull request documentation

The title of a new Pull Request, and/or commit(s) within, begin with a **[GEN]** and/or **[ZH]** tag, depending on the game(s) it targets. If a change does not target a game, then a tag is not necessary. Furthermore, the title consists of a concise and descriptive sentence about the change and/or commit, beginning with an uppercase letter and ending without a dot. The title ideally begins with a word that describes the action that the change takes, for example `Fix *this*`, `Change *that*`, `Add *those*`, `Refactor *thing*`.

Good:
```
[GEN][ZH] Fix uninitialized memory access in Get_OS_Info
```

Bad:
```
Minimal changes for successful build.
```

Currently established commit title tags are

* [GEN]
* [ZH]
* [CORE]
* [CMAKE]
* [GITHUB]
* [LINUX]

If the Pull Request is meant to be merged with rebase, then a note for **Merge with Rebase** should be added to the top of the text body, to help identify the correct merge action when it is ready for merge. All commits of the Pull Request need to be properly named and need the number of the Pull Request added as a suffix in parentheses. Example: **(#333)**. All commits need to be able to compile on their own without dependencies in newer commits of the same Pull Request. Prefer to create changes for **Squash and Merge**, as this will simplify things.

The text body begins with links to related issue report(s) and/or Pull Request(s) if applicable.

To write a link use the following format:

```
* Fixes #222
* Closes #333
* Relates to #555
* Follow up for #666
```

Links are commonly used for

* closing a related issue report or task when this pull request is merged
* closing another pull request when this pull request is merged

Some keywords are interpreted by GitHub. Read about it [here](https://docs.github.com/en/issues/tracking-your-work-with-issues/linking-a-pull-request-to-an-issue).

The text body continues with a description of the change in appropriate detail. This serves to educate reviewers and visitors to get a good understanding of the change without the need to study and understand the associated changed files. If the change is controversial or affects gameplay in a considerable way, then a rationale text needs to be appended. The rationale explains why the given change makes sense.


### Pull request merging rules

Please be mindful when merging changes. There are pitfalls in regards to the commit title consistency.

When attempting to **Squash and Merge** a Pull Request that contains a single commit, then GitHub will default generate a commit title from that single commit. Typically this is undesired, when the new commit title is meant to be kept in sync with the Pull Request title rather than the Pull Request commit title. The generated commit title may need to be adjusted before merging the Pull Request.

When attempting to **Squash and Merge** a Pull Request that contains multiple commits, the GitHub will default generate a commit title from the Pull Request title. Additionally it will generate a commit description from the multiple commits that are part of the Pull Request. The generated commit description generally needs to be cleared before merging the Pull Request to keep the commit title clean.

When attempting to **Rebase and Merge** a Pull Request, then all commits will transfer with the same names to the main branch. Verify that all commit titles are properly crafted, with tags where applicable, trailing Pull Request numbers in parentheses and no unnecessary commit descriptions (texts below the commit title).


### Change log documentation

*Work in progress.*
