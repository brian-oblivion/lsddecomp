# StampSaveTitleFileLetter — MATCHED (round 45, 60/60 words)

> Renamed from `CopyMemcardIconTemplate` on 2026-09-27 (tools/rename.py). Address 0x800507f8.

> Renamed from `Class86E00_3bb8c_g__CopyMemcardIconTemplate` on 2026-09-23 (tools/rename.py). Address 0x800507f8.

> Renamed from `func_800507F8` on 2026-09-23 (tools/rename.py). Address 0x800507f8.

**Unit:** class_3bb8c_g · **Size:** 60 words (0xF0 bytes)

Filed as a `gp_rel`-blocked stub in round 14. That blocker was RESOLVED in
round 42 (`--gp-symbols`, pinned in the Makefile).

## Derivation

```c
extern s32 atoi(char *s);
extern u8 *gSaveTitleGlyphs;

typedef struct {
    s8 raw[6];
} FullWidthChars3;
typedef struct {
    s8 raw[12];
} FullWidthChars6;
typedef struct {
    s8 a, b;
} FullWidthChar;

s32 StampSaveTitleFileLetter(s32 arg0, s32 arg1)
{
    u8 *self = (u8 *)arg0;
    u8 *src = (u8 *)arg1;
    s32 t0;
    s32 idx;
    u8 *p;

    if (src != NULL) {
        t0 = ((u32)(src[0xE] - 0x38) < 2) ? 0xE : 0xD;

        *(FullWidthChar *)(self + 0x18) = *(FullWidthChar *)(gSaveTitleGlyphs + 0x1E);
        *(FullWidthChars6 *)(self + 0x6) = *(FullWidthChars6 *)(gSaveTitleGlyphs + 0x1E);

        idx = atoi((char *)(src + t0)) - 1;
        p = gSaveTitleGlyphs + idx * 2;
        *(FullWidthChar *)(self + 0x8) = *(FullWidthChar *)p;
        return (s32)p;
    } else {
        u8 *q = gSaveTitleGlyphs;

        *(FullWidthChars3 *)(self + 0x6) = *(FullWidthChars3 *)(q + 0x1E);
        return (s32)q;
    }
}
```

**Signature was NOT free to choose.** `include/class_3bb8c.h` already
carries `extern s32 StampSaveTitleFileLetter(s32 arg0, s32 arg1);` (`class_3bb8c_m`'s
own caller, `TaskObjF__WriteMemcardSaveFile`), visible in this same translation unit via the
shared header, so the definition here has to match it exactly
(`conflicting types` otherwise) even though every real use inside the body
is pointer arithmetic. Cast `arg0`/`arg1` to `u8 *` locally instead.

