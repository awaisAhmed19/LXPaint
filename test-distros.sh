#!/usr/bin/env bash

set -euo pipefail

DISTROS=(
    arch
    ubuntu24
    ubuntu22
    debian-bookworm
    debian-trixie
    fedora
    opensuse
    alpine
    void
)

GREEN='\033[0;32m'
RED='\033[0;31m'
RESET='\033[0m'

PASS=()
FAIL=()

for distro in "${DISTROS[@]}"
do
    echo
    echo "======================================"
    echo "Testing ${distro}"
    echo "======================================"

    if docker compose run --rm "$distro" bash -c "
        chmod +x install.sh build.sh dev-install.sh 2>/dev/null || true
        ./install.sh --yes
    "
    then
        PASS+=("$distro")
    else
        FAIL+=("$distro")
    fi
done

echo
echo "========== SUMMARY =========="

for d in "${PASS[@]}"
do
    printf "${GREEN}✓ %-20s PASS${RESET}\n" "$d"
done

for d in "${FAIL[@]}"
do
    printf "${RED}✗ %-20s FAIL${RESET}\n" "$d"
done

echo
echo "${#PASS[@]} passed"
echo "${#FAIL[@]} failed"
