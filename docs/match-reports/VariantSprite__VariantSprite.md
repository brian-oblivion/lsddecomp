# VariantSprite__VariantSprite -- MATCHED (43/43)

> Renamed from `Class879C4__Class879C4` on 2026-09-26 (tools/rename.py). Address 0x80057d10.

> Renamed from `D800879C4__D800879C4` on 2026-09-26 (tools/rename.py). Address 0x80057d10.

> Renamed from `func_80057D10` on 2026-09-19 (tools/rename.py). Address 0x80057d10.

Unit: `src/class_3bb8c_k.c`. Class: `gVariantSpriteMethods` (49 slots, uncarved --
`asm/class_3bb8c_k.s`) -- THIS is its own ctor, resolved via
`tools/classtable.py gVariantSpriteMethods` at `+0x008`, and the callee of this
unit's own `New_VariantSprite` (`GetVariantSpriteMethods()->ctor(...)`, see its
report).

## Signature

```c
void *VariantSprite__VariantSprite(D800879C4Obj *self, s32 arg1, void *arg2, void *arg3);
```

## Body

```c
void *VariantSprite__VariantSprite(D800879C4Obj *self, s32 arg1, void *arg2, void *arg3) {
    GetSpriteMethods()->ctor((Sprite *)self, arg3, 0, &gVariantSpriteCells[arg1], arg2, 0);
    self->methods = GetVariantSpriteMethods();
    self->unk_0xA4 = 0;
    return self->methods->postConstruct(self, arg1);
}
```

Chains through THREE separate uncarved-ground getter functions and their
tables, seeded correctly on the first `m2ctx.py --sig ... --run` pass and
matched with no iteration:

1. `GetSpriteMethods()` (in `asm/psyq_memset.s` -- splat segmentation, not a
   meaningful file grouping) is a plain no-argument getter, `return
   &gSpriteMethods;`. Its `+0x008` slot (a DIFFERENT class's own ctor,
   `Sprite__Sprite`, out of scope) is called with `(self, arg3, 0,
   &gVariantSpriteCells[arg1], arg2, 0)` -- SIX arguments, four in registers and
   two on the stack, even though `GetSpriteMethods` itself takes none: the
   callee simply ignores the extras, same "per-call-site signature"
   precedent as `AcceptGridElem`'s report.
2. `gVariantSpriteCells` is a newly-typed 2-element, `0xC`-byte-stride table
   (address-of only here, never dereferenced -- kept opaque beyond the
   stride).
3. This function is ITSELF `gVariantSpriteMethods`'s ctor: it sets `self->methods =
   GetVariantSpriteMethods()` (`&gVariantSpriteMethods`, this unit's own getter, see
   `GetActorMethods`/`New_VariantSprite`'s reports for the sibling pattern),
   zeroes `self->unk_0xA4`, then TAIL-CALLS its own class's `+0x040` slot
   (`self->methods->postConstruct`, resolves to `VariantSprite__SetVariantClut` -- the first
   function of this unit's successor `class_3bb8c_k`, out of range) and
   forwards its return value.

`D800879C4Methods` and `D800879C4Obj` (declared once, above
`New_VariantSprite`) now carry the `+0x040` slot and the `methods`/`+0xA4`
object fields this function needed, alongside the `+0x008` ctor slot
`New_VariantSprite` already used.

## Naming

**`VariantSprite__VariantSprite` -- tier B.** Confirmed as `gVariantSpriteMethods`'s own
constructor via `tools/classtable.py gVariantSpriteMethods` (`+0x008` resolves here).
Mechanics are construction (chains a base-class ctor, sets its own
vtable pointer and `unk_0xA4`, tail-calls its class's own `postConstruct`
slot) -- named with the `Class__Class` constructor convention even though
the CLASS's own purpose in the game remains completely unknown (it lives
in uncarved ground, `class_3bb8c_k.s`, out of this runner's scope).

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py VariantSprite__VariantSprite   # 43/43
```

## Track 4 (2026-09-25, round 82, alpha)

