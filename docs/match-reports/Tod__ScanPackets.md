# Tod__ScanPackets -- MATCHED (13/13 words)

> Renamed from `func_80043FB0` on 2026-09-25 (tools/rename.py). Address 0x80043fb0.

Round 82, runner echo (code_33808 session, echo #6), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 13/13 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Returns `(u8)self->methods->slot7C(self, arg1, arg2, (u8 *)self->buffer + 8)`: a0..a2 pass through, a3 is buffer+8, and the trailing `andi 0xFF` is the u8 return truncating an s32 slot result.

Table slot (`tools/classtable.py`): gTodMethods +0x078.

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/code_33808.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
/* gTodMethods +0x078: slot +0x07C over the buffer past its first two words. */
u8 Tod__ScanPackets(DataSrc33808 *self, s32 arg1, s32 arg2) {
    return self->methods->slot7C(self, arg1, arg2, (u8 *)self->buffer + 8);
}
```

## Notes

- No shared header was edited. `FileResource.h`, `SceneNode.h`, `BasicClass.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **Tod__ScanPackets**, tier A. Slot +0x078: forwards to slot7C (ScanTodPackets) over the buffer's packet data (past its first two words).

## Track 4 (2026-09-26, round 86, charlie)

Now `u8 Tod__ScanPackets(Tod *self, u8 *out, u32 *sel)` (include/Tod.h): the two pass-through arguments are ScanTodPackets' `out`/`sel`, and it calls the named slot `scanTodPackets` (+0x07C) instead of the unprototyped `slot7C`. The slot is typed `u8` as its occupant is; cc1 still emits the trailing `andi 0xFF` over a u8 slot result, so the s32 the old view gave it was not what the bytes needed. The function itself sits in FileResource's `slot78`, which keeps its name; ModelData__ForwardScanPackets casts it. Bytes unchanged.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `sel` | `tmdId` | B | see ScanTodPackets |
| `(u8 *)buffer + 8` | `TodFile` `frames` | A | Sony's TOD file header is 8 bytes (id, version, resolution, frame count); code_55dd4.h's TodHeader reads the same layout |
