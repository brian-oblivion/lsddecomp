# DreamSys__AddFlashback

**Unit:** DreamSys · **Size:** 52 words · **Status:** MATCHED (52/52)

## What it does

Already forward-declared (`void DreamSys__AddFlashback(DreamSys *this, s32
stage, PlayerSpawnPoint* pos, s32 *angles, s32 unknown, s32 time, s32
day);`). Picks a storage slot -- appending if there's room, otherwise
evicting a pseudo-random one keyed off `dreamTimer` -- and writes all seven
fields of a `FlashbackEntry` into it.

## The C

```c
void DreamSys__AddFlashback(DreamSys *this, s32 stage, PlayerSpawnPoint *pos, s32 *angles, s32 unknown, s32 time, s32 day)
{
	FlashbackEntry *entry;

	entry = this->storedFlasbacks;
	if (this->amountFlashbacksAvailable < 10) {
		entry += this->amountFlashbacksAvailable++;
	} else {
		entry += (u32)this->dreamTimer % 9;
	}
	entry->stageID = stage;
	entry->position = *pos;
	entry->rotation = *(FlashbackRotation *)angles;
	entry->unknown_value_0x1c = unknown;
	entry->timeLimit = time;
	entry->day = day;
}
```

## Two struct corrections (both load-bearing, not cosmetic)

- **`FlashbackEntry::unknown_value_0x1c` was `s32`, retyped to `s16`.** This
  function writes it with a bare `sh` (halfword store) from an `s32`
  argument (`unknown`) -- a genuinely `s32` field fed by an `s32` value
  would store all 4 bytes (`sw`), not 2. Exactly the class of mistake the
  head flagged after `ExecuteLink`'s `bool`-is-`int` finding: assuming word
  granularity at a raw offset reads as a struct-layout error, but the real
  cause is a field that's narrower than guessed. The 2 bytes this frees up
  before `day` become ordinary C alignment padding, keeping
  `sizeof(FlashbackEntry) == 0x24` unchanged (confirmed independently by
  `DreamSys__LoadNextFlashback`'s index arithmetic this round).
- **`pitch`/`heading`/`roll` grouped into one new 12-byte nested struct,
  `FlashbackRotation`.** This function's own `angles` argument is
  block-copied into all three in ONE retail load-all-then-store-all
  sequence (six unaligned `lwl`/`lwr` loads, all six BEFORE any of the six
  unaligned `swl`/`swr` stores) -- the already-documented "whole-struct
  assignment reproduces retail's block-move codegen" idiom, this time at a
  12-byte, non-power-of-2 size. Three separate field assignments, or a
  loop, would not reproduce that instruction ordering (confirmed by the
  residue below).

## The real residue: register identity, resolved by variable shape, not declaration order

First three attempts all reached the SAME instruction sequence with
DIFFERENT physical registers throughout the whole function body (`t0` vs.
`a2`/`v0`, `a0` vs. `v1`, etc. -- a textbook "same instructions, different
registers" residue per `DECOMPILATION_LEARNINGS.md`). The variants tried,
in order:

1. `idx` + `&this->storedFlasbacks[idx]` computed fresh after the branch
   (no hoisted base pointer): wrong from the start (3/52) -- GCC never
   isolated `this + 0x470` into its own register, so it had to keep `this`
   alive to the very end, which starved a register `retail` frees for
   reuse (dreamTimer's modulo) partway through, cascading register
   pressure into `pos`'s handling too.
2. `FlashbackEntry *base = this->storedFlasbacks;` computed up front, `idx`
   a separate local, `entry = &base[idx]` after the branch: reordering
   fixed the base-pointer hoist (matched `t0`'s early computation exactly)
   but a NEW register swap appeared around `pos`/`idx` (`t0`↔`a2`, `a0`↔`v1`
   throughout) -- reordering the three declarations (`base`/`idx`/`entry`)
   did not change this.
3. **Merging `base` and `idx` into ONE pointer variable, incremented in
   place** (`entry = this->storedFlasbacks; entry += ...;`) instead of a
   separate index added at the end: matched immediately, all 52 words.

The working shape has exactly ONE local surviving the branch (a pointer,
already advanced to its final value inside each arm) rather than TWO
(a base pointer plus a separate index combined afterward) -- fewer
simultaneously-live locals gave GCC 2.6.3's allocator less to juggle,
and it picked the same registers retail's compiler did once there was
nothing else competing for them.

## Third-learning check (per head's request)

**Not needed.** `entry` (after the fix) is written once per branch and read
several times afterward with no intervening `jalr` anywhere in the
function -- this is a leaf function, no calls at all. The residue here was
pure register-identity/allocation-shape, unrelated to the `jalr`-aliasing
lever from the previous unit.

## Proposed learning

**When a residue is "same instructions, all registers swapped, throughout
the WHOLE function" (not just one instruction), try reducing the number of
LIVE locals before trying declaration-order permutations.** Two variables
carrying what is conceptually one pointer value (a base + a later-added
offset) gave the register allocator more simultaneously-live state to place
than one variable that is incrementally advanced to its final value inside
each branch -- collapsing them changed the allocator's own choices, even
though every arithmetic operation and its order stayed identical between
attempts 2 and 3. Reordering the *declarations* alone (already tried, no
effect) is a cheaper first move than reshaping the *variables*, but when it
doesn't move a whole-function register swap, reshaping the variable count
is the next lever, not a `register T v asm("$N")` pin (which would still be
banned here regardless).
