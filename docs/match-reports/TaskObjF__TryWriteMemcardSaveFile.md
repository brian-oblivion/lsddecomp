# TaskObjF__TryWriteMemcardSaveFile -- MATCHED 240/240, round 75 (delta). Lever: `a3` is a `u8` parameter (not `s32` + a duplicate `flagCopy` local), and `arg6` is used directly (no early `payload` copy).

REVISITED, round 75: MATCHED 240/240 in 3 builds, whole image `OK: build matches retail`; names/types used (the parameter TYPE was the lever).

## Round 75 (delta): revisit

**Baseline first.** The preserved body, rebuilt live exactly as below
(round-37 snapshot), measured **191/240, `insertions 5 / deletions 5`**,
47 positional skeleton diffs, 239 words (1 short).

**What retail says, read word by word.** `$a3` is copied to `$s4` at entry
and `$s4` is copied again to `$s7` in the first `bne`'s delay slot; `$s4`
feeds the `sb` of `a3 + 0x10`, `$s7` feeds `andi 0xFF; sll 7` for the first
`write` size. Two live pseudos holding one value, with the zero-extension
deferred to the one use that needs it, is what a narrow (`u8`) parameter
looks like in GCC 2.6.3: the promoted incoming word and its QImode copy.
The preserved body imitated that with an `s32 a3` plus a hand-made
`flagCopy = a3` duplicate, which the allocator coalesced differently (one
s-register fewer, the permuted callee-saved set, the empty delay slot).

| build | change | score | ins/del |
| --- | --- | --- | --- |
| 1 | preserved body as-is | 191/240 (239 words) | 5 / 5 |
| 2 | `u8 a3` (definition + forward decl), drop `flagCopy`, size `(a3 << 7) + 0x80` | 238/240 (240 words) | 1 / 1 -- only `lw arg6`/`lw arg7` order swapped |
| 3 | drop `payload = arg6`, pass `arg6` directly | **240/240**, whole image OK | 0 / 0 |

The caller `TaskObjF__WriteMemcardSaveFile` (already matched, passes
`a3 & 0xFF` from its own `char a3`) is byte-identical under the new
prototype -- the whole-image oracle is green.

Tell: two callee-saved registers holding copies of the same incoming
argument register, with a mask (`andi 0xFF`/`0xFFFF`) applied only at one
use, means a narrow parameter type -- not a duplicate local. The same
round-37/-27 "register saturation" story for this function was an
artefact of the duplicate locals invented to reach the frame size.

### Proposed learning

- **Duplicate-register copies of one argument = narrow parameter type.**
  When retail copies `$aN` to one s-reg at entry and then that s-reg to a
  second one later, with a zero/sign-extension appearing only at the
  second's use, type the parameter `u8`/`u16` (both the definition and
  every prototype in scope) instead of adding a copy local. Check the
  caller still matches with the whole-image oracle.

## Final C (matched)

```c
s32 TaskObjF__TryWriteMemcardSaveFile(TaskObjF *self, s32 a1, s32 handle, u8 a3, s32 arg5, s32 arg6, s32 arg7) {
    char pathBuf[0x20];
    char *path;
    s32 fileHandle;
    s32 openMode;
    McIconSource *src;
    McSaveHeader *req;

    path = BuildMemcardPath((DeviceName866E8 *)pathBuf, self->cardSlot, (char *)a1);
    delete(path);
    openMode = ((((u32)arg7 + 0x21FF) >> 13) << 16) | 0x200;
    fileHandle = open(path, openMode);
    if (fileHandle == -1) {
        printf(D_80011530);
        return 0;
    }
    close(fileHandle);
    fileHandle = open(path, 2);
    if (fileHandle == -1) {
        return 0;
    }
    src = ((McIconSourceRef *)arg5)->iconSource;
    req = (McSaveHeader *)BMemPMgrAlloc(0x200);
    req->magic0 = 'S';
    req->magic1 = 'C';
    req->iconFrameFlag = a3 + 0x10;
    req->blockCount = ((u32)arg7 + 0x1FFF) >> 13;
    strcpy(req->title, (char *)handle);
    req->palette[0] = src->palette[0];
    req->palette[1] = src->palette[1];
    req->frame0 = src->frame0;
    req->frame1 = src->frame1;
    req->frame2 = src->frame2;
    write(fileHandle, req, (a3 << 7) + 0x80);
    BMemPMgrFree(req);
    write(fileHandle, (void *)arg6, (((u32)arg7 + 0x7F) >> 7) << 7);
    close(fileHandle);
    return 1;
}
```

