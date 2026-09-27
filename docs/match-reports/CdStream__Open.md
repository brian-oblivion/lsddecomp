# CdStream__Open -- MATCHED (exact length, 75/75 words), round 82

> Renamed from `CdStreamObj__Open` on 2026-09-26 (tools/rename.py). Address 0x80047114.

> Renamed from `func_80047114` on 2026-09-25 (tools/rename.py). Address 0x80047114.

Round 82, runner delta (second session). Unit `src/CdStream.c`. Fresh
ground, no prior attempt. Byte-exact on build 4; whole-image SHA1 green.

- **Where:** slot +0x044 of gCdStreamMethods (open a stream file by name).
- **What:** only when idle (state 0): fail (return 1) with no ring set;
  return 0 if another stream is active. Otherwise build
  `"\" + func_800270B8() + name + ";1"` in a 0x20-byte stack buffer and retry
  `CdSearchFile(&self->loc, path)` (a negative `tries` retries forever;
  timeout returns 1). On success: +0x40 = file size (`CdlFILE.size`, +0x10)
  / +0x38; `gCdStreamAudioMixSet = SetupCdStreamAudio(self)` (SPU CD-volume setup);
  `gActiveCdStream = self`; `seek(self, &self->loc)`; return 0.
- **Levers:** the return structure decides the block layout:

  | form | score |
  | --- | --- |
  | flat early returns (`if (unk2C != 0) return 1; ...`), loop then found code | 75 words, 20/75 (found block placed ahead of the path build, s-regs renumbered) |
  | same with `for (;;) { if (found) break; ... }` | identical, 20/75 |
  | flat, with everything after the checks nested in `if (gActiveCdStream == NULL) {...} return 0;` | 2 words short, 52/75 (the loop's `return 1` cross-jumped into the unk2C one) |
  | **`if (self->unk2C == 0) { ...all of it...; return 0; } return 1;`** | **75/75** |

  The copy `n = tries` is the "retry forever" flag tested by `bltz`, and
  `tries` itself is what gets decremented (retail keeps the param in s0 and
  the copy in s3).
- **Context:** `self->loc` (+0x0C..0x24) is a `CdlFILE`. Its size is read
  here as `*(u32 *)&self->loc[4]`, left untyped in the shared local view
  because the ten earlier methods pass `loc` as a `u8 *`. Unit-local
  externs: `CdSearchFile`, `strcpy`/`strcat` (Sony libc2, linked),
  `func_800270B8` (code_171e0.c), `gCdStreamAudioMixSet` (s32, sdata), `gCdStreamVersionSuffix`
  (`char[]`, the rodata-style `";1"` in sdata, referenced as a symbol and
  never retyped). Added a prototype for `SetupCdStreamAudio`. Unit header comment
  updated: all 18 methods matched.

## Naming

Tier A. `CdStream__Open` -- slot +0x044. Evidence: searches the disc for the named file (`CdSearchFile`), computes the frame count from its size, activates the object as the single active stream (`gActiveCdStream`), and seeks to it -- the standard "open a file for streaming" sequence.

## Source

```c
s32 CdStream__Open(CdStreamObj *self, char *name, s32 tries) {
    char path[0x20];
    s32 n;

    n = tries;
    if (self->unk2C == 0) {
        if (self->ring == NULL) {
            return 1;
        }
        if (gActiveCdStream != NULL) {
            return 0;
        }
        path[0] = '\\';
        strcpy(&path[1], func_800270B8());
        strcat(path, name);
        strcat(path, gCdStreamVersionSuffix);
        while (CdSearchFile(self->loc, path) == 0) {
            if (n >= 0 && --tries < 0) {
                return 1;
            }
        }
        self->unk40 = *(u32 *)&self->loc[4] / self->unk38;
        gCdStreamAudioMixSet = SetupCdStreamAudio(self);
        gActiveCdStream = self;
        self->methods->seek(self, self->loc);
        return 0;
    }
    return 1;
}
```

### Proposed learning

Two `li v0,1` epilogue paths that stay DISTINCT (one `li v0,1` block sitting
right before the epilogue, reached by an early bnez, and a separate
`li v0,1; j end` elsewhere) are a sign of `if (c == 0) { ...; return 0; }
return 1;`. The flat `if (c != 0) return 1;` form lets cc1 cross-jump the
identical returns (words short) or reorders the blocks.

## Track 4 (2026-09-26, round 87)

Class unified as `CdStream` (include/CdStream.h; table gCdStreamObjMethods -> gCdStreamMethods, type CdStreamObj -> CdStream, the Obj suffix dropped per FINISHING-PLAN track 4 step 2). The unit's local view is gone; slots +0x044 open, +0x050 startRead and +0x06C getNextFrame are typed from their occupants, and the object's +0x00C `seekLoc[0x18]` is the CdlFILE `file` (CdStreamFile) that CdSearchFile fills. Zero bytes changed.

Renamed from CdStreamObj__Open (tools/rename.py), the class rename only.

## Naming (track 7, round 99)

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `D_8008A954` | `gCdStreamVersionSuffix` | A | sdata `.asciz ";1"`, strcat'd after the name to make the ISO9660 path CdSearchFile looks up; same role as `gCdFileVersionSuffix` (code_179d8_h.c), a separate copy |
| `D_8008A94C` | `gCdStreamAudioMixSet` | B | written only here, with SetupCdStreamAudio's return (always 1) after the SPU CD mix is set; read nowhere in the executable (only reference in asm/ is its sdata definition) |

Constants: the path buffer is `CDSTREAM_PATH_SIZE` (32, unit-local; the CD
driver's `CD_PATH_SIZE` is 64). The `CdlFILE *` cast on `&self->file` is
because CdStream.h spells Sony's CdlFILE as its own `CdStreamFile`
(same 24-byte layout) to stay free of `<libcd.h>`.
