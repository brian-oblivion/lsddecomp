# DreamSys__DispatchChunkChange

> Renamed from `func_80058E8C` on 2026-09-22 (tools/rename.py). Address 0x80058e8c.

**Unit:** DreamSys · **Size:** 35 instructions · **Status:** MATCHED (35/35 words)

## What it does

Vtable `+0x09C` (this function's own slot). Unconditionally calls a shared
base-class method (`DreamSys__GetBaseMethods()->slot0x9C(this, arg1, arg2)`), then, if
`arg1`'s own class-id header word (masked to the low 12 bits) equals
`0x114`, ALSO calls `this->vt->ProcessChunkChange(this, arg1, arg2)`
(matched earlier this round, same unit).

## The C

```c
void DreamSys__DispatchChunkChange(DreamSys *this, void *arg1, s32 arg2)
{
	DreamSys__GetBaseMethods()->slot0x9C(this, arg1, arg2);
	if ((*(s32 *)(*(void **)arg1) & 0xFFF) == 0x114) {
		this->vt->ProcessChunkChange(this, arg1, arg2);
	}
}
```

```c
typedef struct DreamSysBaseMethods {
	u8 pad00[0x50];
	void (*slot0x50)(struct DreamSys *self);
	u8 pad54[0x9C - 0x54];
	void (*slot0x9C)(struct DreamSys *self, void *arg1, s32 arg2);
} DreamSysBaseMethods;
```

## The class-id header-word check, confirmed from real code for the first time

`DECOMPILATION_LEARNINGS.md`'s open-questions section has asked since round
2026-08-29 what the class-table header word at `+0x000` is ("not a pointer
... a class id, a flags word, or an instance size"). This function is the
first CODE (not just data inspection) this project has matched that reads
it: `*(*(void **)arg1)` dereferences `arg1`'s own vtable pointer, then reads
THAT table's header word, masks it to `0xFFF`, and compares against the
literal `0x114`. This is consistent with "class id, low bits" -- it does
NOT settle the open question fully (still don't know what the other bits
mean, or whether `0x114` maps to a name), but it is now proven that game
code actually reads and branches on this field, not just splat/tooling
inference from the data side.

### Proposed learning

Promote to `DECOMPILATION_LEARNINGS.md`'s open-questions entry on the
class-table header word: `DreamSys__DispatchChunkChange` (`DreamSys`, vtable `+0x09C`)
reads `*(*(void**)obj) & 0xFFF` and compares against a literal class id
(`0x114` here) to decide whether to dispatch a second, class-specific call.
This is the first confirmed in-game READ of that field, and the pattern
(`header & 0xFFF == literal`) is worth grepping for elsewhere before
assuming this project's only lead on the header word is the data-side
census.

## Provenance

round 2026-08-30-d, runner ALPHA, unit DreamSys (whole-unit, third pass).

## Naming

- **Tier B.** Calls the base class's generic +0x9C notify slot unconditionally, then forwards to the already-named DreamSys__ProcessChunkChange only when the notified object's masked type tag equals 0x114. Same shape as DreamSys__DispatchInstanceEffect below.
