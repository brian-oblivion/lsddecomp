# ModelData__ModelData -- MATCHED (46/46 words)

> Renamed from `func_800446FC` on 2026-09-25 (tools/rename.py). Address 0x800446fc.

Round 82, runner echo (code_33808 session, echo #8), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 46/46 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Constructor: active driver's ctor, install D_8006F384 (GetModelDataMethods), store the third argument at +0x34 (the flag that ModelData__BuildResources tests before building the two sub-sources and ModelData__ReleaseResources before releasing them; named `owns` as a reading, not evidence); then adopt the descriptor's buffer (size 0) and call its own +0x064 (ModelData__Load), returning NULL on a nonzero result, or request the descriptor's file.

Table slot (`tools/classtable.py`): D_8006F384 +0x008 (the allocator New_ModelData passes 1 as the third argument; the subclass D_8006F40C's ctor TriggerWorld__TriggerWorld passes 0).

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808` and `CountedBuf33808` sit at the top of
`src/code_33808.c`.

```c
/* D_8006F384 +0x008: constructor -- the active driver's, then this table,
 * `owns` at +0x34; adopt the descriptor's buffer (size 0) and run its own
 * +0x064, whose nonzero result fails the construction (NULL), or else
 * request its file. */
void *ModelData__ModelData(DataSrc33808 *self, Src6F240 *src, s32 owns) {
    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetModelDataMethods();
    self->unk34 = owns;
    if (src->buffer != NULL) {
        self->buffer = src->buffer;
        self->bufferSize = 0;
        if (((s32 (*)())self->methods->setFlag)(self)) {
            goto fail;
        }
    } else {
        self->methods->requestLoadFile(self, src->name);
    }
    return self;
fail:
    return NULL;
}
```

## Notes

First build, using the `goto fail` lever just found on LinkResource__LinkResource (fail label after the final `return self`). Unlike LinkResource__LinkResource the descriptor is not NULL-tested.

## Naming

- **ModelData__ModelData**, tier A. Constructor: adopts a buffer or requests a file, `owns` stored at +0x34.
