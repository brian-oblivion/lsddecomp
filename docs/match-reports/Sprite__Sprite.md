# Sprite__Sprite -- MATCHED (41/41 words), round 82

Round 82, runner alpha (fifth slot on Sprite). Unit `src/graphics/sprite.c`. Fresh ground, no prior body attempt.

- **Where:** gSpriteMethods slot +0x008 (ctor) (`tools/classtable.py`).
- **What:** SceneNode's ctor through `GetSceneNodeMethods()`, installs `GetSpriteMethods()`, then calls the installed reset (+0x040) with all five ctor arguments (arg4/arg5 re-stored at sp+0x10/0x14). Nothing after the call touches `$v0`, so it returns whatever reset returns; the byte match says nothing about the return type.
- **Result:** byte-exact; 41/41 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** the sprite.h 'Not settled' point handled at the call site as briefed: a unit-local function-pointer type `SpriteResetFn` (self + five arguments, returning `void *`) casts SceneNode's one-argument `reset` slot. scene_node.h and sprite.h are untouched; the prototype in sprite.h (returning `void *`) is matched unchanged.
- **Round 98 (runner alpha, track 6):** the cast type, round 82's `SpriteCtorReset_322b4`, renamed `SpriteResetFn` (`renametype.py`, image byte-identical), tier A: it is the `<Class>ResetFn` cast every sibling ctor uses (char_sprite.h's `CharSpriteResetFn`, VariantSprite.h's `VariantSpriteResetFn`), and the body shows what it calls. It differs from them in shape, not purpose: five arguments and a `void *` return, the ctor's call rather than `Sprite__Reset`'s three-argument void signature. Kept local to code_322b4.c since no other unit calls Sprite's reset with arguments (`grep -rn 'reset)(' src/`).

## Source

```c
/* gSpriteMethods slot +0x008 (ctor): the SceneNode ctor, install the table,
 * and hand every argument to reset. */
void *Sprite__Sprite(Sprite *self, void *texture, s32 abr, SpriteRect *rect, void *arg4, s32 arg5) {
    GetSceneNodeMethods()->ctor((SceneNode *)self);
    self->methods = GetSpriteMethods();
    return ((SpriteResetFn)self->methods->reset)(self, texture, abr, rect, arg4, arg5);
}
```

## Track 7 (round 99, charlie)

`arg4`/`arg5` are `resetArg`/`resetWord` (see New_Sprite's report), in the local `SpriteResetFn` too. Byte-exact.

## History (moved from include/Sprite.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
 * reset reads three, where SceneNode's reset takes none (VariantSprite's
 * ctor, round 87, is void: its +0x040 occupant sets no $v0). The slot keeps
```
