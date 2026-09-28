# LinkResource__LinkResource -- MATCHED (41/41 words)

> Renamed from `func_800438B0` on 2026-09-25 (tools/rename.py). Address 0x800438b0.

Round 82, runner echo (GraphicsResources session, echo #8), 2026-09-25. Unit `GraphicsResources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 41/41 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Constructor: active driver's ctor, install gLinkResourceMethods (GetLinkResourceMethods); with a non-NULL { buffer, name } descriptor: adopt a given buffer (+0x10, size +0x14 = 0) and call its own +0x064 (LinkResource__BuildModels), returning NULL on a nonzero result; or, with no buffer, call its own requestLoadFile (+0x06C) with the name. Returns self otherwise.

Table slot (`tools/classtable.py`): gLinkResourceMethods +0x008.

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable` and `SubBlockTable` sit at the top of
`src/graphics/GraphicsResources.c`.

```c
typedef struct ResourceSource {
    /* +0x00 */ void *buffer;
    /* +0x04 */ char *name;
} ResourceSource;

/* gLinkResourceMethods +0x008: constructor -- the active driver's, then this table;
 * with a descriptor, adopt its buffer (size 0) and run its own +0x064, whose
 * nonzero result fails the construction (NULL), or else request its file. */
void *LinkResource__LinkResource(DataSrc33808 *self, ResourceSource *src) {
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetLinkResourceMethods();
    if (src != NULL) {
        if (src->buffer != NULL) {
            self->buffer = src->buffer;
            self->bufferSize = 0;
            if (((s32 (*)())self->methods->setFlag)(self)) {
                goto fail;
            }
        } else {
            self->methods->requestLoadFile(self, src->name);
        }
    }
    return self;
fail:
    return NULL;
}
```

## Notes

Matched on build 7. **Lever: a `goto fail` to a `return NULL` placed AFTER the final `return self`.** Retail has `bnez v0, <epilogue>; move v0,zero; j <epilogue>; move v0,s0`: the failure branch goes straight to the epilogue with the NULL set in its delay slot (reorg stole it from a trailing fail block, which then died). Measured variants:

| source shape | result |
| --- | --- |
| `if (r) return NULL; return self;` inside the buffer branch | 23/41, 1 word LONG: an extra `move v1,v0` before `bnez v1` (return-value set hoisted before the test) |
| same with the cast returning `void *`/`u8`/`s16` | unchanged, 23/41 long |
| `if (r) return NULL;` falling through to the common `return self` (either if/else order) | 39/41 equal length, branch inverted (`beqz v0 -> return self`, j-delay nop) |
| `if (r == 0) return self; return NULL;` | 38/41: branchless sltiu/negu/and |
| `return r ? NULL : self;` | 38/41: branchless |
| `if (r) goto fail;` ... `return self; fail: return NULL;` | **41/41** |

ResourceSource moved up the file to precede this function (no layout change); Tod__Tod uses it unchanged.

## Naming

- **LinkResource__LinkResource**, tier B (head review, round 83: was A). Constructor of the class external code already names LinkResource.
  Head review, round 83: the class name rests on one caller's local view type, named by an earlier runner (DayTaskStageMap.c round 20 for LinkResource; dream_aux.c round 43 for TriggerWorld), not on this body. The body shows mechanics only, so tier B; track 4 may sharpen it.
- **ResourceSource** (type, was `Src6F240`), tier A (round 95, bravo, track 6). This ctor, and Tod's, TodSet's, ModelData's and TriggerWorld's, read `src->buffer` and adopt it when non-NULL, else call `requestLoadFile(self, src->name)`: a buffer to adopt, or a file to request. Named for that mechanism, not for any one class, since five ctors share it. Callers in DayTaskStageMap.c, GameApplicationFileResource.c, class_3bb8c.c, dream_aux.c and TodActor.c pass larger locals of their own cast to it (their stack sizes matter), so it stays defined in src/graphics/GraphicsResources.c; the five class headers forward-declare it.

## Track 4

2026-09-26, round 89 (delta): LinkResource (table `gLinkResourceMethods`,
renamed from D_8006F13C) is unified in `include/LinkResource.h`. The
unit-local views this body used (`DataSrc33808`, `Obj6F13C`, `Buf439EC`,
`Rec6F13C`/`Buf6F13C`, the `extern s32 D_8006F13C[]` array) are gone:
`self` is `LinkResource *`, its +0x02C is `TmdModel **models`, the buffer is
read as `TmdFile *` (include/TmdModel.h), the allocator's descriptor is
`ResourceSource *`, and the getter returns `&gLinkResourceMethods`.
Byte-identical.

## History (moved from include/LinkResource.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
 * The name is round 20's, from DayTaskStageMap.c's local view of the object
 * StageMap__PopulateSlotCells stores in a PlacementGrid's `linkResource`;
 * it is kept on this evidence: the class's own methods map the file's TMD
```