(Local types, externs and the history below are unchanged; the history
is kept as the record of the pre-round-75 stall.)

## Previous title (superseded)

TaskObjF__TryWriteMemcardSaveFile -- STALL. Length: 1 word SHORT (239/240, 0x3BC/0x3C0). Word-match: 191/240 (re-measured round 37; previously recorded 188/240 -- see round-37 note). First real diff: file 0x3F774 / vram 0x8004EF74 (register-permutation set-up; the SEMANTIC first diff, ignoring the permuted callee-saved set, is file 0x3F7A4 / vram 0x8004EFA4, missing `sw $s0,0x30($sp)`, immediately followed by an empty `nop` at file 0x3F7F0 / vram 0x8004EFF0 where retail fills the delay slot with `move $s7,$s4`).

> Renamed from `func_8004EF6C` on 2026-09-20 (tools/rename.py). Address 0x8004ef6c.

> **ROUND 37 (delta): re-verified by rebuilding this EXACT preserved body,
> then ran the permuter for the first time on this function (never
> searched before this round, per the round's own thesis).** Splicing the
> body back in and rebuilding reproduces the residue described below
> exactly (confirmed with `asm-differ`): the whole callee-saved register
> set permuted relative to retail's own, plus the one missing
> `sw $s0,0x30($sp)` / delay-slot `move $s7,$s4` at file
> 0x3F7A4/0x3F7F0. The raw word-match figure re-measured at **191/240**,
> not the previously-recorded 188/240 -- not a regression, just the first
> re-measurement of this body since round 34 relinked the BIOS trampolines
> (`open`/`close`/`delete`/`write`/etc, formerly `func_80050938`-style
> names) this function calls through; the total LENGTH gap (239/240, 1
> word short) is unchanged and the residue is the same kind and same
> location.
>
> Scaffolded with `tools/setup-permuter.sh`; `--debug --stack-diffs` base
> score was **770** (`Stack Differences: 100 (1)`, `Register Differences:
> 34 (5)`, `Insertions: 2 (100)`, `Deletions: 3 (100)`), confirmed via a
> direct `objdump` of `target.o` vs `base.o` to be the SAME register
> permutation visible in the real project build -- not a broken scaffold,
> just a higher base score than the "single insertion+deletion, score
> 200" signature this round's brief predicted, because this residue is a
> genuine multi-register permutation (5 callee-saved registers reassigned)
> on top of the one-word gap, not a lone one-instruction miss.
>
> Ran `timeout 900 permuter.py -j 6 --stop-on-zero --best-only
> --stack-diffs` in the background while doing hand work on the rest of
> this round's list. **65,379 iterations**, floor reached was **112**
> (down from base 770, seen 55 times, never lower), **no zero found**.
> Compile-error noise grew steadily through the run (0 errors early,
> 794 errors per generation by the end) -- the mutator increasingly
> proposing non-compiling variants, not evidence of a converged search.
> The run was launched detached (`nohup ... &`, not this shell's direct
> child), so its own exit code could not be read back with `wait`; the
> log's abrupt mid-line cutoff (score printing truncated mid-number at
> iteration 65379, immediately followed by the interpreter's own
> multiprocessing shutdown warning) lands almost exactly at the 900s
> bound measured against the log file's mtime relative to launch time,
> consistent with the `timeout` bound firing rather than an external
> kill -- but this is circumstantial, not a captured `rc`, and is
> reported as such rather than asserted as `rc=124`. Not closed in this
> budget. Not upgraded to "permuter-exhausted" -- one run at one
> iteration count is not sufficient for that verdict per this project's
> own standing rule. Restored to `INCLUDE_ASM`, not left live.

