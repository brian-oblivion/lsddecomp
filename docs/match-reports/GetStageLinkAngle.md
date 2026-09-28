# GetStageLinkAngle — MATCHED 8/8

> Renamed from `func_8005BF48` on 2026-09-22 (tools/rename.py). Address 0x8005bf48.

**Unit:** DreamSys · **Size:** 8 words · **Status:** MATCHED, round 43.

## History

Filed round 2026-08-30-a as BLOCKED on `gp_rel` (sole reference `gLinkDstStage`
via `%gp_rel`). Round 42 RESOLVED that blocker. Rebuilt fresh this round.

## What it does

`if (gLinkDstStage == 0xC) return 0; else return (s32)&LINK_ANGLE_180;` -- stored
whole into `this->unk_0x880` by `DreamSys__TryStageTimerLink` right before an ExecuteLink
(per the existing header comment on that field and on the prototype
`extern s32 GetStageLinkAngle(void);`).

## First attempt: two `return` statements, WRONG registers (1/8)

```c
s32 GetStageLinkAngle(void)
{
	if (gLinkDstStage == 0xC)
		return 0;
	return (s32)&LINK_ANGLE_180;
}
```

This compiles to the right VALUES but the wrong REGISTER PLAN: GCC put the
loaded global in `$v0` and the constant `0xC` in `$v1`, where retail has them
swapped (`$v1`/`$v0`), and retail also computes the "0" result unconditionally
in a branch delay slot (`addu $a0,$zero,$zero`) rather than via two `return`
paths. 1/8 words matched.

## Final body: single result variable, matching retail's own idiom (8/8)

```c
s32 GetStageLinkAngle(void)
{
	s32 result;

	result = 0;
	if (gLinkDstStage != 0xC)
		result = (s32)&LINK_ANGLE_180;
	return result;
}
```

Writing it as "default to 0, override on the negated condition" instead of
two early returns reproduced retail's exact register choice and its
delay-slot placement of the zero-initialization. Byte-exact on the second
attempt.

`LINK_ANGLE_180` (extern `s32`, single `.word` at `asm/data/7B3C0.sdata.s`) is
only ever address-taken, matching the existing pattern used by
`SPECIAL_DAY_MOOD`/`IsDaySpecial` a few hundred lines below in this same unit.

## Verification

`./build-and-verify.sh` -> `build exit=0`, whole-image SHA1 matches retail.
`tools/funcdiff.py GetStageLinkAngle` -> `8/8 words match`.

### Proposed learning

A "return early with a literal, else return the other value" C shape is not
guaranteed to reproduce retail's register plan even when the VALUES are
byte-identical between build variants -- when retail's own asm shows an
unconditional delay-slot default (`addu $a0,$zero,$zero` before either branch
outcome) followed by a conditional override, write it as a single
default-then-override local variable, not as two `return` statements. Same
family as the already-documented commutative-add operand-order class, but for
early-return shape rather than operand order.

## Provenance

round 43, runner ALPHA, unit DreamSys.

## Naming

`GetStageLinkAngle` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_8005BF48`.

A free function (no `this`, and the disassembly's call site sets
up no arguments): returns `&LINK_ANGLE_180` unless `gLinkDstStage` -- the
destination stage `GetStaticSpawn` and `Test4StageTransition` record -- is 0xC, in
which case 0. `DreamSys__TryStageTimerLink` stores the result in
`stageLinkAngle`, in the same statement group that zeroes `enterRotation` and
`exitRotation`.
"Angle" is read off the constant: `LINK_ANGLE_180`'s single word is 0x000100B4, a
{numerator 0xB4, denominator 1} degree ratio -- byte-identical to the Y word of
`sRotationYaw180`, and the same encoding as every `sCardinalAngles` entry. Tier B:
no carved code reads `stageLinkAngle` back, so the consumer is unobserved.

## Comment moved from src/DreamSys.c (round 92, track 7)

Replaced in the source by a comment that says what the code does; kept here as written.

```c
/* Set (whole word) into `this->stageLinkAngle` by DreamSys__TryStageTimerLink just before an
   ExecuteLink; only ever address-taken here, never dereferenced by this
   unit's queued functions. */
```
