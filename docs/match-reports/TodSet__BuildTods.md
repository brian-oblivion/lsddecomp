# TodSet__BuildTods -- MATCHED (56/56 words)

> Renamed from `func_800452FC` on 2026-09-25 (tools/rename.py). Address 0x800452fc.

Round 82, runner echo (code_33808 session, echo #9), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 56/56 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Build step (gTodSetMethods's setFlag override): SetVec3 fills a three-word request {buffer, 0, 1}; for each of the buffer's `count` offsets, point the request at buffer+offset and allocate a gTodMethods source (New_Tod) over it, storing the object over the offset word. On a NULL, walk back releasing (slot +0x004) every one already built and return 1; otherwise 0.

Table slot (`tools/classtable.py`): gTodSetMethods +0x064.

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808`, `CountedBuf33808` and `Req44858` sit at the
top of / earlier in `src/code_33808.c`.

```c
/* gTodSetMethods +0x064: build a gTodMethods source over each sub-block of the
 * buffer's counted offset table, into the table's own words; 0 when all
 * exist, otherwise release the ones already built and 1. */
s32 TodSet__BuildTods(DataSrc33808 *self) {
    Req44858 req;
    CountedBuf33808 *buf;
    DataSrc33808 **p;
    s32 i;
    s32 n;

    SetVec3(&req, 0, 0, 1);
    buf = self->buffer;
    i = 0;
    n = buf->count;
    p = (DataSrc33808 **)buf->entries;
    for (; i < n; i++) {
        req.buffer = (u8 *)self->buffer + ((CountedBuf33808 *)self->buffer)->entries[i];
        *p = New_Tod((s32)&req);
        if (*p == NULL) {
while (i != 0) {
 i--;
 p--;
 (*p)->methods->release(*p);
 }
            return 1;
        }
        p++;
    }
    return 0;
}
```

## Notes

Fourth build. The first shape, `for (p--; i != 0; i--, p--) release(*p);`, measured 22/56 with i/p swapped between s0/s1 and the decrements scheduled differently. Three while/for forms with the decrement of p at the TOP of the body all match: `while (i != 0) { p--; release(*p); i--; }`, `for (; i != 0; i--) { p--; release(*p); }`, and `while (i != 0) { i--; p--; release(*p); }` (kept). `while (i-- != 0) { p--; ... }` differs. An index form (`buf->entries[i] = ...`, release `buf->entries[--i]`) was far worse (5/56, 0x48 frame, two extra saved registers).

## Naming

- **TodSet__BuildTods**, tier A. Slot +0x064: builds a Tod over each sub-block of the buffer's counted offset table; releases what was built so far on an allocation failure.

## Track 4 (2026-09-26, round 86, charlie)

Touched by Tod's unification (charlie): the call is now `*p = (DataSrc33808 *)New_Tod((Src6F240 *)&req);`, because New_Tod is prototyped `Tod *New_Tod(Src6F240 *)` in include/Tod.h. Two pointer casts, no code; this function's own views (TodSet's) are unchanged. Bytes unchanged.

## Track 4 (2026-09-26, round 88, delta)

Now `s32 TodSet__BuildTods(TodSet *self)` (include/TodSet.h). The counted array holds `Tod *` (New_Tod's return, released through Tod's +0x004), no longer DataSrc33808 *. It sits in the inherited +0x064 `setFlag` slot and keeps its own name: it builds the Tods, which is more than the slot name says. Bytes unchanged.
