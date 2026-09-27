# SceneNode__LinkModel -- MATCHED (16/16 words)

> Renamed from `Class6B5CC__LinkModel` on 2026-09-26 (tools/rename.py). Address 0x8001e770.

> Renamed from `func_8001E770` on 2026-09-17 (tools/rename.py). Address 0x8001e770.

Unit: `code_d294_c` (round 14). Called by `SceneNode__AddChild` (`code_d294.c`)
as its conditional forward target when `other`'s vtable-header tag is 9.
Stores `other` into `self->unk20`, copies one field out of it, then calls
the Psy-Q `GsLinkObject4` through a pointer computed off `other->unkC`.
`void SceneNode__LinkModel(SceneNodeObj *self, GenericObj_d294 *other)`.

## Final source

```c
void SceneNode__LinkModel(SceneNodeObj *self, GenericObj_d294 *other) {
    self->unk20 = other;
    self->unk18 = other->unk10;
    GsLinkObject4((u8 *)((GenericObj_d294 *)self->unk20)->unkC + 0xC, &self->unk10, 0);
}
```

## New struct/extern knowledge (`include/code_d294.h`, additive)

- **`GenericObj_d294` gains `unk10`** (`s32`), immediately following the
  already-known `unkC` (`void *`) with no gap -- read here into
  `self->unk18`.
- **New extern for `GsLinkObject4`** (`void(void*, void*, s32)`), the raw
  Psy-Q symbol (`config/symbols.slps01556.lsdde.txt`, `0x8001EF70`,
  segment `psyq_GsLinkObject4` immediately after this carve) -- not
  previously given its own prototype (only its wrapper `TmdModel__GetHull`
  was declared). Declared with only the shape this one call site needs.

No existing field was retyped or renamed; `SceneNode__UnlinkModel`'s comment in the
header was also updated (round 14) to drop "still uncarved" now that both
functions have moved into this unit -- prototype/type unchanged.

## Derivation notes

- **Residue: reading `other->unkC` directly (rather than through
  `self->unk20`, which was just assigned `other` one statement earlier)
  scored 2/16, with the whole function drifting to the wrong length.**
  Retail reloads `self->unk20` (a `lw a0, 0x20($v0)` right after the
  store) instead of reusing the register that already held `other` --
  the classic "retail re-reads a value it just wrote instead of keeping
  the argument register live" shape. Rewriting the `GsLinkObject4` call's
  first argument to go through `self->unk20` (cast back to
  `GenericObj_d294 *`) instead of the `other` parameter directly matched
  clean, register swap included: retail's `$v0`/`$v1` roles (self in
  `$v0`, the loaded `unk10` value in `$v1`) fell out naturally once the
  source read through the field instead of the parameter.
- The pointer arithmetic `(u8 *)(...)->unkC + 0xC` matches retail's
  `lw a0, 0xC($a0); addiu a0, a0, 0xC` -- read `other->unkC`, then advance
  it 12 bytes; `GsLinkObject4`'s own body is out of scope (Psy-Q library).

### Proposed learning

**When a function stores a parameter into a field and then immediately
uses that SAME value again, check whether retail re-reads it through the
field rather than reusing the parameter's own register.** This is a
sibling of the already-promoted "direct calls beat caching a resolved
pointer" learning from `SceneNode__OnNotify`: GCC 2.6.3 does not always keep a
just-stored value live in its original register when a memory read of the
same value is available and the source re-mentions the field rather than
the parameter -- write the SOURCE the way retail's disassembly reads
(through the field), not the way that looks most natural (through the
still-in-scope parameter), when the two are interchangeable in value but
not in registers.

## Naming (round 50, charlie -- FINISHING-PLAN track 3)

- **`func_8001E770` -> `SceneNode__LinkModel`. Tier A**, and the evidence
  is Sony's own API rather than a reading of this body. `&self->unk10` is
  passed to `GsLinkObject4` as its GsDOBJ2 argument; `GsLinkObject4(tmd,
  objp, n)` links object `n` of a TMD to `objp`. That only type-checks if
  SceneNodeObj has a GsDOBJ2 at +0x10, and it does, field for field:
  +0x10 attribute (the packed flags word the `GetSetBitField` wrapper family
  sets), +0x14 coord2 (the GsCOORDINATE2 this class allocates in its ctor
  and initialises with `GsInitCoordinate2`), +0x18 tmd -- which is precisely
  the field this function writes, one line before the call.
- The `+ 0xC` on the first argument is the TMD file header skip (id, flags,
  nobj), which is what makes the "model data" reading concrete rather than
  generic.
- **`other` is not narrowed.** Other units call this symbol with a different
  class as the receiver and a plain `s32` second argument
  (`include/code_55dd4.h`, `src/class_3bb8c_s.c`); the `SceneNode__` prefix
  is still right, because any caller must carry the GsDOBJ2 layout this body
  dereferences.

## Track 4 (2026-09-25, round 81, charlie)

The signature is now `(SceneNode *self, void *model)`. The argument is the
class-9 child (gTmdModelMethods) that SceneNode__AddChild tag-tests. That class
has no C yet, so the two words LinkModel reads live in a local
`ModelObj_d294` view in `src/code_d294_c.c`: `+0x0C tmdFile` and `+0x10 tmd`.
The object fields are renamed: +0x18 `unk18` is now `tmd` (GsDOBJ2.tmd), and
+0x20 `unk20` is now `model`. The local externs in `include/code_55dd4.h` and
`src/class_3bb8c_s.c`, `(void *self, s32 arg)`, are deleted, and their
callers upcast. Byte-identical.

## Track 4 (2026-09-26, round 87, delta)

The local `ModelObj_d294` view (`tmdFile` at +0x00C, `tmd` at +0x010) was a
view of TmdModel and is deleted; the body now reads the class header
(include/TmdModel.h), byte-identical:

```c
void SceneNode__LinkModel(SceneNode *self, void *model) {
    self->model = model;
    self->tmd = (s32)((TmdModel *)model)->object;
    GsLinkObject4(((TmdModel *)self->model)->data->objects, &self->attribute, 0);
}
```

`data->objects` is `data + 0xC` (TmdFile's object table), the old
`tmdFile + 0xC`. The `void *model` parameter and SceneNode's `void *model`
field (+0x020) are SceneNode's to retype (`struct TmdModel *`); proposed,
not done here.

## Round 95 (bravo): Sony's declarations

`GsLinkObject4` now comes from `<libgs.h>`: `(unsigned long tmd_base, GsDOBJ2
*objp, int n)`. The call casts `data->objects` to `u_long` (Sony types the
TMD base as an address) and `&self->attribute` to `GsDOBJ2 *` (SceneNode.h
spells the embedded GsDOBJ2 as four separate fields; embedding Sony's struct
there is proposed, not done: SceneNode.h has many includers).
Byte-identical.
