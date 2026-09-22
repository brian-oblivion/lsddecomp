# GetClass86AA0Methods

> Renamed from `func_8004D508` on 2026-09-22 (tools/rename.py). Address 0x8004d508.

**Unit:** class_3bb8c_c · **Size:** 4 words · **Status:** MATCHED (4/4)

## What it does

Get-vtable helper for a second small sibling class: returns `&D_80086AA0`,
this unit's `Class86AA0Methods`. Same shape and role as GetClass869D8Methods.

## The C

```c
Class86AA0Methods *GetClass86AA0Methods(void)
{
    return &D_80086AA0;
}
```

## New type: Class86AA0 / Class86AA0Methods

`D_80086AA0` (asm/data/76DC8.data.s, header word 0x24) is D_800869D8's
sibling: same opening shape (header, then `BasicClass__func_17eb0` at
+0x004, ctor at +0x008 -- here Class86AA0__Class86AA0). Only +0x008 (`ctor`) and
+0x0B8 (dispatched by Class86AA0__ForwardIfTag34, this unit's own slot +0x09C in the
same table) are typed; see `include/class_3bb8c.h`.

## Proposed learning

See GetClass869D8Methods.md -- same finding, second instance.