**Three struct-copy shapes, all instances of this round's alignment lever**
(`FormatNumberIntoBuffer`'s report): declare the copied range ALL-`s8` so its
alignment is 1, which is what makes GCC use the unaligned `lwl`/`lwr` word
chunk(s) retail has, with any leftover non-multiple-of-4 bytes as
individual loads/stores rather than a merged halfword.
- `FullWidthChar` (2 bytes) — used twice: copying the raw 2-byte prefix
  `gSaveTitleGlyphs[0x1E..0x20)` into `self[0x18..0x1A)`, and copying a 2-byte
  entry out of a `gSaveTitleGlyphs`-relative lookup table (indexed by
  `atoi(...)  - 1`, doubled) into `self[0x8..0xA)`.
- `FullWidthChars6` (12 bytes, exactly 3 word chunks, no tail) — the `src !=
  NULL` path's bulk copy `self[0x6..0x12) = gSaveTitleGlyphs[0x1E..0x2A)`.
- `FullWidthChars3` (6 bytes, one word chunk + 2 tail bytes) — the `src ==
  NULL` path's shorter copy `self[0x6..0xC) = gSaveTitleGlyphs[0x1E..0x24)`,
  identical shape to `FormatNumberIntoBuffer`'s own struct this round.

**The one register-identity trap, closed on the third attempt:** the
function's return value is a POINTER into the `gSaveTitleGlyphs` template
(confirmed from retail's own register content at `jr $ra` — whichever
branch runs, `$v0` still holds a `gSaveTitleGlyphs`-derived pointer, never
reloaded fresh at the very end). Writing `return (s32)gSaveTitleGlyphs;` as a
fresh expression in the `else` branch cost one extra word: the compiler
reloads the global via a second `%gp_rel` `lw` rather than reusing the
value already sitting in a register from the struct-copy statement just
above it. The fix was a local pointer variable holding the SAME value,
reused for both the copy and the return — but that variable had to be
scoped to the `else` block alone (`u8 *q = gSaveTitleGlyphs;` declared at the top
of that block, not the function's own top-level locals): sharing ONE
function-wide local across both branches for two semantically different
pointers (the template base in one branch, an indexed lookup pointer in
the other) forced the compiler to keep a consistent register for it across
the WHOLE function, which shuffled every other register assignment and cost
far more than it saved (36/60 instead of 59/60 on that attempt).

### Proposed learning

When retail's exit-time register already holds the value you need to
return, don't write a fresh global-read expression for the `return`
statement — reuse whatever local already holds it (computed earlier in the
SAME block) so the compiler doesn't emit a second reload. But scope that
reused local NARROWLY: a function-wide local shared across an if/else for
two DIFFERENT pointer values (even if both ultimately return through the
same statement shape) pins one register for the variable's entire lifetime
across the whole function and can cost far more in register-allocation
ripple than the reload it was meant to save. Prefer a block-scoped local
declared at the top of just the branch that needs it.

## Naming

`StampSaveTitleFileLetter` (was `func_800507F8`), tier B:
copies one or more fixed byte ranges out of the `gSaveTitleGlyphs` template into
the caller's buffer, optionally selecting a table entry via
`atoi()` on a field of the caller-supplied `src` when one is given. Named
from its one real caller context established elsewhere in this project
(`TaskObjF__WriteMemcardSaveFile`, `class_3bb8c_m`, and `class_3bb8c_d`'s
own call site) -- a memcard save-file writer building the save's on-card
icon/header block from a shared template. The exact semantic meaning of
each copied range (icon pixels vs. a formatted date/glyph, per the
sibling `D_8008AA18`/`DecodeFullWidthSjis` context nearby in
`class_3bb8c_d.c`) is not established, so this is named for the
mechanism (template copy) plus its one known call context rather than a
specific claim about pixel vs. text content.

## Track 7 (2026-09-27, round 95)

Renamed from `CopyMemcardIconTemplate` (tools/rename.py), and its data
`gMemcardIconTemplate` -> `gSaveTitleGlyphs`. Nothing in the body is an
icon: measured from the executable, the pointer at 0x8008AAC4 points at
0x80011550, the full-width SJIS string "a".."o" (15 letters, 2 bytes each)
followed at glyph 15 by three full-width spaces and "Day". TitleMenu's
save title (D_8008AA18 -> 0x8001149C) starts as full-width "LSD   Day001",
and the file names are namePrefix "BISLPS-01556" + "-01".."-15"
(D_80086D6C). So the body writes characters 3..8 as "   Day", character
12 as a space, and character 4 as the letter for the file's number; with a
NULL file name it blanks characters 3..5. The `'8'` test exists because
Sony's atoi (libc2/atoi.o, read from lib/) takes a leading 0 as octal, so
"08"/"09" are parsed from their second digit.

**Naming**, tier B: `StampSaveTitleFileLetter` (evidence above; the
mechanics are certain, that the letter identifies the file on the card
screen is not shown by a consumer). `gSaveTitleGlyphs`, tier A, by what it
holds.

Locals: `arg0`/`arg1` -> `titleAddr`/`fileNameAddr`, `self` -> `title`
(now `FullWidthChar *`, one full-width character per element, so the raw
byte offsets 0x18/0x6/0x8 are indices 12/3/4), `src` -> `fileName`
(`char *`), `t0` -> `numberPos`, `idx` -> `letter`, `p` -> `glyph`, `q` ->
`glyphs`; `gSaveTitleGlyphs` declared `FullWidthChar *` (offset 0x1E is
glyph 15). The positions are unit-local `#define`s: SAVE_TITLE_LETTER_FIELD
3, SAVE_TITLE_LETTER 4, SAVE_TITLE_PADDING 12, SAVE_TITLE_GLYPH_SPACES 15,
SAVE_FILE_NAME_NUMBER 13. Image byte-identical at every step.

The signature stays `s32 (s32, s32)`: it is include/class_3bb8c.h's
prototype, which class_3bb8c_d.c and class_3bb8c_f.c call with casts.
Proposed: `s32 StampSaveTitleFileLetter(char *title, char *fileName)` there,
dropping the four casts.

Comments moved out of the source (verbatim):

- on the local `atoi` extern: "Sony's, from libc2 (round 45's own local
  view -- this unit's first use)."
- on `gSaveTitleGlyphs`: "VALUE-of `%gp_rel`, round 45's own local view --
  a fixed rodata template (ROM image still-uncarved,
  `asm/data/1C34.rodata.s` region) this function copies raw byte ranges out
  of; also read by `class_3bb8c_d.c`'s own (differently-typed) local view."
  (Round 95: no other unit declares it now.)
- on the three copy types: "Struct-copy helper types for round 45's
  StampSaveTitleFileLetter, all deliberately all-`s8` (alignment 1) per
  this round's FormatNumberIntoBuffer lever: retail copies these ranges as
  one unaligned `lwl`/`lwr` word chunk per 4 bytes, with any
  non-multiple-of-4 remainder as INDIVIDUAL byte loads/stores, never merged
  into a halfword -- alignment 2 would let GCC trust a halfword move retail
  does not have." It keeps one line in the source (`MATCHING: all-s8 ...`).
- above the definition: "Signature is `include/class_3bb8c.h`'s
  ALREADY-shared `extern s32 StampSaveTitleFileLetter(s32 arg0, s32
  arg1);` (class_3bb8c_m's own caller, TaskObjF__WriteMemcardSaveFile),
  matched exactly -- this unit's own definition must agree with that
  declaration since both are visible in this translation unit. Cast to `u8
  *` internally; retail's own register content at exit (`$v0` left holding
  a pointer into the `gSaveTitleGlyphs` template in every path) confirms
  the real return type is a pointer, loosely read as `s32` by the caller
  that never dereferences it." The block-scoped `glyphs` that residue
  needed keeps one line (`MATCHING: glyphs is the return value`).

The `## Naming` section above (tier B, "template copy") is superseded by
this one.

## Track 6 (2026-09-27, round 96)

The three copy types, named for what they hold (tools/renametype.py, image
byte-identical at every step):

- `Pair2_3bb8c_g` -> `FullWidthChar`, tier A: every `gSaveTitleGlyphs`
  element and every title position is one 2-byte full-width Shift-JIS
  character (the data measured in round 95, above). Fields `a`, `b` ->
  `lead`, `trail` (the SJIS lead and trail bytes); no code accesses either,
  every use is a whole-struct copy.
- `Buf6_3bb8c_g` -> `FullWidthChars3`, tier A: three full-width characters,
  the title's letter field (characters 3..5) blanked from three spaces.
  Now `FullWidthChar chars[3]` in place of `s8 raw[6]`.
- `Buf12_3bb8c_g` -> `FullWidthChars6`, tier A: six full-width characters,
  "   Day" into characters 3..8. Now `FullWidthChar chars[6]` in place of
  `s8 raw[12]`.

Alignment stays 1 (all-`s8` leaves), which is the whole reason these are
structs: the source keeps its one `MATCHING:` line. No existing type in
`include/` or `src/` named a full-width character (grepped for
Sjis/FullWidth/Glyph), so these are new and stay unit-local.

`class_3bb8c_c`'s `Buf6_3bb8c_c` (`s8 a..f`, the formatted day number
copied into the same title at +0x12, characters 9..11) is the same record
as `FullWidthChars3`: a later job, left untouched here. If a second unit
takes these types they move to the header that owns the save title
(proposed: `include/TitleMenu.h`, which describes the SJIS title buffer).
