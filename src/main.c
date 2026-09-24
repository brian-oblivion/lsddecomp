#include "common.h"
#include "Class6D3C8.h"
#include "class_16334.h"

/* Local, opaque: code_8220.h can't be included alongside class_16334.h
 * (both define `struct BasicClassMethods`, per this project's
 * multiple-independent-local-views convention -- headercontention.py
 * confirms the two units' local views collide), and nothing here
 * dereferences a BMemPMgr, only passes the pointer through. */
typedef struct BMemPMgr BMemPMgr;

extern void func_80011994(void);

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
extern void *new_class_6c078(void);

extern BMemPMgr *D_8008A808;
extern Class6D3C8 *D_8008AC20;
extern Class6D3C8CtorArgs D_80066828;

/* Matched in code_1677c.c; not yet declared in any header (no other carved
 * caller existed until now). */
extern Class6D3C8 *New_Class6D3C8(Class6D3C8CtorArgs *arg);

void func_800118DC(void)
{
    void *obj;
    Pad *pad;

    func_80011994();
    SetMem(2);
    D_8008A808 = BMemPMgrInit(0x166C00, 0);
    SetDefaultBMemPMgr(D_8008A808);
    D_8008AC20 = New_Class6D3C8(&D_80066828);
    obj = new_class_6c078();
    pad = New_Pad(0, 0);
    D_8008AC20->methods->forwardToBaseSlot44UnlessFlagged(D_8008AC20, obj, pad);
    D_8008AC20->methods->slot4C(D_8008AC20);
}

void func_80011994(void) {
}
