# TULPAR byte-level encodings: proposal

**Status:** DRAFT, awaiting approval (Özgür Can Güngör, Cemre Karabulut; list bounds of the Group Status Summary: Berra). No code is written against this document before approval.
**Issue:** TULPAR-160, "Freeze byte-level encodings of records, bundles and messages".
**Sources:** RSD v1.1, Table 4.26 (Section 4.9), Table 2.1, Table 4.29, Sections 4.3, 4.4, 4.5, 4.6, 4.8, 5.1 and 6.5.

The RSD leaves the byte level to Phase 1 on purpose: *"Field sizes are given where the architecture gives them; byte-level encodings are frozen in Phase 1."* (note to Table 4.26). This document is the proposal for that freeze. All sizes and channel shares below are calculations from the proposed layouts, not measurements.

## 1. Markers

| Marker | Meaning |
|---|---|
| ✔ | Size stated in Table 4.26. |
| † | Size derived from another RSD rule, cited in the row. Needs approval. |
| proposal | The RSD gives no size; this document proposes one. Needs approval. |
| added field | Not in Table 4.26, but another RSD requirement makes it necessary. |
| assumption | A meaning or algorithm the RSD does not state; this document assumes it. |

Every proposal row names the alternative width and why it was not chosen.

## 2. Canonical encoding rules

These rules apply to every record, bundle and frame. A signed record that violates the canonical encoding is a G3 proof (RSD-CMC-02a), so each rule maps to an exact check whose failure yields "malformed".

| No | Rule (proposal) | Rationale | Alternative and why not |
|---|---|---|---|
| K1 | All integers are big-endian (network byte order). | ns-3 `Buffer::Iterator::WriteHtonU*` convention; the existing `HelloHeader` uses it. | Little-endian: native on x86, but departs from the ns-3 header convention and makes PHY traces harder to read. |
| K2 | Only fixed-width integers (u8, u16, u32, u64; i32 for positions). No variable-length integers. | One value has one encoding; non-canonical varint forms (extra zero bytes) cannot arise (RSD-CMC-02a). | LEB128 varint: saves a few bytes, but needs a "shortest form" check on every field and complicates fixed size limits (RSD-CMC-01c). |
| K3 | No floating point. Continuous quantities are quantized to integers with a stated scale. The producer quantizes once; the receiver never re-quantizes. | RSD-EVD-03b forbids "platform-dependent rounding"; the event log also uses integer nanoseconds. | IEEE-754 binary32/64: NaN, -0 and multiple bit patterns break canonicity. |
| K4 | Quantization is `q = llround(x * scale)` (halves rounded away from zero), then saturated to the field's range. | `std::llround` gives the same result on every platform for IEEE-754 doubles. | `floor`: systematic bias on negative coordinates. |
| K5 | Lists are a count (u8; u16 where needed) followed by the items. Every list type has a fixed maximum count; exceeding it is malformed. | RSD-CMC-01c: "Every ingress parser shall enforce fixed size limits on its input." | Sentinel terminator: the size limit can only be checked at the end of parsing. |
| K6 | Lists with set semantics (parents, common-cause tags, parent epochs, basis) are encoded in strictly ascending order with no duplicates. | The same set encodes to one byte string, so the identifier follows the content (RSD-EVD-02a). | Producer order: the same content would yield two identifiers. |
| K7 | Variable-length byte strings carry a u16 length prefix; the embedded proof carries a u32 length prefix. | No record exceeds 65 535 bytes. An equivocation proof holds two bundles (about 11 kB; "a G3 proof of about 10.5 kB", Section 4.10). | u32 everywhere: 2 bytes wasted per record. |
| K8 | Optional field: a 1-byte presence flag (0 or 1), followed by the value only if the flag is 1. Any other flag value is malformed. | One encoding. | "All zeros means absent": can collide with a valid value and wastes 48 bytes for a hash field. |
| K9 | Enumerations are u8; an undefined value is malformed. | The RSD's closed sets (RSD-HOT-02a, RSD-OBS-03a). | u16: the closed sets are far below 256 values. |
| K10 | Roster sets are bitmaps of ceil(N/8) bytes (N: roster size in the provisioning package); bit i = roster index i, most significant bit first in each byte. Unused padding bits must be 0. | At most 20 nodes (Section 1.4); 3 bytes for N = 20. Digest budget about 370 B (Table 4.29). | List of u16 identifiers: up to 41 bytes for 20 nodes. |
| K11 | Every encoding is consumed exactly. Trailing bytes or a length mismatch reject the input. | One byte string has one parse (RSD-CMC-02b: every build gives the same verdict). | Ignoring trailing bytes: one record gets several byte strings, which breaks signed-equivocation detection. |
| K12 | Hash and signature inputs are prefixed with an ASCII domain label, e.g. `"TULPAR/1/record"`, `"TULPAR/1/bundle"`, `"TULPAR/1/hello"`. | A record signature cannot be reused as a frame or handshake signature. | No label: cross-protocol confusion is left to the luck of the layouts. |
| K13 | Record identifier = SHA-384(`"TULPAR/1/record"` ‖ canonical encoding of all fields except the identifier). | RSD-EVD-02a: "the SHA-384 hash of the canonical encoding of all its other fields". | None (set by the RSD). |

