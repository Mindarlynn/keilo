# Project Rules

This folder contains rules that must be followed when planning or changing this repository.

`plan/` records execution plans, decisions, risks, and implementation notes. `log/` records historical rationale and completed work details. `rule/` records the current mandatory policies. When a rule and an old plan or log conflict, follow `rule/`.

## Rule Index

- [Workflow](./workflow.md)
- [Planning](./planning.md)
- [Development](./development.md)
- [Documentation](./documentation.md)
- [Testing](./testing.md)

## Update Policy

- Follow [Workflow](./workflow.md) when executing plans or making implementation changes.
- Follow [Planning](./planning.md) when turning a request into a plan.
- Follow [Development](./development.md) when changing code or behavior.
- Follow [Documentation](./documentation.md) when creating or updating `plan/`, `rule/`, or `log/` documents.
- Follow [Testing](./testing.md) when adding or changing behavior.
- Update rules in the same change when implementation or workflow changes alter mandatory behavior.
- Keep rules concise and directive.
- Do not store feature history, rejected alternatives, or long rationale in rules; keep those in `log/` and link related plan steps.
- Do not add speculative future features as rules.
