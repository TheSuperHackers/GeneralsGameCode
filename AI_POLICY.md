# AI Contribution Policy

Today, artificial intelligence (AI) tools are being deployed in many coding environments. In this policy, **AI-assisted** refers to the broader development workflow, including AI coding assistants, agentic development tools, and related services. **LLM-generated** refers specifically to content produced by the large language models (LLMs) used by many of these tools. Some of the "lower-end" features like keyword-driven code completion can be useful aids; other LLM-based tools make it very easy to generate contributions without the submitter fully understanding how the code works, or the consequences of specific implementation choices. These latter forms of AI assistance cause major overhead for project maintainers.

To protect the quality of the project and the sanity of our contributors, we have adopted the following policy regarding AI-generated content.

## 1. Accountability

**The human contributor is the sole party responsible for the contribution.**

If you submit a pull request that includes LLM-generated code, documentation, or comments:

- You must fully understand every line of code in the submission.
- You must be able to explain the "why" behind the implementation during the review process.
- You are responsible for verifying that the code is human-readable, maintainable and logically correct.
- All code needs to be tested and verified.

"The AI generated it and it works for me" is never an acceptable answer to a reviewer's question. You must write review responses yourself. AI tools may assist with translation, meaning-preserving edits, or formatting, but you must verify the final wording. If unsure of an answer, say so rather than presenting an AI tool's output as your own.

Autonomous AI agents may not submit pull requests, nor comments in other pull requests, issues or discussions.

## 2. Disclosure

If an LLM was used to generate a significant portion of your contribution, we require you to **disclose it** in the pull request description. You must clarify to what extent the generated content was polished by human intervention.

You must not use AI to create the pull request description itself (unless you’ve used it for translation) - as discussed above, we expect pull requests to be submitted by humans.

## 3. Intentionality

We do not accept pull requests and issues that result from running an AI tool over the codebase to find improvements without prior context or alignment with the project.

- **Focused changes:** Do not submit pull requests that perform broad refactoring or cleanup suggested by AI unless a maintainer specifically requests it.
- **Design First:** For any non-trivial change, we strongly recommend opening an **Issue** or **Discussion** first, or reach out to the **Discord**. Pull requests that arrive out of the blue with significant AI-generated logic that doesn't align with our roadmap or architecture will be closed.
- **Quality over Quantity:** We value one thoughtful, manually crafted pull request over ten AI-assisted fixes for nonexistent or trivial issues.

New contributors are discouraged from submitting pull requests with thousands of lines changed or added with the help of AI, because human code reviewers cannot attend such volumes at the risk of wasting precious time with potentially poor LLM-generated code.

## 4. Prohibited Uses

The following are strictly prohibited and will result in immediate closure of a pull request or issue and potentially a block from the organization:

- **Automated Pull Request Description:** Using AI to write the entire pull request description. The description the AI generates is often vague, overly elaborate, or fails to accurately describe the technical changes. We want to hear from *you* - the developer - why this change matters (see points 1 & 2).
- **Unvetted Boilerplate:** Submitting large blocks of LLM-generated boilerplate that haven't been trimmed to what's actually necessary. If you don't understand what the code does, don't submit the pull request.
- **Hallucinated Features:** Submitting pull requests for features or bug fixes that don't exist, based on AI hallucinations about the project's capabilities.

## 5. Enforcement

TheSuperHackers project maintainers reserve the right to close any pull request that appears to be a low-effort AI contribution, without providing a detailed technical critique. If a maintainer suspects you do not understand your pull request, it will be closed immediately.

Cases of repeated violations of these (or any of our other contributor guidelines) could result in a ban from our repositories.

### Acknowledgement

This policy was written by humans, based on the work in the [Mastodon AI Policy](https://github.com/mastodon/.github/blob/main/AI_POLICY.md), the [CloudNativePG AI Policy](https://github.com/cloudnative-pg/governance/blob/main/AI_POLICY.md), the [Ghostty AI Policy](https://github.com/ghostty-org/ghostty/blob/main/AI_POLICY.md), and the Linux Foundation's [Generative AI Policy](https://www.linuxfoundation.org/legal/generative-ai).

