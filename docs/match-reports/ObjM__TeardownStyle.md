# ObjM__TeardownStyle

> Renamed from `func_80053134` on 2026-09-24 (tools/rename.py). Address 0x80053134.

**Unit:** dream_scene · **Size:** 27 words (0x6C bytes) ·
**Status: MATCHED 27/27**, whole-image SHA1 green.

## What it does

```c
void ObjM__TeardownStyle(Obj87034_3bb8c_l *self) {
    self->methods->slot84(self);
    ReleaseDreamAuxEntities();
    StyleTeardown();
    self->unk54->methods->slot48(self->unk54);
}
```

Straight-line dispatch chain. `ReleaseDreamAuxEntities` is already matched
elsewhere (`src/world/dream_aux.c`); `StyleTeardown` is still uncarved ground
(`asm/dream_scene.s`). Both are called with no arguments and their
return values are unused, declared as plain `extern void func(void);` in
`include/class_3bb8c.h`.

## Notes

- `self->unk54` dispatches its OWN `+0x48` slot — the same numeric offset
  self dispatches directly in `ObjM__DetachTarget`, reused here on a sibling
  object of the presumed-same class (`Obj87034_3bb8c_l *unk54`).
- Diffed clean immediately after `ObjM__PollTimBlockLoad` and `ObjM__DispatchPadEvent` were
  fixed — this function's own two `jal` targets (`ReleaseDreamAuxEntities`,
  `StyleTeardown`) initially resolved to addresses 0xC bytes past retail's,
  purely because OTHER not-yet-fixed functions in this unit were still the
  wrong size and shifting every later cross-unit symbol. Not a bug in this
  function; a reminder that `funcdiff`'s "differs outside this range" warning
  means exactly what it says — don't trust an individual function's `jal`
  target mismatch while siblings in the same unit are still wrong.

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_80053134` | `ObjM__TeardownStyle` | B | see below |

**Evidence.** vtable slot +0x050. Calls the global `StyleTeardown()` and `ReleaseDreamAuxEntities()` directly, then notifies `self->unk54` (another `ObjM` instance) via its own `slot48`. Named for the one global call whose own name is already established.


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the dream_scene/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and dream_day.h's Obj4C/SubObjB/EventArg are gone. Byte-identical.
