#!/usr/bin/env bash

if [ "$#" -ne 2 ]; then
    echo "Incorrect number of arguments"
    echo "Usage: $0 <cmake_build_type> <number_of_processes>"
    exit 1
fi

BUILD_TYPE=$(echo "$1" | tr '[:upper:]' '[:lower:]')
PROCESSES=$2

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)/.."
BIN_PATH="$ROOT/cmake-build-$BUILD_TYPE/3-heat/src/3-heat"

TMPDIR=/tmp mpirun --oversubscribe -np "$PROCESSES" "$BIN_PATH"
