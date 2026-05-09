Use docs/backlog.json as the sole machine execution ledger for this session.

Goal:
Select the next eligible autonomous Airouter launch slice, implement it, run the full local equivalent of every `.github/workflows/ci.yml` gate that would apply to the branch, publish the branch with `docs/backlog.json` already set to `done` in the implementation commit set, open a detailed PR, and stop. After the PR is opened, do not make further edits, commits, or pushes to that branch unless a human explicitly instructs you to resume it.

Session rules:
1. Start by switching to the local main branch.
2. Pull the latest remote changes for main.
3. Fetch origin and verify local main is synchronized with origin/main before reading the queue:
   - `git fetch origin`
   - `git rev-parse HEAD`
   - `git rev-parse origin/main`
   - `git status --short --branch`
   - Continue only if `HEAD == origin/master` and the branch status does not report ahead, behind, or diverged.
4. If switching to main, pulling latest main, fetching origin, or the synchronization check fails, stop immediately and record the failure in docs/backlog.json last_run_summary for the active item only if an item was already selected; otherwise stop and report the repo-state failure.
5. Do not continue work from an already checked-out feature branch unless I explicitly told you to resume that same branch.

Pre-selection PR reconciliation rules:
1. Before selecting new implementation work, check the live GitHub state for the nearest prior auto-PR item for this repo that has branch, pr_number, or pr_url metadata. Do not rely on the item status field; completed work is marked done before PR creation, and the branch/PR gate is what prevents starting the next item too early.
2. Identify the prior PR candidate by reading compact queue metadata only:
   - first identify the normal next eligible item using the queue selector below
   - if a normal next eligible item exists, the prior candidate must have `order < selected.order`; choose the item with the greatest numeric `order` among only those earlier items
   - "prior" never means the largest order in the whole queue, and it never includes future, blocked, dependency-unsatisfied, or otherwise ineligible items after the selected item
   - branch, pr_number, or pr_url metadata on the selected item itself is not prior PR metadata; use it only under the execution rules for resuming `ci_failed` or `changes_requested` items
   - if there is no normal next eligible item, identify the greatest-order auto_pr item for repo "airouter" that has pr_number, pr_url, or branch metadata
   - intended jq shape after selecting an item:
     ```bash
     jq -r --argjson selected_order "$SELECTED_ORDER" '
       .items
       | map(select(
           .execution_mode == "auto_pr"
           and .repo == "airouter"
           and .order < $selected_order
           and ((.branch // null) != null or (.pr_number // null) != null or (.pr_url // null) != null)
         ))
       | sort_by(.order)
       | last
       | if . == null then empty else {id, order, status, branch, pr_number, pr_url, head_commit} end
     ' docs/backlog.json
     ```
3. Resolve the live PR from GitHub:
   - if pr_number is present, run `gh pr view <pr_number> --json number,state,mergedAt,headRefName,headRefOid,url,statusCheckRollup,reviewDecision`
   - otherwise, if branch is present, run `gh pr list --head <branch> --state all --json number,state,mergedAt,headRefName,headRefOid,url,statusCheckRollup,reviewDecision --limit 1`
4. If the prior PR exists and is open, do not select a new queue item. Report the open PR state from GitHub and stop without editing files.
5. If the prior PR is closed without merging, stop and report that the prior PR closed unmerged. Do not select new work until a human decides whether to reopen, replace, mark blocked, or otherwise reconcile that same item.
6. If the prior PR is merged, continue to normal queue selection. Do not make a merge-reconciliation metadata commit.
7. If the prior PR is merged but branch metadata points at the wrong PR or another ambiguous state, stop and report the inconsistency instead of selecting new work.
8. If the prior branch metadata exists but no matching PR can be found, stop and report a ledger inconsistency instead of selecting new work.

