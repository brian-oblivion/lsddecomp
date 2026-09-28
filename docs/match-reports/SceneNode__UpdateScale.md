# SceneNode__UpdateScale -- MATCHED (57/57 words)

> Renamed from `Class6B5CC__UpdateScale` on 2026-09-26 (tools/rename.py). Address 0x8001d008.

> Renamed from `func_8001D008` on 2026-09-23 (tools/rename.py). Address 0x8001d008.

Unit: `code_d294` (round 14). Occupies `SceneNodeMethods` vtable slot
`+0x048` (already typed as `slot48` before this round, per the header's
own note pointing at this function). Reads a 3-entry `{s16 whole; s16
frac}` fixed-point table via three calls to `RatioToFixed12`, then either
overwrites or accumulates the (16-bit-truncated) results into
`self->unk14->unk44`'s first three words. `void SceneNode__UpdateScale(SceneNodeObj
*self, s32 flag, void *data)`.

## Final source

```c
void SceneNode__UpdateScale(SceneNodeObj *self, s32 flag, void *data) {
    s32 r0, r1, r2;
    SceneNodeSub44 *dst;

    r0 = RatioToFixed12(data);
    r1 = RatioToFixed12((u8 *)data + 4);
    r2 = RatioToFixed12((u8 *)data + 8);
    dst = self->unk14->unk44;
    if (flag) {
        dst->unk0 = (s16)r0;
        dst->unk4 = (s16)r1;
        dst->unk8 = (s16)r2;
    } else {
        dst->unk0 += (s16)r0;
        dst->unk4 += (s16)r1;
        dst->unk8 += (s16)r2;
    }
    self->unk14->unk0 = 0;
}
```

## New struct knowledge

Retypes `self->unk14->unk44` from opaque `void *` to the new
`SceneNodeSub44 *` type (see `SceneNode__UpdateRotation`'s report and
`include/code_d294.h`, filled in jointly by this function and
`SceneNode__UpdateRotation` -- documented once there to avoid duplicating the same
struct comment across two reports).

## Derivation notes

- The `(s16)` cast on each `RatioToFixed12` result is load-bearing: retail
  writes each result through a `sll 16`/`sra 16` truncate-then-sign-extend
  pair before the `sw` (a full-word store of the sign-extended 16-bit
  value, not a halfword store) -- `dst->unk0` etc. are genuine `s32`
  fields per their own `sw`/`lw` accesses elsewhere, so the cast belongs on
  the SOURCE value, not the destination type.
- `flag != 0` selects overwrite (`dst->fieldN = (s16)rN`); `flag == 0`
  selects accumulate (`dst->fieldN += (s16)rN`, i.e. `dst->fieldN =
  dst->fieldN + (s16)rN` -- old value read, added, stored back). No
  residue -- matched clean on the first attempt.
- `self->unk14->unk0 = 0` at the end matches the header's own standing
  note ("SceneNode__UpdateRotation/SceneNode__UpdateScale ... own tails both end
  `self->unk14->unk0 = 0`").
- `(u8 *)data + N` pointer arithmetic on the `void *data` parameter needs
  an explicit cast (arithmetic directly on `void *` is not standard C89);
  `RatioToFixed12` itself takes `void *`, so the cast result converts back
  implicitly at the call site.

No other new struct or vtable-slot knowledge; `slot48`'s own signature was
already correct from earlier work.

## Naming

Round 71 (alpha). `func_8001D008` -> `SceneNode__UpdateScale`, **tier A**. Table slot +0x048 (`updateScale`). Three RatioToFixed12 values assigned (flag != 0) or added into GsCOORD2PARAM.scale.vx/vy/vz (SceneNodeSub44 unk0/4/8), then flg = 0. Reset passes SCALE_ONE ({1/1}x3), i.e. unit scale.

## Round 97 (alpha): Sony's GsCOORD2PARAM

SceneNodeSub44 is deleted: coord2->param is Sony's GsCOORD2PARAM (VECTOR scale, SVECTOR rotate, VECTOR trans; 0x28 bytes, offset for offset), so `scaleX`/`scaleY`/`scaleZ` read `scale.vx`/`.vy`/`.vz`. Byte-identical.

## Round 101 (delta): track 7

Step 2: the three raw offsets `(u8 *)data + 4` / `+ 8` read the table as what it is, Ratio16[3] (include/SceneNode.h), through a local `Ratio16 *ratios = data`: `&ratios[0]`, `&ratios[1]`, `&ratios[2]`. The parameter stays `void *` because the prototype and the slot type in include/SceneNode.h say so (not this unit's to change; proposed). Byte-identical.

Step 3 (locals and parameters): `flag` -> `set`, `data` -> `table`, `r0`/`r1`/`r2` -> `sx`/`sy`/`sz`, `dst` -> `param`. Byte-identical.