## 3. Common types

| Type | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| NodeId (origin, sender, vehicle, producer) | 2 | u16 roster index; 0xFFFF reserved | proposal | Vehicle id is 2 B in the LSC beacon (RSD-SAF-02d); the existing `MeshHello::sender` is u16. 0xFFFF is reserved for broadcast and for headquarters. | u8: enough for 20 nodes, but inconsistent with the beacon and closes off the larger-fleet future work of Section 6.6. |
| Hash48 | 48 | bytes | † | SHA-384 digest is 48 B (Table 2.1, RSD-CRY-01d). | None. |
| RecordId | 48 | Hash48 | † | RSD-EVD-02a. | 16-byte truncation: the identifier is a content address and would be open to birthday collisions. |
| Signature | 4627 | bytes | ✔ / † | ML-DSA-87 signature is 4627 B (Table 2.1, Table 4.26, RSD-CRY-01b). | None. |
| MlDsaPublicKey | 2592 | bytes | † | Table 2.1. | None. |
| MlKemEncapsKey, MlKemCiphertext | 1568 | bytes | ✔ / † | Table 2.1, RSD-CRY-01e. | None. |
| GcmTag | 16 | bytes | ✔ | Table 2.1, Table 4.26 (overlay header "16-byte tag"). | None. |
| Window | 4 | u32 window index from mission start | proposal | The window is 5 s (Table 1.7); the reference mission of 30 min has 360 windows, but the mission configuration sets the length. | u16: enough for 91 hours, but the RSD does not bound mission length; 2 bytes cost less than a wrap check for the field's lifetime. |
| SimTime | 8 | u64 simulation time, integer nanoseconds | proposal | Event-log rule (CLAUDE.md: "integer nanoseconds"); RSD-SIM-01. | u32 milliseconds: smaller, but introduces a second time unit. |
| EpochId | 6 | u16 producer NodeId ‖ u32 record sequence number of that producer's epoch announcement | proposal | Producers in partitioned components can create epochs at the same time, so the id must be globally unique (parent-epoch comparison of RSD-CRY-03h). The same (producer, sequence) pair in two different records is itself an equivocation (G3, RSD-SEC-01d). Initial epoch (provisioning) = 0xFFFF ‖ 0. Not a SHA-384 value. | u32 counter: two components produce the same number. 48 B RecordId: 42 B more in every record header and in HELLO. 8-byte hash prefix: can be ground to a collision in about 2^32 operations. |
| CommandId | 6 | u16 console id ‖ u32 console sequence | proposal | Table 4.26: "command id (console, console sequence)". | u16 sequence: may suffice for one console's lifetime, but a wrap breaks the deduplication of RSD-HOT-06g. |
| CertIndex | 2 | u16 index into the provisioning package's certificate list | proposal | The provisioning package is fixed and signed for the whole mission (RSD-ORG-05a). | 48 B certificate hash: unique, but 46 B more per reference, and appears twice in an operator command. |
| Position | 12 | 3 × i32: x east, y north, z up; local ENU relative to the origin in the mission configuration; centimeters | proposal | The LSC beacon has "position and altitude (12 B)" (RSD-SAF-02d), i.e. 3 × 4 B. ns-3 MobilityModel gives Cartesian meters. i32 cm covers ±21 474 km; the Marmara area is about 17 × 10 km. The M8 kinematic check can use integers without trigonometry (RSD-EVD-03b). | Latitude/longitude i32 at 1e-7 degree (about 1.1 cm): distances need trigonometry and are exposed to platform-dependent rounding. Decimeters: no gain in size. |
| PositionSource | 1 | u8: 0 GNSS, 1 radar fix, 2 mutual tracking, 3 other | ✔ set / proposal codes | Closed set of RSD-OBS-03a. | None. |
| ErrorBound | 2 | u16 decimeters, saturating at 6553.5 m | proposal | GNSS error is on the order of meters, radar-fix error tens of meters; 0.1 m resolution is enough. | u32 cm: 2 B more and resolution beyond sensor accuracy. |
| Etx | 2 | u16 Q8.8 fixed point (value × 256), saturating at 65535 | proposal | ETX ≥ 1; no floating point (K3). | u8 Q4.4: 1/16 resolution increases ties in route selection. |
| SchemaVersion | 2 | u16 | added field | RSD-CMC-02c: "A record that carries a schema version other than the mission's shall be rejected". Table 4.26 does not list this field. | u8: enough, but an overflow of the version field later would break the frozen format; the cost is 1 B. |

