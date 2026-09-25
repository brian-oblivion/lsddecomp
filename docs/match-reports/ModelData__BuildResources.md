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
typedef struct Req44858 {
    /* +0x00 */ void *buffer;
    /* +0x04 */ s32 unk4;
    /* +0x08 */ s32 unk8;
} Req44858;

typedef struct Buf44858 {
    /* +0x00 */ u8 pad0[8];
    /* +0x08 */ s32 offset;
} Buf44858;

s32 ModelData__BuildResources(DataSrc33808 *self) {
    Req44858 req;

    if (self->unk34 != 0) {
        SetVec3(&req, (u8 *)self->buffer + ((Buf44858 *)self->buffer)->offset, 0, 1);
        self->unk2C = (s32)New_LinkResource((s32)&req);
        if ((void *)self->unk2C != NULL) {
            req.buffer = (u8 *)self->buffer + 0xC;
            self->unk30 = New_TodSet((s32)&req);
            if (self->unk30 != NULL) {
                return 0;
            }
            self->unk30 = NULL;
        }
        self->methods->slot7C(self);
        return 1;
    }
    return 0;
}
```

## Notes

First build. The redundant `sw zero, 0x30` on the second failure is an explicit `self->unk30 = NULL;` in the source. The request is the same { buffer, name/0, mode } descriptor that D_8006F240's ctor (Tod__Tod) reads; SetVec3 (code_171e0) is declared unprototyped here since each unit carries its own reading of it. The allocators take `s32` in this unit, so the request address is cast.
