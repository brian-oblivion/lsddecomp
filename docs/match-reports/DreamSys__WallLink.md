# DreamSys__WallLink

**Unit:** DreamSys · **Size:** 77 words (0x134 bytes) · **Status:** MATCHED
(77/77 words, whole-image `./build-and-verify.sh` green)

## What it does

`DREAMSYS_METHODS` slot `+0x0E0` (this function's own slot, confirmed via
`tools/classtable.py`, and previously an untyped `void *LinkWall` placeholder
in the header). Forwards through a base-class hook; if the link type is `4`
and no link is already in progress, snapshots a spawn position into
`this->linkCoordinates` and tests/executes a static or dynamic wall link;
always finishes by restoring `*this->unk_0x14` and firing a no-op stub:

```c
void DreamSys__WallLink(DreamSys *this, void* unk_class_86aa0, int arg2)
{
	DreamSys__GetBaseMethods()->slot0xE0(this, unk_class_86aa0, arg2);
	if (arg2 != 4)
		return;
	if (this->unknwon_int_0x44 != 0)
		return;
	this->linkCoordinates = *this->unk_0x4C->methods->slot0xD4(this->unk_0x4C, unk_class_86aa0);
	if (!this->vt->StaticWallLink(this, &this->linkCoordinates) && this->unk_0x124 != 0) {
		this->vt->DynamicLink(this);
	}
	this->vt->DreamSys__RestoreLinkSnapshot(this);
	this->vt->DreamSys__NoOpSlotE8Default(this);
}
```

## The one residue: two early exits must skip the WHOLE function, not just one block

First attempt wrote the two guards as a combined `if (arg2 == 4 &&
this->unknwon_int_0x44 == 0) { ...link-handling... }`, followed
unconditionally by the two tail calls (`DreamSys__RestoreLinkSnapshot`, `DreamSys__NoOpSlotE8Default`).
That reached 75/77 with both remaining diffs being the SAME two early
branches' TARGET address: retail sends both `arg2 != 4` and `unknwon_int_0x44
!= 0` straight to the function's own epilogue, skipping the two tail calls
entirely — not just skipping the link-handling block. Rewriting both guards
as unconditional early `return;` statements (matching retail's actual reach)
closed both words immediately; everything else, including the whole-struct
assignment through the unaligned `lwl`/`lwr`/`lh` triplet and the
`StaticWallLink`/`DynamicLink` dispatch, was already byte-exact on the first
attempt.

## New knowledge

- **`vtable_DreamSys+0x0E0` (`LinkWall`) is this function's own slot**,
  retyped from the untyped placeholder to
  `void (*LinkWall)(DreamSys *this, void *arg1, s32 arg2)`.
- **`vtable_DreamSys+0x0E8` carved out of `unknown_functions_0xe4[6]` and
  named `DreamSys__NoOpSlotE8Default`** — this unit's own existing no-op stub
  (`void DreamSys__NoOpSlotE8Default(void) { }`). Called here as `(this)`; the callee
  ignores the argument entirely (an "empty-bodied vtable occupant" that DOES
  take a parameter per its slot's calling convention, matching the already-
  documented caution that an empty body is not evidence a slot takes no
  arguments).
- **`DreamSysBaseMethods` gets a new slot at `+0x0E0`**, same
  `(self, arg1, arg2)` shape as the already-known `slot0x9C`/`slot0xDC`
  neighbors. Called as `(this, unk_class_86aa0, arg2)` — this function's OWN
  parameters forwarded verbatim.
- **`DreamSysUnk4CMethods` gets a new slot at `+0x0D4`**, returning
  `PlayerSpawnPoint *`. Its result is whole-struct-assigned into
  `this->linkCoordinates` via the compiler's own unaligned
  `lwl`/`lwr`(x2)/`lh` then `swl`/`swr`(x2)/`sh` sequence — no manual
  reconstruction needed, a plain `*a = *b;` on the two `PlayerSpawnPoint`
  values reproduces it exactly.

### Proposed learning

**When an early-exit guard's failure branch target is the function's OWN
epilogue rather than a nearby merge point, write it as an unconditional
`return;`, not as a condition wrapping the rest of the function.** A combined
`if (A && B) { rest-of-function }` and two separate `if (!A) return; if (!B)
return;` are NOT interchangeable here even though they are logically
identical — the FIRST reaches only as far as skipping `{ rest-of-function }`,
while the SECOND (matching retail) skips everything after it including any
unconditional tail calls that would otherwise still run. This is a specific,
checkable instance of the broader "branch TARGETS disagree, not just delay
slots" discriminator already in `DECOMPILATION_LEARNINGS.md` — the tell here
was that the CORRECT logic (guard the link block) still executed the
unconditional tail, whereas retail's actual behavior was "skip absolutely
everything on early exit," a difference invisible from reading the guarded
block alone and only visible by checking where the failing branch actually
lands.