## 4. Records and frames

Fields are listed in the order of Table 4.26. Added fields are placed at the start.

### 4.1 Common record header (RSD-EVD-02, RSD-EVD-05, RSD-OBS-03)

| Field | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| schema version | 2 | SchemaVersion | added field | RSD-CMC-02c. | See Section 3. |
| identifier | 0 (not on the wire) | RecordId | † (48 B) | The receiver computes it by K13. Not carrying it removes the separate failure "carried identifier does not match the content". | Carry it: 48 B per record, and it is undefined whether a mismatch is G3. **Decision needed (S1).** |
| origin | 2 | NodeId | proposal | Section 3. | None. |
| record sequence number | 4 | u32 | proposal | Position in the origin's chain. 360 windows in the reference mission; hundreds of records per window are possible. | u16: 65 535 records / 360 windows = 182 per window; overflows in longer missions. u64: 4 B more. |
| previous-record hash | 48 | Hash48 | † | SHA-384 (RSD-CRY-01d); chain integrity (RSD-EVD-05a). | Truncated hash: chain integrity is a direct security property. |
| window | 4 | Window | proposal | Section 3. | None. |
| epoch | 6 | EpochId | proposal | Section 3. | None. |
| type | 1 | u8 record type code | proposal | Closed set; adding a type is a schema version change anyway (RSD-CMC-02c). About 30 types today. | u16: nowhere near 256 types. |
| uniqueness key | 16 | bytes: first 16 bytes of SHA-384(type-specific key fields) | proposal, assumption | **The RSD does not define this field's meaning.** Assumption: the triple (origin, type, uniqueness key) names one logical record; two different records with the same triple either form a supersedes relation or an equivocation. It is compared only among one origin's own records, so truncation cannot harm another vehicle. | 48 B: 32 B more in every record. 8 B: accidental collision among the origin's own records after about 2^32 records. **Meaning needs approval (S2).** |
| supersedes | 1 + 0/48 | optional RecordId (K8) | proposal | A corrected bid supersedes the earlier one (Section 4.2, task allocation). | Always 48 B: empty in most records. |
| parents | 1 + 48·n | u8 count + RecordId list, K6, n ≤ 16 | proposal | Dependency edges (Table 1.7, Evidence Graph). | u16 count: more than 255 parents is needed nowhere in the RSD. **The bound 16 is a proposal; the RSD gives no value (S3).** |
| common-cause tags | 1 + 2·n | u8 count + u16 tag codes (index into the mission configuration's tag table), K6, n ≤ 8 | proposal | RSD-OBS-03d: tags come from the mission configuration; the record carries only indices. | Text tags: variable length and canonicity problems. **The bound 8 is a proposal (S3).** |
| policy hash | 48 | Hash48 | † | SHA-384; Table 4.26 "policy hash". | None. |

Fixed part without parents, supersedes or tags: 2+2+4+48+4+6+1+16+1+1+1+48 = **134 B**.

The type-specific body follows the header. A record carries no signature of its own; it is covered by its bundle's signature (RSD-EVD-01a).

### 4.2 Signed bundle (RSD-EVD-01, RSD-CRY-01b)

| Field | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| schema version | 2 | SchemaVersion | added field | RSD-CMC-02c, early rejection at bundle level. | Only in records: loses the cheap check before signature verification (RSD-EVD-01g). |
| origin | 2 | NodeId | proposal | None. | None. |
| window | 4 | Window | proposal | None. | None. |
| first record sequence | 4 | u32 | proposal | Same width as the record sequence number. | None. |
| last record sequence | 4 | u32 | proposal | Record count = last − first + 1; no separate count. Records are consecutive; every sequence number in the range appears exactly once. | Separate count field: carries the same information twice and opens a mismatch case. |
| Merkle root | 48 | Hash48 | † | SHA-384. Tree rule in the style of RFC 9162: leaf = SHA-384(0x00 ‖ record), inner node = SHA-384(0x01 ‖ left ‖ right), a lone node is promoted to the next level. | Flat list hash: pull at fragment granularity (RSD-EVD-01d) could not be verified per record. |
| records | Σ (2 + Lᵢ) | per record a u16 length + canonical record (K7) | proposal | None. | None. |
| signature | 4627 | Signature | ✔ | Signed message = `"TULPAR/1/bundle"` ‖ all fields except the signature. Matches RSD-EVD-01a "the header, the records and their Merkle root under one signature" literally. | Header and root only: cryptographically sufficient, but the RSD-EVD-01a acceptance test says the signature "covers ... the records". |

Fixed part (records excluded): 2+2+4+4+4+48+4627 = **4691 B**. For the 5.5 kB bundle of the budget, about 800 B remain for records. A 5 500 B bundle is four fragments of at most 1 400 B (Section 4.4), as Table 4.29 assumes.

### 4.3 Digest (RSD-EVD-01c, RSD-EVD-03)

A digest is not a record; it is a frame payload under the swarm key (RSD-CRY-03c).

| Field | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| window | 4 | Window | added field (proposal) | RSD-EVD-03c logs the divergence "with both hashes and the window". Table 4.26 does not list this field. | Deriving the window from reception time: reception time must not enter evaluation (RSD-EVD-03b). |
| origin set | ceil(N/8) | roster bitmap (K10) | proposal | Shows which origins have an entry; entries follow in ascending roster index. | u16 origin per entry: 40 B more for 20 origins. |
| per origin: highest record sequence | 4 | u32 | proposal | Same as the record sequence number. | None. |
| per origin: chain-head prefix | 8 | bytes | ✔ | Table 4.26 "8-byte chain-head prefix". | None. |
| evidence-set hash | 48 | Hash48 | † | SHA-384. How the evidence-set hash is computed (over sorted record identifiers or as a Merkle root) is a separate decision. | None. |
| state hash | 48 | Hash48 | † | SHA-384. | None. |
| policy version | 2 | u16 version number inside the signed policy set | proposal | The RSD names "policy version" (RSD-EVD-01c; Auditability, Section 4.10) separately from "policy hash". | 48 B policy hash: raises the digest to about 394 B. **The policy-set format belongs to provisioning; the version field must be defined there (S4).** |
| nodes heard directly | see options | | proposal | Two options below. | None. |

Options for "nodes heard directly":

| Option | Encoding | Width (N = 20, 19 neighbors) |
|---|---|---|
| A, fleet-list bitmap (recommended) | Roster bitmap over the fleet list of the provisioning package (K10) | 3 B, fixed |
| B, identifier list | u8 count + ascending u16 NodeId list | 1 + 2·n, up to 39 B |

Full digest for N = 20 with 20 origins:

| Variant | Payload | With the 37 B overlay header | Against "about 370 B" (Table 4.29) |
|---|---|---|---|
| Option A | 4 + 3 + 20·12 + 48 + 48 + 2 + 3 = **348 B** | 385 B | payload −22 B; frame +15 B |
| Option B | 4 + 3 + 20·12 + 48 + 48 + 2 + 39 = **384 B** | 421 B | payload +14 B; frame +51 B |

Table 4.29 does not state whether "about 370 B" includes the overlay header.

### 4.4 Overlay frame header (RSD-NET-01, RSD-CRY-01g)

| Field | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| version | 1 | u8 | proposal | Frame format version. | Version and suite packed 4+4 bits in one byte: saves 1 B but needs bit-field parsing and limits each to 16 values. |
| suite | 1 | u8 crypto suite id | proposal | RSD-CRY-01a "suite identifier". CNSA 2.0 suite = 1; P-384 comparison baseline (Table 2.1) = 2. | None. |
| frame type | 1 | u8 enum | proposal | Closed set: HELLO, HS1, HS2, HS3, hop ACK, telemetry, digest, bundle fragment, pull, C1, C2, ... | None. |
| sender | 2 | NodeId | proposal | None. | None. |
| next hop or broadcast | 2 | NodeId; 0xFFFF = broadcast | proposal | Every frame is a MAC broadcast (RSD-NET-01b); addressing is in the overlay. | Separate broadcast flag: 1 B more and inconsistent states with the identifier. |
| counter | 8 | u64 | ✔ | Table 4.26 "8-byte counter"; input to the IV rule (RSD-CRY-01g). | None. |
| message id | 4 | u32, per sender | proposal | Groups fragments into a message; (sender, message id) is unique. | u16: wraps after about 1.8 hours at 10 messages per second; mission length is not bounded. |
| fragment index | 1 | u8 | proposal | A message has at most 255 fragments × 1 400 B = 357 000 B. Largest single messages: G3 proof about 10.5 kB; two-signature command about 9.3 kB. | u16: no RSD message comes near 255 fragments. |
| fragment count | 1 | u8 | proposal | Same. | None. |
| tag | 16 | GcmTag (hop or swarm key) or truncated HMAC (discovery key, HELLO only) | ✔ | Hop or swarm-key tag (RSD-NET-01f). Placed in the header as its last field to keep the order of Table 4.26; the AES-GCM associated data is the header bytes without the tag. | Tag as a trailer after the payload: the usual GCM position, but breaks the order of Table 4.26. |

Header: **37 B** (21 B + 16 B tag).

**Fragment size.** The 1 400 B of RSD-NET-01f is the fragment's message content; the 37 B header is added on top. A full frame is 1 437 B, within the 2 296 B Wi-Fi MTU (1 465 B with UDP and IPv4 headers). This reading matches Table 4.29: "5.5 kB; four fragments" (5 500 / 1 400 = 3.93), and the 3.13 % bundle share is computed over 5 500 / 1 400 fragments.

### 4.5 HELLO (RSD-NET-02d, RSD-CRY-03a)

HELLO is an overlay frame with frame type = HELLO and next hop = broadcast. It is never encrypted or tagged under the swarm key: components in different epochs must find each other without a common swarm key (RSD-CRY-03b, RSD-CRY-03c). The header's 16 B tag field carries the truncated HMAC under the discovery key.

| Field | Width | Where | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| node id | 0 (header) | header sender field | proposal | Node id is the header's sender field and is not repeated in the body. | None. |
| epoch | 6 | body | proposal | EpochId (Section 3); not a full SHA-384 value. | 48 B epoch hash: HELLO would grow to 97 B. |
| sequence number | 4 | body | proposal | Existing `MeshHello::sequence` is u32; no wrap at a 2 s period. | u16: wraps after 36 hours. Carrying the sequence in the header counter field: saves 4 B (see below). |
| timestamp | 8 | body | proposal | SimTime, integer ns. | None. |
| HMAC-SHA-384 (truncated) | 16 | header tag field | ✔ | Table 4.26 "truncated to 16 B"; key: discovery key (RSD-CRY-03a). Input = `"TULPAR/1/hello"` ‖ header without the tag ‖ body; the first 16 bytes of the HMAC output are kept. | None. |

**Size against the 48 B of Table 4.29:** 37 B header + 18 B body = **55 B, 7 B (14.6 %) above the target.** By linear scaling of the table's own figure, the HELLO share rises from 0.32 % to about 0.37 % (calculation). If the HELLO sequence number is carried in the header's 8-byte counter field instead of the body, a HELLO is 51 B (3 B above). 48 B cannot be reached without shortening the common overlay header.

### 4.6 Handshake HS1 / HS2 / HS3 (RSD-NET-02a, RSD-CMC-04a)

Table 4.26 lists the fields of the three messages together; **which field goes into which message is not in the RSD (S6).** Proposal:

| Message | Fields (in order) | Width | Fragments of ≤ 1 400 B |
|---|---|---|---|
| HS1 | suite id (1), nonce (32), ML-KEM encapsulation key (1568 †) | 1 601 B | 2 |
| HS2 | suite id (1), nonce (32), ML-KEM ciphertext (1568 †), transcript signature (4627 †), key confirmation (48), boot quote (4676) | 10 952 B | 8 |
| HS3 | transcript signature (4627 †), key confirmation (48), boot quote (4676) | 9 351 B | 7 |

| Field | Width | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|
| nonce | 32 | proposal | 256 bits, the security level of CNSA 2.0. | 16 B: may suffice for session freshness, but the quote uses the same nonce (RSD-CMC-04a). |
| key confirmation | 48 | proposal | Full HMAC-SHA-384 (RSD-CRY-01d). | 16 B truncated: the handshake share is small, and the full tag is stronger. |
| suite id | 1 | proposal | Same codes as the overlay header's suite. | None. |
| boot quote | 1 + 48 + 4627 = 4676 | proposal, assumption | build id (u8: E, I, R; RSD-CMC-05b), boot measurement (SHA-384, 48 B), quote signature. The session nonce is signed but not sent again (RSD-CMC-04a). **Assumption:** the RSD gives no separate algorithm for the attestation key, so the quote signature is ML-DSA-87 by RSD-CRY-01b ("every other signed record"), 4627 B. | None. |

**Finding: the handshake size in Table 4.29 does not include the boot quotes.** All values below are calculations, not measurements.

| Case | Bytes per handshake | Against Table 4.29 |
|---|---|---|
| Table 4.29 "about 12.4 kB" = 1568 + 1568 + 2 × 4627 | 12 390 B | none |
| This layout without boot quotes | 12 552 B | +162 B (nonces 64, suite ids 2, key confirmations 96) |
| This layout with two boot quotes (ML-DSA-87 assumed) | **21 904 B** | +9 514 B (+77 %) |

By linear scaling of the table's own share, the hop-handshake share rises from 0.58 % to about 1.03 %, and the table total from at most 7.7 % to about 8.2 % with the HELLO change of Section 4.5 included (calculation). The RSD is not changed in this work; the update of Table 4.29 is a separate Jira issue linked to TULPAR-160.

### 4.7 Link-state advertisement (RSD-NET-02)

Common record header + body (I2).

| Field | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| sequence number | 4 | u32 | proposal | Strictly increasing (RSD-NET-02e); separate from the record sequence number because it counts LSAs only. | Using the record sequence number: Table 4.26 lists the field separately, and RSD-NET-02e compares LSAs with each other. |
| quantized position | 12 | Position | proposal | Section 3. | None. |
| position source | 1 | PositionSource | ✔ set | RSD-OBS-03a. | None. |
| error bound | 2 | ErrorBound | proposal | Section 3. | None. |
| neighbor list | 1 + 8·n | u8 count + {neighbor NodeId 2, ETX forward 2, ETX reverse 2, age 2}, ascending NodeId, n ≤ 19 | proposal | At most 19 neighbors in a 20-node fleet. age: u16 milliseconds, saturating at 65 535; the measurement window is 10 s (RSD-NET-02a). | u64 ns for age: 6 B more per neighbor; ms resolution is enough for a 10 s window. |

Carries no backhaul information (RSD-HOT-04b).

### 4.8 Epoch announcement and revocation record (RSD-CRY-03, RSD-ORG-03d)

Common record header + body. A revocation is the same type with the "excluded" fields present.

| Field | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| epoch id | 6 | EpochId | proposal | = (header origin, header record sequence); a mismatch is malformed. | None. |
| parent epochs | 1 + 6·n | u8 count + EpochId list, K6, 1 ≤ n ≤ 2 | proposal | Two parents on a merge (RSD-CRY-03i). | n ≤ 4: a simultaneous merge of three components is not defined in the RSD. **Bound needs approval (S3).** |
| recipient list | ceil(N/8) | roster bitmap | proposal | K10. | None. |
| hash of the new key | 48 | Hash48 | † | SHA-384 (RSD-CRY-03d). | None. |
| producer | 0 | none | proposal | = header origin; not repeated. | Carry it: opens a mismatch case. |
| excluded certificate id | 1 + 0/2 | optional CertIndex | proposal | Present only in a revocation. | None. |
| embedded proof | 0 / 4 + L | optional, u32 length + proof encoding | proposal | **The proof's own format (the four G3 kinds, RSD-SEC-01d) is not in Table 4.26 (S8).** | None. |

### 4.9 Load receipt (RSD-ORG-05)

| Field | Width | Marker | Rationale |
|---|---|---|---|
| vehicle id | 2 | proposal | NodeId. |
| mission manifest hash | 48 | † | SHA-384; the mission id is the manifest hash (RSD-HOT-02d). |
| device envelope hash | 48 | † | SHA-384 (RSD-ORG-05b). |
| boot quote | 1 + 48 + 4627 | proposal, assumption | Same structure as in Section 4.6; before the mission the nonce is the mission manifest hash. |
| signature | 4627 | † | ML-DSA-87 (RSD-CRY-01b). |

### 4.10 Operator command (RSD-HOT-02, RSD-HOT-06)

Signed with operator keys and not part of a vehicle chain, so it carries no common record header. Content hash (RSD-HOT-06g) = SHA-384(`"TULPAR/1/command"` ‖ all fields except the signatures).

| Field | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| mission id | 48 | Hash48 | † | "the mission identifier, which is the mission manifest hash" (RSD-HOT-02d). | None. |
| command id | 6 | CommandId | proposal | Section 3. | None. |
| type | 1 | u8: 1 ABORT, 2 HOLD, 3 RESUME, 4 PRIORITY, 5 SUPPORT | ✔ set / proposal codes | RSD-HOT-02a: exactly five types. ABORT's immediate or recall form is a parameter. | Two type codes for ABORT: violates "exactly five" of RSD-HOT-02a. |
| target | ceil(N/8) | roster bitmap | proposal | RSD-HOT-06e: "target does not include it". | Group code: cannot target a single vehicle. |
| parameters | 2 + L | u16 length + type-specific structure | proposal | ABORT: u8 form (0 immediate, 1 recall). HOLD and RESUME: empty. **PRIORITY and SUPPORT parameters are not in the RSD (S9).** | None. |
| issue window | 4 | Window | proposal | None. | None. |
| duration | 2 | u16 seconds | proposal | HOLD: default 600 s, at most 1800 s (RSD-HOT-02e); 0 and values above 1800 are malformed. The RSD states seconds. | Window count: the RSD defines the value in seconds. |
| validity | 2 | u16 seconds | proposal | RESUME default 120 s (RSD-HOT-02f). | None. |
| precondition | 1 | u8 predicate code | proposal | "closed set of predicates over the evidence" (RSD-HOT-02f). **The set is not defined in the RSD (S9).** | None. |
| basis | 1 + 48·n | u8 count + RecordId list, K6, n ≤ 4 | proposal, assumption | **"basis" is not defined in the RSD;** assumption: the evidence records the operator relied on. | None. |
| signatures | 1 + 4629·n | u8 count (1 or 2) + {CertIndex 2, Signature 4627}, ascending CertIndex, distinct certificates | ✔ / † | RSD-HOT-02b/c, RSD-HOT-06c. | None. |

### 4.11 Command receipt (RSD-HOT-01m)

Common record header + body; carried in the producer's next bundle.

| Field | Width | Marker | Rationale |
|---|---|---|---|
| command id | 6 | proposal | CommandId. |
| vehicle | 0 | proposal | = header origin. |
| result | 1 | ✔ set / proposal codes | u8: 1 APPLIED, 2 VOID, 3 REJECTED. |
| window | 0 | proposal | = header window. |

Proposal: also carry the command's content hash (48 B), because RSD-HOT-06g distinguishes copies by command id and content hash. This would be an added field not in Table 4.26; **decision needed (S10).**

### 4.12 Group Status Summary (RSD-HOT-03)

Common record header + body; carried in the producer's bundle (RSD-HOT-03c). A summary covers only the members of the producer's own group in that component (RSD-HOT-03a). **The meaning and unit of most of its fifteen fields are not defined in the RSD;** the rows below propose widths only. The list bounds for backlog items, contacts and alerts are proposals awaiting Berra's approval.

| Field | Width | Marker | Rationale and assumption |
|---|---|---|---|
| group | 1 | proposal | u8 group index (three groups). |
| producer, window | 0 | proposal | = header origin, header window. |
| members covered with compact positions | 1 + 23·n | proposal | n ≤ group size in the roster; {vehicle 2, position 12 (Position, altitude included), time of fix 8 (SimTime), security state 1} (RSD-HOT-03j). "Vehicles per security state" is carried as a per-member state. |
| energy | n | proposal, assumption | u8 percent per member. **No energy model in the RSD.** |
| channels OK per modality | 3 | proposal | u8 count for EO, IR and radar. |
| coverage | 2 | proposal, assumption | u16 per mille. **Not defined in the RSD.** |
| regime per sector | 1 + s | proposal | u8 count + u8 (1 S1, 2 S2, 3 S3) (RSD-OBS-04a); three sectors in the reference configuration. |
| top backlog items | 1 + 49·k | proposal (Berra) | k ≤ 5; {RecordId 48, status 1}. **The bound is not in the RSD.** |
| contacts | 1 + 29·c | proposal (Berra) | c ≤ 16; {contact id 16, Position 12, status 1}. **Contact identifier is not in the RSD.** |
| alerts | 1 + 3·a | proposal (Berra) | a ≤ 8; {code 1, NodeId 2}. **Alert codes are not in the RSD.** |
| jamming level | 1 | proposal | u8 0–4 = J0–J4 (Table 4.19). |
| commands applied | 1 + 6·m | proposal | m ≤ 8; CommandId list. |

Size at the proposed bounds, for the reference groups A (6 vehicles), B (7) and C (6), with the whole group in one component (calculation):

| Group | Members | Body | Record with the 134 B common header | Against "about 1 kB" (Table 4.29) |
|---|---|---|---|---|
| B | 7 | 1 + (1 + 23·7) + 7 + 3 + 2 + (1 + 3) + (1 + 49·5) + (1 + 29·16) + (1 + 3·8) + 1 + (1 + 6·8) = **965 B** | 1 099 B | body −35 B; record +99 B |
| A, C | 6 | **941 B** | 1 075 B | body −59 B; record +75 B |

Table 4.29 does not state whether "about 1 kB" includes the record header.

### 4.13 GCS service certificate (RSD-HOT-05, RSD-HOT-06d)

| Field | Width | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|
| role | 1 | proposal | u8: 1 vehicle, 2 operator, 3 gcs. | ASCII "gcs": variable length and canonicity problems (K2). |
| mission id | 48 | † | RSD-HOT-02d. | None. |
| ML-KEM-1024 public key | 1568 | † | Table 2.1. | None. |
| ML-DSA-87 public key | 2592 | † | Table 2.1. | None. |
| validity | 8 | proposal | not before and not after, two u32 Window values. | SimTime: window resolution is enough for a certificate. |
| headquarters root signature | 4627 | † | ML-DSA-87. | None. |

### 4.14 Backhaul hello (RSD-HOT-04, RSD-HOT-07)

Unsigned and never forwarded into the mesh.

| Field | Width | Marker | Rationale |
|---|---|---|---|
| sender | 2 | proposal | NodeId; 0xFFFF = headquarters. |
| link id | 1 | proposal | u8; at most a few backhauls per gateway. |
| window | 4 | proposal | Window. |
| sequence | 4 | proposal | u32. |

### 4.15 LSC beacon

Out of scope: it stays in the LSC's own source tree (RSD-SAF-02d, RSD-SAF-02f).

## 5. Formats needed but not in Table 4.26

These formats are not designed here; each should become its own issue.

| Format | Source | Note |
|---|---|---|
| GCS receipt | RSD-HOT-05f, RSD-HOT-05h | "names the mission, the record identifier and the GCS window", signed with the GCS service key. |
| G3 proof encoding | RSD-SEC-01d, RSD-ORG-03d | Four proof kinds; embedded in the revocation record. |
| Hop acknowledgment, telemetry, pull, C1 and C2 payloads | RSD-NET-01d, Table 4.29 | Overlay frame types without body formats. |
| Provisioning package (roster, certificate, policy set, exclusion list) | RSD-ORG-05, RSD-CRY-02b | The policy version field is needed by the digest (S4). |
| Record type bodies (observation, task, bid, lease, contract, finding, etc.) | Sections 4.2, 4.3, 4.6 | Not in Table 4.26. |

## 6. Code approach (after approval)

RSD-CRY-02e: *"The canonical-encoding validator shall be generated from the same specification as the record decoders and be identical in every build."* This asks for encoders and the validator to be generated from one machine-readable specification. Proposal: once approved, the tables of this document become a specification such as `docs/design/encodings.json`, and the codecs under `common/formats/` are generated from it rather than written by hand. **Decision needed (S11).** If they are written by hand, the move to a generator becomes a separate issue.

## 7. Decisions needing approval

| No | Decision | Proposal |
|---|---|---|
| S1 | Is the record identifier carried on the wire? | No; the receiver computes it. |
| S2 | Meaning and width of the uniqueness key | Take the meaning from the architecture document; width 16 B. |
| S3 | List bounds (parents 16, tags 8, parent epochs 2, basis 4; GSS bounds: Berra) | The values in this document; all proposals. |
| S4 | Digest policy version: u16 number or 48 B hash? | u16, defined in the policy-set format. |
| S5 | HELLO sequence number in the body (55 B) or in the header counter (51 B)? | In the body, as Table 4.26 lists it; 55 B. |
| S6 | Distribution of fields over HS1, HS2 and HS3 | The table of Section 4.6. |
| S7 | Boot-quote signature algorithm | ML-DSA-87 assumed (RSD-CRY-01b); Table 4.29 update tracked in a separate issue. |
| S8 | Embedded proof encoding | Separate issue. |
| S9 | PRIORITY and SUPPORT parameters, RESUME precondition set | Separate issue; not defined in the RSD. |
| S10 | Add the command content hash to the command receipt? | Yes (added field). |
| S11 | Codecs written by hand or generated from a specification (RSD-CRY-02e)? | Generated from a specification. |
| S12 | Position: local ENU, i32 cm | Yes. |
| S13 | "Nodes heard directly": fleet-list bitmap or identifier list? | Fleet-list bitmap (option A). |
