# TodActor__CreateParts -- MATCHED 68/68 (round 75): lever = store-and-increment in one expression, `if ((*p++ = New_Actor()) == NULL) goto fail;` (was STALL: length EXACT 68/68, 49/68 raw, first diff vram 0x80065E20)

> Renamed from `Class65650__CreateParts` on 2026-09-26 (tools/rename.py). Address 0x80065e1c.

> Renamed from `func_80065E1C` on 2026-09-24 (tools/rename.py). Address 0x80065e1c.

**Round 75: MATCHED — see the round 75 section at the end. Everything
between here and that section is the stall history, kept because its
"register-identity, rule 6" classification was WRONG and the record of why
it looked right is the useful part.**

**Unit:** TodActor · **Size:** 68 words (0x110 bytes) · **Status (pre-round-75):** STALL —
**LENGTH exact (68/68 words, no drift); RAW WORD-MATCH 49/68; FIRST REAL DIFF
at file 0x056620 / vram 0x80065E20** (the prologue's `self`<->`p`
register-identity swap, `$s1`/`$s2`). Whole-image red. Restored to
`INCLUDE_ASM`.

## What it does (fully derived, control flow and every field/slot confirmed)

The `+0x70`/`+0x74` array pair's setup body (dispatched through
`slot_setup70`, already known from `TodActor__SetupParts`'s report; the existing
`extern s32 TodActor__CreateParts(TodActor *self);` prototype already had the
right signature). Queries a count from `self->unk5C`'s own vtable slot
`+0x080` (new slot, `Unk5CMethods::slot80`), allocates the two parallel
arrays, re-queries the same slot to populate `self->unk74` with real data,
then constructs each `self->unk70[i]` via a fixed allocator function,
finally priming `self->unk68` by indexing `self->unk70` with a value the
FIRST `slot80` call wrote into a small stack scratch buffer:

```c
s32 TodActor__CreateParts(TodActor *self)
{
    s32 buf[4];
    s32 count;
    s32 i;
    Unk70ElemObj **p;

    count = self->unk5C->methods->slot80(self->unk5C, NULL, buf) & 0xFF;
    self->unk70 = BMemPMgrAlloc(count * 4);
    if (self->unk70 == NULL) {
        goto alloc_fail;
    }
    self->unk74 = BMemPMgrAlloc(count);
    if (self->unk74 == NULL) {
        goto alloc_fail;
    }
    self->unk5C->methods->slot80(self->unk5C, self->unk74, buf);

    p = self->unk70;
    i = 0;
    self->unk6C = 0;
    if (count != 0) {
        do {
            *p = New_Actor();
            if (*p == NULL) {
                goto fail;
            }
            p++;
            self->unk6C = self->unk6C + 1;
            i++;
        } while (i < count);
    }
    self->unk68 = self->unk70[buf[0]];
    return 0;

alloc_fail:
    self->unk74 = NULL;
fail:
    TodActor__DestroyParts(self);
    return 1;
}
```

New types/fields, all confirmed by byte-identical surrounding code:
`Unk5CMethods::slot80` (`s32 (*)(Unk5CObj *, void *, s32 *)`, called twice
with different second arguments — `NULL` to probe, `self->unk74` to
populate), and `New_Actor` (`extern void *New_Actor(void);` — a
plain function, not a vtable dispatch, called with literally no arguments
set up in `$a0`). The `goto`-based control flow mirrors the disassembly
exactly: two allocation-failure paths converge on a shared
`self->unk74 = NULL;` before falling into the teardown call
(`TodActor__DestroyParts`, matched earlier this round) shared with the per-element
allocation-failure path.

Every word of the setup/query/allocation section (roughly the first 17
words) and the tail-call teardown sequence matched immediately. The
residue is entirely inside the per-element construction loop and its
aftermath.

## The residue: two separate issues, one fully closed, one a register-identity swap

### 1. Store-before-branch ordering (CLOSED with a barrier)

A straightforward `*p = New_Actor(); if (*p == NULL) goto fail; p++;`
compiled with the store folded directly into the `beqz`'s delay slot
(replacing `p++`, which then migrated to the loop's `bnez` delay slot
instead) — one instruction SHORTER than retail, which has a standalone
`sw $v0, 0($s1)` immediately after the call, THEN the `beqz` with `p++`
in ITS delay slot (two separate instructions where the naive translation
only has one). A bare `__asm__("")` between the store and the null check
closes this exactly: 35/68 (with a full-image size drift) -> 49/68 with
**no drift**, confirmed via the missing WARNING line in `funcdiff.py`'s
output, not just the printed score.

Tried and rejected: a named `result` temp instead of `*p` directly (no
effect, same 35/68-with-drift baseline); the barrier moved to right after
`New_Actor()`'s call instead of after the store (worse, 26/68 with
drift); the barrier at the very top of the function instead of inside the
loop (worse, 35/68 with drift — does not generalize to fixing this
specific merge); replacing the pointer-walk (`*p`, `p++`) with array
indexing (`p[i]`) (worse, 44/68).

