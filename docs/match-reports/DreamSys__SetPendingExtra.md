> Renamed from `func_80057C7C` on 2026-09-19 (tools/rename.py). Address 0x80057c7c.

# DreamSys__SetPendingExtra -- MATCHED (2/2)

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys`, own vtable slot `+0x0EC`
(base-class-inherited; `include/DreamSys.h` already named this slot in an
earlier commit this round).

## Body

```c
void DreamSys__SetPendingExtra(DreamSys *self, void *extra) {
    self->unk_0x54 = extra;
}
```

A single `sw` store. Newly names `DreamSys::unk_0x54` (splitting the
existing `unknown_values_0x50[8]` array, additive/size-preserving, into
`unknown_values_0x50[4]` + this new `void *unk_0x54`).

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py DreamSys__SetPendingExtra   # 2/2
```
