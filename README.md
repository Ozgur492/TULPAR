# TULPAR

TULPAR is a distributed security and mission architecture for collectives of unmanned aerial vehicles (UAVs) and unmanned surface vehicles (USVs) that explore a maritime area together. It is built and evaluated in the ns-3 network simulator, and a viewer based on the SIMDIS SDK displays the computed results.

Graduation project, Department of Computer Engineering, Yaşar University, 2026–2027. Advisor: Prof. Dr. Ahmet Hasan Koltuksuz.

## The problem

The vehicles work in autonomous groups. They detect and classify surface contacts and keep a shared picture of what they have seen and how certain each conclusion is. Their links appear and disappear as they move, and islands and coastlines block links and lines of sight, so partitions and delayed delivery are a normal operating state rather than an exception.

The project addresses two problems:

- **Sound conclusions without a leader.** The collective must agree on what is on the sea surface while its network keeps splitting and merging.
- **A compromised member.** A compromised vehicle signs with its own valid key, so cryptography alone cannot show that it still behaves like a member.

TULPAR answers both with evidence that every agent evaluates in the same deterministic way, so agreement comes from converging evidence rather than from a central authority. Operators supervise the mission through designated gateways with a small, closed set of mission commands, and headquarters holds no security authority during the mission.

## Scope

- Mission configurations of up to 20 nodes in three groups. The configuration sets the vehicles and their sensor profiles, the groups, the operating area, the mission length and the gateways.
- A reference scenario demonstrates the architecture: a 30-minute exploration of the waters around Marmara Island by twelve UAVs and seven USVs in three groups. The architecture is not tied to this scenario.
- Simulation only: no real hardware or radios. Cryptography uses real algorithms through Botan.

The requirements are specified in the [Requirements Specification Document (RSD)](docs/rsd/TULPAR_RSD.md): baseline requirements decomposed into atomic requirements, each with a verification method and an acceptance criterion. The RSD is a living document; its copy on `main` is the only valid version.

## Architecture

Every vehicle, and the optional command ship, runs the same agent. The agent consists of eight modules:

| Module | Responsibility |
|---|---|
| M-COM Communication & Routing | Neighbor discovery, hop sessions, link-state records, Network Graph and path computation; ingress boundary and jamming indicator |
| M-DEL Reliable Delivery & DTN | Delivery protocol, receipts, hop acknowledgments, bundle store and retries |
| M-CRY Cryptographic Identity & Trust | Certificates, sessions, signing, bundle encryption and key epochs |
| M-EVS Evidence Store | Signed record chains, bundles, digest and pull, root classes and taint |
| M-OBS Observation & Fusion | Association, track initiation, statistical fusion, coverage and uncertainty |
| M-SEC Distributed Security | Evidence layers, evidence strengths, response selection and enforcement lists |
| M-MIS Mission & Cooperation | Mission state machine, task auction, connectivity repair, Verification Backlog, support contracts and operator commands |
| M-LSC Local Safety Controller | Safety-envelope enforcement; acceptance or rejection of every waypoint |

The simulation harness (M-SIM) supplies the channel, mobility, synthetic sensor outputs and injected attacks, keeps the ground truth and writes the event log. No agent can read the ground truth.

Each module is a separate static build target, so a module can use only what its target links. `tools/check_static.py` checks the include and link graph against RSD Table 4.25 and the architectural invariants: for example, security never reaches mission, the local safety controller links nothing but the shared types, and no agent code reaches the ground truth.

## Team

| Member | Area |
|---|---|
| Cemre Karabulut | Communication and delivery; jamming indicator, gateways and backhaul |
| Özgür Can Güngör | Cryptography and the evidence store |
| Berkan Mursal | Observation fusion and distributed security |
| Berra Senem Mıynat | Mission management and operator commands |
| Çağlar Mesci | Local safety controller; simulation infrastructure, SIMDIS viewer, operator console and post-mission analysis |

Folder ownership and the working rules are in `CLAUDE.md`.

## Roadmap

The work is built in stages, with a gate at the end of each month (RSD Section 5.4).

| Gate | Month | Content | Status |
|---|---|---|---|
| G0 | November 2026 | Simulation skeleton: module targets, seeded runs, event log and the G0 checks | In progress ([report](docs/g0/G0_report.md)) |
| G1 | December 2026 | Mesh and delivery | Planned |
| G2 | January 2027 | Evidence store; sessions and keys | Planned |
| G3 | February 2027 | Observation fusion, completing the minimum working system | Planned |
| G4 | March 2027 | Distributed security (Core tier) | Planned |
| G5 | April 2027 | Group autonomy (Target tier) | Planned |
| G6 | May 2027 | Experiments and visualization | Planned |
| Final report | June 2027 | | Planned |

## Getting started

Requirements: Ubuntu 24.04 (or the image in `docker/`), GCC 13, CMake 3.28 or later, Ninja, Python 3 and git.

```bash
scripts/bootstrap.sh     # clone ns-3.49, link module/ into contrib/tulpar, configure
scripts/build.sh         # build the tulpar library, the tulpar-g0 scenario and the test runner
scripts/g0_checks.sh     # gate G0 checks; logs, hashes and a summary in out/g0/
```

Run a scenario by hand:

```bash
cd external/ns-3
./ns3 run "tulpar-g0 --seed=11 --run=1 --duration=60 --eventLog=/tmp/events.log"
```

## Repository layout

| Path | Content |
|---|---|
| `module/` | The ns-3 module `tulpar` |
| `module/agent/<m>/` | One directory and static target per agent module; `root/` wires them together |
| `module/common/` | Shared types and the write-only event sink |
| `module/sim/` | Simulation harness: scenario controller, ground-truth store and event log |
| `module/examples/`, `module/test/` | Scenarios and the ns-3 test suites |
| `tools/` | Static dependency and wall-clock checks |
| `scripts/` | Bootstrap, build and gate checks |
| `docs/` | RSD, gate reports and the Jira backlog map |

## How we work

- **Jira.** All work is tracked in the Jira project `TULPAR`. Every issue names the RSD requirements it covers, and [`docs/backlog/TULPAR_Jira_Backlog.json`](docs/backlog/TULPAR_Jira_Backlog.json) maps the issues to requirements, gates and owners.
- **Branches.** One short-lived branch per Jira issue, created from `main` and named `TULPAR-<n>-<short-name>`. Every commit message starts with the issue key, for example `TULPAR-123: Add beacon jitter`.
- **Pull requests.** Changes reach `main` only through pull requests, and a pull request is merged only when CI passes: static checks, code style, build, ns-3 test suites and gate checks.
- **Evidence.** A requirement is marked verified only with a run id, a seed and a commit. Numbers that no run produced never appear in results.
- **Claude Code** follows the same rules; see [`CLAUDE.md`](CLAUDE.md).

## License

GPL-2.0-only, the license of ns-3 (see [`LICENSE`](LICENSE)).
