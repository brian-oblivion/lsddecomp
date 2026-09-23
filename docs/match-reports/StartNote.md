# StartNote -- STALL: 15 words LONG (402/387 built length), first real diff at vram 0x8002FAC4 (function entry, differing register-save set / frame size 0x140 vs retail's 0x148)

> Renamed from `func_8002FAC4` on 2026-09-20 (tools/rename.py). Address 0x8002fac4.

Unit: `src/code_179d8_m.c`. Round 27, runner bravo. This is the ordered
work-list's item 1 -- FRESH ground, no prior report existed for this
function.

## Screens (clean)

```
grep -n 'gp_rel' asm/nonmatchings/code_179d8_m/StartNote.s            -> no hits
grep -A2 -nE '\b(mflo|mfhi)\b' asm/nonmatchings/code_179d8_m/StartNote.s \
  | grep -E '\b(mult|multu|div|divu)\b'                                   -> no hits
```

## Result

`./build-and-verify.sh` GREEN with `INCLUDE_ASM` restored (self-tested by
splicing the preserved body below back in and rebuilding). Best attempt
compiled clean at **402 words against retail's 387** (`objdump -t
build/src/code_179d8_m.c.o` on the linked object; the `funcdiff.py`
in-range figure, 6/387, is **not trustworthy** given the length mismatch
and its own drift warning -- see CLAUDE.md's four-ways-a-score-lies list).
`tools/asm-differ/diff.py`, which realigns past the length gap, shows the
function's **large-scale structure is already right**: the signature, all
seven branch targets, the two loops, every struct field access and every
call site line up with retail one-for-one, modulo register renames, for
most of the function's length. Two specific, localized residues account
for essentially the whole 15-word gap (see below); nothing else in the
function showed a genuine content/order mismatch in this round's reading.

## Round 30 (charlie) update: rebuilt, confirmed accurate

Re-spliced this exact preserved body and rebuilt from scratch. **All title
figures reconfirmed:** `objdump -t build/src/code_179d8_m.c.o` shows
`StartNote` at `0x648` bytes = **402 words**, retail 387 (15 long,
exactly as titled), and `funcdiff.py`'s in-range figure is **6/387** with
its drift warning firing, matching this report's own caution. No new axis
attempted this round: this is fresh ground from the immediately preceding
round with a thorough two-residue diagnosis already in place (the
early-materialization split and the `D_8008EA26`-cluster addressing cost),
lowest priority of the six assigned functions, and the largest/hardest —
correctly triaged as such by the work order.

## Signature -- corroborated independently by three sibling units, not just derived here

```c
s32 StartNote(s32 a0, s16 a1, s16 a2, u16 a3, u16 a4, u16 a5);
```

Before writing any C, `grep -rn StartNote src/*.c docs/match-reports/*.md`
turned up this **exact** signature already guessed independently by
`code_179d8_i.c`, `code_179d8_j.c` and `code_179d8_k.c` (each calls this
function and typed it from its own call site), plus two live call sites
with concrete argument roles:

- `code_179d8_j.c`'s `SpuVmSeKeyOn` (matched): `return
  StartNote(0x21, (s16) p0, (s16) p1, (u16) p2, outA, outB);` -- `a0 ==
  0x21` is a real sentinel value this function itself branches on (see
  below).
- `code_179d8_k.c`'s `func_800344FC` (its own report, STALL):
  `StartNote(packed, note, vol, (u8)a3, (u16)divided, status)` where
  `packed = (a1<<8)|a0` is a `[screen | slot<<8]` pair into the SAME
  `D_800902E8[][]` array this function itself indexes with `a0`. This
  confirms `a0`'s low byte is a `D_800902E8` row index and its next byte an
  index into that row -- exactly what this function's own disassembly
  does with `a0 & 0xFF` and `(a0<<16)>>>24`.

This cross-corroboration also fixed a signature bug `m2c` could not have
caught: the true 5th parameter (this report's `a4`) gates the function's
main branch (`bnez s2,...` in retail, where `s2` is loaded from the 5th
incoming stack slot). An early draft this round wrote `if (a5 != 0)`
instead of `if (a4 != 0)` -- both compile, both build clean, and the bug
was only caught by cross-checking which REGISTER retail's branch actually
tests (`s2`, not `s6`) against `tools/asm-differ`'s realigned output. Worth
flagging generally: a 5-argument-plus function with two stack-passed tail
arguments is exactly the shape where this kind of off-by-one is invisible
until you diff registers, not just word counts.

## Struct/global knowledge derived this round

- `SlotE968M` / `D_8008E968`: the SAME 0x10-stride table `code_179d8_j.c`
  already documents as `SlotE968` (`unk0`/`unk1`/`unk4`) -- this function's
  own local view, same shape, used here as `&D_8008E968[a2]` (a2 = this
  function's own s16 parameter, NOT a channel id from `func_80032148`).
- `Entry90902E8M` / `D_800902E8[]`: the SAME 172(0xAC)-byte
  `[screen][slot]`-indexed record array `code_179d8_i/j/k.c` each already
  document with their own reduced view. This function only needs
  `unk12` (a byte OFFSET, per `code_179d8_k.c`'s fuller struct) and reads
  a per-voice "speed" `s16` at `*(s16*)((u8*)rec + 0x4E + rec->unk12*2)` --
  the SAME access shape `code_179d8_k.c`'s `func_800344FC` already uses on
  the identical field, corroborating both units' independent readings.
- `Tbl32E978` (already declared for `SpuVmPBVoice`'s stall) needed
  EXTENDING, not a second conflicting type: this function additionally
  touches `unk0`/`unk1`/`unk2`/`unk3`/`unk4`/`unk5`/`unk6`/`unk7` (a
  per-channel byte-field block used in bulk) alongside the existing
  `unkC`/`unkD`, none of which overlap.
- `ObjE970` (already declared with a `+0x18` byte field for the stalled
  `StepVoiceEnvelope`/`StepVoiceFade` bodies) needed a NEW `u16` field at
  `+0x12` -- a "channel-count difficulty threshold" compared unsigned
  against `D_8008EA13`. Since neither prior user of `ObjE970` is currently
  compiled (both are `INCLUDE_ASM`), extending the struct in place was
  safe; confirmed no active code referenced the old shape before editing.
- Twelve NEW plain scratch globals in the `D_8008EA0C`-`D_8008EA20` /
  `D_8008EA24` cluster this unit had not needed yet: `D_8008EA0C`,
  `D_8008EA0D`, `D_8008EA0E`, `D_8008EA0F`, `D_8008EA1C`..`D_8008EA20`,
  `D_8008EA24`. All plain `u8`/`u16` scratch, all already declared with
  identical types in `code_179d8_j.c`'s own header block for the sibling
  `func_80030E90` -- this was the single biggest time-saver this round
  (see "Where this came from" below).

## Where this came from: a sibling unit had already typed almost everything

Before deriving anything by hand, `grep -rn StartNote src/*.c` found
this function's signature independently triple-corroborated (above), and
`code_179d8_j.c`'s header comment for its OWN (still-`INCLUDE_ASM`)
`func_80030E90` already named the exact same globals this function
touches (`D_8008EA0C` through `D_8008EA20`, `D_8008EA24`) as "a `start
channel` setup routine that stages its parameters and a couple of table
lookups into a block of one/two-byte globals before registering a new
active-channel record" -- which is precisely this function's own shape.
Reusing that unit's already-typed externs (rather than re-deriving them
from scratch) turned what would have been a multi-round struct-recovery
exercise into a formatting exercise. **Grep the whole `src/` tree for a
function's name and its likely globals before doing any asm archaeology
by hand** -- for a function this deep into a shared record-family unit,
someone touching an adjacent unit has usually already typed half of it.

## The two residues that account for the 15-word gap

### 1. An early value materializes too soon (byte1/`a0s16` split), ~2-3 words

Retail's `a0` (this function's packed screen/slot id) needs THREE
different views: its low byte (`a0 & 0xFF`, for the `D_800902E8` row
index), a sign-extended 16-bit copy (`s1` in retail, used later as
`StopNote`'s first argument), and a second byte (`(u8)((u16)s1 >>
8)`, the row's slot index) -- and retail computes the LAST TWO from a
SHARED intermediate (`v1 = a0 << 16`, materialized ONCE, then `sra v1,16`
for the sign-extend and, SEPARATELY, `srl v1,24` reusing that SAME shifted
value for the slot byte). Writing this as three independent C expressions
(`byte0 = (u8)a0`, `shifted = a0<<16`, `a0s16 = (s16)(shifted>>16)`,
`byte1 = (u32)shifted>>24`) reproduces the SHARED-INTERMEDIATE shape
correctly (confirmed: `tools/asm-differ` shows the `srl`/multiply-by-172
chain landing byte-for-byte identical to retail once phrased this way,
where an earlier draft using `(u8)(a0>>8)` directly against the raw
32-bit parameter compiled to a simple `srl 8` instead of retail's
`sll 16`/`sra 16`/`srl 24` three-instruction dance).

**What did NOT close: the exact program POINT where this triple gets
materialized.** My C computes it immediately after `byte0`, at the very
top of the function; retail computes it LATER, interleaved with the `s7 =
a3` copy and the `sh a2,sp+0x110` original-parameter save. A bare
`__asm__("")` inserted between `byte0`'s computation and this triple was
tried as a scheduling barrier and made things dramatically WORSE (it
disrupted the compiler's whole prologue scheduling, reordering the
stack-argument loads ahead of the sign-extension code entirely --
reverted immediately, recorded here so the next attempt does not re-try
it at this exact position). Not resolved within this round's budget;
worth a few words at most.

### 2. Mid-loop re-reads of `D_8008EA26` cost 2-3x retail's addressing, ~10-12 words

This is the larger and more interesting residue. Deep in the per-match
loop, retail re-reads the "currently selected channel" scratch
`D_8008EA26` **roughly ten times** (once before each of ten different
0x34-stride record-family array stores: `D_8008D98A`, `D_8008D996`,
`D_8008D99E`, `D_8008D998`, `D_8008D99A`, `D_8008D990`, `D_8008D992`,
`D_8008D99C`, `D_8008D994`, `D_8008D9A0`, `D_8008D988`) -- and reaches
`D_8008EA26`, plus several OTHER nearby scratch globals it needs in the
same stretch (`D_8008EA0D`, `D_8008EA13`, `D_8008EA18`, `D_8008EA1B`),
through ONE shared base pointer materialized once (`s0 = &D_8008EA24`,
where `D_8008EA26` sits at `s0+2`), via a `lh v1,2(s0)` costing **ONE
word per re-read**.

My C reads `D_8008EA26` directly by its global symbol name at each of
those ten sites (it is declared `volatile u16` file-wide, per this unit's
own established idiom, specifically so each re-read is not elided). This
compiles CORRECTLY in content but at 2-3 WORDS per re-read instead of
retail's one (a fresh `lui`+`lhu` pair, sometimes plus a separate `andi`
mask, rather than one small-offset load off an already-materialized base)
-- this is the single largest contributor to the 15-word gap.

**Two things were tried, both instructive, neither landing as-is:**

- **A bare `__asm__("")` before each of the ten reads**, to try to force
  retail's apparent "always reload, never cache" behavior explicitly:
  this actually made the build WORSE (410 words, +8), because it forced
  a FULL fresh `lui`+`lhu`+`andi` at every site rather than reusing even
  the cheap parts of the address computation -- confirming the
  redundant-reload SHAPE was already correct without the barrier; the
  barrier only worsened the ADDRESSING cost. Reverted.
- **An explicit shared base pointer**, `u8 *base = (u8*)&D_8008EA24;`,
  with every mid-block read rewritten as `*(s16*)(base+2)` (for
  `D_8008EA26`) and similar small-offset casts for the others (matching
  retail's `s0`-relative offsets exactly, e.g. `D_8008EA13` at `base-0x11`,
  `D_8008EA18` at `base-0xC`): this DID reproduce the cheap one-word
  addressing, but overshot dramatically in the other direction -- **335
  words (52 SHORT)**, because it let the compiler treat the pointer
  dereferences as ordinary (non-volatile) memory reads and CSE the
  repeated `idx*52` multiply-chain across MULTIPLE of the ten array
  stores, which retail's disassembly shows it does NOT do (retail
  recomputes the index and the full multiply-by-52 chain independently at
  EVERY one of the ten sites, `sll`/`addu`/`sll`/`addu`/`sll`, even though
  the index value provably cannot have changed between them). **This is
  the opposite failure mode from #1 above and equally informative: the
  cheap ADDRESSING and the redundant, un-cached RE-COMPUTATION are two
  separate properties, and neither of the two mechanical levers tried
  (direct volatile-symbol access, or a plain-pointer shared base) gets
  BOTH right simultaneously.** Reverted to the direct-symbol form (the
  402-word state) as the better-corroborated of the two, since it matches
  retail in CONTENT and ORDER everywhere except addressing cost, while the
  335-word pointer version dropped real content retail has.

### Proposed learning

**A scratch global read through a shared-base-pointer idiom (documented
elsewhere in this project as "several globals a few bytes apart, reached
via one materialized base register") needs BOTH properties reproduced
together -- cheap addressing AND no cross-read CSE of the surrounding
computation -- and this round found no single C-level lever that gets
both at once.** Direct symbol access (declared `volatile`) reproduces the
NO-CSE property but not the cheap addressing (retail folds the address
compute into ONE base register that this project's C has no clean way to
force without ALSO defeating the volatile-style non-caching, per this
round's two tried levers). Suspect a `volatile`-qualified POINTER TYPE for
the shared base (`volatile u8 *base = ...`) might thread this needle --
NOT tried this round; the file's existing note on `D_8008EA26` says a
plain (non-volatile) pointer type still preserves the fold for a
single-read case, but this function's TEN-reads-in-a-row case may need
the pointer itself volatile-qualified to also block the CSE. Worth the
first attempt for whoever picks this back up.

## Round 37 (bravo) update: rebuilt, TWO STALE-SYMBOL BUGS FOUND AND FIXED, permuter searched for the first time

Round 37's designated permuter-priority item 5, lowest priority per the
round's own ordering (largest gap). Per the round's "build the inherited
body before you trust its score" instruction, this exact preserved body
was re-spliced into the live unit and rebuilt -- and this is the round-33
lesson landing for real: **the splice did NOT link clean as written.**

Two undefined references, both stale-symbol bugs, neither previously
caught because this body had apparently never actually been linked since
it was written:

1. **`func_80032148` does not exist under that name any more.** It is
   Sony's own `SpuVmVSetUp` (`code_179d8_c.c`'s own header comment: "round
   32 then found it is Sony's ... see docs/match-reports/func_80032148.md"),
   already declared and called with this exact signature
   (`s32 SpuVmVSetUp(s16 a0, s16 a1)`) by `SpuVmPitchBend` earlier in this
   same file. Fixed by calling `SpuVmVSetUp(a1, a2)` instead.
2. **`D_8008EA0D` is not a real symbol.** The disassembly reads it as
   `lbu $v1, -0x17($s0)` where `$s0 = &D_8008EA24` -- a raw negative
   offset off an already-materialized base, never a `%hi`/`%lo` pair of
   its own, and it is referenced by name in NO other `asm/` file in the
   project (unlike its neighbors `D_8008EA0C`, `D_8008EA1C`..`D_8008EA20`,
   each of which IS referenced elsewhere and so already has a splat-
   generated symbol). Consequently splat never created a linker symbol at
   that address, and `extern u8 D_8008EA0D;` is an undefined reference at
   link time, not a compile error -- so it would never have shown up in
   the round's own compile-error grep, only in the linker-error grep
   (`undefined reference`), which is exactly why this project's build
   recipe checks for that pattern explicitly. Fixed by reading the same
   address through the already-linkable `D_8008EA24` base pointer instead
   of inventing a symbol for it: `*((u8 *) &D_8008EA24 - 0x17)`.

**Neither bug is a regression in the DERIVATION -- the C's structural
content, control flow and field accesses are unaffected -- both are
symbol-naming bugs that could only be caught by actually linking the
body**, which is precisely round 33's lesson this round's brief called out
by name. With both fixed, **the title's length figure reconfirms exactly**:
`objdump -t build/src/code_179d8_m.c.o` shows `StartNote` at `0x648`
bytes = **402 words** (retail 387, 15 words LONG, exactly as titled).

### Permuter search

`tools/setup-permuter.sh StartNote <seed>` -- seed built from this
report's preserved body with both fixes above applied. See the Permuter
result subsection for the base `--debug --stack-diffs` score and the real
search's outcome (iteration count and `rc`). Lowest priority of the five
per this round's own ordering (largest gap, 15 words), so the search here
may be run with a shorter bound than the others if the round's time budget
is tight by the time this function is reached.

#### Permuter result

`--debug --stack-diffs` base score: **15053** (Stack Differences 168 x
weight 1 = 168 -- this one DOES show up, unlike the other four functions
in this unit, consistent with this function's own report identifying a
genuine differing register-SAVE SET at entry, not just an unaddressed
reserved-space gap; Register Differences 117 x 5 = 585; Reorderings 10 x
60 = 600; Insertions 76 x 100 = 7600; Deletions 61 x 100 = 6100; zero
Branch differences) -- by far the largest base score of the unit's five
functions, consistent with this being both the largest word-count gap (15)
and the function with the most struct/global derivation.

Given this is the round's lowest-priority target (largest gap, reached
last) and the time already spent on the other four searches, this one ran
with a **reduced `timeout 600`** (10 minutes) rather than the full 900,
per this report's own note that a shorter bound is acceptable here if the
round's budget is tight. **Completed cleanly, `rc=124`** (own bound) after
**51,596 iterations**. Best score: **12471** (from base 15053), saved at
`permuter-work/StartNote/output-12471-1/`; no zero reached. Given the
base score's scale (15053, an order of magnitude above the other four
functions' bases) and the modest fractional improvement in 51k iterations,
this reads as consistent with the report's own "two residues, ~10-12 words
plus ~2-3 words" diagnosis rather than a single tractable defect -- **not
closed, and a deeper search would need a materially longer bound than this
one to be informative given how little ground 51k iterations covered
relative to the base score's scale.**

### Proposed learning

**A preserved near-miss body's own extern declarations can go stale in TWO
different ways that this project's existing checks catch at different
points, and only one of them is a compile error.** A renamed function
(`func_80032148` -> `SpuVmVSetUp`) still compiles fine (implicit
declaration) but fails at LINK time with `undefined reference` -- exactly
what `tools/stalesyms.py` is for, and exactly the failure class this
round's build-oracle recipe's `undefined reference` grep exists to catch
rather than miss silently. A symbol that was never a real linker label at
all (`D_8008EA0D`, reached in the real disassembly only as a raw offset
from an already-named base pointer, never independently referenced
anywhere else in `asm/`) fails the SAME way (`undefined reference`) for a
different underlying reason -- no other `asm/` file happens to need a name
for that exact address, so splat never minted one. Both present
identically at the build oracle (a clean compile, then a linker error);
distinguishing "renamed" from "never named" only matters for the fix
(reuse a sibling unit's current name, vs. re-derive the byte offset from
a neighboring symbol that IS real) -- and either way, this confirms
`tools/stalesyms.py` (or an equivalent build-before-trust check) needs to
run on EVERY preserved body before permuter time is spent on it, not just
ones that look old.

## Round 48 update (runner echo): tested charlie's frame-padding lever -- THIRD confirmed negative for length closure

Round 48's designated test of charlie's `func_800351D0` frame-padding
discovery, applied here since this function's own title already recorded
an explicit frame-size gap (`0x140` built vs retail's `0x148`). Rebuilt the
round-37 preserved body first (with both its stale-symbol fixes:
`func_80032148` -> `SpuVmVSetUp`, `D_8008EA0D` read through the
`D_8008EA24` base pointer), confirming it still links and reproduces
**402/387 built words**, matching every prior round's figure exactly.

Applied `u8 dead[8];` under the established `if (0) { dead[0] = 0; }`
guard, sized to the measured 8-byte gap (`-0x140` built vs `-0x148`
retail). **Result: frame realigns byte-exactly** (`addiu sp,sp,-0x148`,
confirmed via objdump) **but built length is UNCHANGED at 402/387 (still
15 words LONG).**

This is the THIRD function on this unit this round where the lever
produces the identical shape: exact frame-byte recovery, zero effect on
word count. Unlike `SpuVmFlush` (where the same fix's realignment
surfaced a fresh, fixable `andi` mask), realigning this function's frame
did not surface anything new beyond what this report's own two residues
already diagnose -- the early-materialization scheduling point (~2-3
words) and the `D_8008EA26`-cluster addressing cost (~10-12 words), both
already tried multiple ways and confirmed not to respond to simple
mechanical levers (see "The two residues" above). Given this function
already carries a 51,596-iteration permuter search (round 37, not closed)
on top of that diagnosis, no further manual attempt was made this round --
the frame padding data point was the target, and it reproduces this
round's now-consistent verdict. Reverted to `INCLUDE_ASM`; whole-image
SHA1 reconfirmed green.

### Proposed learning (third data point, same unit)

**Three for three on `code_179d8_m` this round: charlie's `dead[N]`/`if(0)`
frame-padding idiom recovers frame byte-alignment exactly every time it is
applied to a measured frame-size gap, and it has closed a missing-WORD-COUNT
gap ZERO of three times on this unit** (`SpuVmFlush`, `StepVoiceFade`,
`StartNote`) -- including on a function that is overall LONG (this one,
15 words over) as readily as on the two that are SHORT. The common thread
across the unit's three tests: every one of this unit's frame gaps is pure
unaddressed register-save-area padding, with whatever content residue the
function actually has (a redundant mask, a persisted early value, an
addressing-cost difference) living entirely independently of the frame
size. `func_800351D0`'s original length recovery came from a SEPARATE,
coincidentally-discovered tail-duplication fix, not from the padding move
itself -- this unit's evidence says that pairing is not the common case.
**Treat the padding fix as a diagnostic-alignment step to apply cheaply
before reading a diff, not as a length-closing lever in its own right,
unless a SEPARATE piece of evidence (like a duplicated tail) is also
found.**

## Preserved body (best attempt, 402/387 built words -- 15 long, structurally correct throughout except the two residues above)

```c
> **ROUND 39 (head): THIS PRESERVED BODY WILL NOT LINK AS WRITTEN.**
> Rename(s) needed before it builds: `func_80032148` -> `SpuVmVSetUp`.
> The symbol was retargeted when that function became a linked Psy-Q
> SDK object, so the old name no longer exists in this tree. The NAME
> is stale; the residue this body demonstrates usually is not. Correct
> the name and REBUILD before trusting any figure attached to this
> block -- including one quoted in its own heading.
>
> Found by `python3 tools/stalesyms.py`. Note this warning is placed only
> where the stale name appears in CODE: a block whose prose merely
> discusses the rename is fine and is deliberately not marked.

#if 0
/* Base pointer for a table of 0x10-byte slots, same shape as
 * code_179d8_j.c's own SlotE968 local view of the same D_8008E968
 * global -- only the three byte fields this function touches are
 * named, per this project's reduced-local-view convention. */
