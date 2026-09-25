# Class86668__PlaySound

> Renamed from `Class86668__SetChildFlag8` on 2026-09-25 (tools/rename.py). Address 0x8004a478.

> Renamed from `func_8004A478` on 2026-09-22 (tools/rename.py). Address 0x8004a478.

**Unit:** class_3ac78 · **Size:** 16 words · **Status:** MATCHED (16/16 words)

## What it does

`Class86668`'s own slot +0x070 method. Reads the `Class866E8` instance held at
`self->unk34`; if non-NULL, forwards its own second parameter plus two literal
`0x7F` values into that sub-object's vtable slot +0x080.

## Derivation

```
lw    $a0, 0x34($a0)        ; a0 = self->unk34 (overwrites self)
beqz  $a0, .L8004A4A8
 ori  $a2, $zero, 0x7F
lw    $v0, 0x0($a0)         ; v0 = sub->methods
lw    $v0, 0x80($v0)        ; v0 = sub->methods->slot80
jalr  $v0
 ori  $a3, $zero, 0x7F
```

Resolved via `tools/classtable.py gClass86668Methods` and `tools/classtable.py
D_800866E8`: `gClass86668Methods` (28 slots, header 0x230) is this function's own
containing class's vtable, and this IS its last slot (+0x070) — the only
reason this class is visible from this unit at all. `D_800866E8` (80 slots,
header 0x114) is the vtable of the sub-object at `self->unk34`; slot +0x080
there is `Class6B5CC__GetSetUnk10Flag8` (outside this unit, in `code_d294.s`), which reads
its own incoming `$a0`/`$a1`/`$a2` (three real params) and ignores a fourth —
consistent with the call here supplying four register args.

**The second parameter is real, not leftover garbage.** The call sets `$a2`
and `$a3` to literal `0x7F` but leaves `$a1` untouched. MIPS's calling
convention fills `$a0..$a3` in order with no gaps: if the call had only three
real arguments (self + two literals), the literals would occupy `$a1`/`$a2`,
not `$a2`/`$a3`. Since they demonstrably occupy the *third* and *fourth*
argument slots, `$a1` must be a real (fourth total, second real) argument —
which can only be `Class86668__PlaySound`'s own second parameter, forwarded unchanged
(no `move` needed since it's already resident in the right register).

Named the two classes by vtable address per project convention (see
`Class6D3C8.h`): `Class86668` (gClass86668Methods) and `Class866E8` (D_800866E8).
Both declared in the new `include/class_3ac78.h`.

## Proposed learning

When a vtable-dispatch call sets later argument registers (`$a2`/`$a3`) to
literals but leaves an earlier one (`$a1`) untouched, that's proof the earlier
register carries a real forwarded parameter, not coincidental leftover state —
MIPS's strict left-to-right register-filling convention means there's no
"skip a register" call shape. This resolves an ambiguity that pure "no
instruction was needed either way" reasoning can't: a caller with no argument
in that slot at all would use `$a1`, not `$a2`, for its first literal.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004A478` | `Class86668__SetChildFlag8` | B | `gClass86668Methods`'s own last slot (`+0x070`). Reads the child object at `self->unk34`, and if non-NULL dispatches its `+0x080` slot. In `Class866E8` that slot is the inherited `Class6B5CC__GetSetUnk10Flag8` (matched, `code_d294_b`), a get-or-set of bit 8 of the object's `unk10` bitfield, taking `(self, value)` -- so the two literal `0x7F`s this call sets up are not read by that occupant. Tier B, and prefixed with the containing class rather than the callee's: the name says what this method does (push a flag down to the held child), not what the child's class is. |

Deliberately NOT asserted: that `Class86668::unk34` is a `Class866E8`. The
header claims it, but the only evidence is that slot `+0x080` exists in
`D_800866E8` -- true of every `Class6B5CC` descendant. The name avoids
depending on it.

The two `0x7F` arguments are left in the call: they cost nothing, they are
what retail's caller sets up, and other occupants of slot `+0x070` in this
table family may read them.
