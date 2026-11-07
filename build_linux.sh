#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release -j"$(nproc 2>/dev/null || echo 4)"

echo ""
echo "Build complete! Run: ./build/V-Mawaqeet"
