# StageMap__FindSlotByNeighbour — MATCHED (15/15 words)

> Renamed from `StageMap__FindElemByUnk32` on 2026-09-26 (tools/rename.py). Address 0x8004c434.

> Renamed from `Class866E8__FindElemByUnk32` on 2026-09-26 (tools/rename.py). Address 0x8004c434.

> Renamed from `func_8004C434` on 2026-09-24 (tools/rename.py). Address 0x8004c434.

Not a vtable slot (not in `gStageMapMethods`), a plain non-virtual helper —
companion to `StageMap__CountFlaggedElements`, walking the same `self->arr` array but
searching a different field.

## Disassembly

```
addu  $a3, $zero, $zero      ; i = 0
li    $a2, 0xEC              ; running byte offset, starts at arr's own offset
.loop:
addu  $v1, $a0, $a2          ; v1 = &self->arr[i]
lw    $v0, 0x4($v1)          ; self->arr[i].unk4
nop
lh    $v0, 0x32($v0)         ; self->arr[i].unk4->unk32 (SIGNED halfword)
nop
beq   $v0, $a1, .found
 addu $v0, $v1, $zero        ; delay slot: v0 = &self->arr[i] (only meaningful if branch taken)
addiu $a3, $a3, 0x1          ; i++
slti  $v0, $a3, 0x7
bnez  $v0, .loop
 addiu $a2, $a2, 0x1C        ; offset += 0x1C
.found:
jr $ra
 nop
```

On the "not found" (loop exhausted) path, `$v0` is whatever the last
comparison's `lh` produced — not a pointer. Retail's own source has no
explicit statement for that path either; see below.

## Final C

```c
Elem *StageMap__FindSlotByNeighbour(Obj866E8 *self, s32 key) {
    s32 i;
    Elem *e;

    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (e->unk4->unk32 == key) {
            return e;
        }
    }
}
```

No `return` after the loop — C89 permits falling off the end of a
non-`void` function (the caller's use of the result is then undefined
behavior, but the function itself compiles exactly as retail's own body
does: whatever's left in `$v0` from the last executed statement). This is
the natural, direct reproduction; anything else (an explicit `return NULL;`
or `return e;` after the loop) would add a instruction retail doesn't have.

**Head confirmation (round 7): that prediction was tested, not just
reasoned.** Rewriting the loop as `break;` plus a single terminal
`return e;` — the form that removes the undefined fall-through — drops the
score to 6/15 and adds exactly one instruction, `addu $v0, $a3, $zero`, at
`0x8004C46C` where retail has `nop`. So the UB-shaped body IS the faithful
reconstruction and must stay.

Worth recording what retail's not-found path actually does, since it is an
artifact rather than an intent: `addu $v0, $v1, $zero` sits in the `beq`'s
delay slot, so it executes on *every* iteration, and on loop exhaustion
`$v0` is left holding `&self->arr[6]` — the last element, not null. Any
caller relying on a null return would be relying on something retail does
not do. The original source almost certainly had no terminal return at all,
with the loop presumed always to find its key.

## Residue and how it closed (2 attempts)

First attempt typed `ElemTarget::unk32` as `u16`, producing `lhu` where
retail has `lh` (signed halfword load) — 14/15. Retyped to `s16`, matching
the SAME residue class already seen this round (`StageMap__FindSlotByNeighbour`'s own
comparison against a plain `s32 key`, decoded as signed). 15/15.

## New struct knowledge (`include/class_3bb8c.h`)

- `Elem::unk4` (`ElemTarget *`, +0x004) — a pointer to another object.
- New opaque type `ElemTarget`, only field known: `unk32` (`s16`, +0x032).

## Attempts

2 (see residue above).

### Proposed learning

None new — same signed/unsigned halfword lesson already documented
elsewhere this project (`TimedTask__CheckTimeout`, `class_39e08`).

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004C434` | `StageMap__FindSlotByNeighbour` | A | Occupant of `gStageMapMethods` +0x118 (`slot118`). Pure linear search: loops `self->arr[7]`, returns the first `Elem *` whose `unk4->unk32 == key`. Named to parallel the already-matched sibling `StageMap__FindSlotIndexByNeighbour` (+0x120), which searches the SAME field (`ElemTarget::unk32`) but returns an index rather than the element pointer -- consistent family naming for two functions doing the identical field comparison with a different return shape. A pure search-and-return is tier A by the "getter" clause. |
