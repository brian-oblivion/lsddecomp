# New_Class6B5CC

> Renamed from `func_8001CA94` on 2026-09-23 (tools/rename.py). Address 0x8001ca94.

**Unit:** code_d294 · **Size:** 24 words · **Status:** MATCHED (24/24 words)

## What it does

`New_Class6B5CC`: the allocator wrapper for the unnamed class whose method
table is `gClass6B5CCMethods` (`tools/classtable.py gClass6B5CCMethods`). Allocates 0x44
bytes, calls the class's own constructor slot (found through the
no-argument getter `GetClass6B5CCMethods`, which just returns `&gClass6B5CCMethods`) on
the fresh block, and on constructor failure frees the block and returns
`NULL`. This is NOT itself a vtable slot — it is the standalone `New_X`
entry point that callers elsewhere in the game presumably use to spawn an
instance.

## The C

```c
Class6B5CCObj *New_Class6B5CC(void) {
    Class6B5CCObj *obj;

    obj = BMemPMgrAlloc(0x44);
    if (obj == NULL) {
        return NULL;
    }
    if (GetClass6B5CCMethods()->ctor(obj) != NULL) {
        return obj;
    }
    BMemPMgrFree(obj);
    return NULL;
}
```

Same shape as `src/Entity.c`'s `New_Entity`.

## Establishing the class

`GetClass6B5CCMethods()` is a plain no-argument getter (`lui/addiu %hi/%lo
(gClass6B5CCMethods); jr $ra`, MEASURED from its own disassembly in
`asm/code_d294_b.s`, the next uncarved slice). Its whole-file `->ctor`
call here checked the return with `bnez`, and that observation, combined
with reading `Class6B5CC__Class6B5CC` (this class's ctor, `docs/match-
reports/Class6B5CC__Class6B5CC.md`) confirmed `Class6B5CC__Class6B5CC` really does return
`self` on success and `NULL` on failure — the two reports cross-check
each other.

See `include/code_d294.h`'s file banner for the full `classtable.py`
census this unit's header is built from.

## Provenance

round 11 (2026-09-03), runner charlie, unit code_d294 (fresh carve, first attempt).

## Naming

Round 71 (alpha). `func_8001CA94` -> `New_Class6B5CC`, **tier A**. Allocates 0x44 bytes, runs `GetClass6B5CCMethods()->ctor` on it, frees and returns NULL if the ctor fails. The project's `New_Class` allocator convention (New_Entity, New_BaseObjO). Not a table slot. Caller: Unk18Obj__Unk18Obj (code_2cc8c_c) stores the result.
