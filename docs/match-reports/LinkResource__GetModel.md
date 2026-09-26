# LinkResource__GetModel -- MATCHED (6/6 words)

> Renamed from `LinkResource__GetEntry` on 2026-09-26 (tools/rename.py). Address 0x80043b58.

> Renamed from `func_80043B58` on 2026-09-25 (tools/rename.py). Address 0x80043b58.

Round 82, runner echo (code_33808 session, echo #5), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 6/6 words, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Indexed getter: `lw v0,0x2C(a0); sll a1,2; addu; lw v0,0(a1)` = `return self->entries[index];`
with `entries` an `s32 *` at +0x2C. Class6D430 (UNIFIED, `include/Class6D430.h`) is 0x2C
bytes, so +0x2C is the LinkResource subclass's own first field; a unit-local minimal
view `Obj6F13C` (pad to +0x2C, then the pointer) carries it rather than touching the
shared header. Return type `s32` is a reading of one `lw`, not a proven type.

Table slot (`tools/classtable.py gLinkResourceMethods`): `gLinkResourceMethods` +0x080.

## Source

```c
/* gLinkResourceMethods +0x080: returns entry `index` of the word array at +0x2C
 * (the first field past the 0x2C-byte Class6D430 base). */
typedef struct Obj6F13C {
    /* +0x000 */ u8 pad0[0x2C];
    /* +0x02C */ s32 *entries;
} Obj6F13C;

s32 LinkResource__GetModel(Obj6F13C *self, s32 index) {
    return self->entries[index];
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  sit directly above the function in `src/code_33808.c`.

## Naming

- **LinkResource__GetModel**, tier A. Slot +0x080: entry `index` of the word array LinkResource__BuildModels filled.

## Track 4

2026-09-26, round 89 (delta): renamed from `LinkResource__GetEntry` with
`tools/rename.py`. The word array at +0x2C is the one LinkResource__BuildModels
fills with `New_TmdModel` results, so entry `index` is a TmdModel; the two
callers that use the value confirm it: code_55dd4.c's TOD model-id packet
passes it to `Class6B5CC__LinkModel` (whose `model` field holds a TmdModel,
include/Class6B5CC.h), and class_3bb8c.c's Class866E8__LoadElementResources
reads its +0x010 (TmdModel's `object`) through Class6D940__ResolveEntry. The
slot is `getModel`, returning `TmdModel *` (include/LinkResource.h).
