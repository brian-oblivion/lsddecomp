# Unk18Obj__SetGeomScreen — MATCHED

> Renamed from `func_8003F28C` on 2026-09-23 (tools/rename.py). Address 0x8003f28c.

Unit: `code_2cc8c_d`. Round 14, runner delta. 8/8 words, full match.

## Signature

```c
void Unk18Obj__SetGeomScreen(Unk18Obj *self);
```

Not a `D_8006E8E4` vtable slot — called directly by symbol.

## What it does

A thin wrapper forwarding `self` unexamined to a PsyQ library call, return
ignored.

```c
void Unk18Obj__SetGeomScreen(Unk18Obj *self) {
    func_80024B90(self);
}
```

## Header changes

`include/code_2cc8c.h`: new extern `func_80024B90(Unk18Obj *self)` (PsyQ
library, `asm/psyq_GsLinkObject4.s`, not decompiled in this project).
