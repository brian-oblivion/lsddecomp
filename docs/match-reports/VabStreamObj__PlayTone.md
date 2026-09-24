# VabStreamObj__PlayTone -- MATCHED (round 75: load the program pointer before computing `lo`, so `hi`'s first use is not the multiply)

REVISITED, round 75: MATCHED (55/55, byte-exact, whole-image oracle `OK: build matches retail`); names/types not relevant (the round-52 names and the unit's existing `SsUtKeyOn` extern were used unchanged).

## Round 75 (runner bravo)

**First measurement, before changing anything.** The round-73 NON_MATCHING
body (`hi = index / 16; lo = index - hi * 16;`) compiled live in place of the
`INCLUDE_ASM`: `5/55 words match`, `insertions 6 / deletions 6 (opcode-level;
positional skeleton diffs 44)`, three words short, frame 0x38 vs 0x30, and
297063 bytes of drift outside the range. So the old figure was misalignment
from a length difference, as suspected, not a register residue.

**Steps, one build each:**

| body | score | ins/del | note |
| --- | --- | --- | --- |
| round-73 body (`/16`, `lo = index - hi*16`) | 5/55 | 6/6 | `andi` fold, 3 words short |
| head's round-17 narrowing `lo = index - (s16)hi * 16` | 46/55 | 1/1 | length exact; `sll 4` fed from the truncated `hi`, v0/v1 swapped |
| `hi = index; hi >>= 4;` / `lo = index; lo -= hi<<4;` / `lo = index - (hi = index>>4)*16` / `lo = index; lo -= hi*16` / `index + hi * -16` | 5/55 | 6/6 | all fold |
| `lo = index - (s16)(hi * 16)` | 10/55 | 2/2 | extra sext |
| `__asm__("")` (and `__volatile__`) between `hi` and `lo` | 5/55 | 6/6 | barrier does NOT block the fold |
| `hi = index >> 4;` hoisted ABOVE the guard | 44/55 | 1/1 | fold blocked, untruncated `sll 4` exactly as retail, but `sra` lands before `bltz` |
| redundant inner `if (hi >= 0)` | 10/55 | 1/1 | retail's hi/lo sequence exactly, plus one extra `bltz` |
| `entry = &tab[hi][index - hi*16]; lo = index - hi*16;` | 49/55 | 0/0 | 6 register-identity diffs |
| **`prog = self->progVagTable[hi]; lo = index - hi*16; entry = &prog[lo];`** | **55/55** | 0/0 | **MATCH** |

**The mechanism (why the lever works).** The fold `index - (index>>4)*16 ->
index & 0xF` is done by COMBINE (three-insn: `sra` -> `sll 4` -> `subu`), and
combine can only find the `sra` through the `LOG_LINKS` of the insn that uses
it. GCC 2.6.3's `flow.c` builds a `LOG_LINK` (a) only within one basic block
and (b) only from the NEXT use of a register after its set. Two measured
consequences:

- Put `hi`'s set in a different block from the multiply (hoist it above the
  guard, or add a redundant branch between them) and the fold disappears, with
  the `sll 4` still fed from the untruncated `hi`, which is retail's exact
  sequence.
- Keep everything in one block but make `hi`'s FIRST use something else (here
  the `hi * 4` of the program-pointer load) and the multiply gets no link to
  the `sra`. The fold disappears with no extra code, and this is retail.

The head's round-17 `(s16)hi * 16` worked for the same reason: the first use
of `hi` became the sign extension. It also changed the operand, which is why
it left one word. Round 17's "seven probes" all put the multiply first, which
is why every one of them folded. The `__asm__("")` barrier does not block the
fold, so it is not a volatile-insn check that stops combine here.

