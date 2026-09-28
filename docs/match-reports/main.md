# main (round 47)

> Renamed from `GameMain` on 2026-09-25 (tools/rename.py). Address 0x800118dc.

> Renamed from `func_800118DC` on 2026-09-25 (tools/rename.py). Address 0x800118dc.

**Unit:** main · **Size:** 46 instructions · **Status:** MATCHED (46/46 words, whole-image SHA1 OK)

## What it does

This is the game's own `main()`. It runs an empty startup stub
(`__main`, already matched, a no-op), sets a Psy-Q memory mode via
`SetMem(2)`, stands up the game's `BMemPMgr` heap (`BMemPMgrInit`), installs
it as the default pool (`SetDefaultBMemPMgr`), constructs the `GameApplication`
instance at `gGameApplication` (`New_GameApplication`, seeded from the constant block
`gGameApplicationConfig = {0x13, 0, 1, 1, 1, 1}`), allocates a second object via the
still-uncarved `New_DrawSystem`, opens a `Pad` (`New_Pad(NULL, 0)`),
and dispatches two methods through `gGameApplication`'s own vtable (`+0x044` and
`+0x04C`) before returning. It never loops -- the real game loop presumably
lives inside whatever `slot4C` (`Application__RunMainLoop`) or a callee reached from it
does; this function is just game-code setup, past which retail's own crt0
takes over `$ra`'s return (the function immediately after, un-carved and
correctly left untouched per the runner brief).

## The C

```c
#include "common.h"
#include "GameApplication.h"
#include "Pad.h"

/* Local, opaque: code_8220.h can't be included alongside Pad.h
 * (both define `struct BasicClassMethods`, per this project's
 * multiple-independent-local-views convention -- headercontention.py
 * confirms the two units' local views collide), and nothing here
 * dereferences a BMemPMgr, only passes the pointer through. */
typedef struct BMemPMgr BMemPMgr;

extern void __main(void);

/* Psy-Q libapi (`SetMem`, linked from `libapi/c159`, splat `o` segment).
 * One `s32` argument observed at this, its only call site. */
extern void SetMem(s32 mode);

/* BMemPMgrInit is fully matched in BMemPMgr.c as a single-argument
 * function (`s32 poolSize`, see docs/match-reports/BMemPMgrInit.md,
 * 31/31 words). THIS call site pushes a second, dead argument (0) that the
 * matched body never reads -- an unspecified-parameter declaration lets the
 * call carry it without contradicting the real prototype, the same idiom
 * code_8220.h already uses for BMemPMgrAlloc/BMemPMgrFree. */
extern void *BMemPMgrInit();

/* SetDefaultBMemPMgr(BMemPMgr *pool) -- one-line `gDefaultBMemPMgr = pool;`, matched
 * in code_8220.c but not yet declared in code_8220.h (no carved caller
 * existed until now). */
extern void SetDefaultBMemPMgr(BMemPMgr *pool);

/* Still asm (psyq_10ee0, game-code allocator, not yet carved). Zero
 * arguments -- its own asm never reads $a0/$a1, and the `addu $a0,$0,$0` /
 * `addu $a1,$0,$0` right after this call's `jal` are argument setup for the
 * NEXT call (`New_Pad(0, 0)`), not for this one. Its return IS used
 * here though: retail's delay slot for the *following* `jal` (`move
 * s0,v0`) captures it before that call can clobber v0 -- the return value
 * of THIS call, not of the one whose delay slot it sits in. */
extern void *New_DrawSystem(void);

extern BMemPMgr *gStartupBMemPMgr;
extern GameApplication *gGameApplication;
extern GameApplicationConfig gGameApplicationConfig;

/* Matched in GameApplicationFileResource.c; not yet declared in any header (no other carved
 * caller existed until now). */
extern GameApplication *New_GameApplication(GameApplicationConfig *arg);

void main(void)
{
    void *obj;
    Pad *pad;

    __main();
    SetMem(2);
    gStartupBMemPMgr = BMemPMgrInit(0x166C00, 0);
    SetDefaultBMemPMgr(gStartupBMemPMgr);
    gGameApplication = New_GameApplication(&gGameApplicationConfig);
    obj = New_DrawSystem();
    pad = New_Pad(0, 0);
    gGameApplication->methods->forwardToBaseSlot44UnlessFlagged(gGameApplication, obj, pad);
    gGameApplication->methods->slot4C(gGameApplication);
}

void __main(void) {
}
```

