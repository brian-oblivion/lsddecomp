# Obj86B60__OnTag2Notify — MATCHED (byte-exact, whole-image `build exit=0`)

> Renamed from `func_8003C48C` on 2026-09-24 (tools/rename.py). Address 0x8003c48c.

Unit: `code_2cc8c` · Size: 36 instructions (0x90 bytes) · Round 23 (2026-09-07),
head. **First of the five `REOPENED -- ASSIGNABLE` functions to be closed, which
is the measurement that says the round-21 `addiu_at` resolution actually
returned ground rather than merely retiring paperwork.**

## History of this report, kept because the verdict was right and then wrong

Filed round 2026-09-02 (runner echo) as **toolchain blocked**, never attempted.
The classification was correct *at the time* and its reasoning was better than
the reasoning it replaced: the function's jump-table dispatch hits
`addiu $at, $at, %lo(jtbl_80011090)`, and echo established with a reproducer
that a `jtbl_*` symbol is NOT a safe exception to the `addiu_at` screen — cc1
emits the same generic pseudo-op for an indexed data global and a switch jump
table, and the fold to three instructions happens in maspsx, below cc1, which
cannot tell them apart. That correction stands and is still in CLAUDE.md.

What changed is the blocker, not the analysis. Round 21 gave maspsx a
`--addiu-at` flag that sets `addiu_at` alone, so the pipeline now emits retail's
unfolded four-instruction form directly. The dispatch sequence reproduces
untouched.

## The match

```c
void Obj86B60__OnTag2Notify(Obj86B60 *self, s32 a1, s32 a2)
{
    Obj86B60Methods *methods;

    methods = self->methods;
    if (self->unk3C != 0) {
        switch (a2) {
        case 0x12:
            methods->slot80(self, a1);
            break;
        case 0x13:
            methods->slot84(self, a1);
            break;
        case 0x21:
            methods->slot74(self, a1);
            break;
        case 0x17:
            methods->slot7C(self, a1);
            break;
        case 0x19:
            methods->slot78(self, a1);
            break;
        }
    }
}
```

A message dispatcher. `a2 - 0x12` indexes `jtbl_80011090` (16 entries) after an
unsigned `sltiu ..., 0x10` range check; five codes forward to one of this class's
own slots with `(self, a1)` unchanged, and the other eleven are no-ops. Reading
`self->methods` into a local BEFORE the `self->unk3C` guard is what puts the
`lw $v1, 0x0($a0)` ahead of the `beqz` — the same idiom the already-matched
sibling `Obj86B60__OnTag5Notify` uses two functions later.

GCC cross-jumps the five arms into ONE `jalr $v0` site: each arm only loads its
slot into `$v0` and jumps to the shared call. That falls out of `break` plus five
identical call shapes and needed no encouragement.

## Struct knowledge added

`Obj86B60Methods` slots `+0x074`..`+0x084`, previously inside
`u8 pad074[0x090 - 0x074]`. **The pad split is additive and preserves the
original 0x1C total** (5 pointers = 0x14, plus a new `pad088` of 8) — required,
because `include/code_2cc8c.h` is shared by six units and any offset movement
would have changed an already-matched function's codegen elsewhere. Verified by
the whole-image SHA1, which is the only thing that can see it.

Slot occupants in the base table `gClass86B60Methods`, from the jump table plus
`tools/classtable.py`: `slot74` = `Obj86B60__func_8003C9B0`, `slot78` = `Obj86B60__func_8003C944`,
`slot7C` = `Obj86B60__func_8003C8D0`, `slot80` = `Obj86B60__func_8003C7F4`,
`slot84` = `Obj86B60__func_8003C858`.

## The only residue, and it is a source-order fact

The first attempt was six words off with everything else exact — three arm bodies
and their three jump-table words permuted. **The order the `case` labels appear
in the SOURCE is the order GCC 2.6.3 lays the arm blocks out in, and the jump
table's contents record it.** Retail's block order is `0x12, 0x13, 0x21, 0x17,
0x19`; writing the cases in ascending numeric order gets five arms in the wrong
places. Reordering the source to match, with no other change, closed it.

### Proposed learning

**A switch's `case` order is recoverable from the binary, and it is not the
numeric order.** GCC 2.6.3 emits the arm basic blocks in SOURCE order and the
jump table's words point at them, so the layout is a direct readout of how the
original author wrote the cases. Two consequences:

- When a dense-switch dispatcher is a handful of words off and the diff is
  *arm bodies swapped* rather than instructions changed, do not reach for
  scheduling — read the arm addresses off the target's own labels, sort the
  slots by address, and write the cases in that order. Here it was 6 words for
  one edit.
- The last arm in source order is the one that FALLS THROUGH into the shared
  cross-jumped call site (no `j` of its own), so the target's fall-through arm
  identifies the final `case` unambiguously. That is a free anchor for the
  ordering even before matching anything.

Corollary for reading a report: this function is proof that a **correctly
reasoned, reproducer-backed stall verdict still expires when its blocker
moves.** Echo's analysis was right and is still worth reading; only the verdict
died. That is exactly what the `REOPENED -- ASSIGNABLE` marker is for, and this
is its first realised match.

## Naming (round 78, delta)

**Tier A.** `func_8003C48C` -> `Obj86B60__OnTag2Notify`. Occupies slot58 in
`gTaskCoreMethods` (the base table), `gClass86B60Methods` and
`gGraphRoomMethods` identically (unoverridden by either derived class --
`tools/classtable.py gTaskCoreMethods`/`gClass86B60Methods`/`gGraphRoomMethods`).
`IntermediateBase__OnNotify` (code_2cc8c_c.c) dispatches an incoming `EventArg` whose
`target->header & 0xF == 2` through `self->methods->slot58`, matching the
already-established `onTag1Notify` (header==1, slot54) naming convention one
slot up. Two independent pieces of evidence agree (the dispatcher's own
switch and the slot's universal, unoverridden occupancy), so tier A.
