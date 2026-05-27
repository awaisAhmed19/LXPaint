
#!/bin/bash
set -e

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
 
echo $REPO_ROOT 
mkdir -p "$REPO_ROOT/build"
cd "$REPO_ROOT/build"

cmake ..

CPU_CORES=$(nproc 2>/dev/null || echo 1)

cmake --build . --parallel "$CPU_CORES"

cd "$REPO_ROOT/build"

./lxpaint
