#!/usr/bin/env bash

PROCESSES=$1

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)/.."
BIN_PATH="$ROOT/cmake-build-debug/3-heat/src/3-heat"

mpirun --oversubscribe -np "$PROCESSES" "$BIN_PATH"
