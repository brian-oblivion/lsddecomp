# func_8005A9CC

**Unit:** DreamSys · **Size:** 88 words · **Status:** STALL, best-reached
57/88, no address drift. Attempted round 2026-09-06 (charlie). Screened
clean against both remaining blockers (`gp_rel`, `nop_mflo_mfhi`) by the
head before assignment.

## What it does

Called through `vtable_DreamSys::func_8005A9CC` (+0x1DC) by `func_80059E98`
(matched this round, see its own report) as the first of three "link test"
tries. Signature `bool (DreamSys *this, PlayerSpawnPoint *currentPos)`,
matching its siblings `func_8005A700`/`func_8005A7A0`.

```c
bool func_8005A9CC(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	s32 result;
	s32 local[4];

	if (this->unknwon_int_0x44 != 0) {
		return false;
	}

	if (this->unk_0x910 == 0) {
		goto staircase;
	}
	if (!this->unk_0x910(this)) {
		return false;
	}
	this->unk_0x908 = 0;
	this->unk_0x910 = 0;
	this->unk_0x90C = 0;
	if (this->unk_0xAC != 4) {
		return false;
	}
	this->vt->func_8005A1A4(this);
	return false;

staircase:
	result = Test4StaircaseNodes(&this->linkCoordinates, currentPos, this->currentStage);
	if (result < 0) {
		return false;
	}
	func_8001E6F8(this, local);
	if (!func_8005C02C(&this->unk_0x888, &this->unk_0x884, local)) {
		return false;
	}
	if (this->unk_0xA8 == 0) {
		return false;
	}

	this->unk_0x918 = *(PlayerSpawnGridPos *)currentPos;
	this->unk_0x91C = currentPos->position;
	this->unk_0x908 = 1;
	this->unk_0x90C = 1;
	this->unk_0x914 = 0;
	this->unk_0x910 = D_80087EEC[func_8005C118()];
	this->vt->func_8001CEB4(this, 1, (void *)this->unk_0x884);
	this->unk_0x910(this);
	return false;
}
```

**Every path returns `false`** -- confirmed against the disassembly, not
assumed: every `j`/fallthrough in the function lands on either an explicit
`li v0,0` or the shared epilogue label that itself does `li v0,0`. There is
no path that reaches the epilogue with a nonzero `$v0`. This makes the
function's practical behavior in its only caller (`func_80059E98`'s
`!this->vt->func_8005A9CC(...) && ...` chain) equivalent to always
continuing to the next link test -- a real quirk of retail's own logic, not
a decompilation error.

## New struct/vtable knowledge committed alongside this round

All in `include/DreamSys.h`:

- **`DreamSys::unk_0x910` retyped `s32` -> `s32 (*)(struct DreamSys *this)`**
  (a function pointer, called through directly, `0` used as its "unset"
  sentinel -- both existing writers set it to literal `0`, safe). Note the
  `struct DreamSys *` spelling: `DreamSys *` cannot be used inside the
  `DreamSys` struct's own definition before its typedef completes (same
  reason `DreamSysBaseMethods::ctor` uses the tag form -- see that field's
  own comment).
- **`DreamSys::unknown_values_0x918[4]` retyped to a new `PlayerSpawnGridPos`
  struct** (`{struct MapChunk chunk; struct MapTile tile;}`, 4 bytes) so a
  single whole-struct assignment reproduces retail's unaligned
  `lwl`/`lwr`+`swl`/`swr` 4-byte copy of `currentPos`'s first two members --
  same idiom as the existing `unk_0x91C` (`struct RelativePos`) next to it,
  which already covers `currentPos->position`. No other reader of this
  field existed before this round.
- **`extern s32 (*D_80087EEC[4])(DreamSys *this)`** -- a table of the four
  already-matched `s32 (DreamSys *this)` functions `func_8005AB2C`/
  `func_8005AC24`/`func_8005AD68`/`func_8005AE40`, confirmed by their own
  existing definitions in `src/DreamSys.c`.
- **`extern s32 Test4StaircaseNodes(...)`** and **`extern s32
  func_8005C02C(...)`** forward/call-site prototypes added near the
  existing `func_8005BD3C` one (same 3-arg shape; `func_8005C02C` is
  blocked by the same gp-relative+addiu_at pair as `func_8005BD3C`, per its
  own existing stub report). `Test4StaircaseNodes` is defined later in this
  same unit's ROM order, so its prototype here is a plain forward
  declaration, not a cross-unit one.
