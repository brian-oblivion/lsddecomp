# DreamSys__DreamSys

**Unit:** DreamSys · **Size:** 63 words (0xFC bytes) · **Status:** MATCHED
(63/63 words, whole-image `./build-and-verify.sh` green)

## What it does

The class constructor, called through the method table's own `Constructor`
slot (`+0x008`, resolved via `tools/classtable.py DREAMSYS_METHODS`) by
`New_DreamSys`. Chains into the shared base class's own constructor, installs
DreamSys's own vtable, stashes the three forwarded constructor arguments,
performs a "buddy-link" step against its `arg1` companion object, and tail-
returns whatever the constructor's own last vtable step returns:

```c
DreamSys *DreamSys__DreamSys(DreamSys *this, void *arg1, s32 arg2, s32 arg3)
{
	void *val;

	GetActorMethods()->ctor(this);
	this->vt = Get_vtable_DreamSys();
	this->unk_0x58 = arg2;
	this->unk_0x5C = (DreamSysUnk5C *)arg3;
	this->unk_0x64 = 0;
	this->unk_0x60 = arg1;
	val = ((DreamSysCtorArgObj *)arg1)->methods->slot0x80(arg1, 0);
	this->vt->slot10(this, val);
	this->vt->GetSetDreamTimeLimit(this, -1);
	this->unk_0x70 = 1;
	this->unk_0x6c = 0;
	this->unk_0x878 = 1;
	this->vt->InitNewGame(this);
	return this->vt->func_800588EC(this);
}
```

Matched first attempt.

## The head's broadcast lever applied directly

This round's head broadcast (`New_Class6D3C8`, closed 24/24) was: a
constructor-family function often should NOT restate a return value with an
explicit `return X;` when the value is already sitting in `$v0` from the
immediately preceding call. This function's disassembly ends with a `jalr` to
`vt->func_800588EC(this)` and NOTHING overwrites `$v0` before the epilogue —
so `return this->vt->func_800588EC(this);` as the function's literal last
statement is *already* the non-restating form: the call's own return value
is the function's return value, with zero extra `move`. No iteration was
needed to find this; reading the disassembly's tail directly (delay slot after
the final `jalr` flows straight into the `lw $ra`/epilogue, no `move $v0,...`
anywhere) made it obvious before writing any C.

## New knowledge (all `include/DreamSys.h`)

- **`DreamSysBaseMethods` (this unit's local view of the shared `gActorMethods`
  base table) gets a new slot at `+0x008`: `ctor`.** Cross-confirmed against
  `include/code_55dd4.h`'s `D800878D4Methods`, which ALREADY names and
  resolves this exact slot as `Actor__Actor`, taking/returning
  `Class65650 *self` — the same base constructor, just viewed through a
  different subclass's local header (per this project's established
  "multiple independent local views of the same table" convention). Typed
  `struct DreamSys *(*ctor)(struct DreamSys *self)` here, matching this
  unit's existing forward-tag convention for `DreamSysBaseMethods`. Its
  return value is discarded at this call site (the base ctor returns `self`
  for chaining, unneeded since the caller already holds `this`).
- **`vtable_DreamSys+0x010` is `slot10`, shared with Class65650's OWN vtable
  at the identical offset.** `code_55dd4.h` already names and resolves it
  there as `Actor__AddChild`, and its own comment identifies it as the "link"
  companion of `slot14`/`Actor__RemoveChild` — a slot THIS unit's header already
  names (at `+0x014`, same offset relationship) with the same companion
  description, just from `DreamSys`'s side. This round's call
  (`this->vt->slot10(this, val)`) is the constructor performing that exact
  link step.
- **`DreamSysCtorArgObj`**, a new minimal opaque class for the constructor's
  own `arg1` parameter (same "vtable pointer at offset 0" shape as this
  unit's other opaque views) — its `slot0x80(self, 0)` returns a companion
  pointer forwarded straight into `vt->slot10`, matching the buddy-link shape.
- **`DreamSys::unk_0x60`** carved out of a 4-byte padding gap: set
  unconditionally to the constructor's `arg1`, `void *` typed (no further use
  in this unit's queued functions).
- **`vtable_DreamSys+0x03C` (`func_800588EC`) retyped from an untyped `void *`
  placeholder to `DreamSys *(*func_800588EC)(DreamSys *this)`** — the
  constructor's last step and this function's own return value (see above).

### Proposed learning

None beyond what the head's own broadcast already captured for
`New_Class6D3C8` — this is a second, independent, zero-attempt confirmation
of the same lever ("a constructor's tail call already leaves the return value
in the right place; do not restate it"), which is worth noting only as
reinforcement, not as a new finding.
