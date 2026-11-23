#!/usr/bin/env bash
# Copyright (c) 2026 TULPAR project team
# SPDX-License-Identifier: GPL-2.0-only
#
# Gate G0 checks (RSD v1.1 Section 5.4):
#   1. static checks and their self-test          RSD-SIM-01a, RSD-SIM-02a, MD5, MD9, MD19
#   2. ns-3 test suite tulpar-g0 (in one process)  RSD-SIM-01c, RSD-SIM-01d, RSD-SIM-02b
#   3. same seed in two processes                  RSD-SIM-01d, gate I-07
#   4. same seed on a loaded and an idle host      RSD-SIM-01a
#   5. hidden attacker label in two processes      RSD-SIM-02b (hidden-label part)
# Writes logs, hashes and a summary to out/g0/. Exit status 0 only if all pass.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
NS3="$ROOT/external/ns-3"
OUT="${OUT:-$ROOT/out/g0}"
SEED="${SEED:-11}"
RUN="${RUN:-1}"
DURATION="${DURATION:-60}"
mkdir -p "$OUT"
SUMMARY="$OUT/summary.txt"
: > "$SUMMARY"
status=0

record() { echo "$1" | tee -a "$SUMMARY"; }

record "== static checks"
if python3 "$ROOT/tools/check_static.py" --root "$ROOT" | tee "$OUT/check_static.txt" &&
   python3 "$ROOT/tools/test_check_static.py" | tee "$OUT/test_check_static.txt"; then
    record "PASS static checks and self-test"
else
    record "FAIL static checks"; status=1
fi

cd "$NS3"
record "== ns-3 test suite tulpar-g0"
if ./test.py --no-build -s tulpar-g0 > "$OUT/test_suite.txt" 2>&1 &&
   grep -q "1 of 1 tests passed (1 passed, 0 skipped, 0 failed, 0 crashed" "$OUT/test_suite.txt" &&
   ./ns3 run --no-build "test-runner --test-name=tulpar-g0 --verbose" > "$OUT/test_cases.txt" 2>&1; then
    record "PASS test suite tulpar-g0 ($(grep -c '^  PASS' "$OUT/test_cases.txt" || true) of 3 test cases passed)"
else
    record "FAIL test suite tulpar-g0"; status=1
fi

run_g0() {  # name, extra args...
    local name="$1"; shift
    ./ns3 run --no-build "tulpar-g0 --seed=$SEED --run=$RUN --duration=$DURATION \
        --eventLog=$OUT/$name.events.log --groundTruthLog=$OUT/$name.truth.log $*" \
        > "$OUT/$name.stdout.txt"
    sha256sum "$OUT/$name.events.log" | cut -d' ' -f1 > "$OUT/$name.events.sha256"
}

record "== same seed, two processes (RSD-SIM-01d, I-07)"
run_g0 same-a
run_g0 same-b
if cmp -s "$OUT/same-a.events.log" "$OUT/same-b.events.log" && [ -s "$OUT/same-a.events.log" ]; then
    record "PASS identical event logs, sha256 $(cat "$OUT/same-a.events.sha256")"
else
    record "FAIL event logs differ"; status=1
fi

record "== loaded host versus idle host (RSD-SIM-01a)"
burners=()
trap 'kill "${burners[@]}" 2>/dev/null || true' EXIT
for _ in $(seq "$(nproc)"); do
    python3 -c "while True: pass" & burners+=("$!")
done
run_g0 loaded
kill "${burners[@]}" 2>/dev/null || true
burners=()
if cmp -s "$OUT/same-a.events.log" "$OUT/loaded.events.log"; then
    record "PASS loaded-host log identical to idle-host log"
else
    record "FAIL loaded-host log differs"; status=1
fi

record "== hidden attacker label (RSD-SIM-02b, hidden-label part)"
run_g0 label-none --attacker=none
run_g0 label-uav1 --attacker=UAV-1
if cmp -s "$OUT/label-none.events.log" "$OUT/label-uav1.events.log" &&
   ! cmp -s "$OUT/label-none.truth.log" "$OUT/label-uav1.truth.log"; then
    record "PASS agent logs identical, ground-truth logs differ"
else
    record "FAIL hidden label changed agent events or did not reach the ground truth"; status=1
fi

record "== run details"
record "ns-3 $(cat "$NS3/VERSION"), $(g++ --version | head -1), seed $SEED, run $RUN, duration ${DURATION} s"
record "repository commit $(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo 'none')"
record "$(cat "$OUT/same-a.stdout.txt")"
[ "$status" -eq 0 ] && record "G0 CHECKS PASSED" || record "G0 CHECKS FAILED"
exit "$status"
