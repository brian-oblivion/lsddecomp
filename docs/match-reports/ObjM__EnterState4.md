# ObjM__EnterState4 -- MATCHED 71/71, round 19 (bravo)

> Renamed from `func_80053ACC` on 2026-09-24 (tools/rename.py). Address 0x80053acc.

Unit: `src/class_3bb8c_l.c`. Originally stalled by echo (round 16), CLOSED by
bravo (round 19) via the permuter. `./build-and-verify.sh` green,
byte-exact (verified directly against `disk/SLPS_015.56` at
`0x442CC-0x443E8`, independent of the whole-image SHA1 which was still red
this round for unrelated reasons -- another function mid-edit in a
different unit).

## Round 19 close: the permuter found it, one variable rename made it real

Set up `tools/setup-permuter.sh ObjM__EnterState4` from the preserved body
below. `--debug --stack-diffs` confirmed the base score (165: 1 register
diff, 1 reordering, 1 deletion, 0 insertions, 0 stack diffs) matched the
report's own characterization exactly before spending any search budget.
Bounded search (`timeout 300`, `-j 6 --stack-diffs --stop-on-zero`) found a
**zero at iteration 698** (~697 non-zero iterations first, scores bouncing
between 165 and several thousand under load with 5 other runners' searches
concurrently active on the same machine).

The winning permuter candidate:

```c
int new_var;
...
new_var = (self->unk1C + ((s32) self->unk38)) & 3;
t = new_var;
if (t == 0) { ... }
```

i.e. the switch discriminant computed into a SEPARATE variable (`new_var`)
and then COPIED into `t`, rather than assigned to `t` directly. This is
exactly the "mention the source expression twice" lever already documented
in `docs/DECOMPILATION_LEARNINGS.md`, applied to a scalar rather than a
loop bound: keeping both `new_var` (the freshly-computed value, which
lands in retail's `$a0`/`move a0,v1` delay-slot register) and `t` (the
value actually tested and switched on, in `$v1`) alive simultaneously
across the switch is what makes GCC materialize the redundant `move`.

Translated verbatim into idiomatic C (renamed `new_var` to `span` for
clarity, otherwise unchanged structure/order):

```c
s32 span;
s32 t;
...
span = (self->unk1C + (s32)self->unk38) & 3;
t = span;
if (t == 0) { ... }
```

Rebuilt through the real toolchain: **71/71, byte-exact, no drift**.
Twelve prior hand-reshaping attempts (round 16) never introduced a SECOND
variable holding the same value -- every one either assigned straight into
`t` or experimented with control-flow shape (switch vs if-chain, case
order, outer polarity), which is exactly the untested axis this closes.

### Proposed learning

**The "mention the value twice, dependent quantity first" lever (already
documented for loop bound/pointer pairs) also applies to a plain scalar
switch discriminant`,` not just to loop setup.** A `move`-into-an-otherwise-
dead-argument-register residue, confined to one delay slot with the rest of
the function settled, is a strong signal to try "compute into a throwaway
local, then copy" before accepting the residue as an unreachable scheduling
artifact -- and the permuter is the fast way to find WHICH untested axis is
live when 12+ manual attempts have already covered order/shape/polarity.

---

# Prior state (round 16-18), preserved for context

Unit: `src/class_3bb8c_l.c`. Runner: echo, round 16. `INCLUDE_ASM` restored;
`./build-and-verify.sh` green.

## Signature

```c
void ObjM__EnterState4(Obj87034_3bb8c_l *self);
```

## What is established (high confidence -- derived directly from the
disassembly, not guessed)

```c
void ObjM__EnterState4(Obj87034_3bb8c_l *self) {
    s32 local18;
    s32 t;
    s32 arg3;

    self->unk20 = 4;
    if (self->unk3C->methods->slotF0(self->unk3C, &local18, -1) == 0) {
        t = (self->unk1C + (s32)self->unk38) & 3;
        if (t == 0) {
            self->methods->slot30(self, 4);
            return;
        }
        arg3 = 0xA;
        switch (t) {
        case 1:
            local18 = 0;
            break;
        case 2:
            local18 = 4;
            break;
        case 3:
            local18 = 7;
            arg3 = 5;
            break;
        }
        ObjM__ForwardToSubChild(self, local18, 0, arg3, 1);
        return;
    }
    ObjM__ForwardToSubChild(self, 0, 0, 5, 1);
}
```

- `self->unk3C->methods->slotF0` is a NEW slot on `DreamSysMethods_3bb8c_l`,
  `s32(void*, s32*, s32)`, added at `+0xF0`. Added to `include/class_3bb8c.h`.
- `self->methods->slot30` is a NEW slot on `Obj87034Methods_3bb8c_l`,
  `void(Obj87034_3bb8c_l*, s32)`, added at `+0x030`. Added to
  `include/class_3bb8c.h`.
