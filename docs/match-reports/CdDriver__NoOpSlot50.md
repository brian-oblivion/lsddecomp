# CdDriver__NoOpSlot50 -- MATCHED (2/2 words, splat-generated)

> Renamed from `Class6D4E8__NoOpSlot50` on 2026-09-26 (tools/rename.py). Address 0x800276c8.

> Renamed from `func_800276C8` on 2026-09-25 (tools/rename.py). Address 0x800276c8.

Unit `CdDriver`. A bare `jr $ra; nop` leaf that splat matched at carve
time (round 47); it had no report until this naming pass.

```c
void CdDriver__NoOpSlot50(void) {
}
```

## Naming

Round 79 (charlie), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800276C8` | `CdDriver__NoOpSlot50` | A |

**Evidence.** `tools/classtable.py gCdDriverMethods` lists it at `+0x050`, between
`CdDriver__Seek` (+0x04C) and `CdDriver__Read` (+0x054); the base class
`gFileResourceMethods` leaves that slot null. The body is empty, so the mechanics are
the whole of it: the `Class__NoOpSlotNN` form the symbols file already uses
(`GameApplication__NoOpSlot5C`, `ObjM__NoOpSlot40`). No dispatch of THIS class's
+0x50 was identified (the local views of `gCdDriverMethods` and `gFileResourceMethods` all
pad over it), so what the slot is FOR is unknown.


Track 4, 2026-09-26 (round 88). The class of gCdDriverMethods (was D_8006D4E8, id 0x13 = DATASOURCE_CD) is CdDriver, in include/CdDriver.h: its ctor calls InitCdDrive, its slots enqueue CD_OP_* requests and drive the CD read state machine, and it is VabDriver's sibling. The object views this function was typed against are replaced by CdDriver, whose fields are all FileResource's (the driver runs on its clients' objects; FileResource's +0x018/+0x01C were named pos/size for it). Byte-identical. `Class6D4E8__NoOpSlot50` -> `CdDriver__NoOpSlot50` by rename.py.
