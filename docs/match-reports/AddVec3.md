# AddVec3 -- MATCHED (16/16 words)

> Renamed from `func_80056794` on 2026-09-23 (tools/rename.py). Address 0x80056794.

Unit `class_3bb8c_s`. Frameless, no self/vtable involved.

## Classification

Clean on all four carve-time screens, and the unit's own carve-time census
already flagged it as "opens on its own $a1, not on $sp" -- i.e. not a method,
just a plain leaf. It is a Vec3 add.

## Body

```c
typedef struct Vec3S {
    s32 x, y, z;
} Vec3S;

void AddVec3(Vec3S *dst, Vec3S *a, Vec3S *b) {
    dst->x = a->x + b->x;
    dst->y = a->y + b->y;
    dst->z = a->z + b->z;
}
```

`Vec3S` is this unit's own local reading of the same shape `class_3bb8c_o.c`
already has under its own name `Vec3O` -- kept separate per the
multiple-independent-local-views convention (this unit is not
`class_3bb8c_o.c`'s to edit).

### Proposed learning

None -- a plain three-field vector add, no residue.

## Naming

Round 70 (alpha). `func_80056794` -> `AddVec3`, **tier A**.

Pure leaf, `dst = a + b` over three s32 components; the mechanics are its
purpose. Free function (no `self`), VerbNoun. Checked no `AddVec3` or
similar existed in the symbols file (only Actor__AddTranslation, a method).

## Track 4 (2026-09-26, round 88, charlie)

Retyped with StyleEffect's unification: `Vec3S` is `LongVec3`. Image byte-identical.
