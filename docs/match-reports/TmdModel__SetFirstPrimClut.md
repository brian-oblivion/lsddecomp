# TmdModel__SetFirstPrimClut -- MATCHED (14/14 words), round 82

> Renamed from `SetTargetOffset` on 2026-09-27 (tools/rename.py). Address 0x80020510.

> Renamed from `func_80020510` on 2026-09-25 (tools/rename.py). Address 0x80020510.

Round 82, runner charlie (matching slot). Unit `src/TmdModel.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table; called from class_3bb8c_k.c as `TmdModel__SetFirstPrimClut(obj, &gStyleEffectClutPos)` (`tools/classtable.py gTmdModelMethods`).
- **What:** `t = self->unk10->unk10; v = xy[0] / 16; t->unk6 = v; t->unk6 = v + xy[1] * 64;`: a double store to one s16 field, the second one reusing the first value without reloading it.
- **Result:** byte-exact; 14/14 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views (`TmdModel`, `ModelData_fa50`, `Quad_fa50`, `TmdObject_fa50`, `Outer_fa50`/`Inner_fa50`/`Target_fa50`) and prototypes live in the unit; no shared header was touched.
- **Lever:** `sh v; ...; addu v, v, x; sh v` to the SAME field with no `lh` in between comes from a named `s32` local stored twice. `t->unk6 += ...` would reload the truncated s16.

## Source

```c
void TmdModel__SetFirstPrimClut(Outer_fa50 *self, s16 *xy) {
    Target_fa50 *t = self->unk10->unk10;
    s32 v;

    v = xy[0] / 16;
    t->unk6 = v;
    t->unk6 = v + xy[1] * 64;
}
```

## Naming

`TmdModel__SetFirstPrimClut` -- KEPT (not renamed this round). Tier C: mechanics known
(`t->unk6 = xy[0]/16; t->unk6 = ... + xy[1]*64;`, a double store overwriting
the field), but its only caller is `src/class_3bb8c_k.c`, a live types-runner
unit this round; the class owning the `Outer_fa50`/`Inner_fa50`/`Target_fa50`
chain is itself unconfirmed (see `TmdModel__AddFirstPrimClut.md`, its sibling).

## Proposed name

`TmdModel__SetFirstPrimClut` -- tier B, discriminating it from its sibling
`TmdModel__AddFirstPrimClut` (renamed this round, no collision): this one SETS
the field from a fresh value, the sibling ACCUMULATES onto the existing
one. Posted to the broadcast for the head to apply once `class_3bb8c_k.c`
is not live and the owning class is known.

## Track 7 (2026-09-26, round 94, bravo)

Named the fields (see `TmdModel__AddFirstPrimClut.md`'s matching entry for the
full rationale; both functions share the same three structs and both are
the only readers/writers, all inside this unit): `Target_fa50::unk6` ->
`offset`, `Inner_fa50::unk10` -> `target`, `Outer_fa50::unk10` -> `inner`.
`class_3bb8c_k.c`'s call site (`TmdModel__SetFirstPrimClut(obj, &gStyleEffectClutPos)`) passes
opaque pointers and never names these fields itself, so the field rename
does not touch it. Compiler-verified accessor list, build and
check-nonmatching.sh green.

## Naming

Track 6 (2026-09-27, round 95, delta): the `Outer_fa50`/`Inner_fa50`/
`Target_fa50` chain is TmdModel -> TmdObject -> the first TMD primitive, and
the field is that primitive's CLUT id. All three placeholder types are
deleted.

- **`Outer_fa50` -> `TmdModel`** (`inner` -> `object`). The one caller,
  `SetStyleEffectSources` (class_3bb8c_k.c), passes the value
  `gStyleEffectTmd`'s slot +0x080 returns for a model id; the same slot's
  result for the same `gStyleEffectModelIds[]` is what
  `StyleEffect__InitByKind` (class_3bb8c_k.c) hands to
  `SceneNode__LinkModel`, which reads it as a `TmdModel` (`->object`,
  `->data->objects`). TmdModel's +0x010 is `object`.
- **`Inner_fa50` -> `TmdObject`** (`target` -> `prims`): TmdObject's +0x010
  is its primitive list.
- **`Target_fa50` -> Sony's `TMD_P_TF3`** (`offset` -> `clut`): +0x006 of
  every textured TMD packet (`TMD_P_TF3`/`TF4`/`TNF3`/..., `<libgs.h>`) is
  `u_short clut`, after `tu0, tv0`. The value stored is `x / 16 + y * 64`,
  which is libgpu's `getClut(x, y)` (`(y << 6) | ((x >> 4) & 0x3f)`) for a
  non-negative 16-aligned x; and the caller's argument `gStyleEffectClutPos` is the
  pair `{0x3F0, 0x1FF}`, VRAM (1008, 511), a standard CLUT position (its
  CLUT id is 0x7FFF). The macro itself is not used: its shift and OR are not
  the division and add retail compiles.

Byte-identical (`lhu`/`sh` unchanged: `u_short` loads as `lhu`, as the s16
placeholder did in its sibling).

Function name kept here, renamed separately with `tools/rename.py` (see the
next entry if present).
