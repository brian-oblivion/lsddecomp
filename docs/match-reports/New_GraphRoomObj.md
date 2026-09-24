# New_GraphRoomObj -- MATCHED (24/24)

> Renamed from `func_80057F68` on 2026-09-24 (tools/rename.py). Address 0x80057f68.

Unit: `src/class_3bb8c_t.c`. Class: `D_800879C4` family -- plain
allocator/constructor wrapper (`New_X` shape), not a vtable slot.

## Signature

```c
void *New_GraphRoomObj(void *arg1);
```

## Body

```c
void *New_GraphRoomObj(void *arg1) {
    void *obj = BMemPMgrAlloc(0x244);
    if (obj != NULL) {
        GetGraphRoomMethods()->ctor(obj, arg1);
        return obj;
    }
    return NULL;
}
```

Textbook `New_X`: allocate `0x244` bytes, null-check, call the
constructor fetched from `gGraphRoomMethods`'s `+0x008` slot (via this unit's
own `GetGraphRoomMethods`, still queued -- forward-declared here) with `(obj,
arg1)`, return the allocation regardless of the ctor's own return value.
`return obj;` sits INSIDE the success `if`, with a trailing `return
NULL;` -- the shape established as necessary for this exact pattern in
`class_3bb8c_p`'s `New_D800879C4` report.

This introduces this unit's own view of `gGraphRoomMethods` (73 slots,
`D_80087AACMethods`/`D_80087AACObj`, currently typing only the ctor slot
`+0x008`) -- this unit owns the WHOLE class (ctor, dtor, every slot are
all in this file), so the struct will grow as more of its functions are
matched.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py New_GraphRoomObj   # 24/24
```
