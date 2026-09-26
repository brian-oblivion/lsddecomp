# Class81940__LoadHeader -- MATCHED (51/51 words)

> Renamed from `DataSrc39094__LoadHeader` on 2026-09-26 (tools/rename.py). Address 0x80048aac.

> Renamed from `func_80048AAC` on 2026-09-25 (tools/rename.py). Address 0x80048aac.

Round 82, runner echo (third echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the first build; whole-image SHA1 green, funcdiff 51/51.

## What it does

Slot +0x078 of gClass81940Methods. With a buffer and a non-NULL name: reset (`headerReady
= 0` when idle, else cancelRequests), enter state 9, then close / open(name,
1, 0) / read(buffer, 0xB358) through the object's own (run-time-bound)
Class6D430 interface slots. State 9 is completed by Class81940__AdvanceLoadState (setFlag).

## Source

```c
/* slot +0x078 of gClass81940Methods: start streaming a file into the buffer */
void Class81940__LoadHeader(DataSrc39094 *self, char *name) {
    if (self->buffer != NULL && name != NULL) {
        if (self->unk2A == 0) {
            self->headerReady = 0;
        } else {
            self->methods->cancelRequests(self);
        }
        self->unk2A = 9;
        self->methods->close(self);
        self->methods->open(self, name, 1, 0);
        self->methods->read(self, self->buffer, 0xB358);
    }
}
```

## Naming

- **Name:** `Class81940__LoadHeader`
- **Tier:** A
- **Evidence:** matches the unit's own established fact 'state 9 = header load': sets state to 9 and issues close/open/read of the header buffer.

## Track 4 (2026-09-26, round 87)

Renamed `DataSrc39094__LoadHeader` -> `Class81940__LoadHeader` with `rename.py` (class rename only). It occupies +0x078, Class6D430's `void *slot78`; the slot keeps the inherited type and its one caller, Class866E8__ApplyRateEntries (src/class_3bb8c.c), calls it through `Class81940LoadHeaderFn` (track 4 step 6). The class (method table gClass81940Methods, id 0x903, a Class6D430 subclass) was named `Class81940` for its table address, 0x80081940 (renamed from `D_80081940` to `gClass81940Methods`), as Class6D940 is (FINISHING-PLAN track 4 step 2); the old `DataSrc39094` was the unit's local view name, and dropping its unit suffix leaves `DataSrc`, which every Class6D430 subclass is. The unified definition is `include/Class81940.h`.