> **ROUND 27 (delta): re-verified, one new attempt, negative.** Rebuilt the
> exact preserved body from a clean `INCLUDE_ASM` baseline: confirmed
> 188/240, no drift beyond the documented 1-word gap. Per the head's
> "arm polarity" lever (invert a guard so the expensive arm falls through,
> which can leave the correct delay slot empty for a different reason),
> tried rewriting the one guard adjacent to the residue --
> `if (fileHandle == -1) { ... }` -> `if (fileHandle < 0) { ... }`
> (semantically identical for an `s32` file handle where `-1` is the only
> negative sentinel value) -- on the theory that a different comparison
> might shift which register holds the `-1`/sentinel and free up the
> source register for the needed delay-slot move. **Regressed
> catastrophically to 6/240**: `< 0` compiles via a completely different
> instruction sequence (sign-bit test, `slt`/shift family) rather than
> retail's direct `bne reg,-1,target`, confirming the ORIGINAL `== -1`
> form is already correct and this residue has nothing to do with HOW the
> sentinel is tested. Reverted immediately. This function's guard was
> already correctly polarized (`bne` on not-equal, matching retail's own
> branch exactly) before this attempt -- the "arm polarity" lever named by
> the coordinator does not apply here; the residue is purely which
> independent register-materializing move (of two duplicate-parameter
> locals already both required for the correct frame size) the compiler
> schedules into a specific branch's delay slot, and it is DOWNSTREAM of a
> register-allocation choice (which physical register holds the `-1`
> literal, and whether that overlaps `a3`'s own home register) rather than
> of source statement order at that point. Not re-attempted further this
> round; restored to `INCLUDE_ASM` unchanged.

Unit: `class_3bb8c_f`. Not toolchain-blocked: no `gp_rel` hit, no
`addiu $at,$at,%lo` hit, no dense-`switch`/`jr $v0` dispatch in
`asm/nonmatchings/class_3bb8c_f/TaskObjF__TryWriteMemcardSaveFile.s`.

**This is the first C ever attempted against this function.** The previous
round's report ("predicted-hard, screened and read but not attempted...
given scale") is superseded by this one -- a full derivation was attempted,
all struct layouts/control flow/arithmetic are confirmed against a real
build, and the residue is now pinned to a single, precisely-characterized
scheduling choice.

## What it does

`s32 TaskObjF__TryWriteMemcardSaveFile(TaskObjF *self, s32 a1, s32 handle, s32 a3, s32 arg5,
s32 arg6, s32 arg7)` (signature per its one caller, `TaskObjF__WriteMemcardSaveFile`, which
is ALREADY MATCHED -- no parameter type here was changed in a way that
touches that caller's own compiled bytes, see the header-discipline note
below). Reads as a **"WriteFile" memory-card/CD streaming write** (the
error string this function logs on failure is literally `"File not create
in WriteFile\n"`, confirmed in rodata at `D_80011530`):

1. Builds a device path via `BuildMemcardPath(pathBuf, self->cardSlot, (char
   *)a1)` (already-matched sibling; `a1` is really a `char *` suffix
   despite its established `s32` type in this file's forward
   declarations -- kept `s32` at the parameter, cast at the call site,
   same reasoning as `handle` below).
2. `func_80050908(path)` -- return value unused, likely a stat/probe call.
3. Computes an "open mode" word: `((((u32)arg7 + 0x21FF) >> 13) << 16) |
   0x200` -- `arg7` (the payload size) rounded up past a reserved
   0x200-byte header, converted to a count of 0x2000-byte blocks, packed
   into the mode word's upper 16 bits alongside a literal `0x200` mode
   flag. Opens with `func_80050938(path, openMode)`. On failure (`-1`),
   logs the error and returns 0.
4. On success: closes that probe handle immediately
   (`func_800508F8`), then re-opens the SAME path with a plain mode `2`
   (`func_80050938(path, 2)`) -- this second handle is the one actually
   used for the rest of the function. Fails the same way (return 0, no
   error log this time) if this second open also fails.
5. Resolves `src = ((McIconSourceRef *)arg5)->iconSource` -- `arg5`'s own type is
   otherwise unestablished; only this one field is ever read.
6. Allocates a 0x200-byte request buffer (`BMemPMgrAlloc`), fills a 4-byte
   header (`'S'`, `'C'`, `(u8)(a3+0x10)`, `(u8)ceil(arg7/0x2000)`),
   `strcpy`s a filename into it (source: `handle`, the function's OWN 3rd
   parameter -- see below), then copies FIVE regions from `src` into it:
   two small 0x10-byte sub-records (an unrolled 2-element array, no loop,
   no alignment check -- a low-alignment whole-struct copy) followed by
   three raw 0x80-byte spans (each with a genuine RUNTIME
   `(src|dst)&3`-checked aligned/unaligned copy, matching the
   already-documented "byte array forces a runtime-checked copy" idiom).
7. Submits two `func_80013488` read/write requests: one for the
   just-built request buffer (size `((a3&0xFF)<<7)+0x80`), one directly
   into the CALLER's own buffer `arg6` (size `arg7` rounded up to a
   multiple of 0x80). Frees the request buffer between the two calls (NOT
   after both -- the second call never touches it). Closes the handle,
   returns 1.

