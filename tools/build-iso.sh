#!/usr/bin/env bash
# Build the full disc image from the user's own extracted disc files with
# the rebuilt executable swapped in.
#
#   ./build-and-verify.sh          # build/SLPS_015.56 exists and matches
#   dumpsxiso <your dump>.bin -x disk/extract -s disk/extract.xml
#   ./tools/build-iso.sh           # build/lsdde-disc.bin + .cue
#
# The result is a complete MODE2/2352 data track: 648,088,896 bytes for the
# Japanese retail dump, byte identical to it, since every file (including
# the rebuilt executable) is byte identical and config/iso.xml carries the
# original timestamps and license data.
set -euo pipefail
cd "$(dirname "$0")/.."

command -v mkpsxiso >/dev/null 2>&1 || { echo "mkpsxiso not found (https://github.com/Lameguy64/mkpsxiso)"; exit 1; }
[ -f build/SLPS_015.56 ] || { echo "build/SLPS_015.56 missing; run ./build-and-verify.sh first"; exit 1; }
[ -d disk/extract/CDI ] || { echo "disk/extract missing; extract your dump with dumpsxiso first"; exit 1; }
[ -f disk/extract/license_data.dat ] || { echo "disk/extract/license_data.dat missing"; exit 1; }

# Staging root with only what the original disc had at its root:
# SYSTEM.CNF and the CDI tree. disk/extract also holds dumpsxiso's own
# outputs and the original executable; those must not enter the image.
mkdir -p build/iso-root
ln -sfn ../../disk/extract/CDI build/iso-root/CDI
ln -sfn ../../disk/extract/SYSTEM.CNF build/iso-root/SYSTEM.CNF

mkpsxiso config/iso.xml -o build/lsdde-disc.bin -c build/lsdde-disc.cue >/dev/null
echo "built build/lsdde-disc.bin ($(stat -c %s build/lsdde-disc.bin) bytes)"
