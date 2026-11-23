#!/usr/bin/env bash
# Copyright (c) 2026 TULPAR project team
# SPDX-License-Identifier: GPL-2.0-only
#
# Fetch ns-3.49, link the TULPAR module into contrib/ and configure the build.
# Usage: scripts/bootstrap.sh
set -euo pipefail

NS3_TAG="ns-3.49"
NS3_URL="https://github.com/nsnam/ns-3-dev-git.git"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
NS3="$ROOT/external/ns-3"

if [ ! -d "$NS3/.git" ] && [ ! -f "$NS3/VERSION" ]; then
    git clone --depth 1 --branch "$NS3_TAG" "$NS3_URL" "$NS3"
fi
test "$(cat "$NS3/VERSION")" = "3.49" || { echo "expected ns-3.49 in $NS3" >&2; exit 1; }

ln -sfn "$ROOT/module" "$NS3/contrib/tulpar"

cd "$NS3"
./ns3 configure -G Ninja --build-profile=default --enable-examples --enable-tests \
    --enable-modules="tulpar"
