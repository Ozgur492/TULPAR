# TULPAR Requirements Specification Document

Living document, maintained in the project repository (Section 1.5)

# 1 Introduction

## 1.1 Project background and motivation

TULPAR is a distributed security and mission architecture for collectives of unmanned aerial vehicles (UAVs) and unmanned surface vehicles (USVs) that explore a maritime area together. The vehicles work in autonomous groups: they detect and classify surface contacts and keep a shared picture of what they have seen and how certain each conclusion is. Their links appear and disappear as they move, and islands and coastlines block links and lines of sight, so vehicles often lose contact with each other. Partitions, delayed delivery and group autonomy therefore run through the whole design.

The architecture is not tied to one scenario. A mission configuration sets the vehicles, their groups, the operating area and the mission length (Section 1.4). The project evaluates and demonstrates the architecture on a reference scenario: a 30-minute exploration of the waters around Marmara Island by twelve UAVs and seven USVs in three groups, which search three sectors in parallel. The island, about 17 km by 10 km, blocks every link that crosses it, so in this scenario partitions are a normal operating state rather than an exception.

Two problems motivate the project. First, the collective must reach sound conclusions about the sea surface without a leader and while the network keeps splitting and merging. Second, it must stay safe and useful when one of its own members is compromised. A compromised vehicle signs with its own valid key, so cryptography alone cannot show that a member is still behaving like one. TULPAR answers both problems with evidence that every agent evaluates in the same deterministic way, so that agreement comes from converging evidence rather than from a central authority (Chapter 2).

This document specifies what TULPAR shall do. It restates the team's approved system architecture, together with its Human-on-the-Loop (HOTL) and National SATCOM design, as numbered requirements. Chapters 1–3 describe the system, its technical context and its operation. Chapter 4 states the requirements with their verification methods and acceptance criteria, and Chapter 5 describes how they will be verified. Chapter 6 sets out the assumptions, guarantees and limitations on which they rest, together with the open values and the optional items, and Chapter 7 lists the references.

The document is self-contained. The architecture documents are internal working documents of the project team, so this document restates every obligation, assumption and limitation it relies on rather than citing their sections. It also includes the requirements that follow from the team's research on common-mode compromise and adaptive grouping. Where an architecture document and this document differ, this document governs.

## 1.2 System objectives

The objectives of TULPAR are the mission needs N-01 to N-16, N-R1R2, N-R3 and N-R4 of Table 1.1. The labels in the first column identify the needs for traceability. The stakeholders are the project team, the project advisor, the architecture reviewers and the evaluation committee. Within the simulated mission, the stakeholders are headquarters and the operators.

**Table 1.1. Mission needs and the requirements that address them.**

| Label | Need | Checked by | Addressed in |
|---|---|---|---|
| N-01 | Three autonomous groups; no guaranteed inter-group communication; fixed composition | E3, E4 | Section 4.1 |
| N-02 | Dynamic mesh; no permanent relays or gateway chains; alternative paths; delayed delivery; routing failure is not malice | E1, E2 | Section 4.4 |
| N-03 | Frames reach neighbors at the full range of the channel model | GT-08; E1 | Section 4.4 |
| N-04 | Heterogeneous capability profiles; near, medium and long range as reference capabilities | E5 | Section 1.4; Section 4.3 |
| N-05 | Evidence-based collective decisions without a leader: independent evidence, sensor fusion, uncertainty, coordination, inter-group verification, responses | E5–E9, E11 | Sections 4.1, 4.2, 4.3 and 4.6 |
| N-06 | No in-mission authority; proof-carrying quarantine and revocation | GT-05; E3, E12 | Sections 4.1, 4.5 and 4.6 |
| N-07 | Mission continuity under communication loss; storage and later delivery | E4 | Sections 4.1 and 4.4 |
| N-08 | Separation of communication, security, mission planning and local safety | Static checks | Sections 4.2, 4.7 and 4.9 |
| N-09 | Connectivity repair with interfaces to the Network Graph and the LSC, without central authority | E10 | Section 4.2 |
| N-10 | Verification Backlog distinct from the delivery queue; prioritization model with stated initial values | E11 | Section 4.2 |
| N-11 | Authenticated inter-group support requests over the mesh or delayed delivery; autonomous acceptance | E11 | Section 4.2 |
| N-12 | Temporary operational integration that keeps identity and membership | E11 | Section 4.1 |
| N-13 | Evidence reassessment with provenance; the result may remain uncertain | E11 | Sections 4.2 and 4.3 |
| N-14 | ns-3 / SIMDIS responsibility split; computed results only; scenarios, outcomes and acceptance tests; no claim of implementation | I-00 to I-10; GT-01 to GT-10 | Section 2.1; Sections 4.9 and 4.11 and RSD-SIM-01 |
| N-15 | Mockups never presented as outputs | Inspection | Section 4.9 |
| N-16 | Decisions with rationale; known limitations; future work; bibliography | Review | Chapters 2, 4, 6 and 7 |
| N-R1R2 | A fleet-wide software compromise is contained and vehicles stay inside their safety envelope | GT-11, GT-13; E13 | Sections 4.5, 4.6, 4.7 and 4.9 |
| N-R3 | Human-on-the-loop supervision without a central security authority (advisor) | GT-12, GT-14, GT-15; E14, E15 | Sections 4.1 and 4.8 and RSD-HOT-07 |
| N-R4 | Defined behavior and conditional recovery under jamming | E15 | Section 4.7 |

## 1.3 Project scope

In this project, TULPAR is built and evaluated in the ns-3 network simulator, and a viewer built on the SIMDIS SDK displays the computed results. Table 1.2 lists what is inside and outside the scope of the project. Every number in this document is a design value, a reference value, a size defined by a standard or a calculation. None is a measured result. No requirement has been implemented or verified yet.

**Table 1.2. System scope.**

| In scope | Out of scope |
|---|---|
| A collective of UAVs and USVs in autonomous groups with heterogeneous sensor profiles, three groups in this project; an optional command ship that runs the same agent as an ordinary node | Real hardware, real radios, real secure chips and real image processing |
| Mission configurations of up to 20 nodes, set by the mission plan: vehicles, sensor profiles, group membership, operating area, sectors, mission length and designated gateways | Fleets larger than 20 nodes; a variable number of groups; changes of group membership during the mission |
| Dynamic multi-hop mesh, reliable delivery, delay-tolerant bundles and connectivity repair | Design of the collision-avoidance algorithm itself, which is an interface the architecture must not weaken |
| Cryptographic identities, sessions, signed evidence and key epochs, using real algorithms through Botan | Physical capture of hardware, side channels and key extraction, which are tested only as a boundary profile |
| Evidence store, distributed evaluation, statistical fusion, state machines, graded responses and enforcement at honest receivers | Locating a jammer; anti-jam waveforms, which appear only as a processing gain |
| Mission state machine, task allocation, support contracts, Verification Backlog and guest participation | Identities that join during the mission; Sybil attacks are excluded by a fixed roster (AS3) |
| Operator supervision channel with bounded commands and status summaries; operation under jamming with degradation levels; containment of a fleet-wide compromise, with its trust boundaries modeled as logical boundaries | Real process isolation, hardware key storage, hardware-rooted attestation and real binary diversification |
| Supervision traffic to a headquarters ground control station through designated gateways, over abstract backhaul profiles | Physical satellite links, terminals and services; radio links beyond the horizon are represented only by backhaul profiles |
| ns-3 simulation, SIMDIS visualization and post-mission analysis by scripts | Language-model inference during or after the mission, which is future work |

## 1.4 General operational concept

The collective is organized in autonomous groups. Each group carries three independent sensor classes, EO-X, IR-Y and radar-Z: the minimum with which it can confirm a contact on its own evidence. Vehicles that carry the same sensor model form one root class, so a second vehicle of a class adds availability and geometry, not independence. A command ship may take part; it runs the same agent as every vehicle, belongs to no group and carries no sensor counted in fusion. Designated gateways carry supervision traffic between the groups and headquarters. Fig. 1.1 shows the reference configuration, with groups A, B and C and the designated gateways USV-1, USV-3 and USV-6.

![System context and operational architecture in the reference configuration with groups A, B and C. Headquarters acts before the mission only; supervision crosses untrusted backhauls to the designated gateways; the simulation harness supplies channel and sensors and displays computed events.](fig1_context.svg)

**Fig. 1.1. System context and operational architecture in the reference configuration with groups A, B and C. Headquarters acts before the mission only; supervision crosses untrusted backhauls to the designated gateways; the simulation harness supplies channel and sensors and displays computed events.**

Headquarters signs one provisioning package before the mission and has no security authority during it. During the mission, operators supervise through a channel with bounded mission-level authority. The system boundary encloses the agents of all vehicles and the command ship and the mesh between them. Table 1.3 lists the external entities, what crosses the boundary and the authority each holds during the mission. On the supervision channel, every component between a signer and a validator is untrusted transport.

**Table 1.3. External entities and interfaces.**

| External entity | What crosses the boundary | Authority during the mission |
|---|---|---|
| Headquarters (pre-mission) | One signed provisioning package per mission, loaded into every vehicle before the mission; load receipts returned to headquarters | Root of trust before the mission only |
| Headquarters GCS application | Copies of Group Status Summaries, alerts and mission reports upward; operator commands and GCS receipts downward | No security authority. It links no security or evidence-evaluation code and holds no vehicle or operator key |
| Operators | Signed command records from a closed set (ABORT, HOLD, RESUME, PRIORITY, SUPPORT), via the GCS or an optional command-ship console | Bounded mission-command authority; no command can change a security state, the roster, a key, the policy or a threshold |
| Designated gateways and backhaul | Supervision traffic only; a gateway injects into the mesh only operator command records and GCS receipts | None; gateways, terminals and backhauls are untrusted transport, and headquarters is not a vertex of any Network Graph |
| Scenario controller and ground truth (M-SIM) | Channel, mobility, synthetic sensor outputs, injected attacks and faults | None; ground truth never reaches an agent |
| SIMDIS viewer and bridge | Display events from agents; allow-listed scenario requests to the scenario controller only | None; the viewer derives no state and the bridge never reaches an agent |
| Post-mission analysis scripts | Read the event log after the run | None; nothing they produce can influence a mission decision |

### Mission configuration and reference scenario

The architecture fixes the mechanisms: the agent and its modules, the record and message types, the rules for evidence, fusion and responses, the state machines, the invariants and the roles of headquarters, operators, gateways and the command ship. A mission configuration sets everything else (Table 1.4). Three limits apply to this generality:

- The number of groups is three in this project; a variable group count is future work (Section 6.6).
- The design is sized for fleets of up to 20 nodes. The routing and the channel and resource budgets (Tables 4.29 and 4.30) are calculated for the 20-node reference fleet, and the mechanisms for larger fleets are future work.
- Every result of the project comes from simulation of the reference configuration and of the variations that the experiments define (Table 5.3). Other configurations are argued from the design, not shown.

**Table 1.4. Mission configuration and the reference configuration.**

| Element | Set by the mission configuration | Reference configuration |
|---|---|---|
| Vehicles | Number of UAVs and USVs and the sensor model each carries, up to 20 nodes in total | Twelve UAVs and seven USVs with EO-X, IR-Y and radar-Z profiles |
| Groups | Group membership, with all three root classes in every group (RSD-ORG-01b) | Groups A, B and C (Fig. 1.1) |
| Command ship | Present or absent | Optional; the 20-node budgets include it |
| Area and mission | Operating area and obstacles, sectors and mission length | The waters around Marmara Island; three sectors; 30 minutes |
| Supervision | Designated gateways and backhaul profiles | USV-1, USV-3 and USV-6 |
| Parameters | Thresholds, timers and weights | The initial values stated with the requirements |

## 1.5 Document conventions

**Sources.** The requirements restate the team's approved system architecture, its Human-on-the-Loop (HOTL) and National SATCOM design and the results of the team's research on common-mode compromise and adaptive grouping (Section 1.1). Requirements marked † come only from the HOTL and SATCOM design.

**Identifiers.** An identifier such as RSD-ORG-03a consists of the document prefix RSD, a three-letter area code (Table 1.5), the number of the requirement within its area and, for an atomic requirement, a letter. A baseline requirement that contains several obligations is decomposed into atomic requirements with letter suffixes. In the requirement tables, a shaded group row carries the baseline identifier and title, and the lettered rows below it are the atomic requirements. RSD-ORG-04 is a single obligation and has no group row. Identifiers are never renumbered or reused.

**Table 1.5. Requirement areas and their codes.**

| Code | Area | Specified in |
|---|---|---|
| ORG | System organization and distributed architecture | Section 4.1 |
| MIS | Mission management, task allocation and inter-group support | Section 4.2 |
| EVD | Evidence records, dissemination and deterministic evaluation | Section 4.3 |
| OBS | Observation fusion and observation statuses | Section 4.3 |
| NET | Communication, networking and connectivity repair | Sections 4.2 and 4.4 |
| CRY | Cryptography, keys and authentication | Section 4.5 |
| SEC | Security evidence, responses and enforcement | Sections 4.6 and 4.9 |
| CMC | Containment of a common-mode (fleet-wide) compromise | Sections 4.5 and 4.6 |
| SAF | Local safety and the Local Safety Controller | Sections 4.7 and 4.9 |
| JAM | Operation and degradation under jamming | Section 4.7 |
| HOT | Human-on-the-Loop supervision and National SATCOM | Sections 4.8 and 4.9 |
| SIM | Simulation, visualization and verification support | Sections 4.9–4.11 |

**Normative language.** "Shall" marks a binding requirement, and each atomic requirement contains exactly one "shall" obligation. Explanatory text, notes and tables of states are not requirements.

**Reference configuration.** A requirement that names the reference configuration or the reference mission plan binds only that configuration (Table 1.4). Every other requirement applies to any mission configuration within the limits of Section 1.4 and the assumptions of Chapter 6. Acceptance criteria are stated for the reference configuration because the tests and experiments run on it.

**Verification and acceptance.** Each verification entry starts with the method codes of Table 1.6 and then gives the acceptance criterion. Criteria name the gate tests (GT), experiments (E), SIMDIS integration gates (I-nn) and scenarios (SCN) they rely on, together with any quantity that "must be 0" in every run. Where a criterion depends on a value or case that is not yet fixed, it refers to an open value in Section 6.5.

**Table 1.6. Verification methods and the number of atomic requirements that use each.**

| Method | Code | Meaning in TULPAR | Evidence produced | Count |
|---|---|---|---|---|
| Inspection | S, I | Static check of module dependencies, build targets or interfaces (S); inspection of a configuration, mission plan or document (I) | Checklist record or static-analysis output | 79 |
| Test | T | Deterministic unit, property, replay, integration or gate test that must pass in every run | Test log with version, inputs and seed | 253 |
| Simulation | M | Statistical measurement in an ns-3 experiment with paired runs and confidence intervals, or a "must be 0" count over every run | Run configuration, seeds, raw output and analysis | 27 |
| Analysis | A | Reasoning, calculation or model checking, for example of the state machines in UPPAAL | Analysis report stating its assumptions | 10 |

A requirement may use more than one method (61 do). Demonstration is not used.

**Statistical verification.** Experiments use paired random-number streams, 30 repetitions per configuration unless a pilot run shows that more are needed, the Wilcoxon signed-rank test with Holm correction, and 95 % confidence intervals (Section 5.1). Hypotheses and thresholds are registered before the test runs. A hypothesis threshold is not a requirement unless a requirement states it. Parameter values are the initial values of the architecture, and only a pre-registered experiment may change them.

**Status.** Every requirement in Chapter 4 is a CORE baseline requirement. The 39 requirements marked † come only from the HOTL and SATCOM design and are counted in Table 5.4. Optional items and future work appear only in Section 6.6. No requirement is implemented or verified yet.

**Document maintenance.** This document is kept as a single file in the project repository and is changed through pull requests. The copy on the main branch is the only valid version; earlier states in the repository history and copies kept elsewhere are not sources. A copy sent outside the repository is identified by a version tag, and version 1.1 of 22 November 2026 carries the tag rsd-v1.1.

## 1.6 Terms and abbreviations

Table 1.7 defines the terms used in the requirements, and Table 1.8 lists the abbreviations.

**Table 1.7. Terms.**

| Term | Meaning |
|---|---|
| Agent | The TULPAR software on one vehicle or on the command ship: eight modules |
| Agent build | A compiled variant of the agent. All builds come from the same source and differ only in memory layout; no build spans two root classes (builds E, I and R in the reference configuration) |
| Collective | All vehicles of a mission together; they share one roster, one security policy and one evidence-merge rule |
| Group | One of the autonomous groups of the collective, three in this project (A, B and C in the reference configuration); membership is fixed in the signed roster for the whole mission |
| Component | A set of vehicles that can currently reach each other over the mesh. A group can split into several components, and a component can contain vehicles of several groups |
| Window | The 5 s interval of mission time in which evidence is evaluated (initial value) |
| Record | A signed statement of one agent, such as an observation, a link-state advertisement, a receipt, a contract, a finding or a proof; its identifier is the hash of its canonical content |
| Bundle | The set of new records an agent signs at the close of a window, under one ML-DSA-87 signature over a header, the records and their Merkle root |
| Digest | A per-window summary that every node broadcasts so that neighbors can find and pull missing records |
| Evidence Graph (EG) | The grow-only set of signed records held by an agent, with dependency edges; the only input to security decisions |
| Observation Graph (OG) | The deterministic function of the EG that holds the cell grid, contacts, fused log-likelihoods and observation statuses |
| Network Graph (NG) | An agent's local view of measured mesh links; it differs between vehicles and is never an input to a security decision |
| Sensor observation | The outcome of one sensor scan of a contact or cell: detected and classified (D_C), detected but not classified (D_U), not detected although the object was observable (N), or unobservable because of shadow, range, sea state or a faulty sensor (U) |
| Observation record | The signed record that carries a sensor observation into the EG |
| Root, root class | An independent source of evidence. Records that share an origin, an input or a common cause belong to one root class and count once |
| Sensor-channel state | The reliability state of one sensor channel of one vehicle: OK, Flagged or Limited; it decides the weight of that channel's observations |
| Observation status | The confidence state of a contact or type attribute: Reported, Corroborated, Confirmed, Contested, Refuted, Unknown or Retracted. It is derived, not assigned |
| Cross-validation | Support or contradiction of an observation by other root classes, computed by fusion; it is not a security proof |
| Evidence strength | The strength of security evidence against a vehicle: G0 to G3. Observation statuses and evidence strengths are separate scales |
| Security metric | One of the checks M1–M17 that turn records into evidence against a vehicle, each with a maximum evidence strength. Those named in this document are M4 authority violation (a record type that the key may never sign), M8 kinematic plausibility, M9 contract consistency, M15 link-claim consistency, M16 geographic leash (recorded only) and M17 path integrity |
| Proof (G3) | Self-proving evidence of misbehavior: an equivocation, a forbidden signed type, an invalid revocation record or a canonically malformed signed record |
| Exploit stage | How far a common-mode exploit has reached: stage I, a parser behind the ingress boundary; stage II, the agent, which can sign only through the key provider; stage III, the key itself |
| Security state | The state of a vehicle in the vehicle security state machine: Trusted, Watch, Restricted(scope), Probation or Quarantined |
| Producer | The member of a component that publishes epoch announcements, revocation records and Group Status Summaries: the Trusted member with the smallest rank number, or the Watch member with the smallest rank if no member is Trusted |
| Revocation record | A signed epoch announcement that excludes a quarantined vehicle from the new swarm key and embeds the proof against it |
| Provisioning package | Everything headquarters signs and loads into every vehicle before the mission |
| Headquarters (HQ) | The pre-mission root of trust. During the mission it hosts the primary operator console in its ground control station (GCS) and has no in-mission security authority |
| Designated gateway | A vehicle named in the signed mission plan that carries a backhaul to headquarters, optionally also the command ship (AS21); USV-1, USV-3 and USV-6 in the reference configuration |
| Backhaul | The supervision link between a designated gateway and headquarters, simulated as an abstract profile PT, P0 or P5 (AS22); untrusted transport (MD20) |
| Verification Backlog | A group's derived, ordered list of observations that need further independent verification |
| Support request, contract | Signed records of the inter-group support protocol; a contract binds one vehicle of the helping group to a region, duration and task |
| Guest participation | Work of a vehicle inside another group's region under a support contract, keeping its identity and membership |
| Connectivity repair | Moving an available vehicle under a time-bounded lease so that a usable multi-hop path appears |
| Evidence capacity regime | The per-sector, per-window classification S1, S2 or S3 of how many independent root classes can currently observe a sector |

**Table 1.8. Abbreviations.**

| Abbreviation | Meaning |
|---|---|
| ADR | Architecture Decision Record |
| AES-GCM | Advanced Encryption Standard in Galois/Counter Mode |
| AODV | Ad hoc On-Demand Distance Vector routing |
| AS | Assumption (AS1–AS22) |
| C1, C2 | Message classes for routine and important traffic |
| CBBA | Consensus-Based Bundle Algorithm (not used; future work) |
| CLM, LIM | Claim and limitation identifiers (Sections 6.2 and 6.3) |
| CNSA | Commercial National Security Algorithm Suite |
| DTN | Delay-tolerant networking |
| ECDH, ECDSA | Elliptic-curve Diffie–Hellman key agreement, elliptic-curve digital signature algorithm |
| EG, OG, NG | Evidence Graph, Observation Graph, Network Graph |
| EO, IR | Electro-optical, infrared |
| ETX | Expected transmission count |
| FIPA | Foundation for Intelligent Physical Agents |
| FSM | Finite-state machine |
| GCS | Ground control station |
| GNSS | Global navigation satellite system |
| GT | Gate test |
| HACMS | High-Assurance Cyber Military Systems (research program) |
| HKDF, HMAC | HMAC-based key derivation function, keyed-hash message authentication code |
| HOTL | Human-on-the-Loop |
| HPKE | Hybrid public key encryption |
| HQ | Headquarters |
| JDL | Joint Directors of Laboratories (data fusion model) |
| LSA | Link-state advertisement |
| LSC | Local Safety Controller |
| MANET | Mobile ad hoc network |
| MD | Architectural invariant (MD1–MD20) |
| ML-DSA, ML-KEM | Module-lattice digital signature and key-encapsulation algorithms |
| MLS | Messaging Layer Security |
| MPR | Multipoint relay |
| OLSR | Optimized Link State Routing |
| RATS | Remote attestation procedures |
| RSD | Requirements Specification Document |
| SATCOM | Satellite communication |
| SHA-384 | Secure hash algorithm with a 384-bit digest |
| TBD, TBR | To be determined; to be resolved |
| UAV, USV | Unmanned aerial vehicle, unmanned surface vehicle |
| UDP | User Datagram Protocol |

# 2 Existing Systems and Technical Context

TULPAR is a new system. It replaces no existing software and does not extend any deployed system. This chapter therefore describes the technologies on which it is built, the existing approaches it adopts, adapts or compares itself with, and the technical problems that shape its design.

## 2.1 Technologies and simulation environment

**Network simulation.** Every agent runs as an ns-3 application on its own node, and its only clock is simulation time [1]. The mesh uses 802.11n ad hoc [2] at a fixed rate as a stand-in for an S-band tactical link (AS6), so delivery and delay results are relative comparisons between configurations. Backhauls to headquarters are abstract profiles (PT, P0 and P5 of AS22), implemented as point-to-point links with configured data rate, delay, loss and outages. No result is a measurement of the Türksat service or of a satellite terminal.

**Cryptography.** From Phase 1, every cryptographic operation calls Botan 3.13.0 [3] and produces real ML-DSA-87 signatures, ML-KEM-1024 ciphertexts and AES-256-GCM tags. Processing delays come from a micro-benchmark table, not from the wall clock. The algorithm suite is the CNSA 2.0 set [4] (Table 2.1): ML-KEM-1024 [5], ML-DSA-87 [6], AES-256-GCM [7], and SHA-384 [8] with HMAC [9] and HKDF [10].

**Table 2.1. Algorithm suite.**

| Task | Algorithm | Sizes | Standard |
|---|---|---|---|
| Key establishment | ML-KEM-1024 | Encapsulation key 1568 B, ciphertext 1568 B, shared secret 32 B | FIPS 203 |
| Signatures | ML-DSA-87 | Public key 2592 B, signature 4627 B | FIPS 204 |
| Encryption and integrity | AES-256-GCM | Key 32 B, IV 96 bit, tag 16 B | NIST SP 800-38D |
| Hash, MAC and key derivation | SHA-384, HMAC-SHA-384, HKDF-SHA-384 | Digest 48 B | FIPS 180-4 |
| Comparison baseline only | ECDH and ECDSA on P-384 (CNSA 1.0) | Signature 96 B, public key 97 B | — |

**Visualization and analysis.** A viewer built on the SIMDIS SDK [11] displays computed events only and is never a source of verification evidence. The state machines are model-checked with UPPAAL [12], [13]. Post-mission analysis is done by scripts that read the event log. The system is one ns-3 module in C++23, built with CMake in a shared Docker image (Ubuntu 24.04, GCC 13). The eight agent modules and the simulation harness are separate build targets, so the build enforces the dependency rules.

ns-3 shows how the protocols and distributed algorithms behave under modeled loss, delay, partitions and topologies. It cannot show hardware timing, real radio or satellite performance, or physical safety. The key provider, the ingress boundary and the independence of the Local Safety Controller are logical boundaries inside one ns-3 process, not separate hardware.

## 2.2 Existing approaches relevant to the project

Surveys describe the security problems of UAV networks and flying ad hoc networks [14] and the development of autonomy and cooperation in unmanned marine vehicles [15]. The architecture relates to the approaches below. Each is adopted only in part, used as a baseline, or rejected for a stated reason:

- **Central mission authority.** A mission authority on a command ship or a ground station could decide revocation and re-keying for the whole collective, but every security decision would then depend on a node that a partition can cut off. TULPAR has no in-mission authority. Its structure corresponds to the zero-trust model [16]: each agent's evaluation function acts as the policy engine, and every honest receiver that refuses the traffic of a quarantined sender acts as an enforcement point.
- **Reputation-based trust management.** Trust management in mobile ad hoc networks commonly derives a trust value for each node from its observed behavior [17]. An on-off attacker can exploit such values by alternating good and bad behavior [18]. TULPAR therefore never quarantines on a score: quarantine needs a self-proving record (G3), and weaker evidence leads only to proportionate, reversible responses (Section 4.6).
- **Detection of misbehaving relays.** Silent dropping cannot be proven, because delay and loss look the same as omission [19]. Watchdog-style overhearing, in which a node checks whether its neighbor forwards what it was given, suffers from collisions, collusion, partial dropping and false accusation [20]. TULPAR therefore never turns a delivery failure into evidence (RSD-NET-03). Failures change only route preferences, so a vehicle that only drops is avoided, not accused.
- **Standard MANET routing.** The ns-3 implementations of AODV [21] and OLSRv1 [22] are comparison baselines for the TULPAR overlay in experiment E1. To reach long links they need a raised PHY slot, and that cost is part of their measured overhead. Security threats to link-state routing are documented for OLSRv2 [23], [24]. The standard integrity protection of OLSRv2 uses keys shared by all routers [25], so it cannot exclude a single vehicle and does not stop an insider who holds the key. In distance-vector routing, the path metric and hop count change at every hop, so a source signature cannot protect them, and a shared group key does not stop an insider [26]. TULPAR admits an edge only when the signed link-state records of both ends confirm it (RSD-NET-02).
- **Delay-tolerant networking.** Delay-tolerant networking addresses networks without continuous end-to-end paths [27], [28]. TULPAR's delay-tolerant delivery takes the bundle identifier, the lifetime and status reporting from the Bundle Protocol [29], but it reports status in signed batches rather than per bundle, routes no bundle through custodians and, unlike Spray-and-Wait [30], sprays no copies. Durability is derived from the evidence digests rather than from a receipt of its own, because a commitment to store data proves storage only when it is challenged [31]. Retransmission timing uses the estimator of [32]. Contact graph routing, which plans forwarding over a schedule of future contacts [33], and message ferrying, in which designated nodes move to carry data between disconnected parts of a network [34], are future work (Section 6.6).
- **Distributed detection with false sensor data.** When the share of sensors that report false data exceeds a critical level, a fused decision becomes uninformative [35]. TULPAR clips the contribution of each root class and assumes at most one attacker per observation scope (AS1), so that a single compromised vehicle cannot decide an observation status.
- **Task allocation.** Multi-robot task allocation problems are classified by whether robots do one or several tasks, whether tasks need one or several robots, and how tasks depend on each other [36], [37]. In TULPAR, the three root classes needed to confirm a contact come from the composition of each group (RSD-ORG-01b), so the allocator assigns tasks that one vehicle can do. Market-based methods can be decentralized and degrade gracefully when agents are lost, but the quality of their result depends on the bidding rule and on communication [38], [39]. CBBA combines an auction with a consensus phase [40]; its analysis assumes a connected network and synchronized rounds in which every agent broadcasts [41]. Asynchronous and partial-replanning variants reduce this communication but still need repeated exchanges [42], [43]. A collective that is often partitioned cannot provide them, so TULPAR uses a single-round sealed-bid auction with a commit step.
- **Connectivity maintenance.** Connectivity can be kept continuously by motion control [44]. TULPAR instead repairs connectivity only when pending delivery value justifies a time-bounded repair task (RSD-NET-06).
- **Fleet-wide compromise.** Many devices that run the same software can be taken over through a few shared weaknesses, as the Mirai botnet showed [45], and state-sponsored actors have been documented pre-positioning themselves in critical-infrastructure networks for later disruptive attacks [46]. An attacker can move between systems with legitimate credentials and native tools [47], so authenticating every input stops outsiders but not an implanted insider. TULPAR answers a fleet-wide compromise by containment rather than detection (Table 3.3), in line with cyber-resiliency engineering, which aims to keep the mission going while components are compromised [48].
- **Software diversity.** Diversifying builds makes exploits that depend on memory layout harder but gives no protection against logic flaws [49]. N-variant execution runs diversified variants side by side under a monitor and can detect some exploit classes without secrets [50], but it needs parallel execution and a monitor, which neither a single mission computer nor one ns-3 process provides. TULPAR therefore uses diversified builds as one barrier among several. Following the observation that replicas should be chosen across the attributes that correlate their failures rather than simply counted [51], it assigns one build to each root class (RSD-CMC-05).
- **Separation of safety functions.** In the Simplex approach, a simple and verifiable controller keeps a system safe when a complex controller fails [52]. In the HACMS program, a red team could not break into a hardened unmanned air vehicle during a six-week effort [53]. TULPAR's Local Safety Controller is the simple controller in this sense: it parses no record (RSD-SAF-02) and has the last word on every waypoint. In the simulation this separation is a logical boundary.
- **Classical public-key cryptography.** ECDH and ECDSA on P-384 serve only as the comparison baseline of experiment E1.
- **Remote attestation.** Session admission by boot quote follows the roles of the RATS architecture [54]. The vehicle is the Attester, headquarters provides the reference values through the provisioning package, and each peer is the Verifier.
- **Known impossibility results.** Consistency and availability cannot both be guaranteed under partition [55], so key epochs are per component. Wormholes cannot be detected without an independent position root: packet leashes, for example, need trusted positions or tightly synchronized clocks [56].

