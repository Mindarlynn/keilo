# Testing Rules

These rules apply when adding or changing behavior.

## Required Coverage

- Every new feature or behavior change must include unit tests or updates to existing tests.
- Tests must cover the changed behavior, not unrelated code.
- Do not add broad unrelated refactors while adding tests.

## Test Design

- Tests should use existing test patterns and helpers.
- Tests must avoid fixed machine paths, real credentials, and external services unless explicitly marked as integration tests.
- Unit tests must be deterministic.

## Verification

- After behavior changes, run the narrowest relevant compile or test command.
- Until this repository has a dedicated test runner, use the compile verification documented in `.travis.yml` as the default full verification path.
- If the full verification is too expensive or unavailable, run the narrowest relevant verification and document what was not run.
- Documentation-only changes do not require tests unless they affect generated artifacts, validation scripts, or repository automation.