### 2. `self` and the array-walk pointer land in SWAPPED physical registers (open)

With the barrier above in place, the ENTIRE rest of the function
(register `s1` vs `s2`, consistently swapped in every single place both
registers appear — the prologue's save order, `self->unk5C` reloads,
`self->unk70`/`self->unk74` stores, the loop's `self->unk6C` bookkeeping,
and the tail's `self->unk68` store and the teardown call's `$a0`) uses
`$s1` for `self` and `$s2` for the array-walk pointer `p`, while retail
uses `$s2` for `self` and `$s1` for `p` — the exact opposite assignment.
Every one of these is otherwise identical (same offsets, same
instructions, same order) once the swap is accounted for; this is a pure
register-IDENTITY difference; per CLAUDE.md rule 6, that is a STALL, not
something fixable with `register T v asm("$N")` or an operand constraint
(both banned).

One more, smaller residue rides along with the swap: at word 42
(`566c4`), retail schedules `addiu $s0,$s0,1` (the `i++`) right after the
loop-count reload, but this build's delay-slot filling puts it one
position earlier (in the `beqz`'s delay slot instead of `p++`/`addiu
$s0,$s0,1`'s natural position) — a knock-on scheduling difference from
the same register swap, not a separate cause.

Tried and rejected as a fix for the swap specifically: reordering the
local declarations (`p` before vs. after `buf`/`count`/`i` — no change,
same 49/68 either way, before OR after the barrier fix was in place).

## Preserved body (best attempt, 49/68, no size drift)

```c
#if 0
s32 TodActor__CreateParts(TodActor *self)
{
    s32 buf[4];
    s32 count;
    s32 i;
    Unk70ElemObj **p;

    count = self->unk5C->methods->slot80(self->unk5C, NULL, buf) & 0xFF;
    self->unk70 = BMemPMgrAlloc(count * 4);
    if (self->unk70 == NULL) {
        goto alloc_fail;
    }
    self->unk74 = BMemPMgrAlloc(count);
    if (self->unk74 == NULL) {
        goto alloc_fail;
    }
    self->unk5C->methods->slot80(self->unk5C, self->unk74, buf);

    p = self->unk70;
    i = 0;
    self->unk6C = 0;
    if (count != 0) {
        do {
            *p = New_Actor();
            __asm__("");
            if (*p == NULL) {
                goto fail;
            }
            p++;
            self->unk6C = self->unk6C + 1;
            i++;
        } while (i < count);
    }
    self->unk68 = self->unk70[buf[0]];
    return 0;

alloc_fail:
    self->unk74 = NULL;
fail:
    TodActor__DestroyParts(self);
    return 1;
}
#endif
```

(`Unk5CMethods::slot80` and `New_Actor` are kept live in
`src/TodActor.c` — both confirmed correct by the byte-identical
setup/allocation section and teardown call, independent of this stall.)

### Proposed learning

**A `__asm__("")` that fixes one residue (store/branch ordering) can
simultaneously introduce a DIFFERENT one (a register-identity swap
between two otherwise-unrelated locals/parameters) as a side effect of
the same global register-allocation pass it perturbs.** This is a new
combination for this unit's residue catalogue: previously a barrier
either helped a specific instruction-order issue outright
(`TodActor__FindPartIndex`) or did nothing (`TodActor__SetTod`'s slot134 case) or
actively hurt (`TodActor__SetLightMode`'s parameter-copy deferral) — never
"fixes one thing, breaks a different thing" in the same function. Since
GCC's register allocator operates on the WHOLE function, a barrier placed
deep inside a loop is not a purely local scheduling hint; it can shift
which hard register any pseudo in the function gets, including ones far
away from the barrier's textual position. Worth checking the ENTIRE diff,
not just the region near a barrier's insertion point, before declaring a
barrier a clean win.

## Round 18 (permuter pass, charlie): type/declaration-axis search (negative)

Per the head's round-wide broadcast (two "unfixable register-identity"
stalls elsewhere overturned by a mistyped declaration, not a real
register-identity limit), this function was one of three in this unit's
queue flagged as carrying a register-shaped verdict worth re-testing on
type/declaration axes rather than expression/statement shape.

Built a `PERM_GENERAL` seed over: `count`'s type (`s32` vs `u32`),
`i`'s type (`s32` vs `u32`), the count-extraction expression (`& 0xFF`
vs an explicit `(u8)` cast), and the `p =`/`i = 0` statement pair's
order (three variants: original, same, swapped -- kept symmetric with
the sibling seeds this round). 2x2x2x3 = **24 combinations, run
exhaustively** (`permuter.py`: "Will run for 24 iterations",
`permuter exit=0`, all 24 completed).

**Best score across all 24 combinations was 368 -- the base score
itself** (several combinations reproduce it exactly; every other
combination scored worse, 568). No type or declaration axis tried here
improves on the existing 49/68 preserved body. This is a clean negative
result specific to the type axis: the `self`/`p` whole-function
register-identity swap documented above is not explained by `count`'s or
`i`'s signedness, the count-extraction expression's form, or the
`p`/`i` initialization order -- consistent with (not proof of, but
consistent with) this being a genuine allocator-internal decision rather
than a masked-type residue like the sibling cases the head cited. The
existing STALL classification (49/68, register-identity swap, CLAUDE.md
rule 6) stands; this round adds evidence the type axis specifically was
checked and ruled out, not merely assumed clean by analogy to the two
overturned cases. `INCLUDE_ASM` untouched.

## Round 19 (echo): two more axes tried, both negative

Re-verified the 49/68 claim first (matched exactly, no size drift, same
residue). Tried two more axes not yet covered by round 14 or round 18:

1. **Split-combined-expression** (the axis that closed `GetSetBitField`'s
   register-pair swap, `docs/match-reports/GetSetBitField.md`) applied to
   the function's one remaining combined expression,
   `self->unk68 = self->unk70[buf[0]];`, rewritten as
   `i = buf[0]; self->unk68 = self->unk70[i];` (reusing the existing `i`
   local after the loop is done with it). Result: **worse, 45/68** (new
   divergences appeared at word 42, 47, 50-52 that weren't in the
   baseline diff) -- reverted immediately.
2. **A second `__asm__("")` at the very top of the function**, in
   addition to (not instead of) the existing in-loop barrier -- testing
   whether combining both barrier positions (rather than either alone,
   both already tried separately in round 14) might close the
   whole-function swap. Result: **no change, identical 49/68 diff**
   (same 18 diff lines, byte-for-byte) -- reverted (the extra barrier is
   simply inert here, not harmful, but not helpful either).

Both axes are now ruled out in addition to round 14's declaration-order/
barrier-position sweep and round 18's type/declaration-axis exhaustive
search. Filing unchanged as STALL at 49/68, `INCLUDE_ASM` restored
(confirmed via a fresh build, `build exit=0`, whole-image OK, before and
after this round's two attempts).

### Proposed learning

**The split-combined-expression axis is not a universal register-pair-
swap fix -- it helped `SceneNode__AddToActorParents`'s swap (this same round, one word)
and `GetSetBitField`'s swap (round 18, full close), but made THIS
function's whole-function swap measurably worse.** Three data points now
exist for this axis across two different residue shapes (a 2-value pair
swap confined to a handful of instructions, vs. a whole-function swap
touching nearly every instruction that references either value): it
fully closed the confined case, partially helped a second confined case,
and hurt the whole-function case. The axis is worth trying early on a
CONFINED register-pair residue; it is not obviously worth trying on a
residue that already reads as "the swap touches everything" before
reaching for it.

## Round 20 (alpha): re-verified, one more axis tried (inert), lever screen negative

Re-verified first: dropped the preserved 49/68 body in live, rebuilt.
`funcdiff.py` reproduces **49/68** exactly, the same 18 diff lines at the
same offsets as every prior round -- no drift, no contamination.

**Screened against the round-20 outgoing-arg dead-code lever:** this
function is not a candidate. It has four real `jal`s (`BMemPMgrAlloc`
x2, `New_Actor`, `TodActor__DestroyParts`) and does not appear in round 19's
live census of unexplained-outgoing-area functions -- its residue is a
whole-function register-IDENTITY swap, not a frame-size question, so the
lever's mechanism (a wider dead call inflating
`current_function_outgoing_args_size`) has nothing to attach to here.

**Tried one more axis not in the prior three rounds' lists: STATEMENT
order (not declaration order) of the three loop-setup writes.** Round 14
tried reordering where `p`'s DECLARATION sits among the locals; this
round instead reordered the three ASSIGNMENT statements themselves --
`p = self->unk70; i = 0; self->unk6C = 0;` rewritten as
`self->unk6C = 0; i = 0; p = self->unk70;` (self-field write first, `p`'s
assignment last). Result: **byte-identical, 49/68, same 18 diff lines**
-- this ordering is completely inert, on top of declaration order
(round 14), the type/declaration permuter sweep (round 18, 24
combinations), the split-expression axis (round 19), and a second
barrier (round 19). Reverted; confirmed the reversion rebuilds
`build exit=0`, whole-image green, `git diff --stat src/TodActor.c`
empty.

Five independent axes have now failed to move this whole-function swap
(declaration order, type/signedness x24, split-expression, statement
order, extra barrier), on top of the exhaustive 24-combination permuter
search. This is about as strong a case as this project's corpus has for
CLAUDE.md's rule-6 STALL classification actually applying as written --
not one untried axis remains that this unit's own residue catalogue has
previously found to matter for a DIFFERENT stall. Remains a STALL at
49/68, `INCLUDE_ASM` restored, `src/` confirmed clean.

### Proposed learning

No new lever found. Recording the negative for the next round: this
function's register-identity swap has now survived every axis this
unit's own catalogue considers a "usual suspect" (declaration order,
statement order, type, split-expression, barrier placement/count). A
future attempt should look for something NOT already in that list rather
than re-testing a variant of one of these five.

## Round 24 (echo): title corrected to the three-figure format; TWO new axes tried, both negative

Re-verified first: dropped the preserved 49/68 body in verbatim, rebuilt.
`funcdiff.py` reproduces **49/68** exactly, no drift. `tools/asm-differ/diff.py`
confirms the first real diff (realigned) is at file offset **0x056620** /
vram **0x80065E20** -- the prologue's `sw $s2`/`move $s2,a0` vs retail's
`sw $s1`/`move $s1,a0`, i.e. the `self`<->`p` swap starting from word 1.
Corrected the title to the three-figure format.

**Axis 1: round 21's "eliminate a loop-carried pointer, index directly"
register-pair-swap fix — this unit's residue is EXACTLY the shape that lever
targets** (`func_8003F848`'s report: "a register-pair swap in a loop body
wants [the pointer] deleted"), so this was the most promising untried axis
in the project's current catalogue for this specific residue shape. Rewrote
the per-element loop to drop the `Unk70ElemObj **p` local entirely and index
`self->unk70[i]` at each use site (`self->unk70[i] = New_Actor(); ...
if (self->unk70[i] == NULL) ...`), keeping the barrier and everything else
identical. **Result: 2/68, WITH size drift (60109 bytes outside range) —
dramatically worse**, not better. Reverted immediately; confirmed clean
rebuild.

This is a real negative for the lever's scope, not just for this instance:
`func_8003F848`'s swap was between two SCALAR values (a decrementing pointer
and its paired index) with no other structural role for the pointer; here
`p` is the array WRITE target inside a loop whose result (`self->unk70`) is
read again afterward (`self->unk68 = self->unk70[buf[0]];`), and indexing
directly forces the compiler to re-materialize `self->unk70` at every use
instead of keeping a single incrementing address live -- a fundamentally
different register-pressure shape, not a smaller version of the same one.

**Axis 2: the "early-alias" indirection** (assign the sole parameter to a
fresh local immediately at function entry, tried on `TodActor__SetLightMode` last
round for a different residue class and found inert there). Introduced
`TodActor *s2 = self;` at the top and rewrote every `self->` access to
`s2->` (including the `TodActor__DestroyParts(s2)` tail call), keeping `self` itself
otherwise unused after the alias. **Result: 49/68, IDENTICAL residue, no
change whatsoever** (same offset, same diff). A second confirmation that
this axis does nothing for parameter/self register-identity questions in
this unit, now on two different residue shapes (a deferred-copy-timing
residue in `TodActor__SetLightMode`, and this whole-function swap).

Both reverted; confirmed the reversions rebuild `build exit=0`, whole-image
green, before moving to the next function.

This brings the total to seven independent axes ruled out (declaration
order, statement order, type/signedness x24 combinations, split-expression,
extra barrier, pointer-elimination, early-alias), plus the 24-combination
exhaustive permuter sweep. Remains a STALL at 49/68, `INCLUDE_ASM` restored,
`src/TodActor.c` confirmed clean.

### Proposed learning

**The round-21 "eliminate a loop-carried pointer, index directly"
register-pair-swap lever is NOT safe to apply from residue-shape pattern-match
alone.** It requires the pointer to have no role beyond pairing with the
scalar it swaps against; here the pointer is also the array's write cursor
whose final state feeds a later read, and deleting it regressed catastrophically
(49/68 -> 2/68 with drift) rather than helping. Before applying this lever,
check whether the array/buffer the pointer walks is read again through a
DIFFERENT index expression later in the function -- if so, the pointer is
doing double duty and this lever's precondition does not hold.

## Round 25 (delta): block-order check (negative — bare `j` present but not at the residue)

Per the head's round-25 block-order broadcast: grepped for a bare
unconditional `j` (not `beq`/`bne`/`bgez`) whose target is a join with real
work in its delay slot.

```
grep -nE '\*/\s+j\s' asm/nonmatchings/TodActor/TodActor__CreateParts.s
```

**One hit**, at file offset `0x566F4` / vram `0x80065EF4`:

```
lw   $v1, 0x0($v0)
addu $v0, $zero, $zero
j    .L80065F0C
 sw  $v1, 0x68($s2)        ; delay slot: self->unk68 = v1 -- real work
.L80065EFC:
 sw  $zero, 0x74($s2)      ; self->unk74 = NULL   (alloc_fail:)
.L80065F00:
 jal TodActor__DestroyParts
  addu $a0, $s2, $zero
 ori  $v0, $zero, 0x1
.L80065F0C:                 ; shared epilogue (lw $ra / $s3 / $s2 / $s1, jr $ra)
```

Checked whether this is the same shape as `CheckDreamAuxTriggerCondition`'s closure: there,
the jumping arm was placed textually LAST in the C, so GCC let it fall
through and dropped the `j`. Here it's the opposite arrangement already —
`.L80065EFC`/`.L80065F00` (the `alloc_fail:`/`fail:` cleanup block) is
reached from *earlier* failure checks via `beqz` (confirmed at file
offsets `0x56664`, `0x56674`, `0x566B8`), and the SUCCESS path (which
this `j` belongs to) falls through from its own code, then jumps OVER
that cleanup block to reach the shared epilogue. The preserved body
already mirrors this exactly: the cleanup code (`alloc_fail:` / `fail:`
labels) sits at the BOTTOM of the C function, after the main `return 0;`,
which is precisely "the jumping block's target sits after a block that is
placed last" — i.e. this join is already laid out the way
`CheckDreamAuxTriggerCondition`'s fix wants it, not the way its bug had it. Confirmed no
regression by inspection: every attempt in this report's history
(rounds 14-24) preserved this exact `goto alloc_fail; ... alloc_fail: ...
fail: ...` shape without moving the cleanup block, and none of them ever
showed a diff in this region — all recorded diffs are anchored at word 1
(the prologue swap) or the intermediate register-identity residue,
never here.

**Conclusion: the bare `j` exists, but it is not the site of this
function's documented residue and the block-order lever does not apply
to it** — the residue is the whole-function `self`<->`p` register-IDENTITY
swap at the prologue (word 1), a different mechanism than basic-block
layout entirely (and, per CLAUDE.md rule 6, one this project's rules
forbid fixing with register pinning). No new build attempt spent; this is
a screening negative, like the outgoing-arg-lever screens in rounds 20/24.

## Round 31 (alpha): re-verified (no drift), round-27 discriminator confirms the identity classification, no new attempt

Re-verified first: dropped the preserved 49/68 body in live, rebuilt.
`funcdiff.py` reproduces exactly **49/68**, file range `0x5661C-0x5672C`
(0x110 bytes = 68 words, no drift) — same 18 diff lines documented since
round 14. `INCLUDE_ASM` restored, `git diff --stat` confirmed clean.
Blocker screens (`gp_rel`, `mflo`/`mfhi`-into-`mult`/`div`) both clean.

**Applied round 27's callee-saved-register discriminator** (developed seven
rounds after this function's swap was first classified, so this is an
independent re-check with a tool that did not exist when the verdict was
made): `grep -oE 'sw +\$(s[0-7]|fp|ra),' asm/nonmatchings/TodActor/TodActor__CreateParts.s
| sort -u` against retail gives `$ra,$s0,$s1,$s2,$s3`, and the preserved
body's own compiled prologue saves the identical set. Per the discriminator
("SAME set on both sides -> genuine register identity, stop, CLAUDE.md rule 6
applies, banned to fix with pinning") this confirms -- via a mechanism
independent of the seven axes already exhausted (declaration order, statement
order, type/signedness x24 combinations, split-expression, extra barrier,
pointer-elimination, early-alias) plus the 24-combination exhaustive permuter
sweep -- that this is not a masked structural residue of the kind round 27
found elsewhere. No lever in the project's current catalogue targets a
whole-function register-identity swap; none was applied. No new manual or
permuter attempt spent. Remains a STALL at 49/68, `INCLUDE_ASM` restored.

### Proposed learning

Round 27's address-taken-parameter discriminator (`docs/DECOMPILATION_LEARNINGS.md`)
was built to test a DIFFERENT hypothesis (a parameter wrongly promoted to a
callee-saved register instead of retail's stack home slot), but its
"diff the saved-register set" check is equally useful as an independent
confirmation tool for an EXISTING register-identity verdict, on any function,
not just ones freshly suspected of misclassification. Running it costs one
grep and turns "this project's rules say stop, trust the original reasoning"
into "this project's rules say stop, and a second, later-developed mechanism
agrees."

## Round 33 (alpha): re-verified (no drift), round-32 levers screened, `volatile`-qualified walk pointer tried (worse, with drift)

Re-verified first: dropped the preserved 49/68 body in live, rebuilt.
`funcdiff.py` reproduces exactly **49/68**, same 18 diff lines anchored at
the prologue `self`<->`p` swap (`0x056620`/vram `0x80065E20`). `INCLUDE_ASM`
restored, `git diff --stat` clean.

**Round-32 lever 1, tried directly on the specific mechanism that
introduced this function's register swap:** the existing `__asm__("")`
barrier (which closes the store-before-branch ordering sub-issue but
introduces the whole-function `$s1`/`$s2` swap as a side effect) was
replaced with a `volatile`-qualified walk pointer,
`Unk70ElemObj * volatile *p;`, removing the barrier and relying on the
qualifier to fence the `*p = New_Actor();` / `if (*p == NULL)` pair
instead — the theory being that a narrower per-access fence might close
the ordering sub-issue without perturbing the whole-function register
allocation the way a barrier does. **Result: 33/68, WORSE, WITH size
drift (52,571 bytes outside range)** — qualifying the pointee as volatile
forces the compiler to reload through memory more aggressively than
retail does anywhere in this function (retail keeps the array-write
cursor in a register throughout), a structurally different and larger
regression than either the plain unbridged translation or the
barrier-based 49/68. Reverted immediately; confirmed the reversion
rebuilds `build exit=0`, whole-image green, `git diff --stat` clean.

**Levers 2-5 screened:** (2) register-identity skepticism — genuinely
re-examined, not just re-asserted: this function already has SEVEN prior
axes ruled out for the swap specifically (declaration order, statement
order, 24-combination type/signedness permuter sweep, split-expression,
extra barrier, pointer-elimination, early-alias) plus round 27's
callee-saved-set discriminator (both sides save the identical five
registers) confirming genuine reordering rather than a masked-type
promotion/demotion. This round's volatile attempt is an EIGHTH axis, also
negative — consistent with, not merely repeating, the existing
classification. (3) not applicable this round. (4) no new permuter run —
round 18's 24-combination exhaustive sweep already covers the type/
declaration space this function's swap could plausibly respond to. (5)
the swap is a genuine same-registers-different-values difference (`$s1`
holds `self` where retail has `p`, and vice versa, confirmed by reading
actual operand register numbers), not an encoding artifact.

Remains a STALL at 49/68, `INCLUDE_ASM` restored, `src/TodActor.c`
confirmed clean before and after.

### Proposed learning

**The round-32 volatile lever can turn a whole-function register-IDENTITY
side effect into a worse, DIFFERENT residue (a real memory-traffic
regression) rather than a neutral or beneficial substitution for a
barrier** — this is the sharpest negative result among this round's three
volatile experiments in this unit (compare `TodActor__SetDisplay`'s 26/33 and
`TodActor__ApplyTodPacket`'s "no change"): here it actively introduced address
drift, the worst outcome category this project tracks. Qualifying a
pointer's POINTEE as volatile is not a narrow substitute for a barrier
when the pointee is written and read repeatedly inside a loop — it
changes the loop's own memory-access pattern, not just the fencing at one
program point, which is a materially different mechanism from the

## Round 49 (alpha): re-verified (no drift); this function's FIRST genuine open-ended permuter search (round 18's was a 24-combination manual enumeration, not a random search) — one UB-disqualified candidate, one real-oracle-checked negative, no improvement

Per the head's round-49 opening (this unit stale since round 33), and
this function specifically flagged in the assignment as "NEVER
permuter-searched (0 iterations on file)": re-verified first, dropped the
preserved 49/68 body in live, rebuilt. `funcdiff.py` reproduces exactly
**49/68**, same 18 diff lines anchored at the prologue `self`<->`p` swap
(`0x056620`/vram `0x80065E20`). `INCLUDE_ASM` restored, `git diff --stat`
confirmed clean.

**Correcting the assignment's own iteration-count claim, since round 18
DID run `permuter.py` (a `PERM_GENERAL` seed, exhaustively enumerated over
24 concrete combinations) — but that is a bounded manual enumeration over
four hand-picked axes, not an open random search over the permuter's full
mutation set.** This round is the first of the latter kind. Provisioned a
fresh scaffold (`tools/setup-permuter.sh TodActor__CreateParts <seed>` against the
preserved 49/68 body). `--debug --stack-diffs` reproduced base score
**368** exactly, matching round 18's own documented figure — **CHECK 3:
AGREE**, scaffold targets the same residue as the real build.

Launched under low contention (load average 3.85/32 at launch):

```
timeout 900 permuter.py -j 8 --stop-on-zero --best-only permuter-work/TodActor__CreateParts
```

**61,377 iterations, rc=124 (bound fired on its own — confirmed via its
own file, not inferred)**. Four candidates found below the base score of
368, at 110/220/275/280; no zero. All four inspected:

- **110 (best), disqualified by hand-tracing, not built — a textbook
  instance of round 48/delta's UB-forward-trace lever.** The candidate
  hoists `TodActor *unk5C's slot80` into a function pointer local (an
  inert reformulation) but ALSO introduces `volatile int new_var2;
  new_var2 = self->unk6C;` immediately before the loop, then rewrites the
  loop's own `self->unk6C = self->unk6C + 1;` as `self->unk6C = new_var2
  + 1;`. Since `self->unk6C` is reset to `0` just before this, and
  `new_var2` is captured ONCE outside the loop and never updated inside
  it, every iteration recomputes `self->unk6C = 0 + 1 = 1` instead of
  accumulating — the count field ends the loop at `1` instead of `count`,
  regardless of how many elements were actually allocated. This is
  exactly the shape round 48's delta lever describes: a variable
  reassigned to something else (the loop increments `self->unk6C` field
  directly in the real program) and read under its OLD meaning (the
  pre-loop snapshot) is a plain logic bug the scorer cannot see because it
  only diffs compiled bytes, never executes anything. Traced by hand
  (`self->unk6C` after N successful iterations: real program `N`, this
  candidate always `1`); disqualified without spending a build.
- **220, real-oracle-checked (round 47 outcome-3 material) — translated,
  built, and it regresses.** Wraps the `if (count != 0) { do {...} while
  (...); }` block in `do { ... } while (0);` and moves the `__asm__("")`
  barrier from right after the `*p = New_Actor();` store to right
  after the null check instead (a genuinely new, previously-untried
  barrier position — round 14's table only tried "first statement",
  "right after the call", "top of function", never "after the null
  check"). Translated to `src/` verbatim (behavior-preserving: a
  `do{}while(0)` wrapper and a barrier reposition change nothing
  observable) and rebuilt: **35/68, WITH size drift (98,244 bytes outside
  range)** — worse than the trusted 49/68 no-drift baseline in real
  terms, despite scoring better under `--stack-diffs`. Reverted
  immediately; confirmed the reversion rebuilds `build exit=0`,
  whole-image green. This is round 47's third check-3 outcome pattern
  (sub-base scaffold score, real build does not improve) at work on a
  candidate that DID pass check 3 at the scaffold level — confirming that
  AGREEMENT at the scaffold's starting signature does not extend to every
  candidate the search finds; each candidate still needs its own
  real-oracle check.
- **275 and 280, not translated** — both score worse than 220 under the
  same scorer (275 wraps a strictly larger region in the same kind of
  `do{}while(0)`; 280 introduces an inert `self`-aliasing local used only
  on the `alloc_fail:` path). Since the smaller, better-scoring 220
  variant of the same wrapping idea already regressed with drift when
  translated, neither was spent as a real build attempt.

**This closes out this function's fresh-search obligation for the
round.** Combined with round 18's 24-combination exhaustive enumeration
(a different, narrower kind of coverage), this function has now had both
a manual axis sweep and a genuine 61,377-iteration open search, neither
finding an improvement — the whole-function `self`<->`p` register-identity
swap (per CLAUDE.md rule 6, a STALL) remains the classification. Remains
a STALL at 49/68, `INCLUDE_ASM` restored, `src/TodActor.c` confirmed
clean before and after.

### Proposed learning

**A permuter search can find the SAME kind of UB/logic-bug candidate
round 48's delta lever describes on a completely different function and
residue shape** (there: a variable reassigned and read under its old
meaning in a UB-relevant context on `Task`; here: a loop-counter
field snapshotted once before a loop and re-read stale every iteration on
a register-identity-swap function) — this is now a second, independent
confirmation that the "trace every touched variable forward to its next
use" check belongs in the standard reading of ANY sub-base candidate, not
just ones that already look suspicious. The candidate here scored best
(110, well below the 220/275/280 cluster) specifically BECAUSE it changes
behavior — a logic bug that shortens the loop's effect is exactly the
kind of "improvement" a byte-diffing scorer rewards and a human tracing
the variable catches immediately.
lever's validated `ResetRCnt` use case.

## Round 71 (charlie): NON_MATCHING body promoted

Re-measured live under current maspsx flags (rounds 42/63) per the head's
note that titles may predate them: dropped the preserved body in as live C
(no `#ifdef`), `build exit=2` with no compile error (only a pre-existing
unrelated warning in `TodActor__FindPartIndex`), `funcdiff.py` reproduces **49/68
words match (file 0x5661C-0x5672C), no size drift** — identical to the
title figure, confirming it was already current. Restored the `#ifdef
NON_MATCHING ... #else INCLUDE_ASM ... #endif` shape (this project's only
accepted track-1b form) with a comment naming the score, the residue class
(whole-function `self`<->`p` register-identity swap, CLAUDE.md rule 6) and
this report. `./build-and-verify.sh` green (bytes unchanged) and
`tools/check-nonmatching.sh` green.

