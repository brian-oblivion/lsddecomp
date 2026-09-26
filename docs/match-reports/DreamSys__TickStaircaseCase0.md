# DreamSys__TickStaircaseCase0

> Renamed from `func_8005AB2C` on 2026-09-22 (tools/rename.py). Address 0x8005ab2c.

**Unit:** DreamSys · **Size:** 62 words (0xF8 bytes) · **Status:** MATCHED
(62/62 words, whole-image `./build-and-verify.sh` green)

## What it does

`(DreamSys *this) -> s32`. Sibling of the already-matched `DreamSys__TickStaircaseCase2`
(same shape, described in its own report) but with **two** windows per arm
instead of one, and different thresholds/constants:

```c
s32 DreamSys__TickStaircaseCase0(DreamSys *this)
{
	if (this->unk_0x914 == 0) {
		DreamSys__ApplyRelativeOffset(this, &STAIRCASE_OFFSET_0, &this->unk_0x91C);
	}
	if (this->unk_0xAC != 4) {
		if (this->unk_0x914 >= 0x85)
			return 1;
		if ((u32)(this->unk_0x914 - 0x2B) < 0xF || (u32)(this->unk_0x914 - 0x4B) < 0xF) {
			this->unk_0xA4 = 2;
		}
	} else {
		if (this->unk_0x914 >= 0x13)
			return 1;
		if ((u32)(this->unk_0x914 - 8) < 2 || (u32)(this->unk_0x914 - 0xD) < 2) {
			this->vt->SceneNode__UpdateRotation(this, 0, &ROTATION_YAW_PLUS45);
		}
	}
	this->unk_0xA0 = 1;
	this->unk_0x914++;
	return 0;
}
```

| arm | band guard | in-band windows (either) | in-band action |
| --- | --- | --- | --- |
| `unk_0xAC != 4` | `< 0x85` | `0x2B..0x39`, `0x4B..0x59` | `this->unk_0xA4 = 2` |
| `unk_0xAC == 4` | `< 0x13` | `8..9`, `0xD..0xE` | `vt->SceneNode__UpdateRotation(this, 0, &ROTATION_YAW_PLUS45)` |

Past the band guard, both arms `return 1` immediately; otherwise both fall to
a shared tail: `unk_0xA0 = 1`, `unk_0x914++`, `return 0`.

## Residue and the fix (worth recording precisely)

The very first attempt used `DreamSys__TickStaircaseCase2`'s exact idiom verbatim, just with
the guard inverted from `<` to `>=` swapped into a plain `if (cond) {...}
else return 1;`:

```c
/* WRONG shape, 29/62 (then worse variants), function 8 bytes too long */
if (this->unk_0x914 < 0x85) {
	if (window_a || window_b) { this->unk_0xA4 = 2; }
} else {
	return 1;
}
```

This produced a function **2 words too long**, which (as `build exit=`
correctly reported: the whole-image `make check` failed, not just this
function) shifted every later linked address, including two unrelated,
already-established data symbols (`STAIRCASE_OFFSET_0`, `STAIRCASE_OFFSET_2`) that this same
function references — their `%lo` immediates read as wrong by exactly the
same +8-byte delta as the size overshoot. This looked at first like a broken
symbol/linker resolution (worth flagging: do not chase a linker mystery
before checking whether your OWN function's word count already matches
retail — an `asm-differ` size warning, or comparing `funcdiff`'s reported
byte range width against the `nonmatching N` in the stale `.s` header, settles
it in seconds).

The actual defect was the **same open branch-polarity question already
logged for `DreamSys__TimerTick` this round**: plain `if (cond) {A} else
{B};` does not reliably reproduce retail's choice of which side is the
fallthrough and which is a forward branch once the `if`-body's own size
changes. `DreamSys__TickStaircaseCase2`'s single-window version happened to compile
correctly with plain `if`/`else`; this function's *two*-window (`||`)
version did not, changing GCC's own placement heuristic. The fix, applied to
BOTH arms, was the same "guard clause" rewrite already used for
`DreamSys__TimerTick`: invert the outer test and return early, so the
window-check code becomes the unconditional tail rather than one arm of a
two-sided `if`/`else`:

```c
if (this->unk_0x914 >= 0x85)
	return 1;
if (window_a || window_b) { this->unk_0xA4 = 2; }
```

This matched first try after the rewrite (62/62).

## New knowledge

- **`STAIRCASE_OFFSET_0` (`struct RelativePos`)**, passed as `DreamSys__ApplyRelativeOffset`'s `a`
  argument by this function — same call shape as `DreamSys__TickStaircaseCase2`'s
  `STAIRCASE_OFFSET_2`, just a different constant 0x10 bytes earlier in the same
  table.
- **Confirms and sharpens the branch-polarity lesson from
  `DreamSys__TimerTick`'s report this round**: it is not just "an `if`/`else`
  can compile with either side as fallthrough" in the abstract — the SAME
  logical shape (`if (a < limit) {...} else return N;`) compiled correctly
  with plain `if`/`else` for `DreamSys__TickStaircaseCase2`'s single-window body and
  INCORRECTLY for this function's two-window (`||`) body. The `if`-body's own
  size/complexity is part of what decides GCC 2.6.3's fallthrough choice, not
  just the source's polarity. The reliable fix in both instances tried this
  round is the same: write the smaller side as an inverted-condition early
  `return`, forcing the larger side to be the unconditional tail rather than
  a branch of an `if`/`else`.
- **A whole-image size regression can manifest as an apparently-unrelated
  symbol-resolution mystery.** Two long-established data symbols
  (`STAIRCASE_OFFSET_0`/`STAIRCASE_OFFSET_2`, both already used correctly by the
  already-matched `DreamSys__TickStaircaseCase2`) appeared to resolve to addresses 8 bytes
  higher than their names once this function was 2 words too long — not a
  linker bug, just "the three ways a score lies" #3 (address drift) wearing
  an unfamiliar costume. Running `make extract` (permitted; changes nothing
  tracked) to double check the auto-generated symbol files was a dead end
  here — the actual fix was fixing this function's own instruction count.

### Proposed learning

**A whole-image address-drift can look exactly like a broken/off-by-N symbol
resolution for data your function references, especially for two symbols 0x10
bytes apart in an undivided table** — check your own function's word count
against retail's `nonmatching N` header (or `funcdiff`'s reported byte range)
BEFORE suspecting the linker or `config/undefined_syms_auto*.txt`. Also
promotes the `DreamSys__TimerTick` finding from a single instance to a
pattern: `if (cond) {big} else {small-return};` is not reliably reproduced by
plain `if`/`else` once "big" grows past whatever GCC 2.6.3's placement
heuristic keys on — write the small side as an inverted guard-clause early
return instead, unconditionally, as a first attempt rather than a last resort.

## Naming

- **Tier B.** One of the 4-entry STAIRCASE_TICK_FNS dispatch table's own functions, in table order (this is index 0), chosen by DreamSys__TryStaircaseLink via GetLastSpawnExtra. Does an initial DreamSys__ApplyRelativeOffset against STAIRCASE_OFFSET_0, then a stage-family bounds check incrementing the unk_0x914 attempt counter. Only ever wired up after Test4StaircaseNodes succeeds -- the "staircase" context is solid; the difference between cases 0..3 is not.
