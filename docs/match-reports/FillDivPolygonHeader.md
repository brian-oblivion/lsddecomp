# FillDivPolygonHeader -- MATCHED (round 44, 27/27 words)

> Renamed from `FillRCPolyHeader` on 2026-09-26 (tools/rename.py). Address 0x8001a380.

> Renamed from `func_8001A380` on 2026-09-24 (tools/rename.py). Address 0x8001a380.

Unit `code_8220_b`. Reopened round 42 after the `gp_rel` blocker that stalled
it at carve time (round 13) was resolved (`--gp-symbols`, see
`docs/research/gp-relative-blocker.md`). This report supersedes the round-13
stub, which recorded no attempt and no score.

## Result

```
FillDivPolygonHeader: 27/27 words match (file 0xAB80-0xABEC)
```

Whole-image `./build-and-verify.sh` passes (`build exit=0`).

## Final C

```c
extern s32 sDivClipWidth;
extern s32 sDivClipHeight;
extern s32 D_80090C18;
extern s32 sNdivOverrideSet;
extern s32 sNdivOverride;

void FillDivPolygonHeader(void *arg0, void *arg1, PolyUV4 *arg2, s32 arg3, u16 arg4, u16 arg5)
{
    u8 *dst = (u8 *)arg0;
    u8 *prim = (u8 *)arg1;
    s32 val;
    s32 code;
    s32 code2;

    if (sNdivOverrideSet) {
        val = sNdivOverride;
    } else {
        val = D_80090C18;
    }
    code = sDivClipWidth;
    code2 = sDivClipHeight;

    *(s32 *)dst = val;
    *(s32 *)(dst + 0x4) = code;
    *(s32 *)(dst + 0x8) = code2;

    if (arg3 != 0) {
        *(u16 *)(dst + 0xC) = arg4;
        *(u16 *)(dst + 0xE) = arg5;
    }

    *(PolyUV4 *)(dst + 0x10) = *arg2;
    *(s32 *)(dst + 0x14) = *(s32 *)(prim + 0x30);
}
```

Populates a GPU primitive header at `arg0` (`gDivPolygon3`/`gDivPolygon4`
depending on caller): a selected OT/code word at +0x00, `sDivClipWidth` at
+0x04, `sDivClipHeight` at +0x08, an unaligned `PolyUV4` at +0x10 copied from
`*arg2`, and the plain word at `arg1 + 0x30` at +0x14. Only the two `u16`
stack args (`arg4`/`arg5`, stored at +0x0C/+0x0E) are conditional on
`arg3 != 0`.

## The one real trap: a store that LOOKS conditional on `arg3` is not

Retail's `if (arg3 != 0)` region reads, disassembled:

```
beqz  a3, .L8001A3CC
 sw   v1, 8(t0)          <- delay slot: ALWAYS executes
sh    t1, 0xC(t0)
sh    a1, 0xE(t0)
.L8001A3CC:
```

The `sw v1,8(t0)` sits in the branch's delay slot, so it runs **regardless**
of `arg3` -- only the two `sh` half-word stores are actually gated. My first
attempt modeled all three stores (`+0x8`, `+0xC`, `+0xE`) as conditional
(matching the *visual* nesting of the assembly under the branch), which
looked entirely plausible and cost the most attempts of this stall: it
reproduced 19/27 words with a residue that read as ordinary register-swap +
scheduling noise, when the real defect was three lines up, in which
statements were even inside the `if`. Moving `*(dst+0x8) = code2;` OUT of the
`if` and leaving only the two `u16` stores inside is what closed the last 8
words in one attempt -- everything else in that 19/27 attempt (structure,
declared locals, load order) was already correct.

**Read every branch region for what actually rides the delay slot before
trusting the C-level nesting implied by the disassembly's indentation.** A
delay-slot instruction is not "inside" the branch just because a diff tool or
a human eye reads it that way; it executes unconditionally by definition of
the MIPS pipeline. asm-differ's realignment does not flag this on its own --
it takes reading the raw `.s` (or the `~>` branch-target markers) and asking
"which of these instructions is literally the one word after the branch".

## Derivation notes (kept for the next similar sibling)

- **The two-arm store collapses to ONE only with an explicit temp + separate
  store statement, never a ternary and never two `*(dst)=X;` stores (one per
  arm).** `*(s32 *)dst = cond ? A : B;` and `if (cond) *(dst)=A; else
  *(dst)=B;` both reproduced the VALUE correctly but cost one extra word:
  GCC 2.6.3's jump/dbr pass duplicated the one-instruction store into both
  arms rather than sharing a single post-merge store, because a
  one-instruction shared tail is small enough to be a profitable duplication
  candidate. Declaring `s32 val;`, assigning it in each arm, and storing it
  in ONE statement placed *after* the `if`/`else` avoided the duplication --
  but only once the merge block itself had enough real work in it (see next
  point).
- **The two unconditional-global reads (`sDivClipWidth`, `sDivClipHeight`) must be
  assigned to locals placed AFTER the `val` if/else, not before it and not as
  initializers at the top of the function.** Putting them before the branch
  hoists their loads ahead of the `sNdivOverrideSet` test entirely, which is a
  structurally different (and wrong, longer) instruction sequence. Putting
  them in the right position is also what gives the post-merge block enough
  bulk to stop GCC's single-instruction duplication reflex from the point
  above.
