# Entity__MoodCue69 -- MATCHED (53/53 words)

> Renamed from `func_800623E8` on 2026-09-24 (tools/rename.py). Address 0x800623e8.

Unit: `Entity_e` (round 13). A mood handler that dispatches `slot148`, gates a
final-tick check against `this->unk80 - 1`, and fires two independent
"every N ticks" checks against `out->unk4` (mod 4 and mod 200).
`void Entity__MoodCue69(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue69(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (this->unk84 == this->unk80 - 1) {
        out->unk1C = 0x19;
        out->unk20 = -2;
    }
    if (out->unk4 % 4 == 0) {
        out->unk30 = 0x15;
        out->unk34 = -1;
    }
    if (out->unk4 % 200 == 0) {
        out->unk44 = 0xD;
        out->unk48 = 1;
    }
}
```

## Derivation notes

Two division-by-constant idioms, both resolved by compiling one-line probes
through the pinned `cc1` rather than hand-decoding the magic constants (see
CLAUDE.md's "Escalate, do not experiment" reproducer pipeline and
`Entity__MoodCue64`'s match report, same technique):

- The `out->unk4 & 3` test (`andi $v0, $v0, 0x3`) is GCC's equality-to-zero
  form of `% 4 == 0` -- for a power-of-two modulus the low bits alone decide
  divisibility regardless of sign, so GCC substitutes the bitwise AND
  directly instead of the magic-number sequence used for non-power-of-two
  divisors elsewhere in this unit (`% 300`, `% 30`).
- The `0x51EB851F` magic constant with a post-`mfhi` shift of 6, followed by
  a `*200` reconstruction (`sll 1`/`addu`/`sll 3`/`addu`/`sll 3` == `*2, +1,
  *8, +1, *8` == `*200`) and a final `subu`/`bne` against the original
  dividend, is GCC 2.6.3's standard "is `x` a multiple of 200" idiom.
  Verified directly: `int f(int x){return x/200;}` through
  `tools/gcc263/cc1 -mips1 -mcpu=3000 -quiet -G0 -O2` reproduces this exact
  magic/shift pair byte-for-byte. A shift of 6 uniquely identifies divisor
  200 among nearby candidates (160/200/240/250/300/320 probed; each gives a
  different shift amount), so the shift alone is enough to pin the divisor
  without touching the magic-number-to-divisor formula.

`this->methods->slot148` was already typed by `Entity_d`'s work (returns
`s32`); `this->unk80`/`this->unk84` and all `EntityMoodHandlerArg` fields
touched here were already known from sibling handlers in this unit. No new
struct knowledge.

### Proposed learning

**A shift amount after `mfhi` uniquely fingerprints a division's divisor,
faster than decoding the magic constant.** Probing `int f(int x){return
x/N;}` for a handful of candidate `N` near the guess and comparing the `sra
$2,$2,K` shift each produces is enough to identify the right `N` -- no two
of the divisors this project has hit so far (30, 200, 300) share a shift
amount. This generalizes `Entity__MoodCue64`'s reproducer-over-hand-decoding
learning: don't even need to match the whole instruction sequence, just the
shift constant.
