/* code_2cc8c -- first 20-function slice of the 0x2CC8C block (153 functions
 * total; the remainder is the code_2cc8c_b asm segment).
 *
 * Carved in round 10 on the belief that this block had the LOWEST
 * toolchain-blocker density of any uncarved segment. That belief was
 * retracted the same round for two of the 20 -- `Obj86B60__OnTag2Notify` and
 * `Obj86B60__SetState`, whose jump-table dispatch hits
 * `addiu $at, $at, %lo(jtbl_*)`. The retraction was RIGHT: a `jtbl_*` symbol
 * is not a safe exception to that screen, because cc1 emits the same generic
 * pseudo-op for an indexed data global and a switch jump table and the fold
 * happens in maspsx, below cc1, which cannot tell them apart.
 *
 * BOTH OF THOSE ARE NOW MATCHED (round 23). The analysis above stands; only
 * the VERDICT expired, because round 21 resolved `addiu_at` itself -- maspsx
 * gained a `--addiu-at` flag that emits retail's unfolded four-instruction
 * indexed form directly (docs/research/addiu-at-blocker.md). `addiu_at` is
 * no longer a blocker anywhere; do not screen for it and do not file a stall
 * against it -- and as of round 63 (CLAUDE.md, "Open toolchain blockers")
 * there are no open toolchain blockers of any kind left in this project.
 *
 * The history is kept rather than deleted because it is this unit that
 * established the jtbl-is-not-an-exception discriminator, and that finding
 * outlived the blocker it was about.
 *
 * Shape: this is class-framework code. Objects carry their method table at
 * offset 0 (`lw $v1, 0x0($a0)` then `lw $v0, 0xNN($v1)` then `jalr`), so
 * resolve slots with tools/classtable.py rather than by counting. The
 * struct is `Obj86B60` (include/code_2cc8c.h). Every function in this file
 * is the DEFAULT implementation of its slot in `gTaskCoreMethods` (72 slots,
 * `Get_vtable_TaskCore()`), the real base table for this whole class family
 * -- NOT `gClass86B60Methods` (78 slots) or `gGraphRoomMethods` (73 slots),
 * which are two independent, sibling DERIVED tables that inherit most of
 * this unit's functions unmodified and override a few (`Class86B60`
 * overrides SetState/Tick/RefreshViewValue; see the header's own top
 * comment for the round-78 correction and the evidence). The first two
 * functions (`Obj86B60__OnTag2Notify`/`Obj86B60__OnTag5Notify`) are
 * `EventArg`-tag dispatchers reached from `IntermediateBase__OnNotify`
 * (code_2cc8c_c.c); the five `Obj86B60__func_8003Cxxx` handlers they
 * dispatch to are undifferentiated leaf state-transition helpers (tier C --
 * see each one's own match report); `Obj86B60__SetState` is the base
 * `reason`-coded state-transition entry point (slot60); `Obj86B60__Tick`/
 * `Obj86B60__RefreshViewValue` are per-frame slots (90/94); the rest are
 * small setters/getters around a `frameCounter`+`activeSlot` ring-buffer
 * bookkeeping scheme and a "fade" pair (`Obj86B60__SetFadeRate`,
 * `Obj86B60__TickColorFade`, gated through `Obj86B60__SetFadeCallbackEnabled`/
 * `Obj86B60__TickFadeCallback`) that computes a running RGB value from
 * `frameCounter * unk84` against a base colour.
 *
 * Round 78 (delta): full track-3 naming pass. All 20 functions were already
 * matched (rounds 10-23); this round named every one via `tools/rename.py`
 * and corrected a pre-existing error in the header's slot74..slot84
 * occupant mapping (it had the five message handlers reversed -- see
 * include/code_2cc8c.h's own comment on that struct field). See each
 * function's own match report's `## Naming` section for evidence.
 */

#include "common.h"
#include "code_2cc8c.h"

void Obj86B60__OnTag2Notify(Obj86B60 *self, s32 a1, s32 a2)
{
    Obj86B60Methods *methods;

    methods = self->methods;
    if (self->unk3C != 0) {
        switch (a2) {
        case 0x12:
            methods->slot80(self, a1);
            break;
        case 0x13:
            methods->slot84(self, a1);
            break;
        case 0x21:
            methods->slot74(self, a1);
            break;
        case 0x17:
            methods->slot7C(self, a1);
            break;
        case 0x19:
            methods->slot78(self, a1);
            break;
        }
    }
}

void Obj86B60__OnTag5Notify(Obj86B60 *self, s32 a1, s32 a2)
{
    Obj86B60Methods *methods;

    methods = self->methods;
    Get_vtable_IntermediateBase()->update((IntermediateBase *)self, (BasicClass *)a1, a2);
    if (self->unk3C != 0) {
        u32 bound;

        bound = self->frameCounter;
        if ((u32)self->unk40 < bound) {
            methods->slot60(self, 6);
        }
    }
    switch (self->unk20) {
    case 2:
        methods->slot60(self, 4);
        break;
    case 4:
        methods->slotAC(self);
        break;
    case 7:
        methods->slotC0(self);
        break;
    case 8:
        methods->slot60(self, 3);
        break;
    }
}

