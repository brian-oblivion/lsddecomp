# PickStyleFallbackConfig -- MATCHED (62/62 words), ObjMStyleActor

> Renamed from `func_80054758` on 2026-09-23 (tools/rename.py). Address 0x80054758.

Round 46 (second sitting, alpha). Byte-exact, whole-image SHA1 verified.

## Signature

```c
void *PickStyleFallbackConfig(void);
```

The first function in the unit (ROM order), so every extern it needs is a
fresh copy (same reasoning as `StyleUpdateEffectSlots`/`TryStartStyleCue`/
`StyleBuildEffectSlots`).

## New externs

```c
extern s32 sStyleDay;         /* already s32 in ObjMStyleActor.c */
extern s32 gStyleStage;         /* already s32 in ObjMStyleActor.c and this unit's own StyleScrollVramStrips */
extern s8 gStyleVariantPicks[];        /* 16-entry table, indexed by (sStyleDay+gStyleStage)&0xF */
extern s32 gStyleVariant;
extern s8 gStyleVariantConfigCounts[];        /* divisor table, indexed by "kind" -- raw index, no scale */
extern s32 sStyleConfigIndex;
extern s32 gStyleVariantConfigs[];       /* array of raw base addresses, indexed by "kind" (scaled x4) */
extern s32 sStyleClearColor;
extern u8 sStyleDecorColorsB[];        /* address only taken */
extern u8 gStylePalette[];        /* 3-byte-stride table, indexed by a byte field */
extern s32 sStyleDecorColors;
extern u8 sStyleDecorColorsA[];        /* address only taken */
extern s32 sStyleDecorVariant;
```

## Body

```c
void *PickStyleFallbackConfig(void) {
    s32 sum;
    s32 kind;
    s32 divisor;
    s32 remainder;
    s8 *result;
    s32 b3;
    s32 b2;
    u8 *tab;

    sum = sStyleDay + gStyleStage;
    kind = gStyleVariantPicks[sum & 0xF];
    gStyleVariant = kind;
    divisor = gStyleVariantConfigCounts[kind];
    remainder = sum % divisor;
    sStyleConfigIndex = remainder;
    result = (s8 *) gStyleVariantConfigs[kind] + remainder * 4;
    if (kind == 0) {
        b3 = result[3];
        sStyleClearColor = (s32) (gStylePalette + b3 * 3);
        b2 = result[2];
        tab = sStyleDecorColorsB;
        if (b2 != 0x12) {
            tab = sStyleDecorColorsA;
        }
        sStyleDecorColors = (s32) tab;
        if (remainder < 4) {
            sStyleDecorVariant = 1;
        } else if (remainder < 6) {
            sStyleDecorVariant = 2;
        }
    }
    return result;
}
```

Notes:

- `gStyleVariantConfigs[kind]` is loaded as a raw `s32` *value* (not an address-of),
  then used as a base address for further byte-granular pointer arithmetic
  (`+ remainder * 4`) -- exactly the `gStyleSceneRefs` "pointer stored as a plain
  scalar" idiom already established elsewhere in this unit, just for a
  different global.
- The `sum % divisor` compiles to the standard MIPS `div`/`break 7`
  (divide-by-zero trap)/`break 6` (`INT_MIN / -1` trap)/`mfhi` sequence.
  There is no `mflo` anywhere in retail's bytes, confirming the source uses
  only `%`, never `/`, on this pair -- writing an unused `quotient = sum /
  divisor;` alongside it would be wrong (and would very likely emit a
  spurious `mflo`).
- `gStyleVariantConfigCounts[kind]` and `gStyleVariantPicks[idx]` are indexed with NO scale factor
  in retail (`addu $at,$at,$a0`, not `sll`+`addu`) because both are `s8`
  arrays -- plain C array indexing on a 1-byte element type already
  reproduces this without any special casting.

## Lever: a ternary's branch/default assignment ORDER is not guaranteed to
match source intent -- write it as an explicit default-then-override

