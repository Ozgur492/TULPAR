# TULPAR byte-level encodings: proposal

**Status:** DRAFT, awaiting approval (Özgür Can Güngör, Cemre Karabulut; list bounds of the Group Status Summary: Berra). No code is written against this document before approval.
**Issue:** TULPAR-160, "Freeze byte-level encodings of records, bundles and messages". RSD corrections: TULPAR-479.

## 0. Sources and how they are used

| Priority | Source | Use |
|---|---|---|
| 1 | RSD (`docs/rsd/TULPAR_RSD.md`) | Governs in every case. |
| 2 | HOTL & National SATCOM Final Integration Package, Redundant Gateway Edition | Replaces V13.3 and V13.2 on HOTL and SATCOM topics. Used: HL-11, HL-12, HL-15, HL-38, HL-41, HL-44. |
| 3 | V13.3 Change Set (frozen) | Changes V13.2. Used: C07 (quotes in HS2 and HS3), C08 (malformed record as M4), C10 (Section 11.10), C19 (Tables C.4 and C.5). |
| 4 | V13.2 System Architecture | Used: Section 7.2, 7.3, 7.5, 7.7, 8.2; Table 5.2; Tables 7.4, 8.1, 8.2; Appendix C (Tables C.1 to C.5). |
| — | Common-Mode Compromise Rev 4.1 | A proposal report; only the parts the RSD adopted apply. No field in this document rests on it alone. |

The architecture is used where the RSD leaves the detail: *"Field sizes are given where the architecture gives them; byte-level encodings are frozen in Phase 1."* (note to Table 4.26). Where the architecture conflicts with the RSD, or has no counterpart in it, it is not used; each such case is in Section 9. Known RSD-over-architecture differences that apply here: three agent builds E, I, R (RSD-CMC-05), not one per group (V13.3 C07); the command ship is not a mandatory fourth gateway (CMC-R-14 not adopted); a Group Status Summary goes as one copy to each reachable designated gateway (RSD-HOT-05a), not to the command ship (V13.3 C10).

The source texts are PDF and DOCX extractions. Every table used below was read in full; no value was taken from a table that could not be read reliably. Where a source was unreadable, the row says "not readable in source".

**How widths are chosen.** By the architecture where it gives a size; otherwise for correctness, security and simplicity. Tables 4.29 and 4.30 are calculations, not requirements, and no width is chosen to match them. Where a frozen size differs from those tables, the difference and its effect on the channel share are stated with the format and collected in Section 8. All sizes and shares here are calculations from the proposed layouts, not measurements.

**The RSD stays binding.** Until a new RSD version changes it, the RSD governs; findings are listed in Section 8 and the code follows the current RSD text.

**No field is repeated.** One piece of information is carried in one field. A Table 4.26 field that another field of the same frame or record already carries is marked "derived" with width 0. Section 4.2 explains the one case where a value appears in a bundle header and in its records.

## 1. Markers

| Marker | Meaning |
|---|---|
| ✔ | Size stated in RSD Table 4.26. |
| A | Size stated in the architecture; the source is cited. |
| † | Size derived from another RSD rule, cited in the row. |
| proposal | Neither the RSD nor the architecture gives a size; this document proposes one. |
| added field | Not in Table 4.26, but another RSD requirement makes it necessary (see Section 8). |
| derived | Listed in Table 4.26, carried by another field; width 0. |
| assumption | A meaning or algorithm no source states; this document assumes it. |

Every proposal row names the alternative and why it was not chosen. Every row marked †, proposal or assumption needs approval.

## 2. Canonical encoding rules

These rules apply to every record, bundle and frame. A signed record that violates the canonical encoding is a G3 proof (RSD-CMC-02a), so each rule maps to an exact check whose failure yields "malformed".

| No | Rule (proposal) | Rationale | Alternative and why not |
|---|---|---|---|
| K1 | All integers are big-endian. | ns-3 `Buffer::Iterator::WriteHtonU*` convention; the existing `HelloHeader` uses it. | Little-endian: departs from the ns-3 header convention. |
| K2 | Only fixed-width integers (u8, u16, u32, u64; i32 for positions). No variable-length integers. | One value has one encoding (RSD-CMC-02a). | LEB128 varint: needs a "shortest form" check on every field and complicates fixed size limits (RSD-CMC-01c). |
| K3 | No floating point. Continuous quantities are quantized to integers with a stated scale. The producer quantizes once; the receiver never re-quantizes. | RSD-EVD-03b forbids "platform-dependent rounding". | IEEE-754: NaN, -0 and multiple bit patterns break canonicity. |
| K4 | Quantization is `q = llround(x * scale)`, then saturated to the field's range. | `std::llround` gives the same result on every platform for IEEE-754 doubles. | `floor`: bias on negative coordinates. |
| K5 | Lists are a count (u8, or u16 where needed) followed by the items. Every list type has a fixed maximum count; exceeding it is malformed. | RSD-CMC-01c: "Every ingress parser shall enforce fixed size limits on its input." | Sentinel terminator: the limit can only be checked at the end. |
| K6 | Lists with set semantics (parents, parent epochs) are encoded in strictly ascending order with no duplicates. | The same set encodes to one byte string, so the identifier follows the content (RSD-EVD-02a). | Producer order: the same content would yield two identifiers. |
| K7 | Variable-length byte strings carry a u16 length prefix; an embedded proof carries a u32 length prefix. | No record exceeds 65 535 bytes; an equivocation proof is about 10 kB (Section 4.9). | u32 everywhere: no case needs it. |
| K8 | Optional field: a 1-byte presence flag (0 or 1), followed by the value only if the flag is 1. Other flag values are malformed. Where the type determines presence (Section 4.11), no flag is sent. | One encoding. | "All zeros means absent": can collide with a valid value. |
| K9 | Enumerations are u8; an undefined value is malformed. | Closed sets of the RSD and the architecture. | u16: no set comes near 256 values. |
| K10 | Roster sets are bitmaps of ceil(N/8) bytes (N: roster size), bit i = roster position i, most significant bit first. Unused padding bits must be 0. | Fixed size for a mission, one encoding per set, no sorting rule. N ≤ 20 gives 3 bytes. Roster positions are fixed by the signed roster (AS3). | List of 8-byte certificate identifiers: up to 160 bytes and needs K6 checks. See D8 in Section 9. |
| K11 | Every encoding is consumed exactly; trailing bytes or a length mismatch reject the input. | One byte string has one parse (RSD-CMC-02b). | Ignoring trailing bytes: breaks signed-equivocation detection. |
| K12 | Hash, MAC and signature inputs are prefixed with an ASCII domain label, e.g. `"TULPAR/1/record"`, `"TULPAR/1/bundle"`, `"TULPAR/1/hello"`. | A signature or MAC of one kind cannot be reused as another kind. | No label: cross-protocol confusion is left to the layouts. |
| K13 | Record identifier = SHA-384(`"TULPAR/1/record"` ‖ canonical encoding of all fields except the identifier). | RSD-EVD-02a. V13.2 Table 8.1: "rid — SHA-384 of the canonical encoding of all other fields". | None. |
| K14 | A derived field is not encoded. Where a value appears in two places because each must stand alone (Section 4.2), the decoder checks equality; a mismatch is malformed. | One field per piece of information. | Carrying both unchecked: two sources of truth. |

