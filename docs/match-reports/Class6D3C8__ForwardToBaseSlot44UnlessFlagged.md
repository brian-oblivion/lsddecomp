# Class6D3C8__ForwardToBaseSlot44UnlessFlagged

> Renamed from `Class6D3C8__ForwardToBaseUnlessOverridden` on 2026-09-24 (tools/rename.py). Address 0x80026108.

> Renamed from `func_80026108` on 2026-09-24 (tools/rename.py). Address 0x80026108.

**Unit:** code_1677c · **Size:** 26 instructions · **Status:** MATCHED (26/26 words)

## What it does

A method on the class at `D_8006D3C8` (slot `+0x044`) that falls back to
the base class's own implementation of the same slot when this object
hasn't been given an override (`self->unk18 == 0`).

## Derivation

```
addiu $sp, $sp, -0x20
sw    $s0, 0x10($sp)
addu  $s0, $a0, $zero      ; s0 = self
sw    $s1, 0x14($sp)
addu  $s1, $a1, $zero      ; s1 = a1
sw    $ra, 0x1c($sp)
sw    $s2, 0x18($sp)
lw    $v0, 0x18($s0)        ; v0 = self->unk18
nop
bnez  $v0, .L80026154        ; skip if unk18 != 0
 addu $s2, $a2, $zero          ; s2 = a2
jal   GetClass6E4F0Methods            ; -> &D_8006E4F0 (base class table)
 nop
lw    $v0, 0x44($v0)             ; base table slot +0x044
addu  $a0, $s0, $zero
addu  $a1, $s1, $zero
addu  $a2, $s2, $zero
jalr  $v0
 addu $a3, $zero, $zero
.L80026154:
... epilogue, no v0 setup ...
```

**This is the class-hierarchy evidence for this whole unit.** Resolving
`GetClass6E4F0Methods` (`tools/classtable.py 0x8006E4F0`) shows a 19-slot table
that is itself derived from BasicClass, and `classtable.py 0x8006D3C8 --vs
0x8006E4F0` shows this unit's class shares +0x00C/+0x048/+0x04C *exactly*
with that intermediate table — evidence this class's real parent is that
intermediate class, not BasicClass directly (the earlier `--vs 0x8006B58C`
comparison only showed a match on the BasicClass-common low slots, which
both classes inherit). Recorded in `include/Class6D3C8.h`.

No instruction sets `$v0` after the conditional call, so the function's own
return value (if used at all) is whatever the base method leaves behind —
treated as `void` here since nothing in this unit consumes it.

```c
void Class6D3C8__ForwardToBaseSlot44UnlessFlagged(Class6D3C8 *self, void *a1, void *a2) {
    if (self->unk18 == 0) {
        GetClass6E4F0Methods()->slot44(self, a1, a2, 0);
    }
}
```

## Proposed learning

`classtable.py <derived> --vs <hypothesized-base>` giving a short/patchy
match doesn't rule out inheritance — it may mean the diff is against the
wrong ancestor. When a derived table's *high* slots (added by the subclass)
line up byte-for-byte with a *different* candidate table's high slots too,
re-run `--vs` against that candidate; a longer identical run there is the
real parent. This class's real parent (D_8006E4F0, an intermediate between
BasicClass and this class) was only found this way.

## Naming

**`Class6D3C8__ForwardToBaseSlot44UnlessFlagged` -- tier B.** Mechanics are
clear from the body: when `self->unk18 == 0` it forwards straight to the
intermediate base class's own `slot44` occupant (`GetClass6E4F0Methods()->slot44`,
same slot number as the one this function itself occupies, `+0x044`), and
does nothing otherwise. What `unk18 != 0` actually MEANS in game terms (an
override flag set by some other, uncarved code path) is not established, so
the name describes the forwarding mechanism only, not why a caller would
set the flag.

**Field `Class6D3C8Methods.slot44` was NOT renamed** despite this
function's clear mechanics, because `src/main.c` (`main`, another
unit) dispatches it by field name directly
(`gClass6D3C8->methods->slot44(gClass6D3C8, obj, pad)`) -- renaming the
struct definition here would break that unit's build, which is outside this
runner's ownership. See `## Proposed field names` below.

**Head, round 77:** applied as `forwardToBaseSlot44UnlessFlagged` (the method was renamed at review: 'overridden' asserted a purpose the body does not show).

## Proposed field names

- **`Class6D3C8Methods.slot44` -> `forwardToBaseUnlessOverridden`**, tier B,
  same evidence as the function name above. Only accessor outside this unit
  is `src/main.c:60-61` (`gClass6D3C8->methods->slot44(gClass6D3C8, obj, pad)`),
  so the rename needs that call site updated in the same commit as the
  struct definition.
- **`Class6D3C8Methods.slot4C`** -- NOT proposing a name. Its occupant
  (`Class6E4F0__RunMainLoop`) is not in this unit and was not derived this round; all
  that's observable locally is the call shape at `src/main.c:61`
  (`gClass6D3C8->methods->slot4C(gClass6D3C8)`, no extra arguments, dispatched
  once right after `slot44` during startup). That's a call-site pattern, not
  a mechanics derivation of what the function itself does -- naming it from
  that alone would be the "guess at purpose" the naming rules warn against.
  Left as `slot4C` for whoever carves `Class6E4F0__RunMainLoop`'s own unit.

Posted to `tools/broadcast.sh post --from echo`.
