# LinkResource__GetTmdObject -- MATCHED (7/7 words)

> Renamed from `LinkResource__GetRecord` on 2026-09-26 (tools/rename.py). Address 0x80043b3c.

> Renamed from `func_80043B3C` on 2026-09-25 (tools/rename.py). Address 0x80043b3c.

Round 82, runner echo (GraphicsResources session, echo #6), 2026-09-25. Unit `GraphicsResources`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 7/7 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

`sll 3; subu; sll 2` (index*28), `lw 0x10(a0)` (FileResource `buffer`), `addiu 0xC`, `addu` = `&((Buf6F13C *)self->buffer)->recs[index]` with 0x1C-byte records from +0x0C of the buffer. Same shape as charlie's round-82 lever in TmdModel.

Table slot (`tools/classtable.py`): gLinkResourceMethods +0x07C.

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/graphics/GraphicsResources.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
/* gLinkResourceMethods +0x07C: the address of record `index`, 0x1C bytes each,
 * from +0x0C of the buffer. */
typedef struct Rec6F13C {
    u8 data[0x1C];
} Rec6F13C;

typedef struct Buf6F13C {
    /* +0x00 */ u8 pad0[0xC];
    /* +0x0C */ Rec6F13C recs[1];
} Buf6F13C;

Rec6F13C *LinkResource__GetTmdObject(FileResource *self, s32 index) {
    return &((Buf6F13C *)self->buffer)->recs[index];
}
```

## Notes

- No shared header was edited. `FileResource.h`, `scene_node.h`, `basic_class.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **LinkResource__GetTmdObject**, tier A. Slot +0x07C: address of record `index`, 0x1C bytes each.

## Track 4

2026-09-26, round 89 (delta): renamed from `LinkResource__GetRecord` with
`tools/rename.py`. The buffer is a TMD file: LinkResource__MapModel hands
`buffer + 4` (TmdFile's `flags`) to GsMapModelingData, and
LinkResource__BuildModels reads the object count at +0x08 (`nobj`) and
builds one TmdModel per 0x1C-byte record from +0x0C (`objects[]`,
TmdObject, include/TmdModel.h). Record `index` is therefore
`&((TmdFile *)buffer)->objects[index]`, a `TmdObject *`; the slot is
`getTmdObject` (include/LinkResource.h).
