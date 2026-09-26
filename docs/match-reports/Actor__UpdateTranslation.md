# Actor__UpdateTranslation -- MATCHED (30/30 words)

> Renamed from `BaseObjO__UpdateVec14` on 2026-09-25 (tools/rename.py). Address 0x800573cc.

> Renamed from `func_800573CC` on 2026-09-18 (tools/rename.py). Address 0x800573cc.

Unit: `class_3bb8c_o` (round 17). The shared worker behind
`Actor__SetTranslation`/`Actor__AddTranslation`: overwrites or accumulates a 3-word vector
into `self->unk14->vec18`, then clears `self->unk14->unk0`.

## Final source

```c
typedef struct SplitCoord2O {
    s32 unk0;      /* +0x000 */
    u8 pad4[0x14];   /* +0x004 .. +0x017, unknown */
    Vec3O vec18;      /* +0x018 .. +0x023 */
} SplitCoord2O;

void Actor__UpdateTranslation(BaseObjO *self, s32 flag, Vec3O *v) {
    BaseObjO *t = self;
    SplitCoord2O *u = t->unk14;

    if (flag) {
        u->vec18 = *v;
    } else {
        u->vec18.x += v->x;
        u->vec18.y += v->y;
        u->vec18.z += v->z;
    }
    t->unk14->unk0 = 0;
}
```

## Derivation

Two residues, both closed:

1. **The overwrite branch (`flag != 0`) is a WHOLE-STRUCT ASSIGNMENT, not
   three separate field writes.** Retail loads all three words of `v`
   FIRST (into `$v0`/`$v1`/`$a0`), THEN stores all three into
   `u->vec18` -- the documented "whole-struct assignment... batches N
   words per iteration cycling temp registers" idiom. Writing
   `u->vec18[0]=v->x; u->vec18[1]=v->y; u->vec18[2]=v->z;` (per-field,
   `vec18` as an `s32[3]`) instead compiles to interleaved load/store
   pairs -- wrong shape. Retyping `SplitCoord2O::vec18` from `s32[3]` to a
   plain `Vec3O` and writing `u->vec18 = *v;` reproduces the batched
   load-then-store shape directly (this project's compiler emits a
   genuine block-move for a whole-struct assignment). The accumulate
   branch (`flag == 0`) is NOT a whole-struct operation in retail --
   confirmed already-correct per-field `load/add/store` triples, which the
   `.x += / .y += / .z +=` form (accessing the SAME `Vec3O` field, just
   with `+=` instead of `=`) reproduces without needing an array form
   there either.
2. **`self` is read through a second name (`t`) at both uses, not
   re-read as `self` at the end.** Retail moves `$a0` into `$t0` as the
   VERY FIRST instruction and uses `$t0` (never `$a0` again) for both the
   `unk14` read at the top and the final `unk14->unk0 = 0` write. Since
   nothing in this function actually clobbers `$a0`, using `self` directly
   throughout would have been arithmetically equivalent -- but the
   register CHOICE (an otherwise-unremarkable caller-saved `$t0` rather
   than reusing the argument register `$a0` for the whole function) only
   appeared once a second name (`BaseObjO *t = self;`) was introduced and
   used at BOTH access points. This is the "mention the value again"
   family from `DECOMPILATION_LEARNINGS.md`, applied to a plain parameter
   rather than a computed value.

### Proposed learning

- **A block-write branch and an accumulate branch operating on the SAME
  struct field can want OPPOSITE representations (whole-struct assignment
  vs. per-field arithmetic) in the SAME function.** Confirmed here: the
  `flag != 0` arm needed `u->vec18 = *v;` (array-typed field would have
  been wrong), while the `flag == 0` arm needed `.x +=`/`.y +=`/`.z +=` on
  that exact same field (batching those into a hypothetical
  `u->vec18 += *v` construct isn't even valid C for a struct, so this
  wasn't a live alternative, but it's worth remembering that "this field
  wants whole-struct form" is a per-BRANCH property, not a per-field one).
- **An unused-looking `move $t0,$a0` at function entry, with `$a0` never
  referenced again, is a real signal that the source names the parameter
  through a SECOND local -- even when the parameter itself would have
  worked as written.** Confirmed as a genuine (if small, 1-word) residue
  rather than an artifact: omitting the second name compiled clean but
  used `$a0` directly at both sites, 1 word short via a register-identity
  mismatch that a fresh `BaseObjO *t = self;` closed exactly.

## Naming

**`Actor__UpdateTranslation` -- tier A.** The shared worker behind
`SetVec14`/`AddVec14`: overwrites or accumulates a `Vec3O` into
`self->vecTarget->vec18` depending on `flag`, then always clears
`self->vecTarget->unk0`. "Update" covers both the overwrite and the
accumulate case without picking one, which is what the shared (flag-gated)
function needs; the two one-line callers get the more specific
`Set`/`Add` names instead.

## Track 4 (2026-09-25, round 82, delta)

Renamed from `BaseObjO__UpdateVec14`. The body behind +0x0B8/+0x0BC: sets or adds coord2->coord.t (tx/ty/tz of SceneNodeSub14; the set path is a whole-Vec3 copy through a LongVec3 cast, as the old Vec3 member was), then clears coord2->flg so the coordinate is recomputed. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_o.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
