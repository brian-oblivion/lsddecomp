# StreamTaskObj__func_8003BD10

> Renamed from `func_8003BD10` on 2026-09-23 (tools/rename.py). Address 0x8003bd10.

**Unit:** code_2c054 · **Size:** 25 words · **Status:** MATCHED (25/25)

## Summary

Straight-line forward: call the shared `TaskCoreMethods` singleton's slot
`+0x078` on `self`, then if `self->unkCC` is set, mark `self->unk38 = 2` and
call `self->methods->slot60(self, 0x12)`.

```c
void StreamTaskObj__func_8003BD10(StreamTaskObj *self) {
    Get_vtable_TaskCore()->slot78(self);
    if (self->unkCC != 0) {
        self->unk38 = 2;
        self->methods->slot60(self, 0x12);
    }
}
```

## Evidence

- `Get_vtable_TaskCore()` returns `&gTaskCoreMethods` (`TaskCoreMethods`, established
  elsewhere in `code_2c054.h`). `tools/classtable.py gTaskCoreMethods` shows slot
  `+0x078 = TaskCore__OnPadConfirm` (a different unit, not touched here — only the
  slot's existence and signature matter for this call site).
- `self->methods` is `StreamTaskObjMethods*` (`gStreamTaskObjMethods`).
  `tools/classtable.py gStreamTaskObjMethods` shows slot `+0x060 = StreamTaskObj__func_8003BC14`, which
  is this unit's own queued `StreamTaskObj__func_8003BC14` — confirms the slot exists and
  that its signature is `(StreamTaskObj *self, s32 a1)`.
- Both call results are discarded in the disassembly, so both slots are typed
  `void` here (no counter-evidence).

## Proposed learning

`tools/classtable.py <table>` cross-referenced against the OTHER known table
(`gStreamTaskObjMethods` vs `gTaskCoreMethods`) is a fast way to confirm a newly-added vtable
slot's existence and arity before writing the call: the callee occupying that
slot is often another function already queued (sometimes in the same unit,
sometimes not), and its own parameter list is direct evidence for the slot's
signature — cheaper than deriving the slot purely from the caller's register
setup.

## Naming

**StreamTaskObj__func_8003BD10** -- tier C. Occupies `gStreamTaskObjMethods`
slot `+0x078`; up-calls the base slot, then if `unkCC` is set, marks
`unk38 = 2` and re-enters this class's own state-transition slot with code
`0x12`. Neither `unk38` nor "state `0x12`" has a confirmed game meaning
(see `TaskCore__Init`'s report for the same `unk38` field from
the other side), so left `Class__func_xxxxx`.
