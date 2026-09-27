# BasicClass__Finalize

> Renamed from `BasicClass__func_17f2c` on 2026-09-24 (tools/rename.py). Address 0x80017f2c.

**Unit:** code_8220 · **Size:** 27 instructions · **Status:** MATCHED (27/27 words)

BasicClass vtable slot `+0x00C` — see `BasicClass__BasicClass.md` for the
class's overall design.

## What it does

The base "finalize" hook, called by `BasicClass__Release` (`release`,
slot `+0x004`) before it frees `self`. Dispatches three further virtual
calls through `self->methods`, all THIS class's own slots, in address
order:

- `+0x030` (`notifyParents`, `self, 1`) — a subclass-specific hook. The base
  occupant, `BasicClass__NotifyParents`, is out of this round's carved slice
  (`TmdRenderer`), so its own behaviour is unconfirmed here; only that this
  function calls it with a literal `1` second argument.
- `+0x018` (`removeAllChildren`, `self`) — this unit's own
  `BasicClass__RemoveAllChildren` (own report): walks and detaches every entry in
  `children`.
- `+0x028` (`clearParentRefs`, `self`) — this unit's own
  `BasicClass__ClearParentRefs` (still `INCLUDE_ASM` this round, but its
  extern prototype and behaviour are already established from its own
  disassembly): frees every node in `parentRefs` and nulls the list head.

## The C

```c
void BasicClass__Finalize(BasicClass *self)
{
    self->methods->notifyParents(self, 1);
    self->methods->removeAllChildren(self);
    self->methods->clearParentRefs(self);
}
```

## Matched first attempt — the interesting negative result is what DIDN'T
## need doing

Each of the three calls re-reads `self->methods` from scratch
(`lw $v0,0x0($s0)` appears three separate times in retail, once per call),
rather than caching it once into a register and reusing it across the three
`jalr`s. Writing the natural, un-cached C above reproduced this exactly —
GCC 2.6.3 does NOT hoist/cache `self->methods` across an intervening
INDIRECT call here, consistent with DECOMPILATION_LEARNINGS' "a repeated
read of an unchanging pointer field is not reliably CSE'd... GCC 2.6.3
reloads it" (there, the discriminator was an intervening whole-struct
assignment; here it's an intervening unknown indirect call — same
underlying reason, the compiler can't prove the callee didn't write back
through `self`). No caching was attempted or needed; recorded here so a
future function facing the SAME shape doesn't spend an attempt manually
caching `self->methods` first and then have to undo it.

## Provenance

round 11 (2026-09-03), runner delta, unit code_8220 (fresh carve).

## Naming (round 74)

`BasicClass__Finalize`, **tier A**: matches vtable slot `+0x00C`
(`finalize`), already documented in `code_8220.h`'s `BasicClassMethods`
comment. Called by `BasicClass__Release` before freeing `self`.

## Polish (round 97, runner delta)

`notifyParents(self, 1)` -> `BASICCLASS_EVENT_FINALIZED`, a new enum in
`include/BasicClass.h`. Evidence: the base `BasicClass__OnNotify`
(`TmdRenderer.c`) acts on event 1 and only 1, by removing the sender from its
children, and this function, the base finalizer, is its sender. Byte-identical.