**SsUtKeyOn extern: observed, NOT changed, because this function did not need
it.** The unit declares `extern s16 SsUtKeyOn(s16, s16, s16, s16, s32, s32,
s32);`. `include/psyq/LIBSND.H` has `short SsUtKeyOn(short x7)`, and the
`#ifdef NON_MATCHING` definition in `src/code_179d8_j_b.c` reads it as `s32
SsUtKeyOn(s16, s16, s16, s16, u16, s16, s16)`. That definition is a stall
(65/252), not a byte-exact one. Callers checked: `grep -rln "jal *SsUtKeyOn$"
asm/` finds exactly one, this function, and `grep` in `src/` finds no other C
call. At this call site the disagreement is byte-invisible: the 5th argument
is a `u8` (`lbu`, stored straight to `0x10($sp)` under either `s32` or
`short`); the 6th and 7th are already `(s16)` casts; and the `s16` return
type is what produces retail's `sll`/`sra` on `$v0` after the `jal`. The unit
matched with the extern as it stands, so I left it alone.

**Budget:** 18 builds in all, no permuter search. The match came before any
Gate 3 decision was needed.

### Proposed learning

**The `x - (x >> k) * 2^k -> x & (2^k-1)` fold is a combine fold, and it
depends on where the shifted value is FIRST used.** flow.c links a register's
set only to its next use in the same basic block. If the multiply is the first
use of `hi = x >> k`, combine folds it to `andi` and the function comes out
3 instructions short. If anything else uses `hi` first (an array index
`tab[hi]`, a cast `(s16)hi`), or the set is in another block, the retail-style
`sll`+`subu` survives, fed from the untruncated `hi`. **The tell:** retail has
`sra; ...; sll k; subu` where your build has `andi 0xf` straight off the
original register. **The lever:** reorder the statements so that some other
use of `hi` comes before `lo = x - hi * 2^k`. A bare `__asm__("")` between the
two does NOT block it. This generalises the round-74 goto-loop lesson:
several "compiler residues" turn out to be pass-scope effects (loop notes,
basic blocks, first-use links) that source order controls.

---

## History (round 17 - round 73), kept for the derivation

Original title: VabStreamObj__PlayTone -- STALL (best: 5/55 words match, but see caveat below)

> Renamed from `func_8002CA3C` on 2026-09-18 (tools/rename.py). Address 0x8002ca3c.

Unit: `code_179d8_e`. Runner: echo, round 17. Restored to `INCLUDE_ASM`.

## Class: compiler-behavior residue (NOT gp_rel, NOT addiu_at) -- reproduced in isolation

Screened clean on both documented blockers (no `gp_rel`, no `addiu $at,$at,%lo`
hits in the `.s`). This is a THIRD, previously undocumented residue class,
confirmed with a from-scratch minimal reproducer through the pinned pipeline
(see below) -- not merely suspected.

## What the function does (high confidence, established via m2c + manual read)

`gVabStreamObjMethods`'s vtable slot +0x080. Signature:
`s32 VabStreamObj__PlayTone(VabStreamObj *self, s32 index, s32 arg2, s32 arg3)`.

```c
if (index >= 0) {
    hi = index >> 4;                      /* array-of-pointers index */
    lo = index - hi * 16;                 /* sub-index within that chunk */
    entry = &self->progVagTable[hi][lo];  /* VagAtrView *, 0x20-byte stride */
    result = SsUtKeyOn(self->vabId, (s16)hi, (s16)lo,
                            (s16)(entry->center + self->pitchOffset), entry->shift,
                            (s16)arg2, (s16)arg2);   /* NOTE: arg2 passed TWICE */
    if (result >= 0) {
        SsUtAutoVol(result, (s16)arg2, (s16)arg3, 2);
        return result;
    }
}
return -1;
```

(field/type names in this preview updated to round 52's renames, same as
the preserved body below -- `unk50/unk54/unk60/unk4/unk5` were the field
names m2c/the original derivation actually saw.)

