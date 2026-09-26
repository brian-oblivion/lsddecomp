# BgLayer__BgLayer -- MATCHED (29/29 words)

> Renamed from `func_80044220` on 2026-09-25 (tools/rename.py). Address 0x80044220.

Round 82, runner echo (code_33808 session, echo #7), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 29/29 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Class6B5CC's constructor through GetClass6B5CCMethods(), then installs this class's table (GetBgLayerMethods) and calls slot +0x040 with (self, arg1, arg2). GCC forwards the stored table pointer, so `self->methods->reset` compiles to a use of the getter's return value, as retail has it. Slot +0x040 is Class6B5CC's `reset(self)` in the unified macro; this class's occupant takes two more arguments, so the call casts rather than retyping the shared slot.

Table slot (`tools/classtable.py`): D_8006F2C4 +0x008 (constructor).

## Source

The unit-local view `DataSrc33808` (a Class6D430 subclass built with the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.

```c
/* A three-byte vector, and the D_8006F2C4 object (a Class6B5CC subclass). */
typedef struct Vec3S8 {
    s8 x;
    s8 y;
    s8 z;
} Vec3S8;

typedef struct Obj6F2C4 {
    CLASS6B5CC_FIELDS(Class6B5CCMethods);
    /* +0x044 */ u8 pad44[0x10];
    /* +0x054 */ Vec3S8 unk54;
} Obj6F2C4;

/* D_8006F2C4 +0x008: constructor -- Class6B5CC's, then this table, then
 * slot +0x040 with the two arguments. */
void BgLayer__BgLayer(Obj6F2C4 *self, s32 arg1, s32 arg2) {
    GetClass6B5CCMethods()->ctor((Class6B5CC *)self);
    self->methods = GetBgLayerMethods();
    ((void (*)())self->methods->reset)(self, arg1, arg2);
}
```

## Notes

- Byte-exact on the first build.
- The Vec3S8/Obj6F2C4 typedefs (first written for BgLayer__SetColor) were moved up to sit before this function, which uses Obj6F2C4 and comes earlier in ROM; typedef position emits no code.
- No shared header was edited; prototypes for other units' functions are local
  to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **BgLayer__BgLayer**, tier B. Constructor: Class6B5CC's ctor, then this table, then Reset with the two arguments.

## Track 4 (2026-09-26, round 88, alpha)

Class unified in `include/BgLayer.h`. `self` is `BgLayer *` (was the unit-local `Obj6F2C4`), the parameters are `(struct Map44294 *src, s32 mode)` (were `s32 arg1, arg2`), and reset is called through `BgLayerResetFn`, a typedef of BgLayer__Reset's own parameter list (was an unprototyped `void (*)()` cast): the inherited +0x040 slot takes self alone (FINISHING-PLAN track 4 step 6). Byte-identical.

Later the same round (alpha, second class): TileMap unified too (`include/TileMap.h`, same round): `src` is `TileMap *` (was `struct Map44294 *`). Byte-identical.
