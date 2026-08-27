#!/bin/bash

ROOT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
DIST_DIR="$ROOT_DIR/dist"
CORES="$(nproc --all)"
echo "CORES: $CORES"
echo

echo "*** building sysmodule ***"
echo
cd sysmodule
make -j$CORES
cd "$ROOT_DIR"

echo "*** building overlay ***"
echo
cd overlay
make -j$CORES
cd "$ROOT_DIR"
