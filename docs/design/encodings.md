# TULPAR byte-level encodings: proposal

**Status:** DRAFT, awaiting approval (Özgür Can Güngör, Cemre Karabulut; list bounds of the Group Status Summary: Berra). No code is written against this document before approval.
**Issue:** TULPAR-160, "Freeze byte-level encodings of records, bundles and messages".
**Sources:** RSD v1.1, Table 4.26 (Section 4.9), Table 2.1, Tables 4.29 and 4.30, Sections 4.3, 4.4, 4.5, 4.6, 4.8, 5.1 and 6.5.

The RSD leaves the byte level to Phase 1 on purpose: *"Field sizes are given where the architecture gives them; byte-level encodings are frozen in Phase 1."* (note to Table 4.26). This document is the proposal for that freeze.

**How widths are chosen.** Every width is chosen for correctness, security and simplicity. Tables 4.29 and 4.30 are calculations, not requirements, and no width is chosen to match them. Where a frozen size differs from those tables, the difference and its effect on the channel share are stated with the format and collected in Section 8. All sizes and shares in this document are calculations from the proposed layouts, not measurements.

**The RSD stays binding.** Until a new RSD version changes it, the RSD governs. Where this document finds the RSD wrong, ambiguous or outdated, the finding is listed in Section 8 with a proposed correction, and the code follows the current RSD text. No width proposed here departs from a requirement. Where a requirement is ambiguous, the reading chosen is stated in Section 8.

**No field is repeated.** One piece of information is carried in one field. Where Table 4.26 lists a field that another field of the same frame or record already carries, the field is marked "derived" with width 0 and its source. Section 4.2 explains the one case where a value appears both in a bundle header and in the records it carries.

## 1. Markers

| Marker | Meaning |
|---|---|
| ✔ | Size stated in Table 4.26. |
| † | Size derived from another RSD rule, cited in the row. Needs approval. |
| proposal | The RSD gives no size; this document proposes one. Needs approval. |
| added field | Not in Table 4.26, but another RSD requirement makes it necessary (see Section 8). |
| derived | Listed in Table 4.26, carried by another field; width 0. |
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
| K7 | Variable-length byte strings carry a u16 length prefix; the embedded proof carries a u32 length prefix. | No record exceeds 65 535 bytes. An equivocation proof holds two bundles (about 11 kB; "a G3 proof of about 10.5 kB", Section 4.10). | u32 everywhere: 2 bytes wasted per record for no case that needs it. |
| K8 | Optional field: a 1-byte presence flag (0 or 1), followed by the value only if the flag is 1. Any other flag value is malformed. | One encoding. | "All zeros means absent": can collide with a valid value. |
| K9 | Enumerations are u8; an undefined value is malformed. | The RSD's closed sets (RSD-HOT-02a, RSD-OBS-03a). | u16: the closed sets are far below 256 values. |
| K10 | Roster sets are bitmaps of ceil(N/8) bytes (N: roster size in the provisioning package); bit i = roster index i, most significant bit first in each byte. Unused padding bits must be 0. | Fixed size for a mission, one encoding per set without a sorting rule, and membership is a single bit test. N ≤ 20 (Section 1.4) gives 3 bytes. | List of u16 identifiers: variable length, needs K6 ordering and duplicate checks. |
| K11 | Every encoding is consumed exactly. Trailing bytes or a length mismatch reject the input. | One byte string has one parse (RSD-CMC-02b: every build gives the same verdict). | Ignoring trailing bytes: one record gets several byte strings, which breaks signed-equivocation detection. |
| K12 | Hash and signature inputs are prefixed with an ASCII domain label, e.g. `"TULPAR/1/record"`, `"TULPAR/1/bundle"`, `"TULPAR/1/hello"`. | A record signature cannot be reused as a frame or handshake signature. | No label: cross-protocol confusion is left to the luck of the layouts. |
| K13 | Record identifier = SHA-384(`"TULPAR/1/record"` ‖ canonical encoding of all fields except the identifier). | RSD-EVD-02a: "the SHA-384 hash of the canonical encoding of all its other fields". | None (set by the RSD). |
| K14 | A derived field is not encoded. Where a value appears in two places because each place must stand alone (Section 4.2), the decoder checks equality and rejects a mismatch as malformed. | One piece of information, one field; no mismatch state can exist unchecked. | Carrying both without a check: two sources of truth. |

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
| Window | 4 | u32 window index from mission start | proposal | The window is 5 s (Table 1.7); the mission configuration sets the mission length and the RSD does not bound it. u32 cannot wrap in any mission. | u16: enough for 91 hours, but would need a wrap rule for a length the RSD leaves open. |
| SimTime | 8 | u64 simulation time, integer nanoseconds | proposal | Event-log rule (CLAUDE.md: "integer nanoseconds"); RSD-SIM-01. One time unit in the whole system. | u32 milliseconds: smaller, but introduces a second time unit and a conversion. |
| EpochId | 6 | u16 producer NodeId ‖ u32 record sequence number of that producer's epoch announcement | proposal | See "Epoch identifier width" below. | See below. |
| CommandId | 6 | u16 console id ‖ u32 console sequence | proposal | Table 4.26: "command id (console, console sequence)". | u16 sequence: a wrap would break the deduplication of RSD-HOT-06g. |
| CertIndex | 2 | u16 index into the provisioning package's certificate list | proposal | The provisioning package is fixed and signed for the whole mission (RSD-ORG-05a), so an index is unambiguous. | 48 B certificate hash: also unambiguous, but a second way to name the same certificate. |
| Position | 12 | 3 × i32: x east, y north, z up; local ENU relative to the origin in the mission configuration; centimeters | proposal | The LSC beacon has "position and altitude (12 B)" (RSD-SAF-02d), i.e. 3 × 4 B. ns-3 MobilityModel gives Cartesian meters. i32 cm covers ±21 474 km. The M8 kinematic check can use integers without trigonometry (RSD-EVD-03b). | Latitude/longitude i32 at 1e-7 degree: distances need trigonometry and are exposed to platform-dependent rounding. |
| PositionSource | 1 | u8: 0 GNSS, 1 radar fix, 2 mutual tracking, 3 other | ✔ set / proposal codes | Closed set of RSD-OBS-03a. | None. |
| ErrorBound | 2 | u16 decimeters, saturating at 6553.5 m | proposal | GNSS error is on the order of meters, radar-fix error tens of meters; 0.1 m resolution is enough. | u32 cm: resolution beyond sensor accuracy. |
| Etx | 2 | u16 Q8.8 fixed point (value × 256), saturating at 65535 | proposal | ETX ≥ 1; no floating point (K3). | u8 Q4.4: 1/16 resolution increases ties in route selection. |
| SchemaVersion | 2 | u16 | added field | RSD-CMC-02c: "A record that carries a schema version other than the mission's shall be rejected". | u8: enough today, but an overflow later would break the frozen format. |