## 2.3 Technical problems and constraints

**Radio and channel.** In ns-3, a unicast frame's acknowledgment timeout has no propagation term. Unicast therefore fails beyond about 1.35 km, while the sea channel supports links of up to 32 km. Every overlay frame is consequently a MAC broadcast, and the overlay adds reliability where it is needed. One ML-DSA-87 signature (4627 bytes) does not fit the 2296-byte Wi-Fi MTU, so the overlay fragments messages itself. The radio's data rate, not its range, sets the evidence budget.

**Insider compromise.** A compromised vehicle behaves like an arbitrary (Byzantine) participant in the classical sense [57], within the limits below. It holds its valid keys through a signing oracle and controls its own software. It can:

- coordinate with other compromised vehicles;
- sign arbitrary content of the types its key provider allows;
- show different records to different neighbors;
- drop, delay and replay messages;
- lie about its position, links, sensor results and energy.

It cannot produce another vehicle's signature, break the primitives, be in two places at once or read the simulation's ground truth. External actors without keys can transmit, jam and replay. Sybil identities [58] are excluded by the fixed roster (AS3).

The design target is at most one compromised vehicle in each observation scope (AS1, f = 1), and every security experiment includes a two-colluder variant. Table 2.2 lists the three attacker profiles. The key-extracted and common-mode profiles are boundary profiles: they are injected in specific experiments to show where the guarantees stop.

**Table 2.2. Attacker profiles.**

| Profile | What the attacker holds | What crosses to other vehicles | Design answer |
|---|---|---|---|
| Oracle-constrained (normal threat model) | Control of the software, not of the key | Only messages its key provider allows | Evidence-based detection and graded responses |
| Key-extracted (boundary profile) | The key, outside the provider | A key; a forbidden-type signature becomes possible and makes the broken assumption visible | M4 proof; injected in E12 |
| Common-mode (boundary profile) | An exploit for a flaw in the shared agent software, reaching the agent through a network input | An exploit inside a network input; the budget of one attacker per scope no longer holds | Containment, not detection |

**Assumptions and constraints.** The requirements rest on assumptions that the design cannot prove; each is either stressed in an experiment or kept out of scope. Section 6.1 lists all 22 assumptions (AS1–AS22) with the consequence if each one breaks and how it is stressed; Table 2.3 lists the constraints that shape the requirements.

**Table 2.3. Constraints.**

| Constraint | Consequence for the requirements |
|---|---|
| The project builds and evaluates the system in simulation; no real vehicle, radio, sensor or satellite service is used | Verification uses test, simulation, analysis and inspection; no requirement claims hardware or physical validation |
| The network is simulated in ns-3 with 802.11n ad hoc at a fixed rate as a stand-in for an S-band tactical link | Delivery and delay results are relative comparisons between configurations |
| Real cryptographic algorithms (ML-DSA-87, ML-KEM-1024, AES-256-GCM, SHA-384) through Botan from Phase 1; processing delays from a micro-benchmark table | Cryptographic requirements are specified in Section 4.5; simulated time never depends on the host machine |
| The architecture adds no module beyond the eight agent modules and no in-mission authority; every security claim names its assumptions; the simulation harness is not part of the vehicle software | No requirement may introduce an in-mission authority, a new module or an unconditional security claim |
| No in-mission security authority; headquarters acts before the mission only, apart from bounded supervision | RSD-ORG-03 and RSD-ORG-05 on authority |
| Fixed roster; no identity joins during the mission | Group membership is fixed; RSD-ORG-02 |
| National SATCOM reference: Türksat 5B; Türksat 6A only if service availability and terminal compatibility are verified; satellite services operated outside Türkiye are outside the reference scope; backhauls are abstract profiles | No requirement may assume a satellite performance figure beyond AS22 |
| The A/B/C grouping of Fig. 1.1 is the reference configuration | Tests and experiments use the reference fleet of 19 vehicles and the command ship; a requirement binds that configuration alone only when it names it (Section 1.5) |
| Task allocation uses the single-round sealed-bid auction with a commit step | CBBA is future work; RSD-MIS-01 |
| Initial parameter values may be changed only by a pre-registered experiment | Requirements quote initial values (Chapter 4) |
| Evaluation functions are deterministic: no wall-clock time, reception time, randomness, unordered iteration or variable rounding | RSD-EVD-03 |
| Schedule: eight phases from November 2026 to May 2027 and the final report in June 2027; Core tier at gate G4 (March 2027), Target tier at gate G5 (April 2027) | Scope is cut in tier order if time runs short (Section 5.4) |

## 2.4 Motivation for the proposed distributed architecture

A collective that is often partitioned cannot depend on any particular node for a decision. TULPAR replaces central authority with three elements:

- the provisioning package that headquarters signs before the mission;
- self-proving evidence (G3) that any agent can check;
- deterministic derivation, by which agents that hold the same evidence reach the same states.

The groups share only what is fixed before the mission: one roster, one security policy identified by its hash, and one evidence-merge rule in which evidence is a grow-only set merged by union, the simplest form of a conflict-free replicated data type [59]. Agreement comes from converging evidence rather than from voting or a leader. During a partition every component keeps deciding, and states may differ until the components merge.

Two separations follow from the threat model. First, a signed observation is evidence to be weighed, not a fact, because a valid signature shows who said something, not that it is true. Second, routing and accountability are kept apart. Silent dropping cannot be proven, and loss cannot be told apart from omission, so delivery failure never becomes evidence. This is what makes dynamic routing acceptable: no security decision depends on who should have forwarded a message.

# 3 Operational Activities of the Proposed System

This chapter describes how the collective operates. The statements here explain the architecture; the binding obligations are the requirements of Chapter 4, and Chapter 5 describes how they are verified.

## 3.1 UAV–USV collective organization

A group is fixed in the signed roster, while a component is the set of vehicles that can currently reach each other. A group can split into two components behind an island, and a guest from another group can join a component for the length of its contract. Security evaluation and key epochs are organized per component, and task allocation and the Verification Backlog per group. Losing contact is a normal state, not a reason to end the mission.

Every vehicle and the command ship run the same agent of eight modules (Fig. 3.1). A ninth module, M-SIM, belongs to the simulation harness. A module either records facts or evaluates them, never both. The Evidence Store, Observation & Fusion and Distributed Security form the deterministic core: the other modules give the core only signed records and receive only states and needs. Three graphs have different consistency properties:

- The **Evidence Graph** is the only input to security decisions. It converges in every agent that receives the same records.
- The **Observation Graph** and the security states are deterministic functions of the Evidence Graph.
- The **Network Graph** is each vehicle's own view of measured links. It is never used for a security decision.

**Table 3.1. Agent modules.**

| Module | Responsibility | Cannot |
|---|---|---|
| M-COM Communication & Routing | Neighbor discovery, hop sessions, overlay framing, link-state records, Network Graph, path computation, connectivity analysis; ingress boundary for network input and jamming indicator | Evaluate link-claim metrics; write a security state; move a vehicle |
| M-DEL Reliable Delivery & DTN | Delivery protocol, receipts, hop acknowledgments, bundle store, retry ladder | Write delivery quality to the Evidence Store; attribute path corruption |
| M-CRY Cryptographic Identity & Trust | Certificates, sessions, signing oracle, bundle encryption, key epochs; schema check before signing and attestation quotes | Export a private key; derive the Trusted list itself |
| M-EVS Evidence Store | Signed record chains, bundles, digest and pull, root classes, taint | Accept unsigned data; evaluate |
| M-OBS Observation & Fusion | Association, track initiation, statistical fusion, coverage, uncertainty records | Change a vehicle or channel state; re-broadcast derived results |
| M-SEC Distributed Security | Evidence layers K0–K4, metrics, strengths G0–G3, response selection, enforcement lists | Command motion; see delivery quality |
| M-MIS Mission & Cooperation | Mission state machine, task auction, connectivity repair, Verification Backlog, support contracts, guest participation; operator command handling and Group Status Summary | Change policy; override local safety; command another group's vehicles |
| M-LSC Local Safety Controller | Safety-envelope enforcement and liveness protection; acceptance or rejection of every waypoint | Read ground truth; be disabled by security or mission; parse records, bundles or operator commands |

![Agent module architecture and internal interfaces I1–I23. Arrows show permitted flows; the forbidden flows are part of the requirements. ](fig2_modules.svg)

**Fig. 3.1. Agent module architecture and internal interfaces I1–I23. Arrows show permitted flows; the forbidden flows are part of the requirements. **

## 3.2 Mission workflow

**Before the mission.** Headquarters signs one provisioning package per mission and loads it into every vehicle and the command ship. Each vehicle returns a signed load receipt, and the mission starts only when every vehicle reports the same mission manifest hash and its own expected device envelope hash. After that, headquarters holds no security authority.

**During the mission.** Time is divided into 5 s windows. At the close of each window, every agent signs its new records as one bundle, with a single signature over a header, the records and their Merkle root [60], and evaluates the frozen evidence set of the window in a fixed order. Each group plans its own work: it allocates tasks among its members, decides whether to help another group, decides whether to repair connectivity or wait, and keeps working when it is cut off. No group can change the collective policy, which covers security thresholds, evidence and independence rules, fusion parameters, quotas, contract limits, cryptographic rules and the constraints of the Local Safety Controller.

Each vehicle runs three state machines (Fig. 3.2). The mission state machine is owned by Mission & Cooperation. The vehicle security state machine applies the responses of the security evaluation. The connection state machine follows the Network Graph directly. Entering the Alone connection state never by itself sends a vehicle back. Return is entered only through the mission state machine, under the conditions shown in Fig. 3.2(b), which include an applied operator ABORT.

![Vehicle security, mission and connection state machines. Quarantine needs a G3 proof; Alone never triggers Return; rally and the LSC return are not mission transitions.](fig3_states.svg)

**Fig. 3.2. Vehicle security, mission and connection state machines. Quarantine needs a G3 proof; Alone never triggers Return; rally and the LSC return are not mission transitions.**

**After the mission.** Post-mission analysis is done by scripts that read the event log and cite the record identifiers on which each line rests. A report written by a language model would not be bit-reproducible, so none is used.

## 3.3 Communication and distributed coordination

The communication layer moves data between vehicles whose links appear and disappear as they move and as islands and coastlines block line of sight. It uses no permanent relays or mandatory gateway chains. The mesh is an application-layer overlay over UDP in which every frame is a MAC broadcast. Links enter a Network Graph only when both ends sign them, so that an insider cannot forge topology claims. The quality of each link is measured as its expected transmission count (ETX), the inverse of the probability that one transmission and its acknowledgment both succeed [61]. Messages are handled by class (Table 3.2). A message for an unreachable destination waits as a bundle at its source until the destination appears or its lifetime ends.

**Table 3.2. Message classes.**

| Class | Examples | Delivery | Routing |
|---|---|---|---|
| C0 | Telemetry (position, velocity, energy) at 1 Hz | One-hop broadcast; unacknowledged; newest value wins | One hop |
| C1 | Mission data for one destination, reports to headquarters | Hop acknowledgment; cumulative and selective end-to-end acknowledgment; retries | Source-routed by ETX; bundle if unreachable |
| C2 | Support requests, offers, contracts, results, repair leases | Hop acknowledgment; per-message end-to-end acknowledgment; both paths after the second failure | Source-routed primary and node-disjoint backup by −log p; bundle if unreachable |
| C3 | Evidence records and proofs | Signed bundle broadcast once per window; digest and pull; proofs first | Anti-entropy; Section 4.3 |

Coordination does not depend on a coordinator. Evidence spreads by a single broadcast per window, followed by digest-and-pull repair. Agreement without a leader rests on two properties: the evidence set is a grow-only set merged by union, and the evaluation functions are pure. During a partition every component keeps deciding. After contact, proofs and revocation records are merged first, and states re-converge from the merged evidence (Fig. 3.3). Proofs travel ahead of all other traffic and are never held back by the evidence-plane cap of 10 % of channel time.

![Mesh communication, network partitions and recovery. Each component keeps deciding during a partition; after contact, proofs and revocation records are merged first and states re-converge from the merged evidence.](fig5_partition.svg)

**Fig. 3.3. Mesh communication, network partitions and recovery. Each component keeps deciding during a partition; after contact, proofs and revocation records are merged first and states re-converge from the merged evidence.**

## 3.4 Observation, detection and evidence fusion

Fusion answers two questions about the world: does an object exist at a given position, and which class is it? These are the questions of level 1 of the JDL data fusion model [62]. Fusion keeps uncertainty explicit (MD6) and is a deterministic function of the evidence (MD8).

Several concepts are kept apart (Fig. 3.4):

- A **sensor observation** is the outcome of one scan. An **observation record** is the signed statement that carries it, and a signed observation is evidence to be weighed, not a fact.
- The **sensor-channel state** (OK, Flagged, Limited) sets the weight of a channel's observations.
- A **root class** groups evidence that shares an origin or a common cause. Independence is counted in root classes, not vehicles, because a common-cause failure limits the benefit of redundancy [63].
- The **observation status**, from Reported to Retracted, is the resulting confidence in a contact. It is recomputed from the evidence at every window close by fixed precedence rules.

None of these is a security proof. Content findings act on observations and channels and never change a vehicle's security state.

![Evidence processing, observation fusion and distributed security decisions. Every agent evaluates the frozen evidence set of each window in the same fixed order; only a G3 proof leads to quarantine.](fig4_evidence.svg)

**Fig. 3.4. Evidence processing, observation fusion and distributed security decisions. Every agent evaluates the frozen evidence set of each window in the same fixed order; only a G3 proof leads to quarantine.**

Observations inside one root class are not independent and are not summed. Each class contributes only its most informative observation, and a class that contradicts itself contributes nothing. The contribution is also clipped, which bounds the influence of any one class; probabilistic 3D mapping likewise clamps the log-odds of each cell between fixed bounds, so that no cell becomes too certain to change [64]. The thresholds ensure that no single class can raise a contact above Reported, and that no two faulty or compromised classes can produce Confirmed. Whether a group can confirm a contact at all depends on how many independent classes can currently see the sector. The evidence capacity regime turns this into a per-sector planning test. A group that cannot settle an observation with its own sensors places it on its Verification Backlog and may ask another group for help. A contact becomes Unknown only after verification has been tried within its budget.

## 3.5 Security evaluation and autonomous responses

Security evidence is graded G0 to G3. Only self-proving evidence is G3. That covers an equivocation, a record type the key may never sign, an invalid revocation record and a canonically malformed signed record. All other evidence is weighed by independent roots and can at most restrict.

Two principles govern what the collective does with security evidence:

- **Nobody decides for others.** Each agent decides what it accepts. A restriction means that honest receivers stop accepting some of a vehicle's messages, so a compromised agent that ignores its own restriction gains nothing.
- **Least intrusive sufficient response.** The response is the lightest one that is sufficient against the observed harm and acceptable under every hypothesis still open. The responses range from L1, the lightest restriction, to L4, quarantine.

Only a proof can quarantine. A quarantined vehicle is excluded from the next swarm key by a revocation record that embeds the proof, so any agent can check it.

The common-mode attacker profile is answered by containment rather than detection. Every vehicle runs the same source code, so a remote exploit could compromise many vehicles at once and then wait without causing any anomaly. Such an exploit can reach three stages. At stage I it controls a parser behind the ingress boundary and can only drop, delay or reorder its own vehicle's inputs. At stage II it controls the agent, which signs only what the key provider allows. At stage III it has broken the key provider and holds the key. Table 3.3 lists the barriers. Each is a stated assumption about a real vehicle and a logical boundary in the simulation. None is claimed to stop a logic flaw shared by every build.

**Table 3.3. Barriers against a fleet-wide compromise.**

| Stage | Barrier | What it gives | Rests on | Requirement |
|---|---|---|---|---|
| Entry | Ingress boundary without keys, actuators or a transmit interface, with signatures verified by M-CRY on its output; key provider signs only schema-valid records | An exploited parser can only drop, delay or reorder its own vehicle's inputs; breaking the boundary yields the agent, not the key; a malformed record that does get signed proves key misuse | AS2, AS19 | RSD-CRY-02, RSD-CMC-01 and RSD-CMC-02 |
| Spread | One build per root class from the same source (E, I and R in the reference configuration) | A layout-dependent exploit stays within one class unless px allows; a logic flaw reaches every build | AS12 | RSD-CMC-05 and RSD-CMC-06 |
| Persist | Session admission by boot quote | A vehicle whose stored image was changed is refused after its next boot | AS2 | Section 4.5 |
| Dwell | None during the mission | A memory-resident implant lives for the whole mission | — | — |
| Trigger | LSC envelope enforcement and liveness protection | Out-of-envelope waypoints are rejected; a silent mission computer leads to a hold within 5 s | AS20 | Sections 4.7 and 4.9 |

## 3.6 Mission management and safety

Mission functions receive needs from the evidence core, never commands. The mission functions are:

- **Task allocation.** Tasks are allocated within a connected component by a single-round sealed-bid auction over signed records. A commit window prevents duplicate winners for exclusive tasks, repair leases and support contracts.
- **Inter-group support.** A group may ask another group for help but cannot command its vehicles: the receiving group treats the request as an external task in its own auction.
- **Connectivity repair.** When valuable traffic is waiting for an unreachable destination, the planner may move a vehicle under a time-bounded lease to a position from which measured links close the gap. Waiting is always an alternative.

Isolation must never remove safety. The security layer never commands motion, and a restricted or quarantined vehicle still feeds everyone's collision avoidance with its position. The Local Safety Controller (LSC) has the last word on every waypoint and has two responsibilities. Envelope enforcement checks every waypoint against separation, geofence and no-go zones, energy reserve, kinematic and sea-state limits. Liveness protection holds the vehicle after 5 s of mission-computer silence and returns it after 120 s or after a third unplanned silence.

For jamming, the most likely cause of link loss at sea, the architecture defines degradation levels J0 to J4, each with what still works. A vehicle that stays Alone with the jamming indicator raised for 300 s finishes its task and goes to its group's rally point. A jam never becomes an accusation. "Zero collisions" is a criterion of the simulated runs, not a guarantee of physical safety (AS20).

## 3.7 Human-on-the-Loop supervision and SATCOM

The collective works without a human in the loop, and nothing in it waits for one. Where a link exists, an operator can see how old each vehicle's information is and can issue the commands of Table 3.4. Mission supervision and security authority are separate powers: no command can change a security state, the roster, a key, the policy or a threshold. The primary console is at the headquarters ground control station (GCS), and the command ship is optional.

Supervision traffic reaches headquarters through the designated gateways, which are USV-1, USV-3 and USV-6 in the reference configuration (Fig. 3.5). Status flows up as sealed copies, one to each reachable gateway. A held copy is released only by a verified GCS receipt. Commands flow down, and every vehicle validates them itself, whatever path delivered them. Cryptography detects modification, forgery and replay on this path. It cannot prevent dropping or delaying, which the design makes visible through per-vehicle age at the console: each vehicle is shown as Live, Delayed (more than two summary periods old) or Lost (more than twenty).

![Human-on-the-Loop supervision through designated gateways and national SATCOM. Status flows up as sealed copies released only by verified GCS receipts; commands flow down and every vehicle validates and applies them itself.](fig6_hotl.svg)

**Fig. 3.5. Human-on-the-Loop supervision through designated gateways and national SATCOM. Status flows up as sealed copies released only by verified GCS receipts; commands flow down and every vehicle validates and applies them itself.**

**Table 3.4. Operator commands.**

| Command | Target | Signatures | Effect when applied |
|---|---|---|---|
| ABORT | Vehicle, group or collective | 2 | Mission state machine → Return; the LSC checks every waypoint. Two forms: an immediate abort carries a validity (default 120 s) and is void if it arrives later; a recall carries no validity and applies whenever it arrives within the same mission. Absorbing once applied |
| HOLD | Group or sector | 2 | No new exploration task in the target for its duration (default 600 s, at most 1800 s); current tasks finish; repairs and support continue; the mission state does not change |
| RESUME | Group or sector | 2 | A HOLD with duration zero; requires a precondition and a validity (default 120 s) |
| PRIORITY | Contact or sector | 1 | Sets the importance class within the policy range; the latest-issued record per target applies |
| SUPPORT | Requesting and helping group | 1 | Creates a support request on the operator's behalf with the highest importance class; the helping group still decides; RSD-MIS-02 |

## 3.8 Simulation and visualization workflow

A scenario controller (M-SIM) supplies the channel, mobility, synthetic sensor outputs and injected attacks. Ground truth is kept in a store that no agent can read, and the event log records every decision with its evidence identifiers, run identifier and seed. The viewer receives display events through a bridge that carries no evidence, and it derives no state.

Chapter 5 describes how the requirements are verified: the verification layers, the architectural invariants checked in every run, the gate tests, the experiments, the statistical method and the development phases.

# 4 Requirements

This chapter states the requirements of TULPAR in the form recommended by ISO/IEC/IEEE 29148 [65]: each states what the system must achieve and can be checked by test, inspection, simulation or analysis. The 59 baseline requirements are grouped under eleven headings and decomposed into 308 atomic requirements, each with one "shall" obligation, a verification method and an acceptance criterion (Section 1.5). Sections 4.1–4.9 contain the functional requirements and their interface constraints, Section 4.10 the performance and non-functional requirements, and Section 4.11 the requirements on simulation and verification. Each section begins with the engineering intent of its requirements; Chapter 3 describes the operation they constrain. The notes after the tables give the design rationale and point to the limitations of Section 6.3.

All 308 atomic requirements are CORE baseline requirements. The 39 requirements marked † come only from the HOTL and SATCOM design. No requirement is implemented or verified yet.

## 4.1 System Organization and Architecture

**Table 4.1. System organization requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-ORG-01 | @G Group composition (a–d). |  |
| RSD-ORG-01a | The reference mission plan shall assign UAV-1 to UAV-12 and USV-1 to USV-7 to groups A, B and C as listed in Fig. 1.1. | **I.** The roster and mission plan of the reference configuration assign group A = {UAV-1 to UAV-4, USV-1, USV-2}, group B = {UAV-5 to UAV-8, USV-3 to USV-5} and group C = {UAV-9 to UAV-12, USV-6, USV-7}. Any other assignment fails. |
| RSD-ORG-01b | Each group of a mission plan shall include vehicles of all three root classes EO-X, IR-Y and radar-Z. | **I.** For each group, the sensor-profile fields of the roster list at least one vehicle with EO-X, one with IR-Y and one with radar-Z, and the three profiles have three different modalities. |
| RSD-ORG-01c | The command ship shall belong to no group. | **I.** The command ship's roster entry has no group membership. |
| RSD-ORG-01d | The command ship shall carry no sensor that is counted in fusion. | **I, T.** The command ship's roster entry has no sensor profile that fusion counts. Unit test: the observer and class counts of the evidence capacity regime exclude the command ship. |
| RSD-ORG-02 | @G Fixed membership and guest participation (a–b). |  |
| RSD-ORG-02a | The group membership of every vehicle shall remain as recorded in the signed roster for the whole mission. | **T.** Run-time check against the event log of every run: in every agent's view and every window, each vehicle's membership equals its roster value. Zero deviations pass. |
| RSD-ORG-02b | No field of a support request, offer, contract or guest-admission record shall change a vehicle's cryptographic identity, group membership, security state or policy. | **T, S.** Guest-admission unit test: no field of a request, offer, contract or admission record writes identity, membership, security state or policy (MD15). SCN-CV-01 deterministic check in every run: the helper's identity, membership, security-state and policy fields are never changed through the contract. |
| RSD-ORG-03 | @G No in-mission security authority (a–d). |  |
| RSD-ORG-03a | No node, including the command ship, shall hold a key or a message type that grants it authority over another node's security state during the mission. | **S.** The provisioning package of the reference mission gives the command ship only its own vehicle keys. No record type carries a security-state assignment that a receiver applies on the sender's authority. Every security-state transition has exactly one writer, the local evaluation function. |
| RSD-ORG-03b† | The headquarters GCS application shall contain no code path that writes a vehicle's security state, the roster, a key, the policy or a threshold. | **S, T.** Static check: the GCS application build links no M-SEC, M-OBS or evidence-evaluation code and holds no vehicle or operator key. GT-12: no command, valid or invalid, changes a security state. |
| RSD-ORG-03c | When an agent has verified a G3 proof against a vehicle, the agent shall quarantine that vehicle at once, without waiting for the window close or for a decision of any other node. | **T.** GT-08 and E12 equivocation runs: for every honest agent that verifies the proof, the event log records the Quarantined decision at the simulation time at which verification completes, the decision cites the proof record, and no decision cites a message from the command ship or headquarters. |
| RSD-ORG-03d | When a vehicle has been quarantined in a component, the producer of that component shall publish a revocation record that embeds the G3 proof and excludes the vehicle from the next swarm key. | **T.** E3 and E12 runs (equivocation, invalid revocation record): every component that holds the proof by the end of the run publishes a revocation record whose embedded proof passes every revocation-record check at every receiver, and the excluded certificate identifier is absent from the recipient list of the new epoch. |
| RSD-ORG-04 | Entering the Alone connection state shall not, by itself, cause a vehicle to enter the Return mission state. | **A.** Model check of the connection and mission state machines: no transition leads from Connection = Alone to Mission = Return unless another Return condition holds. Supporting run-time check in every run: Alone is never followed by Return without another trigger. |
| RSD-ORG-05 | @G Pre-mission provisioning (a–c). |  |
| RSD-ORG-05a | Headquarters shall sign one provisioning package per mission and load it into every vehicle and the command ship before the mission starts. | **I, T.** Inspection of the provisioning-script output: one package per mission, with every item signed. GT-05 passes. |
| RSD-ORG-05b | After loading, each vehicle shall sign a load receipt that carries the mission manifest hash and its own device envelope hash. | **T.** GT-05: every vehicle returns a signed load receipt that carries both hashes. |
| RSD-ORG-05c | The mission shall start only when every vehicle reports the same mission manifest hash and each device envelope hash matches the value that headquarters recorded for that vehicle. | **T.** GT-05: all mission manifest hashes are equal and every device envelope hash matches its recorded value. A negative case of GT-05 with one altered hash does not start the mission. |

**Design rationale.**
- Every group carries all three root classes because confirmation needs at least three classes in two modalities. The command ship carries no fusion sensor because it would otherwise count as an observer in every scope.
- RSD-ORG-02b restricts what cooperation records can write; it does not freeze a guest's security state. Dynamic membership would need a new state machine and a new authority and is left to future work (Section 6.6).
- A proof shows that a key was misused, not who used it: a cloned key also excludes the genuine vehicle (AS2, LIM-37), and a vehicle quarantined in one component stays Trusted in another until the proof arrives (LIM-13). Return is entered only through the mission state machine (Fig. 3.2).
- The load receipt carries a mission manifest hash over the shared part of the package and a device envelope hash over the vehicle's own keys. The acceptance criterion of RSD-ORG-05c also includes a negative case with one altered hash.

The build of each vehicle is fixed by RSD-CMC-05 (Section 4.6), and the 20-node claim boundary by RSD-SIM-06 (Section 4.11).

## 4.2 Mission Management and Cooperation

### Task allocation

