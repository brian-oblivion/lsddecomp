# Class86668__SetState

> Renamed from `Class86668__OnEventArg` on 2026-09-25 (tools/rename.py). Address 0x8004a3ec.

> Renamed from `Obj865C8__OnEventArg` on 2026-09-23 (tools/rename.py). Address 0x8004a3ec.

> Renamed from `func_8004A3EC` on 2026-09-23 (tools/rename.py). Address 0x8004a3ec.

**Unit:** class_39e08 · **Size:** 27 words (0x6C bytes) · **Status:** MATCHED (27/27 words)

## What it does

Method-table slot +0x060, shared VERBATIM (same function address) between
`gClass865C8Methods` and its sibling `gClass86668Methods` -- confirmed by
`tools/classtable.py`, both tables list `Class86668__SetState` at +0x060. Always
forwards to the BASE class's own +0x060 implementation first (return value
discarded), then -- based on the ORIGINAL argument, not the base call's
return -- sets a flag and notifies through the class's own +0x07C slot.

## Derivation

```
jal   Get_vtable_IntermediateBase
 move $s0, $a1              ; s0 = original arg1, saved across the call
move  $a0, $s1               ; a0 = self
lw    $v0, 0x60($v0)
jalr  $v0
 move $a1, $s0                ; a1 = arg1, forwarded unchanged
li    $v0, 4
bne   $s0, $v0, .L8004A440    ; compares the SAVED arg1, not the call's return
 li   $v1, 1
lw    $v0, 0x0($s1)
sw    $v1, 0x28($s1)
lw    $v0, 0x7C($v0)
jalr  $v0
 move $a0, $s1
.L8004A440:
```

Written as:

```c
void Class86668__SetState(Obj865C8 *self, s32 arg1) {
    Get_vtable_IntermediateBase()->slot60(self, arg1);
    if (arg1 == 4) {
        self->unk28 = 1;
        self->methods->noop7C(self);
    }
}
```

`IntermediateBaseMethods::slot60` is typed as a discarded-return call
(`void`, `void *self, s32 arg1`), since the compiled `$v0` from the `jalr`
is overwritten by `li $v0,4` before ever being read -- the base call's
return value is genuinely unused here, not just unread by this residue-free
match.

## Proposed learning

**A function can occupy the same vtable slot in two sibling classes' tables
and still read/write instance fields — this is only safe (and only proof of
a real shared base layout) when the fields it touches (`unk28` here) are
confirmed to exist at the same offset in both.** Worth checking before
reusing a shared-slot function's disassembly as "generic" — if it touched a
field unique to one subclass, sharing the slot would be a decompilation bug,
not a real retail fact.

## Naming

`Class86668__OnEventArg` -- tier B. Occupies +0x060 (matches the header's pre-existing `onEventArg` field name, shared verbatim with `gClass86668Methods` at the same offset). Forwards to the base, and on `arg1 == 4` (the code `CheckTimeout` raises) sets `eventCode = 1` and calls `Noop7C`.

## Track 4

2026-09-25, round 84 (bravo): class unified in `include/Class86668.h`. Renamed from `Class86668__OnEventArg`, tier A: it occupies +0x060, IntermediateBase's `setState` slot (IntermediateBase__SetState in the parent table), and its first call is the base setState with the same argument. For state 4 it also sets `result` (was `eventCode`) = 1 and calls +0x07C, now named `onState4` (NULL in this class's own 0x80-byte table; empty Class865C8__OnState4 and ObjM__NoOpSlot7C in the subclasses). State 4 is what Class86668__CheckTimeout raises. Signature `(Class86668 *self, s32 state)`. Image byte-identical.
