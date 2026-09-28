# RegisterStyleConfig -- MATCHED, round 44 (2026-09-15)

> Renamed from `func_800544E4` on 2026-09-23 (tools/rename.py). Address 0x800544e4.

Unit `ObjMStyleActor`. **29/29 words, byte-exact.** Reopened, never attempted
before this round.

## What it does

Register-once initializer: if `gStyleGrid` (a `.sdata` flag word, zero at
boot) is already set, return 0. Otherwise stash the 5 arguments (4 in
registers, 1 on the stack at `0x28($sp)`) into a scatter of `.sbss` globals,
zero two adjacent words (`D_8008ACA0`, and `sStyleCueSlots` immediately below it
by pointer decrement), then tail-call `ApplyStyleConfig()` and return its
result.

```c
extern s32 gStyleGrid;
extern s32 gStyleStage;
extern s32 gStyleTickCount;
extern s32 sStyleDay;
extern s32 sStyleUnreadArg;
extern s32 gStyleSceneRefs;
extern s32 gStyleVariant;
extern s32 D_8008ACA0;

extern s32 ApplyStyleConfig(void);

s32 RegisterStyleConfig(s32 a0, s32 a1, s32 a2, s32 a3, s32 arg4) {
    s32 *p;
    s32 i;

    if (gStyleGrid == 0) {
        i = 1;
        p = &D_8008ACA0;
        gStyleGrid = a0;
        gStyleStage = a1;
        gStyleSceneRefs = a2;
        gStyleVariant = -1;
        sStyleDay = a3;
        sStyleUnreadArg = arg4;
        gStyleTickCount = 0;
        do {
            *p = 0;
            i--;
            p--;
        } while (i >= 0);
        return ApplyStyleConfig();
    }
    return 0;
}
```

## Two levers, both worth generalizing

1. **The 2-word zero loop is NOT two scalar assignments.** Writing
   `D_8008ACA0 = 0; sStyleCueSlots = 0;` directly compiles to two `gp_rel` stores
   (both symbols are `.sbss`, both are in `config/gp-symbols.txt`) -- a
   completely different, shorter instruction sequence than retail's
   absolute-address `lui`/`addiu` pointer with a decrementing `do`/`while`
   loop. `tools/m2ctx.py --run` was decisive here: it recovers the loop shape
   (`s32 *p = &D_8008ACA0; do { *p = 0; i--; p--; } while (i >= 0);`)
   directly from the asm, which a from-scratch reading of two adjacent `sw`s
   would not suggest. **`sStyleCueSlots` is never named in the C** -- it is
   reached purely by decrementing `p` from `&D_8008ACA0`, relying on the
   linker's real (fixed, splat-derived) placement of the two words adjacent
   in `.sbss`.
2. **Guard-clause direction changes codegen shape, not just readability.**
   `if (flag != 0) return 0;` followed by the main body compiles to an
   inline early-return block (`j`+`move v0,zero`) spliced in BEFORE the main
   body, with a jump over it -- not what retail does. Retail's `bnez` skips
   straight to the shared epilogue at the very end, meaning the source was
   `if (flag == 0) { <body>; return f(); } return 0;`, not the intuitively
   "cleaner" early-return-first form. Inverting the guard was the second and
   final fix needed to reach byte-exact.

### Proposed learning

When a stub touches a run of `.sbss`/`.sdata` globals plus a short zeroing
loop over two or more of them, seed with `tools/m2ctx.py --sig ... --run`
before hand-writing anything -- the pointer-decrement loop shape it recovers
is not the same code a straightforward per-field-assignment reading would
produce, and the difference changes the instruction count.

## Naming

**RegisterStyleConfig** -- tier B. Free function (VerbNoun, unrelated to the `ObjM`/`DreamSys` cluster above): a register-once guard over a `.sdata` flag word (`gStyleGrid`), stashing five style-config arguments into `.sbss` globals and zeroing two more before tail-calling `ApplyStyleConfig`. Mechanically well understood from the disassembly; "style" describes the data it touches (color/config table consumed by `FillStyleFromConfig`/`StyleM`), not a confirmed game concept.

## Track 7 (2026-09-27, round 98, delta)

- The zeroing loop no longer reaches `sStyleCueSlots` through
  `D_8008ACA0`, which is only splat's label for the SECOND of its two
  slots: `extern void *sStyleCueSlots[2]` and
  `slot = &sStyleCueSlots[ARRAY_COUNT(sStyleCueSlots) - 1]` link to the
  same address, and the loop shape (the lever above) is kept. Byte-exact.
- Parameters `a0..a3, arg4` are `grid, stage, sceneRefs, day, unreadArg`
  (ObjM__InitStyleAndWorld passes the StageMap, its stage, &ctorSound and
  the day; ObjMStyleActor's prototype names the first four the same way).
- `D_8008AC78` is `sStyleUnreadArg` (tools/rename.py), tier B: this
  function stores its fifth argument there, and nothing in the image
  reads it (no reference in any `asm/` file or `src/` unit, measured).
  The one caller passes 0.
- The result is cast, `(s32)ApplyStyleConfig()`; that removed the
  unit's "return makes integer from pointer" warning (typeviews baseline
  rewritten in the same commit).