- Both offsets/signatures are read straight off the disassembly (call-site
  argument registers, `jalr` target load offset) and are reliable regardless
  of the stall below -- only the *codegen* of this one function is in
  question, not the struct layout.
- All literal values (`unk20=4`, mask `&3`, case values 0/4/7, `arg3`
  0xA/5, the trailing `1`/`0`) are confirmed against the raw bytes.
- `ObjM__ForwardToSubChild` (the sibling-unit helper from `class_3bb8c_m`, matched by
  echo round 15) is called TWICE in source -- once at the end of the
  `ret==0`+switch path, once for the `ret!=0` path -- not once after a
  shared if/else. This was itself a finding: see "the two-call lever" below.

## The residual: one register in one delay slot

Every byte matches except a single, isolated register-allocation choice.
Right after computing `t` and testing the outer `bnez` (branch away if
`t != 0`), retail fills the branch's delay slot with:

```
bnez v1, <switch entry>
 move a0, v1        ; a0 := t  (retail)
```

Every C structure this round produced instead filled that slot with the
first useful instruction of the switch body (`li v0, 2`, the immediate
needed for the `case 2` test), which is the ordinarily-expected delay-slot
fill and is what m2c also would produce. Retail's choice keeps a REDUNDANT
copy of `t` alive in `a0` (a register otherwise dead at that point -- `self`
already lives in `s0`) purely so the LAST comparison in the tree (`case 3`,
reached only via the earlier `slti v0,v1,3` range check) can test `a0`
instead of `v1`:

```
slti v0, v1, 3
beqz v0, <case3 test>      ; only reachable when t == 3
 ...
<case3 test>:
li v0, 3
beq a0, v0, <case3 body>   ; retail: compares a0 (== t, untouched since
                            ; the top), not v1
```

Every one of this round's C variants that reached the switch produced this
same test IN THE SAME POSITION but comparing `v1` (the live variable),
never `a0`. The two are semantically identical (both hold `t`), so this is
not a mismatch of what the code MEANS -- it is a difference in a single GCC
2.6.3 -O2 register-allocation/scheduling decision that this round's C could
not force from any source variant tried.

## What was tried (12+ distinct builds)

1. **If/else with a single shared call at the end** (`arg1 = local18` /
   `arg1 = 0` merging into one `ObjM__ForwardToSubChild(...)` call) -- 66/71 total
   function length (5 words SHORT). GCC CSE'd the `ret==0` and `ret!=0`
   paths' call setup (`a0=self`, `sp[0x10]=1`, `a2=0`) into one shared tail,
   which retail does NOT do -- retail duplicates that setup once per path,
   sharing only the bare `jal` instruction.
2. **Two separate calls with explicit `return` in each branch** (the
   version above) -- 70/71 total length (1 word SHORT), and reproduces
   retail's per-path call-setup duplication exactly. This is the closest
   variant and the one preserved in `#if 0` in the source. Confirmed via
   `tools/asm-differ/diff.py ObjM__EnterState4` that from the `case 1` target
   label onward (roughly the back half of the function) every single word
   matches retail; the only structural gap is the one delay-slot
   instruction described above and its downstream `beq a0,v0` vs
   `beq v1,v0`.
3. **Switch case declaration order** (`1,2,3` vs `2,1,3` vs `3,1,2`
   implicitly via other experiments): changes which case's BODY comes
   first in the emitted block order (confirmed: body order follows
   declaration order) but not the TEST order, which is always `==2` first
   regardless (GCC picks the value-sorted median as the pivot, independent
   of source order). `1,2,3` declaration order was needed to make the body
   order match retail (case 1 body before case 2 body); this fix is
   included in the preserved variant above.
4. **`t` assigned inside the `if` condition** (`if ((t = ...) == 0)`)
   instead of as its own statement -- no change (still 28/71 in-range,
   same divergence point).
5. **Splitting `case 3` out of the switch into a separate trailing
   `if (t == 3)`** -- DID reproduce a fresh-register comparison for that
   case (closer superficially, 31/71) but lost the `slti v0,v1,3` range
   check entirely (GCC no longer needed a 3-way balanced switch, so it
   fell back to a plain `bne` chain) -- moves further from retail's actual
   instruction mix, not closer.
6. **Folding `case 0` into the switch itself** (as an explicit `case 0`, and
   separately as `default:`) instead of a preceding `if (t == 0)` -- both
   variants scored WORSE overall (49/71 total-length-corrected in one case,
   21/71 in the other) because GCC's 4-value balanced-tree pivot differs
   completely from the 3-value {1,2,3} tree retail actually emits; the
   `t == 0` case is definitely NOT part of the switch's own dispatch tree in
   retail (confirmed independently: retail's `t==0` path is a plain
   `bnez`-not-taken fallthrough with no switch machinery at all).