void Obj86B60__SetState(Obj86B60 *self, s32 a1)
{
    Obj86B60Methods *methods;

    methods = self->methods;
    Get_vtable_IntermediateBase()->setState((IntermediateBase *)self, a1);
    switch (a1) {
    case 5:
        methods->slotE4(self, self->target->unselectedColor);
        methods->slotF0(self, self->target->unk8, 0);
        self->frameCounter = 0;
        self->unk3C = 1;
        break;
    case 6:
        self->unk38 = 1;
        methods->slot94(self);
        break;
    case 4:
    case 7:
        self->frameCounter = 0;
        self->unk3C = 0;
        break;
    case 8:
        self->frameCounter = 0;
        break;
    case 9:
    case 0xA:
    case 0xB:
    case 0xE:
    case 0xF:
    case 0x10:
    case 0x11:
        self->unk20 = 5;
        self->frameCounter = 0;
        switch (a1) {
        case 0xB:
            methods->slot90(self);
            break;
        case 0xF:
            methods->slot10C(self);
            break;
        case 0x11:
            methods->slot110(self);
            break;
        }
        break;
    }
}

void Obj86B60__SetFrameBound(Obj86B60 *self, s32 a1)
{
    self->unk40 = a1;
    if (a1 >= 0) {
        self->unk40 = a1 * 20;
    }
}

void Obj86B60__ForwardToChild(Obj86B60 *self, s32 a1)
{
    Unk48Obj *child;

    child = self->unk48;
    if (child != NULL) {
        child->methods->slot80(child, a1, 0x60, 0x60);
    }
}

void Obj86B60__func_8003C7F4(Obj86B60 *self, s32 a1)
{
    if (self->target != NULL) {
        self->methods->slot70(self, 0x10);
        self->methods->slot60(self, 0xA);
    }
}

void Obj86B60__func_8003C858(Obj86B60 *self, s32 a1)
{
    s32 reason;

    if (self->target != NULL) {
        self->methods->slot70(self, 0x10);
        reason = 0xF;
        if (self->unk3C == 1) {
            reason = 0xB;
        }
        self->methods->slot60(self, reason);
    }
}

void Obj86B60__func_8003C8D0(Obj86B60 *self, s32 a1)
{
    if (self->target != NULL && self->unk3C != 1) {
        self->methods->slot70(self, 0x10);
        self->methods->slot60(self, 0x11);
    }
}

void Obj86B60__func_8003C944(Obj86B60 *self, s32 a1)
{
    void (*handler)(Obj86B60 *self);

    if (self->target == NULL) {
        return;
    }
    if (self->unk3C == 1) {
        handler = self->methods->slotEC;
    } else if (self->unk3C == 2) {
        handler = self->methods->slot118;
    } else {
        return;
    }
    handler(self);
}

void Obj86B60__func_8003C9B0(Obj86B60 *self, s32 a1)
{
    void (*handler)(Obj86B60 *self);

    if (self->target == NULL) {
        return;
    }
    if (self->unk3C == 1) {
        handler = self->methods->slotE8;
    } else if (self->unk3C == 2) {
        handler = self->methods->slot114;
    } else {
        return;
    }
    handler(self);
}

void Obj86B60__Tick(Obj86B60 *self)
{
    Unk4CObj *target;
    s32 idx;

    target = self->target;
    idx = self->activeSlot;
    if (target->unk24[idx] != NULL) {
        self->methods->slot108(self);
    } else if (idx == target->unkC) {
        self->methods->slot94(self);
    }
}

void Obj86B60__RefreshViewValue(Obj86B60 *self)
{
    if (self->viewCallback != NULL) {
        self->viewCallback(self->viewCallbackCtx);
    }
    self->methods->slot60(self, 7);
}

void Obj86B60__SetCallback(Obj86B60 *self, void (*a1)(void *ctx), void *a2)
{
    self->viewCallback = a1;
    self->viewCallbackCtx = a2;
}

void Obj86B60__SetFadeCallbackEnabled(Obj86B60 *self, s32 a1)
{
    Obj86B60Methods *methods;

    methods = self->methods;
    switch (a1) {
    case 0:
        self->fadeCallback = NULL;
        break;
    case 1:
        self->fadeCallback = methods->slotB0;
        break;
    }
}

void Obj86B60__func_8003CB30(Obj86B60 *self, s32 a1)
{
    Obj86B60Methods *methods;

    methods = self->methods;
    switch (a1) {
    case 0:
        self->unk8C = NULL;
        break;
    case 1:
        self->unk8C = methods->slotC4;
        break;
    }
}

typedef struct { s8 r, g, b; } RGB8003CB68;

void Obj86B60__SetColors(Obj86B60 *self, s8 *a1, s8 *a2, s8 *a3)
{
    *(RGB8003CB68 *)self->baseColor = *(RGB8003CB68 *)a1;
    *(RGB8003CB68 *)self->unk93 = *(RGB8003CB68 *)a2;
    *(RGB8003CB68 *)self->unk96 = *(RGB8003CB68 *)a3;
}

void Obj86B60__SetFadeRate(Obj86B60 *self, s32 a1)
{
    self->unk84 = a1;
}

s32 Obj86B60__TickFadeCallback(Obj86B60 *self)
{
    s32 result;

    result = 1;
    if (self->fadeCallback != NULL) {
        result = self->fadeCallback(self);
    }
    if (result != 0) {
        self->methods->slot60(self, 5);
    }
    return result;
}

s32 Obj86B60__TickColorFade(Obj86B60 *self)
{
    s32 prod;
    u8 buffer[3];

    prod = self->frameCounter * self->unk84;
    buffer[0] = prod + self->baseColor[0];
    buffer[1] = prod + self->baseColor[1];
    buffer[2] = prod + self->baseColor[2];
    self->methods->slotE4(self, buffer);
    self->unk78->methods->slotB8(self->unk78, 1, buffer);
    return (u8)prod >= 0x81;
}

s32 Obj86B60__func_8003CCDC(Obj86B60 *self)
{
    s32 result;

    result = 1;
    if (self->unk8C != NULL) {
        result = self->unk8C(self);
        if (result == 0) {
            goto epilogue;
        }
    }
    self->methods->slot60(self, 8);
epilogue:
    return result;
}
