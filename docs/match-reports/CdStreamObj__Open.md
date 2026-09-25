# CdStreamObj__Open -- MATCHED (exact length, 75/75 words), round 82

> Renamed from `func_80047114` on 2026-09-25 (tools/rename.py). Address 0x80047114.

Round 82, runner delta (second session). Unit `src/code_3770c.c`. Fresh
ground, no prior attempt. Byte-exact on build 4; whole-image SHA1 green.

- **Where:** slot +0x044 of gCdStreamObjMethods (open a stream file by name).
- **What:** only when idle (state 0): fail (return 1) with no ring set;
  return 0 if another stream is active. Otherwise build
  `"\" + func_800270B8() + name + ";1"` in a 0x20-byte stack buffer and retry
  `CdSearchFile(&self->loc, path)` (a negative `tries` retries forever;
  timeout returns 1). On success: +0x40 = file size (`CdlFILE.size`, +0x10)
  / +0x38; `D_8008A94C = SetupCdStreamAudio(self)` (SPU CD-volume setup);
  `gActiveCdStreamObj = self`; `seek(self, &self->loc)`; return 0.
- **Levers:** the return structure decides the block layout:

  | form | score |
  | --- | --- |
  | flat early returns (`if (unk2C != 0) return 1; ...`), loop then found code | 75 words, 20/75 (found block placed ahead of the path build, s-regs renumbered) |
  | same with `for (;;) { if (found) break; ... }` | identical, 20/75 |
  | flat, with everything after the checks nested in `if (gActiveCdStreamObj == NULL) {...} return 0;` | 2 words short, 52/75 (the loop's `return 1` cross-jumped into the unk2C one) |
  | **`if (self->unk2C == 0) { ...all of it...; return 0; } return 1;`** | **75/75** |

  The copy `n = tries` is the "retry forever" flag tested by `bltz`, and
  `tries` itself is what gets decremented (retail keeps the param in s0 and
  the copy in s3).
- **Context:** `self->loc` (+0x0C..0x24) is a `CdlFILE`. Its size is read
  here as `*(u32 *)&self->loc[4]`, left untyped in the shared local view
  because the ten earlier methods pass `loc` as a `u8 *`. Unit-local
  externs: `CdSearchFile`, `strcpy`/`strcat` (Sony libc2, linked),
  `func_800270B8` (code_171e0.c), `D_8008A94C` (s32, sdata), `D_8008A954`
  (`char[]`, the rodata-style `";1"` in sdata, referenced as a symbol and
  never retyped). Added a prototype for `SetupCdStreamAudio`. Unit header comment
  updated: all 18 methods matched.

## Naming

Tier A. `CdStreamObj__Open` -- slot +0x044. Evidence: searches the disc for the named file (`CdSearchFile`), computes the frame count from its size, activates the object as the single active stream (`gActiveCdStreamObj`), and seeks to it -- the standard "open a file for streaming" sequence.

## Source

```c
s32 CdStreamObj__Open(CdStreamObj *self, char *name, s32 tries) {
    char path[0x20];
    s32 n;

    n = tries;
    if (self->unk2C == 0) {
        if (self->ring == NULL) {
            return 1;
        }
        if (gActiveCdStreamObj != NULL) {
            return 0;
        }
        path[0] = '\\';
        strcpy(&path[1], func_800270B8());
        strcat(path, name);
        strcat(path, D_8008A954);
        while (CdSearchFile(self->loc, path) == 0) {
            if (n >= 0 && --tries < 0) {
                return 1;
            }
        }
        self->unk40 = *(u32 *)&self->loc[4] / self->unk38;
        D_8008A94C = SetupCdStreamAudio(self);
        gActiveCdStreamObj = self;
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
