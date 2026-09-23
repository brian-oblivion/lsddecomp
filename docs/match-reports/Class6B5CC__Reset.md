# Class6B5CC__Reset

> Renamed from `func_8001CE30` on 2026-09-23 (tools/rename.py). Address 0x8001ce30.

**Unit:** code_d294 · **Size:** 33 words · **Status:** MATCHED (33/33 words)

## What it does

`Class6B5CC` vtable slot `+0x040`, this class's own init hook -- the
ctor's (`Class6B5CC__Class6B5CC`) last action before returning `self`. Zeroes
`self->unk24` and `self->unk10` (the packed bit-flags word the five
`func_8001D3xx` setters operate on -- confirms it's meant to start at
zero), copies a fixed engine matrix/table into `self->unk14` via the
Psy-Q library helper `func_80012838` (`the 0x2258..0x8220 Psy-Q block (now linked from lib/, formerly asm/psyq_2258.s)`, not decompiled --
out of game-code scope), then calls its own two newly-discovered slots
`+0x044`/`+0x048` (`Class6B5CC__UpdateRotation`/`Class6B5CC__UpdateScale`, both still queued)
with a literal flag `1` and one of two rodata tables
(`ROTATION_ZERO`/`SCALE_ONE`), and finally sets `self->unk14->unk0 = 1`.

## The C

```c
void Class6B5CC__Reset(Class6B5CCObj *self) {
    self->unk24 = 0;
    self->unk10 = 0;
    func_80012838(0, self->unk14);
    self->methods->slot44(self, 1, ROTATION_ZERO);
    self->methods->slot48(self, 1, SCALE_ONE);
    self->unk14->unk0 = 1;
}
```

## Provenance

round 11 (2026-09-03), runner charlie, unit code_d294, second pass. Matched on the first build.
Established `Class6B5CCMethods::slot44`/`slot48` (`(self, s32, void*)`,
still-queued occupants `Class6B5CC__UpdateRotation`/`Class6B5CC__UpdateScale`) and the
`ROTATION_ZERO`/`SCALE_ONE` rodata tables (0xC bytes each, shape confirmed
independently by `Class6B5CC__UpdateScale`'s own disassembly reading three
`{s16,s16}` pairs out of its 3rd argument via `RatioToFixed12`).
