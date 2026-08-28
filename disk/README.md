# disk/

Drop the game executable here as `disk/SLPS_015.56`.

Bring your own copy. It is the ground truth this project verifies against and
is never committed.

## Getting it out of a disc image

`tools/extract_exe.py` reads the ISO 9660 filesystem out of a raw CD image
(`.bin` with 2352-byte sectors, or a 2048-byte-per-sector `.iso`) and writes
the executable here:

```sh
python3 tools/extract_exe.py "LSD - Dream Emulator (Japan).bin"
```

`tools/setup.sh` runs it for you if it finds exactly one image in the repo.

## Expected

    SLPS_015.56   505856 bytes   SHA1 76322eeade5ebb22dca57fdeac7d68c30f06308d

That is the Japanese retail release, SLPS-01556. No other revision is
supported; a different hash means a different build and every address in
`config/` is wrong for it.
