# New_SceneNode

> Renamed from `New_Class6B5CC` on 2026-09-26 (tools/rename.py). Address 0x8001ca94.

> Renamed from `func_8001CA94` on 2026-09-23 (tools/rename.py). Address 0x8001ca94.

**Unit:** SceneNode · **Size:** 24 words · **Status:** MATCHED (24/24 words)

## What it does

`New_SceneNode`: the allocator wrapper for the unnamed class whose method
table is `gSceneNodeMethods` (`tools/classtable.py gSceneNodeMethods`). Allocates 0x44
bytes, calls the class's own constructor slot (found through the
no-argument getter `GetSceneNodeMethods`, which just returns `&gSceneNodeMethods`) on
the fresh block, and on constructor failure frees the block and returns
`NULL`. This is NOT itself a vtable slot — it is the standalone `New_X`
entry point that callers elsewhere in the game presumably use to spawn an
instance.

## The C

```c
SceneNodeObj *New_SceneNode(void) {
    SceneNodeObj *obj;

    obj = BMemPMgrAlloc(0x44);
    if (obj == NULL) {
        return NULL;
    }
    if (GetSceneNodeMethods()->ctor(obj) != NULL) {
        return obj;
    }
    BMemPMgrFree(obj);
    return NULL;
}
```

Same shape as `src/world/entity.c`'s `New_Entity`.

## Establishing the class

`GetSceneNodeMethods()` is a plain no-argument getter (`lui/addiu %hi/%lo
(gSceneNodeMethods); jr $ra`, MEASURED from its own disassembly in
`asm/SceneNode.s`, the next uncarved slice). Its whole-file `->ctor`
call here checked the return with `bnez`, and that observation, combined
with reading `SceneNode__SceneNode` (this class's ctor, `docs/match-
reports/SceneNode__SceneNode.md`) confirmed `SceneNode__SceneNode` really does return
`self` on success and `NULL` on failure — the two reports cross-check
each other.

See `include/scene_node.h`'s file banner for the full `classtable.py`
census this unit's header is built from.

## Provenance

round 11 (2026-09-03), runner charlie, unit SceneNode (fresh carve, first attempt).

## Naming

Round 71 (alpha). `func_8001CA94` -> `New_SceneNode`, **tier A**. Allocates 0x44 bytes, runs `GetSceneNodeMethods()->ctor` on it, frees and returns NULL if the ctor fails. The project's `New_Class` allocator convention (New_Entity, New_Actor). Not a table slot. Caller: Viewport__Viewport (task) stores the result.

## Round 101 (delta): track 7

Step 4 (constants): `0x44` -> `sizeof(SceneNode)` (the struct is 0x44 bytes, include/scene_node.h; the whole-image build proves the size). Byte-identical.

Step 5 (comments): the unit banner was rewritten as documentation (lifecycle, children, transform, attribute; the helpers' header). It held no project history. The banner before this pass, verbatim:

```c
/*
 * code_d294 -- SceneNode (include/scene_node.h), part 1 of 3: slots +0x000
 * to +0x070. New, the ctor (allocates the GsCOORDINATE2 and GsCOORD2PARAM)
 * and Finalize; the BasicClass child-list overrides, which link or unlink a
 * TmdModel child as it is added or removed; OnNotify, which dispatches on
 * the sender's class id; Reset (identity transform); UpdateRotation and
 * UpdateScale (set or add three Ratio16s into the GsCOORD2PARAM); attach to
 * and detach from a parent's coordinate; and the first five setters over
 * GsDOBJ2.attribute. Part 2 is scene_node.c, part 3 code_d294_c.c.
 */
```

## History (moved from src/SceneNode.c, comments pass)

The file's banner carried its edge evidence:

> Edges. The file starts after Sony's libgte/divgt4a and ends before the
> head of the old psyq_GsLinkObject4 (0xF770). It was carved as three
> slices (code_d294, _b, _c) and merged in round 101: tuboundary.py finds
> no rodata crossing and no forced boundary anywhere in the stretch, calls
> the first slice edge "boundary unlikely (single-user data)" and the
> second "boundary possible", and the content is one class throughout, so
> content decided the merge.