- **`extern s32 func_8005C118(void)`** -- corrected from a guessed
  `(DreamSys *this)` signature: the disassembly's call site leaves `$a0`
  holding an unrelated leftover value (`currentPos->position.z`, from the
  immediately preceding `lh`) with no explicit argument setup, matching the
  existing "empty delay slot, no a0-a3 setup" shape already documented for
  `func_8005BF48`.

## The residue, precisely

**Zero address drift; the entire body other than one spot is
instruction-for-instruction identical to retail**, including the exact
`lwl`/`lwr`/`swl`/`swr` shape for both the `PlayerSpawnGridPos` copy and the
`RelativePos` copy, and the entire "unk_0x910 call vs staircase" branch
structure and every literal constant. The one divergence:

Retail's `beqz $v0,.L8005AA44` (the `this->unk_0x910 == 0` test) has a
`nop` in its delay slot, and the subsequent `jalr $v0` (calling
`this->unk_0x910(this)`) ALSO has a `nop` in ITS delay slot -- i.e. no
`move $a0,$s0` before that call at all, because `$a0` still holds the
original `this` argument unclobbered since function entry (nothing between
the top of the function and this call touches `$a0`), so the call is
correct without an explicit re-move. My compiled version instead HOISTS
`.L8005AA44`'s first real instruction (`addiu $a0,$s0,0x16c`, computing
`&this->linkCoordinates` for the staircase branch) into the first `nop`
slot, which clobbers `$a0`, forcing a real `move $a0,$s0` into the second
`nop` slot to restore it before the `this->unk_0x910(this)` call. Both
versions have the same total instruction count in this stretch (2 real
instructions filling 2 delay slots either way), so nothing drifts, but the
`addiu` ends up ONE INSTRUCTION EARLIER than retail, shifting everything
between it and the next matching landmark by exactly one word. The
alignment recovers by itself once both streams reach the shared "return
false" epilogue, but not before the `PlayerSpawnGridPos`/`RelativePos` copy
section, which then shows a genuine (if minor) register choice difference
riding on top of the shift (`$v1` vs `$v0` holding the second 4-byte chunk
of the copy) that I did not get to isolate from the shift itself.

**What was tried, all rebuilt and measured:**

- `if`/`else` vs `goto`-to-a-later-label for the exact same branch (both
  spellings of the identical CFG) -- **byte-identical**, confirming the
  hoist is a scheduling choice independent of how the branch is spelled in
  C.
- A bare `__asm__("")` scheduling barrier at three positions: the very top
  of the function, immediately before the `this->unk_0x910 == 0` test, and
  as the first statement of the `staircase:` block. The first and third
  placements caused real drift (136065 bytes off a clean whole-image
  rebuild) -- the barrier is not free of side effects on a body this size,
  it suppresses other optimizations too, not just the one hoist. The
  second placement (immediately before the branch) produced **zero
  observable effect at all** -- the compiler optimized the empty asm
  statement away without changing scheduling, since nothing there
  conflicts with it. None of the three placements isolated the fix CLAUDE.md's
  test requires (removing it should change only ORDER, not identity) --
  here it either changed nothing or changed far more than intended.

**Reading it**: this is the CLAUDE.md/DECOMPILATION_LEARNINGS residue class
"A `nop` retail has and you do not... delay slot filling differs when the
source statement order differs" -- except here source statement order was
tried and does NOT move it, which is itself worth recording: not every
delay-slot-fill difference is reachable by reordering the C, and a
same-instruction-count reordering across a forward branch target is a
plausible case where it never will be, since the compiler's decision to
hoist appears to depend on something below the C source's visibility (which
optimization pass runs, or global scheduling state) rather than on
statement order.

### Proposed learning

Add a documented NEGATIVE for the `__asm__("")` scheduling-barrier lever:
placing it at a branch or label boundary to suppress a specific delay-slot
hoist is not reliable -- it can be silently optimized away with no effect
(if nothing local conflicts with it) or can suppress unrelated scheduling
elsewhere in the same function, producing drift far larger than the single
instruction it was aimed at. The lever's only confirmed-safe use so far
remains its original one: as the function's very first statement, fixing
PROLOGUE callee-save STORE ORDER specifically (see
`DECOMPILATION_LEARNINGS.md`'s existing entry) -- not general delay-slot
hoisting later in a function's body.
