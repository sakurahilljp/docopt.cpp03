# Rules

## Git Workflow and Code Review Process

Whenever modifying code, creating branches, committing, or merging changes in this project, you must adhere to the following workflow steps:

1. **Work in a Feature Branch**: Always create and switch to a dedicated working branch (e.g., `test/name`, `feature/name`, `fix/name`) before making any changes. Never work directly on `main` unless explicitly granted a temporary exception by the user.
2. **Propose Before Modifying**: Before editing any files, present the proposed code changes (diff or concept) to the user and obtain their explicit approval ("OK" or equivalent). Do not modify files using code-writing tools before this approval.
3. **Update Changelog**: When modifying functional code or specifications, always update the project's changelog file (e.g., `CHANGELOG.md` or `ChangeLog.md`) to document the changes before requesting a review.
4. **Request Review Before Committing**: Once the changes and changelog updates are applied and verified (e.g., tests pass), present the exact code diff and request a code review from the user.
5. **Commit Only After Review Approval**: Commit the changes to the working branch only after the user reviews and approves the modified code.
6. **Request Approval Before Merging**: Before merging the working branch into `main`, ask for the user's explicit permission.
7. **Use Normal Merge**: When merging the approved working branch into `main`, always perform a normal merge (non-fast-forward) using `git merge --no-ff` to preserve history, unless instructed otherwise.
