# ExecuteLink

**Unit:** DreamSys · **Size:** 41 words · **Status:** MATCHED (41/41)

## What it does

Already forward-declared (`bool ExecuteLink(DreamSys *system, s32 stage,
s32 unk1, s32 unk2);`) and called from four already-matched functions
(`DreamSys__DynamicLink`, `DreamSys__StaticWallLink`, `DreamSys__TryTunnelLink`).
This is the body: records `unk1` into `unknwon_int_0x44`, calls a
base-class hook, bails if that hook cleared the flag, otherwise commits the
new stage and (conditionally) resets the dream timer and fires a secondary
notification.

## The C

```c
bool ExecuteLink(DreamSys *system, s32 stage, s32 unk1, s32 unk2)
{
	DreamSysUnk58 *obj;

	system->unknwon_int_0x44 = unk1;
	system->vt->slot30(system, unk1);
	if (system->unknwon_int_0x44 == 0) {
		return false;
	}
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

## Field identification (all via manual offset accounting, not guesswork)

`unknwon_int_0x44` was already named at exactly this offset. The other
three raw offsets this function touches (`0x68`, `0x24`, `0x164`) were NOT
obviously anything from their surrounding comments, so I hand-summed
`include/dream_sys.h`'s `DreamSys` struct field-by-field from the top
(every pointer field counted as 4 bytes, matching this project's `-m32`
verification convention from `DECOMPILATION_LEARNINGS.md` — a real `-m32`
host build wasn't available in this environment, `gnu/stubs-32.h` missing,
so this was done by direct arithmetic on the struct instead):

- `+0x24` → `dreamTimer` (falls right after `unk_0x14` + its 12-byte pad).
- `+0x68` → `isFlashbackSession`. This one is easy to miss: `bool` in this
  codebase is `typedef int bool;` (`include/types.h`), so it is a full
  32-bit field, not a single byte — the `lw`/`beqz` word-test in the
  disassembly is consistent with that, not with a byte-sized flag.
- `+0x164` → `currentStage` (after `areaMoods`/`entityMoods`, two
  0x10-byte `MoodGraphContributor`s, landing exactly on `currentStage`).

`unk_0x58` stays `s32` in the struct itself (matches the established
dual-typed convention from `DreamSys__StopVoice` — cast locally to
`DreamSysUnk58 *` rather than retyping the field project-wide, since other
call sites in this unit still use it as a plain integer).

## New vtable slots

- `vtable_DreamSys` gained `slot30` (`+0x030`), resolved via
  `tools/classtable.py gDreamSysMethods` to `BasicClass__NotifyParents` — the
  same shared base-class slot `dream_day.h`/`GameApplication.h` already name
  `slot30` with an identical `(self, s32 arg1)` signature. Return
  discarded here too.
- `DreamSysUnk58Vtable` gained `slot0x80` (three `s32` args, all literal
  constants — `0x90`, `0x6E`, `0x6E` — at this one call site; nothing here
  suggests their meaning).

## Residue and fix

First attempt wrote `system->currentStage = stage;` in natural trailing
position (after the `isFlashbackSession` guard, before the `unk2` guard) —
38/41, with the `sw $s1, 0x164($s0)` instruction landing one delay slot
later than retail. Retail schedules that store into the delay slot of the
EARLIER branch (`beqz v0, ...` testing `isFlashbackSession`), since it does
not depend on that branch's outcome — the standard "unconditional value
computed early, for free, in a branch's delay slot" idiom already
documented in `DECOMPILATION_LEARNINGS.md`. Moving the assignment earlier
in the C source, immediately after the early-return guard and before the
`isFlashbackSession` check, reproduced the exact scheduling. This is a
source-order fix, not a reshape — the statement's dependencies didn't
change, only where GCC saw it relative to the branch it could hide behind.

## Third-learning check (per head's request)

**Not needed.** Every field read happens once, with the only cross-call
survivor being the already-known `unk1` value held in `$a1`/`this` across
the `slot30` call — and that's a live PARAMETER value re-read from the
just-stored field (`system->unknwon_int_0x44`), not a value threaded
through a local. GCC reloaded it from memory on its own with no local
needed, matching retail's own `lw $v0, 0x44($s0)` reload after the call.
No `jalr`-aliasing lever applied here.

## Proposed learning

None beyond re-confirming the existing "default value computed early, in
whichever branch's delay slot needs it" idiom — worth noting it applies to
a store into `*self` just as much as to a scalar return value.

## Track 4 (2026-09-26, round 87, VabStreamObj)

`include/dream_sys.h`'s `DreamSysUnk58`/`DreamSysUnk58Vtable` view is deleted.
`DreamSys::soundObj` is cast to `VabStreamObj *` (`include/VabStreamObj.h`),
and the slots are called by the class's names: `slot0x80` -> `playTone`
(`VabStreamObj__PlayTone`: index = program << 4 | tone, then vol and
endVol; it returns the voice), `slot0x84` -> `stopVoice`, and `slot0x9C` ->
`setPitchOffset` (its argument is an octave: pitchOffset = octave * 12 - 24).
The view had typed stopVoice `void`, but its occupant returns s32 (always
-1). The whole image stays byte-identical with the s32 slot, because the one
call site discards the value. `soundObj` itself stays `s32`: it is
DreamSys's field.