**Table 4.2. Task allocation requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-MIS-01 | @G Leaderless task allocation (a–h). |  |
| RSD-MIS-01a | Within a connected component, the system shall allocate tasks by the single-round sealed-bid auction over signed task and bid records, without a coordinating node. | **S, T.** Inspection: the build contains one allocator, the single-round auction with a commit step, and no consensus-based allocator. Unit test with scripted tasks and bids: allocation completes within one bid window and no message from a coordinating node is involved. |
| RSD-MIS-01b | A task identifier shall be the hash of the task type, the task object and the window in which the need appeared. | **T.** Unit test: two agents that observe the same need in the same window create task records with the same identifier, and the evidence store holds one record. |
| RSD-MIS-01c | The auction shall use, for each vehicle and task, only that vehicle's newest signed bid, so that a corrected bid supersedes the earlier one. | **T.** Unit test: a vehicle publishes a bid and then a corrected bid; every agent computes the winner from the corrected bid only. |
| RSD-MIS-01d | At the close of the bid window, each agent shall select as winner the highest bid in its own evidence set, break ties by the smaller rank number, and ignore any bid that implies an impossible arrival time. | **T.** Unit test with scripted bids that include equal bids and a bid with an impossible arrival time: every agent with the same bids selects the same winner, ties go to the smaller rank number, and the impossible bid is ignored. |
| RSD-MIS-01e | The winner of an exclusive task shall start executing it only after one window has passed without a competing lease or contract for the same task in its evidence set. | **T.** GT-06: no exclusive task is executed by two vehicles. Event-log check: every committed lease or contract is preceded by one full window without a competing record. |
| RSD-MIS-01f | When two leases or contracts compete for the same exclusive task, including after a merge of components, the agents shall resolve the conflict before either vehicle moves by keeping the earlier record (by window, then by the smaller rank number) and cancelling the other. | **T.** GT-06: with bids delayed so that two agents pick different winners, the conflict is detected within one window and resolved by the stated rule before either vehicle moves. E4 merge runs: conflicting assignments are resolved by the same rule (event-log check). |
| RSD-MIS-01g | A duplicate assignment of a duplicable task shall be recorded in the event log. | **T.** Scripted test with two vehicles assigned the same duplicable task: the event log records the duplicate assignment. |
| RSD-MIS-01h | A duplicate assignment of a duplicable task shall not produce a finding against either vehicle. | **T.** Event-log check in the reference and E11 runs: no finding has a duplicate assignment of a duplicable task as its basis. |

### Inter-group support requests

The support protocol keeps the structure of the FIPA Contract Net interaction protocol [66]: a call for proposals with a deadline, a proposal or a refusal, acceptance of the chosen proposal and a completion message. It adds a contract message that binds the assigned vehicle, and what a delay-tolerant, adversarial setting needs: deferral, cancellation, deterministic request identifiers and validity deadlines.

**Table 4.3. Inter-group support requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-MIS-02 | @G A support request is never a command (a–b). |  |
| RSD-MIS-02a | Mission & Cooperation shall provide no interface through which a record from another group sets a waypoint of a receiving vehicle. | **S.** Static dependency check: no interface leads from a foreign record to the waypoint interface of Mission & Cooperation (MD14). |
| RSD-MIS-02b | A support request shall become a task of the receiving group only through that group's own auction. | **T.** SCN-CV-01 deterministic check in every run: a support request never changes a receiving vehicle's mission state without a task won in the receiving group's auction (MD14). |

### Verification Backlog and collaborative verification

**Table 4.4. Verification Backlog and collaborative verification requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-MIS-03 | @G Per-group Verification Backlog (a–m). |  |
| RSD-MIS-03a | Each group shall maintain a Verification Backlog that is a deterministic function of its Evidence Graph and Observation Graph, with items created from the uncertainty records of the Observation Graph. | **T, M.** Replay test in the manner of GT-09: the same evidence delivered in different orders gives every agent of a connected group the same backlog items in the same order. E11 reports backlog age with 95 % confidence intervals; no pass threshold is registered for backlog age. |
| RSD-MIS-03b | Backlog items shall be ordered by the priority score S(n, t), which combines mission importance, urgency, the value of another observation and its cost, in descending order, then by earlier review time, then by item identifier. | **T.** Unit test with scripted items: the computed order equals the order derived by hand from the priority score with the initial parameter values, with ties broken by review time and then by item identifier. |
| RSD-MIS-03c | An item whose observation value is zero shall not be tasked. | **T.** SCN-CV-01 deterministic check in every run: no item with zero value is tasked. |
| RSD-MIS-03d | A local verification task shall be created for an item when its best local score is at least θloc (initial value 0.3) and not lower than its best external score. | **T.** Unit test with scripted scores on both sides of θloc: a local task is created exactly when the local score is at least θloc and not lower than the external score. |
| RSD-MIS-03e | When no local task is created, a support request shall be sent for an item when its best external score is at least θext (initial value 0.5) and higher than its best local score, to the group that offers the highest observation value according to its capability inventory. | **T.** Unit test: a request is sent exactly when no local task is created, the external score is at least θext and higher than the local score; it is addressed to the group with the highest value in the capability inventory. SCN-CV-01 variant CV-a produces a REQUEST to group B. |
| RSD-MIS-03f | A group shall have no more than two open support requests at a time. | **T.** Unit test, and SCN-CV-01 variant CV-f (requests forged in quantity): no group ever has more than two open requests. |
| RSD-MIS-03g | Every message of the support protocol (REQUEST, OFFER, REFUSE, DEFER, ACCEPT, REJECT-OFFER, CONTRACT, RESULT and CANCEL) shall be a signed C2 record. | **T.** Unit test: every protocol message type is produced as a signed C2 record; a protocol message without a valid signature is rejected. |
| RSD-MIS-03h | A support request identifier shall be the hash of the backlog item identifier, the need type and the window. | **T.** Unit test: two agents of the requesting group that see the same need in the same window create the same request identifier, and the duplicates merge. |
| RSD-MIS-03i | A support-protocol message that arrives after its deadline shall be treated as invalid. | **T.** Unit test: a message received after its deadline leaves the contract state unchanged, and the contract enters Expired when its deadline passes without the required message. |
| RSD-MIS-03j | Only a vehicle that is Trusted in the deriving agent's view shall be eligible to take a support contract. | **T.** Unit test: a vehicle in Watch, Restricted, Probation or Quarantined never wins a support contract in the receiving group's auction. |
| RSD-MIS-03k | Each Reassessed outcome that does not resolve an item shall decrement the item's verification budget, whose initial value is 3. | **T.** Unit test: a new item starts with budget 3, and each unresolved Reassessed outcome lowers it by exactly one. |
| RSD-MIS-03l | An item shall become Exhausted when its budget is spent, when its best score is zero or below, or when no eligible verifier exists inside or outside the group. | **T.** Unit test: an item becomes Exhausted in each of the three stated cases and in no other case. |
| RSD-MIS-03m | A contact shall receive the status Unknown only when its backlog item is Exhausted. | **T.** Run-time check in every run: every Unknown status references an Exhausted backlog item. SCN-CV-01 variant CV-b (contact absent) ends Reported or Unknown, with no Confirmed status. |

### Connectivity repair

**Table 4.5. Connectivity repair requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-NET-06 | @G Value-triggered connectivity repair (a–d). |  |
| RSD-NET-06a | The connectivity-repair trigger shall take as inputs only the pending delivery value and Network Graph connectivity; the only security input that may change it is the removal of a quarantined or transit-restricted vehicle from the Network Graph. | **S, T.** Static check: the inputs of the trigger function are the pending delivery value and Network Graph connectivity only (MD13). MD13 test with a fabricated suspicion, and SCN-CR-01 variant CR-f (fabricated need from one origin): the trigger is unchanged. |
| RSD-NET-06b | A repair shall be executed only if the winning bid's net utility is at least θrep (initial value 0.2) and exceeds the utility of waiting, which weighs the probability of a predicted contact before the deadline against the expected delay. | **T.** Unit test of the decision with scripted utilities on both sides of θrep and of the waiting utility. SCN-CR-01 variants CR-d (only candidate unsafe) and CR-g (gap too wide for one position): no vehicle moves and the bundles wait. |
| RSD-NET-06c | A link created by a repair move shall enter the Network Graph only after it has been measured and confirmed by signed link-state records of both ends. | **T.** SCN-CR-01, every run: the number of Network Graph edges admitted without two measured, signed confirmations is 0. MD16 test: a repair move whose link fails leaves the target unreachable in the Network Graph. |
| RSD-NET-06d | A repair that fails, is released or is abandoned shall not cause any pending bundle to be discarded. | **T.** SCN-CR-01, every run: no bundle is discarded because a repair was attempted; after a repair that produces no path within Ttry, the bundles remain queued. |

### Post-mission analysis

**Table 4.6. Post-mission analysis requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-MIS-04 | @G Post-mission analysis by scripts (a–c). |  |
| RSD-MIS-04a | Post-mission analysis shall be performed by scripts that read the event log after the run. | **I.** Inspection of the analysis pipeline: the incident summary and metric tables are produced by scripts from the event log, and no manual or model-generated step is involved. |
| RSD-MIS-04b | The system shall contain no language-model report generator and no semantic-fusion component. | **S.** Inspection of the build and its dependency list: no language-model component and no semantic-fusion engine are present. |
| RSD-MIS-04c | Every line of the post-mission incident summary shall cite the record identifiers it rests on. | **I, T.** The analysis scripts check that every cited record exists in the event log and that every number in the incident summary matches the log. Inspection of the incident summary of one reference run finds no line without a record citation. |

**Design rationale.**
- For exclusive tasks the auction gives the same safety as a consensus allocator with fewer messages, and it works in a component of any size. It assumes honest bidding (AS15); over-bidding within physical limits remains a known limit (LIM-16).
- The backlog is derived like the Observation Graph, so all members of a connected group compute the same items and priorities without a leader. Value-based priority holds only as far as the sensor model is right (AS9), and the helping group pays in coverage (LIM-20).
- The repair trigger uses only delivery value and connectivity, which keeps suspicion out of mission movement (MD13). Repair relies on predicted links and stale positions (AS16, AS17), so a move can fail; RSD-NET-06c and RSD-NET-06d keep a failed move from corrupting the Network Graph or losing data.

The optional scenarios SC-7 and SC-8 (Table 6.7) would add measurements of these requirements.

## 4.3 Detection and Evidence Fusion

### Observation statuses

The status of a contact, and separately of its type attribute, is recomputed from the evidence at every window close and has no transitions of its own. The first row of Table 4.7 whose condition holds gives the status. L is the fused log-likelihood ratio, L+ and L− the sums of positive and negative class contributions, n+ and n− the numbers of supporting and opposing classes, and m+ and m− their modalities. Decision classes are: A0 collects information, A1 is a reversible adaptation and A2 a high-impact decision.

**Table 4.7. Observation statuses in order of precedence.**

| Status | Condition | May support |
|---|---|---|
| Retracted | The contact had support, and no untainted supporting class remains | Audit only |
| Confirmed | L ≥ b, m+ ≥ 2 and L− > −a; with b > 2c this implies n+ ≥ 3 | A2 |
| Refuted | L ≤ −b, n− ≥ 2, m− ≥ 2, and the supporting side has at most one modality | Audit only |
| Contested | L+ ≥ a and L− ≤ −a | A0; verification |
| Corroborated | L ≥ a, n+ ≥ 2 and L− > −a | A1 |
| Unknown | The contact's backlog item is Exhausted and no row above holds | A0, conservative option |
| Reported | Default: no row above holds | A0; collect information and open a verification need |

### Evidence records and dissemination

**Table 4.8. Evidence record and dissemination requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-EVD-01 | @G Bundles, digests and pull (a–g). |  |
| RSD-EVD-01a | At the close of each window in which it has new records, each node shall broadcast, exactly once, one bundle that carries those records, with the header, the records and their Merkle root under one signature. | **T.** Unit and integration tests of the bundle path: in a window with new records, a node emits exactly one bundle whose signature covers the header, the records and the Merkle root, and transmits it once. |
| RSD-EVD-01b | A node with no new records in a window shall send no bundle for that window. | **T.** Unit test: a node with no new records in a window emits no bundle. |
| RSD-EVD-01c | Each node shall broadcast one digest per window that contains, for each origin, the highest record sequence number it holds and a prefix of the chain-head hash, together with a hash of its evidence set, a hash of its derived states with the policy version, and the set of nodes it heard directly. | **T.** Unit test: each node emits exactly one digest per window, and the digest contains every listed field. |
| RSD-EVD-01d | A node that learns from a neighbor's digest that it lacks records of an origin shall pull the missing range at fragment granularity. | **T.** Integration test: a bundle that lost one 1400-byte fragment is completed by pulling that fragment only. GT-08 reports the fragment repair rate. |
| RSD-EVD-01e | A receiver shall accept at most one bundle per origin and window for evaluation. | **T.** Unit test: repeated pushes of bundles for the same origin and window lead to exactly one evaluated bundle. |
| RSD-EVD-01f | A receiver shall treat a second, different bundle for an origin and window whose signature verifies as an equivocation proof, even when the origin's quota is exhausted. | **T.** Unit test and E12 equivocation runs: a second, different valid bundle for the same origin and window yields a proof at the receiver, including when the receiver's quota for that origin is exhausted. |
| RSD-EVD-01g | A receiver shall check a bundle's size, the roster membership of its origin and the origin's quota before verifying the bundle's signature. | **T.** Unit test with instrumented verification: an oversized bundle, a bundle from a non-roster origin and a bundle over quota are rejected without any signature-verification call. |
| RSD-EVD-02 | @G Content-addressed records (a–b). |  |
| RSD-EVD-02a | The identifier of a record shall be the SHA-384 hash of the canonical encoding of all its other fields. | **T.** Unit test for every record type: the identifier equals the SHA-384 hash of the canonical encoding of the other fields, and the same content always yields the same identifier. |
| RSD-EVD-02b | A record that arrives over several paths shall be stored once and counted as one root. | **T.** The same record delivered over two paths leaves the stored record count and the root count unchanged (MD3). |

### Deterministic evaluation and late evidence

**Table 4.9. Deterministic evaluation and late-evidence requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-EVD-03 | @G Same evidence, same states (a–d). |  |
| RSD-EVD-03a | Agents that hold the same evidence set shall derive the same observation statuses, sensor-channel states and vehicle security states. | **T.** GT-09: the same valid evidence delivered in many different orders gives identical final states. Run-time check in every run: equal evidence hashes in the digests imply equal state hashes (MD8). |
| RSD-EVD-03b | The evaluation functions shall not depend on wall-clock time, reception time, randomness, unordered iteration or platform-dependent rounding. | **S, T.** Code inspection of the evaluation functions for the listed dependencies. Test: the same evidence evaluated under two different reception orders and processing speeds gives identical state hashes. |
| RSD-EVD-03c | An agent that observes equal evidence hashes with different state hashes shall log the divergence. | **T.** E12 version-mismatch test: the divergence is logged with both hashes and the window. |
| RSD-EVD-03d | An agent shall not adopt a derived state from another agent. | **S, T.** Static check: no interface writes a derived state from a received message. E12 version-mismatch test: no agent's state changes because of another agent's state hash. |
| RSD-EVD-05 | @G Backdating and late evidence (a–g). |  |
| RSD-EVD-05a | A receiver shall reject a record whose window precedes the window of its predecessor in the origin's chain. | **T.** E12 backdating runs: backdated records accepted = 0; each rejection records a K1 finding against the origin. |
| RSD-EVD-05b | A receiver that has heard an origin's chain head in window v shall reject any later-arriving record of that origin dated before window v − 2. | **T.** Unit test: after the receiver has heard an origin's head in window v, a record of that origin dated before v − 2 is rejected and a record dated v − 2 or later is accepted. |
| RSD-EVD-05c | A receiver shall accept a pushed bundle only if the bundle belongs to the current or the previous window. | **T.** Unit test: a pushed bundle from an earlier window than the previous one is rejected, and the same records are accepted when they arrive by pull. |
| RSD-EVD-05d | All late records of one window shall be re-evaluated together, from the earliest affected window, in the four-stage order freeze, observation, security, apply. | **T.** GT-10: a late record re-opens an old window, and the affected windows are re-evaluated in stage order from the earliest one. |
| RSD-EVD-05e | Re-evaluation shall produce at most one status or channel transition per object and window and no two-window cycle. | **T.** GT-10: at most one transition per object and window and no two-window cycle (MD17). |
| RSD-EVD-05f | After a re-evaluation, only the present state shall be enforced. | **T.** GT-10, including this check: after re-derivation, enforcement lists and permissions reflect only the present state. |
| RSD-EVD-05g | A physical effect already taken under a past state that a re-evaluation rewrites shall be marked in the event log. | **T.** GT-10, including this check: every physical effect taken under a rewritten past state is marked in the event log. |

### Observation fusion

**Table 4.10. Observation fusion requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-OBS-01 | @G Clipped per-class fusion and status rules (a–g). |  |
| RSD-OBS-01a | For each contact and claim, fusion shall take from each root class only the observation with the largest absolute log-likelihood ratio as that class's contribution. | **T.** Unit test with several observations from one class: only the observation with the largest absolute log-likelihood ratio enters L. |
| RSD-OBS-01b | A root class whose strongest detection and strongest miss both have an absolute log-likelihood ratio of at least λmin (initial value 1.0) shall contribute zero. | **T.** Unit test: a class holding a detection and a miss, both with absolute value at least 1.0, contributes zero; a miss below 1.0 inside a class is ignored (for example, a distant camera miss with detection probability 0.3 gives about −0.34 and cannot cancel a close detection). |
| RSD-OBS-01c | Each class contribution shall be clipped to [−c, c] and combined with a neutral prior into the fused log-likelihood ratio L and the sums L+ and L− of the positive and negative class contributions. | **T.** Unit test: a radar detection with a log-likelihood ratio of +4.55 contributes +3; L, L+ and L− equal their definitions for scripted inputs. |
| RSD-OBS-01d | Observation statuses shall be derived by the rules of Table 4.7 in their order of precedence, with the initial thresholds c = 3, a = 4 and b = 7. | **T.** Unit tests over scripted evidence sets that cover every row of Table 4.7: the derived status equals the table, and exactly one status results for every evidence set. |
| RSD-OBS-01e | The fusion parameters of every policy set shall satisfy c < a ≤ 2c < b ≤ 3c and b ≥ a + c. | **I.** Inspection of the policy set of every run configuration: both inequalities hold. |
| RSD-OBS-01f | A contact shall be created only after two detections within three scans inside one association gate. | **T, M.** Unit test: a single detection, or two detections more than three scans apart, creates no contact; two detections within three scans inside one gate create one contact. E6 reports the share of false contacts suppressed against the pre-registered hypothesis of at least 90 %; that result is reported and does not decide this requirement. |
| RSD-OBS-01g | Observations of a sensor channel in state Limited shall have weight zero in fusion. | **T.** Unit test: after a channel becomes Limited, its observations contribute zero from the next window, and its class contributes from its remaining observations. |
| RSD-OBS-02 | @G No single-class conclusion; signatures do not make content true (a–b). |  |
| RSD-OBS-02a | A contact or type attribute shall not reach the status Corroborated or Confirmed when only one root class supports it. | **T.** MD6 test and a property test over generated evidence sets: no evidence set with one supporting class yields Corroborated or Confirmed. |
| RSD-OBS-02b | A valid signature on an observation record shall not, by itself, increase the support that fusion counts for the record's content. | **T.** MD2 test: a signed false observation from one class never rises above Reported. E6 reports the number of poisoned observations used. |
| RSD-OBS-03 | @G Claim-scoped common causes (a–d). |  |
| RSD-OBS-03a | Every record whose content depends on a vehicle position shall declare its position source class: GNSS, radar fix, mutual tracking or other. | **T.** Schema test: every position-deriving record type carries a position-source tag from the closed set; a record without the tag fails encoding. |
| RSD-OBS-03b | Root-class dependency shall be evaluated separately for each claim type (object existence and type; positions and position-dependent claims; protocol violations; link claims), with no merge across claim types. | **T.** Unit test: an EO observation from UAV-1 and a radar observation from USV-1, both navigated by GNSS, form two classes for object existence and one class for position. |
| RSD-OBS-03c | For position claims, all GNSS-derived positions shall form one root class. | **T, M.** E9 GNSS-spoofing runs: no independence is credited across GNSS-derived positions (event-log check in every run). |
| RSD-OBS-03d | Common-cause tags for sensor model and software version shall be taken from the mission configuration, never from the claimant's own statement. | **T.** Unit test: a record whose own tags claim a different sensor model or software version than the mission configuration is assigned to the root class that the configuration defines. |

### Evidence capacity

**Table 4.11. Evidence capacity requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-OBS-04 | @G Per-sector evidence capacity regime (a–b). |  |
| RSD-OBS-04a | For every sector and window, the agents of a group shall classify the sector as S3 when R ≥ 3 and M ≥ 2, as S1 when R ≤ 1, and as S2 otherwise, where R is the number of root classes for object existence that can currently observe the sector and M is the number of their modalities. | **T, M.** Unit test with scripted presence, channel states and shadowing: the regime follows the stated rule; with group A's two USVs in the island's shadow, A's sector is S2, and three classes of a single modality also give S2. E11 reports regime shares next to the realized verification rate. |
| RSD-OBS-04b | The regime shall be computed only from the roster, the sensor profiles and the current derived states. | **T.** Replay test: every member of a connected group derives the same regime for each sector and window. |

### Evidence-plane budget and proof dissemination

**Table 4.12. Evidence-plane budget and proof dissemination requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-EVD-04 | @G Evidence-plane budget and proof priority (a–d). |  |
| RSD-EVD-04a | Evidence-plane traffic (bundles, digests and pulls) shall use no more than 10 % of channel time. | **M.** E1: the upper bound of the 95 % confidence interval of the measured evidence-plane share is below 10 %. |
| RSD-EVD-04b | Outside a merge, pull traffic shall use no more than the pull budget β (initial value 5 %) of channel time. | **M.** E4: the measured pull share outside merge phases does not exceed β. The pull-share metric and its averaging interval are open (TBD-01, Section 6.5). |
| RSD-EVD-04c | Every evidence-plane queue shall send proofs and revocation records first, then state-changing records (findings, link-state records and contracts), then all other records. | **T.** Unit test of every evidence-plane queue with mixed content: the transmission order is proofs and revocation records, then state-changing records, then the rest. |
| RSD-EVD-04d | Proofs and revocation records shall not be deferred by the evidence-plane cap. | **T, M.** Event logs of E1 and E3: the number of proofs or revocation records deferred by the cap is 0 in every run. |
| RSD-EVD-06 | @G Proof dissemination (a–c). Delivery is conditional on D1–D5 (guarantee G-e, Section 6.2). |  |
| RSD-EVD-06a | Every agent that verifies a G3 proof shall re-broadcast it once, without suppression. | **T.** GT-08 and E3 event logs: every agent that verifies a proof re-broadcasts it exactly once, regardless of how many neighbors have already broadcast it. |
| RSD-EVD-06b | When two components meet, they shall exchange proofs and revocation records before digests, pulls and pending bundles. | **T, M.** E3 and E4 merge runs: whenever one component holds a proof or revocation record that the other lacks, those records are the first evidence-plane transmissions between the components after their hop sessions open. GT-08 passes. E3 reports the time from a G3 event to quarantine in every group against its pre-registered hypothesis (at most 2 s at p95 within a component; at most 5 s at p95 after the first verified link); that result tests a design hypothesis and does not decide this requirement. |
| RSD-EVD-06c | When a repaired path becomes valid, proofs and revocation records for the target set shall be sent before C2 and C1 bundles. | **T.** SCN-CR-01 event log: after a repaired path becomes valid, proofs and revocation records for the target set precede all C2 and C1 bundles. |

**Design rationale.**
- Because b > 2c, Confirmed needs at least three supporting classes; because b ≥ a + c, removing one class from a Confirmed contact leaves it at least Corroborated, which bounds the effect of taint after a proof. With f compromised classes, L shifts by at most 2fc (CLM-17). These are algebraic properties: if the sensor models are wrong, the statuses are wrong (AS9, tested in E5).
- Dependencies are scoped to the claim they can corrupt: a shared GNSS corrupts positions, not the existence of an object seen by two sensors. Dependency tags come from the mission configuration, so a claimant cannot declare itself independent.
- Convergence requires eventual delivery (AS4) and identical deterministic software (AS12). Equal evidence hashes with different state hashes reveal a fault or a compromised agent, which is why states are never copied (RSD-EVD-03d).
- RSD-EVD-06 specifies the mechanisms that carry proofs, not their delivery, which a lossy channel cannot guarantee; the GT-08 outcome "proof reaches every agent" holds for one scripted scenario only. Guarantee G-e states the conditions under which a proof reaches another honest agent (Section 6.2).

The limitations of this section are LIM-01, LIM-02, LIM-13, LIM-15, LIM-17, LIM-25, LIM-43 and LIM-47 (Section 6.3). RSD-CMC-06 bounds Corroborated and Confirmed statuses at honest agents under a build-confined compromise (Section 4.6).

## 4.4 Communication and Networking

**Table 4.13. Communication and networking requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-NET-01 | @G Broadcast overlay over UDP (a–f). |  |
| RSD-NET-01a | The mesh shall be implemented as an application-layer overlay over UDP, with its core (neighborhood, link-state processing, Network Graph, path computation and connectivity analysis) independent of the socket layer. | **S.** The agent sends every overlay frame through UDP sockets, and no ns-3 routing protocol is installed in TULPAR runs; the mesh core library builds without the socket adapter. |
| RSD-NET-01b | Every overlay frame, including hop-addressed frames, shall be transmitted as a MAC broadcast. | **T.** PHY trace of a reference run: no unicast MAC data frame originates from an agent. Integration test with two nodes 32 km apart in open sea under the sea channel model: overlay frames are received. |
| RSD-NET-01c | The data rate and the non-unicast rate of every agent's Wi-Fi station shall both be set to HtMcs0. | **I.** Station-manager configuration: DataMode = NonUnicastMode = HtMcs0. |
| RSD-NET-01d | The receiving hop shall acknowledge each C1 and C2 frame with an overlay hop acknowledgment under the hop session. | **T.** Unit test: every received C1 and C2 frame produces one hop acknowledgment; telemetry, HELLO, digest and bundle frames produce none. |
| RSD-NET-01e | A C1 or C2 frame whose hop acknowledgment has not arrived within 15 ms shall be retransmitted, up to three times per hop. | **T.** Unit test with suppressed acknowledgments: the frame is sent four times in total, 15 ms apart, and the hop is then declared failed. |
| RSD-NET-01f | The overlay shall split every message into fragments of at most 1400 bytes, each carrying its own hop or swarm-key tag. | **T.** Fragmentation unit test: a bundle carrying a 4627-byte signature is split into fragments of at most 1400 bytes, each with its own tag. GT-08 at 10 % frame loss: a lost fragment is repaired alone (fragment repair rate reported). |
| RSD-NET-02 | @G Verified neighborhood and two-end edge rule (a–e). |  |
| RSD-NET-02a | Two nodes shall become verified neighbors only after they hold a hop session from the three-message handshake and authenticated frames have passed in both directions within the last measurement window (10 s). | **T.** GT-02 (discovery and hop session). Unit test: a neighbor heard in one direction only is not a verified neighbor and contributes no edge. |
| RSD-NET-02b | An undirected edge shall enter a node's Network Graph only if the latest fresh link-state advertisements of both ends list each other as neighbors. | **T.** MD2 test: a signed one-sided edge never enters the Network Graph; it is still stored as evidence for M15. Zero edges are admitted without two signed confirmations. |
| RSD-NET-02c | No edge shall enter the Network Graph from a predicted position, a channel-model prior or a planned repair. | **T.** MD16 test: a repair move whose link fails leaves the target unreachable. Unit test: channel-model priors used by the planner never appear as Network Graph edges. |
| RSD-NET-02d | A received HELLO shall serve only as a trigger for session establishment and never, by itself, create a link or an edge. | **T.** Unit test: a valid HELLO without a completed handshake creates no link and no edge. |
| RSD-NET-02e | A link-state advertisement whose sequence number is not greater than that of the last accepted advertisement of the same origin shall be rejected. | **T.** Unit test: a replayed or reordered link-state advertisement with an older or equal sequence number is rejected. |
| RSD-NET-03 | @G Delivery and accountability are separate (a–c). |  |
| RSD-NET-03a | Reliable Delivery & DTN shall have no interface that writes to the Evidence Store or to Distributed Security, except the corrupted-copy fact (I3). | **S.** Static dependency check: the delivery build target links the evidence and security targets only through the corrupted-copy fact interface. |
| RSD-NET-03b | Packet loss, delay or disconnection alone shall not produce a finding, a candidate or a change of security state. | **T, M.** Loss-only scenario of Phase 5 and every E2 run: anomaly records = 0 (must be 0). |
| RSD-NET-03c | The avoidance penalty shall remain a local, unsigned and unshared routing input. | **S, T.** No record type carries the avoidance penalty. Unit test: changing a node's penalty changes route preference and no security state. |
| RSD-NET-04 | @G Security state in routing (a–d). |  |
| RSD-NET-04a | Path computation shall exclude every vehicle that is Quarantined in the computing agent's view, including as a last resort. | **T.** Network Graph filter unit test (MD11): a Quarantined vehicle is absent from every computed path, also when it is the only cut vertex. |
| RSD-NET-04b | Path computation shall exclude every vehicle Restricted in the transit scope whenever a path without such vehicles exists. | **T.** Unit test: when a path without transit-restricted vehicles exists, no computed path contains one. |
| RSD-NET-04c | A vehicle Restricted in the transit scope shall carry only end-to-end protected C1 or C2 traffic, and only when no path without a transit-restricted vehicle exists. | **T, M.** GT-07: with an honest cut vertex restricted by two accusers, the last-resort path keeps the component connected for end-to-end protected C1 and C2 traffic only. E7 reports delivery with and without the last-resort path against its pre-registered hypothesis; that result does not decide this requirement. |
| RSD-NET-04d | No security state other than Quarantined and Restricted in the transit scope, and no suspicion, shall change path computation. | **S.** Static check: path computation reads only the quarantine and transit-restriction list from Distributed Security (I9). |
| RSD-NET-05 | @G Delay-tolerant delivery (a–f). |  |
| RSD-NET-05a | A C1 or C2 message whose destination is not in the source's Network Graph shall be held as a bundle in the source's store until the destination appears or the message lifetime ends. | **T.** Delivery state-machine unit test: Queued → Bundled when the destination is not in the Network Graph; the bundle is retained until the destination appears or the lifetime ends. |
| RSD-NET-05b | When the destination of a bundle appears in the Network Graph within the bundle's lifetime, the bundle shall be sent over an ordinary source route. | **T.** Unit test: Bundled → Queued when the destination appears, followed by transmission over a source route. E4 partition runs: bundles are delivered after the merge within their lifetime. |
| RSD-NET-05c | A message whose lifetime ends before delivery shall be reported to the application as undeliverable. | **T.** Unit test: Retrying or Bundled → Undeliverable at lifetime end, with an undeliverable notice to the application. |
| RSD-NET-05d | No transition of the message delivery state machine shall write evidence. | **T.** Unit test over every delivery-state transition: none writes a record to the Evidence Store (MD1). |
| RSD-NET-05e | A bundle for a disconnected destination shall be encrypted by encapsulation to the destination's static ML-KEM-1024 key, with a fresh key per bundle, and signed by the source with ML-DSA-87. | **T.** Unit test: a bundle decrypts only with the destination's static key; two bundles never share a symmetric key; the source signature verifies. |
| RSD-NET-05f | The bundle queue shall send proofs and revocation records first, then C2 items in deadline order, then C1 items. | **T.** Unit test with a mixed bundle queue: transmission order is proofs and revocation records, then C2 by deadline, then C1. |

