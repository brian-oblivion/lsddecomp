# Class866E8__SetChildParams — MATCH

> Renamed from `func_8004ACF8` on 2026-09-22 (tools/rename.py). Address 0x8004acf8.

**Unit:** class_3ac78 · **Size:** 51 instructions · **Result:** 51/51 words

## What it does

A simple counted loop: for `i` in `[0, count)`, fetch a child object via
`self->methods->slotB8(self, i)` and call two of ITS OWN vtable slots
(`+0x44`, `+0x48`) on it, each with a literal `1` and a running
accumulator that steps by a different amount per call (`arg3 += 3` for
`slot44`, `arg2 += 6` for `slot48`).

`slotB8` (`func_80042828`) and the child's own class are not decompiled;
only the two slots this function reaches on the child are typed, as
`UnkChildObj_3ac78`/`UnkChildMethods_3ac78` in `include/class_3ac78.h`.

## Final source

```c
void Class866E8__SetChildParams(Class866E8 *self, s32 count, s32 arg2, s32 arg3)
{
    s32 i;
    UnkChildObj_3ac78 *child;

    for (i = 0; i < count; i++) {
        child = self->methods->slotB8(self, i);
        child->methods->slot44(child, 1, arg3);
        arg3 += 3;
        child->methods->slot48(child, 1, arg2);
        arg2 += 6;
    }
}
```

## Residue

None — matched on the first attempt. A plain `for` loop with the two calls
and their accumulator updates written in straight source order was enough;
no reordering or barrier was needed.

## Provenance

round 2026-09-02, runner ALPHA, unit class_3ac78.