## 3. Common types

| Type | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| NodeId (origin, sender, next hop, neighbor, vehicle, producer) | 8 | 8-byte certificate identifier | A | V13.2 Section 7.2: "only an 8-byte certificate identifier travels over the air"; Table C.3: "sender id (8), next-hop id or broadcast (8)"; Table 5.2: "Neighbour id (8)". All-ones = broadcast; all-zero = headquarters. | 2-byte roster index (previous draft): smaller, but the architecture fixes 8 bytes. The LSC beacon's 2-byte vehicle id (RSD-SAF-02d) belongs to the LSC's raw frame only. |
| CertId (operator, excluded certificate) | 8 | 8-byte certificate identifier | A | V13.2 Section 7.2, Table 7.5. | None. |
| SuiteSchemaId | 1 | u8, one value per (format version, crypto suite, schema version) combination, listed in the specification | A | V13.2 Appendix C: "versioned with a one-byte suite and schema identifier"; Table C.3: "Version and suite (1)". Values are an enumeration, not bit fields. | u16 schema version (previous draft): the architecture fixes one byte. Separate version and suite bytes: the architecture fixes one. |
| Hash48 | 48 | bytes | † | SHA-384 (Table 2.1, RSD-CRY-01d). | None. |
| RecordId | 48 | Hash48 | † / A | RSD-EVD-02a; V13.2 Table 8.1. | Truncation: the identifier is a content address. |
| BundleId | 48 | SHA-384(`"TULPAR/1/bundle-id"` ‖ bundle header) | A meaning / † width | HL-12: the identifier of a record sent to headquarters "is the content hash of the signed bundle; it never changes and is the same in every copy". The bundle header holds the Merkle root, so it commits to every record; one bundle per origin and window (RSD-EVD-01e). The signature bytes are left out so that the identifier does not depend on whether signing is deterministic (RSD Section 4.5 rationale: "Equivocation is detected on canonical content, not on signature bytes"). Computable from a copy that carries only the header, the signature and one Merkle path (Section 4.13). | Hash over the whole bundle including all records: a copy for headquarters would have to carry every record of the bundle. |
| Signature | 4627 | bytes | ✔ | ML-DSA-87 (Table 2.1, RSD-CRY-01b). | None. |
| MlDsaPublicKey | 2592 | bytes | † | Table 2.1. | None. |
| MlKemEncapsKey, MlKemCiphertext | 1568 | bytes | ✔ / † | Table 2.1; V13.2 Table C.3. | None. |
| GcmTag | 16 | bytes | ✔ / A | Table 4.26; V13.2 Table C.3. | None. |
| Window | 4 | u32 window index from mission start | proposal | The mission configuration sets the mission length and no source bounds it; u32 cannot wrap. | u16: 91 hours; would need a wrap rule. |
| SimTime | 8 | u64 simulation time, integer nanoseconds | proposal | One time unit across the system, as in the event log (RSD-SIM-01). | u32 ms: a second time unit. |
| EpochRef | 12 | producer NodeId (8) ‖ epoch number u32 (4) | proposal | See "Epoch identifier" below. | See below. |
| CommandId | 5 | console u8 ‖ console sequence u32 | proposal | Table 4.26 and V13.3 C19: "command id = (console, console sequence)". Consoles are named in the mission plan (HL-30: a primary and an optional console). | u16 console: no mission names more than a few consoles. u16 sequence: a wrap would break RSD-HOT-06g. |
| Position | 12 | 3 × i32: x east, y north, z up; local ENU from the mission-configuration origin; centimeters | proposal | 12 B matches the LSC beacon's "position and altitude (12 B)" (RSD-SAF-02d) and V13.2 Table 5.2 "pos 12". Integer kinematics without trigonometry (RSD-EVD-03b). | Latitude/longitude (V13.2 Table C.1 for observations): needs trigonometry and is exposed to platform rounding. See D9. |
| PositionSource | 1 | u8: 0 GNSS, 1 radar fix, 2 mutual tracking, 3 other | A set / proposal codes | RSD-OBS-03a; V13.2 Table 5.2 "pos_src 1". | None. |
| ErrorBound | 2 | u16 decimeters, saturating at 6553.5 m | A width / proposal unit | V13.2 Table 5.2 "pos_err 2". | None. |
| Etx | 2 | u16 Q8.8 fixed point (value × 256), saturating | A width / proposal scale | V13.2 Table 5.2 "quantised ETX ... (2 + 2)". ETX ≥ 1; the hysteresis band of ±0.5 (V13.2 Section 5.4) needs better than 0.5 resolution. | Q4.12: range only up to 16. |
| BuildId | 1 | u8: 1 E, 2 I, 3 R | proposal codes | RSD-CMC-05b: three builds. V13.3 C07 "build identifier per group" is not used (RSD governs). | None. |

### Epoch identifier (decided: E1 + E3)

**Width.** The epoch reference is carried in every record header and in HELLO, and partitioned components open epochs without knowing of each other (V13.2 Section 7.5; RSD-CRY-03i). It must therefore be unique across components without coordination. A reference that contains the producer's 8-byte identity and a producer-local number is unique by construction across producers; two values from one producer can only collide inside that producer's own chain.

| Option | Width | Verdict |
|---|---|---|
| Counter only (u32) | 4 | Rejected: two components both open "epoch 5". |
| Random value | 8 | Rejected: chosen by the producer, so a compromised producer can copy another component's value. |
| Truncated hash of the announcement, 16 B | 16 | Rejected: probabilistic, and 4 B larger than the structured reference. |
| Full SHA-384 of the announcement | 48 | Rejected: 36 B more in every record header and in HELLO. |
| **Producer NodeId (8) ‖ epoch number u32 (4)** | **12** | **Chosen.** Unique across producers by construction; the producer part is the identity that already travels over the air. |

**Uniqueness rule (decided by Özgür: E1 + E3).**

- **E1, semantic key as in the architecture.** The epoch number is a u32 chosen by the producer and strictly increasing over its own announcements. The semantic uniqueness key is (producer, epoch number), as in V13.2 Table 8.2 "(producer, epoch_id)". Two different announcements under one key are an equivocation (G3, RSD-SEC-01d).
- **E3, deterministic adoption between announcements of one producer.** Among announcements of the same producer that share a parent epoch, every agent adopts the one with the smallest epoch number and keeps the others for decryption only, for two windows, in the same way RSD-CRY-03h orders two producers. Agents with the same evidence therefore adopt the same epoch (RSD-EVD-03a), and no new proof type is introduced. RSD-CRY-03h does not yet state this case (F19).