**Design rationale.**
- Overlay fragmentation matters for evidence: with IP fragmentation over broadcast and 10 % frame loss, a four-fragment bundle would arrive complete only 66 % of the time. The ns-3 AODV and OLSRv1 baselines need a raised PHY slot, which they pay for in their measured overhead (LIM-22).
- Under AS1 and AS13 a single insider cannot insert an edge involving an honest node into an honest Network Graph. Two colluding insiders can confirm a fake edge between themselves, and a wormhole that tunnels the handshake passes two-end confirmation (LIM-05).
- If a transit-restricted vehicle were always removed, two colluding accusers could split a component by accusing the honest vehicle that joins it. The last-resort path avoids this at a bounded cost: the restricted vehicle sees only ciphertext and can drop or delay but not alter undetected (LIM-26).
- Bundle encryption follows the hybrid pattern of HPKE [67]: a key-encapsulation mechanism, here ML-KEM-1024, gives a fresh key for each bundle, and AES-256-GCM protects the content (RSD-NET-05e).

A vehicle that only drops is avoided, not detected (LIM-03, LIM-04), and delay-tolerant bundles give no forward secrecy and no delivery during disconnection (LIM-12). Path selection by traffic class, the retry ladder, the TULPAR Delivery Protocol and its receipts are design details of the architecture and are not specified as requirements here.

## 4.5 Cryptography and Authentication

Signatures are kept off the message path. Periodic traffic is protected with AES-256-GCM and HMAC at a cost of microseconds per message; signatures appear only on evidence and topology records, at most one bundle per node per 5 s window, verified off the forwarding path after cheap quota checks. The estimated cost is about 0.96 ms per signature and 0.51 ms per verification, about 2 ms of verification per second per vehicle; these are estimates scaled from published cycle counts and remain TBR-01 until the Phase 1 micro-benchmark, which still describes the development machine rather than the vehicle processor (LIM-11).

Authentication works at three layers. The discovery key authenticates HELLO messages against outsiders only, because every insider holds it. A hop session, set up by a three-message ML-KEM-1024/ML-DSA-87 handshake, protects every hop-addressed frame between verified neighbors. An end-to-end session protects C1 and C2 payloads, so relays forward ciphertext they can neither read nor alter undetected; a bundle for a disconnected destination is encapsulated to the destination's static key. The handshake also carries a boot quote, so that a peer can refuse a vehicle whose stored agent image was changed (RSD-CMC-04).

**Table 4.14. Cryptography and authentication requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-CRY-01 | @G Cryptographic algorithm suite with real implementations (a–g). |  |
| RSD-CRY-01a | Key establishment for hop and end-to-end sessions and for bundle encapsulation shall use ML-KEM-1024. | **I, T.** Configuration check of the suite identifier; crypto unit tests of the handshake and of bundle encapsulation with ML-KEM-1024 test vectors. |
| RSD-CRY-01b | Bundles, handshake transcripts, link-state advertisements and every other signed record shall be signed with ML-DSA-87. | **I, T.** Crypto unit tests: every signed record type verifies under ML-DSA-87; a wrong key produces a real verification failure. |
| RSD-CRY-01c | Message encryption and integrity shall use AES-256-GCM. | **T.** Crypto unit tests: AES-256-GCM encryption and tag verification; a modified ciphertext or associated datum fails verification. |
| RSD-CRY-01d | Hashing, message authentication and key derivation shall use SHA-384, HMAC-SHA-384 and HKDF-SHA-384. | **T.** Crypto unit tests of SHA-384, HMAC-SHA-384 and HKDF-SHA-384 against published test vectors. |
| RSD-CRY-01e | From Phase 1, every cryptographic operation in the simulation shall be performed with the real algorithm through Botan 3.13.0. | **S, T.** Build inspection: Botan 3.13.0 is linked into the crypto target from Phase 1; no stub produces signatures, ciphertexts or tags. Packet traces carry real sizes (4627-byte signatures, 1568-byte ciphertexts). |
| RSD-CRY-01f | ML-DSA-87 signing shall run in deterministic mode in the simulation. | **T.** Two signatures over the same payload with the same key are byte-identical in simulation runs. |
| RSD-CRY-01g | Every AES-256-GCM IV shall be constructed deterministically by a fixed rule for its session type, so that no IV repeats under one key. | **T.** IV-rule unit tests for hop sessions, end-to-end sessions (including several attempts of one message) and swarm-key broadcasts: no IV repeats under one key (CLM-36). |
| RSD-CRY-02 | @G Key provider (a–e). |  |
| RSD-CRY-02a | The key provider shall never export a private key. | **S.** The key-provider interface exposes no operation that returns private-key material; the agent reaches keys only through this interface (AS2). |
| RSD-CRY-02b | The key provider shall refuse to sign the record types reserved for headquarters: roster, certificate, policy set and exclusion list. | **T.** Unit test: the provider refuses each headquarters-reserved type. E12 key-leak runs: a vehicle-signed reserved type appears only under the key-extracted profile and is then detected as G3 (M4). |
| RSD-CRY-02c | The key provider shall persist the head of the agent's record chain before it produces each signature. | **T.** Crash-restart test of GT-13: a restart produces no equivocation (no two records at one chain position). |
| RSD-CRY-02d | Before signing, the key provider shall copy the record into its own memory, validate the copy against the canonical encoding of the mission's single schema version, and sign exactly the validated copy. | **T.** GT-11: the provider refuses a canonically malformed record; the bytes signed equal the bytes validated. |
| RSD-CRY-02e | The canonical-encoding validator shall be generated from the same specification as the record decoders and be identical in every build. | **S, T.** Inspection of the build: the validator is generated from the decoder specification. GT-11: every build gives the same verdict on the same record. |
| RSD-CRY-03 | @G Discovery key, swarm key and key epochs (a–i). |  |
| RSD-CRY-03a | Every HELLO shall be authenticated with an HMAC under the collective-wide discovery key. | **T.** Unit test: a HELLO with an invalid HMAC is ignored; a valid one triggers session establishment. |
| RSD-CRY-03b | The discovery key shall remain constant for the whole mission. | **T.** GT-02: two components in different epochs reconnect after the old key's transition period, through HELLO under the unchanged discovery key and hop sessions, without a common swarm key. |
| RSD-CRY-03c | No identity, evidence or end-to-end confidentiality property shall depend on the swarm key, which provides only outer confidentiality for telemetry, digests and bundles. | **S, T.** Static check: no security, evidence or end-to-end path reads the swarm key. E12 epoch-failure runs: epoch failures change no security state (reported as a hypothesis). |
| RSD-CRY-03d | The new swarm key of an epoch shall be distributed over pairwise hop sessions and be accepted by a recipient only after it matches the key hash in the producer's signed epoch announcement. | **T.** GT-03: every non-quarantined member receives a key matching the announced hash and switches epoch. |
| RSD-CRY-03e | Every roster member that is not Quarantined, including vehicles in Watch, Restricted and Probation, shall receive the new swarm key. | **T.** GT-03: members in Watch and Restricted receive the new key and their permitted traffic continues. |
| RSD-CRY-03f | A recipient that receives a key that does not match the announced hash shall record a G1 finding against the hop peer that delivered it, not a proof against the producer. | **T.** GT-04: a relay that forwards a wrong epoch key receives a G1 finding; no G3 is raised against the honest producer. |
| RSD-CRY-03g | A recipient that has not received a valid key within two windows shall request it from the next candidate producer. | **T.** E12 key-withholding producer: recipients request the key from the next candidate after two windows and obtain it. |
| RSD-CRY-03h | When two epoch announcements share a parent epoch, every agent shall adopt the announcement of the producer with the smaller rank number and keep the other key for decryption only, for two windows. | **T.** E12 two-producer case: every agent adopts the smaller-rank announcement and keeps the other key for decryption only, for two windows. |
| RSD-CRY-03i | When two components merge, a new epoch shall be produced only if a G3 event occurred in either component since their common parent epoch. | **T.** Merge tests in E3 and E4: a new epoch appears exactly when a G3 event occurred in either component since the common parent epoch; otherwise the epoch of the smaller-rank producer continues. |
| RSD-CMC-04 | @G Session admission by boot quote (a–e). |  |
| RSD-CMC-04a | In handshake messages HS2 and HS3, each side shall include a quote, signed by its key provider's attestation key, over the boot measurement of the loaded agent image, the build identifier and the session nonce. | **T.** Unit test of HS2 and HS3: the quote fields are present and the quote verifies under the attestation key with the session nonce. |
| RSD-CMC-04b | The receiver shall end the handshake when the quote is missing or when the measurement is outside the reference set of that vehicle's build in the provisioning package. | **T, M.** Test with a changed stored image: after its next boot the vehicle's handshakes are refused (Persist stage). E13 reports refused sessions. |
| RSD-CMC-04c | A refused quote shall be recorded as a signed fact. | **T.** Unit test: each refusal produces one signed fact in the evidence store. |
| RSD-CMC-04d | A missing or mismatching quote shall not, by itself, lead to quarantine or revocation. | **T.** Unit test and E13 runs: no quote mismatch produces a G3, a quarantine or a revocation record. |
| RSD-CMC-04e | The attestation key shall sign only boot-measurement quotes whose measured value the agent cannot choose. | **S.** The attestation-key interface accepts no measured value from the agent; the measurement is taken by the provider. |

**Design rationale.**
- Deterministic signing keeps runs reproducible; a real vehicle would use hedged signing, the FIPS 204 default. Equivocation is detected on canonical content, not on signature bytes, so detection works in both modes.
- Under the oracle-constrained profile a compromised vehicle has a signing oracle, not the key (AS2). Every claim that rests on the key provider is therefore conditional on AS2, and the provider is a logical boundary in the simulation (LIM-32).
- A failure of the epoch mechanism can weaken outer confidentiality but never changes a security decision, because restrictions are enforced by honest receivers rather than by withholding the key. Epochs are per component (CLM-21), and the swarm key has no forward or post-compromise secrecy (CLM-23).
- A boot quote covers the stored image at boot, not the run-time state (LIM-27). Run-time control-flow attestation exists but is expensive for embedded systems [68], and it is future work (Section 6.6). A non-reference measurement may come from a storage or provisioning fault, so it ends the session but is never a proof.

RSD-CRY-02d, RSD-CRY-02e and RSD-CMC-04 belong to the common-mode containment design (Section 4.6). Signatures are verified on the output of the ingress, not inside it (RSD-CMC-01e, Section 4.6).

## 4.6 Cybersecurity and Resilience

Evidence against a vehicle is graded by the strengths of Table 4.15. A candidate reaches G2 only if at least two independent accusing roots exist and they outnumber the clearing roots, where a clearing root is an observer of the same stream or claim whose own evaluation ended normally. One liar therefore cannot raise an honest vehicle to G2, and one compromised observer cannot shield a vehicle that two honest observers accuse. Table 4.16 lists the responses and the minimum evidence strength each requires. A response must also be acceptable under every explanation of the evidence that has not been ruled out: H0 normal noise, H1 a fault of the vehicle itself, H2 the channel or the environment, H3 external electronic warfare, H4 a compromised vehicle, H5 a fault of shared infrastructure, H6 a false accusation, H7 a stale view of the state and H8 an unmodeled cause. Every hypothesis except H4 is innocent.

**Table 4.15. Evidence strengths G0–G3.**

| Strength | Meaning |
|---|---|
| G0 Unattributed | Cannot be tied to the named vehicle, for example an authentication failure |
| G1 Single root | One independent root |
| G2 Independent support | At least two independent accusing roots, more than the clearing roots |
| G3 Proof | Self-proving: an equivocation, a forbidden signed type, an invalid revocation record or a canonically malformed signed record |

**Table 4.16. Responses and their preconditions.**

| Response | Minimum strength | Acceptable under innocent hypotheses | Under compromise |
|---|---|---|---|
| L1 Reject a message | Any | Acceptable | Sufficient for that message only |
| L2 Restrict a sensor channel | G1 (Flagged), G2 (Limited) | Acceptable: the data is kept, only its weight changes | Sufficient for false data |
| L3 Restrict a scope (state Restricted) | G2 | Acceptable under H1, H5 and H8; H0, H2, H3, H6 and H7 must be ruled out first | Usually sufficient for network and mission behavior |
| L4 Quarantine | G3 | Not acceptable under any innocent hypothesis; used only after a proof rules them all out | Sufficient |

### Evidence strength and response selection

**Table 4.17. Evidence strength and response requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-SEC-01 | @G Graded evidence; quarantine only with proof (a–f). |  |
| RSD-SEC-01a | Each candidate shall be graded G0, G1, G2 or G3 by the definitions of Table 4.15, within the strength cap of its metric. | **T.** Unit tests per metric with scripted findings: no metric produces a strength above its cap. |
| RSD-SEC-01b | A candidate shall reach G2 only if at least two independent accusing roots exist and they outnumber the clearing roots. | **T.** Unit tests: one accusing root gives at most G1; two independent accusing roots with fewer clearing roots give G2; two accusing roots against two clearing roots do not. |
| RSD-SEC-01c | A vehicle shall enter the Quarantined state only on a G3 proof. | **T, A.** Run-time check in every run: every Quarantined decision lists a G3 record among its evidence. UPPAAL property 1 holds. E7 and E12: wrongful quarantine = 0 (must be 0). |
| RSD-SEC-01d | Only an equivocation, a record of a type that its signing key may never sign (a headquarters-only type under a vehicle key, an operator type under a vehicle key, or a vehicle type under an operator key), an invalid revocation record or a canonically malformed signed record shall constitute G3 evidence. | **T.** Unit tests: each of the four proof types yields G3; no other evidence combination does. GT-11 covers the canonically malformed record. |
| RSD-SEC-01e | An honest agent shall not quarantine or revoke a vehicle without a G3 proof made of that vehicle's own signatures, whatever the number of compromised vehicles. | **T, M.** Unit test: evidence that contains no record signed by the accused key yields no G3 proof against it. E13, every run: quarantines or revocations of vehicles whose key and key provider are intact = 0 (must be 0; CLM-51). |
| RSD-SEC-01f | No log entry, summary, console view or display label shall state that a vehicle is compromised unless a G3 proof against that vehicle exists. | **T.** Log scan of every E13 run and of GT-12: statements of compromise without a G3 proof = 0 (must be 0). |
| RSD-SEC-02 | @G Least intrusive sufficient response (a–e). |  |
| RSD-SEC-02a | For each candidate, the evaluation shall select the lightest response among L1 to L4 that is sufficient against the harm the candidate describes and acceptable under every open hypothesis. | **T.** Unit tests over scripted candidates and open-hypothesis sets: the selected response equals the lightest response of Table 4.16 that meets both conditions. |
| RSD-SEC-02b | When no response is both sufficient and acceptable, the evaluation shall apply the heaviest acceptable response and raise a verification need. | **T.** Unit test: a candidate for which no response is both sufficient and acceptable receives the heaviest acceptable response and produces a verification need. |
| RSD-SEC-02c | Findings of the link-claim metric M15 and the path-integrity metric M17 shall never exceed strength G2. | **T.** Unit tests: M15 and M17 findings never exceed G2. GT-01 passes. |
| RSD-SEC-02d | No candidate based on M15 or M17 evidence shall lead to the Quarantined state. | **A, T.** UPPAAL property 2: no path from an M15- or M17-based candidate to Quarantined. GT-07: two accusers of an honest cut vertex cause a restriction, never a quarantine. |
| RSD-SEC-02e | The geographic-leash metric M16 shall produce records only, without changing any vehicle state. | **T.** Unit test and E8 runs: M16 records change no vehicle state. |
| RSD-SEC-04 | @G Taint after a proof (a–c). |  |
| RSD-SEC-04a | An agent that holds a G3 proof against a vehicle shall treat every record of that vehicle from mission start as tainted. | **T, M.** Unit test: after a proof, all of the vehicle's records from mission start carry weight zero. E12 reports taint-caused status changes; under f = 1 a Confirmed contact falls at most to Corroborated (CLM-34). |
| RSD-SEC-04b | Taint shall be derived locally from the evidence and never be transmitted as a record. | **S.** No record type carries taint. |
| RSD-SEC-04c | When a sensor channel becomes Limited, the agent shall treat that channel's records as tainted from the start of the incident minus 60 s. | **T.** Unit test: a channel that becomes Limited taints its records from 60 s before the incident start; the records that caused the restriction stay untainted (RSD-EVD-05e). |
| RSD-SEC-05 | @G M17 attribution keeps the source in the suspect set (a–e). |  |
| RSD-SEC-05a | The suspect set of a corrupted copy shall contain the source and every forwarder on the path of the attempt that carried it. | **T.** GT-01: the source is in every suspect set computed from a corrupted copy. |
| RSD-SEC-05b | The source shall remain in the suspect set until a signed first-hop receipt showing a correct copy clears it. | **T.** MD12 unit test: corrupted copies on two disjoint paths give a finding against the source unless a signed first-hop receipt clears it. |
| RSD-SEC-05c | A forwarder shall be named as suspect only after the source has been cleared, from the intersection of the forwarder sets of at least two corrupted copies on different but overlapping paths, minus every forwarder that carried an intact copy of the same flow in the same incident. | **T.** GT-01: on overlapping paths with a clearing first-hop receipt, the common forwarder is named, and every branch of the attribution rule is reached by at least one case. |
| RSD-SEC-05d | When more than one suspect remains, the result shall be recorded as ambiguous, with no change of state. | **T.** Unit test: an ambiguous attribution produces a record and no state change. |
| RSD-SEC-05e | An M17 finding shall reach G2 only with findings from two different destinations, on two different flows, against the same suspect within one incident. | **T, M.** Unit test: findings from one destination stay G1. E8 reports attribution correctness in f = 1 runs against its pre-registered threshold. |
| RSD-SEC-06 | @G Reversible restriction under the vehicle security state machine (a–h). |  |
| RSD-SEC-06a | The vehicle security state machine shall implement the transitions shown in Fig. 3.2(a) with the initial parameter values. | **T, A.** State-machine unit tests of every transition; the C++ table and the UPPAAL model are generated from one configuration file. |
| RSD-SEC-06b | A Restricted vehicle shall return to Watch at once, without penalty and without the restriction being counted, when the basis of its restriction is withdrawn. | **T.** Unit test: when the accuser becomes tainted, the vehicle returns to Watch at once and its restriction count is unchanged. |
| RSD-SEC-06c | A Restricted vehicle shall leave the Restricted state only through withdrawal of the basis, through Probation or on a proof. | **A.** UPPAAL property 4 holds. |
| RSD-SEC-06d | The n-th restriction of a vehicle in a mission shall last at least T0 · 2^(n−1), with T0 = 60 s, before the vehicle may enter Probation. | **T.** Unit test: the first restriction lasts at least 60 s and the second at least 120 s before Probation. |
| RSD-SEC-06e | A third restriction in a mission shall keep the vehicle Restricted for the rest of the mission. | **T.** Unit test: a third restriction never leads to Probation. |
| RSD-SEC-06f | A vehicle in Probation shall hold no role or contract. | **A, T.** UPPAAL property 6; unit test: a vehicle in Probation wins no task and holds no contract. |
| RSD-SEC-06g | A vehicle's security state shall rise by at most one step per window, except on a proof. | **A, T.** UPPAAL property 5; run-time check in every run. |
| RSD-SEC-06h | The Quarantined state shall be absorbing for the rest of the mission. | **A.** UPPAAL property 3 holds. |

### Common-mode containment

**Table 4.18. Common-mode containment requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-CMC-01 | @G Ingress boundary (a–g). |  |
| RSD-CMC-01a | Every network parser of the agent, except the radio firmware and the LSC beacon parser, shall run behind one ingress boundary. | **S.** Static dependency check: every network parser except the radio stack and the beacon parser belongs to the ingress target. |
| RSD-CMC-01b | The ingress boundary shall hold no private-key operation and no actuator or waypoint interface. | **S, T.** Static check: the ingress classes have no reference to the signing interface or the waypoint interface. GT-11 passes. The E13 stage-II cells model a broken AS19: the exploited agent acts under the oracle-constrained profile and obtains no key (AS2). |
| RSD-CMC-01c | Every ingress parser shall enforce fixed size limits on its input. | **T.** Fuzzing of every ingress parser and decoder: no input beyond the size limit is accepted and no crash is recorded. |
| RSD-CMC-01d | The ingress boundary shall hold no transmit interface. | **S.** Static dependency check: the ingress classes have no reference to the signing, waypoint or transmit interfaces (0 references). |
| RSD-CMC-01e | Every signature on a received record, bundle or handshake shall be verified by M-CRY (I6) on the canonical bytes that leave the ingress, not inside the ingress. | **S, T.** Static check: the ingress classes contain no signature-verification call. GT-11e: a forged bundle that the implanted ingress marks as valid is rejected by M-CRY. |
| RSD-CMC-01f | The agent shall accept no record whose verification result did not come from M-CRY. | **T.** GT-11e: the forged bundle is not accepted; accepted records without an M-CRY verification result = 0 (must be 0). |
| RSD-CMC-01g | A compromised ingress (stage I) shall cause no effect other than dropping, delaying or reordering its own vehicle's inputs. | **S, T.** Static checks of RSD-CMC-01b and RSD-CMC-01d. GT-11e, run with deterministic signing (RSD-CRY-01f): the ingress issues no transmission, signature request or waypoint. An offline replay without a key provider, fed the logged M-CRY-verified inputs with their arrival windows, the vehicle's own sensor and clock inputs and the processing-delay table, reproduces every canonical signed payload byte for byte and every security-relevant decision window by window, and every logged signature verifies over its replayed payload. Signature bytes are not compared, and inspection of the replay tool confirms that it never signs. |
| RSD-CMC-02 | @G Canonically malformed signed record as proof (a–c). |  |
| RSD-CMC-02a | A signed record that violates the canonical encoding of the mission's schema version shall be treated as a G3 proof that the key was used outside the key provider or that the provider is faulty. | **T.** GT-11: a key-extracted attacker's malformed signed record yields a G3 proof at every receiver. |
| RSD-CMC-02b | Every build shall give the same verdict on the same record. | **T.** GT-11: every build gives the same verdict on the same record. |
| RSD-CMC-02c | A record that carries a schema version other than the mission's shall be rejected without forming a proof. | **T.** GT-11: a record with another schema version is rejected without a proof. |
| RSD-CMC-05 | @G One agent build per root class (a–e). |  |
| RSD-CMC-05a | No agent build shall span two root classes for object claims. | **S.** Static check of the provisioning package: no build identifier maps to vehicles of two root classes. |
| RSD-CMC-05b | The reference configuration shall use three agent builds from the same source: build E for the EO-X vehicles, build I for the IR-Y vehicles and the command ship, and build R for the radar-Z vehicles. | **I.** Provisioning package: build references are keyed by E, I and R, each with the reference boot measurement of its build, and every vehicle carries a build tag. |
| RSD-CMC-05c | Builds shall differ only in memory layout, so that every build derives the same outputs from the same evidence. | **T.** Three-build runs: equal evidence hashes imply equal state hashes across builds (MD8). |
| RSD-CMC-05d | Experiment E13 shall model cross-build exploit transferability as the scenario parameter px with the values 0, 0.1, 0.5 and 1. | **I.** E13 configuration: px takes the four values. |
| RSD-CMC-05e | Every E13 result on spread and mission effect shall be reported as a function of px, with the statement that px is a synthetic parameter. | **I.** Report review: every E13 figure and table labels px and states that it is synthetic (LIM-44). |
| RSD-CMC-06 | @G Fusion bound under a build-confined compromise (a–b). |  |
| RSD-CMC-06a | When every compromised build lies in one root class, every Corroborated status at an honest agent shall rest on at least one supporting root class outside that class. | **T, M.** GT-11d1: implanted EO-X agents alone leave the contact Reported at every honest agent. GT-11d2: with one honest IR-Y detection added, the status equals Table 4.7 applied to the clipped values. E13 provenance check in every run with the per-class build map: Corroborated statuses without a supporting class outside the compromised class = 0 (must be 0). |
| RSD-CMC-06b | When every compromised build lies in one root class, every Confirmed status at an honest agent shall rest on at least two supporting root classes outside that class. | **T, M.** GT-11d3: with an honest radar-Z detection added, the contact is Confirmed exactly when L ≥ b and two modalities support it. GT-11d4 uses the per-group build map with all three classes of sector A implanted, and the contact reaches Confirmed, which shows why RSD-CMC-05a is needed. E13 provenance check: Confirmed statuses with fewer than two supporting classes outside the compromised class = 0 (must be 0); false verification rates are reported against matched f = 0 runs, with no zero target (CLM-49). |
| RSD-CMC-07 | @G Command and receipt validity under a compromise of the gateway build (a–c). |  |
| RSD-CMC-07a | While build R is compromised, no vehicle shall apply a forged, replayed, expired or out-of-validity operator command. | **T.** E13 gateway cell GW-1 (build R compromised, all supervision traffic dropped from 900 s, command ship not a gateway): such applications = 0 in every run (must be 0). |
| RSD-CMC-07b | While build R is compromised, no honest vehicle shall release a held copy on a GCS receipt that fails verification. | **T.** E13 cell GW-1: copies released on an invalid receipt = 0 in every run (must be 0). |
| RSD-CMC-07c | While build R is compromised, the console shall show no vehicle age younger than the age computed from the signed window of the vehicle's newest summary. | **T.** E13 cell GW-1: console ages younger than their signed windows = 0 in every run (must be 0). |

**Design rationale.**
- A signature proves misbehavior only for self-contradiction, a type the key may never sign, a revocation record whose embedded proof fails and a record the provider would have refused. All other evidence is weighed by independent roots and can at most restrict. Link-claim and path-integrity evidence is capped at G2 because two colluders could make an honest vehicle look guilty; such an attack can cost a scope, never the vehicle's identity (LIM-06).
- A proof shows that a key was misused by a certain time, not when the compromise began, so all earlier records are suspect; under f = 1 the fusion thresholds limit the cost to one class per contact (LIM-25).
- If AS19 holds, an exploited parser can only drop, delay or reorder its own vehicle's inputs (stage I). If AS19 breaks, the attacker reaches the agent and acts under the oracle-constrained profile (stage II); the key-extracted profile needs a broken AS2 as well (stage III). Build diversity changes memory layout, not logic [49], and even layout diversity can fail: low-entropy randomization can be brute-forced [69], and a single memory disclosure lets an attacker rebuild a code-reuse exploit at run time [70]. Vulnerability records can show that two systems share few known flaws, but they cannot prove that the systems are independent [71]. Cross-build transferability px is therefore a scenario parameter, and E13 reports results as a function of px rather than measuring real exploit success. Radio firmware and the operating-system network stack are outside the architecture (LIM-35).
- One build per root class keeps a build-specific exploit inside one class, which is the case the fusion thresholds bound (RSD-CMC-06, CLM-49). With one build per group, such an exploit controls every class of a sector and can produce Confirmed, as GT-11d4 shows. The price is that the three gateway USVs share build R (LIM-45).