**Hand-derived, not permuter-found.** The promoted body is the one at
"Preserved body (best attempt, 49/68, no size drift)" above, produced by
round 14's manual `__asm__("")` barrier placement. Every later round's
permuter work (round 18's 24-combination exhaustive enumeration, round 49's
61,377-iteration open search) is a negative result against this same body,
not a source of any edit incorporated into it — the best permuter
candidate found (score 110) was disqualified by hand-tracing as a logic
bug (round 49) and never built or merged.

NON_MATCHING body promoted, round 71.

Head, round 71 merge: the `src/` NON_MATCHING body is written without the
order-only `__asm__("")` after `*p = New_Actor();` (track 1b: byte-only
constructs stay in the report, round 66). The byte-shaped form, with the
barrier, is the preserved body above.

## Round 75 (charlie): MATCHED 68/68, whole image `OK: build matches retail`

REVISITED, round 75: MATCHED; names/types not relevant (no name or type
changed; `buf[4]`, `s32 count`, `s32 i` all kept).

**First measurement, before any change:** the preserved body (with its
`__asm__("")` after `*p = New_Actor();`) rebuilt live: 49/68, length
exact, `insertions 6 / deletions 6`, **positional skeleton diffs 19** — so
the N/N was not an honest alignment: the store/branch/`p++` region was
still structurally different, not only renamed.