First attempt used `tab = (b2 == 0x12) ? sStyleDecorColorsB : sStyleDecorColorsA;`, which
compiled to the OPPOSITE physical layout from retail: GCC chose
`sStyleDecorColorsA` as the unconditional default and `sStyleDecorColorsB` as the
conditional override (with the branch polarity flipped to match, `bne`
where retail has `beq`) -- functionally identical, but two words differ
because the *constant addresses* land in the swapped slots and the branch
test is inverted. Retail's actual shape is imperative, not ternary-shaped:
compute the default (`sStyleDecorColorsB`) unconditionally, then overwrite it with
`sStyleDecorColorsA` only when `b2 != 0x12`:

```c
tab = sStyleDecorColorsB;
if (b2 != 0x12) {
    tab = sStyleDecorColorsA;
}
```

Reproduced exactly on the next build; no permuter needed.

### Proposed learning

**A two-word residue where only two `lui`/`addiu` immediate operands are
SWAPPED (and a nearby branch's polarity is flipped to match) is a ternary
whose default/override assignment order doesn't match the source's — GCC is
free to pick EITHER operand as the speculative "compute first" value for a
`cond ? A : B` expression, independent of which literal is written first in
the ternary.** Rewrite as an explicit `x = A; if (!cond) x = B;` (or the
mirror) matching whichever operand retail's bytes show being computed
*before* the branch, rather than trying to guess the right ternary spelling.

## Attempts

2 (first: ternary form, 59/62 with two swapped-constant words and one
inverted branch; second: explicit default-then-override, byte-exact).

## Naming

**`PickStyleFallbackConfig`, tier B.**

Literal call site in `ApplyStyleConfig` (ObjMStyleActor.c, already matched):
`cfg = func_80054758();`, used only when the direct per-`gStyleStage` config
table entry (`sStyleStageConfigs[gStyleStage]`) is NULL -- i.e. this is the fallback
path. Body hashes `sStyleDay + gStyleStage` into a 16-entry table to pick
a `gStyleVariant` ("kind"), then a per-variant divisor/remainder select a
config row. "Fallback" is evidenced by the call site; "kind"/variant
selection mechanics are evidenced by the body; WHY a fallback is needed, or
what the variant means in gameplay terms, is not established (tier B, not
A).

## Round 93 polish (delta, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `D_800873DC` | `gStyleVariantPicks` | B | 16 bytes, each 0..3, indexed `(day + stage) & 0xF`; the byte read is stored as `gStyleVariant`. |
| `D_800873D8` | `gStyleVariantConfigCounts` | A | 4 bytes {7, 10, 12, 5}, indexed by the variant, the divisor of the config index; they are exactly the record counts of the four tables `gStyleVariantConfigs` points at (0x80087340: 7 words, ...735C: 10, ...7384: 12, ...73B4: 5). |
| `D_800873C8` | `gStyleVariantConfigs` | A | 4 pointers, one per variant, to arrays of 4-byte config records; the function returns `table[variant] + index * 4`, the record ApplyStyleConfig/FillStyleFromConfig read. |
| `D_8008AC84` | `sStyleConfigIndex` | A | written with the config index `(day + stage) % count`; nothing reads it. |
| `D_800872C4` | `gStylePalette` | A | 24 RGB triples (FillStyleFromConfig, ApplyStyleConfig and this function index it by a config byte); this function takes byte 3's entry as `sStyleClearColor`. |
| `D_80087234`, `D_8008726C` | `sStyleDecorColorsA`, `sStyleDecorColorsB` | B | two 18-triple colour tables (one per decor band); B when config byte 2 is palette entry 18 (`STYLE_DECOR_B_PALETTE_INDEX`), A otherwise. Which look each is, is not established, hence the letters. |
| `gStyleFlushColor` | `sStyleClearColor` | A | its only reader, StyleUpdateDecorSet, hands the adjusted copy to the viewport's setClearColor. |
| `gStyleColorTable` | `sStyleDecorColors` | A | the current band colour table: StyleBuildDecorSet colours band i from entry i. |
| `gStyleKind` | `gStyleStage` | A | RegisterStyleConfig stores its arg1 there, and its one caller, ObjM__InitStyleAndWorld, passes `self->stage`. |
| `gStyleCounter` | `sStyleDay` | A | RegisterStyleConfig stores its arg3 there; its one caller passes `DreamSys__GetCurrentDayAndYear()` (`currentDay + 1`). |

Locals: `seed` (day + stage), `variant`, `count`, `index`, `config`, `clearIndex`/`decorIndex` (config bytes 3 and 2), `decorColors`.