(Code block above updated round 79 to match `src/main.c` as it stands: the
`+0x044` slot is `forwardToBaseSlot44UnlessFlagged` in the live source, not
the `slot44` name this report's code block still carried from round 47 --
`GameApplicationMethods.forwardToBaseSlot44UnlessFlagged` was renamed by a later
round without this report's inline C being refreshed. No behavior changed;
this is a documentation sync only.)

## Naming (round 79)

- **`func_800118DC` -> `main`, tier A.** Evidence: `crt0` (Sony's,
  `config/splat.slps01556.lsdde.yaml`'s `main` c-segment comment: "main: the
  game's own `main()` (func_800118DC) plus a two-word stub") calls this
  function directly as the game's entry point; its body stands up the heap,
  constructs the root `GameApplication` object, and dispatches into its vtable --
  the shape of a C program's `main`. **Not named literally `main`**: doing
  so trips GCC 2.6.3's own special-case for a function spelled exactly
  `main` -- it silently inserts an extra `jal __main` as the function's
  first statement (`expand_main_function`, unconditional unless
  `-ffreestanding`, which this project's pinned flags do not pass). Verified
  directly: renaming to `main` compiled clean but failed to LINK
  (`undefined reference to '__main'`), because our C's own explicit
  `__main()` call sits ALONGSIDE the compiler's now-auto-inserted
  one, not in place of it. Reverted before committing; picked `main`
  instead, which is not special-cased. Image byte-identical either way for
  the body itself, since the byte match here has always come from the
  explicit call, not from a live `main`-triggered auto-insertion.

  **This is also new evidence for `__main`'s own identity, below.**

- **`__main` -- NOT renamed. Likely Sony's/the toolchain's own
  `__main`, not authored game code; flagged to the head, not renamed.**
  `tools/sdkname.py __main` returns only a TINY (2-word) EXACT
  fingerprint shared by a dozen unrelated library functions (`ClearOTag`,
  several `gte_*` macros, `SeVibOn`/`SetVib`, `KeyOnCheck`, `__nulldev`,
  and, notably, `__main _obj/none`) with no disambiguating position (both
  functions in this unit get the same generic "before `_obj/malloc`" note,
  which is not the "between two placed modules of one library" pattern
  `sdkname.py`'s own methodology needs for identification) -- exactly the
  shape `docs/PROGRESS.md` round 78 already swept for project-wide
  ("Adjacency leads ... find all five of round 78's hand finds plus
  SsStart/SsStart2, and no 2-word stub") and found nothing among 2-word
  stubs. So the corpus fingerprint ALONE is not enough, and the runner brief's
  screen (round 78's five TINY/AMBIGUOUS misses) says not to act on it alone.

  But the `main` rename experiment above produces a SECOND, independent,
  build-level signal that points the same way: `__main` is called as
  literally the FIRST statement inside `main`, at the exact position
  GCC 2.6.3 auto-inserts a call to a symbol named `__main` when the
  enclosing function is itself named `main` (unconditional under this
  project's flags, confirmed by reproducing the undefined-reference error
  above). The parsimonious reading is that retail's own source named this
  function `main` and let the compiler insert the call implicitly; our
  explicit `__main()` call is a source-level workaround that happens
  to reproduce the exact same bytes GCC would have emitted on its own,
  which is consistent with -- not proof against -- `__main` being
  the linked definition of `__main` that made that original build succeed
  (Sony's crt/libgcc-provided stub, the standard ritual symbol GCC's `main`
  special-case expects, not a Psy-Q library function with its own SDK
  module identity).

  **Not renamed, and no game name given, per the runner brief and
  CLAUDE.md's Sony-ownership rule** ("Never write C for a function a Sony
  object owns" / "give no game name to anything Sony owns") -- reclassifying
  it into `config/sdk-in-game.txt` or attempting to actually spell it
  `__main` (which would also mean pulling it out of this unit and
  potentially restructuring the call as an auto-inserted one) is a bigger,
  riskier structural change than a naming pass should make unilaterally,
  and is left for the head/operator to decide. Posted to
  `tools/broadcast.sh`.

- **Globals, all three named (this unit is their only C reference site):**
  - `D_8008A808` -> `gStartupBMemPMgr`, tier B. Holds `BMemPMgrInit`'s
    return value between that call and the immediately following
    `SetDefaultBMemPMgr(...)` call -- set once, read once, both in
    `main`. Mechanics are clear (it stages the newly created heap
    pointer); whether any still-uncarved code elsewhere also reads this
    exact global (as opposed to `gDefaultBMemPMgr`, a different address,
    `BMemPMgr.c`) is not established, hence tier B rather than A.
  - `D_8008AC20` -> `gGameApplication`, tier B. The one instance of `GameApplication`
    the game constructs, matching this header's own stated convention of
    keeping the class's identity tied to its vtable address until a
    game-purpose name is established (track 4). Read and written only here
    and in the shared header's comments (updated by this rename).
  - `D_80066828` -> `gGameApplicationConfig`, tier A: purely mechanical, it
    IS the one `GameApplicationConfig` block in the image, passed to
    `New_GameApplication` at its only call site. `{0x13, 0, 1, 1, 1, 1}`, per
    `include/GameApplication.h`'s existing documentation of which two fields
    (`+0x00`, `+0x14`) `GameApplication__GameApplication` actually reads.

  All three: `./build-and-verify.sh` byte-identical, `tools/check-nonmatching.sh`
  green, after each individual `rename.py` run.

### Proposed learning

**A function named exactly `main` is not a safe rename target under this
project's pinned GCC 2.6.3 flags, even when the source shape is otherwise
exactly a C program's entry point.** GCC's `main`-special-case
unconditionally inserts a `jal __main` as that function's first compiled
statement (unless `-ffreestanding`, which this project does not pass), so
renaming a matched, byte-exact function to literally `main` adds an
UNDEFINED reference at link time regardless of what the function's own C
body already does -- the failure is a link error, not a compile error, and
it says nothing about whether the rename is otherwise correct. This is also
a positive identification tool: a function that a `main`-rename experiment
demands a `__main` definition for, at the exact call site an existing
matched callee already occupies, is independent (build-level, not corpus-
fingerprint) evidence that the callee IS that toolchain-standard symbol.
Use a non-colliding name (`main`, `EntryMain`, etc.) for the game's
real entry point instead.

Also in `include/GameApplication.h`: `GameApplicationMethods.unk4C` (never dispatched
by any carved C before this) is retyped from `void *` to `void
(*slot4C)(GameApplication *self);` -- this function is its first caller.

## Two levers, one of them a straight misreading corrected by re-deriving from the encoding

1. **`BMemPMgrInit(0x166C00, 0)` -- a genuinely dead second argument.**
   `BMemPMgrInit` is already matched elsewhere as strictly one-argument
   (`docs/match-reports/BMemPMgrInit.md`, 31/31), and its body never reads
   a second parameter. But THIS call site's own delay slot sets `$a1 = 0`
   regardless -- `addu $a1,$zero,$zero` right after the `jal`. An
   unspecified-parameter (`void *BMemPMgrInit();`) local declaration lets
   the call push two arguments without contradicting the real, ANSI,
   single-argument definition seen from `BMemPMgr.c`'s own translation
   unit -- the exact idiom `code_8220.h` already documents for
   `BMemPMgrAlloc`/`BMemPMgrFree`. First guess (passing only
   `BMemPMgrInit(0x166C00)`) simply omitted the dead `$a1` instruction and
   the whole build stayed silently one word short there; adding the extra
   literal `0` argument reproduced the instruction exactly.

2. **The delay slot after a `jal` belongs to the PREVIOUS value in that
   register, not to the call it sits beside -- and getting this backwards
   cost the whole match until it was caught.** `2138: jal New_Pad` /
   `213c: move $s0,$v0` looks, read casually, like "save this call's
   return into `$s0`". It is not: the delay slot instruction executes
   architecturally *before* the jump is taken, so `$v0` at that point is
   still whatever it held *before* `New_Pad` ran -- i.e. the return
   of the PRECEDING call, `New_DrawSystem()`. `New_Pad`'s own
   return only becomes available in `$v0` *after* it returns, several
   instructions later (`2158: jalr $v1` / `215c: move $a2,$v0`). First
   draft read both `$a1` and `$a2` at the `slot44` dispatch as the SAME
   value (`New_Pad`'s return, used twice) purely because they carry
   the same hex bytes in this one build -- the actual instruction ordering
   (an entirely separate `move a0,zero`/`move a1,zero` pair for
   `New_Pad`'s own two arguments intervening between the `jal
   New_DrawSystem` and the `jal New_Pad`) never lines up under
   that reading, and no amount of restructuring `pad`'s C (separate
   locals, `register`, an unprototyped `slot44` field) moved the
   registers even one bit -- because the C was describing the wrong
   value's lifetime, not the wrong shape. Reassigning `$s0` to
   `New_DrawSystem()`'s return (crossing the real call boundary,
   `New_Pad`) and `$a2`/`pad` to `New_Pad`'s own return
   (never crossing a call after its own) closed to 46/46 on the next
   build, with zero further changes.

## Proposed learning

**A `move $reg_n, $v0` sitting in a `jal`'s OWN delay slot reads the value
`$v0` held BEFORE that call, never the call's own return** -- the delay
slot executes before the jump is taken. When two calls sit close together
and a delay-slot move looks like it's "saving this call's result", check
whether it's actually salvaging the PRECEDING call's still-live return
before this call clobbers it. Getting this backwards produces a plausible
looking C body (right call targets, right argument count, right-looking
duplicate-value dispatch) that no amount of local-variable restructuring
can close, because the mismatch is about WHICH VALUE'S LIFETIME crosses
the call boundary, not about how that value's C expression is shaped. Round
46's session (bravo, alpha) already established the general "value crossing
a CALL boundary needs its own local" pattern; this is the sharper corollary
for reading which call the crossing value belongs to in the first place.

## Head note, round 79: the entry point is `main`, the stub is `__main`

Alpha found that spelling the entry point `main` makes cc1 insert
`jal __main`, which failed to link. The head measured the conclusion in a
worktree. With `func_80011994` renamed `__main`, `GameMain` renamed `main`,
and `main`'s explicit first-statement call to the stub DELETED, the image is
byte-identical: retail's first call is the one cc1 inserts itself, so the
original source spelled it `main` and never wrote the call. `__main` is
Sony's empty `_obj/none` stub: an EXACT (TINY) fingerprint, placed directly
before the `_obj/malloc` object, and the call only cc1 emits. Both oracles
green. `GameMain` is withdrawn: the name was a workaround for the link
failure, not a claim about the code.

## Track 4 (2026-09-26, round 87, bravo)

The local GetDrawSystem/New_DrawSystem extern this unit carried is gone; it comes from `include/DrawSystem.h` (gDrawSystemMethods unified), with a pointer cast where this unit's own slot type asks for one. Byte-identical.

## History (moved from src/main.c, round 101)

Track 7 (alpha) rewrote the unit's comments to describe the code only. What
they recorded, verbatim as they stood at `57fb90b6`:

The unit banner:

```c
/*
 * The game's entry point. `main` (formerly `func_800118DC`) is called
 * directly by Sony's `crt0` (config/splat.slps01556.lsdde.yaml, the `main`
 * c-segment) and runs once: it sets the Psy-Q memory mode, stands up the
 * game's `BMemPMgr` heap, constructs the root `GameApplication` object from a
 * fixed ctor-args block, opens a `Pad`, and dispatches into the root
 * object's own vtable (+0x044, +0x04C) before returning. It never loops --
 * the real game loop lives inside whatever the dispatched vtable slots (or
 * a callee reached from them) do.
 *
 * The first thing `main` calls is `__main`, and the C does not write that
 * call: cc1 inserts `jal __main` at the top of any function spelled `main`,
 * and retail's first instruction pair is exactly that call (round 79: the
 * explicit call removed, the image stays byte-identical). `__main` is
 * Sony's empty stub from the SDK's `_obj/none` module (see its symbols
 * entry). It lives here as matched C only because no object places it.
 */
```

The declarations block (the opaque typedef it opens is gone: `BMemPMgr`
now comes from `include/BMemPMgr.h`):

```c
/* Local, opaque: nothing here dereferences a BMemPMgr (BMemPMgr.h), it
 * only passes the pointer through. */
typedef struct BMemPMgr BMemPMgr;

/* Psy-Q libapi (`SetMem`, linked from `libapi/c159`, splat `o` segment).
 * One `s32` argument observed at this, its only call site. */
extern void SetMem(s32 mode);

/* BMemPMgrInit is fully matched in BMemPMgr.c as a single-argument
 * function (`s32 poolSize`, see docs/match-reports/BMemPMgrInit.md,
 * 31/31 words). THIS call site pushes a second, dead argument (0) that the
 * matched body never reads -- an unspecified-parameter declaration lets the
 * call carry it without contradicting the real prototype, the same idiom
 * BMemPMgr.h already uses for BMemPMgrAlloc/BMemPMgrFree. */
extern void *BMemPMgrInit(); /* arity-ok: the dead 2nd argument IS byte-load-bearing here -- retail emits `move a1,zero` in the jal's delay slot at 0x80011900 */

/* SetDefaultBMemPMgr(BMemPMgr *pool) -- one-line `gDefaultBMemPMgr = pool;`, matched
 * in BMemPMgr.c but not yet declared in BMemPMgr.h (no carved caller
 * existed until now). */
extern void SetDefaultBMemPMgr(BMemPMgr *pool);

/* New_DrawSystem comes from include/DrawSystem.h (through GameApplication.h). */
```

Above the `initSystems` call:

```c
    /* GameApplication__InitSystems takes no 4th argument: include/GameApplication.h. */
```

The `arity-ok` line's retail evidence: the dead second argument of
`BMemPMgrInit` is the `move a1,zero` in the `jal`'s delay slot at
0x80011900 (see "Two levers" above).

## Naming (round 101, track 7, alpha)

| old | new | tier | evidence |
| --- | --- | --- | --- |
| local `obj` | `drawSystem` | A | it holds `New_DrawSystem()`'s return and is passed as `initSystems`'s `DrawSystem *drawSystem` |
| `2` (SetMem) | `CONSOLE_RAM_MB` | A | Psy-Q libapi's `SetMem(n)` takes the RAM size in megabytes, 2 on a retail console, 8 on a development board |
| `0x166C00` | `DEFAULT_POOL_SIZE` = `(1435 * 1024)` | A | `BMemPMgrInit(s32 poolSize)` (src/app/BMemPMgr.c) stores it as `pool->poolSize`; the pool is installed by `SetDefaultBMemPMgr`, whose only caller is `main`, as the `gDefaultBMemPMgr` every `BMemPMgrAlloc` uses |

`New_Pad(0, 0)` keeps its literals with a comment: they are `PadInit`'s mode
and the port, and a name would restate them.

Two constructs carry `/* MATCHING: */` lines. The unread `0` passed to
`BMemPMgrInit` is retail's `move $a1, $zero` ("Two levers", 1). The
`GameApplicationInitSystemsFn` cast is measured: calling the slot through its
own four-parameter type with a trailing `0` compiles to 41/46 words
(`funcdiff.py main`, reverted), so the cast is what matches.