## 4.7 Safety and Fault Tolerance

Table 4.19 gives the degradation levels under jamming and what still works at each level.

**Table 4.19. Degradation levels under jamming.**

| Level | Condition | What still works |
|---|---|---|
| J0 | No jamming indicator | Everything |
| J1 | Indicator raised; links remain | Mesh and evidence in the priority order of RSD-JAM-01e; telemetry reduced to 0.2 Hz; repair positions inside the suspected area excluded |
| J2 | Group cut from the other groups (Connection = Intra-group) | Group autonomy: fusion, security and allocation per component; revocation per component |
| J3 | Vehicle Alone with the indicator | Individual operation: own sensing, signed records stored, LSC envelope; procedural separation when beacons are lost; lost-link rule |
| J4 | J3 and GNSS denied | As J3; position source switched to inertial or radar navigation, GNSS class flagged; altitude bands stay barometric |

**Table 4.20. Safety and degraded-operation requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-SAF-01 | @G Local safety in every state (a–f). |  |
| RSD-SAF-01a | The LSC shall be active in every mission state and every security state. | **T.** MD10 test: collision avoidance runs in every mission state and every security state. |
| RSD-SAF-01b | No security or mission decision shall disable the LSC. | **S, T.** Static check: no interface from M-SEC or M-MIS can disable the LSC. GT-13 runs the LSC in every mission and security state. |
| RSD-SAF-01c | A vehicle shall not move to a waypoint of a movement-producing mission state (Observation with a detour, Repair, Support, Return or rally) before the LSC has accepted that waypoint. | **T.** Run-time check in every run: every Repair and Support entry, and every movement entry, references an LSC acceptance record; zero waypoints are executed without acceptance. |
| RSD-SAF-01d | When the LSC rejects a proposed waypoint, the mission state machine shall return to its previous state and record the rejection reason. | **T.** SCN-CR-01 variant CR-d: the LSC rejection returns the mission state machine to its previous state with the reason recorded. |
| RSD-SAF-01e | Safety messages from any vehicle shall be accepted in every security state and passed to collision avoidance only as statements about the sender's own position and intent. | **A, T.** UPPAAL property 7 holds. E12 false-safety-warning runs: false warnings change only the separation from the sender (separation changes reported). |
| RSD-SAF-01f | The position of a Restricted, Quarantined or isolated vehicle shall remain an input to every vehicle's collision avoidance, with enlarged uncertainty once the vehicle is Restricted or worse. | **T.** Unit test: a Quarantined vehicle's telemetry position remains in every collision-avoidance input with enlarged uncertainty. |
| RSD-SAF-03 | @G LSC envelope, liveness protection and procedural separation (a–l). |  |
| RSD-SAF-03a | When no heartbeat from the mission computer has arrived for 5 s, the LSC shall hold the vehicle in place with collision avoidance active (stage 1). | **T.** GT-13: when the mission computer stops in every mission and security state, the vehicle holds within 5 s. |
| RSD-SAF-03b | After 120 s of heartbeat silence, or after a third unplanned silence in the mission, the LSC shall return the vehicle along the deconflicted route of the separation plan (stage 2). | **T.** GT-13: stage 2 follows after 120 s of silence and after a third unplanned silence. |
| RSD-SAF-03c | The LSC shall reject every waypoint that violates separation, geofence and no-go zones, energy reserve, kinematic limits or sea-state limits, whether the mission computer is honest, faulty or compromised. | **T, M.** GT-13: with the heartbeat kept alive, every out-of-envelope waypoint is rejected. E13 and E15: collisions in the simulated runs = 0 (a simulation test criterion under modeled sensors, not a physical-safety guarantee; CLM-42). |
| RSD-SAF-03d | Each LSC shall broadcast its own fixed-length beacon with its position at 1 Hz, independently of the mission computer. | **T.** Unit test: each LSC emits one 24-byte beacon per second while the mission computer is stopped. |
| RSD-SAF-03e | For separation, the LSC shall give its own sensors precedence over beacons and over position telemetry passed on by the mission computer. | **T.** Unit test: when own-sensor data and a beacon disagree, separation uses the own-sensor position. |
| RSD-SAF-03f | A beacon whose position is kinematically impossible given earlier reports for the same identifier shall mark that identifier as conflicted, with both positions treated as occupied. | **T.** GT-13: a beacon with an impossible jump for a real identifier marks it conflicted and keeps both positions occupied. |
| RSD-SAF-03g | A contact known only from beacons shall not cause the vehicle to leave its pre-assigned procedural slot. | **T.** GT-13: a beacon-only contact never moves the vehicle out of its slot. |
| RSD-SAF-03h | The LSC shall keep a beacon-only track for 5 s after its last frame, in a table of at most 64 beacon-only tracks. | **T.** Unit test: a beacon-only track expires 5 s after its last frame; the table holds at most 64 tracks. |
| RSD-SAF-03i | The LSC shall switch to procedural separation within 5 s when no beacon of a neighbor is younger than 5 s or when the table of beacon-only tracks is full. | **T.** GT-13: with beacons suppressed, procedural separation engages within 5 s of the last beacon; a flood beyond 64 beacon-only tracks also switches to procedural separation. |
| RSD-SAF-03j | Under procedural separation, each UAV shall move to its pre-assigned altitude band, measured barometrically, while each USV holds its position, relying on its own radar with a wider margin. | **T.** Unit test: under procedural separation each UAV commands its pre-assigned barometric altitude band and each USV holds position. |
| RSD-SAF-03k | The agent shall restart during the mission only after a crash, and not periodically. | **S.** The agent and scenario configuration contain no periodic restart; restarts occur only on a crash event. |
| RSD-SAF-03l | A restart after a crash shall reload the read-only image and resume from the chain head that the key provider persisted before its last signature. | **T.** GT-13: a crash restart produces no equivocation. |
| RSD-JAM-01 | @G Jamming indicator, degradation levels and transmission priority (a–g). |  |
| RSD-JAM-01a | Communication & Routing shall raise the jamming indicator when the frame-reception error rate is at least 30 % for two windows, or when every neighbor whose link the shadow map and the last positions predict is lost. | **T.** Unit test with injected reception errors and predicted-neighbor loss: the indicator rises exactly when either condition holds. |
| RSD-JAM-01b | The jamming indicator shall never produce a finding against a vehicle. | **T, M.** E15: anomaly records = 0 in every run (must be 0). |
| RSD-JAM-01c | Each vehicle shall derive its degradation level J0 to J4 from its Connection FSM state and its jamming indicator by the conditions of Table 4.19. | **T, M.** Unit test of the level derivation over every combination of Connection state, indicator and GNSS status. E15 reports time per level. |
| RSD-JAM-01d | Under level J1, the vehicle shall reduce its telemetry rate to 0.2 Hz. | **T.** Unit test: under J1 the telemetry period is 5 s. |
| RSD-JAM-01e | Under reduced capacity, every vehicle shall transmit in the fixed order safety messages and LSC beacons; proofs and revocation records; operator commands and receipts; Group Status Summary; C2 traffic; telemetry; bundles; pull repair, with the evidence-plane cap still applied. | **T.** Unit test of the transmission queue with all eight classes present: the order of RSD-JAM-01e holds and the evidence-plane cap still applies. |
| RSD-JAM-01f | Connectivity-repair candidate positions inside a suspected jamming area shall be excluded. | **T.** Unit test: candidate repair positions inside a suspected jamming area are excluded. |
| RSD-JAM-01g | Under level J4, the position source shall switch to inertial or radar navigation, with the GNSS class flagged. | **T.** Unit test: under J4 the position source changes and the GNSS class is flagged; altitude bands remain barometric. |
| RSD-JAM-02 | @G Lost-link rule (a–d). |  |
| RSD-JAM-02a | A vehicle that has been Alone with the jamming indicator raised for Tll = 300 s shall finish its current task and then take a rally task to its group's rally point from the mission plan. | **T, M.** Unit test: after 300 s Alone with the indicator, the vehicle completes its task and creates a rally task. E15 reports rally outcomes. |
| RSD-JAM-02b | A rally task shall not change the mission state to Return. | **A.** Model check (MD4): no rally transition leads to Return. |
| RSD-JAM-02c | A vehicle on a rally task shall resume normal planning when it verifies a neighbor again. | **T.** Unit test: a verified neighbor ends the rally and normal planning resumes. |
| RSD-JAM-02d | When a rally fails, the vehicle shall hold at its best position estimate. | **T.** Unit test with the rally point jammed: the vehicle holds at its best position estimate. |

**Design rationale.**
- The two-stage fail-safe is used instead of periodic restarts. A restart from a read-only image would remove a memory-resident implant, as proactive recovery does by periodic rejuvenation, but such recovery needs enough healthy replicas and enough time [72]. In a mission as short as the 30-minute reference mission, an attacker whose entry path stays open re-infects within seconds, and each restart costs an outage; periodic restart for long missions is future work (Section 6.6).
- LSC beacons are not authenticated. A forger can keep a ghost alive while transmitting, exhaust the track table and mislead the LSC about a vehicle whose real position it never received (LIM-36). Beacon authentication is future work.
- Recovery after jamming is stated in three layers. It is guaranteed that a jam never produces an accusation and that signed records are kept up to the store quota. Under AS4, the records reach the collective if the vehicle survives and contact returns in time; this is a conditional statement. Re-convergence within the drain time T_drain of Section 4.10 is a hypothesis tested in E15. The jamming model omits carrier-sense blocking, reactive jamming and near-far effects (LIM-33), and under total jamming the collective mechanisms stop (LIM-31).

RSD-SAF-02f requires the LSC's own source tree and build job; bounded model checking of the beacon decoder is an optional item (Table 6.7).

## 4.8 Human-on-the-Loop and National SATCOM

Four concepts are kept separate. Command validity is whether a record is authentic, authorized and well formed (RSD-HOT-02, RSD-HOT-06). Command delivery is whether it arrives, which is never guaranteed (RSD-HOT-08). Command expiration is whether it arrives in time: an immediate ABORT and a RESUME carry a validity, and every HOLD ends by itself (RSD-HOT-01). Abort behavior is what an applied ABORT does: it enters the mission state machine toward Return like any other Return condition and does not bypass the LSC.

**Table 4.21. Supervision states at the console.**

| State | Age of the vehicle's newest summary, from its signed window | What the operator can expect |
|---|---|---|
| Live | ≤ 2 summary periods (60 s) | Commands normally arrive within seconds; preconditions usually still hold |
| Delayed | > 2 and ≤ 20 summary periods (600 s) | Commands may arrive late; a RESUME may become void |
| Lost | > 20 summary periods | Commands wait as bundles; standing orders and HOLD end times decide until contact returns |

### Command authority, validity and application

**Table 4.22. Command authority, validity and application requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-HOT-01 | @G Bounded command authority and present-only application (a–m). |  |
| RSD-HOT-01a | No operator command shall change a security state, the roster, a key, the policy or a threshold. | **T, M.** GT-12: no command, valid or invalid, changes a security state, the roster, a key, the policy or a threshold. E14: security-state changes caused by commands = 0. |
| RSD-HOT-01b | Command handling shall have no write interface to Distributed Security or to Cryptographic Identity & Trust. | **S.** Static check: no write interface leads from command handling to M-SEC or M-CRY (MD18). |
| RSD-HOT-01c | A vehicle shall decide the physical application of each command at each window close only from its own present window and its present evidence. | **T, M.** GT-12f; E14: applications outside validity or with a false present precondition = 0 (must be 0). |
| RSD-HOT-01d | Another vehicle's APPLIED receipt shall not be an input to a vehicle's decision to apply a command. | **T.** GT-12f: component B voids a RESUME received after its validity and stays held until its HOLD ends, despite component A's APPLIED receipts. |
| RSD-HOT-01e | A recomputed command history shall not authorize a movement. | **T.** GT-12f: after a merge, the recomputed history causes no new movement at any vehicle. |
| RSD-HOT-01f | Vehicles that hold the same command records shall derive the same command history. | **T, M.** GT-12f: after the merge the command history is identical in A and B. E14: command-history divergence after merge = 0; physical-state differences are logged and reported, not judged. |
| RSD-HOT-01g | A HOLD-type record shall be applied only if it is newer than the last HOLD-type record the vehicle applied, ordered by issue window, then by longer duration, then by console identifier. | **T.** GT-12: HOLD and RESUME from two consoles in both arrival orders give each vehicle the same physical result; an older HOLD arriving after a newer applied RESUME changes nothing. |
| RSD-HOT-01h | A HOLD shall be applied only while the present window lies between its issue window and its issue window plus its duration. | **T.** Unit test: a HOLD stops applying at issue window plus duration without any further record. |
| RSD-HOT-01i | A RESUME shall be applied only within its validity, only if its precondition holds on the vehicle's present evidence, and only if it is newer than the last HOLD-type record the vehicle applied. | **T.** GT-12: a RESUME without precondition is rejected; a RESUME received after its validity, or with a false precondition, is void and the earlier HOLD keeps its end time. |
| RSD-HOT-01j | An immediate ABORT shall be applied only if it reaches the vehicle within its validity. | **T.** GT-12f: a late immediate ABORT is void. |
| RSD-HOT-01k | A recall ABORT shall be applied whenever it reaches the vehicle within the same mission. | **T.** GT-12f: a late recall ABORT is applied. |
| RSD-HOT-01l | An applied ABORT shall not be undone during the mission. | **T.** Unit test: a HOLD or RESUME after an applied ABORT does not leave the Return state. |
| RSD-HOT-01m | Each vehicle shall report APPLIED, VOID or REJECTED for every command it receives, in a receipt carried in its next bundle. | **T.** Unit test: every received command produces exactly one receipt with APPLIED, VOID or REJECTED in the vehicle's next bundle. |
| RSD-HOT-02 | @G Closed command set (a–h). |  |
| RSD-HOT-02a | The operator command set shall consist exactly of ABORT, HOLD, RESUME, PRIORITY and SUPPORT. | **S, T.** The command record schema admits exactly five types; a record of any other type is rejected. |
| RSD-HOT-02b | An ABORT, HOLD or RESUME record shall require two operator signatures. | **T.** GT-12: an ABORT or HOLD with one signature is rejected. |
| RSD-HOT-02c | A PRIORITY or SUPPORT record shall require one operator signature. | **T.** Unit test: a PRIORITY or SUPPORT record with one valid operator signature is accepted. |
| RSD-HOT-02d | Every command record shall carry the mission identifier, which is the mission manifest hash. | **T.** GT-12: a command of another mission is rejected. |
| RSD-HOT-02e | Every HOLD record shall carry an explicit duration, by default 600 s and at most 1800 s. | **T.** Unit test: a HOLD without a duration, or with a duration above 1800 s, is rejected. |
| RSD-HOT-02f | Every RESUME record shall carry a precondition from the closed set of predicates over the evidence and a validity, by default 120 s. | **T.** GT-12: a RESUME without a precondition is rejected. |
| RSD-HOT-02g | The console shall present the immediate and the recall form of ABORT as separate, explicitly labeled choices. | **I.** Console inspection: both ABORT forms appear as separately labeled choices and are shown in the command list, in the receipts and in the supervision layer. |
| RSD-HOT-02h | A vehicle key that signs an operator command type, or an operator key that signs a vehicle record type, shall be treated as a G3 proof (M4). | **T.** Unit test: a vehicle-key signature on an operator type yields a G3 proof against the key's owner. |
| RSD-HOT-06 | @G Every vehicle validates every command itself (a–h). |  |
| RSD-HOT-06a† | Every vehicle shall validate every operator command record and every GCS receipt itself, by the same rules whatever path delivered it. | **T.** GT-14 (a)–(h): commands injected on a backhaul are validated exactly as commands from a neighbor. |
| RSD-HOT-06b† | A vehicle shall ignore a command record whose mission identifier differs from its own mission. | **T.** GT-14 (c): a valid command of another mission is not applied. |
| RSD-HOT-06c† | A vehicle shall ignore a command record that lacks the required number of valid signatures from distinct operator certificates of the mission's provisioning package, each over the canonical encoding of every other field. | **T.** GT-12: a command with one field changed after signing, and a two-signature command signed twice with one operator key, are not applied and are logged. GT-14 (a), (b). |
| RSD-HOT-06d† | A vehicle shall treat a command signature made with the GCS service key as invalid. | **T.** GT-14 (h): a command signed with the GCS service key is not applied. |
| RSD-HOT-06e† | A vehicle shall ignore a command record whose target does not include it. | **T.** Unit test: a command whose target excludes the vehicle is not applied by it. |
| RSD-HOT-06f† | A vehicle shall ignore a command record whose issue window is more than two windows in the future. | **T.** GT-14 (d): a command dated more than two windows ahead is not applied. |
| RSD-HOT-06g† | A copy that carries the same command identifier and content hash as a command already applied shall not be applied again. | **T.** GT-14 (e) and GT-12g: a valid command arriving again, through two gateways and from the command ship, is applied once. |
| RSD-HOT-06h† | A command or GCS receipt that fails validation shall be rejected without creating any proof against the vehicle or gateway that forwarded it. | **T.** GT-14: each rejected input is logged at the rejecting node and creates no proof against the forwarding vehicle (E14: 0 proofs against forwarders). |

### Upward status reporting

**Table 4.23. Status reporting requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-HOT-03 | @G Group Status Summary with per-vehicle freshness (a–j). |  |
| RSD-HOT-03a | The producer of each connected component shall publish a Group Status Summary for the members of its group in that component every 6 windows (30 s). | **T, M.** Unit test: one summary per component producer every six windows. E14 reports summary delivery and age distributions per cell. |
| RSD-HOT-03b | The content of a Group Status Summary shall be a deterministic function of the component's evidence, with a fixed set of fields that includes the compact positions of RSD-HOT-03j. | **T.** Replay test: the same component evidence yields byte-identical summary content. |
| RSD-HOT-03c | A Group Status Summary shall travel inside the producer's next signed bundle (I20). | **T.** Unit test: each summary is contained in a signed bundle of its producer and verifies under that bundle's signature. |
| RSD-HOT-03d | The console shall keep the newest summary per group and producer, so that a summary of one component never replaces that of another. | **T.** Partition test with group A split into two components: both producers' summaries are retained. |
| RSD-HOT-03e | The console shall show two summaries of one group side by side without merging them. | **I.** Console inspection in the same partition test: both summaries are shown side by side. |
| RSD-HOT-03f | The console shall take each vehicle's age from the newest summary that lists it, showing a vehicle that appears in no summary younger than 600 s as unaccounted, with the time it was last listed. | **T.** Unit test: a vehicle absent from every summary younger than 600 s is shown as unaccounted with its last-listed time. |
| RSD-HOT-03g | The console shall classify each vehicle as Live, Delayed or Lost by the bounds of Table 4.21. | **T.** Unit test of the classification at the bounds 2 and 20 summary periods; no vehicle behavior depends on the console state (static check: no interface from the console state to any vehicle). |
| RSD-HOT-03h† | Summary ages shall be computed from the summary's signed window, never from its arrival time. | **T.** GT-14 (headquarters cases): a valid old summary sent again keeps its original age and does not replace a newer one; displayed age equals signed age. |
| RSD-HOT-03i† | A summary dated more than two windows ahead of the receiver's time shall be rejected. | **T.** GT-14: a summary dated more than two windows ahead is rejected and not shown as data. |
| RSD-HOT-03j† | Every Group Status Summary shall carry compact positions of its covered members: vehicle identifier, position, altitude and time of fix. | **I, T.** Schema inspection of the summary record; unit test of the field contents. |

### Designated gateways, backhaul and headquarters

**Table 4.24. Gateway, backhaul and headquarters requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-HOT-04 | @G Headquarters is an endpoint; backhaul is untrusted transport (a–f). |  |
| RSD-HOT-04a† | Neither headquarters nor any backhaul link shall appear as a vertex or an edge in any vehicle's Network Graph. | **S, T.** Static check: no Network Graph vertex for headquarters and no backhaul edge. GT-14 passes. |
| RSD-HOT-04b† | The link-state advertisement shall carry no backhaul information. | **S.** GT-15 (a): the link-state payload has no backhaul field. |
| RSD-HOT-04c† | A vehicle shall have a backhaul only if the signed mission plan names it as a designated gateway. | **I, T.** Scenario inspection: only listed gateways have a point-to-point link. GT-14 (j): no copy is addressed to a vehicle outside the designated-gateway list. |
| RSD-HOT-04d† | A designated gateway shall inject into the mesh only operator command records and GCS receipts received over its backhaul, dropping and logging every other record. | **T.** GT-14 (f): a record that is neither an operator command nor a GCS receipt is dropped and logged by the gateway. |
| RSD-HOT-04e† | A designated gateway shall forward no vehicle-to-vehicle traffic over its backhaul. | **T.** GT-14 (j): no packet with a vehicle source and a vehicle destination crosses a point-to-point device (count 0). |
| RSD-HOT-04f† | IP forwarding shall be disabled on the GCS node and on every vehicle node. | **I, T.** Node configuration check; a run-time check confirms the isolation. |
| RSD-HOT-05 | @G Redundant copies and release by verified GCS receipt (a–k). |  |
| RSD-HOT-05a† | A vehicle shall send each record addressed to headquarters as one copy to each designated gateway reachable in its own current Network Graph view. | **T.** GT-14 (i): one summary record arrives as three copies through three gateways. |
| RSD-HOT-05b† | A vehicle shall send no copy to a designated gateway that is unreachable in its view or that it holds as Quarantined. | **T.** GT-14 (k): a gateway Quarantined in a sender's view receives no copy from that sender (count 0). |
| RSD-HOT-05c† | Each record for headquarters shall be sealed once to the GCS service key, so that a gateway can neither read nor alter it. | **T.** Unit test: a gateway cannot decrypt a sealed record, and a modified copy fails verification at the GCS. |
| RSD-HOT-05d† | The GCS application shall accept a record as data only if it decrypts, its signature verifies under a vehicle certificate of the mission's roster, its record chain, Merkle root and canonical encoding are intact, its producer and every listed vehicle are roster members of the group the summary is for, its window is not more than two windows ahead of headquarters time, and its record identifier has not been accepted before. | **T.** GT-14 headquarters cases: a summary with one modified byte, a summary signed outside the roster, a summary listing a vehicle of another group and a future-dated summary are not shown as data. E14: invalid reports shown as data = 0. |
| RSD-HOT-05e† | A later valid copy of an already accepted record shall change no display, age or state at the GCS. | **T.** GT-14 (i): the record is processed once and duplicates change no display. |
| RSD-HOT-05f† | The GCS application shall return, for every valid copy, one receipt signed with the GCS service key that names the mission, the record identifier and the GCS window, within the limit of 20 receipts per window per backhaul. | **T.** GT-14 (i): every valid copy gets a receipt and all copies are released. |
| RSD-HOT-05g† | An invalid copy shall be logged and listed separately at the GCS, without being displayed as data and without a receipt. | **T.** GT-14: invalid copies receive no receipt and appear in the rejected-report list. |
| RSD-HOT-05h† | A producer or a gateway shall release a held copy only on a GCS receipt that verifies under the mission's GCS service certificate, names exactly that record identifier and is not dated more than two windows ahead. | **T, M.** GT-14 (g): a receipt forged by a gateway releases no copy anywhere. E14: copies released by an invalid receipt = 0. |
| RSD-HOT-05i† | Without a receipt, a producer shall re-send its newest unacknowledged summary per group after two summary periods, as one new copy to each designated gateway then reachable, for at most three rounds. | **T.** Unit test: an unacknowledged summary is re-sent after two summary periods, at most three times, newest per group only. |
| RSD-HOT-05j† | A gateway shall hold each copy until a valid receipt or the end of the record's lifetime (600 s from its signed window), in a queue of at most 64 records with the oldest dropped first. | **T.** Unit test: the gateway queue never exceeds 64 records and expires copies at the end of their lifetime. |
| RSD-HOT-05k† | Releasing a copy shall not remove the record from the producer's evidence store. | **T.** Unit test: after release the record remains in the producer's evidence store. |
| RSD-HOT-08 | @G No dependence on backhaul; no supervision guarantee (a–f). |  |
| RSD-HOT-08a† | No local mission or security decision shall wait for, or depend on, the availability of a backhaul, of headquarters or of the command ship. | **S, M.** GT-15. E14: security outcomes with profile P5 are equivalent to those with PT under the two one-sided tests registered before the runs (TBD-02); accusations from backhaul loss = 0. |
| RSD-HOT-08b† | When no designated gateway is reachable, records addressed to headquarters shall wait as bundles within their lifetime. | **T.** E14: records wait as bundles and expire at their lifetime; none is discarded earlier. |
| RSD-HOT-08c† | The simulation shall represent backhauls only by the abstract profiles PT, P0 and P5 of AS22, implemented as ns-3 point-to-point links with configured data rates, delay, packet loss and scheduled outages. | **I.** Scenario inspection: backhauls are point-to-point links configured from the parameter file; no orbit, propagation or terminal model exists. |
| RSD-HOT-08d† | Every output that uses a backhaul profile shall label its parameter values as scenario inputs (AS22). | **I.** Inspection of E14 outputs: every figure and table that uses a profile labels it as a scenario input. |
| RSD-HOT-08e† | E14 shall count every summary and command attempt, including undelivered ones, in its delivery and age statistics. | **I.** Inspection of E14 analysis: undelivered items count as failures and as censored observations in the age distribution. |
| RSD-HOT-08f† | No project report shall state that TULPAR ran on Türksat, that a SATCOM link was validated, that satellite latency was measured, that supervision delivery is guaranteed or uninterrupted, or that cryptography prevents dropping or delaying. | **I.** Review of every project report and figure caption against the forbidden statements of RSD-HOT-08f. |

**Design rationale.**
- Physical application is decided only on present conditions, so a recomputed command history never creates a new movement. Applying a command because another vehicle reported it applied would let a compromised vehicle forge an authorization. Evidence converges after a merge; physical states need not.
- ABORT and HOLD records need two signatures, so a single stolen operator key can only reprioritize or request support. A record that fails validation proves nothing against the vehicle that forwarded it, because that vehicle did not sign it.
- A valid summary shows who produced it, not that it is true (LIM-29). Age is computed from the signed window, so a delayed old summary never looks fresh.
- The designated gateways are fixed in the signed mission plan, so the Network Graph is unchanged and no insider can attract traffic by announcing a backhaul. Redundant copies remove the wait after a single gateway is lost but guarantee neither delivery nor continuous supervision. A compromised gateway can drop, delay or repeat traffic but cannot read, alter or forge it.

E14 results are not measurements of the Türksat service (AS21, AS22, LIM-39 to LIM-41), and its statistical thresholds are open (TBD-02, Section 6.5). Command and receipt validity during a compromise of the gateway build is specified by RSD-CMC-07 (Section 4.6); the three gateways share build R (LIM-45).

## 4.9 Interfaces and Data Formats

Several invariants of the architecture are statements about which module may call or write to which other module: delivery quality never reaches evidence (MD1), security never commands motion (MD5), ground truth never reaches an agent (MD9), the LSC parses no record (MD19), and no security module reads backhaul state (MD20). They are checked statically, through the dependency rules of the build, because a missing interface cannot be misused at run time. Table 4.25 lists the internal interfaces of Fig. 3.1 with the requirement that governs each; a flow between modules that is not listed is not permitted. External interfaces are listed in Table 1.3.

**Table 4.25. Internal interfaces.**

| # | Producer → consumer | Kind | Content | Governing requirements |
|---|---|---|---|---|
| I1 | M-COM ⇄ M-DEL | Local call | Next hop, source and backup route, "unreachable" notice | RSD-NET-04, RSD-NET-05 |
| I2 | M-COM → M-EVS | Signed record | Link-state advertisement, link-measurement fact | RSD-NET-02 |
| I3 | M-DEL → M-EVS | Signed record | Received records, PERSISTED receipt, corrupted-copy fact | RSD-NET-03a |
| I4 | Sensors → M-EVS | Signed record | Observation record | RSD-OBS-03a |
| I5 | M-MIS → M-EVS | Signed record | Task, bid, repair lease, support and contract records | RSD-MIS-01, RSD-MIS-03 |
| I6 | M-CRY → all | Local service | Sign, verify, session, epoch announcement | RSD-CRY-01, RSD-CRY-02, RSD-CMC-01e |
| I7 | M-EVS → M-OBS, M-SEC | Derived view | Evidence set, root classes, taint | RSD-EVD-03 |
| I8 | M-OBS → M-SEC | Derived view | Refutation counts per sensor channel | RSD-OBS-01g |
| I9 | M-SEC → M-COM | Derived list | Quarantined and transit-restricted vehicles | RSD-NET-04 |
| I10 | M-SEC → M-CRY | Derived list | Trusted list, epoch recipients, G3 events | RSD-CRY-03 |
| I11 | M-SEC, M-OBS → M-MIS | Need and state | Verification needs, unexplored cells, vehicle states; never commands | RSD-SEC-03 |
| I12 | M-COM → M-MIS | Local state | Connection FSM state, component membership, cut-vertex warning | RSD-ORG-04 |
| I13 | M-MIS → M-DEL | Local call | Predicted contact times; priority of support bundles | RSD-NET-05 |
| I14 | M-MIS → M-LSC | Waypoint | Proposed waypoint with validity | RSD-SAF-01c, RSD-SAF-02b |
| I15 | Agent → SIMDIS bridge | Display event | States and explanations; no evidence | RSD-SIM-03 |
| I16 | M-COM → M-MIS | Local state | ConnectivityRequirement | RSD-NET-06a |
| I17 | M-LSC → M-MIS | Local reply | Accept or reject a waypoint, with reason | RSD-SAF-01d |
| I18 | M-OBS → M-MIS | Derived view | Uncertainty record for the Verification Backlog | RSD-MIS-03a |
| I19 | M-MIS → M-DEL | Local call | Support request, offer and contract messages with C2 priority and deadline | RSD-MIS-03g |
| I20 | M-MIS → M-DEL | Signed record | Group Status Summary in the producer's next signed bundle | RSD-HOT-03c |
| I21 | M-EVS → M-MIS | Derived view | Operator command records and receipts in the frozen evidence set | RSD-HOT-01 |
| I22 | M-MIS → M-LSC | Heartbeat | Fixed-format liveness signal every second; a counter only | RSD-SAF-02e, RSD-SAF-03a |
| I23 | M-COM → M-MIS | Local state | Jamming indicator and degradation level of the own vehicle | RSD-JAM-01 |
| — | Delivery quality → M-EVS or M-SEC | Forbidden | — | RSD-NET-03a (MD1) |
| — | Backhaul state → any security module | Forbidden | — | RSD-HOT-07 (MD20) |

