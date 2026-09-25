# Sprite__Sprite -- MATCHED (41/41 words), round 82

Round 82, runner alpha (fifth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior body attempt.

- **Where:** gSpriteMethods slot +0x008 (ctor) (`tools/classtable.py`).
- **What:** Class6B5CC's ctor through `GetClass6B5CCMethods()`, installs `GetSpriteMethods()`, then calls the installed reset (+0x040) with all five ctor arguments (arg4/arg5 re-stored at sp+0x10/0x14). Nothing after the call touches `$v0`, so it returns whatever reset returns; the byte match says nothing about the return type.
- **Result:** byte-exact; 41/41 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** the Sprite.h 'Not settled' point handled at the call site as briefed: a unit-local function-pointer type `SpriteCtorReset_322b4` (self + five arguments, returning `void *`) casts Class6B5CC's one-argument `reset` slot. Class6B5CC.h and Sprite.h are untouched; the prototype in Sprite.h (returning `void *`) is matched unchanged.

## Source

```c
/* gSpriteMethods slot +0x008 (ctor): the Class6B5CC ctor, install the table,
 * and hand every argument to reset. */
void *Sprite__Sprite(Sprite *self, void *texture, s32 abr, SpriteRect *rect, void *arg4, s32 arg5) {
    GetClass6B5CCMethods()->ctor((Class6B5CC *)self);
    self->methods = GetSpriteMethods();
    return ((SpriteCtorReset_322b4)self->methods->reset)(self, texture, abr, rect, arg4, arg5);
}
```
