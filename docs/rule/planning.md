# Planning Rules

These rules apply when turning a user request or high-level concept into an implementation plan.

## Scope

- Plans must be the smallest set of steps that can satisfy the requested goal.
- Do not include unrequested features, speculative improvements, or broad refactors.
- Identify what is explicitly in scope and what is out of scope.

## Required Risk Review

- Each plan must list relevant risks before implementation starts.
- Consider security, performance, resource usage, maintainability, compatibility, data loss, migration, and testing risks.
- Each listed risk must include a mitigation.
- If a risk cannot be reasonably mitigated, stop and ask the user before implementation.

## Plan Shape

- Each plan must have an `overview.md`.
- Each implementation step must be split into its own markdown file when the work has multiple steps.
- Each step file must include:
  - `Goal`
  - `Scope`
  - `Decision`
  - `Reason`
  - `Risks`
  - `Mitigation`
  - `Implementation`
  - `Tests`
- Step titles must use `[ ]` for planned work and `[x]` for completed work.

## Execution Gate

- Do not start implementing a non-trivial plan until the risks and mitigations are documented.
- If the user explicitly asks for immediate implementation, keep the plan lightweight but still apply the risk and mitigation check.