This structure (control flow, argument counts -- including the double-`arg2`
7th argument to `SsUtKeyOn`, which m2c independently confirmed -- field
offsets, and the `self->unk50[hi][lo]` addressing) is corroborated by
`.venv/bin/python3 tools/m2ctx.py code_179d8_e --sig 's32
VabStreamObj__PlayTone(ObjDA34 *self, s32 index, s32 arg2, s32 arg3)' --run`, which
produces the same shape independently from the raw instructions. I am
confident this part is right.

## The residue: retail does NOT fold `index - (index>>4)*16` into `index&0xF`

Retail's `hi`/`lo` split:

```
sra   v0, a0, 0x4      ; hi = index >> 4  (bare shift, no bias correction)
sll   a1, v0, 0x10
sra   a1, a1, 0x10     ; hi16 = (s16)hi
sll   v1, v0, 0x4      ; v1 = hi << 4
subu  v1, a0, v1       ; lo = index - hi*16   <-- UNFOLDED subtraction
sll   a2, v1, 0x10
sra   a2, a2, 0x10     ; lo16 = (s16)lo
sll   v0, v0, 0x2      ; hi*4  (array-of-pointers stride)
sll   v1, v1, 0x5      ; lo*32 (Chunk179D8E stride)
```

Every C form tried compiles the `lo` computation to a single `andi $t,$t,0xf`
instead, using the ORIGINAL `index` register directly (bypassing `hi`
entirely) -- and correspondingly loses the two `sll`/`sra` "widen hi/lo back
to a full register" pairs and the `subu`, an instruction-count deficit large
enough to shift every following instruction and drag the whole function's
score down to noise (5/55, plus an out-of-range byte-count warning).

**This is not a source-shape guess -- it reproduces in isolation, unconditionally,
across seven independent minimal probes through the exact pinned pipeline**
(`tools/gcc263/cpp` -> `cc1 -O2` -> `maspsx --aspsx-version=2.34
--dont-force-G0 --expand-div` -> `mipsel-linux-gnu-as`):

1. `hi = x>>4; lo = x - hi*16;` (plain, no guard) -> folds to `andi`.
2. Same, with an s16-typed "hi16" copy computed immediately after hi (mimicking
   retail's instruction order) -> still folds.
3. Same, wrapped in `if (x>=0) { ... }` (matching CA3C's own negative-index
   guard) -> hi's bias-check DOES disappear (matches retail's bare `sra`!) but
   `lo` STILL folds to `andi`.
4. `hi = x/16; lo = x - hi*16;` with NO guard -> hi gets bias-corrected
   (`bgez`/`addiu` before the `sra`, which retail does NOT have) but `lo`
   stays UNFOLDED (`subu`) -- the only variant that preserved the `subu`.
