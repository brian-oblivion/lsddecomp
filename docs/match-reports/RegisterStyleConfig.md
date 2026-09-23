# RegisterStyleConfig -- MATCHED, round 44 (2026-09-15)

> Renamed from `func_800544E4` on 2026-09-23 (tools/rename.py). Address 0x800544e4.

Unit `class_3bb8c_m`. **29/29 words, byte-exact.** Reopened, never attempted
before this round.

## What it does

Register-once initializer: if `D_8008AB4C` (a `.sdata` flag word, zero at
boot) is already set, return 0. Otherwise stash the 5 arguments (4 in
registers, 1 on the stack at `0x28($sp)`) into a scatter of `.sbss` globals,
zero two adjacent words (`D_8008ACA0`, and `D_8008AC9C` immediately below it
by pointer decrement), then tail-call `ApplyStyleConfig()` and return its
result.

```c
extern s32 D_8008AB4C;
extern s32 D_8008AC6C;
extern s32 D_8008AC70;
extern s32 D_8008AC74;
extern s32 D_8008AC78;
extern s32 D_8008AC7C;
extern s32 D_8008AC80;
extern s32 D_8008ACA0;

extern s32 ApplyStyleConfig(void);

s32 RegisterStyleConfig(s32 a0, s32 a1, s32 a2, s32 a3, s32 arg4) {
    s32 *p;
    s32 i;

    if (D_8008AB4C == 0) {
        i = 1;
        p = &D_8008ACA0;
        D_8008AB4C = a0;
        D_8008AC6C = a1;
        D_8008AC7C = a2;
        D_8008AC80 = -1;
        D_8008AC74 = a3;
        D_8008AC78 = arg4;
        D_8008AC70 = 0;
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
   `D_8008ACA0 = 0; D_8008AC9C = 0;` directly compiles to two `gp_rel` stores
   (both symbols are `.sbss`, both are in `config/gp-symbols.txt`) -- a
   completely different, shorter instruction sequence than retail's
   absolute-address `lui`/`addiu` pointer with a decrementing `do`/`while`
   loop. `tools/m2ctx.py --run` was decisive here: it recovers the loop shape
   (`s32 *p = &D_8008ACA0; do { *p = 0; i--; p--; } while (i >= 0);`)
   directly from the asm, which a from-scratch reading of two adjacent `sw`s
   would not suggest. **`D_8008AC9C` is never named in the C** -- it is
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

**RegisterStyleConfig** -- tier B. Free function (VerbNoun, unrelated to the `ObjM`/`DreamSys` cluster above): a register-once guard over a `.sdata` flag word (`D_8008AB4C`), stashing five style-config arguments into `.sbss` globals and zeroing two more before tail-calling `ApplyStyleConfig`. Mechanically well understood from the disassembly; "style" describes the data it touches (color/config table consumed by `FillStyleFromConfig`/`StyleM`), not a confirmed game concept.
