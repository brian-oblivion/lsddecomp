# ApplyStyleConfig -- MATCHED, round 44 (2026-09-15)

> Renamed from `func_80054558` on 2026-09-23 (tools/rename.py). Address 0x80054558.

Unit `ObjMStyleActor`. **41/41 words, byte-exact.** Reopened (both `gp_rel`
and `addiu_at` are resolved), never attempted before this round.

## What it does

Looks up a "cfg" byte-array pointer for the current style index
(`gStyleStage`, set by `RegisterStyleConfig`) in the 14-entry pointer table
`sStyleStageConfigs`; if the slot is NULL, falls back to `PickStyleFallbackConfig()` to
produce one. Feeds `cfg` into the already-matched `FillStyleFromConfig(style,
cfg)` against the fixed global `sStyleConfig` (a `StyleM` instance, split by
splat into two adjacent labels `sStyleConfig`/`sStyleKind2AltColor` purely because
something else references the middle of it -- the object is one 0x20-byte
struct). Then does its own separate raw-byte read of `cfg[1]`/`cfg[2]`: if
`cfg[1] >= 4`, stores a `gStylePalette[cfg[2]]` colour-table entry pointer into
`sStyleDecorColor`. Always returns `&sStyleConfig`.

```c
struct StyleM;   /* forward tag; full definition stays where it already is,
                  * right before FillStyleFromConfig further down this unit */

extern s32 sStyleConfig;
extern s8 *sStyleStageConfigs[];
extern s8 *PickStyleFallbackConfig(void);
extern void FillStyleFromConfig(struct StyleM *style, s8 *cfg);
extern u8 gStylePalette[][3];
extern const u8 *sStyleDecorColor;

void *ApplyStyleConfig(void) {
    s8 *cfg = sStyleStageConfigs[gStyleStage];

    if (cfg == 0) {
        cfg = PickStyleFallbackConfig();
    }
    FillStyleFromConfig((struct StyleM *) &sStyleConfig, cfg);
    if (cfg[1] >= 4) {
        sStyleDecorColor = gStylePalette[cfg[2]];
    }
    return &sStyleConfig;
}
```

## The one real lever: single join point, not two return statements

First attempt wrote the natural-looking guard form:

```c
if (cfg[1] < 4) {
    return &sStyleConfig;
}
sStyleDecorColor = gStylePalette[cfg[2]];
return &sStyleConfig;
```

This built 40/41 words with the tail one word SHORT: retail has an extra
`move v0,s1` immediately before falling into the shared epilogue, which my
version didn't emit. Cause: retail's `gStylePalette[cfg[2]]` address
computation clobbers `v0` as scratch (it's a 3-way live register at that
point -- the delay slot of the `bnez` unconditionally sets `v0 = s1` before
either path runs), so retail needs to explicitly restore `v0 = s1` before
returning. My version's register allocator picked different scratch
registers (`v1`/`a0`) for the same arithmetic, leaving `v0` untouched from
the delay slot -- so GCC correctly elided the now-redundant restore, but
that made the two versions different LENGTHS, not just different register
names, and the whole build shifted the same way every drift bug in this
project does. **Rewriting the two-return form as a single-join form** (`if
(cfg[1] >= 4) { store; } return &sStyleConfig;`) changed which registers GCC
picked for the store's address arithmetic, which coincidentally restored the
exact retail register assignment and the redundant `move` reappeared,
matching byte-for-byte on the next build.

### Proposed learning

A one-word-short diff whose only visible difference is a missing final
`move $vN,$sM` right before the epilogue, with everything else matching
including branch targets, is a REGISTER ALLOCATION artifact of how the
early-return is phrased, not a missing statement. Two semantically identical
control-flow shapes -- "early `return X;` then a second `return X;` at the
end" vs "wrap the tail in `if (!cond) { ... }` and fall through to one
`return X;`" -- are not guaranteed to compile to the same registers for
code that executes only on the non-early path, because they hand the
register allocator different amounts of the function to consider live at
once. When a near-miss is exactly one word short and the only reg swaps are
around a computation that feeds a value already sitting in the return
register from an earlier branch, try collapsing to a single join point
before anything more invasive.

## Naming

**ApplyStyleConfig** -- tier B. Looks up the current style's config-byte pointer (`sStyleStageConfigs[gStyleStage]`), falling back to the uncarved `PickStyleFallbackConfig` if unset, fills the shared `StyleM` global via `FillStyleFromConfig`, and conditionally sets a colour-table pointer. Same tier and caveat as `RegisterStyleConfig`.

## Track 7 (2026-09-27, round 98, delta)

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `D_800873EC` | `sStyleStageConfigs` | A | indexed by gStyleStage; NULL entries fall back to PickStyleFallbackConfig |
| `D_80087424` | `sStyleConfig` | A | the StyleConfig record this returns and ObjM keeps as `styleConfig`; now declared `StyleConfig` |
| `cfg[1] >= 4` | `cfg->fogLevel >= STYLE_DECOR_FOG_LEVEL` | B | the byte indexes sStyleFogNears; 4 and 5 are its two nearest distances (4096, 2048) |

The config pointer is a unit-local `StyleStageConfig` (four `s8`:
colorMode, fogLevel, farColorIndex, clearColorIndex -- what
FillStyleFromConfig stores each byte as). Zero bytes.

Proposal: `sStyleKind2AltColor` (0x80087430, ObjMStyleActor) is
`sStyleConfig + 0x00C`, i.e. `sStyleConfig.clearColor`: its "alternate
colour" is the current config's clear colour.
