> Renamed from `func_800280EC` on 2026-09-17 (tools/rename.py). Address 0x800280ec.

# ServiceCdDriver — MATCHED (49/49 words)

Round 45, runner echo (second sitting), `src/code_179d8_q.c`. Its address is
taken 3x elsewhere in the slice (this function itself, twice as a
`VSyncCallback` argument, once by `StartCdService`) — a function pointer.

## Result

Byte-exact, second attempt (one intermediate near-miss, see below).

```c
extern s32 func_80018458(void); /* code_8220_b */
extern s32 gCdUseVSyncCallback;
extern s32 D_8008A898;
extern void func_8002858C(void); /* code_179d8_r */
extern void func_800286E4(void); /* code_179d8_r */
extern s32 gCdQueueEnabled;
extern void VSyncCallback(void (*cb)(void));

/* This unit's own slot at +0x068 of D_8006D4E8's table (see GetClass6D4E8Methods's
 * class-map comment above); only the one slot this call site dispatches is
 * typed here, following the pad-to-offset convention include/code_171e0.h
 * uses for D_8006D430's own table. */
typedef struct D_8006D4E8Methods D_8006D4E8Methods;
struct D_8006D4E8Methods {
    u8 pad00[0x68];
    void (*slot68)(void);
};

s32 ServiceCdDriver(void)
{
    if (gCdLock != 0) {
        return 0;
    }

    if (func_80018458() != 0) {
        return 0;
    }

    if (gCdUseVSyncCallback != 0) {
        VSyncCallback(0);
    }

    if (D_8008A898 == 1) {
        func_8002858C();
    } else if (D_8008A898 == 2) {
        func_800286E4();
    }

    if (gCdQueueEnabled != 0) {
        ((D_8006D4E8Methods *)GetClass6D4E8Methods())->slot68();
    }

    if (gCdUseVSyncCallback != 0) {
        VSyncCallback((void (*)(void))ServiceCdDriver);
    }

    return 0;
}
```

## Derivation

Two early-return guards (the `gCdLock` latch, then `func_80018458()`
gp_rel getter from `code_8220_b`) both return literal `0` — **not** the
callee's own return value, even for the `func_80018458()` guard. This was
the one wrinkle: an intermediate attempt captured `func_80018458()`'s result
in a local and did `return result;`, reasoning that the branch target skips
straight to the epilogue with the call's return value still live in `$v0`
and therefore needing no extra move. That built clean (49/49 minus one word)
but was wrong — retail's delay slot for that `bnez` is `move v0,zero`, which
executes unconditionally (MIPS delay-slot semantics: it runs whether the
branch is taken or not) and is only semantically meaningful on the
branch-taken (early-return) path, where it overwrites the call's result
with 0 right before falling into the epilogue. So both guards are plain
`return 0;`, and the disassembly's `move v0,zero` in that slot is not
evidence of anything conditional — it is the same "if (cond) return 0;"
idiom as the first guard, just with the zeroing sharing a delay slot instead
of getting a fallthrough instruction of its own.

Body: an optional `VSyncCallback(0)` (`gCdUseVSyncCallback`), a two-way dispatch on
`D_8008A898` (1 -> `func_8002858C`, 2 -> `func_800286E4`, both in the
sibling `code_179d8_r` unit — declared extern here per the
per-call-site-typed convention `code_179d8_h.c` already established for
cross-unit libcd calls, now confirmed to apply to cross-unit game-code calls
too), an optional virtual dispatch through `D_8006D4E8`'s own table slot
+0x68 (guarded by `gCdQueueEnabled`), and finally an optional
self-re-registration as a `VSyncCallback` (its own address, cast — the
callback type is `void (*)(void)` and this function is typed `s32 (void)`
for its early-return-0 paths, so the cast is required and harmless: nothing
ever reads a return value through the callback).

### Proposed learning

**A `bnez`/`beqz` branch's delay-slot instruction runs on BOTH paths, and
when it sets a register to a constant right before the branch target's
epilogue, that is ordinary `if (cond) return CONST;` — not evidence the
constant depends on the branch direction.** Getting this backwards (deciding
the delay-slot zero must apply only to the not-taken path, so the taken path
should preserve the call's live return value) produces C that still
compiles and still looks plausible, and the resulting diff is a single
clean word, not a structural mismatch — cheap to miss on a skim of the
diff output.