**The lever: write the store and the increment as one expression.**

```c
            if ((*p++ = New_Actor()) == NULL) {
                goto fail;
            }
```

in place of `*p = New_Actor(); __asm__(""); if (*p == NULL) goto fail; p++;`.
That is retail's instruction sequence read literally: `sw v0,0(s1)` right
after the call, then `beqz v0` with `addiu s1,s1,4` in its delay slot — the
increment happens BEFORE the test, unconditionally. With the store and
increment in one expression the barrier is unnecessary, and **the
"whole-function `self`<->`p` register-identity swap" disappears with it**:
every register in the function matches. The swap was never a
register-allocator stall; it was the priority shift caused by the extra
`*p` reload/`p++` statement (and by the barrier), i.e. a SHAPE residue that
presented as identity. Build count: 4 builds from the preserved body to
68/68.

Measured steps (each a fresh build, no compile error):

| body | score | ins/del | skeleton | note |
| --- | --- | --- | --- | --- |
| preserved (do/while + barrier) | 49/68 | 6/6 | 19 | s1/s2 swapped |
| `for` loop, `u8 count`, split store/test | 22/68 | 1/1 | 39 | still swapped, frame 0x40, drift |
| `for` loop, `(*p++ = ...) == NULL`, `buf[4]` | 56/68 | 0/0 | 0 | all regs right, frame 0x40 (8 bytes too big) |
| same, `buf[2]` | 68/68 | 0/0 | 0 | whole image OK, but buf size chosen to cancel the 8 |
| **original guard + do/while, `(*p++ = ...) == NULL`, `buf[4]`** | **68/68** | 0/0 | 0 | **whole image OK — committed** |

