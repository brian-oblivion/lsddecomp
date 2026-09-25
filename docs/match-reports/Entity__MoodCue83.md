# Entity__MoodCue83

> Renamed from `func_800636E4` on 2026-09-25 (tools/rename.py). Address 0x800636e4.

**Unit:** Entity_f · **Size:** 40 words · **Status:** MATCHED (40/40 words)

## What it does

Small mood-dispatch handler. `if (this->unk84 % 15 == 0) { out->unk10=0;
out->unk1C=0xC; out->unk20=2; }` then, independently, `if (this->unkFC ==
this->unk80) { out->unk1C=-2; this->methods->slot16C(this); this->unk44=1;
}`.

## Derivation

The `%15` divisor was NOT obvious from the magic constant alone
(`0x88888889`, GCC's magic multiplier, is shared across several nearby
divisors depending on the accompanying shift/reconstruction). Read the
RECONSTRUCTION instructions instead of the magic constant: retail rebuilds
the multiple as `(quotient << 4) - quotient` (`sll` by 4 then `subu`
quotient), i.e. `quotient * 15` — that arithmetic identity is what pins
the divisor, not the magic number by itself. Verified by matching, not
guessed.

## Proposed learning

**When reverse-engineering a `%N` from a magic-multiply sequence, read the
RECONSTRUCTION shift/add chain (the `sll`/`addu`/`subu` after `mfhi`), not
just the magic constant** — the same magic constant is reused by GCC 2.6.3
across a small family of related divisors, and only the final
multiply-back tells you which one. An early guess of `%8` here (plausible
from a passing glance at the same magic constant used elsewhere) would
have been silently wrong; the reconstruction math (`quotient*16 -
quotient*1 = quotient*15`) is unambiguous.