Table 4.26 lists the record formats on which requirements depend. Field sizes are given where the architecture gives them; byte-level encodings are frozen in Phase 1.

**Table 4.26. Records and formats relied on by the requirements.**

| Record or frame | Key fields | Relied on by |
|---|---|---|
| Common record header | Identifier (SHA-384 of the canonical encoding), origin, record sequence number, previous-record hash, window, epoch, type, uniqueness key, supersedes, parents, common-cause tags, policy hash | RSD-EVD-02, RSD-EVD-05, RSD-OBS-03 |
| Signed bundle | Origin, window, first and last record sequence, Merkle root, records, ML-DSA-87 signature (4627 B) | RSD-EVD-01; RSD-CRY-01b |
| Digest | Per origin: highest record sequence and 8-byte chain-head prefix; evidence-set hash; state hash with policy version; nodes heard directly | RSD-EVD-01c, RSD-EVD-03 |
| Overlay frame header | Version and suite, frame type, sender, next hop or broadcast, 8-byte counter, message id, fragment index and count, 16-byte tag | RSD-NET-01, RSD-CRY-01g |
| HELLO | Node id, epoch, sequence number, timestamp, HMAC-SHA-384 under the discovery key (truncated to 16 B) | RSD-NET-02d, RSD-CRY-03a |
| Handshake HS1 / HS2 / HS3 | Nonce, ML-KEM encapsulation key or ciphertext, transcript signature, key confirmation, suite id; HS2 and HS3 add the boot quote | RSD-NET-02a, RSD-CMC-04a |
| Link-state advertisement | Strictly increasing sequence number, quantized position with source and error bound, neighbor list with ETX in each direction and age | RSD-NET-02 |
| Epoch announcement and revocation record | Epoch id, parent epochs, recipient list, hash of the new key, producer; a revocation record adds the excluded certificate id and the embedded proof | RSD-CRY-03, RSD-ORG-03d |
| Load receipt | Vehicle id, mission manifest hash, device envelope hash, boot quote, signature | RSD-ORG-05 |
| Operator command | Mission id, command id (console, console sequence), type, target, parameters, issue window, duration, validity, precondition, basis, one or two operator signatures from distinct certificates over the canonical encoding of every other field | RSD-HOT-02, RSD-HOT-06 |
| Command receipt | Command id, vehicle, result (APPLIED, VOID, REJECTED), window | RSD-HOT-01m |
| Group Status Summary | Group, producer, window, members covered with compact positions, vehicles per security state, energy, channels OK per modality, coverage, regime per sector, top backlog items, contacts, alerts, jamming level, commands applied | RSD-HOT-03 |
| GCS service certificate | Role "gcs", mission id, ML-KEM-1024 and ML-DSA-87 public keys, validity, headquarters root signature | RSD-HOT-05, RSD-HOT-06d |
| Backhaul hello | Sender (headquarters or gateway), link id, window, sequence; unsigned; never forwarded into the mesh | RSD-HOT-04, RSD-HOT-07 |
| LSC beacon (raw frame) | Vehicle id (2 B), position and altitude (12 B), velocity (6 B), counter (4 B); 24 bytes; no authentication | RSD-SAF-02d, RSD-SAF-03 |

### Requirements

**Table 4.27. Interface boundary requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-SEC-03 | @G Security never commands motion (a–b). |  |
| RSD-SEC-03a | Distributed Security shall expose to Mission & Cooperation only need and state interfaces (I11), and no waypoint interface. | **S.** Static check: the security build target exposes no waypoint interface to the mission target. |
| RSD-SEC-03b | No transition of the vehicle security state machine shall trigger a maneuver. | **T.** E12 runs: no security-state transition is followed by a waypoint attributable to it in the event log. |
| RSD-SAF-02 | @G The LSC parses no record and has its own build (a–f). |  |
| RSD-SAF-02a | The LSC shall parse no evidence record, bundle or operator command. | **S, T.** GT-13 and static check: no record or bundle decoder is reachable from the LSC. |
| RSD-SAF-02b | The inputs of the LSC shall be limited to its own sensors, LSC beacons, fixed-format position telemetry passed on by the mission computer, waypoints (I14) and the heartbeat (I22). | **S.** Interface inventory of the LSC target lists exactly the five inputs. |
| RSD-SAF-02c | The LSC build target shall link no agent module and no record decoder. | **S.** MD19 static check: the LSC build target links no agent module and no record decoder. |
| RSD-SAF-02d | The LSC beacon shall be a fixed-length 24-byte raw frame carrying vehicle id (2 B), position and altitude (12 B), velocity (6 B) and a counter (4 B), with no variable field. | **T.** Unit test of the beacon encoder and decoder: every frame is 24 bytes; a frame of any other length is discarded. |
| RSD-SAF-02e | The heartbeat from the mission computer to the LSC shall be a fixed-format liveness counter sent every second. | **T.** Unit test: the heartbeat carries only a counter and is emitted every second while the mission computer runs. |
| RSD-SAF-02f | The LSC shall be built from its own source tree by its own build job, with no code generated from the agent's specification. | **I, S.** Inspection of the build configuration: a separate source tree and build job, and no generated code shared with the agent. The MD19 link check passes. |
| RSD-HOT-07 | @G Security derivation never reads backhaul state (a–c). |  |
| RSD-HOT-07a† | Distributed Security, Observation & Fusion, evidence evaluation, the Connection FSM and the component computation shall have no read path to backhaul state. | **S.** GT-15 (a): no reader of backhaul state exists in the listed modules. |
| RSD-HOT-07b† | Within vehicle software, only a designated gateway's delivery forwarding shall observe whether its own backhaul accepts a frame. | **S.** GT-15 (a): the only vehicle-side reader is the gateway's delivery forwarding. |
| RSD-HOT-07c† | The loss of a backhaul shall not enter the jamming indicator or any evidence. | **T, M.** Unit test: a backhaul outage leaves the jamming indicator unchanged. E14: accusations from backhaul loss = 0. |
| RSD-SIM-03 | @G SIMDIS displays computed events only (a–g). |  |
| RSD-SIM-03a | The SIMDIS viewer shall derive no state, edge, contact, lease or contract and interpolate nothing except vehicle positions between deltas. | **S, T.** Static check of the viewer code; hash matching: the hash of displayed states equals the agents' state hashes in the event log. |
| RSD-SIM-03b | Every displayed security state, edge, contact, lease and contract shall carry its provenance: run id, simulation time, producing agent and module, and the record ids it rests on. | **T.** Integration gates I-09 and I-10: every displayed repair edge carries its link-state record ids, and every displayed contract carries its CONTRACT record id. |
| RSD-SIM-03c | The bridge shall carry display events and allow-listed test requests only, and never evidence. | **S.** The bridge message schema contains no evidence record type. |
| RSD-SIM-03d | A viewer test request shall reach only the scenario controller and take effect only as a scheduled scenario event. | **T.** Gate I-02: a test request receives a unique acknowledgment and its effect appears in the simulation through the scenario controller; static check: the bridge has no reference to agent classes. |
| RSD-SIM-03e† | An OPERATOR_COMMAND request from the viewer shall reach only the GCS application and never an agent directly. | **T.** Gate I-11: an OPERATOR_COMMAND request appears as a command record in the event log and reaches no agent directly. |
| RSD-SIM-03f | The ns-3 process shall continue running when the viewer disconnects. | **T.** Gate I-04: with the bridge cut, ns-3 keeps running and the viewer recovers with a snapshot. |
| RSD-SIM-03g | Every mockup shall carry a watermark and be stored apart from run outputs. | **I.** Inspection of the mockup folder and of the output naming scheme before each review. |

**Design rationale.**
- With only need and state interfaces, security can raise the priority of verification but cannot move a vehicle (RSD-SEC-03). The separation rests on the absence of an interface, not on correct code paths.
- The LSC parses no variable-length input because it is the last barrier against a fleet-wide trigger. Its beacon parser is reachable by an outsider, which is why bounded model checking of its decoder is an optional item (Table 6.7). A separate source tree and build job keep a compromised build pipeline from reaching the agent and the LSC together (RSD-SAF-02f).
- Keeping backhaul state out of every security module makes backhaul availability irrelevant to security (CLM-47). Evidence crossing the SIMDIS bridge would bypass the simulated loss and delay. The general display rules are checked by gates I-00 to I-10, and gate I-11 checks the backhaul display.

## 4.10 Performance and Non-Functional Requirements

A performance statement can only say what the simulated system achieves under modeled channels, processing delays and fleet. This section distinguishes three kinds of statement. A requirement has a pass or fail criterion. A design target is tested as a pre-registered hypothesis, and missing it is a finding against the design, not a failed requirement. A budget is a calculation that shows the design can fit the channel and the processor; it is not a measurement.

The baseline contains two binding performance obligations: the evidence-plane cap (RSD-EVD-04a, RSD-EVD-04b) and the reproducible timing model (RSD-SIM-01). The other quantitative targets are hypotheses (Table 4.31). Whether any of them should become a requirement is a team decision.

**Table 4.28. Reproducibility and timing-model requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-SIM-01 | @G Benchmark-based processing delays and reproducible runs (a–d). |  |
| RSD-SIM-01a | Cryptographic processing delays shall be scheduled in simulation time from the micro-benchmark delay table, never from the wall clock during a run. | **S, T.** Static check: no wall-clock call is reachable from agent code. Test: the same run on a loaded and an idle host gives identical event logs. |
| RSD-SIM-01b | The delay table shall be produced by a micro-benchmark of ML-DSA-87 signing and verification and ML-KEM-1024 encapsulation and decapsulation on the development machine. | **I.** The micro-benchmark report lists the four operations with the development machine, Botan version and repetition count, and the delay table is loaded as a simulation parameter. |
| RSD-SIM-01c | All random streams and generated keys of a run shall be derived from the run seed, with MRG32k3a streams separated by run number and keys seeded by run and node number. | **S.** Code inspection of the random-stream and key-generation set-up. |
| RSD-SIM-01d | Two runs of the same test configuration with the same seed shall produce identical event logs. | **T.** Gate G0 and integration gate I-07: two runs with one seed give identical trace hashes and identical analysis results. |

### Channel and resource budgets

Table 4.29 is a calculation from the 802.11n airtime of HT-MCS0 broadcast frames for the 20 nodes of the reference fleet, with every node sending a 5.5 kB bundle in every window. A 1400-byte fragment takes 1.99 ms. Experiment E1 compares the measured shares with this table. Two events add short bursts: a G3 proof of about 10.5 kB re-broadcast once by all 20 nodes takes about 300 ms of airtime in total, and a merge after 600 s of partition must move about 13.2 MB of evidence, which takes about 310 s (LIM-15). The merge estimate uses the drain formula. After a partition of duration T_part, the evidence that two components X and Y must exchange is B_merge ≈ T_part × (ρ_X + ρ_Y) × S, where ρ_X and ρ_Y are the summed bundle rates of the origins in X and in Y and S is the mean bundle size. The drain time is T_drain ≈ B_merge / (β_merge × R_eff), where β_merge is the share of channel time that pulls may use during a merge and R_eff is the effective broadcast rate. Experiment E4 compares measured drain times with T_drain (Table 4.31). The budget depends on the fleet size N. Most entries grow linearly with N, while the hop handshakes and the per-receiver authentication tags of telemetry grow with the number of neighbor pairs, which is at most N(N−1)/2 (190 in the reference fleet). The table therefore holds only for the reference fleet.

**Table 4.29. Channel budget of the 20-node reference fleet in normal operation (calculation).**

| Traffic | Assumption | Channel share |
|---|---|---|
| Telemetry | 1 Hz per node; 150 B payload plus per-receiver 8-byte MACs | 1.27 % |
| HELLO | Every 2 s per node; 48 B | 0.32 % |
| Hop handshakes | 190 pairs, about 12.4 kB per handshake, every 600 s | 0.58 % |
| Bundles | Every node every window; 5.5 kB; four fragments | 3.13 % |
| Pull repair | 20 % of bundles sent once more by a neighbor | 0.63 % |
| Digests | Every node every window; about 370 B | 0.26 % |
| C1/C2 and hop acknowledgments | Two flows, one 600 B message per 5 s, three hops | 0.16 % |
| Group Status Summary | Three components; about 1 kB every 30 s; one copy per reachable designated gateway, at most four, each over at most 10 hops | ≤ 0.4 % (worst-case estimate) |
| Operator commands | Worst case one two-signature command per minute, flooded once by 20 nodes | 0.53 % |
| LSC beacons | 20 vehicles at 1 Hz; 24-byte raw broadcast frame | 0.42 % |
| Supervision traffic on a backhaul | Separate point-to-point link | 0 % of the mesh channel |
| **Total** |  | **≤ 7.7 %** |
| Evidence plane (bundles, pull, digests) | Cap 10 % (RSD-EVD-04a) | 4.10 % (calculation) |
| Control plane (HELLO, handshakes) | Hypothesis threshold 5 % | 0.90 % |

**Table 4.30. Resource budget per vehicle in the reference configuration (calculation).**

| Resource | Worst case in a 30-minute mission | Basis |
|---|---|---|
| Signature verification | 4 bundles/s × 0.51 ms ≈ 2 ms per second (0.2 % of one core) | Estimate from published cycle counts; TBR-01 until the Botan benchmark |
| Signing | 1 bundle per 5 s × 0.96 ms | Estimate; TBR-01 |
| Handshakes | 19 per 600 s | Hop-session design |
| Evaluation per window | Target ≤ 100 ms (2 % of the window), measured in Phase 5 | Incremental evaluation; re-derivation at most once per window |
| Evidence store | ≤ 39.6 MB (7200 bundles of 5.5 kB) | Bundle quota |
| Event log | ≤ 40 MB | One line per decision and per record |

The per-vehicle figures grow with the fleet size and the mission length: a vehicle keeps a hop session with each neighbor, at most 19 in the reference fleet, and stores up to one bundle per node and window, 7200 in a 30-minute mission. In the simulation all 20 agents share one process, so immutable records are stored once and each agent keeps an index; copying them per agent would cost about 792 MB per run instead of about 40 MB. The wall-clock time of a run is unknown until the pilot of the execution-time gate (Section 5.4).

### Design targets

**Table 4.31. Quantitative design targets.**

| Target | Value | Tested by | Status in this RSD |
|---|---|---|---|
| Evidence-plane share | ≤ 10 % of channel time | E1 (upper 95 % bound) | Requirement: RSD-EVD-04a |
| Control-plane share | ≤ 5 % of channel time | E1 (upper 95 % bound) | Pre-registered hypothesis |
| Proof to quarantine within a component | ≤ 2 s (p95) | E3 | Pre-registered hypothesis |
| Proof spread after a merge | ≤ 5 s (p95) after the first verified link | E3 | Pre-registered hypothesis |
| Evaluation time per window | ≤ 100 ms | Phase 5 measurement (benchmark-scaled) | Design target; no registered test threshold |
| Merge drain time | Within 20 % of the calculated T_drain | E4 | Pre-registered hypothesis; the optional SC-8 uses the same threshold |
| C2 delay during a merge | ≤ 2 × normal (median) | E4 | Pre-registered hypothesis |
| Commands to Live vehicles | Within 10 s (p95) under the conditions of CLM-48 | E14 | Hypothesis (CLM-48) |
| Supervision share of the mesh channel | To be registered before the runs | E14 | Pre-registered hypothesis; value TBD-02 |
| Display delay at 10 Hz | 95th percentile within 250 ms | Integration gate I-08 | Gate pass condition (display only) |

### Other non-functional characteristics

The remaining quality characteristics are fixed by requirements specified elsewhere or by architecture constraints:

- **Determinism and reproducibility.** Agents with the same evidence derive the same states (RSD-EVD-03), the same seed gives the same event log, and processing delays never come from the wall clock (RSD-SIM-01).
- **Auditability.** Every decision record carries its evidence identifiers, policy version and open hypotheses, and post-mission analysis cites record identifiers (RSD-MIS-04).
- **Portability and maintainability.** The mesh core is independent of the socket layer (RSD-NET-01a). The system is one ns-3 module in C++23 under GPL-2.0-only, built with CMake in a shared Docker image (Ubuntu 24.04, GCC 13), with the eight agent modules and M-SIM as separate build targets so that the build enforces the dependency rules. These are implementation constraints of the architecture and are not specified as requirements here.
- **Display accessibility.** The viewer shows a state letter next to each state color, so that viewers who cannot distinguish red from green can read the display.
- **Scale.** The design is sized for fleets of up to 20 nodes (Section 1.4). Quantitative statements refer to the 20-node reference fleet, and figures for larger fleets are calculations (RSD-SIM-06).

## 4.11 Simulation and Verification

The requirements below make the simulation a valid source of verification evidence: processing delays come from measured benchmarks, runs are reproducible, ground truth never reaches an agent, and the experiments follow their registered plan. Chapter 5 describes how they are used.

**Table 4.32. Simulation and verification requirements.**

| ID | Requirement | Verification and acceptance |
|---|---|---|
| RSD-SIM-02 | @G Ground truth never reaches an agent (a–b). |  |
| RSD-SIM-02a | Ground truth shall be held in a store that agent code cannot access. | **S, T.** Unit test that fails if an agent class includes the ground-truth header (gate G0, MD9 static check). |
| RSD-SIM-02b | Changing only the hidden attacker label of a scenario shall change no agent decision. | **T.** Counterfactual test: two runs that differ only in the hidden attacker label give identical decisions; disabling the attack changes them. |
| RSD-SIM-04 | @G Invariants checked in every run (a–c). |  |
| RSD-SIM-04a | Every run shall check the invariants MD1 to MD20 by the checks listed for them in Table 5.1. | **I, T.** The run pipeline contains one check per invariant; the check report of every run lists MD1–MD20 with zero violations. |
| RSD-SIM-04b | Every run of an experiment shall check every "must be 0" quantity that the experiment matrix lists for that experiment. | **I.** For each experiment, the analysis pipeline evaluates every "must be 0" quantity of its matrix row in every run; a single non-zero value fails the experiment. |
| RSD-SIM-04c | The seven deterministic acceptance rules listed in Section 5.2 shall be checked automatically against the event log of every run. | **I, T.** The automatic checker covers the seven acceptance rules (with MD1–MD20 in rule 7) and reports zero violations per run. |
| RSD-SIM-05 | @G Statistical reporting against pre-registered thresholds (a–e). |  |
| RSD-SIM-05a | Every probabilistic result shall be reported with its 95 % confidence interval. | **I.** Every statistical result table and figure shows its 95 % confidence interval. |
| RSD-SIM-05b | Every statistical hypothesis shall be tested only against the threshold, metric, repetition count and test registered for it in the pre-registration record before its test runs. | **I.** The pre-registration record of each experiment is dated before its first test run, and every reported verdict cites it. |
| RSD-SIM-05c | Comparisons between configurations shall use paired runs and the Wilcoxon signed-rank test with Holm correction for multiple comparisons. | **I.** The analysis scripts implement paired runs and Holm-corrected Wilcoxon tests. |
| RSD-SIM-05d | Thresholds and weights shall be tuned only on calibration runs that are separate from the test runs. | **I.** Calibration and test runs use disjoint seed sets; the on-off attacker appears only in test runs. |
| RSD-SIM-05e | Absolute delivery and delay results shall be reported only as comparisons between configurations. | **I.** Review of every reported delivery and delay figure: each is a comparison between configurations. |
| RSD-SIM-06 | @G Claim boundary at the tested fleet size (a–b). |  |
| RSD-SIM-06a | No project report shall claim a scalability result for a fleet larger than the tested 20-node reference fleet. | **I.** Review of every project report and of this document: no scalability claim beyond 20 nodes. |
| RSD-SIM-06b | Every figure for a fleet larger than 20 nodes shall be labeled as a calculation. | **I.** Review: every figure for more than 20 nodes is labeled as a calculation (LIM-47). |

# 5 Verification

This chapter describes how the requirements of Chapter 4 will be verified. No verification has been carried out yet; the chapter defines the methods, the tests and the order in which they are introduced.

## 5.1 Verification approach

The architecture checks requirements in four layers, from cheapest to most expensive:

- **Static checks and unit tests** verify module boundaries, encodings and state machines. They run in the phase that builds the module and are kept as regression tests.
- **Fifteen gate tests** check the transitions between mechanisms, where integration errors are most likely. Each is a small deterministic scenario with a pass criterion (Table 5.2).
- **Invariants and "must be 0" quantities** are checked against the event log of every run.
- **Statistical experiments E1–E15** compare the design with simpler rules on paired runs, against hypotheses and thresholds registered in advance (Table 5.3). They run on the reference configuration and on the variations that each experiment defines, and their results hold only for these configurations (Section 1.4).

Passing a gate test does not prove the system secure; it exposes integration errors before the more expensive work. Every random stream of a run is derived from the run seed with the MRG32k3a generator [73] (RSD-SIM-01c), so two runs with the same seed produce identical event logs (RSD-SIM-01d). Configurations are compared on paired runs with the Wilcoxon signed-rank test [74] and the Holm correction for multiple comparisons [75] (RSD-SIM-05c).

A deterministic criterion passes or fails on a single run, and a "must be 0" quantity fails on one violation. A statistical criterion is judged on the confidence interval of its registered metric, and a threshold tuned on the test runs is not valid (RSD-SIM-05d). Simulated results count as verification evidence only under three conditions. Ground truth must never reach an agent (RSD-SIM-02), every invariant must be checked in every run (RSD-SIM-04), and every probabilistic result must be judged against a threshold fixed in advance (RSD-SIM-05). Reproducibility is RSD-SIM-01 (Section 4.10).

Four verification conditions are not yet fixed, and the affected criteria say so rather than assume a value:

- The statistical thresholds of E14 are not registered (TBD-02).
- The pull-share metric of RSD-EVD-04b is undefined (TBD-01).
- Byte-level encodings are frozen only in Phase 1, so format requirements can be checked for field presence and fixed sizes only.
- Run counts stay provisional until the execution-time gate (Section 5.4).

## 5.2 Architectural invariants and acceptance rules

Table 5.1 lists the invariants that must hold in every run, with the check for each. Seven deterministic acceptance rules are checked automatically against the event log of every run (RSD-SIM-04c):

- Rule 1: no quarantine without a G3 proof (MD7);
- Rule 2: the same observation arriving over several paths counts once (MD3);
- Rule 3: agents with equal evidence hashes publish equal state hashes (MD8);
- Rule 4: no agent reads ground truth, so changing only the hidden attacker label changes no decision (MD9);
- Rule 5: a bridge request outside the allow-list is rejected;
- Rule 6: the same seed gives the same event log (I-07);
- Rule 7: every invariant MD1–MD20 holds.

**Table 5.1. Architectural invariants MD1–MD20.**

| ID | Invariant | How it is checked |
|---|---|---|
| MD1 | Packet loss or disconnection alone never produces an accusation | Static: no write interface from delivery to the Evidence Graph or security. Test: zero anomaly records under loss injection |
| MD2 | A valid signature does not make content true | Test: a signed false observation from one class never rises above Reported; a signed one-sided edge never enters the Network Graph |
| MD3 | The same data arriving over several paths is not counted as several roots | Design: record identifier = content hash. Test: the same record over two paths leaves the root count unchanged |
| MD4 | Disconnection is not mission abort | Model check: no unconditional transition from Connection = Alone to Mission = Return |
| MD5 | Security evaluation never commands motion | Static: security has only need and state interfaces to mission |
| MD6 | No definitive conclusion without sufficient evidence | Test: the status rules never give Corroborated or Confirmed from one class |
| MD7 | Quarantine (L4) only with G3 | Run-time check and UPPAAL model |
| MD8 | Same evidence set, same state | Run-time comparison of evidence and state hashes in digests |
| MD9 | Ground truth never reaches agents | Test: changing only the hidden attacker label changes no decision |
| MD10 | Local safety is active in every state | Test: collision avoidance runs in every mission and security state; every movement entry has an LSC acceptance record |
| MD11 | No route is computed through a quarantined vehicle | Test: Network Graph filter |
| MD12 | The source stays in the M17 suspect set until a signed first-hop receipt clears it | Unit test: corrupted copies on two disjoint paths give a finding against the source; a forwarder is named only after the source is cleared |
| MD13 | A repair trigger never derives from an accusation; the only security input that can change it is the removal of a quarantined or transit-restricted vehicle from the Network Graph | Static: the trigger's inputs are pending delivery value and Network Graph connectivity only; test with a fabricated suspicion |
| MD14 | A support request is never a command; the receiving group's planner decides | Static: no interface from a foreign record to mission waypoints. Test: a request alone never changes a receiving vehicle's mission state |
| MD15 | Guest participation never changes a vehicle's identity, group membership, security state or policy | Unit test on the guest-admission path; no contract field can write membership |
| MD16 | The Network Graph is updated only from measured links, never from predicted positions or planned repairs | Test: a repair move whose link fails leaves the target unreachable in the Network Graph |
| MD17 | Within a window, observation statuses use the channel states of the previous window, and a channel restriction never removes the evidence that caused it | Test: replay of a fixed evidence set gives at most one status or channel transition per object and window and no two-window cycle |
| MD18 | An operator command never changes a security state, the roster, a key, the policy or a threshold; a vehicle applies a command only from its own present window and present evidence | Static: no write interface from command handling to M-SEC or M-CRY. Test GT-12. Run time: applications outside validity or with a false present precondition must be 0 (E14) |
| MD19 | The LSC parses no evidence record, bundle or operator command; its only inputs are its own sensors, LSC beacons, fixed-format position telemetry, waypoints and the heartbeat | Static: the LSC build target links no agent module and no record decoder. Test GT-13 |
| MD20 | Headquarters is an endpoint and a backhaul is untrusted transport: neither is part of any Network Graph, and no security evaluation reads backhaul state | Static: no Network Graph vertex for headquarters, no link-state field for backhauls, no read path from security modules to backhaul state. Run time: GT-14, GT-15 |

## 5.3 Gate tests and experiments

Every experiment stresses an architecture decision, an assumption or both, and every security experiment includes an f = 2 variant. Results in the last column of Table 5.3 must not be claimed from the experiment.

**Table 5.2. Gate tests.**

| Test | Scenario | Phase | Requirements relying on it |
|---|---|---|---|
| GT-01 | Corrupted copies on two disjoint paths, then on two overlapping paths with a clearing first-hop receipt | 1 (logic), 6 | RSD-SEC-02c, RSD-SEC-05 |
| GT-02 | Two components in different epochs reconnect after the old key's transition period | 4 | RSD-NET-02a, RSD-CRY-03b |
| GT-03 | Re-key while the group has members in Watch and Restricted | 4 | RSD-CRY-03d, RSD-CRY-03e |
| GT-04 | A relay forwards a wrong epoch key | 4 | RSD-CRY-03f |
| GT-05 | Provisioning with per-vehicle private keys | 4 | RSD-ORG-05 |
| GT-06 | Bids delayed so that two agents pick different winners for an exclusive task | 6 | RSD-MIS-01e, RSD-MIS-01f |
| GT-07 | Two attackers accuse an honest cut vertex | 5 | RSD-NET-04c, RSD-SEC-02d |
| GT-08 | Long partition, large G3 proof, 10 % frame loss; counts the agents that missed every re-broadcast and received the proof through digest and pull (D3 of G-e); an agent that stays without the proof while a neighbor holding it is in contact for five windows is a D3 failure | 1–2 | RSD-NET-01f; RSD-ORG-03c, RSD-EVD-01d, RSD-EVD-06 |
| GT-09 | The same valid evidence delivered in many different orders | 2 | RSD-EVD-03a, RSD-MIS-03a |
| GT-10 | A late record re-opens an old window | 2 | RSD-EVD-05 |
| GT-11 | Malformed record presented to the key provider; a key-extracted attacker sends one; the same record checked by every build. Cases d1–d4: implanted EO-X agents with honest support added step by step, and the per-group build map as baseline (d4). Case e: an implanted ingress forges, drops, delays and reorders inputs, followed by the keyless replay. Case f: an implanted class reports misses on a true contact that two honest classes see | 2, 4; cases d–f at 5 | RSD-CRY-02d, RSD-CRY-02e, RSD-CMC-01b, RSD-CMC-01d to RSD-CMC-01g, RSD-CMC-02, RSD-CMC-06, RSD-SEC-01d |
| GT-12 | Invalid, modified, replayed, future-dated and other-mission commands; HOLD and RESUME orders; GT-12f partition cases; GT-12g–i gateway cases | 6 | RSD-HOT-01, RSD-HOT-02, RSD-HOT-06; RSD-ORG-03b |
| GT-13 | Mission computer stops in every state; keeps the heartbeat but sends out-of-envelope waypoints; beacons suppressed, forged or flooded | 5 | RSD-SAF-01b, RSD-SAF-02a, RSD-SAF-03, RSD-CRY-02c |
| GT-14 | Backhaul injection cases (a)–(h); headquarters report cases; three-gateway duplicate (i); isolation checks (j), (k) | 6 | RSD-HOT-03h, RSD-HOT-03i, RSD-HOT-04, RSD-HOT-05, RSD-HOT-06 |
| GT-15 | Static check that no security path reads backhaul state; review of the command log of an E14 run | 6 | RSD-HOT-04b, RSD-HOT-07, RSD-HOT-08a |

