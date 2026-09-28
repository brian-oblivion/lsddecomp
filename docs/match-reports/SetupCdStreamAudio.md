# SetupCdStreamAudio -- MATCHED (exact length, 19/19 words), round 81

> Renamed from `func_80047240` on 2026-09-25 (tools/rename.py). Address 0x80047240.

Round 81, runner echo. Unit `src/cd/cd_stream.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot of gCdStreamMethods.
- **What:** fills a stack SpuCommonAttr (mask 0x2C3 = SPU_COMMON_MVOLL|MVOLR|CDVOLL|CDVOLR|CDMIX; master volume 0x3FFF both sides, CD volume 0x7FFF both sides, cd.mix = 1) and calls SpuSetCommonAttr; returns 1. SpuCommonAttr/SpuExtAttr/SpuVolume are declared locally from LIBSPU.H so field offsets 0x10/0x18 fall out of the struct (frame 0x40 = 0x10 outgoing + 0x28 struct + ra).
- **Levers:** none needed.
- **Context:** local view `CdStreamObj` (BASICCLASS_FIELDS + `cdResult[8]` at +0x24, `s32 unk2C` at +0x2C, `u32 *ring` at +0x50) and the libcd/libspu externs are in `src/cd/cd_stream.c`, declared from the Psy-Q prototypes.

## Naming

Tier B. `SetupCdStreamAudio` -- free function (not a table slot), called only from `CdStream__Open`. Evidence: fills a stack `SpuCommonAttr` (master volume near-max, CD channel volume near-max, CD mix enabled) and calls `SpuSetCommonAttr`. Mechanics are clear (SPU mix setup for CD-XA audio); exactly why it lives here rather than in Open itself is not established, so tier B.

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

## Constants (track 7, round 99)

`0x2C3` is `SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL |
SPU_COMMON_CDVOLR | SPU_COMMON_CDMIX` (`<libspu.h>`), exactly the five
fields the body sets; `cd.mix = 1` is `SPU_ON`. The volumes are
`CDSTREAM_MASTER_VOLUME` 0x3FFF and `CDSTREAM_CD_VOLUME` 0x7FFF, each the
maximum of its libspu range (master volume -0x4000..0x3FFF, CD input
-0x8000..0x7FFF), i.e. both at full. The unit now takes `SpuCommonAttr` and
`SpuSetCommonAttr` from Sony's header instead of re-declaring them.
