# ComputeStyleCueFalloff -- MATCHED (36/36 words)

> Renamed from `func_8005627C` on 2026-09-23 (tools/rename.py). Address 0x8005627c.

Unit: `class_3bb8c_r` (round 17 continuation). The shared helper every
`gStyleCueCallbacks` slot occupant calls first: reads a small signed tag byte off
`ctx->methods`, looks it up with a NEGATIVE index into a 15-entry global
table, and returns a chained division result.

## Final source

```c
typedef struct StyleCueParam StyleCueParam;
typedef struct StyleCueParamMethods {
    u8 pad0[0x6];
    s8 tag; /* +0x006 */
} StyleCueParamMethods;
struct StyleCueParam {
    StyleCueParamMethods *methods; /* +0x000 */
    s32 kind;
    u8 pad8[0x8];
    s32 falloff;
    u8 pad14[0x8];
    s32 unk1C, unk20, unk24, unk28, unk2C, unk30, unk34, unk38, unk3C;
    u8 pad40[0x4];
    s32 unk44, unk48, unk4C, unk50;
};

extern s32 gStyleCueDistanceTable[];

s32 ComputeStyleCueFalloff(StyleCueParam *ctx) {
    s32 t = gStyleCueDistanceTable[-ctx->methods->tag];
    s32 q = t / ctx->unk28;

    return ctx->falloff / q;
}
```

## Derivation

- **Placement.** This function's own ROM address, `0x8005627C`, is AFTER
  all 14 `gStyleCueCallbacks` occupants that call it (they run `0x80055A88` ..
  `0x80056238`; this function sits right before the blocked
  `IsStyleVariantEven`). Its definition lives at that later position in the
  file, with a forward declaration (`s32 ComputeStyleCueFalloff(StyleCueParam *ctx);`)
  near the top where `StyleCueParam` is defined -- getting this wrong the first
  time (defining it inline with the struct, ahead of the 14 callers)
  miscompiled the whole file: every one of the 14 callers' own bodies
  still matched byte-for-byte, but every `jal ComputeStyleCueFalloff` in them
  encoded the WRONG target address, and `funcdiff`'s per-function windows
  read as near-total garbage until the ordering was fixed. Round 17's own
  hard rule about strict ROM-address order is not decorative.
- **`gStyleCueDistanceTable[-tag]`, not a hand-transcribed `base - tag*4`.** Retail
  computes `&gStyleCueDistanceTable - tag*4` via `sll`/`subu` (tag is small and
  negative, e.g. -1..-14, landing INSIDE the 15-word table declared at
  `gStyleCueDistanceTable` itself, confirmed directly against
  `asm/data/76DC8.data.s`). Writing the array-index form with a negated
  index lets GCC regenerate the identical `sll`/`subu` address
  computation, rather than transcribing the lowering by hand.
- **Two chained signed divisions**, each compiling to the ordinary `div`
  + zero/overflow `break` guard sequence this pinned toolchain always
  emits for `/`RESULT -- no special handling needed, `t / ctx->unk28` and
  `ctx->falloff / q` reproduce both guard blocks directly.
- `ctx` (the "a0" parameter every one of the 14 callers forwards
  unchanged) is the SAME `StyleCueParam` type used for the "self"/target
  parameter in every leaf -- see those reports and the file banner for why
  a single type was used for both roles.

### Proposed learning

- **Getting the position of a function that OTHER functions call before
  it wrong (in terms of ROM order, not in terms of C declaration order)
  makes the callers' OWN scores UNREADABLE, not just the misplaced
  function's.** All 14 legitimate call sites showed as near-total
  mismatches purely from an encoded absolute jump target, even though
  their own bodies were already byte-correct -- diagnosing this required
  recognizing that funcdiff's window read garbage while asm-differ's
  address-anchored view showed the SAME instructions at SHIFTED addresses,
  the signature of pure ordering drift rather than a real body bug.

## Naming

**Tier B.** Free function (called with `ctx`, not a `self` of its own type in the class-method sense), so no `Class__` prefix. The name reflects the mechanism, not the game meaning: `gStyleCueDistanceTable` (the same table `IsStyleCueNear`/`FindNearestStyleCueEntry`, code_8220_b.c/class_3bb8c_n.c, index with `dist < table[...]` -- confirming it holds distance thresholds) is read at a NEGATIVE tag index, divided down, and used as the divisor for `ctx->falloff`'s own value; `falloff` is used by every `StyleCueNN` occupant that calls this helper first. What the resulting scaled quantity represents in the running game (a cue repeat count? a duration?) is not established.
