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

echo
echo "*** bundling dist ***"
echo

TITLE_ID="$(grep -oP '"title_id":\s*"0x\K(\w+)' "$ROOT_DIR/sysmodule/sysmodule.json")"
echo "TITLE_ID: $TITLE_ID"

rm -rf "$DIST_DIR"

mkdir -p "$DIST_DIR/atmosphere/contents/$TITLE_ID/flags"
cp -vf "$ROOT_DIR/sysmodule/sysmodule.nsp" "$DIST_DIR/atmosphere/contents/$TITLE_ID/exefs.nsp"
cp -vf "$ROOT_DIR/sysmodule/toolbox.json" "$DIST_DIR/atmosphere/contents/$TITLE_ID/toolbox.json"
> "$DIST_DIR/atmosphere/contents/$TITLE_ID/flags/boot2.flag"

mkdir -p "$DIST_DIR/switch/.overlays"
cp -vf "$ROOT_DIR/overlay/NX-FanControl.ovl" "$DIST_DIR/switch/.overlays/NX-FanControl.ovl"

echo
echo "*** packaging NX-FanControl.zip ***"
echo

cd "$DIST_DIR"
rm -f NX-FanControl.zip
zip -r NX-FanControl.zip . >/dev/null
cd "$ROOT_DIR"

echo "*** dist ready: $DIST_DIR ***"
