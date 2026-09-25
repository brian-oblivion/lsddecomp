# func_80041DAC -- MATCHED (32/32 words), round 82

Round 82, runner alpha (fourth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior attempt.

- **Where:** D_8006EC74 and D_8006ED4C slot +0x04C (attachToParent override).
- **What:** if `parent` (+0x00C) is still NULL, calls Sprite's attachToParent (`GetSpriteMethods()->attachToParent`, i.e. the inherited Class6B5CC one) with the zero offset `D_8006EE10` (three zero words in .data), then calls the object's own slot +0x0BC (func_80041E2C) with the caller's third argument. No return value is produced on the skip path (`$v0` holds the loaded parent), so it is written `void` although the Class6B5CC slot type returns a pointer.
- **Result:** byte-exact, 32/32 words, 0 ins / 0 del, whole-image SHA1 green. First build.
- **Types:** self is the unit-local `SpriteView_322b4` (it gained `slotBC` at +0x0BC in its local method view); the base call casts to `Sprite *`. `extern Vec3_d294 D_8006EE10;` in the unit. No shared header touched.

## Source

```c
/* SpriteMethods_322b4 (unit-local): */
    void (*slotBC)(SpriteView_322b4 *self, Pair_322b4 *src);   /* +0x0BC = func_80041E2C */
extern Vec3_d294 D_8006EE10;

void func_80041DAC(SpriteView_322b4 *self, Class6B5CC *parent, Pair_322b4 *pos) {
    if (self->unkC == 0) {
        GetSpriteMethods()->attachToParent((Sprite *)self, parent, &D_8006EE10);
        self->methods->slotBC(self, pos);
    }
}
```
