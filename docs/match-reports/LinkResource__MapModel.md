# LinkResource__MapModel -- MATCHED (9/9 words)

> Renamed from `func_80043B18` on 2026-09-25 (tools/rename.py). Address 0x80043b18.

Round 82, runner echo (code_33808 session, echo #6), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 9/9 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Tail of `GsMapModelingData((u32 *)self->buffer + 1)`: the buffer holds a TMD, and +4 skips its id word (the TMD `flags` word is what GsMapModelingData expects to start from). Prototype declared locally (LIBGS.H spelling in a comment).

Table slot (`tools/classtable.py`): gLinkResourceMethods +0x078.

## Source

The unit-local view `DataSrc33808` (a Class6D430 subclass built with the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
/* LIBGS.H: void GsMapModelingData(unsigned long *p); */
void GsMapModelingData(u32 *p);

/* gLinkResourceMethods +0x078: map the TMD in the buffer (past its id word). */
void LinkResource__MapModel(Class6D430 *self) {
    GsMapModelingData((u32 *)self->buffer + 1);
}
```

## Notes

- No shared header was edited. `Class6D430.h`, `SceneNode.h`, `BasicClass.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **LinkResource__MapModel**, tier A. Slot +0x078: GsMapModelingData wrapper over the buffer's TMD.

## Track 4

2026-09-26, round 89 (delta): LinkResource (table `gLinkResourceMethods`,
renamed from D_8006F13C) is unified in `include/LinkResource.h`. The
unit-local views this body used (`DataSrc33808`, `Obj6F13C`, `Buf439EC`,
`Rec6F13C`/`Buf6F13C`, the `extern s32 D_8006F13C[]` array) are gone:
`self` is `LinkResource *`, its +0x02C is `TmdModel **models`, the buffer is
read as `TmdFile *` (include/TmdModel.h), the allocator's descriptor is
`Src6F240 *`, and the getter returns `&gLinkResourceMethods`.
Byte-identical.
