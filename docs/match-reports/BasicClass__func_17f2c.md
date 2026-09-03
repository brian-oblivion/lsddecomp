# BasicClass__func_17f2c

**Unit:** code_8220 · **Size:** 27 instructions · **Status:** MATCHED (27/27 words)

BasicClass vtable slot `+0x00C` — see `BasicClass__BasicClass.md` for the
class's overall design.

## What it does

The base "finalize" hook, called by `BasicClass__func_17eb0` (`release`,
slot `+0x004`) before it frees `self`. Dispatches three further virtual
calls through `self->methods`, all THIS class's own slots, in address
order:

- `+0x030` (`onFinalize`, `self, 1`) — a subclass-specific hook. The base
  occupant, `BasicClass__func_182cc`, is out of this round's carved slice
  (`code_8220_b`), so its own behaviour is unconfirmed here; only that this
  function calls it with a literal `1` second argument.
- `+0x018` (`removeAllChildren`, `self`) — this unit's own
  `BasicClass__func_18040` (own report): walks and detaches every entry in
  `children`.
- `+0x028` (`clearParentRefs`, `self`) — this unit's own
  `BasicClass__func_1813c` (still `INCLUDE_ASM` this round, but its
  extern prototype and behaviour are already established from its own
  disassembly): frees every node in `parentRefs` and nulls the list head.

## The C

```c
void BasicClass__func_17f2c(BasicClass *self)
{
    self->methods->onFinalize(self, 1);
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
