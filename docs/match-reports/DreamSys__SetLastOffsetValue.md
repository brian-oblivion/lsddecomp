> Renamed from `func_80057C6C` on 2026-09-19 (tools/rename.py). Address 0x80057c6c.

# DreamSys__SetLastOffsetValue -- MATCHED (2/2)

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys`, own vtable slot `+0x0E4`
(base-class-inherited; `include/DreamSys.h` already named this slot in an
earlier commit this round).

## Body

```c
void DreamSys__SetLastOffsetValue(DreamSys *self, s16 val) {
    self->field_0x48 = val;
}
```

A single `sh` store, splat-matched-length two-word leaf.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py DreamSys__SetLastOffsetValue   # 2/2
```
