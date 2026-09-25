/*
 * The game's entry point. `main` (formerly `func_800118DC`) is called
 * directly by Sony's `crt0` (config/splat.slps01556.lsdde.yaml, the `main`
 * c-segment) and runs once: it sets the Psy-Q memory mode, stands up the
 * game's `BMemPMgr` heap, constructs the root `Class6D3C8` object from a
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
#include "common.h"
#include "Class6D3C8.h"
#include "class_16334.h"

/* Local, opaque: nothing here dereferences a BMemPMgr (code_8220.h), it
 * only passes the pointer through. */
typedef struct BMemPMgr BMemPMgr;

/* Psy-Q libapi (`SetMem`, linked from `libapi/c159`, splat `o` segment).
 * One `s32` argument observed at this, its only call site. */
extern void SetMem(s32 mode);

/* BMemPMgrInit is fully matched in code_8220.c as a single-argument
 * function (`s32 poolSize`, see docs/match-reports/BMemPMgrInit.md,
 * 31/31 words). THIS call site pushes a second, dead argument (0) that the
 * matched body never reads -- an unspecified-parameter declaration lets the
 * call carry it without contradicting the real prototype, the same idiom
 * code_8220.h already uses for BMemPMgrAlloc/BMemPMgrFree. */
extern void *BMemPMgrInit(); /* arity-ok: the dead 2nd argument IS byte-load-bearing here -- retail emits `move a1,zero` in the jal's delay slot at 0x80011900 */

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
extern Class6D3C8 *gClass6D3C8;
extern Class6D3C8CtorArgs gClass6D3C8CtorArgs;

/* Matched in code_1677c.c; not yet declared in any header (no other carved
 * caller existed until now). */
extern Class6D3C8 *New_Class6D3C8(Class6D3C8CtorArgs *arg);

void main(void)
{
    void *obj;
    Pad *pad;

    SetMem(2);
    gStartupBMemPMgr = BMemPMgrInit(0x166C00, 0);
    SetDefaultBMemPMgr(gStartupBMemPMgr);
    gClass6D3C8 = New_Class6D3C8(&gClass6D3C8CtorArgs);
    obj = New_DrawSystem();
    pad = New_Pad(0, 0);
    gClass6D3C8->methods->forwardToBaseSlot44UnlessFlagged(gClass6D3C8, obj, pad);
    gClass6D3C8->methods->slot4C(gClass6D3C8);
}

/* Sony's _obj/none (round 79); the call to it is cc1's, inside main. */
void __main(void) {
}