5. Same division form WITH the `if (x>=0)` guard (i.e., CA3C's actual shape)
   -> the guard eliminates the bias-check (bare `sra`, matching retail) but
   ALSO re-enables the `andi` fold for `lo` -- the two "range now known"
   simplifications are evidently the SAME optimizer decision, not two
   independent ones, so they cannot be pried apart by choosing `/` vs `>>`.
6. A bare `__asm__("")` scheduling barrier between computing `hi` and `lo`
   does not block the fold (it happens at a tree/RTL level before instruction
   scheduling, which the barrier constrains).
7. A full-shape probe (struct self, real field offsets, two helper calls,
   matching register pressure) with `/`-based `hi` inside the guard -- gets
   EVERY OTHER INSTRUCTION IN THE FUNCTION BYTE-IDENTICAL to retail (bare
   `sra` for hi, matching `lh`/`lw`/`lbu`/`sw` sequence, matching stack
   layout) except this one `andi` vs `sll`+`subu` substitution. This is the
   strongest evidence the rest of my reconstruction is correct and this is
   an isolated, load-bearing residue.

Reproducer (probe 5/7, the closest, saved for anyone re-verifying):

```c
extern int helper(short hi, short lo);
int probe(int x) {
    int hi, lo;
    if (x >= 0) {
        hi = x / 16;
        lo = x - hi * 16;
        return helper((short)hi, (short)lo) + hi*4 + lo*32;
    }
    return -1;
}
```

```sh
tools/gcc263/cpp -Iinclude -Iinclude/psyq -undef -lang-c -nostdinc -Dmips -D__GNUC__=2 /tmp/probe5.c \
  | tools/gcc263/cc1 -mips1 -mcpu=3000 -quiet -G0 -O2 \
  | .venv/bin/python3 tools/maspsx/maspsx.py --aspsx-version=2.34 --dont-force-G0 --expand-div \
  | tools/binutils/bin/mipsel-linux-gnu-as -march=r3000 -EL -no-pad-sections -G0 -o /tmp/probe5.o
tools/binutils/bin/mipsel-linux-gnu-objdump -d /tmp/probe5.o
```
Result: `andi s1,a0,0xf` where retail-equivalent code would need `sll`+`subu`.

## What this means

GCC 2.6.3 `-O2`, once it can prove (via an earlier `>= 0` / `< 0` guard) that
the dividend is non-negative, appears to ALWAYS simplify
`x - (x/16 or x>>4)*16` into `x & 0xF` -- there is no plain-C spelling of
"divide by 16 and take the remainder, guarded by a prior non-negativity
check" that keeps the unfolded subtraction. Every reshaping only moves WHERE
this happens, never whether it happens. This is a genuinely new residue class,
distinct from `gp_rel` and `addiu_at` -- worth a name if it recurs
(tentatively: the "shift-remainder blocker" or "div-mod CSE fold").

**Scope not yet measured.** I have not run a corpus census (unlike the two
documented blockers, which have `docs/research/*.md` write-ups with censuses).
This report is the first sighting; per CLAUDE.md's standard ("a minimal
reproducer that FAILS to reproduce is itself the result" -- here it DOES
reproduce, repeatedly, which is the mirror image finding), it should be
escalated for review rather than re-attempted by the next runner without new
information. **Do not re-stage this function until someone either (a) finds a
source form none of the seven probes tried, or (b) confirms via corpus census
that this is systemic and worth a `docs/research/*.md` writeup of its own.**

## Attempts

7 build-verified attempts against the real function (all logged above as
probes 1-3 and 5 applied directly, plus the two-var/`hi*16` explicit-multiply
variants not separately listed), all on the SAME axis (how to spell the
hi/lo split): plain shift, explicit multiply instead of shift, `%` operator,
division operator, with and without an intermediate s16 copy, with and
without a scheduling barrier. **The untested axis is whether the object
layout or argument count is subtly wrong in a way that would produce a
DIFFERENT arithmetic relationship between hi and lo than "index/16,
index%16" -- I did not find any instruction evidence for that, but it is
the one axis all 7 attempts share.**

## Preserved best-effort body

```c
#if 0
s32 VabStreamObj__PlayTone(VabStreamObj *self, s32 index, s32 arg2, s32 arg3) {
    s32 hi;
    s32 lo;
    VagAtrView *entry;
    s16 result;

    if (index >= 0) {
        hi = index / 16;
        lo = index - hi * 16;
        entry = &self->progVagTable[hi][lo];
        result = SsUtKeyOn(self->vabId, (s16)hi, (s16)lo, (s16)(entry->center + self->pitchOffset),
                                entry->shift, (s16)arg2, (s16)arg2);
        if (result >= 0) {
            SsUtAutoVol(result, (s16)arg2, (s16)arg3, 2);
            return result;
        }
    }
    return -1;
}
#endif
```

(Updated to round 52's names: `ObjDA34`/`Chunk179D8E` are now
`VabStreamObj`/`VagAtrView`; `self->unk50/unk54/unk60` are now
`progVagTable`/`vabId`/`pitchOffset`; `entry->unk4/unk5` are now
`entry->center/shift` -- confirmed round 52 against Sony's real `VagAtr`
in `include/psyq/LIBSND.H`, where they land on the struct's own
same-named bytes. Bytes/derivation unchanged.)