### The `handle` parameter is really a C string, not a numeric handle

Despite the name (inherited from this file's existing forward
declaration, established before this round) and its `s32` type, this
function's own use of it is exclusively as `strcpy`'s SOURCE argument --
i.e. a filename/tag string, unrelated to the internal file handles this
function opens and closes via `func_80050938`. **Kept `s32` at the
parameter** (cast to `(char *)handle` at the one use site) rather than
retyped, since `TaskObjF__WriteMemcardSaveFile` -- the ONLY caller, already matched --
forwards this same value through unchanged with its own `s32 handle`
parameter; retyping either signature risks that caller's own compiled
bytes for no byte-level benefit (the cast is functionally identical
either way).

## New types (all local to `class_3bb8c_f.c` -- none shared, no header
## changes made this round)

**RENAMED round 60** (track 3 naming pass; these are local typedefs, not
symbol-table entries, so `tools/rename.py` does not touch them -- edited
by hand in `src/class_3bb8c_f.c` and re-verified with an isolated
`cpp|cc1` syntax check since the body sits in `#if 0` and the normal
build never compiles it; see `## Naming` below). Original names, carried
from the round that first derived this body: `StreamSmallSub`/
`StreamRawBlock`/`StreamSrcObj`/`StreamArg5Obj`/`StreamReq`, with fields
`f0..fE`/`raw`/`arr`/`unk10`/`tag0,tag1,b2,b3,name,arr,blkA,blkB,blkC`.

```c
typedef struct IconPaletteHalf {
    s16 color[8];
} IconPaletteHalf;

typedef struct IconFrame {
    u8 raw[0x80];
} IconFrame;

typedef struct McIconSource {
    u8 pad0[0x14];
    IconPaletteHalf palette[2];  /* +0x14 */
    u8 pad34[0x40 - 0x34];
    IconFrame frame0;              /* +0x40 */
    IconFrame frame1;                /* +0xC0 */
    IconFrame frame2;                  /* +0x140 */
} McIconSource;

typedef struct McIconSourceRef {
    u8 pad0[0x10];
    McIconSource *iconSource;
} McIconSourceRef;

typedef struct McSaveHeader {
    u8 magic0;
    u8 magic1;
    u8 iconFrameFlag;
    u8 blockCount;
    char title[0x5C];
    IconPaletteHalf palette[2];
    IconFrame frame0;
    IconFrame frame1;
    IconFrame frame2;
} McSaveHeader;
```

`IconPaletteHalf` deliberately has no `s32` member (alignment 2) so a
whole-struct copy compiles to the unaligned `lwl`/`lwr` + `swl`/`swr`
idiom already documented for `Descriptor10`
(`include/class_3bb8c.h`) and `Block24` (`src/class_3bb8c_r.c`).
`IconFrame` is a plain byte array (alignment 1) so a whole-struct
copy compiles to the RUNTIME-alignment-checked dual-path copy retail
actually shows for the three 0x80-byte spans -- confirmed against
`class_3bb8c_r.c`'s own comment on `Block24`: *"a byte array... compiles
the copy as a generic runtime-alignment-checked memcpy loop instead"*.
Both idioms transferred to this function unchanged, on the first attempt,
for all five copy regions.

