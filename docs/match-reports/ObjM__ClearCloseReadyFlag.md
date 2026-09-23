# ObjM__ClearCloseReadyFlag

> Renamed from `func_80054200` on 2026-09-23 (tools/rename.py). Address 0x80054200.

**Unit:** class_3bb8c_m · **Size:** 2 instructions · **Status:** MATCHED (2/2 words)

## What this function does

Trivial single-statement setter, whole body is `jr $ra` / `sw $zero,
0x84($a0)` (the store lives in the branch delay slot, executing before the
return completes).

```c
void ObjM__ClearCloseReadyFlag(ObjM *self) {
    self->unk84 = 0;
}
```

## Residue

None.

## Provenance

round 15 (2026-09-04), runner echo, fresh carve `class_3bb8c_m`.