**Epoch identifier width.** The epoch appears in every record header and in HELLO. Partitioned components open epochs without knowing of each other (RSD-CRY-03h, RSD-CRY-03i), so the identifier must be unique without coordination, and two different epochs must never compare equal; otherwise RSD-CRY-03h ("When two epoch announcements share a parent epoch...") would match unrelated epochs. Options:

| Option | Width | Uniqueness without coordination | Verdict |
|---|---|---|---|
| Counter (u32) | 4 | No: two components both open "epoch 5". | Rejected. |
| Random value | 8 | Probabilistic; the value is chosen by the producer, so a compromised producer can pick any value, including another component's. | Rejected. |
| Truncated hash of the announcement, 8 B | 8 | Probabilistic; the producer can grind the announcement content to a chosen 64-bit prefix collision in about 2^32 hash operations. | Rejected. |
| Truncated hash, 16 B | 16 | Collision-resistant at 2^64 work. | Rejected: 10 B more in every record header than the structured option, for a weaker (probabilistic) property. |
| Full SHA-384 of the announcement | 48 | Collision-resistant. | Rejected: 42 B more in every record header and in HELLO. |
| **Structured: producer NodeId (2) ‖ record sequence number of the announcement in the producer's chain (4)** | **6** | **By construction.** Each producer opens epochs only in its own chain, and a chain position holds one record. Two different announcements with the same identifier are two records at one chain position, which is an equivocation and a G3 proof against the producer (RSD-SEC-01d). | **Proposed.** |

The initial epoch from the provisioning package is 0xFFFF ‖ 0. The structured identifier needs no hash computation and is verifiable against the announcement's own header (Section 4.8).

## 4. Records and frames

Fields are listed in the order of Table 4.26. Added fields are placed at the start.

### 4.1 Common record header (RSD-EVD-02, RSD-EVD-05, RSD-OBS-03)

