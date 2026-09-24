# New_BaseObjO -- MATCHED (24/24 words)

> Renamed from `func_80056FE4` on 2026-09-18 (tools/rename.py). Address 0x80056fe4.

Unit: `class_3bb8c_o` (round 17). The BaseObjO allocator -- allocates 0x58
bytes, constructs, frees and returns `NULL` on construction failure.

## Final source

```c
extern void *BMemPMgrAlloc(s32 size);
extern void *BMemPMgrFree(void *ptr);
extern BaseObjOMethods *DreamSys__GetBaseMethods(void);

void *New_BaseObjO(void) {
    BaseObjO *self = BMemPMgrAlloc(0x58);

    if (self != NULL) {
        if (DreamSys__GetBaseMethods()->ctor(self) != NULL) {
            return self;
        }
        BMemPMgrFree(self);
        return NULL;
    }
    return NULL;
}
```

## Derivation

This is the `New_X` allocator sub-shape #3 from `DECOMPILATION_LEARNINGS.md`
("tests BOTH the allocation and the constructor's return, freeing on
constructor failure -- plain `if`/`return`"), confirmed by disassembly: a
literal `ori $a0,$zero,0x58` allocation, an unconditional test of the
result, then `DreamSys__GetBaseMethods()->ctor(self)`'s OWN return checked against
NULL -- nonzero means success (return `self`), zero means failure
(`BMemPMgrFree(self)` then return `NULL`). `DreamSys__GetBaseMethods` is already
declared elsewhere (`code_55dd4.h`) returning `D800878D4Methods *`; this
unit uses its own local `BaseObjOMethods *` reading of the same table (see
`BaseObjO__BaseObjO`'s report). `BMemPMgrAlloc`/`BMemPMgrFree` are the
project's established single-argument pool allocator pair.

`0x58` is a literal allocation size, not `sizeof(BaseObjO)`, per the
project's established "`sizeof` is not how this game allocates" idiom --
`BaseObjO` need not (and does not) claim to be exactly 0x58 bytes wide in
this header; only the fields actually touched by this unit's functions are
named, all of which fit within the first 0x58 bytes.

### Proposed learning

None beyond the already-documented `New_X` sub-shape #3.

## Naming

**`New_BaseObjO` -- tier A.** Matches the project's documented `New_X`
allocator sub-shape #3 exactly (alloc, test, ctor, test-ctor's-own-return,
free-on-failure) -- mechanics are the entire visible purpose of this
function, which is why the previous round's own report already used this
name in prose before anyone renamed it. Constructs a `BaseObjO`, not a
`Class65650`: it calls `DreamSys__GetBaseMethods()->ctor(self)` with a SINGLE
argument, which only makes sense if that slot resolves to
`BaseObjO__BaseObjO` (this unit's own 1-argument base ctor) rather than to
`Class65650`'s real, 3-argument constructor
(`code_55dd4.c:class_65650__Constructor`) -- confirmed directly by
`DreamSys__GetBaseMethods`'s declared return type, `BaseObjOMethods *` here (this
unit's own reading of the SAME table `code_55dd4.h` calls
`D800878D4Methods`).
