# Development Rules

These rules apply when changing code, behavior, configuration, or repository automation.

## Minimal Change

- Implement only the requested behavior.
- Do not add unrequested features.
- Do not perform broad refactors unless required for the requested change.
- Prefer existing project patterns over new abstractions.
- Add a new abstraction only when it removes real complexity or matches an established local pattern.

## Behavior Changes

- Behavior changes must be intentional and documented in the related plan or log.
- Public API, data format, command, configuration, or workflow changes must update the related rule or documentation.
- If the requested change conflicts with an existing rule, stop and ask the user before changing the rule.

## User Changes

- Do not revert user changes unless the user explicitly asks.
- If existing uncommitted changes affect the requested work, preserve them and work with them.
- If unrelated uncommitted changes exist, leave them alone.

## Completion

- Finish implementation with the relevant tests or verification.
- Record important implementation notes in `log/` when they explain decisions that future work may need.
