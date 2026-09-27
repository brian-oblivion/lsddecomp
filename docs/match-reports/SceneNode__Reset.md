# SceneNode__Reset

> Renamed from `Class6B5CC__Reset` on 2026-09-26 (tools/rename.py). Address 0x8001ce30.

> Renamed from `func_8001CE30` on 2026-09-23 (tools/rename.py). Address 0x8001ce30.

**Unit:** code_d294 · **Size:** 33 words · **Status:** MATCHED (33/33 words)

## What it does

`SceneNode` vtable slot `+0x040`, this class's own init hook -- the
ctor's (`SceneNode__SceneNode`) last action before returning `self`. Zeroes
`self->unk24` and `self->unk10` (the packed bit-flags word the five
`func_8001D3xx` setters operate on -- confirms it's meant to start at
zero), copies a fixed engine matrix/table into `self->unk14` via the
Psy-Q library helper `func_80012838` (`the 0x2258..0x8220 Psy-Q block (now linked from lib/, formerly asm/psyq_2258.s)`, not decompiled --
out of game-code scope), then calls its own two newly-discovered slots
`+0x044`/`+0x048` (`SceneNode__UpdateRotation`/`SceneNode__UpdateScale`, both still queued)
with a literal flag `1` and one of two rodata tables
(`ROTATION_ZERO`/`SCALE_ONE`), and finally sets `self->unk14->unk0 = 1`.

## The C

```c
void SceneNode__Reset(SceneNodeObj *self) {
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
Established `SceneNodeMethods::slot44`/`slot48` (`(self, s32, void*)`,
still-queued occupants `SceneNode__UpdateRotation`/`SceneNode__UpdateScale`) and the
`ROTATION_ZERO`/`SCALE_ONE` rodata tables (0xC bytes each, shape confirmed
independently by `SceneNode__UpdateScale`'s own disassembly reading three
`{s16,s16}` pairs out of its 3rd argument via `RatioToFixed12`).

## Naming

Round 71 (alpha). `func_8001CE30` -> `SceneNode__Reset`, **tier B**. Table slot +0x040, called last by the ctor. Zeroes `tick` and the GsDOBJ2 attribute, GsInitCoordinate2(0, coord2) (no parent), sets rotation to ROTATION_ZERO ({0/1}x3) and scale to SCALE_ONE ({1/1}x3), flg = 1. Mechanics are a full reset to an identity transform; tier B because what the game uses a re-run of slot +0x040 for is not established here. Subclass overrides of this slot are named FinishConstruct / Reset / InitDefaults / InitState in other units, all consistent.

## Round 95 (bravo): Sony's declarations

`GsInitCoordinate2` now comes from `<libgs.h>`, `(GsCOORDINATE2 *super,
GsCOORDINATE2 *base)`. The call is `GsInitCoordinate2(NULL, (GsCOORDINATE2
*)self->coord2)`: the cast stands until SceneNode.h's SceneNodeSub14 (which
is GsCOORDINATE2 offset for offset) becomes Sony's type. Byte-identical.
