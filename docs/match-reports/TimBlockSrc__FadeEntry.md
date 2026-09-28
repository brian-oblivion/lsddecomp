# TimBlockSrc__FadeEntry -- MATCHED (30/30 words)

> Renamed from `func_800435D0` on 2026-09-25 (tools/rename.py). Address 0x800435d0.

Round 82, runner echo (GraphicsResources session, echo #8), 2026-09-25. Unit `GraphicsResources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 30/30 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Under Lock/UnlockActiveDataSource: copies a three-byte s8 vector into entry `index` (16-byte entries from +0x40, vector at entry +0x0C), then calls FadeClutRow(entry, index) (which reads the entry's shift/mask/vector and does a StoreImage).

Table slot (`tools/classtable.py`): gTimBlockSrcMethods +0x080.

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable` and `SubBlockTable` sit at the top of
`src/graphics/GraphicsResources.c`.

```c
typedef struct Vec3S8 {
    s8 x;
    s8 y;
    s8 z;
} Vec3S8;

typedef struct Ent6F0B8 {
    /* +0x00 */ u16 shift;
    /* +0x02 */ u16 mask;
    /* +0x04 */ u8 pad4[8];
    /* +0x0C */ Vec3S8 vec;
    /* +0x0F */ u8 padF;
} Ent6F0B8;

void TimBlockSrc__FadeEntry(Obj6F0B8 *self, s32 index, Vec3S8 *src) {
    Ent6F0B8 *e;

    LockActiveDataSource();
    e = &self->entries[index];
    e->vec = *src;
    FadeClutRow(e, index);
    UnlockActiveDataSource();
}
```

## Notes

First build. The three lb then three sb is whole-struct assignment of the 3 x s8 `Vec3S8` (the lever from BgLayer__SetColor). Vec3S8's typedef moved up the file (no code change) and the local `Ent6F0B8` gained `vec` at +0x0C (size unchanged, 16). FadeClutRow is prototyped locally from its asm (a0 = entry pointer, a1 = index used in sllv).

## Naming

- **TimBlockSrc__FadeEntry**, tier B. Slot +0x080: sets one entry's 3-byte target colour and hands it to FadeClutRow.

## Track 4 (2026-09-25, round 83, bravo)

Occupant of +0x080, now the `fadeEntry` slot; stores the colour into `entries[index].color` (`TimBlockSrcColor`, was the unit's `Vec3S8`). The class (id 0xF03, table `gTimBlockSrcMethods`) is unified as `TimBlockSrc` in `include/TimBlockSrc.h`. Any source block above is the pre-unification spelling; the live body in `src/graphics/GraphicsResources.c` takes the unified types, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
