#!/usr/bin/env bash
set -euo pipefail

cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++-16
cmake --build build-debug
ctest --test-dir build-debug --output-on-failure
