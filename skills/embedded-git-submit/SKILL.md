---
name: embedded-git-submit
description: Inspect embedded-firmware repository changes and prepare safe, reviewable Git commit and push commands. Use when the user asks to commit code, generate Git submission instructions, split changes into commits, or diagnose why a firmware repository cannot be committed cleanly.
---

# Embedded Git Submit

Create a reproducible Git submission plan for embedded projects. Protect source code, avoid generated firmware artifacts, and keep each commit focused.

## Operating Mode

- When the user asks for commands or instructions, inspect the repository and output exact commands without executing write operations.
- When the user explicitly asks to commit or push, perform the same inspection first, then execute only the authorized operations.
- Treat commit and push as separate actions. A request to commit does not automatically authorize a push.
- Never force-push, rewrite history, delete branches, discard changes, or amend an existing commit unless the user explicitly requests that exact action.
- Preserve unrelated user changes. Do not stage files merely because they are present.

## Inspection Workflow

Run read-only checks before proposing a submission:

```bash
git rev-parse --show-toplevel
git status --short --branch
git diff --stat
git diff --check
git diff
git diff --cached --stat
git diff --cached
git log -5 --oneline
```

Also inspect untracked files and the repository's ignore rules. Do not print secret values while checking filenames or configurations.

For embedded repositories, identify build output and local IDE state before staging. Typical files that should not enter a source commit unless the user explicitly requires them include:

```text
OBJ/ Objects/ Listings/ build/ Debug/ Release/
*.o *.d *.axf *.elf *.hex *.bin *.map *.lst
*.uvguix.* *.user *.log *.bak
.vscode/ generated dependency files and temporary programmer output
```

Do not assume every binary is disposable: calibration data, test vectors, golden images, assets, or released firmware may be intentional. Ask when classification is ambiguous.

Stop and alert the user if the candidate files appear to include credentials, private keys, tokens, production certificates, personal data, proprietary company code, or unexplained large binaries.

## Build the Commit Set

Group files by one logical purpose. Prefer separate commits for unrelated changes such as:

- driver or feature implementation;
- defect correction;
- tests;
- build-system or CI changes;
- documentation only;
- mechanical formatting or renaming.

Do not split a change when the implementation and its required test or interface update must remain atomic. Never create artificial commits merely to increase commit count.

Stage explicit paths rather than using `git add .`:

```bash
git add -- path/to/file.c path/to/file.h
git diff --cached --check
git diff --cached --stat
git diff --cached
```

If only part of a file belongs in the commit, recommend `git add -p -- path/to/file` and explain what hunk belongs to the commit.

## Commit Messages

Use the existing repository language and convention when one is clear. Otherwise use a concise Conventional Commits style:

```text
feat(uart): add DMA receive pipeline
fix(can): reject out-of-order fragments
refactor(driver): separate GPIO setup from device logic
test(protocol): cover CRC mismatch recovery
docs(freertos): explain ISR-to-task notification flow
build(ci): add host-side C tests
```

The subject should describe the result in imperative form, normally within 72 characters. Add a body only when it explains motivation, constraints, compatibility, risk, or verification that the subject cannot express.

Do not claim tests passed unless they were actually run. Record skipped verification honestly.

## Verification

Choose checks from repository evidence instead of inventing commands. Prefer, in order:

1. documented project test or build command;
2. existing CI-equivalent command;
3. host-side unit tests for hardware-independent C modules;
4. target build with the repository's established Keil, CMake, Make, or vendor workflow;
5. static checks such as `git diff --check` when the target toolchain is unavailable.

For Keil projects, do not pretend a successful host C compilation proves the complete target firmware builds. State the exact verification performed.

## Command Output Contract

When asked only for instructions, return commands in execution order using resolved branch names and explicit paths:

```bash
# 1. Review
git status --short --branch
git diff --check

# 2. Stage one logical change
git add -- Drivers/uart.c Drivers/uart.h App/comm_task.c

# 3. Verify the staged snapshot
git diff --cached --check
git diff --cached --stat
git diff --cached

# 4. Commit
git commit -m "feat(uart): add DMA receive pipeline"

# 5. Push only when requested
git push -u origin <resolved-branch>
```

Before a push, inspect the upstream and remote:

```bash
git branch --show-current
git remote -v
git status --short --branch
```

Do not place access tokens in remote URLs or command history. If authentication is missing, stop at the authentication boundary and tell the user what connection is required.

## Final Report

After an executed commit, report:

- commit SHA and subject;
- files intentionally included;
- verification actually run and its result;
- whether the commit was pushed and to which branch;
- remaining unstaged or untracked changes.
