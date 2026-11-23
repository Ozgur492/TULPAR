#!/usr/bin/env bash
# Copyright (c) 2026 TULPAR project team
# SPDX-License-Identifier: GPL-2.0-only
#
# Build the TULPAR library, the G0 example and the ns-3 test runner.
# Usage: scripts/build.sh [extra ninja targets]
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT/external/ns-3"
./ns3 build tulpar tulpar-g0 test-runner "$@"
