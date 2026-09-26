# Class879C4__Class879C4 -- MATCHED (43/43)

> Renamed from `D800879C4__D800879C4` on 2026-09-26 (tools/rename.py). Address 0x80057d10.

> Renamed from `func_80057D10` on 2026-09-19 (tools/rename.py). Address 0x80057d10.

Unit: `src/class_3bb8c_p.c`. Class: `gClass879C4Methods` (49 slots, uncarved --
`asm/class_3bb8c_q.s`) -- THIS is its own ctor, resolved via
`tools/classtable.py gClass879C4Methods` at `+0x008`, and the callee of this
unit's own `New_Class879C4` (`GetClass879C4Methods()->ctor(...)`, see its
report).

## Signature

```c
void *Class879C4__Class879C4(D800879C4Obj *self, s32 arg1, void *arg2, void *arg3);
```

## Body

```c
void *Class879C4__Class879C4(D800879C4Obj *self, s32 arg1, void *arg2, void *arg3) {
    GetSpriteMethods()->ctor((Sprite *)self, arg3, 0, &gClass879C4Cells[arg1], arg2, 0);
    self->methods = GetClass879C4Methods();
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
   &gClass879C4Cells[arg1], arg2, 0)` -- SIX arguments, four in registers and
   two on the stack, even though `GetSpriteMethods` itself takes none: the
   callee simply ignores the extras, same "per-call-site signature"
   precedent as `AcceptGridElem`'s report.
2. `gClass879C4Cells` is a newly-typed 2-element, `0xC`-byte-stride table
   (address-of only here, never dereferenced -- kept opaque beyond the
   stride).
3. This function is ITSELF `gClass879C4Methods`'s ctor: it sets `self->methods =
   GetClass879C4Methods()` (`&gClass879C4Methods`, this unit's own getter, see
   `GetActorMethods`/`New_Class879C4`'s reports for the sibling pattern),
   zeroes `self->unk_0xA4`, then TAIL-CALLS its own class's `+0x040` slot
   (`self->methods->postConstruct`, resolves to `Class879C4__SetVariantClut` -- the first
   function of this unit's successor `class_3bb8c_q`, out of range) and
   forwards its return value.

`D800879C4Methods` and `D800879C4Obj` (declared once, above
`New_Class879C4`) now carry the `+0x040` slot and the `methods`/`+0xA4`
object fields this function needed, alongside the `+0x008` ctor slot
`New_Class879C4` already used.

## Naming

**`Class879C4__Class879C4` -- tier B.** Confirmed as `gClass879C4Methods`'s own
constructor via `tools/classtable.py gClass879C4Methods` (`+0x008` resolves here).
Mechanics are construction (chains a base-class ctor, sets its own
vtable pointer and `unk_0xA4`, tail-calls its class's own `postConstruct`
slot) -- named with the `Class__Class` constructor convention even though
the CLASS's own purpose in the game remains completely unknown (it lives
in uncarved ground, `class_3bb8c_q.s`, out of this runner's scope).

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py Class879C4__Class879C4   # 43/43
```

## Track 4 (2026-09-25, round 82, alpha)

The base class is unified as `Sprite` (`include/Sprite.h`, table `gSpriteMethods`, formerly `D_8006EE1C`). The unit-local `D8006EE1CMethods` view is gone: the call goes through `SpriteMethods`' ctor, whose parameters are `(Sprite *self, void *texture, s32 abr, SpriteRect *rect, void *arg4, s32 arg5)` (Sprite__Sprite forwards all five to its +0x040 reset, which reads texture/abr/rect), so `self` is upcast (a pointer cast, no code) and `gClass879C4Cells` is now `SpriteRect[2]` instead of an opaque 12-byte entry. Byte-identical: whole image green, 0 new warnings.

## Track 4 (2026-09-26, round 87, alpha)

Renamed from `D800879C4__D800879C4` (tools/rename.py). The class is unified
as `Class879C4` (`include/Class879C4.h`, table `gClass879C4Methods`,
formerly `D_800879C4`), the `ClassXXXXX` convention of Class876FC and
FadeBox: the sprites' role in the game is not established, so no
descriptive name. The unit-local `D800879C4Obj` / `D800879C4Methods` views
are gone. Current body:

```c
void Class879C4__Class879C4(Class879C4 *self, s32 variant, void *arg2, void *texture) {
    GetSpriteMethods()->ctor((Sprite *)self, texture, 0, &gClass879C4Cells[variant], arg2, 0);
    self->methods = GetClass879C4Methods();
    self->unkA4 = 0;
    ((Class879C4ResetFn)self->methods->reset)(self, variant);
}
```

- **Return type `void`, settled by the bytes.** `typeviews.py --merge`
  reported a CONFLICT at +0x040: Sprite's `reset` returns void, the old
  local `setVariantClut` returned `void *` so this ctor could `return` it.
  The occupant, `Class879C4__SetVariantClut`, returns nothing (its body
  never writes `$v0`), and this ctor ends in the `jalr` with no `move v0,
  s1` after it. A void ctor ending in a void call compiles to the same 43
  words (whole image green), so the slot keeps the occupant's `void` and the
  ctor returns nothing, as ScreenSprite's does. The ctor slot, from
  SceneNode, still says `void *`; New_Class879C4 ignores the result.
- **The reset call's cast.** The slot (SceneNode's `reset(Self *self)`)
  takes no argument; the occupant takes the variant, so the call casts to
  `Class879C4ResetFn` (round 85's Class65650 precedent; no code).
- Parameter names: `variant` picks the texture cell here and the CLUT row in
  SetVariantClut; `texture` is forwarded as Sprite's `texture`; `arg2` as
  Sprite's opaque `arg4`. `unk_0xA4` -> `unkA4` (only this ctor writes it).