**Table 5.3. Experiments E1–E15.**

| ID | Scenario | Can show | Cannot show |
|---|---|---|---|
| E1 | Control and evidence load | Channel shares against Table 4.29; micro-loops [76]; overlay against AODV and OLSRv1 | Absolute performance of a tactical radio (AS6) |
| E2 | Droppers and attraction attack | Delivery with backup paths; zero anomaly records under loss | Detection of pure droppers (impossible) |
| E3 | Evidence propagation and escape window | Time from proof to quarantine and to revocation in every group | Delivery across a partition that never heals |
| E4 | Partition and merge | Drain time against T_drain (Section 4.10); convergence and divergence time | Behavior at unsimulated fleet sizes |
| E5 | Fusion under model error | False Confirmed and Refuted rates under wrong sensor models | Correctness of the sensor models themselves |
| E6 | False and coordinated observations | Poisoned observations used; track-initiation suppression | Detection of a consistent lie seen by one observer |
| E7 | False accusation and colluding accusers | Wrongful quarantine (must be 0); mission damage under f = 2 | Guarantees beyond two colluders per scope |
| E8 | Link claims and path integrity | M15 time to G2; M17 attribution; M16 records on honest links | Wormhole or colluding-pair detection |
| E9 | GNSS spoofing and regional degradation | Attribution to external causes; no wrongful restriction | Defense against collective-wide spoofing |
| E10 | Connectivity repair (SCN-CR-01) | C2 delay and coverage cost against wait-only | Real link prediction at sea (AS16) |
| E11 | Collaborative verification (SCN-CV-01) | Resolution per cost against FIFO and importance-only backlogs | Value estimates when the sensor model is wrong (AS9) |
| E12 | Protocol attacks | Time until honest agents quarantine and components revoke; invalid records accepted (must be 0) | Hardware key extraction |
| E13 | Fleet-wide compromise under the per-class build map, with two per-group baseline cells (px = 0 and 1) and the gateway cell GW-1 | Spread and mission loss as functions of px; provenance of verified statuses; wrongful quarantine (must be 0); command and receipt validity in GW-1; collisions in simulated runs (must be 0) | Whether real builds resist real exploits; strength of real isolation or attestation |
| E14 | Human supervision over designated gateways | Command and summary delivery with failures counted; applications outside validity (must be 0); history divergence (must be 0) | Performance of the real Türksat service or terminal; operator workload |
| E15 | Jamming | Time per degradation level; re-convergence; rally outcomes; anomaly records and collisions (must be 0) | Real jamming physics, anti-jam waveforms, carrier-sense blocking |

## 5.4 Development phases and integration gates

The work is built in phases with monthly gates, ordered so that each phase relies on the invariants of the previous one. Each gate falls at the end of its month:

- Phase 0, the simulation skeleton (gate G0, November 2026);
- Phase 1, mesh and delivery (G1, December 2026);
- Phase 2, the evidence store (G2, January 2027), with Phase 4, sessions and keys, built in parallel in the same month and checked by GT-02 to GT-05;
- Phase 3, observation fusion (G3, February 2027), which completes the minimum working system of Phases 0–3;
- Phase 5, distributed security (G4, Core tier, March 2027);
- Phase 6, group autonomy (G5, Target tier, April 2027);
- Phase 7, experiments and visualization (G6, May 2027).

The final report follows in June 2027. The gates G0–G6 are unrelated to the evidence strengths G0–G3 of Section 4.6. Each gate introduces its static checks, invariant tests and gate tests, for example GT-11 in Phases 2 and 4 (gate G2) with its cases d–f at G4, GT-13 at G4, and GT-12, GT-14 and GT-15 at G5. The SIMDIS integration gates I-00 to I-11 check that the display shows computed results only.

If time runs short, scope is cut in tier order:

- **Core tier (G4).** Must show that a vehicle that drops messages is routed around without being accused, and that a signed but false observation cannot reach a verified status from a single source.
- **Target tier (G5).** Adds the remaining security loop, operator commands, degradation under jamming and the exploit scenarios of E13.

The tier is a planning attribute, not an approval status. The first verification status changes are expected at G0, when the simulation skeleton is checked for identical traces under the same seed (RSD-SIM-01d) and for the absence of ground truth in agent code (RSD-SIM-02).

**Execution-time gate.** Run counts are fixed by a short gate before the experiment runs are committed. Its pilot runs the reference mission at 20 nodes with full cryptography and full logging for three seeds and records the wall-clock time per run, the peak memory and the log size. The pilot runs as soon as the agent can execute that mission, and at the latest at gate G5. The team then states the machine-hours H it can dedicate to the runs. The gate passes if N × (mean time per run) × 1.2 ≤ H, where N is the number of runs in the registered experiment plan and the factor 1.2 allows for reruns. If it fails, the plan is reduced in this order: the optional items, the E13 seeds from 20 to 10, and the repetitions of the other experiments from 30 to 20, and the reduced statistical power is reported. If even the reduced plan does not fit, statistical results are reported as indicative only and the deterministic tests carry the evidence.

## 5.5 Traceability and coverage

Every atomic requirement carries at least one verification method and an acceptance criterion. All 59 baseline requirements are specified, each in exactly one section, and they decompose into 308 atomic requirements (Table 5.4). Of these, 145 name at least one gate test, experiment, integration gate or scenario. The others are checked by the unit tests, static checks and inspections that their criteria describe.

**Table 5.4. Coverage of the baseline requirements by area.**

| Area | Baseline rows | Atomic requirements | Of which SATCOM-only (†) | Specified in |
|---|---|---|---|---|
| ORG System organization | 5 | 14 | 1 | Section 4.1 |
| MIS Mission management | 4 | 26 | 0 | Section 4.2 |
| NET Mesh, delivery and connectivity repair | 6 | 28 | 0 | Sections 4.2 and 4.4 |
| OBS Observation fusion | 4 | 15 | 0 | Section 4.3 |
| EVD Evidence | 6 | 27 | 0 | Section 4.3 |
| CRY Cryptography | 3 | 21 | 0 | Section 4.5 |
| CMC Common-mode containment | 6 | 25 | 0 | Sections 4.5 and 4.6 |
| SEC Security evaluation and responses | 6 | 29 | 0 | Sections 4.6 and 4.9 |
| SAF Local safety | 3 | 24 | 0 | Sections 4.7 and 4.9 |
| JAM Operation under jamming | 2 | 11 | 0 | Section 4.7 |
| HOT Supervision and SATCOM | 8 | 65 | 37 | Sections 4.8 and 4.9 |
| SIM Simulation and evaluation | 6 | 23 | 1 | Sections 4.9 to 4.11 |
| **Total** | **59** | **308** | **39** |  |

Every mission need of Table 1.1 except N-16 is served by at least one atomic requirement. N-16 asks for decisions with rationale, known limitations, future work and a bibliography, which the design rationale of Chapters 2 and 4, Chapter 6 and Chapter 7 provide. Experiment E5 is named by no acceptance criterion: it validates the fusion decision under wrong sensor models rather than checking one requirement (Table 5.3).

## 5.6 Validation and verification status

Verification asks whether each requirement is met. Validation asks whether the specified system serves the mission needs. A requirement can pass every test and still miss its need, so validation is planned separately (Table 5.5). Experiment E5 runs the fusion rules with deliberately wrong detection and false-alarm probabilities and with close contacts. It tests whether the clipped fusion rule remains useful when AS9 and AS11 fail, and it is validation evidence for N-04 and N-05.

**Table 5.5. Validation questions, planned evidence and limits.**

| Validation question | Needs | Planned evidence | Limit of the evidence |
|---|---|---|---|
| Do the groups keep working when cut off, and do they agree again after contact? | N-01, N-07 | E3, E4 | Agreement only among agents that reconnect; no statement across a partition that never heals (CLM-05, AS4) |
| Does the mesh deliver around failures without turning loss into accusation? | N-02, N-03 | E1, E2; GT-08; MD1 checks | Relative results on the 802.11n stand-in, not the performance of a tactical radio (AS6, LIM-09) |
| Do heterogeneous sensors and fusion give trustworthy contact statuses? | N-04, N-05, N-13 | E5, E6, E9, E11 | Synthetic sensors whose models the agents may or may not share (AS9); association errors (LIM-17) |
| Is exclusion proof-based and leaderless? | N-05, N-06 | GT-05; E3, E7, E8, E12 | Design target f = 1 per scope; f = 2 variants show where the guarantees stop (LIM-06) |
| Are communication, security, mission planning and local safety kept apart? | N-08 | Static checks of the module boundaries | Logical module boundaries in one ns-3 process (LIM-32) |
| Do connectivity repair and collaborative verification add value at an acceptable cost? | N-09 to N-12 | E10 (SCN-CR-01), E11 (SCN-CV-01) | Predicted links and stale positions (AS16, AS17; LIM-18); coverage cost to the helping group (LIM-20) |
| Is the simulation evidence trustworthy, and does the display show only computed results? | N-14, N-15 | GT-01 to GT-10; I-00 to I-11 | SIMDIS is a display and never evidence; mockups are never presented as outputs |
| Is a fleet-wide software compromise contained, and do vehicles stay inside their safety envelope? | N-R1R2 | GT-11, GT-13; E13 | Cross-build transferability px is a scenario parameter; boundaries are logical; "zero collisions" holds only for the simulated runs (LIM-27, LIM-28, LIM-32) |
| Can a human supervise the collective without becoming a security authority? | N-R3 | GT-12, GT-14, GT-15; E14, E15 | Backhauls are assumed profiles, not the Türksat service (LIM-39); operator workload is not measured |
| Does the collective degrade in defined steps and recover under jamming? | N-R4 | E15 | Frame-error model with a processing gain; no carrier-sense blocking or reactive jamming (LIM-33) |

The verification status of a requirement changes only on evidence (Table 5.6). An optional item of Table 6.7 cannot become VERIFIED as part of the baseline; its results are reported as optional results. Results tables carry explicit placeholders until runs exist, and no hypothetical value is entered, even as an example.

**Table 5.6. Verification status rules.**

| Status | Condition | Evidence required |
|---|---|---|
| NOT TESTED | No procedure for the requirement has been run | None; this is the current status of every requirement |
| TEST PLANNED | The procedure is implemented and scheduled at a phase gate or in the experiment plan | Reference to the test code or experiment configuration |
| VERIFIED | The requirement is implemented and its acceptance criterion has passed | A MEASURED RESULT or DETERMINISTICALLY TESTED RESULT with run identifier, seed and commit |
| FAILED | The acceptance criterion was not met | The same references; the result stays visible in the evaluation |
| INCONCLUSIVE | A statistical criterion could not be decided, for example because the confidence interval contains the threshold | The same references, with the interval; the result stays visible |
| NOT APPLICABLE | The requirement was withdrawn by a later revision of this document | The change record of that revision |

**Notes.** The hidden-label counterfactual is the simplest test that no decision depends on ground truth: a run in which only the attacker's label changes must behave identically. Separating calibration runs from test runs keeps tuned parameters from being judged on the data that tuned them. The scenarios SC-0, SC-7 and SC-8 are optional (Table 6.7).

# 6 Assumptions, Limitations and Open Items

## 6.1 Assumptions

The requirements rest on the assumptions in Table 6.1. The design cannot prove them; each is either stressed in an experiment or marked as out of scope. Section 6.2 shows which guarantees rest on which assumptions.

**Table 6.1. Assumptions AS1–AS22.**