The base class is unified as `Sprite` (`include/Sprite.h`, table `gSpriteMethods`, formerly `D_8006EE1C`). The unit-local `D8006EE1CMethods` view is gone: the call goes through `SpriteMethods`' ctor, whose parameters are `(Sprite *self, void *texture, s32 abr, SpriteRect *rect, void *arg4, s32 arg5)` (Sprite__Sprite forwards all five to its +0x040 reset, which reads texture/abr/rect), so `self` is upcast (a pointer cast, no code) and `gVariantSpriteCells` is now `SpriteRect[2]` instead of an opaque 12-byte entry. Byte-identical: whole image green, 0 new warnings.

## Track 4 (2026-09-26, round 87, alpha)

Renamed from `D800879C4__D800879C4` (tools/rename.py). The class is unified
as `VariantSprite` (`include/VariantSprite.h`, table `gVariantSpriteMethods`,
formerly `D_800879C4`), the `ClassXXXXX` convention of StyleEffect and
FadeBox: the sprites' role in the game is not established, so no
descriptive name. The unit-local `D800879C4Obj` / `D800879C4Methods` views
are gone. Current body:

```c
void VariantSprite__VariantSprite(VariantSprite *self, s32 variant, void *arg2, void *texture) {
    GetSpriteMethods()->ctor((Sprite *)self, texture, 0, &gVariantSpriteCells[variant], arg2, 0);
    self->methods = GetVariantSpriteMethods();
    self->unkA4 = 0;
    ((VariantSpriteResetFn)self->methods->reset)(self, variant);
}
```

- **Return type `void`, settled by the bytes.** `typeviews.py --merge`
  reported a CONFLICT at +0x040: Sprite's `reset` returns void, the old
  local `setVariantClut` returned `void *` so this ctor could `return` it.
  The occupant, `VariantSprite__SetVariantClut`, returns nothing (its body
  never writes `$v0`), and this ctor ends in the `jalr` with no `move v0,
  s1` after it. A void ctor ending in a void call compiles to the same 43
  words (whole image green), so the slot keeps the occupant's `void` and the
  ctor returns nothing, as ScreenSprite's does. The ctor slot, from
  SceneNode, still says `void *`; New_VariantSprite ignores the result.
- **The reset call's cast.** The slot (SceneNode's `reset(Self *self)`)
  takes no argument; the occupant takes the variant, so the call casts to
  `VariantSpriteResetFn` (round 85's TodActor precedent; no code).
- Parameter names: `variant` picks the texture cell here and the CLUT row in
  SetVariantClut; `texture` is forwarded as Sprite's `texture`; `arg2` as
  Sprite's opaque `arg4`. `unk_0xA4` -> `unkA4` (only this ctor writes it).

## Track 6 (2026-09-26, round 93, bravo)

The class `Class879C4` is now `VariantSprite` (`include/VariantSprite.h`,
`python3 tools/renametype.py Class879C4 VariantSprite`), tier B: the
mechanics are certain and are the whole of what the class adds to Sprite --
`variant` (0 or 1) picks the texture cell the Sprite ctor binds
(`gVariantSpriteCells`) and the CLUT row the reset slot sets
(`gVariantSpriteClutX/Y`). What the sprites are in the game is not
established (their only builder is StyleEffect, kinds 2 and 3, and every
path passes variant 0), which is why it is not tier A. The table, getter,
allocator, methods and the three data tables followed the class name.
The same tool run rewrote `Class879C4` tokens inside this report's older
history prose (the known renametype behaviour pending an operator
decision); those lines were left as the tool wrote them.

The ctor keeps the Class__Class form. `unkA4` (+0x0A4), which it zeroes,
keeps its placeholder: nothing else reads or writes it, so no name says
more than "zeroed by the ctor".

## Track 7 (2026-09-27, round 96, bravo)

Comments quoted below are verbatim as the file stood before this round's
comment pass, i.e. with this round's renames already applied (the
`LinkQueryBuf` one as it stood before step 2).

- Parameter `arg2` -> `resetArg` (see New_VariantSprite's report).

The function comment, reworded; the old one, verbatim:

```c
/* The base-class ctor, Sprite__Sprite, through GetSpriteMethods(), with
 * `self` upcast; then this class's table, and its reset slot
 * (VariantSprite__SetVariantClut) with the variant, through VariantSpriteResetFn.
 * The retail ctor ends in that call without setting $v0: it returns
 * nothing. */
```
