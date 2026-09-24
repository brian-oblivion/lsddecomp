# Obj865C8__EnterState2 — MATCHED (33/33 words)

> Renamed from `func_80049E20` on 2026-09-23 (tools/rename.py). Address 0x80049e20.

## Disassembly shape

```
addiu $sp, $sp, -0x20
sw    $s0, 0x18($sp)
addu  $s0, $a0, $zero        ; s0 = self
sw    $ra, 0x1C($sp)
sw    $a1, 0x10($sp)         ; arg1 -> outgoing 5th arg slot for the call below
lw    $a0, 0x34($s0)         ; self->subB
lw    $a1, 0x40($s0)         ; self->unk40
lw    $a2, 0x44($s0)         ; self->unk44
lw    $a3, 0x48($s0)         ; self->unk48
jal   func_80052B70
 nop
lw    $v1, 0x0($s0)          ; self->methods
addu  $a0, $s0, $zero
sw    $v0, 0x4C($s0)         ; self->unk4C = result
lw    $v1, 0x10($v1)         ; methods->slot10
jalr  $v1
 addu $a1, $v0, $zero        ; a1 = result (still in $v0, not reloaded)
lw    $a0, 0x4C($s0)         ; self->unk4C (reloaded, clobbered by the call)
lw    $a1, 0xC($s0)          ; self->unk0C
lw    $v0, 0x0($a0)          ; unk4C->methods
lw    $a2, 0x38($s0)         ; self->unk38
lw    $v0, 0x44($v0)         ; unk4C->methods->slot44
jalr  $v0
 nop
ori   $v0, $zero, 0x2
sw    $v0, 0x3C($s0)         ; self->unk3C = 2
...
jr    $ra
```

The key to the residue-free read: `Obj865C8__EnterState2`'s own incoming `arg1`
($a1) is stored at `0x10($sp)` in the prologue — not as a dead argument
spill, but because that slot IS the o32 outgoing-argument home for a call's
5th parameter, and `func_80052B70` takes 5 args (4 in `$a0-$a3`, the 5th on
the stack, confirmed from `func_80052B70`'s own prologue in
`asm/class_3bb8c.s`, which loads its 5th param from `0x48($sp)` against its
own `0x38`-byte frame). `arg1` is silently forwarded, never touched by name
inside this function's body.

## Final C

```c
void Obj865C8__EnterState2(Obj865C8 *self, s32 arg1) {
    self->unk4C = func_80052B70(self->subB, (s32)self->unk40, (s32)self->unk44, (s32)self->unk48, arg1);
    self->methods->slot10(self, self->unk4C);
    self->unk4C->methods->slot44(self->unk4C, (s32)self->unk0C, (s32)self->unk38);
    self->unk3C = 2;
}
```

(The `(s32)` casts were added across two later passes in round 2026-09-02
— `unk0C`/`unk38` when `Obj865C8__Deinit` proved those two fields are really
pointers, and `unk40`/`unk44`/`unk48` when `Obj865C8__Dtor` proved those
three are too. See "Correction" below. This snippet reflects the CURRENT
committed source.)

## New struct knowledge (`include/class_39e08.h`)

- `Obj865C8::unk0C`, `unk38`, `unk40`/`unk44`/`unk48`: at the time this
  function was matched, all five were typed `s32` — this function's own
  call sites only ever forward them as opaque register values (never
  dereferencing them), consistent with either a scalar or a pointer. Since
  corrected (see "Correction" below): `unk0C` is `Obj0C *`, `unk38` is
  `SubObjD *`, and `unk40`/`unk44`/`unk48` are all `SubObjG *`. `unk4C` was
  typed `Obj4C *` (the object `func_80052B70` returns) from the start,
  unaffected.
- `Class865C8Methods::slot10` typed `void (*)(Obj865C8 *self, Obj4C *arg1)`
  — a BasicClass-inherited slot (`BasicClass__func_17f98`, same address
  `Class6D3C8.h` already lists at its own local `+0x010` as an untyped
  `unk10`; not reconciled there per this project's per-unit local-view
  convention).
- New opaque type `Obj4C`/`Obj4CMethods`: the object `func_80052B70`
  allocates and returns, dispatched through at `+0x044` only
  (`Obj865C8::unk4C->methods->slot44(unk4C, self->unk0C, self->unk38)`).
  Same "vtable at offset 0, only the reached slot named" policy as this
  unit's existing `SubObjA`/`SubObjB`.
- `func_80052B70` (uncarved, unit `class_3bb8c`) declared locally:
  `Obj4C *func_80052B70(SubObjB *a0, s32 a1, s32 a2, s32 a3, s32 a4);` — its
  own disassembly is a `New_X`-shaped allocator (0x88-byte alloc via
  `BMemPMgrAlloc`, ctor via `GetObjMMethods`, then dispatches its new
  object's own +0x008 slot with all 5 forwarded args, or returns 0 if the
  allocation failed).

## Attempts

1 (matched on first attempt, once the o32 stack-argument convention
explained the otherwise-dead-looking `arg1` store).

### Proposed learning

An incoming register argument stored to `sp+0x10` in the prologue with no
later load from that slot is not necessarily a dead/unused-parameter spill —
check whether the same offset is the o32 outgoing-argument home (args 5+ go
at `$sp+0x10` in the CALLER's own frame) for a call made later in the same
function. Here it was `Obj865C8__EnterState2` silently forwarding its own 2nd
parameter as the 5th argument to `func_80052B70`. Worth checking cross-call
argument counts (via the callee's own prologue, e.g. how far up its stack it
loads incoming args from) before writing off such a store as inert.

## Correction (added in round 2026-09-02, not a rewrite of the above)

All five of this function's opaquely-forwarded fields (`unk0C`, `unk38`,
`unk40`, `unk44`, `unk48`) were later proven to be real pointer types by
functions that DO dereference them: `Obj865C8__Deinit`/`Obj865C8__Init` settled
`unk0C` (`Obj0C *`) and `unk38` (`SubObjD *`); `Obj865C8__Dtor` settled
`unk40`/`unk44`/`unk48` (all `SubObjG *`). This function's own derivation
above is left as originally written — it was, and remains, an accurate
account of what THIS function's disassembly alone shows, which cannot
distinguish a forwarded scalar from a forwarded pointer. The struct field
types in `include/class_39e08.h` and this function's own call sites (both
the `func_80052B70` call and the `slot44` call) now carry explicit `(s32)`
casts to preserve the byte-identical register-passthrough behavior; funcdiff
was reconfirmed at 33/33 after each retyping pass.

## Naming

`Obj865C8__EnterState2` -- tier B. Unconditionally sets `state = 2` at the end and constructs a new object via `func_80052B70`, stored at `unk4C`; called from both `Obj865C8__AdvanceState` and `Obj865C8__OnTag2Notify`. Named for the one state transition its body always performs.