| Field | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| schema version | 2 | SchemaVersion | added field | RSD-CMC-02c. | See Section 3. |
| identifier | 0 | RecordId | derived (48 B) | The receiver computes it by K13 from the other fields. Carrying it would repeat information the content already determines. | Carry it: a second source of truth whose mismatch case the RSD does not classify. **Decision needed (S1); see finding F8.** |
| origin | 2 | NodeId | proposal | Section 3. | None. |
| record sequence number | 4 | u32 | proposal | Position in the origin's chain; cannot wrap in any mission. | u16: 65 535 records per origin can be exceeded in long missions. u64: no mission needs it. |
| previous-record hash | 48 | Hash48 | † | SHA-384 (RSD-CRY-01d); chain integrity (RSD-EVD-05a). | Truncated hash: chain integrity is a direct security property. |
| window | 4 | Window | proposal | Section 3. | None. |
| epoch | 6 | EpochId | proposal | Section 3. | None. |
| type | 1 | u8 record type code | proposal | Closed set; adding a type is a schema version change (RSD-CMC-02c). About 30 types today. | u16: nowhere near 256 types. |
| uniqueness key | 16 | bytes: first 16 bytes of SHA-384(type-specific key fields) | proposal, assumption | **The RSD does not define this field's meaning (F10).** Assumption: (origin, type, uniqueness key) names one logical record; two different records with the same triple either form a supersedes relation or an equivocation. It is compared only among one origin's own records, so a collision can only implicate its own producer. | 48 B: no security gain for a comparison within one origin. 8 B: accidental collision among one origin's records becomes plausible at about 2^32 records. **Meaning needs approval (S2).** |
| supersedes | 1 + 0/48 | optional RecordId (K8) | proposal | A corrected bid supersedes the earlier one (Section 4.2, task allocation). | Always 48 B: empty in most records. |
| parents | 1 + 48·n | u8 count + RecordId list, K6, n ≤ 16 | proposal | Dependency edges (Table 1.7, Evidence Graph). | u16 count: more than 255 parents is needed nowhere in the RSD. **The bound 16 is a proposal (S3).** |
| common-cause tags | 1 + 2·n | u8 count + u16 tag codes (index into the mission configuration's tag table), K6, n ≤ 8 | proposal | RSD-OBS-03d: tags come from the mission configuration; the record carries only indices. | Text tags: variable length and canonicity problems. **The bound 8 is a proposal (S3).** |
| policy hash | 48 | Hash48 | † | SHA-384; Table 4.26 "policy hash". | None. |

Fixed part without parents, supersedes or tags: 2+2+4+48+4+6+1+16+1+1+1+48 = **134 B**.

The type-specific body follows the header. A record carries no signature of its own; it is covered by its bundle's signature (RSD-EVD-01a).

### 4.2 Signed bundle (RSD-EVD-01, RSD-CRY-01b)

| Field | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| schema version | 2 | SchemaVersion | added field | RSD-CMC-02c, rejection before signature verification. | Only in records: the bundle-level checks of RSD-EVD-01g would have to parse records first. |
| origin | 2 | NodeId | proposal | Needed before any record is parsed: "A receiver shall check a bundle's size, the roster membership of its origin and the origin's quota before verifying the bundle's signature." (RSD-EVD-01g). | None. |
| window | 4 | Window | proposal | One bundle per origin and window (RSD-EVD-01e), checked before parsing records. | None. |
| first record sequence | 4 | u32 | proposal | Same width as the record sequence number. | None. |
| last record sequence | 4 | u32 | proposal | Record count = last − first + 1; no separate count. Records are consecutive; every sequence number in the range appears exactly once. | Separate count field: repeats information. |
| Merkle root | 48 | Hash48 | † | SHA-384. Tree rule in the style of RFC 9162: leaf = SHA-384(0x00 ‖ record), inner node = SHA-384(0x01 ‖ left ‖ right), a lone node is promoted to the next level. | Flat list hash: pull at fragment granularity (RSD-EVD-01d) could not be verified per record. |
| records | Σ (2 + Lᵢ) | per record a u16 length + canonical record (K7) | proposal | None. | None. |
| signature | 4627 | Signature | ✔ | Signed message = `"TULPAR/1/bundle"` ‖ all fields except the signature. Matches RSD-EVD-01a "the header, the records and their Merkle root under one signature" literally. | Header and root only: cryptographically sufficient, but the RSD-EVD-01a acceptance test says the signature "covers ... the records". |

**Why origin, window and sequence appear in both the bundle header and the records.** A record is a content-addressed unit that travels alone: it is pulled by range (RSD-EVD-01d), embedded in proofs (RSD-ORG-03d) and hashed into its identifier with all its fields (RSD-EVD-02a). Its origin, window and sequence number cannot be left out of the record. The bundle header needs the same values before any record is parsed (RSD-EVD-01g, RSD-EVD-01e). Under K14 the decoder checks that every record's origin equals the bundle origin, every record's window equals the bundle window, and the record sequence numbers run from first to last; any mismatch is malformed.

Fixed part (records excluded): 2+2+4+4+4+48+4627 = **4 691 B**. A 5 500 B bundle is four fragments of at most 1 400 B (Section 4.4).

### 4.3 Digest (RSD-EVD-01c, RSD-EVD-03)

A digest is not a record; it is a frame payload under the swarm key (RSD-CRY-03c). The sender is the overlay header's sender field.

| Field | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| window | 4 | Window | added field | RSD-EVD-03c logs the divergence "with both hashes and the window" (F7). | Deriving the window from reception time: reception time must not enter evaluation (RSD-EVD-03b). |
| origin set | ceil(N/8) | roster bitmap (K10) | proposal | Shows which origins have an entry; entries follow in ascending roster index, so the origin is not repeated in each entry. | u16 origin per entry: repeats position information the order already gives. |
| per origin: highest record sequence | 4 | u32 | proposal | Same as the record sequence number. | None. |
| per origin: chain-head prefix | 8 | bytes | ✔ | Table 4.26 "8-byte chain-head prefix". | None. |
| evidence-set hash | 48 | Hash48 | † | SHA-384. How the evidence-set hash is computed (over sorted record identifiers or as a Merkle root) is a separate decision. | None. |
| state hash | 48 | Hash48 | † | SHA-384. | None. |
| policy version | 48 | Hash48: the policy hash | proposal | The state hash is meaningful only together with the exact policy that produced it (RSD-EVD-03c divergence logging; E12 version-mismatch test). The policy hash identifies that policy without a separate numbering authority, cannot be confused between two policy sets, and is the same value every record header already carries. | u16 version number: needs a numbering field in the provisioning format that does not exist yet, and two policy sets with the same number would compare equal. See F9. |
| nodes heard directly | ceil(N/8) | roster bitmap over the fleet list (K10) | proposal | Fixed size, one encoding, no sorting rule. | u8 count + ascending u16 NodeId list: variable length, needs K6 checks; 1 + 2·n bytes (up to 39 B). |

Full digest for N = 20 with 20 origins:

| Part | Bytes |
|---|---|
| Payload: 4 + 3 + 20·12 + 48 + 48 + 48 + 3 | **394 B** |
| Frame with the 37 B overlay header | **431 B** |
| Table 4.29 "about 370 B" | payload +24 B; frame +61 B |
| Digest share (Table 4.29: 0.26 %), linear scaling | 0.28 % if 370 B was the payload; 0.30 % if it was the whole frame |

### 4.4 Overlay frame header (RSD-NET-01, RSD-CRY-01g)

| Field | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| version | 1 | u8 | proposal | Frame format version. | Version and suite packed 4+4 bits in one byte: needs bit-field parsing and limits each to 16 values. |
| suite | 1 | u8 crypto suite id | proposal | RSD-CRY-01a "suite identifier". CNSA 2.0 suite = 1; P-384 comparison baseline (Table 2.1) = 2. | None. |
| frame type | 1 | u8 enum | proposal | Closed set: HELLO, HS1, HS2, HS3, hop ACK, telemetry, digest, bundle fragment, pull, C1, C2, ... | None. |
| sender | 2 | NodeId | proposal | None. | None. |
| next hop or broadcast | 2 | NodeId; 0xFFFF = broadcast | proposal | Every frame is a MAC broadcast (RSD-NET-01b); addressing is in the overlay. | Separate broadcast flag: a second field for the same information. |
| counter | 8 | u64 | ✔ | Table 4.26 "8-byte counter"; input to the IV rule (RSD-CRY-01g) and replay protection. Strictly increasing per sender and key. | None. |
| message id | 4 | u32, per sender | proposal | Groups fragments into a message; (sender, message id) is unique and cannot wrap in any mission. | u16: wraps after about 1.8 hours at 10 messages per second. |
| fragment index | 1 | u8 | proposal | A message has at most 255 fragments × 1 400 B = 357 000 B. Largest single messages: G3 proof about 10.5 kB (8 fragments), two-signature command about 9.3 kB (7 fragments). | u16: no RSD message comes near 255 fragments. |
| fragment count | 1 | u8 | proposal | Same. | None. |
| tag | 16 | GcmTag (hop or swarm key), or truncated HMAC under the discovery key for HELLO | ✔ | RSD-NET-01f "each carrying its own hop or swarm-key tag". Placed as the header's last field to keep the order of Table 4.26; the authenticated header bytes exclude the tag. | Tag as a trailer after the payload: the usual GCM position, but breaks the order of Table 4.26. |

Header: **37 B** (21 B + 16 B tag).

**Fragment size.** The 1 400 B of RSD-NET-01f is read as the fragment's message content; the 37 B header is added on top. A full frame is 1 437 B, within the 2 296 B Wi-Fi MTU (1 465 B with UDP and IPv4 headers). This reading matches Table 4.29: "5.5 kB; four fragments" (5 500 / 1 400 = 3.93). The RSD text allows both readings (F3).

### 4.5 HELLO (RSD-NET-02d, RSD-CRY-03a)

HELLO is an overlay frame with frame type = HELLO and next hop = broadcast. It is never encrypted or tagged under the swarm key: components in different epochs must find each other without a common swarm key (RSD-CRY-03b, RSD-CRY-03c). The header's 16 B tag field carries the truncated HMAC under the discovery key. No field is repeated between header and body.

| Field (Table 4.26) | Width | Where | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| node id | 0 | header sender | derived | The header's sender field is the node id. | Repeat in body: two fields for one value. |
| epoch | 6 | body | proposal | EpochId (Section 3). | 48 B epoch hash: see "Epoch identifier width". |
| sequence number | 0 | header counter | derived | The header's 8-byte counter is strictly increasing per sender and is already the frame's sequence; a separate HELLO sequence would repeat it. HMAC replay protection uses it. | u32 in the body: a second sequence for the same frame. |
| timestamp | 8 | body | proposal | SimTime, integer ns. Distinct from the counter: it states send time, the counter states order. | u32 ms: second time unit. |
| HMAC-SHA-384 (truncated) | 16 | header tag | ✔ | Table 4.26 "truncated to 16 B"; key: discovery key (RSD-CRY-03a). Input = `"TULPAR/1/hello"` ‖ header without the tag ‖ body; the first 16 bytes of the HMAC output are kept. | None. |

Size: 37 B header + 14 B body = **51 B**. Table 4.29 assumes 48 B; the HELLO share rises from 0.32 % to about 0.34 % by linear scaling (calculation; F4).

### 4.6 Handshake HS1 / HS2 / HS3 (RSD-NET-02a, RSD-CMC-04a)

Table 4.26 lists the fields of the three messages together; **which field goes into which message is not in the RSD (S6, F17).** Proposal:

| Message | Fields (in order) | Width | Fragments of ≤ 1 400 B |
|---|---|---|---|
| HS1 | suite id (1), nonce (32), ML-KEM encapsulation key (1568 †) | 1 601 B | 2 |
| HS2 | suite id (1), nonce (32), ML-KEM ciphertext (1568 †), transcript signature (4627 †), key confirmation (48), boot quote (4676) | 10 952 B | 8 |
| HS3 | transcript signature (4627 †), key confirmation (48), boot quote (4676) | 9 351 B | 7 |

| Field | Width | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|
| nonce | 32 | proposal | 256 bits, the security level of CNSA 2.0; the quote also binds it (RSD-CMC-04a). | 16 B: 128 bits, below the suite's security level for a value an attacker may try to repeat. |
| key confirmation | 48 | proposal | Full HMAC-SHA-384 (RSD-CRY-01d). | 16 B truncated: weaker for the one value that proves both ends hold the session key. |
| suite id | 1 | proposal | Same codes as the overlay header's suite. Sent in HS2 as well, so the responder commits to the suite it accepted. | None. |
| boot quote | 1 + 48 + 4627 = 4676 | proposal, assumption | build id (u8: E, I, R; RSD-CMC-05b), boot measurement (SHA-384, 48 B), quote signature. The session nonce is signed but not sent again (no repetition). **Assumption:** the RSD gives no separate algorithm for the attestation key, so the quote signature is ML-DSA-87 by RSD-CRY-01b ("every other signed record"), 4627 B (F2). | None. |

Total: 21 904 B per handshake (with quotes, ML-DSA-87 assumed). Table 4.29 assumes about 12.4 kB = 1568 + 1568 + 2 × 4627 = 12 390 B, without the quotes. Effect on the handshake share: 0.58 % → about 1.03 % (calculation; F1).

### 4.7 Link-state advertisement (RSD-NET-02)

Common record header + body (I2).

| Field | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| sequence number | 4 | u32 | proposal | Strictly increasing over LSAs (RSD-NET-02e); not the record sequence number, which counts all records of the origin. | Using the record sequence number: Table 4.26 lists the field separately, and RSD-NET-02e compares LSAs with each other. |
| quantized position | 12 | Position | proposal | Section 3. | None. |
| position source | 1 | PositionSource | ✔ set | RSD-OBS-03a. | None. |
| error bound | 2 | ErrorBound | proposal | Section 3. | None. |
| neighbor list | 1 + 8·n | u8 count + {neighbor NodeId 2, ETX forward 2, ETX reverse 2, age 2}, ascending NodeId, n ≤ 19 | proposal | At most 19 neighbors in a 20-node fleet. age: u16 milliseconds, saturating at 65 535; the measurement window is 10 s (RSD-NET-02a). | u64 ns for age: resolution and range far beyond a 10 s window. |

Carries no backhaul information (RSD-HOT-04b).

### 4.8 Epoch announcement and revocation record (RSD-CRY-03, RSD-ORG-03d)

Common record header + body. A revocation is the same type with the "excluded" fields present.

| Field | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| epoch id | 0 | EpochId | derived | = (header origin, header record sequence number) by the definition of Section 3. | Carry it: repeats the header. |
| parent epochs | 1 + 6·n | u8 count + EpochId list, K6, 1 ≤ n ≤ 2 | proposal | Two parents on a merge (RSD-CRY-03i). | n ≤ 4: a simultaneous merge of three components is not defined in the RSD. **Bound needs approval (S3).** |
| recipient list | ceil(N/8) | roster bitmap | proposal | K10. | None. |
| hash of the new key | 48 | Hash48 | † | SHA-384 (RSD-CRY-03d). | None. |
| producer | 0 | NodeId | derived | = header origin. | Carry it: repeats the header. |
| excluded certificate id | 1 + 0/2 | optional CertIndex | proposal | Present only in a revocation. | None. |
| embedded proof | 0 / 4 + L | optional, u32 length + proof encoding | proposal | **The proof's own format (the four G3 kinds, RSD-SEC-01d) is not in Table 4.26 (S8, F13).** | None. |

### 4.9 Load receipt (RSD-ORG-05)

| Field | Width | Marker | Rationale |
|---|---|---|---|
| vehicle id | 2 | proposal | NodeId. |
| mission manifest hash | 48 | † | SHA-384; the mission id is the manifest hash (RSD-HOT-02d). |
| device envelope hash | 48 | † | SHA-384 (RSD-ORG-05b). |
| boot quote | 1 + 48 + 4627 | proposal, assumption | Same structure as in Section 4.6; before the mission the nonce is the mission manifest hash, which is already in the receipt and is not repeated. |
| signature | 4627 | † | ML-DSA-87 (RSD-CRY-01b). |

### 4.10 Operator command (RSD-HOT-02, RSD-HOT-06)

Signed with operator keys and not part of a vehicle chain, so it carries no common record header. Content hash (RSD-HOT-06g) = SHA-384(`"TULPAR/1/command"` ‖ all fields except the signatures).

| Field | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| mission id | 48 | Hash48 | † | "the mission identifier, which is the mission manifest hash" (RSD-HOT-02d). | None. |
| command id | 6 | CommandId | proposal | Section 3. | None. |
| type | 1 | u8: 1 ABORT, 2 HOLD, 3 RESUME, 4 PRIORITY, 5 SUPPORT | ✔ set / proposal codes | RSD-HOT-02a: exactly five types. ABORT's immediate or recall form is a parameter. | Two type codes for ABORT: violates "exactly five" of RSD-HOT-02a. |
| target | ceil(N/8) | roster bitmap | proposal | RSD-HOT-06e: "target does not include it". | Group code: cannot target a single vehicle. |
| parameters | 2 + L | u16 length + type-specific structure | proposal | ABORT: u8 form (0 immediate, 1 recall). HOLD and RESUME: empty. **PRIORITY and SUPPORT parameters are not in the RSD (S9, F11).** | None. |
| issue window | 4 | Window | proposal | None. | None. |
| duration | 2 | u16 seconds | proposal | HOLD: default 600 s, at most 1800 s (RSD-HOT-02e); 0 and values above 1800 are malformed. The RSD states seconds. | Window count: the RSD defines the value in seconds. |
| validity | 2 | u16 seconds | proposal | RESUME default 120 s (RSD-HOT-02f). | None. |
| precondition | 1 | u8 predicate code | proposal | "closed set of predicates over the evidence" (RSD-HOT-02f). **The set is not defined in the RSD (S9, F11).** | None. |
| basis | 1 + 48·n | u8 count + RecordId list, K6, n ≤ 4 | proposal, assumption | **"basis" is not defined in the RSD (F11);** assumption: the evidence records the operator relied on. | None. |
| signatures | 1 + 4629·n | u8 count (1 or 2) + {CertIndex 2, Signature 4627}, ascending CertIndex, distinct certificates | ✔ / † | RSD-HOT-02b/c, RSD-HOT-06c. | None. |

### 4.11 Command receipt (RSD-HOT-01m)

Common record header + body; carried in the producer's next bundle.

| Field | Width | Marker | Rationale |
|---|---|---|---|
| command id | 6 | proposal | CommandId. |
| vehicle | 0 | derived | = header origin. |
| result | 1 | ✔ set / proposal codes | u8: 1 APPLIED, 2 VOID, 3 REJECTED. |
| window | 0 | derived | = header window. |

Proposal: also carry the command's content hash (48 B), because RSD-HOT-06g distinguishes copies by command id and content hash. This would be an added field not in Table 4.26; **decision needed (S10).**

### 4.12 Group Status Summary (RSD-HOT-03)

Common record header + body; carried in the producer's bundle (RSD-HOT-03c). A summary covers only the members of the producer's own group in that component (RSD-HOT-03a). **The meaning and unit of most of its fifteen fields are not defined in the RSD (F12);** the rows below propose widths only. The list bounds for backlog items, contacts and alerts are proposals awaiting Berra's approval.

| Field | Width | Marker | Rationale and assumption |
|---|---|---|---|
| group | 0 | derived | = the roster group of the producer (header origin); a summary covers the producer's own group (RSD-HOT-03a). |
| producer, window | 0 | derived | = header origin, header window. |
| members covered with compact positions | 1 + 23·n | proposal | n ≤ group size in the roster; {vehicle 2, position 12 (Position, altitude included), time of fix 8 (SimTime), security state 1} (RSD-HOT-03j). "Vehicles per security state" is carried as a per-member state, not as a second list. |
| energy | n | proposal, assumption | u8 percent per member, in member order. **No energy model in the RSD.** |
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
| B | 7 | (1 + 23·7) + 7 + 3 + 2 + (1 + 3) + (1 + 49·5) + (1 + 29·16) + (1 + 3·8) + 1 + (1 + 6·8) = **964 B** | 1 098 B | body −36 B; record +98 B |
| A, C | 6 | **940 B** | 1 074 B | body −60 B; record +74 B |

Table 4.29 does not state whether "about 1 kB" includes the record header; the share stays within its "≤ 0.4 % (worst-case estimate)" by linear scaling (about 0.44 % if the 1 kB was the full record and 1 098 B is used; F15).

### 4.13 GCS service certificate (RSD-HOT-05, RSD-HOT-06d)

| Field | Width | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|
| role | 1 | proposal | u8: 1 vehicle, 2 operator, 3 gcs. | ASCII "gcs": variable length and canonicity problems (K2). |
| mission id | 48 | † | RSD-HOT-02d. | None. |
| ML-KEM-1024 public key | 1568 | † | Table 2.1. | None. |
| ML-DSA-87 public key | 2592 | † | Table 2.1. | None. |
| validity | 8 | proposal | not before and not after, two u32 Window values. | SimTime: window resolution is enough for a certificate and matches the window checks of RSD-HOT-05. |
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

These formats are not designed here; each should become its own issue (F13).

| Format | Source | Note |
|---|---|---|
| GCS receipt | RSD-HOT-05f, RSD-HOT-05h | "names the mission, the record identifier and the GCS window", signed with the GCS service key. |
| G3 proof encoding | RSD-SEC-01d, RSD-ORG-03d | Four proof kinds; embedded in the revocation record. |
| Hop acknowledgment, telemetry, pull, C1 and C2 payloads | RSD-NET-01d, Table 4.29 | Overlay frame types without body formats. |
| Provisioning package (roster, certificate, policy set, exclusion list) | RSD-ORG-05, RSD-CRY-02b | None. |
| Record type bodies (observation, task, bid, lease, contract, finding, etc.) | Sections 4.2, 4.3, 4.6 | Not in Table 4.26. |

## 6. Code approach (after approval)

RSD-CRY-02e: *"The canonical-encoding validator shall be generated from the same specification as the record decoders and be identical in every build."* This asks for decoders and the validator to be generated from one machine-readable specification. Proposal: once approved, the tables of this document become a specification such as `docs/design/encodings.json`, and the codecs under `common/formats/` are generated from it rather than written by hand. **Decision needed (S11).** If they are written by hand, the move to a generator becomes a separate issue.

## 7. Decisions needing approval

| No | Decision | Proposal |
|---|---|---|
| S1 | Is the record identifier carried on the wire? | No; derived by K13 (see F8). |
| S2 | Meaning and width of the uniqueness key | Take the meaning from the architecture document; width 16 B. |
| S3 | List bounds (parents 16, tags 8, parent epochs 2, basis 4; GSS bounds: Berra) | The values in this document; all proposals. |
| S4 | Digest "policy version" | The 48 B policy hash (see F9). |
| S5 | HELLO sequence number | Carried by the header counter; not repeated in the body (51 B). |
| S6 | Distribution of fields over HS1, HS2 and HS3 | The table of Section 4.6. |
| S7 | Boot-quote signature algorithm | ML-DSA-87 assumed (RSD-CRY-01b); see F2. |
| S8 | Embedded proof encoding | Separate issue. |
| S9 | PRIORITY and SUPPORT parameters, RESUME precondition set | Separate issue; not defined in the RSD. |
| S10 | Add the command content hash to the command receipt? | Yes (added field). |
| S11 | Codecs written by hand or generated from a specification (RSD-CRY-02e)? | Generated from a specification. |
| S12 | Position: local ENU, i32 cm | Yes. |
| S13 | "Nodes heard directly": fleet-list bitmap or identifier list? | Fleet-list bitmap. |

## 8. RSD findings

Places in RSD v1.1 found wrong, ambiguous or outdated while freezing the encodings. Each finding quotes the RSD verbatim, gives the reason and proposes a correction. The RSD stays binding until a new version changes it; the corrections are tracked in the RSD correction issue linked to TULPAR-160. All numbers are calculations, not measurements; channel shares are scaled linearly from the table's own figures, with per-frame overhead not modelled separately.

### Summary of the effect on Table 4.29

| Row | RSD v1.1 | With the frozen encodings (calculation) |
|---|---|---|
| HELLO | 48 B, 0.32 % | 51 B, about 0.34 % |
| Hop handshakes | about 12.4 kB, 0.58 % | 21 904 B, about 1.03 % (ML-DSA-87 quote assumed) |
| Digests | about 370 B, 0.26 % | 394 B payload / 431 B frame, about 0.28–0.30 % |
| Control plane (HELLO, handshakes) | 0.90 % | about 1.37 % (hypothesis threshold 5 % unaffected) |
| Evidence plane (bundles, pull, digests) | 4.10 % | about 4.12–4.14 % (cap 10 % unaffected) |
| Total | ≤ 7.7 % | about 8.2 % |

### Findings

| No | Kind | RSD text (verbatim) | Reason | Proposed correction |
|---|---|---|---|---|
| F1 | Outdated | Table 4.29: "Hop handshakes \| 190 pairs, about 12.4 kB per handshake, every 600 s \| 0.58 %". RSD-CMC-04a: "In handshake messages HS2 and HS3, each side shall include a quote, signed by its key provider's attestation key, over the boot measurement of the loaded agent image, the build identifier and the session nonce." | 12.4 kB = 1568 + 1568 + 2 × 4627 = 12 390 B: one encapsulation key, one ciphertext and two transcript signatures. The two quotes of RSD-CMC-04a are not counted. With the frozen layout and ML-DSA-87 quotes (F2), a handshake is 21 904 B. | Table 4.29: "190 pairs, about 21.9 kB per handshake (two boot quotes included), every 600 s \| about 1.03 %"; update the control-plane line and the total (summary above), marked as calculation. |
| F2 | Ambiguous | RSD-CMC-04a: "a quote, signed by its key provider's attestation key". Table 2.1 lists no attestation algorithm. RSD-CRY-01b: "Bundles, handshake transcripts, link-state advertisements and every other signed record shall be signed with ML-DSA-87." | The quote's signature algorithm, and so the quote size, is not stated. RSD-CRY-01b suggests ML-DSA-87 but does not name quotes. | State in RSD-CMC-04a or Table 2.1: "The attestation key is an ML-DSA-87 key; a quote signature is 4627 B." |
| F3 | Ambiguous | RSD-NET-01f: "The overlay shall split every message into fragments of at most 1400 bytes, each carrying its own hop or swarm-key tag." Table 4.29: "A 1400-byte fragment takes 1.99 ms." and "5.5 kB; four fragments". | It is not stated whether 1400 bytes includes the overlay frame header and tag. With the header included, a 5 500 B bundle is five fragments, which contradicts "four fragments"; the frozen reading is 1400 bytes of message content plus a 37 B header. It is also not stated whether the 1.99 ms airtime includes the overlay header; if not, the bundle and pull shares rise by about 1437/1400 (3.13 % → about 3.21 %, 0.63 % → about 0.65 %). | RSD-NET-01f: "...into fragments of at most 1400 bytes of message content, each sent in one overlay frame that adds the frame header and its own hop or swarm-key tag." Table 4.29: state which headers the 1.99 ms includes. |
| F4 | Outdated | Table 4.29: "HELLO \| Every 2 s per node; 48 B \| 0.32 %". | The frozen HELLO is 51 B: 37 B overlay header (including the 16 B HMAC in the tag field) + 6 B epoch + 8 B timestamp. | Table 4.29: "Every 2 s per node; 51 B \| about 0.34 %". |
| F5 | Outdated | Table 4.29: "Digests \| Every node every window; about 370 B \| 0.26 %". | The frozen digest for N = 20 is 394 B payload, 431 B with the overlay header. The table does not say which it counts. | Table 4.29: "about 431 B including the overlay header \| about 0.30 %". |
| F6 | Wrong (omission) | Table 4.26, Common record header: "Identifier (SHA-384 of the canonical encoding), origin, record sequence number, previous-record hash, window, epoch, type, uniqueness key, supersedes, parents, common-cause tags, policy hash". RSD-CMC-02c: "A record that carries a schema version other than the mission's shall be rejected without forming a proof." | RSD-CMC-02c requires every record to carry a schema version; Table 4.26 does not list it. | Table 4.26: add "schema version" to the common record header (and to the signed bundle header). |
| F7 | Wrong (omission) | Table 4.26, Digest: "Per origin: highest record sequence and 8-byte chain-head prefix; evidence-set hash; state hash with policy version; nodes heard directly". RSD-EVD-03c: "An agent that observes equal evidence hashes with different state hashes shall log the divergence." with the acceptance "the divergence is logged with both hashes and the window". | The window the digest describes is not listed, but the divergence log needs it, and reception time must not stand in for it (RSD-EVD-03b). | Table 4.26, Digest: add "window". |
| F8 | Ambiguous | Table 4.26: "Identifier (SHA-384 of the canonical encoding)" listed as a key field. RSD-EVD-02a: "The identifier of a record shall be the SHA-384 hash of the canonical encoding of all its other fields." | Listing the identifier as a field suggests it is transmitted; but it is fully determined by the other fields, and the RSD does not say how a transmitted identifier that does not match the content is classified (malformed, G3 or neither). | Table 4.26: "Identifier (SHA-384 of the canonical encoding of all other fields; derived by the receiver, not transmitted)". |
| F9 | Ambiguous | RSD-EVD-01c: "a hash of its derived states with the policy version". Table 4.26, Common record header: "policy hash". Section 4.10: "Every decision record carries its evidence identifiers, policy version and open hypotheses". | "Policy version" and "policy hash" are used for what appears to be one identity of the policy set; no version numbering is defined anywhere. | Use one term: "policy hash (SHA-384 of the canonical policy set)" in RSD-EVD-01c, Table 4.26 and Section 4.10. |
| F10 | Ambiguous | Table 4.26, Common record header: "uniqueness key", "supersedes". | Neither the meaning of the uniqueness key nor the rule tying it to supersedes and to equivocation is defined. | Define the uniqueness key: which fields of each record type it covers, and that two records of one origin with equal type and uniqueness key either supersede or equivocate. |
| F11 | Ambiguous | Table 4.26, Operator command: "type, target, parameters, issue window, duration, validity, precondition, basis". RSD-HOT-02f: "a precondition from the closed set of predicates over the evidence". | The predicate set, the meaning of "basis" and the parameters of PRIORITY and SUPPORT are not defined, although RSD-HOT-02a requires a closed command schema. | Add the predicate set, the definition of basis and the PRIORITY and SUPPORT parameters to Section 4.8 or Table 4.26. |
| F12 | Ambiguous | Table 4.26, Group Status Summary: "...vehicles per security state, energy, channels OK per modality, coverage, regime per sector, top backlog items, contacts, alerts, jamming level, commands applied". RSD-HOT-03b: "with a fixed set of fields". | Units and definitions of energy and coverage, the number of backlog items, the contact identifier and the alert codes are not given; a fixed set of fields needs fixed counts. | Define units, list bounds and code sets for these fields. |
| F13 | Wrong (omission) | Table 4.26 title: "Records and formats relied on by the requirements." | Formats relied on by requirements are missing: the GCS receipt (RSD-HOT-05f, RSD-HOT-05h), the G3 proof encoding (RSD-SEC-01d, RSD-ORG-03d), the hop acknowledgment (RSD-NET-01d). | Add these rows to Table 4.26. |
| F14 | Outdated | Table 4.30: "Handshakes \| 19 per 600 s \| Hop-session design". "Signature verification \| 4 bundles/s × 0.51 ms ≈ 2 ms per second". | With two quotes, each handshake adds one quote signature and one quote verification per side. The effect is small: 19 × (0.96 + 0.51) ms per 600 s ≈ 0.05 ms per second, but the table does not mention it. | Table 4.30, Handshakes: add "each with one transcript signature, one quote signature and two verifications per side". |
| F15 | Ambiguous | Table 4.29: "Group Status Summary \| Three components; about 1 kB every 30 s; ... \| ≤ 0.4 % (worst-case estimate)". | With the frozen layout and the proposed list bounds, a summary record of group B is 1 098 B (964 B body + 134 B common header). The table does not say whether 1 kB includes the record header; if it does, the worst case rises to about 0.44 %. | State the summary size as "about 1.1 kB including the record header" and recompute the worst case once the list bounds (F12) are fixed. |
| F16 | Ambiguous | Table 4.26, HELLO: "Node id, epoch, sequence number, timestamp, HMAC-SHA-384 under the discovery key (truncated to 16 B)". Table 4.26, Overlay frame header: "...sender, ..., 8-byte counter, ..., 16-byte tag". | HELLO is an overlay frame, so node id, sequence number and HMAC can be carried by the header's sender, counter and tag fields. The table lists them as if they were separate. | Table 4.26, HELLO: "overlay frame header (sender = node id, counter = sequence number, tag = HMAC-SHA-384 under the discovery key, truncated to 16 B), epoch, timestamp". |
| F17 | Ambiguous | Table 4.26: "Handshake HS1 / HS2 / HS3 \| Nonce, ML-KEM encapsulation key or ciphertext, transcript signature, key confirmation, suite id; HS2 and HS3 add the boot quote". | Which fields belong to which message, and which messages carry a transcript signature, is not stated; the size and share of F1 depend on it. | Table 4.26: one row per message, as in Section 4.6 of this document once approved. |
