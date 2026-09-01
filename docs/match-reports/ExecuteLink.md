# ExecuteLink -- MATCHED (41/41 words)

Unit: `DreamSys`. Round 2026-09-01-e (runner echo).

## Final C

```c
bool ExecuteLink(DreamSys *system, s32 stage, s32 unk1, s32 unk2)
{
	DreamSysUnk58 *obj;

	system->unknwon_int_0x44 = unk1;
	system->vt->func_800182CC(system, unk1);
	if (system->unknwon_int_0x44 == 0)
		return false;
	system->currentStage = stage;
	if (system->isFlashbackSession) {
		system->dreamTimer = 0;
	}
	if (unk2 != 0) {
		obj = (DreamSysUnk58 *)system->unk_0x58;
		obj->vt->slot0x80(obj, 0x90, 0x6E, 0x6E);
	}
	return true;
}
```

`ExecuteLink`'s prototype (`bool ExecuteLink(DreamSys *system, s32 stage, s32
unk1, s32 unk2)`) was already declared in `include/DreamSys.h` by an earlier
round; only the body was `INCLUDE_ASM` and is filled in here.

## Derivation

The common tail call of both `func_8005A700` and `func_8005A7A0` in this
runner's range.

- `system->vt->func_800182CC(system, unk1)` -- vtable slot `+0x030`,
  resolved with `tools/classtable.py DREAMSYS_METHODS` to the inherited
  `BasicClass__func_182cc` slot (address `0x800182CC`, outside this unit and
  this runner's range). Named and added to `struct vtable_DreamSys` in
  `include/DreamSys.h`, splitting the 10-word `unknown_functions_0x18[10]`
  gap into `[6]` + this slot + `[3]`.
- **The residue that took several iterations: `currentStage`/`dreamTimer`
  assignment order.** Retail's `beqz $v0(isFlashbackSession), .L...` has
  `sw $s1, 0x164($s0)` (`currentStage = stage`) IN ITS DELAY SLOT -- which
  executes unconditionally regardless of the branch, while
  `sw $zero, 0x24($s0)` (`dreamTimer = 0`) sits AFTER the branch target and
  only runs when NOT skipped. Writing this as
  `if (isFlashbackSession) { currentStage = stage; dreamTimer = 0; }` (both
  assignments inside the guard) compiles to a real conditional store for
  `currentStage` too, costing one extra `nop` and growing the function by a
  word. The source is actually `currentStage = stage;` UNCONDITIONALLY,
  followed by `if (isFlashbackSession) dreamTimer = 0;` -- a second instance
  of DECOMPILATION_LEARNINGS.md's "default value slides into the guarding
  branch's delay slot for free" idiom, just with the "default" write being
  unconditional rather than a fallback.
- `system->unk_0x58` cast to `DreamSysUnk58 *` and called through a NEW
  slot, `+0x80` (`slot0x80`, three `s32` arguments) -- added alongside the
  already-known `slot0x84` (two arguments, `func_80059E3C`) in
  `DreamSysUnk58Vtable`. The three literal constants (`0x90`, `0x6E`,
  `0x6E`) loaded into `$a1`-`$a3` with nothing else read from `this`
  suggest a text-bank/entry trigger (same shape as `CinematicCall`), but
  nothing here confirms that beyond the argument count.

## Proposed learning

- **"Assign the value unconditionally, then conditionally clear/overwrite a
  DIFFERENT field" is a real variant of the existing "default value slides
  into the delay slot" idiom** (DECOMPILATION_LEARNINGS.md already has the
  "default, then conditionally overwritten" case for a SINGLE field). Here
  it is two DIFFERENT fields: one assignment is unconditional and rides the
  branch's delay slot for free; the second is genuinely conditional. Writing
  both inside the same `if` block over-guards the first and costs a word.