Queue selection rules:
1. Do not read or print the full queue file. Keep queue inspection token-efficient.
2. Do not reconcile or alter prior completed items unless I explicitly ask you to.
3. Select the lowest-order eligible item by running this command and reading only its compact output:
   ```bash
   jq -r '
     .items as $items
     | [ $items[]
         | select(
             (.status == "open" or .status == "ci_failed" or .status == "changes_requested")
             and .execution_mode == "auto_pr"
             and .repo == "airouter"
             and ((.depends_on // []) | all(. as $dep | any($items[]; .id == $dep and .status == "done")))
           )
       ]
     | sort_by(.order)
     | .[0]
     | if . == null then empty else
       {
         id,
         title,
         order,
         status,
         execution_mode,
         repo,
         depends_on,
         files,
         acceptance,
         verification,
         branch,
         pr_url,
         pr_number,
         head_commit,
         attempt_count
       }
       end
   ' docs/backlog.json
   ```
4. The selected item must satisfy:
   - status is one of "open", "ci_failed", or "changes_requested"
   - execution_mode == "auto_pr"
   - repo == "airouter"
   - every item listed in depends_on has status == "done"
5. If the selector prints no item, stop and explain that no eligible item exists. Do not enumerate every queue item.
6. Before starting work, print only the selected item id, status, order, branch, pr_url, and head_commit from the synchronized main checkout. If the item was previously completed in a merged PR but still appears open on synchronized main, stop and report the ledger inconsistency instead of reimplementing it.
7. Never select items with status:
   - "blocked"
   - "done"
8. Optional fast path for manual inspection: `rg -n '"status": "open"|"status": "ci_failed"|"status": "changes_requested"' docs/backlog.json | head -1` may be used to find the first candidate, then inspect only a small surrounding window. If that candidate is not eligible, continue with the jq selector above rather than reading the full file.

Execution rules:
1. After selecting the item, create a fresh branch from updated main.
   - If status is "ci_failed" or "changes_requested" and the item already has a branch, check out that existing branch after synchronizing main instead of creating a new branch.
   - When resuming a "ci_failed" or "changes_requested" item, transition it back to "in_progress" before making fixes.
2. Limit edits to the files listed on the queue item unless acceptance criteria cannot be met without a minimal adjacent edit. If scope expands, document exactly why in last_run_summary.
3. Implement only the selected queue item. Do not begin work on any other queue item in this session.
4. Satisfy the listed acceptance criteria and verification requirements for the selected item.
5. Run the full local equivalent of `.github/workflows/ci.yml` before PR submission.
   - Preferred command from the finalized branch state: `.github/scripts/run_ci_equivalent.sh origin/main HEAD`.
   - First reproduce the workflow planner against the branch diff from synchronized `origin/main`: identify the selected Go modules, whether frontend assets changed, whether `.github/scripts/**` changed, and whether review-sensitive paths changed using the same path rules as `plan-ci`.
   - For every selected Go module, run exactly the PR lint tooling from `ci.yml`: install `staticcheck@v0.6.0`, install `golangci-lint@v1.64.5`, run `gofmt -l` over all Go files in the selected module and fail if it prints any path, run `go vet ./...`, run `staticcheck ./...`, and run the workflow errcheck gate (`golangci-lint run ./internal/memory/...` for `gateway`, `golangci-lint run ./internal/...` for `cca`).
   - For every selected Go module, run the PR test gate: `go test ./...`.
   - For every selected Go module, also run the merge build gate that `ci.yml` runs after merge: `go build ./...`.
   - If `.github/scripts/**` changed, run `python3 .github/scripts/test_ai_adversarial_review.py`.
   - If `portal/internal/web/static/**` changed or the workflow planner marks frontend changed, run `node portal/internal/web/static/app_test.js`.
   - If review-sensitive paths changed, run all locally runnable review tooling above and note that the live AI adversarial review itself runs only on GitHub with repository secrets.
   - Do not summarize CI as "full", "equivalent", or "passed" unless every locally runnable gate selected by `ci.yml` has been run after the final code and queue edits.