Also new: `extern const char D_80011530[];` (the rodata error string --
referenced, not retyped, per this round's broadcast) and two
project-external prototypes local to this call site's own shape (neither
declared elsewhere in the project): `extern s32 func_80013488(s32 handle,
void *buf, s32 size);` and `extern void func_80012C20(const char *fmt);`
(this call site passes only the format string; `code_8220.h`'s existing
3-arg view of the same symbol is a DIFFERENT call site's shape, per the
per-unit local-view convention).

## Best body reached (188/240, 0x3BC/0x3C0 -- 1 word short, zero drift
## beyond that)

```c
s32 TaskObjF__TryWriteMemcardSaveFile(TaskObjF *self, s32 a1, s32 handle, s32 a3, s32 arg5, s32 arg6, s32 arg7) {
    char pathBuf[0x20];
    char *path;
    s32 fileHandle;
    s32 openMode;
    s32 flagCopy;
    s32 payload;
    McIconSource *src;
    McSaveHeader *req;

    payload = arg6;
    path = BuildMemcardPath((DeviceName866E8 *)pathBuf, self->cardSlot, (char *)a1);
    delete(path);
    openMode = ((((u32)arg7 + 0x21FF) >> 13) << 16) | 0x200;
    fileHandle = open(path, openMode);
    flagCopy = a3;
    if (fileHandle == -1) {
        printf(D_80011530);
        return 0;
    }
    close(fileHandle);
    fileHandle = open(path, 2);
    if (fileHandle == -1) {
        return 0;
    }
    src = ((McIconSourceRef *)arg5)->iconSource;
    req = (McSaveHeader *)BMemPMgrAlloc(0x200);
    req->magic0 = 'S';
    req->magic1 = 'C';
    req->iconFrameFlag = a3 + 0x10;
    req->blockCount = ((u32)arg7 + 0x1FFF) >> 13;
    strcpy(req->title, (char *)handle);
    req->palette[0] = src->palette[0];
    req->palette[1] = src->palette[1];
    req->frame0 = src->frame0;
    req->frame1 = src->frame1;
    req->frame2 = src->frame2;
    write(fileHandle, req, (((flagCopy & 0xFF) << 7)) + 0x80);
    BMemPMgrFree(req);
    write(fileHandle, (void *)payload, (((u32)arg7 + 0x7F) >> 7) << 7);
    close(fileHandle);
    return 1;
}
```

(round 60: this snapshot now matches the LIVE preserved body in
`src/class_3bb8c_f.c` exactly -- the `func_80050908`/`func_80050938`/
`func_800508F8`/`func_80013488`/`func_80012C20`-style names in the prose
above this point in the report are historical, from before round 34
relinked these as Sony's `delete`/`open`/`close`/`write`/`printf`; the
code block itself is now current and resumable, confirmed via
`tools/stalesyms.py`.)

## Levers that mattered, in order

1. **A naive signed `>>` on `arg7` compiles to `sra`; retail uses `srl`
   (logical) at all three of this function's shift sites.** `arg7` is
   `s32`, so `(arg7 + K) >> N` sign-extends by default. Retail's own
   original source evidently treated these as unsigned at the shift.
   Casting the shifted operand to `(u32)` first (`((u32)arg7 + K) >> N`)
   reproduces `srl` exactly at all three sites (`openMode`'s block count,
   `req->b3`, and the final payload-size rounding) with zero other
   change. A one-line, mechanical, fully-resolved fix -- not a residue.
2. **The struct-copy idioms (low-alignment whole-struct copy vs.
   byte-array runtime-checked copy) transferred perfectly on the first
   attempt**, closing what was, by instruction count, the bulk of this
   function (roughly 130 of its 240 words) with zero iteration needed
   beyond getting the two struct SHAPES (`StreamSmallSub` all-`s16`,
   `StreamRawBlock` a plain byte array) right.