typedef struct {
    u8 unk0; /* +0x0 */
    u8 unk1; /* +0x1 */
    u8 pad2[0x4 - 0x2];
    u8 unk4; /* +0x4 */
    u8 pad5[0x10 - 0x5];
} SlotE968M;
extern SlotE968M *D_8008E968;

/* A 172 (0xAC)-byte record; D_800902E8 is an array of pointers to
 * arrays of these, indexed [screen][slot]-style by a packed argument
 * (slot in the high byte, screen in the low byte) -- same array
 * code_179d8_j.c/_k.c/_i.c already document, each with its own reduced
 * local view. This function only needs the `unk12` byte-offset field
 * (see code_179d8_k.c's fuller Entry90902E8 for what it points at:
 * `*(s16 *)((u8 *)rec + 0x4E + rec->unk12 * 2)` is a per-voice "speed"
 * table this function also reads, same access shape as that unit's own
 * func_800344FC). */
typedef struct {
    u8 pad0[0x12];
    u8 unk12; /* +0x12 */
    u8 pad13[0xAC - 0x13];
} Entry90902E8M;
extern Entry90902E8M *D_800902E8[];

extern u8 D_8008EA0C;
extern u8 D_8008EA0D;
extern u8 D_8008EA0E;
extern u8 D_8008EA0F;
extern u8 D_8008EA1C;
extern u8 D_8008EA1D;
extern u8 D_8008EA1E;
extern u8 D_8008EA1F;
extern u8 D_8008EA20;
extern u16 D_8008EA24;

