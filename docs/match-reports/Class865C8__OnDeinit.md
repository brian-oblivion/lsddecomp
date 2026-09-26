# Class865C8__OnDeinit

> Renamed from `Obj865C8__RunSubUpdates` on 2026-09-26 (tools/rename.py). Address 0x80049c50.

> Renamed from `func_80049C50` on 2026-09-23 (tools/rename.py). Address 0x80049c50.

**Unit:** class_39e08 · **Size:** 22 words (0x58 bytes) · **Status:** MATCHED (22/22 words)

## What it does

Method-table slot +0x050 of `gClass865C8Methods` (`Obj865C8`, see
`include/class_39e08.h`). Reads `self->subA`, then dispatches two calls
through THAT sub-object's own vtable, slots +0x090 then +0x074, both with
just the sub-object as argument.

## Derivation

```
lw    $s0, 0x18($a0)     ; s0 = self->subA
lw    $v0, 0x0($s0)      ; v0 = subA->methods
lw    $v0, 0x90($v0)
jalr  $v0
 move $a0, $s0
lw    $v0, 0x0($s0)      ; reload subA->methods
lw    $v0, 0x74($v0)
jalr  $v0
 move $a0, $s0
```

Written as:

```c
void Class865C8__OnDeinit(Obj865C8 *self) {
    SubObjA *sub = self->subA;

    sub->methods->slot90(sub);
    sub->methods->slot74(sub);
}
```

`SubObjA` is an opaque, minimally-typed view (vtable pointer at offset 0,
only the two slots this function dispatches through named) -- same policy as
`DreamSysEntityObj` in `include/DreamSys.h`. Nothing here identifies which
concrete class `subA` points to; both slot numbers exceed `gClass865C8Methods`'s own
33-slot table, so it is a genuinely different class, not a self-dispatch.

## Proposed learning

None beyond what's already documented.

## Naming

`Class865C8__OnDeinit` -- tier A. Ticks `subA` (`slot90`/`slot74`) every call; matches the existing `runSubUpdates` field name already on file. A pure per-frame forwarding leaf: mechanics are its purpose.