7. **Inverted outer polarity** (`if (t != 0) {switch...} else {slot30}`) --
   worse (20/71).

None of these moved the ONE-instruction delay-slot residue. This looks like
a genuine GCC 2.6.3 -O2 scheduler/allocator artifact tied to something
about the function's overall shape (total local count, or an earlier
expression's live-range accounting) rather than anything expressible by
rearranging this particular switch/if. Left for a future round with fresh
eyes, or for the toolchain-reproducer route in CLAUDE.md's "Escalate, do
not experiment" section if it recurs elsewhere.

### Proposed learning

When a near-miss residue is confined to exactly one delay-slot register
choice with every other byte matching (confirmed via
`tools/asm-differ/diff.py`, not just the word-count from `funcdiff.py`),
that is strong evidence the C's CONTROL FLOW and VALUES are already right
and the gap is pure GCC 2.6.3 register-allocation scheduling -- worth
distinguishing from a stall where the control-flow shape itself is still
wrong, since the two call for very different next steps (the former is
close to unmatchable-by-rearranging-C; the latter usually yields to more
structural exploration).

## HEAD NOTE (round 16): the 28/71 headline was a drift-corrupted number, and the real residue is TWO coupled instructions, not one

The head spliced this preserved body back into `main`, built, and measured it
before consolidating. Three corrections, in increasing order of importance.

**1. `28/71` is not this body's score.** `funcdiff.py` reported it together
with its own guard:

```
ObjM__EnterState4: 28/71 words match (file 0x442CC-0x443E8)
WARNING: the build differs OUTSIDE this range too (159121 bytes) - a size change may have
         shifted linked addresses, so this per-function read is NOT trustworthy.
```

That is CLAUDE.md's way-a-score-lies #3 (address drift) firing exactly as
designed. The body is one word SHORTER than retail, so every later address
shifts and the per-function window stops meaning what it says. The report's
own body already carried a hand-corrected `70/71`, so the analysis was sound
-- but the TITLE and the summary line carried the untrustworthy number, and a
title is what the next round triages from. **Never headline an in-range score
that came with a drift warning.**

**2. Read with `asm-differ`, which aligns rather than windows, the residue is
two differing instructions and one missing word:**

```
retail                                built
44324:  move  a0,v1     <- delay slot  44324:  li    v0,0x2    <- delay slot
44348:  li    v0,0x2    <- br target   (absent -- this is the missing word)
44374:  beq   a0,v0,44394              44370:  beq   v1,v0,44390
```

**3. And they are ONE decision, not two.** Retail's compiler copies the masked
switch value `v1` into `a0` in the `bnez` delay slot, and then compares
against that copy (`beq a0,v0`) for the case-3 test. The build instead sinks
`li v0,0x2` into the delay slot and keeps comparing `v1`. So this is a
rematerialize-into-the-argument-register choice whose consequence shows up in
a later comparison -- not an isolated delay-slot filler. Note retail's
`move a0,v1` is DEAD on the not-taken path (`a0` is overwritten by
`move a0,s0` at 0x4432C), which is why it reads as a scheduling artifact and
why the twelve hand reshapes could not reach it.

### Consequence for the next round: this is a PERMUTER candidate, and the report's
### own proposed learning should not be read as closing it

Two instructions and one word in a 71-word function, with control flow and
every literal already confirmed, is squarely Gate 3 territory in
docs/PARALLEL-RUNS.md ("near-misses like 88/90"). The runner's proposed
learning ends "close to unmatchable-by-rearranging-C", which is true and is
exactly why it should go to the permuter rather than to another hand pass --
the permuter mutates C source under the pinned toolchain, which is not a
toolchain change. **A zero from it is a LEAD, not an answer:** translate it to
idiomatic C and re-verify with funcdiff before believing it. If only UB or
duplicate-arm forms reach zero, mark the class permuter-exhausted here.

This is NOT a toolchain escalation. Nothing here has been reproduced in
isolation, and a coupled register/delay-slot choice inside a switch is the
kind of thing GCC 2.6.3 does legitimately.

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_80053ACC` | `ObjM__EnterState4` | B | see below |

**Evidence.** vtable slot +0x094. Sets `self->phase = 4`. The state-code numbering is confirmed, not guessed: sibling unit class_3bb8c_m already established `ObjM__EnterState7`/`ObjM__EnterState8`/`ObjM__EnterStateA` for the SAME field on the SAME class, and this unit's own `ObjM__HandleStateCode` dispatches codes 0xA..0x11 onto exactly the same run of vtable slots (+0x094..+0x0AC) that these three functions occupy, so 4/5/6 continue that one numbering.