Needs, from this unit's top-of-file scaffolding: `VabStreamObj`,
`VagAtrView`, `extern s16 SsUtKeyOn(s16, s16, s16, s16, s32, s32,
s32);`, `extern void SsUtAutoVol(s16, s16, s16, s32);` -- all already
present in `src/code_179d8_e.c`.

### Proposed learning

GCC 2.6.3 `-O2` in this pinned toolchain unconditionally folds
`x - (x/N or x>>k)*N` (N a power of two) into `x & (N-1)` whenever it can
independently prove `x >= 0` -- and it can prove that from an ordinary
preceding `if (x < 0) return ...;` guard, the single most natural way to
write this kind of bounded index-splitting code. This fold could not be
avoided by choosing `/` vs `>>`, by an intermediate narrower type, or by a
scheduling barrier in 7 tried variants. If retail code elsewhere splits an
index into `hi = idx>>k; lo = idx - hi*(1<<k)` (or equivalent) *guarded by a
non-negativity check just like this one*, screen for this residue shape
(`andi` in your build where retail has `sll`+`subu`, with hi's bias-check
also missing on both sides) before spending attempts reshaping the C -- it
reproduces in isolation and is very unlikely to be source-shape-fixable.

---

## HEAD ADJUDICATION (round 17) — the class is WRONG; this is not a blocker

Runner echo classified this as a possible third toolchain blocker on the
strength of seven probes, all of which folded. **The head re-ran the probe
matrix and the fold is NOT unconditional.** Reclassified: **source-shape
residue, one operand short of the reproduced sequence.**

### Why the "new blocker" reading could not have been right

Retail came out of THIS compiler. A construct that the pinned pipeline
cannot emit is a blocker; a construct it *does* emit from some other source
form is a shape problem. Before accepting "no C reaches this", the shape
space has to be exhausted, and the seven probes all varied the same axis —
`/` vs `>>`, guard vs no guard, barrier vs none — while holding the TYPE of
`hi` fixed at `s32`. Type is the axis that matters.

### The lever: narrow the multiplicand

The fold `index - (index>>4)*16` -> `index & 0xF` fires only while GCC 2.6.3
can prove the multiplicand equals `index >> 4`. Narrowing it to 16 bits
breaks that proof and the fold does not happen:

```c
extern int sink(int,int,int,int);
struct S { int *p[8]; };
int probe(struct S *self, int index) {
    int hi, lo;
    if (index < 0) return -1;
    hi  = index >> 4;
    lo  = index - (short)hi * 16;          /* <-- the narrowing */
    return sink((short)hi, (short)lo, (int)self->p[hi], lo);
}
```

Through the pinned pipeline (`cpp` -> `cc1 -mips1 -mcpu=3000 -G0 -O2` ->
`maspsx --aspsx-version=2.34 --dont-force-G0 --expand-div` -> `as`):

```
bltz  a1, ...
sra   v0, a1, 0x4       hi, bare arithmetic shift, NO bias correction
sll   a0, v0, 0x10
sra   a0, a0, 0x10      (s16)hi
sll   a3, a0, 0x4       hi16 << 4
subu  a3, a1, a3        lo -- UNFOLDED, the subu retail has
sll   a1, a3, 0x10
sra   a1, a1, 0x10      (s16)lo
sll   v0, v0, 0x2       hi * 4, off the UNTRUNCATED int hi
lw    a2, 0(v0)
```

Compare retail (`asm/nonmatchings/code_179d8_e/VabStreamObj__PlayTone.s`, 0x8002CA64
onward):

```
sra   $v0, $a0, 4
sll   $a1, $v0, 16
sra   $a1, $a1, 16
sll   $v1, $v0, 4        <-- retail shifts $v0 (untruncated)
subu  $v1, $a0, $v1
sll   $a2, $v1, 16
sra   $a2, $a2, 16
sll   $v0, $v0, 2
sll   $v1, $v1, 5
```

