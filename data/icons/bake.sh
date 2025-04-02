#!/bin/bash

cd "$(dirname "$0")"
rm -rf baked
mkdir -p baked

# 1. copy pre-made icons
cp reference/16x16.png baked/
cp reference/24x24.png baked/
cp reference/32x32.png baked/
cp reference/scalable.svg baked/

# 2. generate remaining resolutions
function rescale {
    DIMENSION="$1"
    inkscape \
        --export-filename="baked/${DIMENSION}x${DIMENSION}.png" \
        --export-width=$DIMENSION \
        --export-height=$DIMENSION \
        reference/scalable.svg
    return #?
}
rescale 48 || exit 1
rescale 64 || exit 1
rescale 128 || exit 1
rescale 256 || exit 1
rescale 512 || exit 1

# 3. create ICNS bundle
png2icns \
    baked/zero-elabviewer.icns \
    baked/16x16.png \
    baked/32x32.png \
    baked/48x48.png \
    baked/128x128.png \
    baked/256x256.png \
    baked/512x512.png \
    || exit 1

# 4. create ICO bundle
icotool \
    --create \
    --output=baked/zero-elabviewer.ico \
    baked/16x16.png \
    baked/24x24.png \
    baked/32x32.png \
    baked/48x48.png \
    baked/256x256.png \
    || exit 1
