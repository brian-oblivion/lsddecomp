# TileMap__TileMap -- MATCHED (34/34 words)

> Renamed from `GridIndexSrc__GridIndexSrc` on 2026-09-25 (tools/rename.py). Address 0x80044d40.

> Renamed from `func_80044D40` on 2026-09-25 (tools/rename.py). Address 0x80044d40.

Round 82, runner echo (code_33808 session, echo #8), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 34/34 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Constructor: active driver's ctor, install D_8006F498 (GetTileMapMethods), store the third argument at +0x3C, clear +0x42; when the second argument is 0, set +0x40 = 1, clear +0x2A and call its own +0x064 (TileMap__Load).

Table slot (`tools/classtable.py`): D_8006F498 +0x008.

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808` and `CountedBuf33808` sit at the top of
`src/code_33808.c`.

```c
typedef struct Obj6F498 {
    CLASS6D430_FIELDS(DataSrc33808Methods);
    /* +0x02C */ u8 pad2C[0x10];
    /* +0x03C */ s32 unk3C;
    /* +0x040 */ u16 unk40;
    /* +0x042 */ u16 unk42;
} Obj6F498;

/* D_8006F498 +0x008: constructor -- the active driver's, then this table;
 * store `arg2` at +0x3C, clear +0x42, and with no `arg1` set +0x40, clear
 * +0x2A and run its own +0x064. */
void TileMap__TileMap(Obj6F498 *self, s32 arg1, s32 arg2) {
    s32 unused[8];

    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetTileMapMethods();
    self->unk3C = arg2;
    self->unk42 = 0;
    if (arg1 == 0) {
        self->unk40 = 1;
        self->unk2A = 0;
        self->methods->setFlag((DataSrc33808 *)self);
    }
}
```

## Notes

First build; the D_8006F514 ctor (TileAtlas__TileAtlas) shape with one more stored argument. The 0x40 frame with no stack use is the unused-local-array lever (`s32 unused[8];`). Unit-local `Obj6F498` moved up to precede this function and gained +0x3C (s32) and +0x40 (u16); TileMap__Load uses it unchanged (whole-image oracle green).
