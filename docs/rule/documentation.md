# Documentation Rules

These rules apply whenever a `plan/`, `rule/`, or `log/` document is created or updated.

## Folder Responsibilities

- `rule/` must contain current mandatory policies.
- `plan/` must contain execution plans and current implementation state.
- `log/` must contain feature-level or decision-level history.
- Do not use one folder to replace another folder's responsibility.

## Rule Documents

- Rules must be concise and directive.
- Rules must describe current mandatory behavior only.
- Do not store long rationale, rejected alternatives, or implementation history in rules.
- New rules must be linked from `rule/README.md`.
- When a rule conflicts with an older plan or log, follow the rule and update the stale plan or log if the change affects current work.

## Plan Documents

- Keep `plan/` organized by state: `active/`, `completed/`, and `archived/`.
- Each plan must be a folder, not a single large markdown file.
- Each plan folder must contain an `overview.md`.
- Completed plan steps must link related `log/` and `rule/` documents from `overview.md`.

## Log Documents

- Keep `log/` organized by feature, domain, or decision area, not by execution status.
- A log document should record decisions made, reasons for changes, risks considered, implementation notes, related plan steps, and related rules.
- Logs may preserve older detailed planning text when it is useful history.
- Logs must not become the source of current mandatory policy.

## Link Rules

- Prefer relative markdown links.
- Verify markdown links after a documentation structure change.
