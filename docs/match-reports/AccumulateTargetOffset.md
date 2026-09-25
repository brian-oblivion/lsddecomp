# AccumulateTargetOffset -- MATCHED (16/16 words), round 82

> Renamed from `func_800204D0` on 2026-09-25 (tools/rename.py). Address 0x800204d0.

Round 82, runner charlie (matching slot, second pass on the unit). Unit `src/code_fa50.c`. Fresh ground, no prior attempt.

- **What:** sibling of `func_80020510`: `t = self->unk10->unk10; t->unk6 += xy[0] / 16; t->unk6 += xy[1] * 64;` with `xy` an `s32 *` (retail `lw`, not `lh`).
- **Result:** byte-exact; 16/16 words, whole-image SHA1 green. Third build.
- **Builds:** (1) `v = t->unk6 + xy[0]/16; t->unk6 = v; t->unk6 = v + xy[1]*64;` (80020510's shape) -- 2/16 with an address shift from another function; loads `t` before `xy[0]`. (2) `v = xy[0]/16 + t->unk6;` -- 9/16: load order right, but `lh` where retail has `lhu`, and v0/v1 swapped. (3) two `+=` statements -- 16/16.
- **Lever:** `lhu; addu; sh; ...; addu (same reg); sh` with NO reload between the two stores is two `+=` statements on the s16 field. `lhu` (not `lh`) is the tell: the value is only ever truncated back into the s16, which a named `s32` local defeats (it forces `lh`). GCC 2.6.3 carries the stored value into the second `+=` without reloading. Contrast `func_80020510`, whose FIRST store is a plain `=` of a fresh value, where the named-local shape is the right one.

## Source

```c
typedef struct Target_fa50 { u8 pad0[0x6]; s16 unk6; } Target_fa50;
typedef struct Inner_fa50 { u8 pad0[0x10]; Target_fa50 *unk10; } Inner_fa50;
typedef struct Outer_fa50 { u8 pad0[0x10]; Inner_fa50 *unk10; } Outer_fa50;

void AccumulateTargetOffset(Outer_fa50 *self, s32 *xy) {
    Target_fa50 *t = self->unk10->unk10;

    t->unk6 += xy[0] / 16;
    t->unk6 += xy[1] * 64;
}
```

### Proposed learning

A double store to one s16 field with `lhu` before the first add and no reload before the second is `f += a; f += b;`, not a named local (a named `s32` local gives `lh`). Discriminator against `func_80020510`'s named-local lever: whether the first store is `f = new` (local) or `f = f + new` (`+=`).

## Naming

`AccumulateTargetOffset` -- tier B. Free function, `VerbNoun`: `t->unk6 +=
xy[0] / 16; t->unk6 += xy[1] * 64;` on the `Outer_fa50`/`Inner_fa50`/
`Target_fa50` chain, which is NOT the TmdModel class (a different `self`
type, unrelated to D_8006BEA0). No caller exists anywhere in `src/` yet (its
call site is still undecompiled asm elsewhere), so the class that actually
owns this chain is unknown -- hence no `Class__` prefix. Named for its one
settled fact: it is the sibling of `func_80020510` (same field, same two
scaled adds) that ACCUMULATES onto the existing value, where
`func_80020510` (kept unrenamed -- see below) OVERWRITES it; that is the one
difference the two match reports establish. Not renamed to a `Target_fa50`-
scoped name because the type itself is only a structural guess (nesting
depth, not a confirmed class).

## Proposed field names

None from this function beyond the naming above; `Outer_fa50`/`Inner_fa50`/
`Target_fa50` and their `unk10`/`unk6` fields are left as unit-local
placeholders pending whichever unit's class actually owns this chain.
