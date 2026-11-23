# Shared development image of TULPAR (RSD v1.1 Section 2.3: Ubuntu 24.04, GCC 13).
#
# Build:  docker build -t tulpar-dev docker/
# Use:    docker run --rm -it -v "$PWD":/work -w /work tulpar-dev
#         scripts/bootstrap.sh && scripts/build.sh && scripts/g0_checks.sh
#
# Botan 3.13.0 (RSD-CRY-01e), SIMDIS SDK 1.25 and UPPAAL 5.0.0 are added in the
# phases that first need them.
FROM ubuntu:24.04

RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      build-essential g++-13 cmake ninja-build git python3 ca-certificates \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /work
