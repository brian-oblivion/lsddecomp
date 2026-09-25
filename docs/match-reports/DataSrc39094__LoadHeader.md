# DataSrc39094__LoadHeader -- MATCHED (51/51 words)

> Renamed from `func_80048AAC` on 2026-09-25 (tools/rename.py). Address 0x80048aac.

Round 82, runner echo (third echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the first build; whole-image SHA1 green, funcdiff 51/51.

## What it does

Slot +0x078 of D_80081940. With a buffer and a non-NULL name: reset (`headerReady
= 0` when idle, else cancelRequests), enter state 9, then close / open(name,
1, 0) / read(buffer, 0xB358) through the object's own (run-time-bound)
Class6D430 interface slots. State 9 is completed by DataSrc39094__SetFlag (setFlag).

## Source

```c
/* slot +0x078 of D_80081940: start streaming a file into the buffer */
void DataSrc39094__LoadHeader(DataSrc39094 *self, char *name) {
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

- **Name:** `DataSrc39094__LoadHeader`
- **Tier:** A
- **Evidence:** matches the unit's own established fact 'state 9 = header load': sets state to 9 and issues close/open/read of the header buffer.