extern Rec34Half D_8008D98A[];
extern Rec34Half D_8008D990[];
extern Rec34Byte D_8008D992[];
extern Rec34S16 D_8008D998[];
extern Rec34Half D_8008D9A0[];

extern void func_8002D6A4(void);
extern void func_8002D8E0(s32 a0);
extern s32 note2pitch(void);
extern void func_8002D1B4(s32 a0, u16 a1);
extern u8 StopNote(s16 a0, s16 a1, s16 a2, u16 a3);

/* Called as `StartNote(0x21, p0, p1, p2, outA, outB)` from
 * code_179d8_j.c's SpuVmSeKeyOn and as
 * `StartNote(packed, note, vol, (u8)a3, (u16)divided, status)` from
 * code_179d8_k.c's func_800344FC -- signature confirmed independently
 * by three sibling units' own extern guesses (code_179d8_i/_j/_k all
 * agree on this exact shape). `a0` is a packed [screen | slot<<8]
 * dispatch id into `D_800902E8`; `a1`/`a2` are the "key" values
 * `StopNote`'s own three-field match loop checks; `a4`/`a5` are
 * 7-bit-percentage volume/pan bytes staged into the same
 * D_8008EA10/D_8008EA11 scratch globals the interpolation-setup
 * functions elsewhere in this unit use. */
