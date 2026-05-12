#!/bin/bash
# Copy engine binaries baked into the image (/opt/eqemu/bin) into the
# host-mounted /home/eqemu/server/bin so a `docker compose pull && up -d`
# rolls forward to whatever this image was built with.
#
# Skip silently if either side is missing — keeps the wrapper safe on
# first-run boot before /home/eqemu/server is initialized.

set -e

SRC=/opt/eqemu/bin
DST=/home/eqemu/server/bin
SHA_FILE=/opt/eqemu/build-sha

if [ ! -d "$SRC" ]; then
    echo "# sync-binaries: $SRC not present — image has no baked binaries, skipping."
    exit 0
fi

if [ ! -d /home/eqemu/server ]; then
    echo "# sync-binaries: /home/eqemu/server not initialized yet — skipping (run installer first)."
    exit 0
fi

mkdir -p "$DST"

if [ -f "$SHA_FILE" ]; then
    echo "# sync-binaries: syncing image binaries ($(cat "$SHA_FILE")) -> $DST"
else
    echo "# sync-binaries: syncing image binaries -> $DST"
fi

cp -af "$SRC"/. "$DST"/
