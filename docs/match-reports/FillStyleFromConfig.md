# FillStyleFromConfig — MATCHED (byte-exact, whole-image `build exit=0`)

> Renamed from `func_800545FC` on 2026-09-23 (tools/rename.py). Address 0x800545fc.

Unit: `class_3bb8c_m` · Size: 25 words · Round 23 (2026-09-07), head.
**Matched on the FIRST attempt.** This function's prior disposition was a
carve-time stub saying "do not attempt"; see below.

## History — a false blocker, and how it survived

`src/class_3bb8c_m.c`'s carve-time header comment (round 15) listed four
functions with a blocker profile and the instruction *"All four have stub
reports; do not attempt them."* `FillStyleFromConfig`'s entry was **`addiu-$at`, and
nothing else.**

`addiu_at` was resolved in round 21. From that moment this was matchable ground
that no round would look at, because the unit's own comment told every runner
not to. The other three entries (`gp_rel`, twice, plus one `gp_rel` +
`addiu_at`) are still correctly blocked — which is what made the stale one hard
to notice: three of the four lines were right.

Re-screened with `tools/nearmiss.py` this round it came back clean on both live
blockers, and it matched immediately. The unit comment has been corrected in
place and now says which screen it was run with and when.

## The match

```c
void FillStyleFromConfig(struct StyleM *style, s8 *cfg) {
    style->unkC  = gStylePalette[cfg[3]];
    style->unk18 = gStylePalette[cfg[2]];
    style->unk1C = sStyleFogNears[cfg[1]];
    style->unk14 = cfg[0];
}
```

Four fields of a style/appearance descriptor, filled from a four-byte config
block. Field order in the source is `unkC, unk18, unk1C, unk14` — read off the
store order in the `.s`, not sorted by offset.

`&gStylePalette` is materialised ONCE into `$a2` and reused by both entries; that
falls out of two `gStylePalette[...]` references and needed no local.

## Data established from the executable

- **`u8 gStylePalette[][3]`** — 0x48 bytes = 24 three-byte entries. The first four
  are `00/00/00`, `40/40/40`, `80/80/80`, `FF/FF/FF`: a greyscale ramp, so RGB
  triples. **The `[3]` element type is what produces retail's address
  arithmetic**: `sll $v0, $v1, 1` / `addu $v0, $v0, $v1` / `addu $v0, $v0, $a2`
  is `i*2 + i + base`, i.e. a stride-3 index. A `u8 *` plus explicit `* 3`
  would be a different expression to write and the same bytes; the array typing
  is the honest spelling.
- **`s32 sStyleFogNears[]`** — six words, `0x6800, 0x5000, 0x3800, 0x2000, 0x1000,
  0x0800`, monotonically decreasing. A size/threshold ramp, indexed by `cfg[1]`.
- `cfg` is read at +0, +1, +2, +3 with `lb`, so signed bytes.

## The destination is NOT an `ObjM`, and that mattered

The unit's own class is `ObjM`, so the reflex is to type `$a0` as one and carve
`unkC`/`unk1C` out of its pads. **That would have been wrong and it would have
been wrong in a shared header.** `ObjM::unk14` and `::unk18` are already
established as unrelated object pointers (`FieldM14 *`, `FieldM18 *`) by five
other functions in this unit; this function writes a colour-table pointer to
+0x018 and a plain sign-extended byte to +0x014. Same offsets, incompatible
types — so `$a0` is a different struct.

`struct StyleM` therefore lives in `src/class_3bb8c_m.c`, not in
`include/class_3bb8c.h` (eleven units). Nothing in the shared header changed.

### Proposed learning

**A unit's carve-time blocker profile is a transcribed screen, and it decays
exactly like any other written list — but it decays into a DIRECTIVE, which is
what makes it worse than a stale count.** This one ended with "do not attempt
them", so it did not merely misinform the next reader, it instructed them. One
of its four lines went stale when `addiu_at` was resolved, and the other three
staying correct is precisely why nobody re-read it.

Two things follow, and the second is the general one:

- **When a blocker is resolved, unit HEADER COMMENTS need the same sweep as
  match reports.** Round 22 built the `REOPENED -- ASSIGNABLE` marker so
  `progress.py` could return ground from stale *reports*; a stale line in a
  `src/*.c` comment is invisible to every tool and is read by every runner
  assigned to that unit. Grep for them: `grep -rn 'addiu' src/*.c`.
- **A screen recorded without saying WHICH screen it was cannot be aged.** The
  corrected comment now names the screen (two greps as of round 21), the date,
  and points at `tools/nearmiss.py` — so the next reader can tell whether it
  predates a fix instead of having to re-derive it. Record the method next to
  the verdict, or the verdict outlives its method.

## Naming

**FillStyleFromConfig** -- tier A. Pure field-fill: copies four bytes of `cfg` into the four fields of a `StyleM` (two directly as colour-table lookups, one as a `sStyleFogNears` table lookup, one as a plain sign-extended byte). No branching, deterministic, mechanics are the entire function -- tier A.

## Track 7 (2026-09-27, round 98, delta)

The local `struct StyleM` view (unkC/unk14/unk18/unk1C) is retired onto
include/class_3bb8c.h's `StyleConfig`, whose clearColor/colorMode/
farColor/fogNear are the same four words; `cfg` is a `StyleStageConfig`
(four named bytes, not `s8 *` indexing). `D_8008730C` is
`sStyleFogNears` (tools/rename.py), tier A: six fogNear distances, 26624
down to 2048, indexed by the config's fog level, stored as
StyleConfig::fogNear. Byte-exact on the first build.

Moved from the source comment above the old struct: "FillStyleFromConfig's
destination (D_80087424, via ApplyStyleConfig) is not an ObjM: it is the
record ObjM keeps as `styleConfig`, which include/class_3bb8c.h views as
StyleConfig. The two views stay separate here: the record is not a class,
and merging them is a global's type (track 4b)." They are now one view.
