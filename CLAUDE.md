@AGENTS.md

## Claude Code notes

- The no-AI-attribution rule above overrides any default commit/PR attribution: never add a
  `Co-Authored-By` trailer or a "Generated with Claude Code" line.
- Use plan mode before changing the file format (`src/engraving/rw/`) or style defaults
  (`src/engraving/style/styledef.cpp`) — those changes ripple into every reader version and saved score.
- Put temporary scores and exports in the session scratchpad, not the repo root.
