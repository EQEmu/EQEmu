# AGENTS.md

## Monomyth Repository Workflow Rules

### CHANGELOG / CHANGES REQUIREMENT

Any user-visible gameplay, balance, rule, spell, item, pet, buff, zone, combat, persistence, or configuration change MUST update:

- `CHANGELOG.md`

before PR submission.

The changelog entry should:
- briefly describe the change
- mention important gameplay implications
- mention new rules/config where applicable
- mention migrations/schema changes where applicable

### PR VALIDATION REQUIREMENT

Before marking work complete or opening a PR:
- verify `CHANGELOG.md` was updated
- verify the entry matches the implemented behavior
- fail closed if the implementation changed gameplay behavior but no changelog update exists

### MONOMYTH DESIGN PHILOSOPHY

Prefer:
- classic EQ friction
- emergent gameplay
- parser-friendly systems
- meaningful progression
- hidden optimization depth
- conservative incremental changes

Avoid:
- flattening mechanics
- over-normalization
- unnecessary automation
- removing skill expression