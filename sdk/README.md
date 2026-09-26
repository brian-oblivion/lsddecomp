# sdk/

Drop your Psy-Q SDK disc image(s) here, as downloaded. Do not unpack them.

The game links Sony's Psy-Q runtime libraries, and this project links the very
same objects out of the SDK rather than re-deriving them as C. The objects are
Sony's bytes, so — like the game executable in `disk/` — they are never
committed. `tools/setup.sh` converts what it finds here into `lib/`, and
`config/psyq-objects.txt` records which object from which disc goes where.

## Which discs

The "Programmer Tool — Runtime Library" discs, redump format, from
<https://archive.org/download/ps1_sdks>:

    Programmer Tool - Runtime Library Version 3.3 (Japan)_DTL-S2190_redump.zip
    Programmer Tool - Runtime Library Version 3.5 (Japan) (En,Ja)_DTL-S2300_redump.zip
    Programmer Tool - Runtime Library Version 3.6 (Japan)_DTL-S2310_redump.zip

These are the versions `config/psyq-objects.txt` names today (its first
column); most objects come from 3.3.

`python3 tools/psyq_sdk.py install` tells you exactly which version(s) the
manifest currently needs if one is missing. The version is read from the file
name, so keep the original name (a `.bin` or `.iso` of the data track with the
same name also works).

## Why more than one version

The executable's embedded library ids date its `libgpu`/`libcd` to December
1995 while its `libetc` matches the 3.5 disc exactly. Games mixed library
builds, so an object is taken from whichever disc holds the build the game
linked — measured by `tools/psyq_sdk.py match`, never assumed.

## Layout after setup

    sdk/*.zip           your discs
    sdk/work/<ver>/     extracted track, PSX/LIB + PSX/INCLUDE, .OBJ, .o, match.txt
    lib/<lib>/<mod>.o   the objects the build links (from config/psyq-objects.txt)
