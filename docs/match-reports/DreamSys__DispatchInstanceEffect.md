# DreamSys__DispatchInstanceEffect

> Renamed from `func_80058F18` on 2026-09-22 (tools/rename.py). Address 0x80058f18.

**Unit:** DreamSys · **Size:** 37 words · **Status:** MATCHED (37/37 words, full build verified byte-exact)
**Vtable slot:** `gDreamSysMethods +0xDC` (this function's own slot)

## Context

Structurally identical to the already-matched `DreamSys__DispatchChunkChange` (vtable
`+0x9C`, see `docs/match-reports/DreamSys__DispatchChunkChange.md`): unconditionally calls a
shared base-class method through `GetActorMethods()`'s returned table, then
conditionally dispatches a second, class-specific call gated on `arg1`'s own
class-id header word. The only differences from `DreamSys__DispatchChunkChange` are the two
offsets involved and the class-id constant being compared.

`GetActorMethods()->slot0xDC` resolves (via `tools/classtable.py gActorMethods`)
to `Actor__OnActorLinkCommand`, out of this unit's scope. `this->vt->slot0x1E8` resolves
(via `tools/classtable.py gDreamSysMethods`) to
`DreamSys__InstanceEffectsOnJournal` -- already forward-declared in
`include/DreamSys.h` and already has a real body pending as
`DreamSys__InstanceEffectsOnJournal` (`INCLUDE_ASM` elsewhere in this file).

The vtable's own field name at `+0x1E8` was a stale placeholder,
`InstanceEffectsOnPlayer` -- it never matched the actual symbol
(`DreamSys__InstanceEffectsOnJournal`, confirmed by `tools/classtable.py`
and already used as the `INCLUDE_ASM` name in `src/world/DreamSys.c`). Corrected
in this round; no call site referenced the old name, so this is a plain
fix, not a rename requiring an out-of-scope edit.

## The C

```c
void DreamSys__DispatchInstanceEffect(DreamSys *this, void *arg1, s32 arg2)
{
	GetActorMethods()->slot0xDC(this, arg1, arg2);
	if ((*(s32 *)(*(void **)arg1) & 0xFFFFF) == 0x1F234) {
		this->vt->InstanceEffectsOnJournal(this, arg1, arg2);
	}
}
```

`DreamSysBaseMethods` gained a new slot:

```c
typedef struct DreamSysBaseMethods {
	u8 pad00[0x50];
	void (*slot0x50)(struct DreamSys *self);
	u8 pad54[0x9C - 0x54];
	void (*slot0x9C)(struct DreamSys *self, void *arg1, s32 arg2);
	u8 padA0[0xDC - 0xA0];
	void (*slot0xDC)(struct DreamSys *self, void *arg1, s32 arg2);
} DreamSysBaseMethods;
```

## Class-id header-word check: a SECOND confirmed in-game read, with a wider mask

`DreamSys__DispatchChunkChange` was the first confirmed in-game read of the class-table
header word (`& 0xFFF == 0x114`). This function reads the SAME shape
(`*(*(void**)arg1) & MASK == CONSTANT`) but with `MASK = 0xFFFFF` (20 bits,
not 12) and `CONSTANT = 0x1F234`. `tools/classtable.py --scan` confirms
`gEntityMethods` (96 slots) has header word exactly `0x1F234` -- so this function
is checking "is `arg1` an instance of `gEntityMethods`'s class" before
dispatching `InstanceEffectsOnJournal`, consistent with the vtable slot's
name (`gEntityMethods` is plausibly the Journal entity's class).

This is worth promoting: the previous open question assumed "low 12 bits"
from a single data point. A second data point using 20 bits, on a header
value that doesn't fit in 12 bits (`0x1F234 > 0xFFF`), shows the field is
wider than 12 bits in at least some checks -- the boundary is still
unconfirmed, but "12 bits" is now known to be too narrow as a general rule.

### Proposed learning

Promote to `DECOMPILATION_LEARNINGS.md`'s class-table header-word open
question: a second in-game read (`DreamSys__DispatchInstanceEffect`, `DreamSys`, vtable
`+0xDC`) masks `*(*(void**)obj)` with `0xFFFFF` (not `0xFFF`) and compares
against `0x1F234` -- a literal that does not fit in 12 bits and matches
`gEntityMethods`'s header word exactly (`tools/classtable.py --scan`). The
class-id field is confirmed to occupy more than 12 bits in at least this
check; the previous "low 12 bits" framing from `DreamSys__DispatchChunkChange` should be
read as "this function happened to use a 12-bit mask", not as the field's
true width.

## Attempt log

Matched on the first attempt: the shape was already proven by
`DreamSys__DispatchChunkChange` in the same unit, and `tools/classtable.py` resolved both
the base-class slot (`+0xDC` -> `Actor__OnActorLinkCommand`) and this class's own slot
(`+0x1E8` -> `DreamSys__InstanceEffectsOnJournal`) directly.

## Provenance

round 2026-09-02, runner ALPHA, unit DreamSys.

## Naming

- **Tier B.** Calls the base class's generic +0xDC notify slot unconditionally, then forwards to the already-named DreamSys__InstanceEffectsOnJournal only when the notified object's masked type tag equals 0x1F234.
