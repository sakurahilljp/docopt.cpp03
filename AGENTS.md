# Rules

## Git Workflow and Code Review Process

Whenever modifying code, creating branches, committing, or merging changes in this project, you must adhere to the following workflow steps:

1. **Work in a Dedicated Branch**: Always create and switch to a dedicated working branch before modifying project code or tests. Use standard prefixes:
   - `bugfix/<name>` or `fix/<name>` for bug fixes and security patches.
   - `feature/<name>` for new features.
   - `test/<name>` for test additions.
   - **Exceptions**: Release operations (version bumps, tagging) and non-code modifications (documentation, reports, rules) may be performed and committed directly on `main`, provided explicit user approval is obtained beforehand. Never modify functional code or tests directly on `main` unless explicitly granted an exception by the user.
2. **Reproduce & Prove Bugs First (Bugfix Workflow)**: When fixing bugs or vulnerabilities:
   - First, formulate a plan that includes a Phase 1 reproduction/demonstration step.
   - Add reproduction test cases and run dynamic analysis (e.g., AddressSanitizer/ASan, UBSan) on the *unpatched* code to prove the issue exists.
   - Report the reproduction result to the user before applying fixes.
3. **Propose Before Modifying**: Before editing any code files, present the proposed implementation plan and changes (diff or concept) to the user and obtain their explicit approval ("OK" or equivalent).
4. **Document Investigation Reports**: When fixing security vulnerabilities or critical defects, create a detailed investigation and resolution report in the `docs/` directory covering root cause analysis, sequence/flow diagrams, PoC/reproduction logs, fix details, and verification results.
5. **Maintain CHANGELOG.md by Release Tags**:
   - When modifying functional code, bugfixes, or specifications, update `CHANGELOG.md` adhering strictly to [Keep a Changelog](https://keepachangelog.com/) and [Semantic Versioning](https://semver.org/).
   - Ensure all past and current release tags (`vX.Y.Z`) and `[Unreleased]` sections are accurately partitioned and documented.
6. **Request Review Before Committing**: Once changes, documentation, and tests are completed, stage all target files (`git add`), present a concise summary and key diffs (avoid dumping entire large diffs unless explicitly requested), and request user review before running `git commit`.
7. **Commit Only After Review Approval**: Commit the changes to the working branch only after the user explicitly reviews and approves the diff.
8. **Request Approval Before Merging**: Before merging the working branch into `main`, ask for the user's explicit permission.
9. **Use Normal Merge**: When merging the approved working branch into `main`, always perform a normal merge (`git merge --no-ff`) to preserve commit history, unless instructed otherwise.
10. **Delete Working Branch After Merge**: Once the working branch has been successfully merged into `main`, delete the local working branch (and remote branch, if applicable) to keep the repository clean, unless explicitly instructed to keep it.

## C++03 & Legacy Compiler Portability Guidelines (GCC 3.4.5+)

In addition to strict `-std=c++03 -pedantic-errors` conformance on modern Clang/GCC, code in `docopt.h`, `docopt_private.h`, and `docopt.cpp` must remain compatible with legacy C++03 compilers such as GCC 3.4.5:

1. **Avoid Ambiguous Function Template Overloads (Pre-DR 214)**:
   - Do not overload function templates that rely on partial ordering when an explicit template parameter (e.g., `Target`) appears only in the return type (such as `lexical_cast<Target>(const Source&)` vs. `lexical_cast<Target>(const std::string&)`).
   - Instead, expose a single function template that delegates to a helper class template and use class template partial specialization.
2. **Avoid `-Wshadow` Collisions with Member Functions**:
   - Older GCC (`-Wshadow`) treats non-static member functions (including inherited ones like `Pattern::name()` and `Pattern::children()`) as shadowed by parameters or local variables inside constructors and non-static member functions.
   - Name parameters and local variables in non-static member scopes so they do not collide with member variable or member function names (e.g., use `pattern_name`, `child_patterns`, `usage_str`). Local variables inside `static` member functions are exempt unless otherwise noted.