3. **The register/frame-size gap (this function is 9-register-saturated,
   per the round's own census) needed two explicit, EARLY-assigned local
   variables that are otherwise pure duplicates of an existing
   parameter** (`flagCopy = a3;` and `payload = arg6;`, both assigned
   immediately after the point retail's own asm shows the equivalent
   materialization) to get GCC to allocate a persistent register for
   each, closing the frame size from `0x50` (8 registers, 2 short) to the
   correct `0x58` (10 registers, exact). Without both locals, the
   register count undershoots; with a differently-typed or
   differently-positioned single local, it either undershoots by 2 words
   or OVERSHOOTS by 1 (see residue below) -- this pairing is the only
   combination found that lands the frame size exactly while staying
   within one word of the total length.

## Residue: one word, confirmed as the SAME register-saturation /
## delay-slot-scheduling class already documented for this function's own
## caller

Every remaining word difference is either (a) a pure register-identity
swap (the whole callee-saved set is permuted relative to retail's own --
`s0`↔`s1`↔`s2` etc., same values, same instructions, different physical
registers throughout) or (b) this ONE genuine scheduling difference:
retail's compiled tail places the SECOND duplicate-register move
(`move $s7, $s4` -- materializing `flagCopy` into its persistent home)
in the DELAY SLOT of the `bne $s2, $s1, ...` branch that tests the first
open call's result; this build computes the equivalent move ONE
INSTRUCTION EARLIER instead, leaving that exact delay slot as an explicit
`nop`. Same total number of "real" instructions, same values, same final
register assignments after the branch -- purely a one-word scheduling
placement, the same "which independent instruction fills a delay slot"
class already documented in several other functions' reports this round.

**Confirmed this is genuinely hard to move, not merely unexplored**, via
~10 variants, each rebuilt and re-measured:
- Both explicit locals typed `s32` vs. one/both typed `void *` -- no
  change to either the frame size or the delay-slot placement (typing is
  cosmetic here).
- `flagCopy`/`payload` assigned at their "natural" position (immediately
  before first use) vs. both moved to the very top of the function, vs.
  swapped (whichever one is "first" moved to the top) -- length oscillated
  between 235 and 241 words depending on the exact combination, NEVER
  landing on the correct 240 with zero drift; the 239/240 configuration
  documented above is the closest reached.
- A bare `__asm__("");` scheduling barrier (the permitted form -- reorders
  only, per CLAUDE.md's own test) placed immediately before `flagCopy =
  a3;`, to try to force the assignment to stay adjacent to the branch --
  made the score WORSE (187 vs 188), confirming the barrier does not
  target the specific reordering needed here.
- Declaration order of the two locals swapped -- byte-identical output,
  consistent with this project's repeated finding that declaration order
  alone does not drive register/scheduling decisions in this GCC 2.6.3
  build.

This is the SAME class `TaskObjF__TryWriteMemcardSaveFile`'s own caller, `TaskObjF__WriteMemcardSaveFile`,
already hit and documented (per that function's own report, referenced
in this function's prior round's write-up) as "a full 9-value bijection
permuted relative to retail's own... reshaping does not resolve it" --
and per `Class866E8__BuildFootprintSlots`'s precedent (7 reshaping variants, zero
movement), this project's own experience is that this class does not
respond to further manual source reshaping. Not pursued further this
round; `TaskObjF__WriteMemcardSaveFile`'s residue is register PERMUTATION with zero
length drift, while this one is permutation PLUS a one-word scheduling
gap -- worth noting as a variant of the same family (register-saturated
functions can show EITHER pure permutation OR permutation-plus-one-word,
depending on whether the specific values in play happen to need a
duplicate-register materialization), not a new independent class.

## Screening

```
grep -n 'gp_rel' asm/nonmatchings/class_3bb8c_f/TaskObjF__TryWriteMemcardSaveFile.s        # no hits
grep -n 'addiu *$at, *$at, *%lo' asm/nonmatchings/class_3bb8c_f/TaskObjF__TryWriteMemcardSaveFile.s  # no hits
```
Clean of both open toolchain blockers (per the coordinator's own
screening this round, not re-run).

## Attempts

~14 iterations: initial full transcription (2/240 raw, 235/240 true
length -- 5 words short, no compile errors, all struct/control-flow
derivation correct on the first pass) -> two signed/unsigned shift fixes
(genuine correctness fixes, zero length change, but necessary for a
trustworthy score) -> `flagCopy` local alone (236/240, +1 word) ->
`payload` local alone, `void *`-typed, assigned at its "natural" late
position (241/240, one word TOO LONG -- a redundant register-materializing
move retail does not have) -> `payload` moved to the very top of the
function (239/240, 188 real words matching -- the configuration kept) ->
five further variants (position swaps, `s32` vs `void *` typing, a
scheduling barrier, declaration-order swap) all either matched or
regressed the 188/240 best, none improved it.

## Naming (round 60, track 3)

`func_8004EF6C` -> `TaskObjF__TryWriteMemcardSaveFile`. **Tier A.**
Evidence: this unit's own `TaskObjF__WriteMemcardSaveFile` (its only
caller) is the bounded-retry wrapper around it, and the "Try" prefix
matches this project's existing convention for a single, non-retrying
attempt a caller may retry (`SceneNode__TryAttachNearby`,
`src/code_d294_b.c`). "Memcard" is established by `BuildMemcardPath`'s
own `bu00:`/`bu10:` device templates; "SaveFile" is established by the
0x200-byte buffer's structural match to the PS1 memory-card save file
header format (see the local-type naming note below) plus the BIOS
create-with-block-count `open()` convention (`openMode`'s upper 16 bits
encode a block count only when creating a file).

Local types renamed for the same reason, Tier B (structural match to a
well-known format, not a string/symbol-table fact): `StreamSmallSub` ->
`IconPaletteHalf`, `StreamRawBlock` -> `IconFrame`, `StreamSrcObj` ->
`McIconSource`, `StreamArg5Obj` -> `McIconSourceRef`, `StreamReq` ->
`McSaveHeader`, with fields renamed to match (`tag0/tag1` -> `magic0/
magic1`, `b2` -> `iconFrameFlag`, `b3` -> `blockCount`, `name` ->
`title`, `arr` -> `palette`, `blkA/blkB/blkC` -> `frame0/frame1/frame2`).
These are unit-local typedefs, not symbols in `config/symbols.slps01556.lsdde.txt`,
so `tools/rename.py` does not apply; edited directly in `src/class_3bb8c_f.c`
and confirmed to still parse with an isolated `cpp|cc1` pass (the body is
`#if 0`, so the normal build never compiles it and could not have caught
a syntax error here).

### Proposed learnings

- **A register-saturated function's residue is not always PURE
  permutation.** `TaskObjF__WriteMemcardSaveFile` (this function's own caller) hit a
  9-value bijection with ZERO length drift; this function, also
  9-register-saturated, hits permutation PLUS a genuine one-word
  delay-slot placement difference. Screen for BOTH shapes when a
  function is flagged register-saturated -- a 1-word gap on top of an
  otherwise-permuted register set is not evidence the derivation is
  wrong, it can be this same family showing up slightly differently.
- **Two independent local variables, each a pure duplicate of an
  existing parameter, may BOTH be needed to reach a saturated function's
  correct register count -- and the exact PAIRING (which is assigned
  early vs. late, matching or not matching) determines whether the total
  length lands short, long, or exact**, independent of either variable's
  own type. This is a search worth doing systematically (try all
  early/late combinations) before concluding a saturated function is a
  pure-permutation stall, since the combination space is small (a handful
  of position pairs) and cheap to enumerate with the fast
  `build-and-verify.sh` cycle this project already has.
- **The "byte array forces a runtime-alignment-checked copy vs. a
  no-`s32`-member struct forces the unconditional `lwl`/`lwr` idiom"
  pair (documented separately for `Block24` and `Descriptor10`/`Elem`-
  family structs) transfers cleanly to a genuinely new, much larger
  derivation with zero iteration.** Worth trusting fully on sight when
  the disassembly shows this exact shape (a runtime `(src|dst)&3` check
  with two code paths vs. an unconditional `lwl`/`lwr` sequence with
  none) rather than re-deriving it from first principles each time.

## Track 4 (2026-09-26, round 89)

Parameters retyped with TaskObjF's unification (include/TaskObjF.h), byte-identical: `a1` is `char *fileName` (TaskObjF::fileName; BuildMemcardPath's suffix), `handle` is `char *title` (TaskObjF::title; strcpy'd into the header), `arg5` is `struct TimImage *icon` (TaskObjF::iconImage, from Class86B60's iconHandle; the icon source is its FileResource `buffer`, +0x010, so the local McIconSourceRef view is gone), `arg6` is `void *data` and `arg7` is `s32 size`. Only CopyMemcardIconTemplate's own `(s32, s32)` view still takes casts.
