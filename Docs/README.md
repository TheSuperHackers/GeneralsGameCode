## Generals Modding Guide

### What is this documentation and its purpose?

This folder hosts the authoritative, living documentation for the community-maintained Generals Game Code project (Generals and Zero Hour). The docs explain how gameplay systems, INI classes, modules, behaviors, and enums work in this open-source upgrade of the original games, so modders can rely on a single, consistent reference.

We expect the docs to be:

- Accurate: Reflect the Retail 1.08 (Generals) and 1.04 (Zero Hour) code
- Complete: Each property lists type, description, default, example, and version flags
- Consistent: Shared concepts are described identically across documents
- Practical: Avoid engine-internal jargon; focus on modder outcomes

### Versions explained (community upgrade branch)

These docs track the community upgrade branch built on the classic releases. Use flags on every property and enum value to communicate baseline availability:

- `*(Retail Generals 1.08 baseline)*` — rooted in the original Generals 1.08 behavior
- `*(Retail Zero Hour 1.04 baseline)*` — rooted in the original Zero Hour 1.04 behavior

Notes:

- The project fixes bugs and may add improvements while striving for compatibility; behavior may differ from the original binaries where fixes apply.
- When something exists only in Zero Hour, mark it `*(Retail Zero Hour 1.04 baseline)*`.

### Why documentation belongs in the open-source community repo

Keeping docs here ensures:

- Updates to code and docs are reviewed together
- Docs remain current with fixes and clarifications
- Contributors can maintain docs alongside code changes
- Consistent standards across all documents

### How documentation is created (with AI involvement)

Process used to author and maintain Retail docs:

1) Code and behavior study
- Identify the class/behavior and its base class
- Read `buildFieldParse` and `FieldParse` tables in `.cpp`
- Verify member types/defaults in headers
- Locate enums and their string tables

2) Draft the document
- Overview and Usage (Limitations, Conditions, Dependencies)
- Properties: Type, Description, Default, Example, Version flags
- Enums: full value lists with flags
- Examples and a complete Template

3) Versioning
- Mark Retail Generals 1.08 vs Retail Zero Hour 1.04 per property/enum value

4) Cross-linking
- Link to other Retail docs using relative paths

5) Review cycle
- Self-consistency check against related docs
- Peer review (target 2 approvals)
- Apply feedback and update Last Updated + Document Log

6) AI assistance
- AI may assist with drafting and standardization
- Authors validate details against source and correct inaccuracies
- Avoid mentioning engine-internal function names in user-facing text

### Contributor guide: required structure for each document

1) Overview
2) Usage
- Limitations
- Conditions
- Dependencies

3) Table of Contents

4) Properties
- One subsection per property: Type, Description, Default, Example, Version flags
- Enum sections listing every value with flags

5) Examples

6) Template
- Use ```ini with inline comments and version flags
- Defaults should reflect source initializers

7) Notes

8) Source Files
- Base class line first
- Include both Retail Generals and Retail Zero Hour paths

9) Changes History

10) Document Log
- Reset to: “- 16/12/2025 — AI — Initial document created.”

11) Status and Reviewers

Style and consistency rules:

- Use modder-friendly language; no internal function names
- Keep shared text identical across related body modules
- Use `Type: Alias (see [DocName](RelativePath.md))` for cross-references
- Apply the same section ordering across all docs

