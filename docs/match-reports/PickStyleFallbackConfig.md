# PickStyleFallbackConfig -- MATCHED (62/62 words), class_3bb8c_n

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
extern s32 gStyleCounter;         /* already s32 in class_3bb8c_m.c */
extern s32 gStyleKind;         /* already s32 in class_3bb8c_m.c and this unit's own DrawStyleTables */
extern s8 D_800873DC[];        /* 16-entry table, indexed by (gStyleCounter+gStyleKind)&0xF */
extern s32 gStyleVariant;
extern s8 D_800873D8[];        /* divisor table, indexed by "kind" -- raw index, no scale */
extern s32 D_8008AC84;
extern s32 gStyleVariantConfigs[];       /* array of raw base addresses, indexed by "kind" (scaled x4) */
extern s32 gStyleFlushColor;
extern u8 gStyleDecorColorsB[];        /* address only taken */
extern u8 gStylePalette[];        /* 3-byte-stride table, indexed by a byte field */
extern s32 gStyleColorTable;
extern u8 gStyleDecorColorsA[];        /* address only taken */
extern s32 gStyleDecorVariant;
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

    sum = gStyleCounter + gStyleKind;
    kind = D_800873DC[sum & 0xF];
    gStyleVariant = kind;
    divisor = D_800873D8[kind];
    remainder = sum % divisor;
    D_8008AC84 = remainder;
    result = (s8 *) gStyleVariantConfigs[kind] + remainder * 4;
    if (kind == 0) {
        b3 = result[3];
        gStyleFlushColor = (s32) (gStylePalette + b3 * 3);
        b2 = result[2];
        tab = gStyleDecorColorsB;
        if (b2 != 0x12) {
            tab = gStyleDecorColorsA;
        }
        gStyleColorTable = (s32) tab;
        if (remainder < 4) {
            gStyleDecorVariant = 1;
        } else if (remainder < 6) {
            gStyleDecorVariant = 2;
        }
    }
    return result;
}
```

Notes:

- `gStyleVariantConfigs[kind]` is loaded as a raw `s32` *value* (not an address-of),
  then used as a base address for further byte-granular pointer arithmetic
  (`+ remainder * 4`) -- exactly the `gStyleTargetObj` "pointer stored as a plain
  scalar" idiom already established elsewhere in this unit, just for a
  different global.
- The `sum % divisor` compiles to the standard MIPS `div`/`break 7`
  (divide-by-zero trap)/`break 6` (`INT_MIN / -1` trap)/`mfhi` sequence.
  There is no `mflo` anywhere in retail's bytes, confirming the source uses
  only `%`, never `/`, on this pair -- writing an unused `quotient = sum /
  divisor;` alongside it would be wrong (and would very likely emit a
  spurious `mflo`).
- `D_800873D8[kind]` and `D_800873DC[idx]` are indexed with NO scale factor
  in retail (`addu $at,$at,$a0`, not `sll`+`addu`) because both are `s8`
  arrays -- plain C array indexing on a 1-byte element type already
  reproduces this without any special casting.

## Lever: a ternary's branch/default assignment ORDER is not guaranteed to
match source intent -- write it as an explicit default-then-override

First attempt used `tab = (b2 == 0x12) ? gStyleDecorColorsB : gStyleDecorColorsA;`, which
compiled to the OPPOSITE physical layout from retail: GCC chose
`gStyleDecorColorsA` as the unconditional default and `gStyleDecorColorsB` as the
conditional override (with the branch polarity flipped to match, `bne`
where retail has `beq`) -- functionally identical, but two words differ
because the *constant addresses* land in the swapped slots and the branch
test is inverted. Retail's actual shape is imperative, not ternary-shaped:
compute the default (`gStyleDecorColorsB`) unconditionally, then overwrite it with
`gStyleDecorColorsA` only when `b2 != 0x12`:

```c
tab = gStyleDecorColorsB;
if (b2 != 0x12) {
    tab = gStyleDecorColorsA;
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

Literal call site in `ApplyStyleConfig` (class_3bb8c_m.c, already matched):
`cfg = func_80054758();`, used only when the direct per-`gStyleKind` config
table entry (`D_800873EC[gStyleKind]`) is NULL -- i.e. this is the fallback
path. Body hashes `gStyleCounter + gStyleKind` into a 16-entry table to pick
a `gStyleVariant` ("kind"), then a per-variant divisor/remainder select a
config row. "Fallback" is evidenced by the call site; "kind"/variant
selection mechanics are evidenced by the body; WHY a fallback is needed, or
what the variant means in gameplay terms, is not established (tier B, not
A).