6. If local CI fails, treat the failure as part of the same implementation task:
   - inspect the failing check output
   - fix the underlying issue when it is within the selected item scope or a minimal adjacent edit is required
   - rerun the failed check, then rerun the full CI-equivalent workflow once targeted failures are repaired
   - repeat this repair loop until CI passes or the failure is blocked by environment, missing credentials, external service availability, an ambiguous design decision, or scope that would exceed the selected queue item
7. If local CI is still failing after the repair loop is blocked or exhausted:
   - update docs/backlog.json for the selected item with:
     - status = "ci_failed"
     - branch
     - head_commit
     - updated_at
     - attempt_count += 1
     - last_run_summary including the remaining failure cause, checks run, fixes attempted, and why the runner stopped
   - commit the queue-file update if appropriate to preserve continuity
   - stop
8. If CI passes:
   - update docs/CHANGELOG.md with any relevant user-facing or operational changes before the final commit, push, and PR creation
   - update docs/backlog.json for the selected item before the final implementation commit is finalized with:
     - status = "done"
     - branch
     - started_at if newly set
     - updated_at
     - attempt_count += 1
     - last_run_summary summarizing problem, implementation, tests, any scope expansion, and the planned PR submission
     - leave pr_url and pr_number unchanged unless they were already present from an earlier human-directed resume
     - leave head_commit unchanged or null; the final commit SHA is available from Git and GitHub after the branch push
   - include the queue-file `done` edit in the same intentional implementation commit set before the first push; if implementation was already committed locally, amend or squash before pushing so there is no standalone "update launch queue item to done" commit
   - push the completed branch exactly once
   - open a detailed PR
   - do not edit, commit, or push any post-PR metadata update
   - report the PR URL in the final response and stop; do not start another queue item in this run

Post-PR reconciliation rules:
1. After an item has branch or PR metadata, the runner must not edit files, commit, push, or select another queue item until the prior PR is resolved. Resolve the live PR state from GitHub by pr_number when present, otherwise by branch.
2. If the PR is open, report the live state from GitHub and stop. Do not update docs/backlog.json for pending checks, failed checks, requested review changes, PR URL, PR number, or head SHA.
3. If the PR is closed without merging, stop and report the unmerged closure. Do not alter the ledger unless a human explicitly instructs the reconciliation.
4. If a human explicitly instructs the runner to fix failed checks or review feedback on the same PR, resume the existing branch and keep all changes in the next intentional implementation commit. Do not make status-only commits.
5. After merge is verified, stop reconciliation and continue normal queue selection. Do not commit post-merge metadata.

PR requirements:
The PR description must include:
- queue item ID and title
- problem being fixed
- files changed
- acceptance criteria checklist mapping
- tests run, including full ci.yml-equivalent run
- risks or follow-ups
- note that docs/backlog.json was updated to done before branch push / PR creation and will not receive post-PR status commits

Important constraints:
- Do not start the next queue item after opening the PR.
- Do not silently skip CI.
- For the selected item, "done" means the agent completed implementation, local verification, and pre-PR queue bookkeeping on the branch; the next session's branch/PR gate verifies whether the submitted PR is still open before selecting more work.
- After PR creation, no status-only edits, commits, or pushes are allowed. Same-branch fixes require explicit human instruction.
- After merge, do not push completion metadata to the old PR branch; update docs/backlog.json directly on synchronized main.
- Never create, push, or leave behind a commit whose only purpose is marking the queue item done after the implementation commit or after PR creation. Queue completion belongs with the implementation before the first branch push.
- If the selected item appears ambiguous, too large, or blocked by an unrecorded design decision, stop and explain instead of improvising a broader change.

At the end, report:
- selected queue item ID
- branch name
- CI result
- PR URL if created
- exact docs/backlog.json fields updated