s32 StartNote(s32 a0, s16 a1, s16 a2, u16 a3, u16 a4, u16 a5)
{
    Entry90902E8M *s6;
    SlotE968M *slot;
    s32 s3;
    s32 chan;
    u8 matchCount;
    u8 chanScan;
    s16 origA2;
    s32 shifted;
    s16 a0s16;
    u8 byte0;
    u8 byte1;
    u8 idBuf[0x80];
    u8 chanBuf[0x80];

    origA2 = a2;
    byte0 = (u8) a0;
    shifted = a0 << 16;
    a0s16 = (s16) (shifted >> 16);
    byte1 = (u32) shifted >> 24;
    s6 = &D_800902E8[byte0][byte1];

    if (func_80032148(a1, a2) != 0) {
        return -1;
    }

    slot = &D_8008E968[a2];
    D_8008EA22 = (s16) a0;
    D_8008EA0E = (u8) a3;
    D_8008EA0F = 0;
    D_8008EA10 = (u8) a4;
    D_8008EA11 = (u8) a5;
    D_8008EA16 = slot->unk1;
    D_8008EA17 = slot->unk4;
    D_8008EA0C = slot->unk0;

    if ((u32) D_8008EA13 >= D_8008E970->unk12) {
        return -1;
    }

    s3 = 0;
    if (a4 != 0) {
        matchCount = 0;
        for (chanScan = 0; chanScan < D_8008EA0C; chanScan++) {
            Tbl32E978 *entry = &D_8008E978[D_8008EA13 * 16 + chanScan];

            if (D_8008EA0E < entry->unkC) {
                continue;
            }
            if (entry->unkD < D_8008EA0E) {
                continue;
            }
            idBuf[matchCount] = entry->unk16;
            chanBuf[matchCount] = chanScan;
            matchCount++;
        }

        if (matchCount != 0) {
            s32 s2;
            u8 s1;

            s2 = a4 * 127;
            for (s1 = 0; s1 < matchCount; s1++) {
                Tbl32E978 *entry2;

                D_8008EA24 = idBuf[s1];
                D_8008EA18 = chanBuf[s1];

                entry2 = &D_8008E978[D_8008EA13 * 16 + D_8008EA18];
                D_8008EA1B = entry2->unk0;
                D_8008EA19 = entry2->unk2;
                D_8008EA1A = entry2->unk3;
                D_8008EA1C = entry2->unk4;
                D_8008EA1D = entry2->unk5;
                D_8008EA20 = entry2->unk1;
                D_8008EA1E = entry2->unk6;
                D_8008EA1F = entry2->unk7;

                chan = func_8002CF18(0) & 0xFF;
                D_8008EA26 = chan;
                if (chan < D_8008E9D0) {
                    D_8008D9A3[chan].unk0 = 1;
                    D_8008D98A[D_8008EA26].unk0 = 0;
                    D_8008D996[D_8008EA26].unk0 = (s16) a0;
                    D_8008D99E[D_8008EA26].unk0 = D_8008EA0D;
                    D_8008D998[D_8008EA26].unk0 = D_8008EA13;
                    D_8008D99A[D_8008EA26].unk0 = origA2;

                    if ((s16) a0 != 0x21) {
                        s16 speed = *(s16 *) ((u8 *) s6 + 0x4E + s6->unk12 * 2);

                        D_8008D990[D_8008EA26].unk0 = s2 / speed;
                    }

                    D_8008D992[D_8008EA26].unk0 = (u8) a5;
                    D_8008D99C[D_8008EA26].unk0 = D_8008EA18;
                    D_8008D994[D_8008EA26].unk0 = a3;
                    D_8008D9A0[D_8008EA26].unk0 = D_8008EA1B;
                    D_8008D988[D_8008EA26].unk0 = D_8008EA24;

                    func_8002D6A4();
                    if (D_8008EA24 == 0xFF) {
                        func_8002D8E0(*(u8 *) &D_8008EA26);
                    } else {
                        func_8002D1B4(matchCount, note2pitch());
                    }
                    s3 = (s3 << 4) | D_8008EA26;
                }
            }
        }
    } else {
        StopNote(a0s16, a1, a2, a3);
    }

    return s3;
}
#endif
```

Note: `ObjE970` gained a `+0x12` `u16` field for this attempt (see
"Struct/global knowledge" above) -- that extension is left in place in
`src/` (outside the `#if 0`) since it does not affect any currently
compiled function and the next attempt at this function, or at
`StepVoiceEnvelope`/`StepVoiceFade` (which also use `D_8008E970`), will need
it again.

