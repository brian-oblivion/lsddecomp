# SetupCdStreamAudio -- MATCHED (exact length, 19/19 words), round 81

> Renamed from `func_80047240` on 2026-09-25 (tools/rename.py). Address 0x80047240.

Round 81, runner echo. Unit `src/code_3770c.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot of gCdStreamObjMethods.
- **What:** fills a stack SpuCommonAttr (mask 0x2C3 = SPU_COMMON_MVOLL|MVOLR|CDVOLL|CDVOLR|CDMIX; master volume 0x3FFF both sides, CD volume 0x7FFF both sides, cd.mix = 1) and calls SpuSetCommonAttr; returns 1. SpuCommonAttr/SpuExtAttr/SpuVolume are declared locally from LIBSPU.H so field offsets 0x10/0x18 fall out of the struct (frame 0x40 = 0x10 outgoing + 0x28 struct + ra).
- **Levers:** none needed.
- **Context:** local view `CdStreamObj` (BASICCLASS_FIELDS + `cdResult[8]` at +0x24, `s32 unk2C` at +0x2C, `u32 *ring` at +0x50) and the libcd/libspu externs are in `src/code_3770c.c`, declared from the Psy-Q prototypes.

## Naming

Kept `func_`. The class gCdStreamObjMethods is a CD streaming (StSetRing/CdSync) object; a naming pass can call it e.g. `CdStreamObj` (tier B: from the libcd calls only).

## Source

```c
s32 SetupCdStreamAudio(CdStreamObj *self) {
    SpuCommonAttr attr;

    attr.mask = 0x2C3;
    attr.mvol.left = 0x3FFF;
    attr.mvol.right = 0x3FFF;
    attr.cd.volume.left = 0x7FFF;
    attr.cd.volume.right = 0x7FFF;
    attr.cd.mix = 1;
    SpuSetCommonAttr(&attr);
    return 1;
}
```
