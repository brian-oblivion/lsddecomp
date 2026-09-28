# DreamSys__TickStaircaseCase3

> Renamed from `func_8005AE40` on 2026-09-22 (tools/rename.py). Address 0x8005ae40.

**Unit:** DreamSys · **Size:** 73 words (0x124 bytes) · **Status:** MATCHED
(73/73 words, whole-image `./build-and-verify.sh` green)

## What it does

Third sibling of `DreamSys__TickStaircaseCase2`/`DreamSys__TickStaircaseCase0` (same retry-counter shape),
with a THIRD extra piece: both arms, after their own window checks, also
compute a boolean "close to the next trigger" flag that (if set) bumps
`unk_0x88` to 2 — shared tail logic neither of the other two siblings had:

```c
s32 DreamSys__TickStaircaseCase3(DreamSys *this)
{
	s32 flag;

	if (this->unk_0x914 == 0) {
		DreamSys__ApplyRelativeOffset(this, &STAIRCASE_OFFSET_3, &this->unk_0x91C);
	}
	if (this->unk_0xAC != 4) {
		if (this->unk_0x914 >= 0x71)
			return 1;
		if ((u32)(this->unk_0x914 - 0x1E) < 0xF || (u32)(this->unk_0x914 - 0x52) < 0xF) {
			this->unk_0xA4 = 1;
		}
		flag = (u32)(this->unk_0x914 - 0x1E) < 0x34;
	} else {
		if (this->unk_0x914 >= 0x13)
			return 1;
		if ((u32)(this->unk_0x914 - 6) < 2 || (u32)(this->unk_0x914 - 0xF) < 2) {
			this->vt->SceneNode__UpdateRotation(this, 0, &sRotationYawMinus45);
		}
		flag = (u32)this->unk_0x914 < 9;
	}
	if (flag) {
		this->unk_0x88 = 2;
	}
	this->unk_0xA0 = 1;
	this->unk_0x914++;
	return 0;
}
```

| arm | band guard | in-band windows | in-band action | shared "flag" test |
| --- | --- | --- | --- | --- |
| `unk_0xAC != 4` | `< 0x71` | `0x1E..0x2C`, `0x52..0x60` | `unk_0xA4 = 1` | `(unk_0x914-0x1E) < 0x34` |
| `unk_0xAC == 4` | `< 0x13` | `6..7`, `0xF..0x10` | `vt->SceneNode__UpdateRotation(this,0,&sRotationYawMinus45)` | `unk_0x914 < 9` |

Matched first attempt, with the `DreamSys__TimerTick`/`DreamSys__TickStaircaseCase0`
guard-clause lever (invert the band test, early `return 1;`, let the
window-check code be the unconditional tail) applied up front rather than
discovered by iteration — both out-of-band early exits land on the SAME
physical `return 1;` block in retail (unlike `DreamSys__TickStaircaseCase0`'s two separate
copies), and writing two ordinary `if (x >= limit) return 1;` guard clauses,
one per arm, let GCC's own tail-merge find that sharing without any
additional coaxing.

## New knowledge

- **`STAIRCASE_OFFSET_3`** (`struct RelativePos`), a third constant in the same table
  as `DreamSys__TickStaircaseCase2`'s `STAIRCASE_OFFSET_2` and `DreamSys__TickStaircaseCase0`'s `STAIRCASE_OFFSET_0`,
  passed as `DreamSys__ApplyRelativeOffset`'s `a` argument.
- **`sRotationYawMinus45`**, another opaque forwarded-pointer constant for
  `vt->SceneNode__UpdateRotation`'s `arg2`, same shape as the already-known
  `sRotationYawPlus45`.
- **A single flag value can be computed from TWO DIFFERENT numeric
  expressions depending on which arm set it** (`(unk_0x914-0x1E) < 0x34` in
  one arm, `unk_0x914 < 9` in the other) and still merge into ONE shared
  `if (flag) { ... }` block afterward — worth remembering when a residue
  looks like "two arms feeding the same test" resists a single shared
  expression; the VALUES computed differ, not just their representation.

### Proposed learning

None new beyond confirming, a third time this round
(`DreamSys__TimerTick`, `DreamSys__TickStaircaseCase0`, this function), that `if (cond)
return N;` as an unconditional guard clause — rather than `if (!cond) {...}
else return N;` — is the more reliable first attempt for an out-of-band early
exit in this codebase's `New_X`-adjacent retry-counter family, and that GCC's
own tail-merge (not manual `goto` engineering) is enough to unify two such
guard clauses into one physical block when retail does so.

## Naming

- **Tier B.** Table index 3 of the same sStaircaseTickFns family, against STAIRCASE_OFFSET_3; same evidence and caveat.