| ID | Assumption | If it breaks | How it is stressed |
|---|---|---|---|
| AS1 | At most one attacker per observation scope (f = 1) | Link-claim and path-integrity metrics can produce G2 against an honest vehicle; fusion can be blinded | f = 2 variants in every security experiment |
| AS2 | A compromised vehicle has a signing oracle, not the key: the key provider is a separate hardware component that never exports a key and signs only schema-valid records. In the simulation it is the only interface to the key | An equivocation proof blames the key; the attacker can sign malformed records and headquarters-only types | Key-extracted profile (E12) |
| AS3 | Honest pre-mission certificate authority (headquarters), an intact provisioning package in every vehicle and a fixed roster | Sybil identities collapse independent-root counting | Out of scope |
| AS4 | Partitions eventually heal (eventual delivery) | Convergence only among vehicles that can communicate | Long partitions; vehicle loss (E4) |
| AS5 | Loose clock synchronization at window level | Freshness, replay and geographic-leash checks give wrong results | Clock-skew runs (E12) |
| AS6 | 802.11n ad hoc stands in for an S-band data link | Absolute delivery and delay do not represent a real radio | Relative comparisons only; slow-radio profile (E1) |
| AS7 | Island shadow is binary line of sight, without diffraction | Observability and link expectations are wrong near the coast | Map-error runs (E10) |
| AS8 | The maximum range comes from the propagation model; atmospheric ducting is excluded | The geographic leash records honest long links as impossible | Range-margin sweep (E8) |
| AS9 | The detection and false-alarm probabilities of the synthetic sensors equal what the agents believe | Status accuracy falls | Runs with deliberately wrong models (E5) |
| AS10 | Root classes separate common causes correctly | Independence is over-counted | GNSS spoofing, fog, shared software fault (E9) |
| AS11 | Association assigns two claims to the same contact correctly | False support or false contradiction | Close-contacts runs (E5) |
| AS12 | All agents run the same source, policy and parameter version and behave deterministically; binaries may be diversified per root class, with no build spanning two classes, without changing any output | Same evidence no longer gives the same state (MD8); states diverge | Version-mismatch test; three-build runs (E13) |
| AS13 | ML-DSA-87 is unforgeable and SHA-384 is collision-resistant | All provable claims fall | Out of scope |
| AS14 | AES-256-GCM integrity holds and an IV never repeats under one key | Path-integrity attribution to corruption inside a node falls | IV-rule unit tests |
| AS15 | Agents bid honestly in task allocation | A compromised vehicle can capture tasks | Dishonest-bid runs; kinematic and contract checks (E12) |
| AS16 | The predicted link quality at a candidate repair position is approximate | Repair moves fail to close the gap | Repair success rate; map-error runs (E10) |
| AS17 | Last-known positions of a disconnected component are stale but bounded in age | Repair is aimed at the wrong place | Staleness sweep (E10) |
| AS18 | Every vehicle's certificate is known collective-wide, so a guest can open hop sessions in any group's mesh | Guest participation is impossible without new key distribution | Holds by construction (one roster, AS3) |
| AS19 | Every network input is parsed behind an ingress boundary that holds no key, no actuator interface and no transmit interface, and signatures are verified by M-CRY on its output: a separate unprivileged process or partition in a real vehicle, a statically checked logical boundary in the simulation | An exploited parser reaches the agent (oracle-constrained profile); keys remain protected by AS2 | E13 stage-II cells; GT-11e; static dependency check |
| AS20 | The LSC runs on separate hardware with separately built code, from its own source tree and by its own build job; its network inputs are the fixed-length LSC beacon and fixed-format position telemetry, neither authenticated to the LSC; its own sensors, these inputs and procedural separation suffice to keep separation | A fleet-wide trigger, a compromised mission computer or a compromised build pipeline can cause collisions or losses | E13 and E15 trigger and jamming runs; GT-13; static check (MD19) |
| AS21 | All USVs belong to the 11–15 m unmanned surface vehicle class and can carry a Ka-band satellite terminal; this is a reference design assumption, and the terminal's power, radome, modem, mounting and sea-state limits are not verified | Smaller USVs cannot carry the terminal; the designated gateway set shrinks to the command ship | Design limit (LIM-41) |
| AS22 | Backhaul profiles are scenario inputs. PT: 6 Mbit/s up and 25 Mbit/s down (manufacturer's stated terminal rates), 280 ms assumed one-way delay, 1 % assumed loss, outages of 0, 30 and 120 s as experiment factors; P0 for the command ship with the same assumptions; P5 means no backhaul. No value is a measured performance of a real service | Supervision delay and availability differ from E14; no security result changes | E14 profile cells |

No assumption states that a proof reaches every honest agent; guarantee G-e gives the conditions under which a proof reaches another honest agent (Section 6.2).

## 6.2 Guarantees and their boundaries

Every security and safety requirement holds only under the conditions stated for it. This section shows which guarantees the requirements rest on, how far those guarantees reach under the attacker profiles of Table 2.2, and which limitations remain. It adds no new claim. Each claim carries one of the following labels:

- **provable:** holds mathematically or structurally, under stated assumptions;
- **conditional:** holds under a model assumption whose truth is unknown;
- **assumption:** cannot be proven, so it is written down and stressed in an experiment;
- **impossible:** cannot be achieved, so the design is built around it;
- **no guarantee:** a liveness property that the design cannot ensure;
- **hypothesis:** tested by an experiment.

Every provable claim has the form "this node said this". No claim about the physical world is proven unconditionally.

### Guarantees the requirements rest on

Table 6.2 gives each claim with its label and the assumptions or reasons it rests on; the last column names the requirements that rely on it.

**Table 6.2. Claims, their labels and the requirements that rely on them.**

| Claim | Statement | Label | Rests on | Requirements relying on it |
|---|---|---|---|---|
| CLM-01 | Two different signed records at one chain position or under one semantic key prove that the key holder deviated (G3) | Provable | AS13, AS2; both records reach an honest node | RSD-EVD-01f, RSD-SEC-01 |
| CLM-03 | Silent dropping can be proven | Impossible [19] | — | RSD-NET-03 (loss never accuses) |
| CLM-04 | Agents with the same records derive the same Evidence Graph and states | Provable | Deterministic implementation; same policy and parameters (AS12) | RSD-EVD-03 |
| CLM-05 | All honest agents eventually receive the same records | Assumption | AS4 | RSD-EVD-06, RSD-NET-05 |
| CLM-08 | An edge without two-end confirmation never enters the Network Graph | Provable | Signatures; deterministic rule | RSD-NET-02 |
| CLM-12 | Wormholes are detected | Impossible without an independent position root [56], [24] | — | RSD-SEC-02e (M16 records only) |
| CLM-13 | A valid hop tag with an invalid end-to-end tag implies corruption inside a node | Provable | AES-GCM integrity (AS14); hop key only at the two ends | RSD-SEC-05 |
| CLM-17 | f compromised classes shift the fused log-likelihood by at most 2fc | Provable (algebraic) | Correct class assignment | RSD-OBS-01, RSD-OBS-02 |
| CLM-20 | Every member receives the new epoch key | No guarantee (liveness) | Fallback to the next candidate | RSD-CRY-03 |
| CLM-21 | Partitioned components share one epoch | Impossible [55] | — | RSD-CRY-03 (epochs per component) |
| CLM-23 | A compromised member cannot read past or future traffic | No guarantee | The swarm key has no forward or post-compromise secrecy (future work) | RSD-CRY-03 |
| CLM-26 | Every component excludes a proven vehicle | No guarantee (liveness) | The proof must reach the component (AS4) | RSD-ORG-03, RSD-EVD-06 |
| CLM-27 | A support request is never a command | Provable (structural) | Module interfaces (MD14) | RSD-MIS-02 |
| CLM-34 | Under f = 1, a G3 taint lowers a Confirmed contact at most to Corroborated | Conditional | AS1; b ≥ a + c | RSD-SEC-04a |
| CLM-35 | Components in different key epochs can always rediscover each other and merge | Provable (structural) | Constant discovery key; certificates | RSD-NET-02a, RSD-CRY-03b |
| CLM-36 | No AES-GCM IV repeats under one key | Provable (structural) | IV construction rule; AS14 | RSD-CRY-01g |
| CLM-37 | Under the oracle-constrained profile, a compromised agent obtains signatures only on schema-valid records | Provable | AS2 | RSD-CRY-02 |
| CLM-38 | A canonically malformed signed record proves that the key was used outside the provider or that the provider is faulty; it does not prove an exploit or a compromise | Provable | AS13; one validator in every build; one schema version per mission | RSD-CMC-02 |
| CLM-39 | An operator command never changes a security state | Provable (structural) | MD18 | RSD-HOT-01, RSD-ORG-03b |
| CLM-40 | Vehicles with the same command records derive the same command history and apply an immediate ABORT or a RESUME only within its validity | Provable | MD18, MD8; AS5 for windows | RSD-HOT-01 |
| CLM-41 | The simulated LSC holds the vehicle within 5 s of mission-computer silence and rejects every out-of-envelope waypoint | Provable (structural, simulation) | MD19; LSC logic | RSD-SAF-03 |
| CLM-42 | No collision occurs in the simulated runs under the modeled sensors, beacons and procedural separation | Hypothesis | AS20; sensor and actuator models | RSD-SAF-03 (E13, E15) |
| CLM-43 | An exploited ingress parser cannot use keys or command motion | Conditional | AS19, a logical boundary in the simulation | RSD-CMC-01 |
| CLM-44 | A dormant implant that does not act can be detected from behavior | Impossible (by construction) | — | RSD-CMC-01, RSD-CMC-02 and RSD-CMC-04 to RSD-CMC-07 (containment instead of detection) |
| CLM-45 | Operator commands and summaries reach every component | No guarantee (liveness) | AS4 | RSD-HOT-08 |
| CLM-46 | After a regional jam, the collective re-converges within the drain time T_drain (Section 4.10) if vehicles survive and contact returns in time | Hypothesis | AS4 | RSD-JAM-01, RSD-JAM-02 (E15) |
| CLM-47 | Backhaul availability does not enter the derivation of any security state | Provable (structural) | MD8, MD20 | RSD-HOT-07 |
| CLM-48 | Commands reach every vehicle of the target component within 10 s (p95) when the component has a live backhaul at issue, no outage occurs in the next 10 s, the profile is PT and the mesh is the reference mesh | Hypothesis | AS4, AS22 | RSD-HOT-08 (E14) |
| CLM-49 | When every compromised build lies in one root class, each Corroborated (Confirmed) status at an honest agent has at least one (two) supporting classes outside that class; no zero rate of false verification is claimed | Provable | Fusion thresholds (b > 2c); AS9–AS11; RSD-CMC-05a | RSD-CMC-06 |
| CLM-50 | A compromised ingress (stage I) cannot make its agent accept an unverified record | Conditional | AS19, a logical boundary in the simulation | RSD-CMC-01e, RSD-CMC-01f, RSD-CMC-01g |
| CLM-51 | A vehicle whose key and key provider are intact is not quarantined by honest agents, whatever the number of compromised vehicles | Provable | MD7; AS13 | RSD-SEC-01c, RSD-SEC-01e |
| CLM-52 | One compromised class can be prevented from suppressing true contacts | Impossible | Clipping only bounds the shift of the fused value to 2c (CLM-17, LIM-43) | RSD-OBS-01c |

Two consequences follow. First, every security guarantee has a provable part about records and signatures, but its effect in the collective depends on delivery (AS4). This is why RSD-EVD-06 states only the mechanisms that carry proofs, and why no requirement promises that a proof or a command reaches every agent. Second, the requirements on physical safety rest on a hypothesis (CLM-42) and on AS20, so "zero collisions" is a criterion of the simulated runs, not a guarantee.

### Guarantees under the attacker profiles

Table 6.3 shows how the central guarantees behave when the design target of one oracle-constrained attacker per scope is exceeded. The common-mode profile is treated separately in Table 6.4, because what holds under it depends on how the compromised builds fall across the root classes.

**Table 6.3. Central guarantees under the attacker profiles.**

| Guarantee | f = 1, oracle-constrained | f = 2 | Key extracted or cloned | Long partition |
|---|---|---|---|---|
| No quarantine without proof (MD7) | Holds | Holds | Holds; the proof blames the key, so the genuine vehicle is quarantined too | Holds |
| Loss never accuses (MD1) | Holds | Holds | Holds | Holds |
| Equivocation is detected | Holds | Holds | Holds; blames the key | Only after contact |
| Forbidden-type signatures do not appear | Holds | Holds | Broken; M4 detects the signature (G3) | Holds |
| One class cannot reach Corroborated | Holds | Two classes can (shift up to 4c) | Holds per key | Holds |
| M15 and M17 point to the misbehaving node | Holds | G2 against an honest vehicle is possible; never L4 | Holds | Holds |
| Same evidence, same state (MD8) | Holds | Holds | Holds | Holds within each component |
| Proof reaches every agent | Within a component | Within a component | Within a component | Only after contact |
| Vehicles stay inside the safety envelope | Holds under AS20 | Holds under AS20 | Holds under AS20 | Holds under AS20 |

**Table 6.4. Guarantees under correlated compromise.**

| ID | Guarantee | Holds when | Basis | In the simulation and on a real vehicle |
|---|---|---|---|---|
| G-a | A vehicle whose key and key provider are intact is not quarantined or revoked by honest agents | Any number of compromised vehicles and builds, under AS13 | A G3 proof needs the accused key's own signatures (MD7) | Modeled fully; a real vehicle also needs a correct provider, and a faulty provider can get its own vehicle quarantined (LIM-37) |
| G-b | No operator command or GCS action changes a security state | Always | MD18; RSD-ORG-03b, RSD-HOT-01a | Structural in both |
| G-c | Every vehicle stays inside its safety envelope, and a silent mission computer leads to a hold within 5 s | AS20, including the separate LSC source tree and build job | MD19; GT-13 | In the simulation the LSC is a separate build target, a logical boundary only (LIM-32); a real vehicle needs separate hardware, sensors and actuators |
| G-d | Every Corroborated (Confirmed) status at an honest agent has at least one (two) supporting classes outside the compromised class | All compromised builds lie in one root class (RSD-CMC-05a), under AS9–AS11 | Fusion rules of Table 4.7; CLM-49 | Deterministic in the model; bounds honest errors but says nothing about their rate |
| G-e | A G3 proof held by an honest agent reaches another honest agent | Delivery conditions D1–D5; an honest path alone is not sufficient on a lossy channel | Each honest agent keeps the proof, re-broadcasts it once, announces it in every digest and serves pulls; proofs go first and bypass the evidence-plane cap (RSD-EVD-04, RSD-EVD-06) | Modeled with the radio loss model (GT-08, E3); no delivery bound and no guarantee across a partition that never heals; real links add correlated loss and jamming |
| G-f | M15 and M17 attribution is meaningful | At most one compromised vehicle per scope | AS1 | Modeled |
| G-g | No honest vehicle is wrongly restricted | At most one compromised vehicle per scope | AS1; G2 cap of M15 and M17 | Lost under common-mode compromise; the cost is bounded by the last-resort path (LIM-26) |

When the compromised builds span two or more root classes, or px = 1, G-d, G-f and G-g fail: Confirmed can be forged wherever every class present is compromised, tasks can be captured and the mission can be lost. G-a, G-b, G-c and G-e remain, and G-e holds only where honest contacts satisfy D1–D5. If every agent is implanted, only G-c remains.

Guarantee G-e rests on five delivery conditions. None adds a protocol; each names a property that the design already provides or that the channel must have:

- **D1 Connectivity over time.** After the proof exists, a time-ordered chain of contacts between honest agents leads to the receiver; a simultaneous end-to-end path is not required.
- **D2 Fair loss.** During a contact, a digest-and-pull exchange succeeds with a probability that stays above zero, as on a fair-loss link [77]. Losses on a real channel are correlated in time, so independence between attempts is not assumed.
- **D3 Retry by digest and pull.** An honest agent that missed every re-broadcast learns from a neighbor's digest that a proof exists and pulls it. GT-08 checks this rather than assuming it.
- **D4 Channel access and priority.** Each honest agent of a contact gets transmit opportunities, and proofs lead every queue and bypass the evidence-plane cap. A jammer, or an insider flooding beyond its quota, can violate D4.
- **D5 Retention.** Honest agents keep the proof for the whole mission; the evidence set only grows.

Under D1–D5 the proof reaches the receiver by the end of the chain. The time it takes is measured, not bounded.

## 6.3 Known limitations

Table 6.5 lists the known limitations in condensed wording. This is the single place where limitations are described; other sections cite them by identifier. LIM-46 is added only if the optional scenario SC-13 runs, and LIM-48 is reserved for future work.

**Table 6.5. Known limitations.**

| ID | Limitation | Size or condition | Measured by |
|---|---|---|---|
| LIM-01 | Consistent lie seen by one observer | Undetectable; its effect is limited because no decision rests on one source | E6 |
| LIM-02 | Coordinated coverage of an area by attackers | Limited by the number of independent classes per area | E5, E6 |
| LIM-03 | Cut vertex held by an attacker | No disjoint path; only contacts or a repair help | E2 |
| LIM-04 | Pure droppers and the attraction attack | Effective until the avoidance penalty grows | E2 |
| LIM-05 | Colluding pair and wormhole | M16 records only; not attributed | E8 |
| LIM-06 | Two colluders per scope (f = 2) | An honest vehicle can lose a scope temporarily, never be quarantined; the last-resort path limits the network damage | E7 |
| LIM-07 | Collective-wide GNSS spoofing | Relative checks pass; authenticated GNSS is out of scope | E9 |
| LIM-08 | The channel model is not a measured sea; evaporation ducting | Affects the M16 margin and repair predictions | E8, E10 |
| LIM-09 | Radio stand-in: 802.11n at 2.4 GHz for an S-band link | Relative results only | E1 |
| LIM-10 | Binary shadow without diffraction | Wrong observability near the coast | E10 |
| LIM-11 | Cryptographic processing times from a development machine | May not represent the vehicle processor | Micro-benchmark report |
| LIM-12 | No forward secrecy for bundles encrypted to static keys | Old bundles are exposed if a key leaks | Future work |
| LIM-13 | Evidence escape window | Bounded by contact opportunities | E3 |
| LIM-14 | CSMA at long range | Propagation delay of up to about 12 slots at 32 km; carrier sense is late and collisions rise | E1 |
| LIM-15 | Merge storms after long partitions | Drain time of about 310 s after 600 s of partition in the worst case; proofs first | E4 |
| LIM-16 | Dishonest bids within physical limits | Partly visible through M8 and M9 | E12 |
| LIM-17 | Association errors | False support or false contradiction | E5 |
| LIM-18 | Connectivity repair relies on predicted links and stale positions | Wasted moves; both sides may move | E10 |
| LIM-19 | A repair vehicle is a single point of delay at a cut vertex | Delay until the lease ends | E10 |
| LIM-20 | Collaborative verification costs the helping group coverage | Bounded by quotas and its own auction | E11 |
| LIM-21 | The helping group values a request with the requester's estimate | The request carries evidence references | E11 |
| LIM-22 | Comparison with ns-3 baselines crosses layers | Overlay against IP routing; overhead per hop reported | E1 |
| LIM-23 | The attacks that are detected are the attacks that are injected | Hidden-label test, held-out on-off attacker, f = 2 variants | All experiments |
| LIM-24 | No in-mission policy switching | One policy set per mission; human recall only through the bounded operator channel | Design limit |
| LIM-25 | Taint from mission start | A G3 event can lower statuses that rested on the attacker's earlier records (at most one class per contact) | E12 |
| LIM-26 | Last-resort path through a restricted vehicle | The restricted vehicle may drop or delay that traffic | E7 |
| LIM-27 | Dormant implant in memory | Undetectable and not removed during the mission; contained only when it acts | E13 |
| LIM-28 | Schema-valid exploit or logic flaw shared by every build | Diversity changes layout, not logic (px = 1); no proof is produced | E13 |
| LIM-29 | Operator delay; summaries are claims | Information is as old as each vehicle's newest summary; a RESUME may become void; a compromised producer can sign a false summary that headquarters cannot detect | E14 |
| LIM-30 | Long-range links | Radio links beyond the horizon and satellite links are not modeled physically; backhaul profiles are scenario inputs | E14 |
| LIM-31 | Total jamming | Collective mechanisms stop; individual operation and conditional recovery only | E15 |
| LIM-32 | Trust boundaries are logical in the simulation | Key provider, ingress boundary, boot measurement and LSC independence are not hardware or processes; results show their consequences, not their strength | Design limit |
| LIM-33 | Fidelity of the jamming model | No carrier-sense blocking, reactive jamming or near-far effects; relative results only | E15 |
| LIM-34 | Live but compromised mission computer | Mission-level harm inside the safety envelope is not detected by the LSC | E13 |
| LIM-35 | Radio and satellite-terminal firmware, operating-system network stack | Reachable by outsiders; outside the architecture; includes the terminal and modem of a gateway | Design limit |
| LIM-36 | Unauthenticated LSC beacons | A forger can keep a ghost alive, exhaust the beacon-only track table and mislead the LSC about a vehicle whose real position it never received | GT-13; E13 |
| LIM-37 | Key provider fault | A faulty provider can get an innocent vehicle quarantined and revoked; the proof shows key misuse or a fault, not intent | Design limit (AS2) |
| LIM-38 | Visibility | Physical and electromagnetic visibility of vehicles and terminals is not modeled; no result concerns detectability | Design limit |
| LIM-39 | Türksat service performance | No terminal or satellite service is used; latency, availability and outages of PT and P0 are assumed, not measured | E14 |
| LIM-40 | Backhaul abstraction | Dedicated point-to-point links; no shared beam, contention or jitter; rain fade and satellite-link jamming appear only as outages; dropping and delaying by a gateway are detected only by missing receipts and growing age | E14 |
| LIM-41 | Platform class | The 11–15 m USV class is an assumption (AS21); smaller USVs are not represented | Design limit |
| LIM-42 | Reactive adversary | No adversary reacts to vehicle emissions or visibility; jammer positions are scenario inputs | E15 |
| LIM-43 | False misses from one compromised class | Not limited; the class can keep a true contact at Reported, and clipping bounds only its shift of the fused value | GT-11f; E13 |
| LIM-44 | Cross-build transferability px is synthetic | No real exploit is run; every E13 result is a function of px | E13 |
| LIM-45 | All three gateway USVs share build R | A build-R compromise can silence or bias supervision through every gateway at once, without any security effect, because no local decision depends on supervision (RSD-HOT-08a) | E13 cell GW-1 |
| LIM-47 | Evidence convergence after a partition grows with the number of vehicles | Estimates beyond 20 nodes are unverified calculations (RSD-SIM-06) | E4 at 20 nodes |

## 6.4 What this document does not claim

This document does not claim:

- zero false positives, delivery during disconnection, proof of silent dropping, wormhole detection or physical truth from signatures;
- detection of a dormant implant, implementation strength of the trust boundaries or collective operation under total jamming;
- physical safety beyond the simulated runs;
- any measurement of the Türksat service, a satellite terminal or a radio link beyond the horizon (LIM-30, LIM-39);
- that a command, a summary or a proof reaches every vehicle or component (CLM-26, CLM-45);
- that supervision survives a compromise of the build that the gateways share (LIM-45);
- behavior of fleets larger than the 20-node reference configuration, of a collective with another number of groups or of any mission configuration that was not simulated;
- that any requirement has been implemented or verified.

## 6.5 Open values

A TBD marks a value or case that is not yet defined; a TBR marks a value that exists but must be confirmed or replaced. No requirement quotes a guessed number in place of either. Run counts stay provisional until the execution-time gate (Section 5.4), and byte-level encodings are frozen only in Phase 1.

**Table 6.6. Open values (TBD and TBR).**

| ID | Open value | Affects | How it is closed |
|---|---|---|---|
| TBD-01 | Pull-share metric and averaging interval against which the pull budget β is checked | RSD-EVD-04b; experiment E4 | Defined in the E4 experiment plan before the E4 runs |
| TBD-02 | Statistical thresholds of E14: delivery ratio and summary-age quantiles per cell, equivalence margins with and without backhaul, redundant-copy margin, supervision share of the mesh channel | RSD-HOT-08a; Table 4.31; experiment E14 | Registered in the pre-registration record before the E14 runs |
| TBR-01 | ML-DSA-87 signing (≈ 0.96 ms) and verification (≈ 0.51 ms) times on the development machine | Section 4.5; Table 4.30; the timing table of RSD-SIM-01b | Replaced by the Phase 1 micro-benchmark values; LIM-11 remains |

## 6.6 Optional items and future work

The items in this section are not requirements. An optional item runs only if the execution-time gate passes for the full core plan and gate G5 is met on schedule; the team records each selection. A selected item adds results, never a requirement. Future work is outside the scope of the graduation project, and its reserved identifiers are not added to the architecture. The A/B/C grouping of Fig. 1.1 remains the reference configuration. Dynamic group membership during the mission is excluded because it would violate MD15.

### Optional items

**Table 6.7. Optional items.**

| Item | Content | Acceptance if adopted |
| --- | --- | --- |
| SC-0, SC-7 and SC-8 | SC-0, the baseline mission (30 runs); SC-7, uneven observation capacity, as a 2 × 2 matched design that reuses the first 10 SC-0 seeds (30 runs); SC-8, partition and recovery at 300, 600 and 900 s (30 runs) | As registered before the runs; the SC-8 drain time is judged against the 20 % threshold of E4 |
| GT-11e with hedged signing | Repetition of GT-11e with hedged ML-DSA-87 signing | The same verdict as the run with deterministic signing |
| Reproducible signed builds | Agent and LSC artifacts reproducibly built and signed with provenance | Identical hashes from two builds |
| Beacon decoder model checking | LSC beacon decoder free of out-of-bounds access and undefined behavior for every 24-byte input | Bounded model checking finds no counterexample |
| Optional E13 cells | 18 cells: per-group at px 0.1 and 0.5, per-vehicle ablation, stages I and III, entry paths, dormant cells, build-wide start, GW-3 to GW-5; the first to be dropped if the execution-time gate fails | As registered before the runs |
| Gateway build split | Gateway USVs on two radar builds | — |
| Build tag in SIMDIS | Build and class tag shown in SIMDIS (display only; no claim depends on it) | — |
| SC-13 | Geographic sensitivity: island and land mask scaled by 1.5; a limitation that the results come from a synthetic geography (LIM-46) is added only if SC-13 runs | As registered before the runs |

### Future work

**Table 6.8. Future work.**

| Topic | Why it is not in this project |
|---|---|
| Group Design Procedure (GDP), class coverage ρ, three-valued result, circular partition, planned gateway status | The A/B/C grouping stays the reference |
| Generalized G-group configuration | Relevant only once the group count varies |
| EXPLORE need type and its no-waypoint rule; GDP priority weights and in-mission multiplier | Follow the GDP scope; operator PRIORITY already exists |
| Gateway status DIRECT / DEPENDENT / NONE | All reference groups contain one designated gateway |
| Policy adaptation above the 10 % evidence cap; copy cap beyond four gateways | Relevant only beyond about 49 nodes or more than four gateways |
| Full adaptive-grouping experiment matrix (E16) with its gate test GT-16, and the claims and limitation reserved for it (CLM-53 to CLM-55, LIM-48) | Disproportionate to the central demonstration |
| CBBA or other multi-round allocation | The single-round auction with a commit step is enough for 20 vehicles |
| Message ferrying [34]; contact graph routing [33] and Spray-and-Wait [30]; conditional checkpoints; multi-vehicle repair chains | In the reference scenario, gaps come from the island's shadow, which one repair position closes; bundles wait at the source |
| DURABLE receipt; forward secrecy for bundles; contributive group keying | Durability is derived from digests, because a storage commitment proves storage only when challenged [31]; static-key encapsulation; MLS needs a sequencer for its commits [78] |
| Semantic fusion; language-model incident report | Not needed for the reference contacts; not bit-reproducible |
| MPR flooding, Suurballe pairs, two-tier routing, 32- and 64-node scaling | Not needed for 20 nodes |
| Expected information gain; diffraction terrain model; joint probabilistic association | Simpler models suffice and are easier to validate |
| Threshold-signed authority; in-mission policy switching; dynamic regrouping | Partition reachability; one policy set per mission; fixed membership |
| Long-range radio links; real isolation, hardware roots and diversified builds; periodic restart for long missions; run-time attestation; waveform-level anti-jam models; collective attestation; LSC beacon authentication | Outside what the single-process ns-3 simulation can represent |
| Physical SATCOM validation with a real terminal and service; shore line-of-sight backhaul and dynamic gateway selection | Backhauls are abstract profiles; the designated-gateway design keeps the link-state announcement unchanged |

# 7 References

The list contains every work cited in this document, numbered in order of first citation.

@@REFS
ns-3 Project, "ns-3 network simulator," release ns-3.49, Model Library documentation and source code, https://www.nsnam.org.
IEEE, "IEEE Standard for Information Technology — Telecommunications and Information Exchange between Systems — Local and Metropolitan Area Networks — Specific Requirements — Part 11: Wireless LAN Medium Access Control (MAC) and Physical Layer (PHY) Specifications," IEEE Std 802.11-2020, 2021.
J. Lloyd et al., "Botan: Crypto and TLS for modern C++," release 3.13.0, https://botan.randombit.net.
National Security Agency, "Announcing the Commercial National Security Algorithm Suite 2.0," Cybersecurity Advisory, Sep. 2022, rev. May 2025, https://media.defense.gov/2025/May/30/2003728741/-1/-1/0/CSA_CNSA_2.0_ALGORITHMS.PDF.
NIST, "Module-Lattice-Based Key-Encapsulation Mechanism Standard," FIPS 203, 2024, doi: 10.6028/NIST.FIPS.203.
NIST, "Module-Lattice-Based Digital Signature Standard," FIPS 204, 2024, doi: 10.6028/NIST.FIPS.204.
M. Dworkin, "Recommendation for block cipher modes of operation: Galois/Counter Mode (GCM) and GMAC," NIST SP 800-38D, 2007, doi: 10.6028/NIST.SP.800-38D.
NIST, "Secure Hash Standard (SHS)," FIPS 180-4, 2015, doi: 10.6028/NIST.FIPS.180-4.
H. Krawczyk, M. Bellare and R. Canetti, "HMAC: Keyed-hashing for message authentication," RFC 2104, 1997, doi: 10.17487/RFC2104.
H. Krawczyk and P. Eronen, "HMAC-based extract-and-expand key derivation function (HKDF)," RFC 5869, 2010, doi: 10.17487/RFC5869.
U.S. Naval Research Laboratory, "SIMDIS SDK," release 1.25, GitHub repository, https://github.com/USNavalResearchLaboratory/simdissdk.
UPPAAL model checker, version 5.0.0, 2023, https://uppaal.org.
G. Behrmann, A. David and K. G. Larsen, "A tutorial on UPPAAL," in Formal Methods for the Design of Real-Time Systems (SFM-RT 2004), Lecture Notes in Computer Science, vol. 3185, Springer, 2004, pp. 200–236.
O. Ceviz, S. Sen and P. Sadioglu, "A survey of security in UAVs and FANETs: Issues, threats, analysis of attacks, and solutions," arXiv:2306.14281, 2023, doi: 10.48550/arXiv.2306.14281.
I. Bae and J. Hong, "Survey on the developments of unmanned marine vehicles: Intelligence and cooperation," Sensors, vol. 23, no. 10, Art. no. 4643, 2023, doi: 10.3390/s23104643.
S. Rose, O. Borchert, S. Mitchell and S. Connelly, "Zero trust architecture," NIST SP 800-207, 2020, doi: 10.6028/NIST.SP.800-207.
J.-H. Cho, A. Swami and I.-R. Chen, "A survey on trust management for mobile ad hoc networks," IEEE Communications Surveys & Tutorials, vol. 13, no. 4, pp. 562–583, 2011, doi: 10.1109/SURV.2011.092110.00088.
Y. Chae, L. DiPippo and Y. L. Sun, "Trust management for defending on-off attacks," IEEE Transactions on Parallel and Distributed Systems, vol. 26, no. 4, pp. 1178–1191, 2015, doi: 10.1109/TPDS.2014.2317719.
A. Haeberlen, P. Kouznetsov and P. Druschel, "PeerReview: Practical accountability for distributed systems," in Proc. 21st ACM Symposium on Operating Systems Principles (SOSP '07), 2007, pp. 175–188, doi: 10.1145/1294261.1294279.
S. Marti, T. J. Giuli, K. Lai and M. Baker, "Mitigating routing misbehavior in mobile ad hoc networks," in Proc. 6th Annual International Conference on Mobile Computing and Networking (MobiCom 2000), 2000, pp. 255–265, doi: 10.1145/345910.345955.
C. Perkins, E. Belding-Royer and S. Das, "Ad hoc on-demand distance vector (AODV) routing," RFC 3561, 2003, doi: 10.17487/RFC3561.
T. Clausen and P. Jacquet, "Optimized link state routing protocol (OLSR)," RFC 3626, 2003, doi: 10.17487/RFC3626.
T. Clausen, C. Dearlove, P. Jacquet and U. Herberg, "The optimized link state routing protocol version 2," RFC 7181, 2014, doi: 10.17487/RFC7181.
T. Clausen, U. Herberg and J. Yi, "Security threats to the optimized link state routing protocol version 2 (OLSRv2)," RFC 8116, 2017, doi: 10.17487/RFC8116.
U. Herberg, C. Dearlove and T. Clausen, "Integrity protection for the Neighborhood Discovery Protocol (NHDP) and Optimized Link State Routing Protocol version 2 (OLSRv2)," RFC 7183, 2014, doi: 10.17487/RFC7183.
A. Reinhardt, D. Seither, A. König, R. Steinmetz and M. Hollick, "Protecting IEEE 802.11s wireless mesh networks against insider attacks," in Proc. 37th IEEE Conference on Local Computer Networks (LCN 2012), 2012, pp. 224–227.
K. Fall, "A delay-tolerant network architecture for challenged internets," in Proc. ACM SIGCOMM, 2003, pp. 27–34.
V. Cerf et al., "Delay-tolerant networking architecture," RFC 4838, 2007, doi: 10.17487/RFC4838.
S. Burleigh, K. Fall and E. Birrane, "Bundle protocol version 7," RFC 9171, 2022, doi: 10.17487/RFC9171.
T. Spyropoulos, K. Psounis and C. S. Raghavendra, "Spray and wait: An efficient routing scheme for intermittently connected mobile networks," in Proc. 2005 ACM SIGCOMM Workshop on Delay-Tolerant Networking (WDTN '05), 2005, pp. 252–259.
G. Ateniese, R. Burns, R. Curtmola, J. Herring, L. Kissner, Z. Peterson and D. Song, "Provable data possession at untrusted stores," in Proc. ACM Conference on Computer and Communications Security (CCS '07), 2007, pp. 598–609, doi: 10.1145/1315245.1315318.
V. Paxson, M. Allman, J. Chu and M. Sargent, "Computing TCP's retransmission timer," RFC 6298, 2011, doi: 10.17487/RFC6298.
J. A. Fraire, O. De Jonckère and S. C. Burleigh, "Routing in the space internet: A contact graph routing tutorial," Journal of Network and Computer Applications, vol. 174, Art. no. 102884, 2021, doi: 10.1016/j.jnca.2020.102884.
W. Zhao, M. Ammar and E. Zegura, "A message ferrying approach for data delivery in sparse mobile ad hoc networks," in Proc. 5th ACM International Symposium on Mobile Ad Hoc Networking and Computing (MobiHoc '04), 2004, pp. 187–198, doi: 10.1145/989459.989483.
B. Kailkhura, Y. S. Han, S. Brahma and P. K. Varshney, "Asymptotic analysis of distributed Bayesian detection with Byzantine data," IEEE Signal Processing Letters, vol. 22, no. 5, pp. 608–612, 2015, doi: 10.1109/LSP.2014.2365196.
B. P. Gerkey and M. J. Matarić, "A formal analysis and taxonomy of task allocation in multi-robot systems," The International Journal of Robotics Research, vol. 23, no. 9, pp. 939–954, 2004, doi: 10.1177/0278364904045564.
G. A. Korsah, A. Stentz and M. B. Dias, "A comprehensive taxonomy for multi-robot task allocation," The International Journal of Robotics Research, vol. 32, no. 12, pp. 1495–1512, 2013, doi: 10.1177/0278364913496484.
M. B. Dias, R. Zlot, N. Kalra and A. Stentz, "Market-based multirobot coordination: A survey and analysis," Proceedings of the IEEE, vol. 94, no. 7, pp. 1257–1270, 2006.
F. Quinton, C. Grand and C. Lesire, "Market approaches to the multi-robot task allocation problem: A survey," Journal of Intelligent & Robotic Systems, vol. 107, Art. no. 29, 2023, doi: 10.1007/s10846-022-01803-0.
H.-L. Choi, L. Brunet and J. P. How, "Consensus-based decentralized auctions for robust task allocation," IEEE Transactions on Robotics, vol. 25, no. 4, pp. 912–926, 2009, doi: 10.1109/TRO.2009.2022423.
V. Sao, T. D. Ho, S. Bhore and P. B. Sujit, "Event driven CBBA with reduced communication," in Proc. 2025 International Conference on Unmanned Aircraft Systems (ICUAS), 2025, doi: 10.1109/ICUAS65942.2025.11007849.
L. B. Johnson, S. S. Ponda, H.-L. Choi and J. P. How, "Asynchronous decentralized task allocation for dynamic environments," in Proc. AIAA Infotech@Aerospace 2011, St. Louis, MO, USA, 2011, doi: 10.2514/6.2011-1441.
N. Buckman, J. P. How and H.-L. Choi, "Partial replanning for decentralized dynamic task allocation," in Proc. AIAA SciTech 2019 Forum, 2019, doi: 10.2514/6.2019-0915.
M. M. Zavlanos, M. B. Egerstedt and G. J. Pappas, "Graph-theoretic connectivity control of mobile robot networks," Proceedings of the IEEE, vol. 99, no. 9, pp. 1525–1540, 2011, doi: 10.1109/JPROC.2011.2157884.
M. Antonakakis, T. April, M. Bailey et al., "Understanding the Mirai botnet," in Proc. 26th USENIX Security Symposium, 2017, pp. 1093–1110.
CISA, NSA, FBI et al., "PRC state-sponsored actors compromise and maintain persistent access to U.S. critical infrastructure," Joint Cybersecurity Advisory AA24-038A, Feb. 2024, https://www.cisa.gov/news-events/cybersecurity-advisories/aa24-038a.
The MITRE Corporation, "Lateral Movement, Tactic TA0008," MITRE ATT&CK Enterprise Matrix, https://attack.mitre.org/tactics/TA0008/.
R. Ross, V. Pillitteri, R. Graubart, D. Bodeau and R. McQuaid, "Developing cyber-resilient systems: A systems security engineering approach," NIST SP 800-160 Vol. 2 Rev. 1, 2021, doi: 10.6028/NIST.SP.800-160v2r1.
P. Larsen, A. Homescu, S. Brunthaler and M. Franz, "SoK: Automated software diversity," in Proc. 2014 IEEE Symposium on Security and Privacy, 2014, pp. 276–291, doi: 10.1109/SP.2014.25.
B. Cox, D. Evans, A. Filipi, J. Rowanhill, W. Hu, J. Davidson, J. Knight, A. Nguyen-Tuong and J. Hiser, "N-variant systems: A secretless framework for security through diversity," in Proc. 15th USENIX Security Symposium, Vancouver, Canada, 2006.
F. Junqueira, R. Bhagwan, A. Hevia, K. Marzullo and G. M. Voelker, "Surviving Internet catastrophes," in Proc. USENIX Annual Technical Conference, 2005, pp. 45–60.
L. Sha, "Using simplicity to control complexity," IEEE Software, vol. 18, no. 4, pp. 20–28, 2001, doi: 10.1109/MS.2001.936213.
K. Fisher, J. Launchbury and R. Richards, "The HACMS program: Using formal methods to eliminate exploitable bugs," Philosophical Transactions of the Royal Society A, vol. 375, no. 2104, Art. no. 20150401, 2017, doi: 10.1098/rsta.2015.0401.
H. Birkholz, D. Thaler, M. Richardson, N. Smith and W. Pan, "Remote ATtestation procedureS (RATS) architecture," RFC 9334, 2023, doi: 10.17487/RFC9334.
S. Gilbert and N. Lynch, "Brewer's conjecture and the feasibility of consistent, available, partition-tolerant web services," ACM SIGACT News, vol. 33, no. 2, pp. 51–59, 2002, doi: 10.1145/564585.564601.
Y.-C. Hu, A. Perrig and D. B. Johnson, "Packet leashes: A defense against wormhole attacks in wireless networks," in Proc. IEEE INFOCOM 2003, vol. 3, 2003, pp. 1976–1986, doi: 10.1109/INFCOM.2003.1209219.
L. Lamport, R. Shostak and M. Pease, "The Byzantine generals problem," ACM Transactions on Programming Languages and Systems, vol. 4, no. 3, pp. 382–401, 1982.
J. R. Douceur, "The Sybil attack," in Peer-to-Peer Systems (IPTPS 2002), Lecture Notes in Computer Science, vol. 2429, Springer, 2002, pp. 251–260, doi: 10.1007/3-540-45748-8_24.
M. Shapiro, N. Preguiça, C. Baquero and M. Zawirski, "Conflict-free replicated data types," in Stabilization, Safety, and Security of Distributed Systems (SSS 2011), Lecture Notes in Computer Science, vol. 6976, Springer, 2011, pp. 386–400, doi: 10.1007/978-3-642-24550-3_29.
R. C. Merkle, "A digital signature based on a conventional encryption function," in Advances in Cryptology — CRYPTO '87, Lecture Notes in Computer Science, vol. 293, Springer, 1988, pp. 369–378.
D. S. J. De Couto, D. Aguayo, J. Bicket and R. Morris, "A high-throughput path metric for multi-hop wireless routing," in Proc. 9th Annual International Conference on Mobile Computing and Networking (MobiCom '03), 2003, pp. 134–146.
J. Llinas, C. Bowman, G. Rogova, A. Steinberg, E. Waltz and F. White, "Revisiting the JDL data fusion model II," in Proc. 7th International Conference on Information Fusion (FUSION 2004), Stockholm, Sweden, 2004, vol. 2, pp. 1218–1230.
IEC, "Functional safety of electrical/electronic/programmable electronic safety-related systems — Part 6: Guidelines on the application of IEC 61508-2 and IEC 61508-3," IEC 61508-6:2010, Ed. 2.0, 2010.
A. Hornung, K. M. Wurm, M. Bennewitz, C. Stachniss and W. Burgard, "OctoMap: An efficient probabilistic 3D mapping framework based on octrees," Autonomous Robots, vol. 34, no. 3, pp. 189–206, 2013, doi: 10.1007/s10514-012-9321-0.
ISO/IEC/IEEE, "Systems and software engineering — Life cycle processes — Requirements engineering," ISO/IEC/IEEE 29148:2018, 2018.
Foundation for Intelligent Physical Agents, "FIPA Contract Net interaction protocol specification," Document SC00029H, 2002, http://www.fipa.org/specs/fipa00029/SC00029H.html.
R. Barnes, K. Bhargavan, B. Lipp and C. Wood, "Hybrid public key encryption," RFC 9180, 2022, doi: 10.17487/RFC9180.
T. Abera, N. Asokan, L. Davi, J.-E. Ekberg, T. Nyman, A. Paverd, A.-R. Sadeghi and G. Tsudik, "C-FLAT: Control-flow attestation for embedded systems software," in Proc. 2016 ACM SIGSAC Conference on Computer and Communications Security (CCS '16), 2016, pp. 743–754, doi: 10.1145/2976749.2978358.
H. Shacham, M. Page, B. Pfaff, E.-J. Goh, N. Modadugu and D. Boneh, "On the effectiveness of address-space randomization," in Proc. 11th ACM Conference on Computer and Communications Security (CCS '04), 2004, pp. 298–307, doi: 10.1145/1030083.1030124.
K. Z. Snow, F. Monrose, L. Davi, A. Dmitrienko, C. Liebchen and A.-R. Sadeghi, "Just-in-time code reuse: On the effectiveness of fine-grained address space layout randomization," in Proc. 2013 IEEE Symposium on Security and Privacy, 2013, pp. 574–588, doi: 10.1109/SP.2013.45.
M. Garcia, A. Bessani, I. Gashi, N. Neves and R. Obelheiro, "Analysis of operating system diversity for intrusion tolerance," Software: Practice and Experience, vol. 44, pp. 735–770, 2014, doi: 10.1002/spe.2180.
P. Sousa, A. N. Bessani, M. Correia, N. F. Neves and P. Veríssimo, "Highly available intrusion-tolerant services with proactive-reactive recovery," IEEE Transactions on Parallel and Distributed Systems, vol. 21, no. 4, pp. 452–465, 2010.
P. L'Ecuyer, "Good parameters and implementations for combined multiple recursive random number generators," Operations Research, vol. 47, no. 1, pp. 159–164, 1999.
F. Wilcoxon, "Individual comparisons by ranking methods," Biometrics Bulletin, vol. 1, no. 6, pp. 80–83, 1945.
S. Holm, "A simple sequentially rejective multiple test procedure," Scandinavian Journal of Statistics, vol. 6, no. 2, pp. 65–70, 1979.
M. Shand and S. Bryant, "A framework for loop-free convergence," RFC 5715, 2010, doi: 10.17487/RFC5715.
C. Cachin, R. Guerraoui and L. Rodrigues, Introduction to Reliable and Secure Distributed Programming, 2nd ed. Berlin, Germany: Springer, 2011, doi: 10.1007/978-3-642-15260-3.
R. Barnes, B. Beurdouche, R. Robert, J. Millican, E. Omara and K. Cohn-Gordon, "The Messaging Layer Security (MLS) protocol," RFC 9420, 2023, doi: 10.17487/RFC9420.
@@ENDREFS
