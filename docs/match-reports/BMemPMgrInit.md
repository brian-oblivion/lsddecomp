# BMemPMgrInit

**Unit:** code_8220 · **Size:** 31 instructions · **Status:** MATCHED (31/31 words)

## What it does

Creates a `BMemPMgr` memory pool: clamps `poolSize` up to a minimum of
`0x400`, allocates `poolSize + 0x20` bytes from the game's generic heap
(`func_80011D34`, Psy-Q SDK, `the 0x2258..0x8220 Psy-Q block (now linked from lib/, formerly asm/psyq_2258.s)`), and if that succeeds,
initializes the pool header (`freeListHead = pool + 0x1C`, `poolSize`) and
hands off to `SetupBMemPMgrFreeList` (gp_rel-blocked, see its own report) to build
the initial single free block covering the rest of the allocation. On
allocation failure, logs an error via the Psy-Q printf wrapper
(`func_80012C20`) using the rodata format string at `D_8001028C`
(`"bMemPMgr = %p, poolSize = %ld in BMemPMgrInit\n"`) and returns `NULL`.

This confirms the unit's premise from the runner prompt: the format string
IS this function's own diagnostic, and `code_8220` genuinely is a
memory-pool-manager-plus-base-class block.

## The C

```c
void *BMemPMgrInit(s32 poolSize)
{
    BMemPMgr *pool;

    if ((u32)poolSize < 0x400) {
        poolSize = 0x400;
    }
    pool = func_80011D34(poolSize + 0x20);
    if (pool != NULL) {
        pool->freeListHead = (u8 *)pool + 0x1C;
        pool->poolSize = poolSize;
        SetupBMemPMgrFreeList(pool);
    } else {
        func_80012C20(D_8001028C, NULL, poolSize);
    }
    return pool;
}
```

## Three levers, each confirmed by objdump before moving to the next

1. **Unsigned comparison.** `poolSize < 0x400` alone compiles to `slti`
   (signed); retail uses `sltiu`. Cast the LHS: `(u32)poolSize < 0x400`.
   Makes sense — a pool byte count is never meaningfully negative, and
   retail treats it as unsigned here even though the printf format string
   later reads it back with `%ld` (a *different*, signed, view of the same
   bit pattern for display purposes only).

2. **`SetupBMemPMgrFreeList` is genuinely one argument, not two.** The natural
   first draft passed `poolSize` as a second argument (a plausible guess —
   `BMemPMgrInit` clearly wants `SetupBMemPMgrFreeList` to know the pool size).
   That compiled to a real function, one word too long, with a spurious
   `move $a1,$s1` immediately before the `jal`. Checking `SetupBMemPMgrFreeList`'s
   own disassembly (`asm/nonmatchings/code_8220/SetupBMemPMgrFreeList.s`) shows why:
   its own `$a1` is read as a *fallback* pool pointer (`bnez $a1,
   .L80017ADC; ori $v0,$zero,0x1; addu $a1,$a0,$zero` — defaults to `$a0`,
   i.e. self, when the global default pool `gDefaultBMemPMgr` is unset), and
   `BMemPMgrInit`'s call site never sets `$a1` before the `jal` — it is a
   genuinely-uninitialized, forwarded register, the same shape
   DECOMPILATION_LEARNINGS documents as "an unused parameter in the callee
   shows up in the caller as a genuinely uninitialised local." Declaring
   `SetupBMemPMgrFreeList` as `void SetupBMemPMgrFreeList(BMemPMgr *pool)` (one argument)
   and calling it that way removed the extra `move` and closed this to the
   word.

3. **The alloc-failure return needed the single-shared-return SHAPE, not
   just the right guard polarity.** Two structurally-plausible forms were
   tried:
   - Early-return guard: `if (pool == NULL) { log(); return pool; }
     init-success-fields; return pool;` — got branch polarity right
     (`beqz`, matching retail) and the right total size (31/31 words), but
     the LAST word differed: retail computes the failure path's return
     value with `addu $v0,$s0,$zero` (reusing the register that already
     holds the now-proven-zero pool pointer) where this form compiled to
     `li $v0,0x0` (GCC materializing the known-zero value fresh, rather
     than reusing the dead register, once the return sits in its own
     block dominated by the null check). 30/31.
   - **Single shared return** (the form above, `if (pool != NULL) {
     success} else {log}`, one `return pool;` at the very end reached from
     both branches) — 31/31, first try. Apparently keeping BOTH paths'
     final value-materialization physically adjacent to the SAME `return`
     statement (rather than two textually-separate `return pool;`
     statements, one per branch) is what keeps GCC 2.6.3 from treating the
     failure path's `pool` as a provably-dead-and-foldable constant. Filed
     under `### Proposed learning` below since this is the opposite
     prescription from the project's usual "invert the guard, avoid
     `if`/`else` for a big body" advice (DECOMPILATION_LEARNINGS' "Write a
     small early exit as an inverted guard clause") — that advice is about
     LAYOUT/branch-polarity, not about return-value materialization, and
     the two considerations pointed opposite ways here.

## Proposed learning

**A null-checked pointer's `return` in the FAILURE branch can fold to a
fresh `li $v0,0` instead of reusing the now-known-zero register — and
whether it does depends on whether the return is its own textually-separate
statement or shares a single `return` reached from both branches.** Two
`if(x==NULL){...; return x;} ...; return x;` (two return sites) folded the
failure one to a constant, one word off from retail's register-reuse form;
`if(x!=NULL){...} else {...} return x;` (one shared return site) did not
fold and matched byte-exact. Both forms have identical branch polarity and
identical total instruction count up to this one word — only the
materialization of the already-dead-obviously-zero value differs. Worth
checking on any other `New_X`/pool-init-shaped function whose failure path
returns the just-nulled pointer.

## Provenance

round 11 (2026-09-03), runner delta, unit code_8220 (fresh carve, first
function).

## Extern arity (round 59)

**Verdict: arity-ok idiom.** `src/main.c`'s unprototyped declaration stays.

**Callee evidence** (`0x80017A20`): the body does `move s1,a0` and never touches
`$a1` on any path — the definition in `src/code_8220.c`
(`void *BMemPMgrInit(s32 poolSize)`) is right, one argument.

**Why the extern must keep saying nothing.** `func_800118DC` (src/main.c) calls
it as `BMemPMgrInit(0x166C00, 0)`, and that second argument is *byte-load-bearing*:
retail emits it.

```
800118f4:  lui   a0,0x16
800118f8:  ori   a0,a0,0x6c00
800118fc:  jal   80017a20 <BMemPMgrInit>
80011900:  move  a1,zero          <- the dead 2nd argument, in retail
```

Replacing `extern void *BMemPMgrInit();` with the real one-parameter prototype
would make that call a `too many arguments` compile error, and dropping the
argument from the call site would delete `move a1,zero` and break the match.
The unprototyped form is the only spelling that reproduces retail, and it is
the same idiom `include/code_8220.h` uses for `BMemPMgrAlloc`/`BMemPMgrFree`.

**Declaration sites changed:** none (arity unchanged). `/* arity-ok: ... */`
added to `src/main.c:24`. Oracle green after the edit.
