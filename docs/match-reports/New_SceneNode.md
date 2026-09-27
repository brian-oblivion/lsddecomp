# New_SceneNode

> Renamed from `New_Class6B5CC` on 2026-09-26 (tools/rename.py). Address 0x8001ca94.

> Renamed from `func_8001CA94` on 2026-09-23 (tools/rename.py). Address 0x8001ca94.

**Unit:** code_d294 · **Size:** 24 words · **Status:** MATCHED (24/24 words)

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

Same shape as `src/Entity.c`'s `New_Entity`.

## Establishing the class

`GetSceneNodeMethods()` is a plain no-argument getter (`lui/addiu %hi/%lo
(gSceneNodeMethods); jr $ra`, MEASURED from its own disassembly in
`asm/code_d294_b.s`, the next uncarved slice). Its whole-file `->ctor`
call here checked the return with `bnez`, and that observation, combined
with reading `SceneNode__SceneNode` (this class's ctor, `docs/match-
reports/SceneNode__SceneNode.md`) confirmed `SceneNode__SceneNode` really does return
`self` on success and `NULL` on failure — the two reports cross-check
each other.

See `include/code_d294.h`'s file banner for the full `classtable.py`
census this unit's header is built from.

## Provenance

round 11 (2026-09-03), runner charlie, unit code_d294 (fresh carve, first attempt).

## Naming

Round 71 (alpha). `func_8001CA94` -> `New_SceneNode`, **tier A**. Allocates 0x44 bytes, runs `GetSceneNodeMethods()->ctor` on it, frees and returns NULL if the ctor fails. The project's `New_Class` allocator convention (New_Entity, New_Actor). Not a table slot. Caller: Viewport__Viewport (TaskViewport) stores the result.