The `for` form this unit's three siblings matched with this round is NOT
the lever here: it costs 8 bytes of frame over the do/while form (a stack
slot the do/while never allocates), and only a smaller `buf` hides it. The
committed body keeps the do/while and `buf[4]` (retail's 0x10 bytes of
locals are consistent with a 16-byte buffer), so nothing in it is chosen to
cancel something else. `self->unk6C++` and `self->unk6C = self->unk6C + 1`
are byte-identical; the committed form uses `++`.

No permuter search spent (not needed).

### Proposed learning

**A whole-function register-identity swap that appears only together with
a scheduling barrier is a shape residue until proven otherwise.** Here
round 14 fixed a store/branch order with `__asm__("")` and "gained" an
s1/s2 swap; eight rounds then classified the swap as rule-6 identity. The
barrier was compensating for a statement split (`*p = f(); if (*p == NULL)
...; p++;`) that retail's bytes show was one expression
(`(*p++ = f()) == NULL`): store after the call, increment in the branch's
delay slot, i.e. the increment precedes the test. The retail tell is `sw
$v0,0($sN)` immediately after the `jal`, then `beqz $v0` with `addiu
$sN,$sN,4` in the delay slot. Same family as this round's loop-kind lever:
a barrier (or a filler) that fixes one symptom is evidence the statement
shape is wrong, not that the remainder is register allocation.

## Naming

Round 75 (charlie), track 3.

- `TodActor__CreateParts` (was `func_80065E1C`), tier A. Asks modelData->getObjectIds (gModelDataMethods +0x080) for the count, allocates parts (count pointers) and partIds (count bytes), fills partIds, creates one New_Actor per entry counting partCount up, sets mainPart = parts[buf[0]]; on any failure DestroyParts and return 1.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/TodActor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 99, bravo): readable spelling, byte-identical

`buf[4]` is now `tmdId[4]`, the name TodSet__ScanPackets / ScanTodPackets give
that argument: the first scan (out NULL) leaves the model-id packet count in
it, the second turns it into the index of the object whose model-id packet
names that TMD id, and that part becomes `mainPart`. Measured: `tmdId[1]` turns
the image red, so the `[4]` carries a `MATCHING:` line. The `& 0xFF` on the
first scan's result is dropped: the slot returns `u8`, and the image stays
`OK` without it. `count * 4` is `count * sizeof(Actor *)`.
