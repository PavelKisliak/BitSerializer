### [BitSerializer](../../README.md) / Workflow

### Commit message convention

Use the format: `[Component] Short description (#issue)`

Use imperative mood ("Add feature", not "Added feature"). If the commit relates to a GitHub issue, add the issue number in parentheses at the end. Omit if there's no related issue.

Components:
- `[Core]` — Core serialization framework
- `[Json]` — Built-in JSON archive
- `[CSV]` — CSV archive
- `[MsgPack]` — MsgPack archive
- `[RapidJson]` — RapidJSON-based archive
- `[Convert]` — String conversion submodule
- `[Tests]` — Tests
- `[CI]` — CI/CD pipeline
- `[Docs]` — Documentation

Examples:
```
[Json] Add support for reading surrogate pairs
[CSV] Optimize deserialization performance (+22% reading from memory)
[Convert] Improve float-to-string precision
[RapidJson, PugiXml, RapidYaml] Fix loading optional object and array from `Null` (e.g. "myValue": null) (#11)
```

Keep the subject to a **single line with only the main point** — don't enumerate minor details (refactors, renames, doc tweaks, …) that happen to be part of the same change.

### Working-branch commits

On a feature branch, commit only meaningful changes. When a commit still needs fixes or refinements (review feedback, CI failures, static-analysis errors, etc.), fold them into that commit with `git commit --amend` (or `git rebase -i` to squash a follow-up) — do **not** create separate "fix" commits. This keeps history clean; the PR ends up with one commit per logical change. Only make a new commit for a genuinely new piece of work. If the branch is already pushed, force-push after amending (`git push --force-with-lease`) — but only push when the user explicitly asks.

### Pull Request workflow

- **Rebase on latest `master`** before opening a PR. If new commits appear on `master` while your PR is open, rebase again.
- Keep PR history clean — squash or amend commits locally before pushing when it makes sense.
- The project prefers a **linear history** (no merge commits). PRs are merged via rebase.
- If AI-assisted tools were used (code generation, refactoring, analysis), please include a note `AI-assisted, manually verified & adapted` in the PR description indicating that the output was reviewed and adjusted by a human.

### CI Pipeline

The project uses Azure DevOps for CI. The pipeline runs automatically on all pushes and PRs.

#### What CI checks

1. **Preflight** — Clang-tidy, Valgrind memory checks, and the GCC Debug build that publishes test results and code coverage (runs first)
2. **Build and test** — 16 configurations across Windows, Linux, macOS, and ARM

The CI matrix covers:
- Windows: MSVC (x86/x64, static/dynamic), Clang-cl (x64, Release and Debug)
- Linux: GCC 9, GCC 14 (C++23, C++26), Clang 11, Clang 18
- macOS: Clang 14 (C++17, C++20)
- ARM: arm32, arm64, arm64be (cross-compiled, tested with QEMU)