Not chosen: E2 (semantic key (producer, parent epochs)), which would change the key of V13.2 Table 8.2 and turn a legitimate re-announcement from the same parent into a proof; E4 (epoch number = the announcement's record sequence number), which reduces the semantic key to the chain position and leaves the same gap as E1 alone.

The initial epoch from the provisioning package is (all-zero NodeId, 0).

## 4. Records and frames

Fields are listed in the order of Table 4.26. "A" rows cite the architecture table that gives the size.

### 4.1 Common record header (RSD-EVD-02, RSD-EVD-05, RSD-OBS-03; V13.2 Table 8.1)

| Field | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| suite and schema identifier | 1 | SuiteSchemaId | added field / A | RSD-CMC-02c requires a schema version on every record; V13.2 Appendix C gives one byte. | None. |
| identifier (rid) | 0 | RecordId | derived | Computed by K13 from the other fields. V13.2 Table 8.1 lists rid in the header (D1). | Carry it: a second source of truth with an unclassified mismatch case (F8). **S1.** |
| origin | 8 | NodeId | A | Section 3. | None. |
| record sequence number (rseq) | 4 | u32 | proposal | Chain position; cannot wrap in any mission. | u16: can be exceeded in long missions. |
| previous-record hash (prev) | 48 | Hash48 | † | SHA-384; chain integrity (RSD-EVD-05a). | None. |
| window | 4 | Window | proposal | Section 3. | None. |
| epoch | 12 | EpochRef | proposal | Section 3. | None. |
| type | 1 | u8 record type code | proposal | Closed set (V13.2 Table 8.2 plus later types); a new type is a schema change (RSD-CMC-02c). | u16: no need. |
| uniqueness key (ukey) | 0 | derived from type and payload | derived / A | V13.2 Table 8.2 defines the semantic key per type from payload fields: LSA (origin, lsa_seq); CONTRACT (origin, contract_id); epoch announcement and revocation (producer, epoch_id) (see Section 3); proof: content-addressed. Other types have none. Every key is made of fields that are already in the record. | A separate 16 B key field (previous draft): repeats payload information. |
| supersedes | 1 + 0/48 | optional RecordId (K8) | A meaning / proposal width | V13.2 Table 8.1: "Identifier of an earlier record of the same origin that this record updates, or none". | Always 48 B: empty in most records. |
| parents | 1 + 48·n | u8 count + RecordId list, K6, n ≤ 16 | A meaning / proposal bound | V13.2 Table 8.1: "Identifiers of the records it rests on". | u16 count: no case needs it. **Bound 16: S3.** |
| common-cause tags (roots) | 3 | sensor profile u8, build u8 (software version), position source u8; 0xFF = not applicable | A meaning / proposal widths | V13.2 Table 8.1: "Declared common-cause tags: sensor profile, software version, position source". Receivers take sensor model and software version from the mission configuration, never from the record (RSD-OBS-03d). Software version = BuildId. | Variable list of tag codes (previous draft): the architecture fixes the three tag kinds, so a fixed layout is simpler. |
| policy hash | 48 | Hash48 | † / A | V13.2 Table 8.1: "Hash of the policy version used"; Section 7.2: the policy set is "identified by its version hash". | None. |

Fixed part without supersedes and parents: 1+8+4+48+4+12+1+1+1+3+48 = **131 B**.

### 4.2 Signed bundle (RSD-EVD-01, RSD-CRY-01b; V13.2 Table C.4)

| Field | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| suite and schema identifier | 1 | SuiteSchemaId | added field / A | Rejection before signature verification (RSD-EVD-01g). | None. |
| origin | 8 | NodeId | A | Needed before any record is parsed (RSD-EVD-01g). | None. |
| window | 4 | Window | proposal | One bundle per origin and window (RSD-EVD-01e). | None. |
| first record sequence | 4 | u32 | proposal | None. | None. |
| last record sequence | 4 | u32 | proposal | Record count = last − first + 1; no separate count. | Count field: repeats information. |
| Merkle root | 48 | Hash48 | † | RFC 9162-style tree: leaf = SHA-384(0x00 ‖ record), node = SHA-384(0x01 ‖ left ‖ right), a lone node is promoted. | None. |
| records | Σ (2 + Lᵢ) | u16 length + canonical record, in sequence order | proposal | None. | None. |
| signature | 4627 | Signature | ✔ | Signed message = `"TULPAR/1/bundle"` ‖ bundle header (the six fields above). The records are covered through the Merkle root. This is what lets a proof carry "the two conflicting bundle headers with signatures and the Merkle paths of the conflicting records" (V13.2 Table C.4) instead of two whole bundles. | Sign header, records and root (previous draft): a proof would have to carry the whole bundles. RSD-EVD-01a wording: see F18. |

Origin, window and sequence numbers appear in the bundle header and in each record, because a record travels alone (pull, proofs) and is hashed with all its fields (RSD-EVD-02a), while the bundle header is checked before any record is parsed (RSD-EVD-01g). By K14 the decoder checks that every record's origin and window equal the bundle's and that the sequence numbers run from first to last.

Fixed part: 1+8+4+4+4+48 = 69 B header + 4627 B signature = **4 696 B**. A 5 500 B bundle is four fragments of at most 1 400 B (Section 4.4).

### 4.3 Digest (RSD-EVD-01c, RSD-EVD-03; V13.2 Table C.4)

A frame payload under the swarm key (RSD-CRY-03c); the sender is the overlay header's sender.

| Field | Width | Kind | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|---|
| window | 4 | Window | A / proposal width | V13.2 Table 7.4 gives the digest's replay protection as "Counter and window". | None. |
| origin set | ceil(N/8) | roster bitmap (K10) | proposal | Which origins have an entry; entries follow in roster order, so no per-entry origin. | 8 B origin per entry: 160 B for 20 origins. |
| per origin: highest record sequence | 4 | u32 | proposal | Same as rseq. | None. |
| per origin: chain-head prefix | 8 | bytes | ✔ / A | Table 4.26; V13.2 Table C.4. | None. |
| evidence-set hash | 48 | Hash48 | † | SHA-384; its computation is defined with the Evidence Store. | None. |
| state hash | 48 | Hash48 | † | SHA-384. | None. |
| policy version | 48 | Hash48: policy-set version hash | A meaning / † width | V13.2 Section 7.2: the policy set is "identified by its version hash"; the record header carries the same value. | u16 number: no source defines one. |
| nodes heard directly | ceil(N/8) | roster bitmap | proposal | Fixed size, one encoding. | u8 count + 8-byte NodeId list: up to 153 B, needs K6 checks. |

Full digest for N = 20 with 20 origins:

| Part | Bytes |
|---|---|
| Payload: 4 + 3 + 20·12 + 48 + 48 + 48 + 3 | **394 B** |
| Frame with the 52 B overlay header | **446 B** |
| Against Table 4.29 "about 370 B" | payload +24 B; frame +76 B |
| Digest share (0.26 %), linear scaling | about 0.28 % (370 B as payload) or 0.31 % (370 B as frame) |

### 4.4 Overlay frame header (RSD-NET-01, RSD-CRY-01g; V13.2 Table C.3)

| Field | Width | Kind | Marker | Rationale |
|---|---|---|---|---|
| version and suite | 1 | SuiteSchemaId | A | Table C.3 "Version and suite (1)". |
| frame type | 1 | u8 enum | A | Table C.3 "frame type (1)". Closed set: HELLO, HS1, HS2, HS3, hop ACK, telemetry, digest, bundle fragment, pull, TDP data, ACK frame. |
| sender | 8 | NodeId | A | Table C.3 "sender id (8)". |
| next hop or broadcast | 8 | NodeId; all-ones = broadcast | A | Table C.3 "next-hop id or broadcast (8)". |
| counter | 8 | u64 | ✔ / A | "8-byte counter (in clear, covered by the tag)". Strictly increasing per sender and key; IV input (RSD-CRY-01g) and replay protection. |
| message id | 8 | u64 per sender | A | Table C.3 "message id (8)". |
| fragment index | 1 | u8 | A | Table C.3 "fragment index and count (1 + 1)". At most 255 × 1 400 B = 357 000 B per message. |
| fragment count | 1 | u8 | A | Same. |
| tag | 16 | GcmTag, or truncated HMAC under the discovery key for HELLO | ✔ / A | "hop tag or swarm-key tag (16)". Last header field; the authenticated header bytes exclude it. |

Header: **52 B**. The 1 400 B of RSD-NET-01f is the fragment's message content; the header is added on top. A full frame is 1 452 B, within the 2 296 B Wi-Fi MTU (1 480 B with UDP and IPv4). This matches "5.5 kB; four fragments" (Table 4.29). The RSD text allows both readings (F3).

### 4.5 HELLO (RSD-NET-02d, RSD-CRY-03a; V13.2 Table C.3)

An overlay frame with frame type HELLO and next hop broadcast. Never encrypted or tagged under the swarm key (RSD-CRY-03b, RSD-CRY-03c). The header's tag field carries the truncated HMAC under the discovery key.

| Field | Width | Where | Marker | Rationale |
|---|---|---|---|---|
| node id | 0 | header sender | derived | The sender field is the node id. |
| epoch | 12 | body | proposal | EpochRef. |
| sequence number | 0 | header counter | derived | The header counter is strictly increasing per sender; V13.2 Table 7.4 gives HELLO replay protection as "Sequence number and timestamp". |
| timestamp | 8 | body | proposal | SimTime. Send time, distinct from the counter's order. |
| HMAC-SHA-384 (truncated) | 16 | header tag | ✔ / A | Key: discovery key. Input = `"TULPAR/1/hello"` ‖ header without the tag ‖ body; first 16 bytes kept. |

Size: 52 + 20 = **72 B**. Table 4.29 assumes 48 B: +24 B; HELLO share from 0.32 % to about 0.48 % by linear scaling (F4).

### 4.6 Handshake HS1 / HS2 / HS3 (RSD-NET-02a, RSD-CMC-04a; V13.2 Section 7.3; V13.3 C07)

V13.2 Section 7.3: "The initiator sends a nonce and a fresh ML-KEM-1024 encapsulation key. The responder encapsulates a shared secret, signs the transcript with ML-DSA-87 and adds a key-confirmation code. The initiator signs the transcript and adds its own key confirmation." V13.3 C07: "In HS2 and HS3 each side adds a quote". The suite id of Table 4.26 is carried by the overlay header's version-and-suite byte (derived) and covered by the transcript signature.

| Message | Fields (in order) | Width | Fragments |
|---|---|---|---|
| HS1 | nonce (32), ML-KEM encapsulation key (1568) | 1 600 B | 2 |
| HS2 | nonce (32), ML-KEM ciphertext (1568), transcript signature (4627), key confirmation (48), quote (4676) | 10 951 B | 8 |
| HS3 | transcript signature (4627), key confirmation (48), quote (4676) | 9 351 B | 7 |

| Field | Width | Marker | Rationale | Alternative and why not |
|---|---|---|---|---|
| nonce | 32 | proposal | 256 bits, the suite's security level; the quote binds it (RSD-CMC-04a). | 16 B: below the suite's level. |
| key confirmation | 48 | proposal | Full HMAC-SHA-384 (RSD-CRY-01d). | 16 B truncated: weaker for the value that proves key possession. |
| quote | 1 + 48 + 4627 = 4676 | proposal, assumption | build id (BuildId), boot measurement (SHA-384), attestation signature; the session nonce is signed, not sent again (RSD-CMC-04a; V13.3 C19 "Quote: boot measurement, build id, session nonce, attestation-key signature"). **Assumption:** attestation signature is ML-DSA-87 (RSD-CRY-01b; no source names another algorithm) (F2). | None. |

Total **21 902 B** per handshake. Table 4.29: 12 390 B (1568 + 1568 + 2 × 4627; V13.2 Section 7.3: "about 2⋅1568+2⋅4627 ≈ 12.4 kB plus small fields"), without quotes. Without quotes this layout is 12 550 B (+160 B: nonces 64, key confirmations 96). Handshake share from 0.58 % to about 1.03 % (F1).

### 4.7 Link-state advertisement (RSD-NET-02; V13.2 Table 5.2)

Common record header + body. All sizes in this table come from V13.2 Table 5.2.

| Field | Width | Marker | Rationale |
|---|---|---|---|
| lsa_seq | 4 | A | "Strictly increasing; an older or equal sequence number is rejected" (RSD-NET-02e). Part of the ukey (origin, lsa_seq). |
| pos, pos_src, pos_err | 12 + 1 + 2 | A | Position, PositionSource, ErrorBound (Section 3). |
| n | 1 | A | Number of neighbor entries; n ≤ 19 (proposal bound: 20-node fleet). |
| nbr[i] | n × 14 | A | Neighbor NodeId (8), ETX forward and reverse as measured by the origin (2 + 2, Etx), age of the last measurement (2: proposal unit u16 ms, saturating; the measurement window is 10 s and an edge older than 60 s is deleted, V13.2 Section 5.4). Ascending NodeId. |

No backhaul information (RSD-HOT-04b).

### 4.8 Epoch announcement and revocation record (RSD-CRY-03, RSD-ORG-03d; V13.2 Table C.4, Section 7.7)

Common record header + body.

| Field | Width | Marker | Rationale |
|---|---|---|---|
| epoch id | 4 | proposal | Epoch number u32, strictly increasing per producer (E1, Section 3); the full EpochRef is (header origin, epoch number). Semantic key (producer, epoch number). |
| parent epochs | 1 + 12·n | proposal | u8 count + EpochRef list, K6, 1 ≤ n ≤ 2 (two on a merge, RSD-CRY-03i). **Bound: S3.** |
| recipient list | ceil(N/8) | proposal | Roster bitmap. |
| hash of the new key | 48 | † | SHA-384 (RSD-CRY-03d). |
| producer | 0 | derived | = header origin. V13.2 Table C.4 lists "producer id" (no repetition). |
| excluded certificate id | 1 + 0/8 | A width | Optional CertId; present only in a revocation. |
| embedded proof | 0 / 4 + L | proposal | Optional, u32 length + proof (Section 4.9). |

### 4.9 Proof (RSD-SEC-01d; V13.2 Table C.4, Table 8.2; V13.3 C08)

Not in Table 4.26 (F13), but needed by the revocation record. V13.2 Table C.4: "The two conflicting bundle headers with signatures and the Merkle paths of the conflicting records; or the forbidden-type record; or the invalid revocation record". V13.3 C08 and RSD-SEC-01d add the canonically malformed signed record. A proof is also a record type of its own (V13.2 Table 8.2: "Content-addressed").

A **signed inclusion** shows that a record was signed: bundle header (69 B), bundle signature (4627 B), the record (u16 length + bytes) and its Merkle path (u8 count + 48 B per level; the leaf index is rseq − first, derived).

| Field | Width | Marker | Rationale |
|---|---|---|---|
| kind | 1 | proposal codes | u8: 1 equivocation, 2 forbidden type, 3 invalid revocation record, 4 canonically malformed signed record (RSD-SEC-01d). |
| inclusions | 1 or 2 signed inclusions | A content / proposal layout | Kind 1: two, at one chain position or under one semantic key, in ascending record-identifier order. Kinds 2 to 4: one. For kind 4 the record bytes are carried as signed, even though they do not parse. |

An equivocation proof with two 300 B records and five-level paths is about 2 × (69 + 4627 + 302 + 241) ≈ 10.5 kB, in line with Section 4.10 of the RSD ("a G3 proof of about 10.5 kB").

### 4.10 Load receipt (RSD-ORG-05; V13.2 Table C.4; V13.3 C07, C19)

| Field | Width | Marker | Rationale |
|---|---|---|---|
| vehicle id | 8 | A | NodeId. |
| mission manifest hash | 48 | † | SHA-384; covers the schema version (V13.3 C07). |
| device envelope hash | 48 | † | SHA-384 (RSD-ORG-05b). |
| boot quote | 4676 | proposal, assumption | As in Section 4.6 (V13.3 C19 "Boot quote of the loaded image"); the nonce is the mission manifest hash, already in the receipt. |
| signature | 4627 | † | ML-DSA-87. |

### 4.11 Operator command (RSD-HOT-02, RSD-HOT-06; V13.3 C10, C19; HL-41)

Signed by operators, not part of a vehicle chain, so no common record header. Content hash (RSD-HOT-06g) = SHA-384(`"TULPAR/1/command"` ‖ all fields except the signatures). Presence of duration, validity and precondition follows the type (V13.3 C19), so no presence flag is sent except for ABORT's validity.

| Field | Width | Marker | Rationale and source |
|---|---|---|---|
| suite and schema identifier | 1 | added field / A | RSD-CMC-02c. |
| mission id | 48 | † | Mission manifest hash (RSD-HOT-02d). |
| command id | 5 | proposal | CommandId. |
| type | 1 | ✔ set / proposal codes | 1 ABORT, 2 HOLD, 3 RESUME, 4 PRIORITY, 5 SUPPORT (RSD-HOT-02a). |
| target | 1 + v | A kinds / proposal layout | Kind u8 + value, by V13.3 Table 11.12: ABORT "Vehicle, group or collective"; HOLD and RESUME "Group or sector"; PRIORITY "Contact or sector"; SUPPORT "Requesting and helping group". Values: vehicle NodeId (8), group u8, collective (0), sector u8, contact id (16, proposal), group pair (2). A kind not allowed for the type is malformed. |
| parameters | 0 or 1 | A / proposal | PRIORITY: importance class u8 ("Sets the importance class I(n) within the policy range", V13.3 Table 11.12). Other types: none (ABORT's form is the presence of validity). SUPPORT's request content is not defined (F11). |
| issue window | 4 | proposal | Window. |
| duration | 2 (HOLD only) | proposal | u16 seconds; 1 to 1800 (RSD-HOT-02e). |
| validity | 2 (RESUME); 1 + 0/2 (ABORT) | A presence / proposal width | u16 seconds. RESUME: required (RSD-HOT-02f). ABORT: present = immediate, absent = recall (V13.3 "Final clarifications"; RSD-HOT-01j, RSD-HOT-01k). |
| precondition | 1 (RESUME only) | proposal | u8 predicate code. **The predicate set is defined in no source (F11).** |
| basis | 48 | A meaning / † width | "the identifier of the summary the operator saw" (V13.3 C10; HL-41 "basis (summary id)"): the BundleId of the signed bundle that carried the summary to headquarters (HL-12; Section 3). |
| signatures | 1 + 4635·n | A / ✔ | u8 count (1 or 2) + {operator CertId 8, Signature 4627}, ascending CertId, distinct certificates; each signature over the canonical encoding of all other fields (HL-41; RSD-HOT-06c). |

### 4.12 Command receipt (RSD-HOT-01m; V13.3 C19)

Common record header + body; carried in the vehicle's next bundle.

| Field | Width | Marker | Rationale |
|---|---|---|---|
| command id | 5 | proposal | CommandId. |
| vehicle | 0 | derived | = header origin. |
| result | 1 | ✔ set / proposal codes | 1 APPLIED, 2 VOID, 3 REJECTED. |
| window | 0 | derived | = header window. |

Proposal: add the command's content hash (48 B), because RSD-HOT-06g distinguishes copies by command id and content hash; this would be an added field (**S10**).

### 4.13 Group Status Summary (RSD-HOT-03; V13.3 C10, C19; HL-11, HL-15)

Common record header + body; carried in the producer's next signed bundle (RSD-HOT-03c). Covers the members of the producer's own group in that component (RSD-HOT-03a). Content from V13.3 Section 11.10 and Table C.5 with HL-11 and HL-15. The list bounds marked "Berra" await Berra's approval.

| Field | Width | Marker | Rationale |
|---|---|---|---|
| group | 0 | derived | Roster group of the producer (header origin). |
| producer, window | 0 | derived | Header origin and window. |
| members covered, with compact positions | 1 + 29·n | A content / proposal widths | n ≤ group size in the roster. Per member: vehicle NodeId 8, Position 12, time of fix SimTime 8 (HL-15: "vehicle id, position, altitude, time of fix"), security state u8 (1 Trusted, 2 Watch, 3 Restricted, 4 Probation, 5 Quarantined). "Vehicles per security state" is carried by the per-member state, not by a second list. |
| energy minimum and median | 2 | A content / proposal unit | Two u8 values in percent of the vehicle's usable energy (V13.3: "energy (minimum and median)"). No source gives a unit; percent keeps vehicles of different capacity comparable. Alternative: u16 Wh, which needs a per-type capacity the mission configuration does not define. |
| channels OK per modality | 3 | A content / proposal | u8 count each for EO, IR, radar. |
| coverage and regime per sector | 1 + 4·s | A content / proposal | u8 count + {sector u8, coverage u16 per mille of the sector's cells observed, regime u8 (1 S1, 2 S2, 3 S3; RSD-OBS-04a)}. V13.3: "sector coverage; evidence capacity regime of each sector". |
| top backlog items | 1 + 49·k | A bound / proposal layout | k ≤ 3: "the three highest Verification Backlog items" (V13.3). Per item: item_id 48 († SHA-384; V13.2 Table 11.7 "Hash of (contact id, attribute, need type, region cell, window)"), observation status u8. |
| contacts by status | 7 | A content / proposal | u8 count per observation status (seven statuses, RSD Table 4.7). |
| most important contacts | 1 + 29·c | A content / proposal (Berra) | c ≤ 16; {contact id 16 (proposal; no source sizes it), Position 12, status u8}. V13.3: "contacts by status, with the most important ones". |
| alerts | 1 + Σ | A types / proposal (Berra) | a ≤ 8. Types from V13.3 Section 11.10: 1 G3 event, 2 Safe mode, 3 vehicle Alone (each + vehicle NodeId 8 → 9 B), 4 standing order close to firing (+ standing-order index u8 → 2 B). The jamming level and commands applied are separate fields. |
| jamming level | 1 | A content / proposal codes | u8 0 to 4 = J0 to J4 (Table 4.19). |
| commands applied by its members | 1 + 5·m | A content / proposal (Berra) | m ≤ 8; CommandId list. |

V13.3 Section 11.10 also lists "open repairs and contracts"; neither Table 4.26 nor V13.3 Table C.5 has it, so it is left out (D5).

Size at the proposed bounds, all alerts of type 1 to 3, three sectors (calculation):

| Group | Members | Body | Record with the 131 B header | Against "about 1 kB" (Table 4.29) |
|---|---|---|---|---|
| B | 7 | (1 + 29·7) + 2 + 3 + (1 + 4·3) + (1 + 49·3) + 7 + (1 + 29·16) + (1 + 9·8) + 1 + (1 + 5·8) = **957 B** | 1 088 B | body −43 B; record +88 B |
| A, C | 6 | **928 B** | 1 059 B | body −72 B; record +59 B |

**Copy to headquarters.** What goes to headquarters is not the summary record alone but the sealed signed bundle that carries it (RSD-HOT-05c, RSD-HOT-05d, HL-12): headquarters checks the bundle signature, the Merkle root and the record chain. The smallest copy carries the bundle header, the signature, the Merkle path of the summary record and the record itself, sealed to the GCS service key. Its identifier is the BundleId (Section 3), which the GCS receipt and a command's basis name.

| Part | Bytes | Source |
|---|---|---|
| ML-KEM-1024 ciphertext (seal to the GCS service key) | 1 568 | RSD-HOT-05c; RSD-NET-05e pattern |
| Bundle header | 69 | Section 4.2 |
| Bundle signature | 4 627 | ML-DSA-87 |
| Merkle path of the summary record, depth 3 | 1 + 3·48 = 145 | **Assumption:** the bundle holds at most 8 records |
| Summary record (group B, proposed bounds) | 2 + 1 088 = 1 090 | Section 4.13 |
| AES-256-GCM tag of the sealed content | 16 | Table 2.1 |
| **Copy** | **7 515 B** (6 fragments of ≤ 1 400 B) | Calculation |

Channel share by the method of Table 4.29 (airtime of 1.99 ms per 1 400 B, applied per byte as in the bundle row), with the row's own assumptions of three components, at most four copies and at most 10 hops every 30 s: 120 × (7 515 / 1 400) × 1.99 ms / 30 s ≈ **4.27 %** (about 3.2 % with the reference configuration's three gateways). The same method gives about 0.57 % for the 1 kB that the table assumes, so the table's "≤ 0.4 %" is not reproduced by its own assumptions. Scaling the table's 0.4 % linearly by size gives about 3.0 %. All values are calculations (F15).

### 4.14 GCS service certificate (RSD-HOT-05, RSD-HOT-06d; HL-44)

| Field | Width | Marker | Rationale |
|---|---|---|---|
| role | 1 | proposal codes | u8: 1 vehicle, 2 operator, 3 gcs (HL-44: role "gcs"; V13.3 C19: role "operator"). |
| mission id | 48 | † | RSD-HOT-02d. |
| ML-KEM-1024 public key | 1568 | † | Table 2.1. |
| ML-DSA-87 public key | 2592 | † | Table 2.1. |
| validity | 8 | proposal | Two Window values, not before and not after. |
| headquarters root signature | 4627 | † | ML-DSA-87. |

### 4.15 Backhaul hello (RSD-HOT-04, RSD-HOT-07; HL-38)

Unsigned; sent by each end once per window; never forwarded into the mesh.

| Field | Width | Marker | Rationale |
|---|---|---|---|
| sender | 8 | A | NodeId of the gateway; all-zero = headquarters. |
| link id | 1 | proposal | u8; a gateway has one backhaul (RSD-HOT-04c). |
| window | 4 | proposal | Window. |
| sequence | 4 | proposal | u32. |

### 4.16 LSC beacon

Out of scope: it stays in the LSC's own source tree (RSD-SAF-02d, RSD-SAF-02f).

## 5. Formats needed but not in Table 4.26

| Format | Defined in | Note |
|---|---|---|
| GCS receipt | RSD-HOT-05f ("names the mission, the record identifier and the GCS window"); HL-44 | Not designed here; the record identifier it names is the BundleId (Section 3). |
| Proof | V13.2 Table C.4; RSD-SEC-01d | Designed in Section 4.9 because the revocation record embeds it. |
| Hop ACK, telemetry, TDP header, ACK frame, PERSISTED list | V13.2 Table C.3 | TULPAR-480 (owner-cemre). |
| Operator certificate, provisioning package | V13.3 C19; V13.2 Table 7.5 | TULPAR-481 (owner-ozgur). |
| Observation and finding record bodies | V13.2 Tables C.1, 8.2, 8.5 | TULPAR-482 (owner-berkan). |
| Task, bid, repair lease, support and CONTRACT record bodies | V13.2 Table C.5 | TULPAR-483 (owner-berra). |
| Link-measurement and corrupted-copy fact record bodies | V13.2 Table C.2 | TULPAR-484 (owner-cemre). |

The common rules (Sections 2 and 3) and the common record header (Section 4.1) stay in this document; those issues define only their own bodies and frames.

## 6. Code approach (after approval)

RSD-CRY-02e: *"The canonical-encoding validator shall be generated from the same specification as the record decoders and be identical in every build."* Proposal: once approved, the tables here become a machine-readable specification (for example `docs/design/encodings.json`), and the codecs under `common/formats/` are generated from it. **S11.**

## 7. Decisions needing approval

| No | Decision | Proposal |
|---|---|---|
| S1 | Record identifier on the wire? | No; derived (D1, F8). |
| S3 | List bounds: parents 16, parent epochs 2, LSA neighbors 19; GSS contacts 16, alerts 8, commands 8 (Berra) | As listed; all proposals. |
| S5 | HELLO node id and sequence number | Carried by header sender and counter. |
| S6 | Distribution of fields over HS1 to HS3 | As in Section 4.6 (from V13.2 Section 7.3 and V13.3 C07). |
| S7 | Quote signature algorithm | ML-DSA-87 assumed (F2). |
| S10 | Command content hash in the command receipt | Yes (added field). |
| S11 | Codecs generated from a specification (RSD-CRY-02e) | Yes. |
| S12 | Positions: local ENU, i32 cm, for every format | Yes (D9). |
| S13 | Nodes heard directly, recipient list and origin set as roster bitmaps | Yes (D8). |
| S14 | Window in the digest | Decided: yes, 4 B (V13.2 Table 7.4). |
| S15 | Epoch uniqueness rule | Decided: E1 + E3 (Section 3). |
| S16 | Bundle signature over header and Merkle root | Yes (F18). |
| S17 | Units: energy in percent, coverage in per mille, LSA age in ms, ETX Q8.8 | Yes. |
| S18 | Identifier named by the basis and the GCS receipt | Decided: the BundleId of HL-12 (Section 3). |

## 8. RSD findings

Places in RSD v1.1 (tag `rsd-v1.1`) found wrong, ambiguous or outdated. Each quotes the RSD verbatim, gives the reason and proposes a correction. The RSD stays binding until a pull request changes it; the corrections are tracked in TULPAR-479. Numbers are calculations; shares are scaled linearly from the table's own figures, with per-frame overhead not modelled separately.

### Effect on Table 4.29

| Row | RSD v1.1 | With the frozen encodings (calculation) |
|---|---|---|
| HELLO | 48 B, 0.32 % | 72 B, about 0.48 % |
| Hop handshakes | about 12.4 kB, 0.58 % | 21 902 B, about 1.03 % (ML-DSA-87 quote assumed) |
| Digests | about 370 B, 0.26 % | 394 B payload / 446 B frame, about 0.28–0.31 % |
| Control plane | 0.90 % | about 1.51 % (hypothesis threshold 5 %) |
| Group Status Summary | about 1 kB, ≤ 0.4 % | 7 515 B per copy, about 4.27 % (four copies, 10 hops) |
| Evidence plane | 4.10 % | about 4.12–4.15 % (cap 10 %) |
| Total | ≤ 7.7 % | about 12.2 % |

### Findings

| No | Kind | RSD text (verbatim) | Reason | Proposed correction |
|---|---|---|---|---|
| F1 | Outdated | Table 4.29: "Hop handshakes \| 190 pairs, about 12.4 kB per handshake, every 600 s \| 0.58 %". RSD-CMC-04a: "In handshake messages HS2 and HS3, each side shall include a quote, signed by its key provider's attestation key, over the boot measurement of the loaded agent image, the build identifier and the session nonce." | 12.4 kB = 1568 + 1568 + 2 × 4627 = 12 390 B (also V13.2 Section 7.3); the two quotes added by RSD-CMC-04a (V13.3 C07) are not counted. Frozen layout: 21 902 B. | "190 pairs, about 21.9 kB per handshake (two boot quotes included), every 600 s \| about 1.03 %"; update the control-plane line and the total. |
| F2 | Ambiguous | RSD-CMC-04a: "a quote, signed by its key provider's attestation key". RSD-CRY-01b: "Bundles, handshake transcripts, link-state advertisements and every other signed record shall be signed with ML-DSA-87." | No algorithm or size is given for the attestation key, in the RSD or the architecture (V13.3 C07: "a separate attestation key"). | RSD-CMC-04a or Table 2.1: "The attestation key is an ML-DSA-87 key; a quote signature is 4627 B." |
| F3 | Ambiguous | RSD-NET-01f: "The overlay shall split every message into fragments of at most 1400 bytes, each carrying its own hop or swarm-key tag." Table 4.29: "A 1400-byte fragment takes 1.99 ms." and "5.5 kB; four fragments". | Whether 1400 bytes includes the 52 B overlay header is not stated; with the header included a 5 500 B bundle is five fragments. If 1.99 ms excludes the header, the bundle share rises from 3.13 % to about 3.25 % and pull from 0.63 % to about 0.65 %. | "...into fragments of at most 1400 bytes of message content, each sent in one overlay frame that adds the frame header and its own hop or swarm-key tag." State which headers the 1.99 ms includes. |
| F4 | Outdated | Table 4.29: "HELLO \| Every 2 s per node; 48 B \| 0.32 %". | With the architecture's 52 B overlay header (V13.2 Table C.3), a 12 B epoch reference and an 8 B timestamp, HELLO is 72 B. | "Every 2 s per node; 72 B \| about 0.48 %". |
| F5 | Outdated | Table 4.29: "Digests \| Every node every window; about 370 B \| 0.26 %". | Frozen digest for N = 20, with the 4 B window of V13.2 Table 7.4: 394 B payload, 446 B frame. | "about 446 B including the overlay header \| about 0.31 %". |
| F6 | Wrong (omission) | Table 4.26, Common record header: "Identifier (SHA-384 of the canonical encoding), origin, record sequence number, previous-record hash, window, epoch, type, uniqueness key, supersedes, parents, common-cause tags, policy hash". RSD-CMC-02c: "A record that carries a schema version other than the mission's shall be rejected without forming a proof." | RSD-CMC-02c needs a schema version on every record; Table 4.26 omits it. The architecture has it: V13.2 Appendix C, formats are "versioned with a one-byte suite and schema identifier". | Add "suite and schema identifier (1 B)" to the common record header and the signed bundle in Table 4.26. |
| F8 | Ambiguous | Table 4.26: "Identifier (SHA-384 of the canonical encoding)". RSD-EVD-02a: "The identifier of a record shall be the SHA-384 hash of the canonical encoding of all its other fields." | The identifier is fully determined by the other fields; how a transmitted identifier that does not match the content is classified is not stated. | "Identifier (SHA-384 of the canonical encoding of all other fields; derived by the receiver, not transmitted)". |
| F9 | Ambiguous | RSD-EVD-01c: "a hash of its derived states with the policy version". Table 4.26: "policy hash". Section 4.10: "Every decision record carries its evidence identifiers, policy version and open hypotheses". | Two terms for one identity. The architecture resolves it: the policy set is "identified by its version hash" (V13.2 Section 7.2). | Use "policy version hash" in RSD-EVD-01c, Table 4.26 and Section 4.10. |
| F11 | Ambiguous | RSD-HOT-02f: "Every RESUME record shall carry a precondition from the closed set of predicates over the evidence and a validity, by default 120 s." Table 4.26, Operator command: "...type, target, parameters...". | The predicate set is defined in no source (RSD, V13.3 Section 11.10, HOTL). The content of the support request that a SUPPORT command creates ("Creates a support request on the operator's behalf with the highest importance class", V13.3 Table 11.12) is not defined either. PRIORITY's parameter and the basis are defined in V13.3 and are no longer part of this finding. | Define the RESUME predicate set and the SUPPORT request content. |
| F13 | Wrong (omission) | Table 4.26 title: "Records and formats relied on by the requirements." | Formats relied on by requirements are missing from Table 4.26, although defined elsewhere: the proof (V13.2 Table C.4; RSD-SEC-01d, RSD-ORG-03d), the GCS receipt (fields in RSD-HOT-05f), the hop acknowledgment (V13.2 Table C.3; RSD-NET-01d). | Add these rows to Table 4.26. |
| F14 | Outdated | Table 4.30: "Handshakes \| 19 per 600 s \| Hop-session design". | With quotes, each handshake adds one quote signature and one quote verification per side: 19 × (0.96 + 0.51) ms per 600 s ≈ 0.05 ms per second (calculation). | "each with one transcript signature, one quote signature and two verifications per side". |
| F15 | Outdated | Table 4.29: "Group Status Summary \| Three components; about 1 kB every 30 s; one copy per reachable designated gateway, at most four, each over at most 10 hops \| ≤ 0.4 % (worst-case estimate)". RSD-HOT-05c: "Each record for headquarters shall be sealed once to the GCS service key, so that a gateway can neither read nor alter it." RSD-HOT-05d: "...its signature verifies under a vehicle certificate of the mission's roster, its record chain, Merkle root and canonical encoding are intact..." | The copy that travels to headquarters is the sealed signed bundle (RSD-HOT-05c, RSD-HOT-05d, HL-12), not a 1 kB summary: at least the bundle header (69 B), the ML-DSA-87 signature (4 627 B), the Merkle path (145 B at depth 3, assumed), the summary record (1 090 B, group B) and the ML-KEM ciphertext (1 568 B) with the GCM tag (16 B): 7 515 B. By the table's own method (1.99 ms per 1 400 B, per byte; 3 components × 4 copies × 10 hops every 30 s) this is about 4.27 %; the same method gives about 0.57 % for 1 kB, so "≤ 0.4 %" does not follow from the stated assumptions. The same 1 kB assumption appears in HL-21 ("about 1 kB every 30 s inside the producer's bundle"). The supervision share is also an open E14 threshold (TBD-02). Calculation, not measurement. | "Three components; a sealed copy of about 7.5 kB (bundle header, signature, Merkle path, summary record, ML-KEM ciphertext) every 30 s; one copy per reachable designated gateway, at most four, each over at most 10 hops \| about 4.3 % (worst-case calculation)"; recompute the total; correct HL-21 in the same way. |
| F18 | Ambiguous | RSD-EVD-01a: "...one bundle that carries those records, with the header, the records and their Merkle root under one signature." | Read literally, the signature covers the records' bytes; the architecture's proof (V13.2 Table C.4) carries only "the two conflicting bundle headers with signatures and the Merkle paths", which works only if the signature covers the header and the Merkle root. Signing the root covers the records cryptographically. | "...with the header and the Merkle root of its records under one signature." |
| F19 | Ambiguous | RSD-CRY-03h: "When two epoch announcements share a parent epoch, every agent shall adopt the announcement of the producer with the smaller rank number and keep the other key for decryption only, for two windows." | Two announcements of the same producer from the same parent are not ordered by this rule and are not an equivocation under the semantic key (producer, epoch_id) of V13.2 Table 8.2 (Section 3, E1 + E3). | Add to RSD-CRY-03h: "...; between two announcements of the same producer, the one with the smaller epoch number." |

### Removed from the findings

| No | Reason |
|---|---|
| F7 | Not an RSD error: comparing evidence and state hashes needs no window (RSD-EVD-03a). The digest carries a 4 B window because the architecture gives it for replay protection (V13.2 Table 7.4; Section 4.3). |
| F10 | Defined in the architecture: V13.2 Section 8.2 and Tables 8.1 and 8.2 (Section 4.1). |
| F12 | Units and list bounds are encoding decisions (Section 4.13, S3, S17); the alert types are defined in V13.3 Section 11.10. |
| F16 | Encoding decision: HELLO's node id and sequence number are carried by the header's sender and counter (Section 4.5). |
| F17 | Defined in the architecture: V13.2 Section 7.3 and V13.3 C07 (Section 4.6). |

## 9. Deliberate departures from the architecture

| No | Architecture | This proposal | Reason |
|---|---|---|---|
| D1 | V13.2 Table 8.1 lists "rid" in the common header. | Not transmitted; computed by K13. | It is a function of the other fields; carrying it adds a mismatch case (F8). |
| D2 | V13.2 Table 8.1 lists "type, ukey" as header fields. | ukey not transmitted; derived from type and payload. | Every ukey of Table 8.2 is made of payload fields; no repetition. |
| D3 | V13.2 Table C.4: Handshake "... suite id". | Carried by the overlay header's version-and-suite byte. | No repetition; the transcript signature covers the header. |
| D5 | V13.3 Section 11.10: summary lists "open repairs and contracts"; summary "routed as C2 traffic to the command ship". | Left out; copies to each reachable designated gateway. | Not in RSD Table 4.26 (nor in V13.3 Table C.5); RSD-HOT-05a and HL-12 govern routing. |
| D6 | V13.3 C07 Table 7.5: "Build identifier per group (A, B, C)". | BuildId E, I, R. | RSD-CMC-05. |
| D7 | V13.3 C10: "RESUME is a HOLD record with duration zero." | RESUME has its own type code and no duration field. | RSD-HOT-02a: five types, RESUME among them; RSD-HOT-02e requires a duration only on HOLD. |
| D8 | V13.2 Section 7.2: identities travel as 8-byte certificate identifiers. | Sets of roster members (recipient list, nodes heard directly, origin set) are roster bitmaps. | Fixed size and one encoding (K10); the signed roster fixes positions (AS3). Single identities stay 8-byte. |
| D9 | V13.2 Table C.1: observation position "lat/lon". | Local ENU, i32 cm, for every format. | Integer kinematics without trigonometry (RSD-EVD-03b). Observation records are not in this issue; the choice is recorded here so that they follow it. |
| D11 | V13.2 Table 8.2: epoch ukey (producer, epoch_id). | Kept (E1), with the deterministic adoption rule E3 added. | Section 3, S15; the rule is an addition, not a change of the key. |

Architecture-internal inconsistency noted for the IV-rule issue (RSD-CRY-01g): V13.2 Section 7.3 builds swarm-key IVs from "the 4-byte sender identifier", while the sender identifier on the air is 8 bytes (Table C.3). The mapping is not defined; it does not affect the formats here.

