# ModelData__BuildResources -- MATCHED (40/40 words)

> Renamed from `func_80044858` on 2026-09-25 (tools/rename.py). Address 0x80044858.

Round 82, runner echo (code_33808 session, echo #8), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 40/40 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

When +0x34 is set: fills a 3-word stack request with SetVec3(&req, buffer + buffer[+8], 0, 1) and constructs a D_8006F13C object from it (New_LinkResource) into +0x2C; if that succeeded, points req.buffer at buffer + 0x0C and constructs a D_8006F590 object (New_TodSet) into +0x30; returns 0 when both exist. On either failure it calls its own +0x07C (ModelData__ReleaseResources, which releases what was built) and returns 1. With +0x34 clear, returns 0.

Table slot (`tools/classtable.py`): D_8006F384 +0x078 (called by ModelData__Load, its setFlag override).

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808` and `CountedBuf33808` sit at the top of
`src/code_33808.c`.

```c
#include "ModelData.h"

typedef struct Req44858 {
    /* +0x00 */ void *buffer;
    /* +0x04 */ s32 unk4;
    /* +0x08 */ s32 unk8;
} Req44858;

typedef struct Buf44858 {
    /* +0x00 */ u8 pad0[8];
    /* +0x08 */ s32 offset;
} Buf44858;

s32 ModelData__BuildResources(ModelData *self) {
    Req44858 req;

    if (self->ownsResources != 0) {
        SetVec3(&req, (u8 *)self->buffer + ((Buf44858 *)self->buffer)->offset, 0, 1);
        self->linkResource = New_LinkResource((s32)&req);
        if (self->linkResource != NULL) {
            req.buffer = (u8 *)self->buffer + 0xC;
            self->todSet = New_TodSet((s32)&req);
            if (self->todSet != NULL) {
                return 0;
            }
            self->todSet = NULL;
        }
        self->methods->releaseResources(self);
        return 1;
    }
    return 0;
}
```

## Notes

First build. The redundant `sw zero, 0x30` on the second failure is an explicit `self->unk30 = NULL;` in the source. The request is the same { buffer, name/0, mode } descriptor that D_8006F240's ctor (Tod__Tod) reads; SetVec3 (code_171e0) is declared unprototyped here since each unit carries its own reading of it. The allocators take `s32` in this unit, so the request address is cast.

## Naming

- **ModelData__BuildResources**, tier A. Slot +0x078: builds the LinkResource (tmd) and TodSet (tods) sub-objects over the buffer's two sub-blocks.

## Track 4

2026-09-25, round 84 (delta): ModelData (D_8006F384) is unified in `include/ModelData.h`; the unit-shared `DataSrc33808` view no longer types it. +0x02C is `linkResource` (`Class6D430 *`, New_LinkResource's result; the `(s32)` and `(void *)` casts are gone), +0x030 is `todSet` (`Class6D430 *`, New_TodSet's result) and +0x034 is `ownsResources`. The failure path calls `releaseResources(self)` (slot +0x07C). Image byte-identical.

## Track 4 (2026-09-26, round 88, delta)

New_TodSet is now prototyped in include/TodSet.h as `TodSet *New_TodSet(struct Src6F240 *)`, so the call reads `self->todSet = (Class6D430 *)New_TodSet((Src6F240 *)&req)`; the forward declaration that stood before this function is gone. Bytes unchanged.
