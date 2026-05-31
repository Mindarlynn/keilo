# Workflow Rules

These rules apply when executing plans or making implementation changes.

## Base Branches

- `dev` is the base branch for plan execution and implementation work.
- `master` receives changes only through user-approved pull requests from `dev`.

## Branching

- Do not execute a plan directly on `dev` or `master`.
- Start each plan execution from `dev`, then create a new branch for that specific plan.
- Do not reuse an existing work branch for a different plan or unrelated implementation change.
- Create a new branch before starting plan execution or implementation work that changes files.
- Use the `codex/` branch prefix unless the user explicitly requests another branch name.
- Keep branch names short and related to the plan or change.

## Pull Request Flow

- Plan execution should end with changes committed on the working branch.
- Changes intended for `dev` should be merged through a pull request from the working branch.
- Changes intended for `master` should be merged only through a user-approved pull request from `dev`.
- Do not push or create a pull request unless the user asks for it or remote configuration is available and the user has already approved that workflow.
- If a remote is not configured, leave the branch and commit ready for the user to push later.

## Exceptions

- Simple local-only inspections, read-only commands, and direct answers do not require a new branch.
- Emergency fixes may bypass the branch rule only when the user explicitly asks to work on the current branch.
- Repository metadata changes that only set local git config do not require a branch.

## Documentation

- New or changed workflow behavior must be reflected in this rule.
- Plan/log/rule document changes that belong to plan execution must follow the same branch and pull request workflow.
