# TmdModel__AddFirstPrimClut -- MATCHED (16/16 words), round 82

> Renamed from `AccumulateTargetOffset` on 2026-09-27 (tools/rename.py). Address 0x800204d0.

> Renamed from `func_800204D0` on 2026-09-25 (tools/rename.py). Address 0x800204d0.

Round 82, runner charlie (matching slot, second pass on the unit). Unit `src/TmdModel.c`. Fresh ground, no prior attempt.

- **What:** sibling of `TmdModel__SetFirstPrimClut`: `t = self->unk10->unk10; t->unk6 += xy[0] / 16; t->unk6 += xy[1] * 64;` with `xy` an `s32 *` (retail `lw`, not `lh`).
- **Result:** byte-exact; 16/16 words, whole-image SHA1 green. Third build.
- **Builds:** (1) `v = t->unk6 + xy[0]/16; t->unk6 = v; t->unk6 = v + xy[1]*64;` (80020510's shape) -- 2/16 with an address shift from another function; loads `t` before `xy[0]`. (2) `v = xy[0]/16 + t->unk6;` -- 9/16: load order right, but `lh` where retail has `lhu`, and v0/v1 swapped. (3) two `+=` statements -- 16/16.
- **Lever:** `lhu; addu; sh; ...; addu (same reg); sh` with NO reload between the two stores is two `+=` statements on the s16 field. `lhu` (not `lh`) is the tell: the value is only ever truncated back into the s16, which a named `s32` local defeats (it forces `lh`). GCC 2.6.3 carries the stored value into the second `+=` without reloading. Contrast `TmdModel__SetFirstPrimClut`, whose FIRST store is a plain `=` of a fresh value, where the named-local shape is the right one.

## Source

```c
typedef struct Target_fa50 { u8 pad0[0x6]; s16 unk6; } Target_fa50;
typedef struct Inner_fa50 { u8 pad0[0x10]; Target_fa50 *unk10; } Inner_fa50;
typedef struct Outer_fa50 { u8 pad0[0x10]; Inner_fa50 *unk10; } Outer_fa50;

void TmdModel__AddFirstPrimClut(Outer_fa50 *self, s32 *xy) {
    Target_fa50 *t = self->unk10->unk10;

    t->unk6 += xy[0] / 16;
    t->unk6 += xy[1] * 64;
}
```

### Proposed learning

A double store to one s16 field with `lhu` before the first add and no reload before the second is `f += a; f += b;`, not a named local (a named `s32` local gives `lh`). Discriminator against `TmdModel__SetFirstPrimClut`'s named-local lever: whether the first store is `f = new` (local) or `f = f + new` (`+=`).

## Naming

`TmdModel__AddFirstPrimClut` -- tier B. Free function, `VerbNoun`: `t->unk6 +=
xy[0] / 16; t->unk6 += xy[1] * 64;` on the `Outer_fa50`/`Inner_fa50`/
`Target_fa50` chain, which is NOT the TmdModel class (a different `self`
type, unrelated to gTmdModelMethods). No caller exists anywhere in `src/` yet (its
call site is still undecompiled asm elsewhere), so the class that actually
owns this chain is unknown -- hence no `Class__` prefix. Named for its one
settled fact: it is the sibling of `TmdModel__SetFirstPrimClut` (same field, same two
scaled adds) that ACCUMULATES onto the existing value, where
`TmdModel__SetFirstPrimClut` (kept unrenamed -- see below) OVERWRITES it; that is the one
difference the two match reports establish. Not renamed to a `Target_fa50`-
scoped name because the type itself is only a structural guess (nesting
depth, not a confirmed class).

## Proposed field names

None from this function beyond the naming above; `Outer_fa50`/`Inner_fa50`/
`Target_fa50` and their `unk10`/`unk6` fields are left as unit-local
placeholders pending whichever unit's class actually owns this chain.

## Track 7 (2026-09-26, round 94, bravo)

Named the fields themselves, unlike the types above: every accessor of
`unk6`/`unk10` is in this unit (`src/TmdModel.c`, `grep -rl` over `src/`
finds no other file mentioning `Outer_fa50`/`Inner_fa50`/`Target_fa50`), so
the field rule (unit-local struct, name accessed `unk` fields) applies even
though the owning CLASS is still unconfirmed -- that is a track-6 question
about the type name, not this pass's. `Target_fa50::unk6` -> `offset`: the
one field both this function and `TmdModel__SetFirstPrimClut` read and write, and the
literal field this pair of functions is named for. `Inner_fa50::unk10` ->
`target` and `Outer_fa50::unk10` -> `inner`: each names what it points to,
the only fact established about a plain link in the chain. Compiler-verified
accessor list (this unit only), build and check-nonmatching.sh green.

## Naming (track 6, round 95)

2026-09-27, delta: the three placeholder types are replaced by `TmdModel`,
`TmdObject` and Sony's `TMD_P_TF3`, and `offset` is the first primitive's
`clut`. Evidence in `TmdModel__SetFirstPrimClut.md`'s `## Naming`: the chain matches
TmdModel's `object` (+0x010) and TmdObject's `prims` (+0x010), the value is
getClut's `x / 16 + y * 64`, and the sibling's caller passes VRAM
(1008, 511). This function takes the pair as `s32 *` and adds, where the
sibling takes `s16 *` and sets. Byte-identical.
