# TULPAR

Distributed UAV-USV collective for maritime missions, simulated in ns-3 and displayed in SIMDIS. Graduation project, Computer Engineering, Yaşar University (2026-2027).

- Requirements: [`docs/rsd/TULPAR_RSD_v1.1.md`](docs/rsd/TULPAR_RSD_v1.1.md)
- Work tracking: Jira project `TULPAR`; planned issues in [`docs/backlog/TULPAR_Jira_Backlog.json`](docs/backlog/TULPAR_Jira_Backlog.json)
- Rules for Claude Code: [`CLAUDE.md`](CLAUDE.md)

## Build

Requirements: Ubuntu 24.04 (or the image in `docker/`), GCC 13, CMake 3.28 or later, Ninja, Python 3, git.

```bash
scripts/bootstrap.sh     # clone ns-3.49, link module/ into contrib/tulpar, configure
scripts/build.sh         # build the tulpar library, the tulpar-g0 example and the test runner
scripts/g0_checks.sh     # gate G0 checks; logs, hashes and summary in out/g0/
```

Run the skeleton scenario by hand:

```bash
cd external/ns-3
./ns3 run "tulpar-g0 --seed=11 --run=1 --duration=60 --eventLog=/tmp/events.log"
```

## Structure

The agent of one vehicle consists of eight modules (RSD Section 2.3): M-COM, M-DEL, M-CRY, M-EVS, M-OBS, M-SEC, M-MIS and M-LSC. Each is a separate static build target under `module/agent/`, so a module can only use what its target links. `tools/check_static.py` checks the include and link graph against `tools/deps_rules.json`, which encodes RSD Table 4.25 and the invariants MD1, MD5, MD9 and MD19. The simulation harness (M-SIM) in `module/sim/` holds the scenario controller, the ground-truth store and the event log; no agent target links it.

## Phase 0 status

The skeleton runs two vehicles (UAV-1, USV-1) with all eight modules on an 802.11n ad hoc channel. Only M-COM has behavior: a placeholder neighbor beacon with seeded jitter, which makes the event log depend on the run's random streams. The other modules report that they started. The gate G0 checks cover RSD-SIM-01a, RSD-SIM-01c, RSD-SIM-01d, RSD-SIM-02a and the hidden-label part of RSD-SIM-02b; see `docs/g0/`.

## License

GPL-2.0-only, as ns-3 (see `LICENSE`).
