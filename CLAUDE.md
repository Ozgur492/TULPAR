# TULPAR: working rules for Claude Code

TULPAR is an ns-3 simulation of a distributed UAV-USV collective. Read this file before every task.

## Sources of truth

- **Requirements:** `docs/rsd/TULPAR_RSD_v1.1.md` (RSD v1.1). Requirement identifiers (`RSD-XXX-NNa`) are stable and never renumbered. Where an architecture document differs from the RSD, the RSD governs.
- **Work items:** Jira project `TULPAR` on `dasdas.atlassian.net`, through the Atlassian connector. Every code change belongs to one Jira issue.
- **Backlog map:** `docs/backlog/TULPAR_Jira_Backlog.json` lists every issue with its Jira key (`TULPAR-n`), RSD identifier, planned gate, owner labels and acceptance criterion.

## Repository layout

| Path | Content |
|---|---|
| `module/` | The ns-3 module `tulpar`; `scripts/bootstrap.sh` links it to `external/ns-3/contrib/tulpar` |
| `module/agent/<m>/` | Agent modules `com`, `del`, `cry`, `evs`, `obs`, `sec`, `mis`, `lsc` (one static target each) and `root` (wiring) |
| `module/common/` | Shared types and the write-only event sink |
| `module/sim/` | Simulation harness M-SIM: scenario controller, ground-truth store, event log |
| `tools/check_static.py` | Static checks; rules in `tools/deps_rules.json` |
| `scripts/` | `bootstrap.sh`, `build.sh`, `g0_checks.sh` |

## Commands

```bash
scripts/bootstrap.sh           # clone ns-3.49, link the module, configure
scripts/build.sh               # build tulpar, tulpar-g0 and the test runner
scripts/g0_checks.sh           # gate G0 checks; evidence in out/g0/
python3 tools/check_static.py  # static dependency and wall-clock checks
cd external/ns-3 && ./test.py --no-build -s tulpar-g0
```

## Jira workflow

1. **Pick work.** Use the issue the user names. Otherwise:
   `project = TULPAR AND labels = agent-claude AND statusCategory != Done ORDER BY fixVersion ASC, rank ASC`
2. **Before coding.** Read the issue and the RSD text of every requirement it names (search the RSD for the identifier). Move the issue to In Progress.
3. **Branch and commits.** Branch `TULPAR-<n>-<short-name>`; every commit message starts with the issue key, then an imperative summary (`TULPAR-123: Add beacon jitter`).
4. **Tests.** Each acceptance criterion becomes an automated check whose name contains the requirement identifier (for example the test case `RSD-SIM-01d same seed gives identical event logs`). Static (S) criteria go into `tools/check_static.py`; run-based criteria go into the ns-3 test suite or `scripts/`.
5. **Done.** An issue is done only when its acceptance check passes in CI. Comment on the issue with the commit hash, the check name and, for run-based checks, the run id and seed; then move it to Done. For criteria marked Simulation (M), the implementation issue closes when its deterministic checks pass; the statistical verdict is recorded on the experiment issue (E1-E15).
6. **Blocked or unclear.** Comment on the issue, state what is missing and stop. Do not invent a requirement, a threshold or a value. Work found during a task becomes a new issue linked to it.

## Evidence rules (from the RSD)

- Never mark a requirement verified without a run id, seed and commit. Never put a number into a results table that no run produced; placeholders stay visible.
- SIMDIS output is display only and never verification evidence. Mockups carry a watermark and live apart from run outputs.
- Thresholds of a statistical experiment are registered before its test runs; calibration runs are kept apart from test runs.

## Code rules (from the RSD)

- Agent modules are separate static targets. A module includes only its own headers, `common/` and M-CRY (I6); data between modules flows through the interfaces of RSD Table 4.25, wired in `agent/root`. The LSC includes and links nothing but `common/` (MD19). Security never reaches mission (MD5).
- No agent file includes the harness or the ground-truth store (RSD-SIM-02a, MD9); agents get only their identity, the write-only event sink and their stream number.
- Agent code reads no wall clock and uses no unseeded random source. Every random variable gets an explicit stream number from the harness; the run seed and run number select the MRG32k3a substreams (RSD-SIM-01a, RSD-SIM-01c).
- Event log lines are deterministic: integer nanoseconds, fields in fixed order, no pointers, no floating-point formatting.
- Toolchain: ns-3.49 module in C++23 under GPL-2.0-only, built with CMake and Ninja on Ubuntu 24.04 with GCC 13 (`docker/Dockerfile`). From Phase 1, cryptography goes through Botan 3.13.0: ML-KEM-1024, ML-DSA-87 in deterministic mode, AES-256-GCM with rule-based IVs, SHA-384, HMAC-SHA-384 and HKDF-SHA-384 (RSD-CRY-01). The viewer uses SIMDIS SDK 1.25; state machines are model-checked with UPPAAL 5.0.0.
- Processing delays are scheduled in simulation time from the micro-benchmark table, never measured during a run.
- Follow the ns-3 coding style (`.clang-format`, Doxygen with `@` tags, ASCII only in comments).
- Do not use the term "post-quantum" in code comments, documents or reports.
