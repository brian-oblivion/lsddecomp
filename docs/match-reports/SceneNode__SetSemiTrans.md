# SceneNode__SetSemiTrans

> Renamed from `Class6B5CC__SetSemiTrans` on 2026-09-26 (tools/rename.py). Address 0x8001d374.

> Renamed from `func_8001D374` on 2026-09-23 (tools/rename.py). Address 0x8001d374.

**Unit:** SceneNode · **Size:** 11 words · **Status:** MATCHED (11/11 words)

## What it does

`SceneNode` vtable slot `+0x064`. Sets bit 30 (a 1-bit field) of
`self->unk10` to `(a1 != 0)`, tail-returning `GetSetBitField`'s result (the
field's previous value) directly. See `SceneNode__SetDisplay.md` for the shared
`GetSetBitField` background.

## The C

```c
u32 SceneNode__SetSemiTrans(SceneNodeObj *self, s32 a1) {
    return GetSetBitField(&self->unk10, 0x1E, 1, a1 != 0);
}
```

`a1 != 0` compiles to `sltu $r,$zero,$r` (the documented "sltu $zero,$r is
x!=0" lowering), matching retail's `sltu $a3,$zero,$a1`.

## Provenance

round 11 (2026-09-03), runner charlie, unit SceneNode (fresh carve, first attempt).
Matched on the first build (part of the five-function bitfield-setter
group; see `SceneNode__SetDisplay.md`).

## Naming

Round 71 (alpha). `func_8001D374` -> `SceneNode__SetSemiTrans`, **tier A**. Table slot +0x064. Sets attribute bit 30, GsALON (semi-transparency on), to on != 0 and returns the old bit. ObjMStyleActor calls the slot setSemiTrans.

Round 96 (alpha, track 6). The +0x064 slot is `setSemiTransOn`, not
`setSemiTrans`: <libgpu.h> defines the function-like macro
`setSemiTrans(p, abe)`, so `methods->setSemiTrans(self, 1)` expanded to Sony's
macro (a parse error) in every caller that takes Sony's headers
(ObjMStyleActor, class_3bb8c_s, screen_widgets). Sony keeps Sony's names, so the
slot moved; the method names do not collide and stay. Zero bytes.

## Round 101 (delta): track 7

Step 3 (locals and parameters): `a1` -> `on` (GsALON). Byte-identical.