## Naming

**StartNote** (was `func_8002FAC4`) -- Tier A. Signature and role
corroborated independently by three sibling units (`code_179d8_i.c`,
`code_179d8_j.c`, `code_179d8_k.c`, per this report's own "Signature"
section) before any body-level derivation: `code_179d8_k.c`'s
`func_800344FC` calls this in its nonzero-velocity branch and StopNote in
its zero-velocity branch of the SAME MIDI-status-byte switch, and
`code_179d8_j.c` wraps both with the same fixed leading identity constant
(`0x21`) -- a clean NoteOn/NoteOff symmetry, which is the primary evidence
for "Note" rather than the more mechanical "RegisterActiveChannel" this
report's own prose used while deriving it. Registers a new active-voice
record for the given (packed screen/slot identity, note, program/volume,
velocity, pan-split pair, status).

## Proposed field names

`D_8008E970`/`ObjE970` (already locally typed with `difficultyThreshold`/
`masterVolume` field names this round) and `D_8008E978`/`Tbl32E978`
(`bendCurveUp`/`bendCurveDown`, others still `unk0`..`unk7`/`unk16`) are
declared in this unit but only used by functions still `INCLUDE_ASM`
(StepVoiceEnvelope, StepVoiceFade, SpuVmPBVoice, and this
function) -- the field renames are live in `src/code_179d8_m.c` now (pure
documentation, nothing compiled references them yet); the base symbols
themselves (`D_8008E970`, `D_8008E978`) were not renamed since
`D_8008E978` is shared with bravo's live `code_179d8_j_b.c` this round
(see broadcast).