- Declaration ORDER of the two temps (`code` before `code2`) does not affect
  codegen; only the ORDER OF THE ASSIGNMENT STATEMENTS does (they must read
  `sDivClipWidth` before `sDivClipHeight`, matching retail's `gp_rel` load order at
  offsets `0x1C`/`0x20` from `$gp`).
- A bare `__asm__("")` scheduling barrier between the unconditional stores
  and the `if (arg3)` block was tried and made things WORSE (dropped to
  8/27 with drift) -- consistent with CLAUDE.md's own test ("if removing it
  changes which register holds a value, it's banned"; here inserting one
  changed the load ORDER of A824/A828 too, so it wasn't a pure scheduling
  no-op and was reverted).
- Reusing the dead `arg0` parameter to hold `val` instead of a fresh local,
  and flipping the `val` if/else polarity, were both tried and made no
  difference or a regression respectively -- neither is the register-swap
  fix some intermediate attempt (19/27) needed, because that swap turned out
  to be a symptom of the real (mis-modeled-conditional) defect, not an
  independent residue. Once the `+0x8` store moved out of the `if`, the
  register "swap" disappeared on its own.

### Proposed learning

**A store instruction physically inside a branch's disassembly text can
still be the branch's OWN delay-slot filler, and therefore unconditional.**
Before modeling a `beqz`/`bne` block's contents as "everything conditional",
check whether the first instruction after the branch is doing double duty as
its delay slot -- it always executes, taken or not. This cost the bulk of
this function's attempts because the wrong C shape (store nested in the
`if`) still produced a plausible-looking 19/27 near-miss that read like an
ordinary register/scheduling residue, not a semantic error.

## Naming (round 77, alpha)

`func_8001A380` -> `FillDivPolygonHeader`, parameters (`arg0..arg5`) ->
(`table`, `ctx`, `uv`, `hasUv1Codes`, `uv1Clut`, `uv1TPage`). **Tier A**:
pure header-populate leaf, same shape at all 8 call sites (code_8220.h's
own extern comment already derived every field it writes). `hasUv1Codes`/
`uv1Clut`/`uv1TPage`: tier B, evidenced by the FT3/GT3/FT4/GT4 call sites,
which pass `1` plus the calling primitive's own `+0xE`/`+0x16` (FT3/FT4)
or `+0xE`/`+0x1A` (GT3/GT4) fields -- POLY_FTn/GTn's CLUT and TPAGE words
in the Psy-Q layout -- while F3/G3/F4/G4 pass `0, 0, 0` and leave the
table's `+0xC`/`+0xE` untouched. `table`/`ctx` match code_8220_b's/this
unit's own established terms for these two objects (gDivPolygon3/
Quad, and the per-face draw context).

## Round 91 polish (bravo)

Renamed from `FillRCPolyHeader` (`python3 tools/rename.py FillRCPolyHeader
FillDivPolygonHeader`). **Tier A.** Every word it writes is a field of
Sony's DIVPOLYGON3/DIVPOLYGON4 header (libgte.h; the two share it):
`+0x0 ndiv` (number of subdivisions), `+0x4 pih`, `+0x8 piv` (the clip
area), `+0xC clut`, `+0xE tpage`, `+0x10 rgbc` (a CVECTOR, copied from the
primitive's `r0,g0,b0,code` word -- round 77's `uv` parameter, which was
never a UV), `+0x14 ot`. Parameters are now `(void *divp, PolyDrawCtx *ctx,
CVECTOR *rgbc, s32 textured, u_short clut, u_short tpage)`: round 77's
`hasUv1Codes`/`uv1Clut`/`uv1TPage` were already read off POLY_FTn/GTn's
CLUT and TPAGE words at the textured call sites, and the Sony field names
confirm them. Locals `val`/`code`/`code2` -> `ndiv`/`pih`/`piv`. The
trap above (the `piv` store rides the delay slot and is unconditional) is
the code's semantics and needs no comment; the temp-and-late-reads shape
from the derivation notes keeps one `MATCHING:` line.

Globals renamed with it:

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `D_8008A824` | `sDivClipWidth` | A | copied only into `pih`; never written; initial value 320 |
| `D_8008A828` | `sDivClipHeight` | A | copied only into `piv`; never written; initial value 240 |
| `sPolyOtCodeOverrideSet` | `sNdivOverrideSet` | A | gates the word stored at `ndiv` |
| `sPolyOtCodeOverride` | `sNdivOverride` | A | the `ndiv` used while the gate is set |

`D_80090C18`, the default `ndiv`, keeps its name: `rename.py` refuses it
because `config/psyq-objects.ld` pins Sony's `dc_cb` (libpress/vlc2) at
0x80090C14 with 8 bytes, which covers it. Its only writer is code_8220_b's
SortTmdObject, from bits 9-11 of the drawn object's flags, so the word
looks like game data; whether `dc_cb` really is 8 bytes is proposed to the
head. Round 77 had left `D_8008A824`/`D_8008A828` as `D_` names for lack of
a second accessor; the DIVPOLYGON layout is the evidence that was missing.

History moved from include/code_8220.h's old extern comment: this function
was matched in round 44 once the gp_rel blocker that stalled it at carve
time (round 13) was resolved, and the extern lived in the shared header only
so that SubmitPolyF3, then still INCLUDE_ASM, could compile. The
declaration is now a prototype in code_8220_b.c, the only unit that calls it.