**Every structural element matches**: the bare `sra` for `hi`, both s16
widen pairs, the `sll`+`subu` for `lo`, and the `sll 2` on the untruncated
`hi` for the array-of-pointers index. The instruction COUNT is right, which
is what the 5/55 score was really measuring — that score came from three
missing instructions cascading into total drift, not from a wrong body.

### What is actually left

ONE operand. Retail feeds the `sll ..,0x4` from the **untruncated** `$v0`;
the probe above feeds it from the truncated copy. Semantically identical
whenever `hi` fits in 16 bits, so both are legal codegen for the same
source — but no shape tried yet gets the fold blocked AND the multiply on
the untruncated value at once. `(short)hi * 16` blocks the fold by
narrowing the multiplicand, which is exactly the operand that then appears
in the shift.

So the remaining question is narrow and well posed: **what source form
denies GCC the `index >> 4` identity without narrowing the value that
feeds the multiply?** Candidates not yet tried include routing `hi` through
a struct field or a second function's return value, and any form where the
divisor is not a compile-time 16 at the point of the subtraction.

### Disposition

**Not a toolchain escalation. Do not file it as one.** It is a live
matching problem with the shape already reproduced, and it is a good
permuter target — the permuter mutates C source under the pinned toolchain,
which is precisely the search the seven hand-probes were doing one at a
time. Re-staff it with the probe above as the starting body.

Everything echo established about the FUNCTION stands unchanged and was not
re-derived here: the signature, the `self->progVagTable[hi][lo]` addressing
(named `unk50` at the time) with 0x20-byte stride, the `s16 vabId` /
`s32 pitchOffset` field widths (named `unk54`/`unk60` at the time), and the
double-`arg2` seventh argument to `SsUtKeyOn` (visible in retail as
`sw $s1, 0x14($sp)` and `sw $s1, 0x18($sp)` from one sign-extension).

## Naming

Renamed `func_8002CA3C` -> `VabStreamObj__PlayTone`, tier B. Confirmed
`gVabStreamObjMethods`'s own +0x080 slot. Named from the mechanics already
fully derived above, not attempted this round (still `INCLUDE_ASM`, as
instructed): the function resolves a packed `index` into a program/tone
pair (`hi`/`lo`, exactly the `(vabId, prog, tone)` triple `SsUtGetVagAtr`
uses elsewhere in this unit), looks up the matching `VagAtrView` entry,
adds `self->pitchOffset` to its `center` note, and dispatches
`SsUtKeyOn(vabId, hi, lo, pitch, shift, arg2, arg2)` -- a "start
playing this program/tone" call whose result (a voice/handle number) is
then registered via `SsUtAutoVol`. `VabStreamObj__StopVoice`'s own
"index < 0x18" guard (0x18 == 24, the PS1 SPU's own voice count) is the
other half of this same "select a tone/voice" vocabulary, which is why
"PlayTone" rather than a more generic "Dispatch" or "Trigger" name --
mechanics are concrete enough to earn tier B, but the exact GAME-level
event that calls this (a footstep, an ambient loop, dialogue) is not
established from this unit alone, so not tier A.

## Track 1b (round 73)

Promoted the "Preserved best-effort body" above (the `hi = index / 16;
lo = index - hi * 16;` form, 5/55 words, byte-drift outside range) into
`src/code_179d8_e.c` under `#ifdef NON_MATCHING ... #else INCLUDE_ASM ...
#endif`, per docs/FINISHING-PLAN.md track 1b. This is the body actually
verified against the real function in this unit; the HEAD ADJUDICATION's
narrowing probe above it is a generic reproducer that was never re-applied
to `VabStreamObj__PlayTone` itself, so it is not a promotable body yet.
Hand-derived (m2c-assisted, manually read, seven build-verified probes
against the real function) -- not a permuter candidate. `./build-and-verify.sh`
stayed green (no bytes changed) and `tools/check-nonmatching.sh` passed.

NON_MATCHING body promoted, round 73.
