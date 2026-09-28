# New_Actor -- MATCHED (24/24 words)

> Renamed from `New_BaseObjO` on 2026-09-25 (tools/rename.py). Address 0x80056fe4.

> Renamed from `func_80056FE4` on 2026-09-18 (tools/rename.py). Address 0x80056fe4.

Unit: `ObjMStyleActor` (round 17). The BaseObjO allocator -- allocates 0x58
bytes, constructs, frees and returns `NULL` on construction failure.

## Final source

```c
extern void *BMemPMgrAlloc(s32 size);
extern void *BMemPMgrFree(void *ptr);
extern BaseObjOMethods *GetActorMethods(void);

void *New_Actor(void) {
    BaseObjO *self = BMemPMgrAlloc(0x58);

    if (self != NULL) {
        if (GetActorMethods()->ctor(self) != NULL) {
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
result, then `GetActorMethods()->ctor(self)`'s OWN return checked against
NULL -- nonzero means success (return `self`), zero means failure
(`BMemPMgrFree(self)` then return `NULL`). `GetActorMethods` is already
declared elsewhere (`TodActor.c`) returning `D800878D4Methods *`; this
unit uses its own local `BaseObjOMethods *` reading of the same table (see
`Actor__Actor`'s report). `BMemPMgrAlloc`/`BMemPMgrFree` are the
project's established single-argument pool allocator pair.

`0x58` is a literal allocation size, not `sizeof(BaseObjO)`, per the
project's established "`sizeof` is not how this game allocates" idiom --
`BaseObjO` need not (and does not) claim to be exactly 0x58 bytes wide in
this header; only the fields actually touched by this unit's functions are
named, all of which fit within the first 0x58 bytes.

### Proposed learning

None beyond the already-documented `New_X` sub-shape #3.

## Naming

**`New_Actor` -- tier A.** Matches the project's documented `New_X`
allocator sub-shape #3 exactly (alloc, test, ctor, test-ctor's-own-return,
free-on-failure) -- mechanics are the entire visible purpose of this
function, which is why the previous round's own report already used this
name in prose before anyone renamed it. Constructs a `BaseObjO`, not a
`TodActor`: it calls `GetActorMethods()->ctor(self)` with a SINGLE
argument, which only makes sense if that slot resolves to
`Actor__Actor` (this unit's own 1-argument base ctor) rather than to
`TodActor`'s real, 3-argument constructor
(`TodActor.c:TodActor__TodActor`) -- confirmed directly by
`GetActorMethods`'s declared return type, `BaseObjOMethods *` here (this
unit's own reading of the SAME table `TodActor.c` calls
`D800878D4Methods`).

## Track 4 (2026-09-25, round 82, delta)

Renamed from `New_BaseObjO`. BMemPMgrAlloc(0x58) then the table's ctor: the allocator, and the source of the class size 0x58. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/ObjMStyleActor.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 99, alpha)

The allocation is `sizeof(Actor)` (0x58, the header's layout ends there): the same li.
