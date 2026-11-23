# Gate G0 report: simulation skeleton

Date: 23 November 2026. Gate G0 is due at the end of November 2026 (RSD v1.1 Section 5.4).
Produced by `scripts/g0_checks.sh`; the raw evidence is written to `out/g0/` and is not committed.

## Environment

| Item | Value |
|---|---|
| Simulator | ns-3.49 (tag `ns-3.49`, cloned by `scripts/bootstrap.sh`) |
| Compiler | GCC 13.3.0 (Ubuntu 24.04.5), C++23, CMake 3.28.3, Ninja |
| Build profile | `default`, examples and tests enabled |
| Machine | Claude Code cloud workspace, 2 cores, 7 GB RAM |
| Repository commit | none yet: the code is not committed; the checks ran on the working tree |
| Scenario | UAV-1 and USV-1, all eight agent modules, 802.11n ad hoc at HtMcs0, skeleton beacon with seeded jitter, 60 s |

## Results

| Check | Requirement | Result | Evidence |
|---|---|---|---|
| Static include, link and wall-clock checks; the self-test plants 10 violations, all detected, and the clean tree passes | RSD-SIM-02a, MD9, MD5, MD19, RSD-SIM-01a (static part) | PASS | `check_static.txt`, `test_check_static.txt` |
| ns-3 suite `tulpar-g0`: same seed in one process gives identical logs | RSD-SIM-01d | PASS | `test_cases.txt` |
| ns-3 suite `tulpar-g0`: run numbers 1 and 2 give different logs | RSD-SIM-01c (supporting) | PASS | `test_cases.txt` |
| ns-3 suite `tulpar-g0`: hidden attacker label changes no agent event | RSD-SIM-02b (hidden-label part) | PASS | `test_cases.txt` |
| Two separate processes, seed 11, run 1 | RSD-SIM-01d, gate I-07 (log part) | PASS | identical logs, SHA-256 `c0fe145d505646ec...` |
| Same run with both cores busy | RSD-SIM-01a (run part) | PASS | identical to the idle run |
| Two processes with attacker label `none` and `UAV-1` | RSD-SIM-02b (hidden-label part) | PASS | agent logs identical (`c0fe145d...`); ground-truth logs differ (`d6537b08...`, `54746edb...`) |

Each run produced 146 events (114 beacons sent, 14 received; trace hash `4b2524adf671d4b4`). The whole check set was repeated after a full rebuild of ns-3 and the module and gave the same SHA-256. Because a different run number changes the log, the identical logs under one seed show that the random streams, not their absence, are reproducible.

## Not covered at this point

- **RSD-SIM-02b, second part:** "disabling the attack changes them" needs an attack behavior; the skeleton has none. The requirement stays open until the first attack is implemented (Phase 5).
- **SIMDIS gates I-00 to I-02:** they need the SIMDIS SDK 1.25 build and the bridge (owner: simulation infrastructure). The architecture assigns them to G0.
- **Gate I-07, analysis part:** "identical analysis results" needs the analysis scripts, which do not exist yet; only the event logs were compared.
- **RSD-SIM-01b:** the micro-benchmark delay table is Phase 1 work (TBR-01).
- **CI and Docker:** `.github/workflows/ci.yml` and `docker/Dockerfile` have not run yet. The workspace has no Docker daemon and the repository is not on GitHub.

## Proposed verification status after G0 review

| Requirement | Status |
|---|---|
| RSD-SIM-01d | VERIFIED for the skeleton configuration, to be repeated at each gate |
| RSD-SIM-02a | VERIFIED |
| RSD-SIM-01a | VERIFIED for the skeleton (no cryptographic delays exist yet) |
| RSD-SIM-01c | TEST PLANNED (code inspection of key seeding waits for Phase 1 keys) |
| RSD-SIM-02b | TEST PLANNED (hidden-label part passes; attack part open) |

The team records these statuses after reviewing this report; the report itself changes no status.

## Reproduce

```bash
scripts/bootstrap.sh && scripts/build.sh && scripts/g0_checks.sh
```