## NON_MATCHING body promoted, round 67

Placed in `src/code_179d8_m.c` under `#ifdef NON_MATCHING`, `INCLUDE_ASM`
kept in `#else`. Applied both stale-symbol fixes this report's round-37
update already diagnosed but the preserved `#if 0` text above still shows
literally: `func_80032148(a1, a2)` -> `SpuVmVSetUp(a1, a2)` (already
declared earlier in the unit, called the same way by
`SpuVmPitchBend`), and `D_8008EA0D` (no linker symbol of its
own) -> `*((u8 *) &D_8008EA24 - 0x17)`. Also dropped four locally-redundant
declarations that collide with types the unit's shared prelude already
established under different typedef names for the same symbols
(`D_8008D98A`, `D_8008D990` already `Rec34Half`; `D_8008D992` already
`Rec34ByteEdd4`; `D_8008D998` already `Rec34Half` where this body's own
copy said `Rec34S16` -- same `unk0` field either way, so no access-site
change needed) and renamed two field accesses to the names a later
naming pass gave the same offsets: `D_8008E970->unk12` ->
`->difficultyThreshold`, `entry->unkC`/`unkD` -> `->bendCurveUp`/
`->bendCurveDown` (by OFFSET, not by the field's on-disk polarity --
this function's usage doesn't depend on which direction the name
implies). `./build-and-verify.sh` green (zero bytes changed);
`tools/check-nonmatching.sh code_179d8_m` green; `tools/stalesyms.py`
shows no stale references left in `src/code_179d8_m.c` (only the report's
own preserved-block text still carries the old name, expected and
harmless).
