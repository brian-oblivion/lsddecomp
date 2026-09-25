# Entity__MoodCue88

> Renamed from `func_80063D40` on 2026-09-25 (tools/rename.py). Address 0x80063d40.

**Unit:** Entity_f · **Size:** 34 words · **Status:** MATCHED (34/34 words)

## What it does

Near-sibling of `Entity__MoodCue87`: `out->unk10 = this->methods->slot148(this);
if (out->unk4 == this->unk80 / 2) { out->unk1C = 0x12; } if (out->unk4 >=
this->unk80 - 1) { out->unk4 = -1; }`.

## Derivation

The `unk80 / 2` comparison compiles to the signed-divide-by-2 idiom
already documented in `include/Entity.h`'s own comment on `Entity::unk80`
(`(x + (unsigned)x>>31) >> 1`, from `Entity__MoodCue09`/`Entity__MoodCue11` in
Entity.c) — confirmed the plain `/2` operator reproduces it here too
without needing to hand-transcribe the shift/add sequence, consistent
with this project's established "just write `%`/`/`, trust the compiler"
policy for constant divisors.

An early draft hand-transcribed the idiom directly as
`(this->unk80 + (u32)this->unk80 >> 31) >> 1` — this is WRONG C even
setting the toolchain question aside: `+` binds tighter than `>>` in C, so
that expression parses as `(unk80 + (u32)unk80) >> 31`, not the intended
`unk80 + ((u32)unk80 >> 31)`. Caught before it ever got a build (a
precedence bug, not a residue), and replaced with the plain `/ 2` per the
existing documented idiom.
