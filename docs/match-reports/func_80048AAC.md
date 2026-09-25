# func_80048AAC -- MATCHED (51/51 words)

Round 82, runner echo (third echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the first build; whole-image SHA1 green, funcdiff 51/51.

## What it does

Slot +0x078 of D_80081940. With a buffer and a non-NULL name: reset (`unk2C
= 0` when idle, else cancelRequests), enter state 9, then close / open(name,
1, 0) / read(buffer, 0xB358) through the object's own (run-time-bound)
Class6D430 interface slots. State 9 is completed by func_800489B4 (setFlag).

## Source

```c
/* slot +0x078 of D_80081940: start streaming a file into the buffer */
void func_80048AAC(D_80081940Obj *self, char *name) {
    if (self->buffer != NULL && name != NULL) {
        if (self->unk2A == 0) {
            self->unk2C = 0;
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
