/*
 * class_3bb8c_c -- three small sibling classes, each built by its own
 * New_X/ctor pair (allocate, chain a base ctor, install the class's own
 * vtable): Class869D8, Class86AA0 and Class86B60. All three follow the
 * same class-framework shape documented in docs/research/class-framework.md
 * and already used elsewhere in this codebase (e.g. class_3ac78.c's
 * Class866E8). Class86B60 (include/Class86B60.h, a TaskCore) is the
 * largest of the three -- its own vtable (gClass86B60Methods, 78 slots)
 * is occupied mostly by the sibling unit class_3bb8c_d.c; this unit
 * contributes only the allocator and ctor.
 *
 * Two free functions round out the unit: CheckObj866E8CountFlag, called
 * directly (not through any vtable) from
 * Class86B60__CommitNameEntry, computes a 0/1 flag from an Obj866E8's own fields; and
 * FormatNumberIntoBuffer, called from Class86B60's own ctor, formats a
 * number into a shared buffer whose broader role (nearby rodata strings
 * hint at a memory-card save label) is not established from this unit
 * alone.
 *
 * All 20 definitions here are matched, 0 INCLUDE_ASM.
 */
#include "common.h"
#include "class_3bb8c.h"
#include "DreamSys.h"
#include "Class6B5CC.h"
#include "Viewport.h"
#include "Class869D8.h"
#include "Class86AA0.h"
#include "Class86B60.h"
#include "VabStreamObj.h"

Class869D8 *New_Class869D8(void)
{
    Class869D8 *self;

    self = BMemPMgrAlloc(0xDC);
    if (self != NULL) {
        GetClass869D8Methods()->ctor(self);
        return self;
    }
    return NULL;
}

void Class869D8__Class869D8(Class869D8 *self)
{
    GetViewportMethods()->ctor((Viewport *)self);
    self->methods = GetClass869D8Methods();
    self->methods->initDefaults(self);
}

void Class869D8__InitDefaults(void) {
}

void Class869D8__Update(Class869D8 *self)
{
    if (self->viewNode != NULL && self->otReady != 0) {
        GetViewportMethods()->update((Viewport *)self);
    }
}

void func_8004D35C(void) {
}

void func_8004D364(void) {
}

void func_8004D36C(void) {
}

void func_8004D374(void) {
}

Class869D8Methods *GetClass869D8Methods(void)
{
    return &gClass869D8Methods;
}

Class86AA0 *New_Class86AA0(void)
{
    Class86AA0 *self;

    self = BMemPMgrAlloc(0x3C);
    if (self != NULL) {
        GetClass86AA0Methods()->ctor(self);
        return self;
    }
    return NULL;
}

void Class86AA0__Class86AA0(Class86AA0 *self)
{
    GetClass6B5CCMethods()->ctor((Class6B5CC *)self);
    self->methods = GetClass86AA0Methods();
    self->unk34 = 0;
    self->flags36 = 0;
    self->nextInCell = NULL;
}

void Class86AA0__Reset(void) {
}

/* Only the low byte of the sender's class id is read: 0x34 is an Actor
 * (Actor__DispatchLinkCommand makes the same test the other way round). */
void Class86AA0__DispatchLinkCommand(Class86AA0 *self, BasicClass *sender, s32 event)
{
    if (*(u8 *)sender->methods == 0x34) {
        self->methods->onActorLinkCommand(self, sender, event);
    }
}

/* tryAttachNearby keeps Class6B5CC's one-parameter slot type; this caller
 * passes the sender and event too, as Actor__OnActorLinkCommand does. */
void Class86AA0__OnActorLinkCommand(Class86AA0 *self, void *sender, s32 event)
{
    GetClass6B5CCMethods()->dispatchLinkCommand((Class6B5CC *)self, sender, event);
    if (event >= 9) {
        return;
    }
    do {
        if (event < 5) {
            return;
        }
    } while (0);
    ((void (*)(Class86AA0 *, void *, s32))self->methods->tryAttachNearby)(self, sender, event);
}

void *Class86AA0__ReturnSelf(Class86AA0 *self)
{
    return self;
}

Class86AA0Methods *GetClass86AA0Methods(void)
{
    return &gClass86AA0Methods;
}

Class86B60 *New_Class86B60(struct DreamSys *dreamSys)
{
    Class86B60 *self;

    self = BMemPMgrAlloc(0xC4);
    if (self != NULL) {
        GetClass86B60Methods()->ctor(self, dreamSys);
        return self;
    }
    return NULL;
}

void Class86B60__Class86B60(Class86B60 *self, struct DreamSys *dreamSys)
{
    DreamSys *dream;
    VabStreamObj *sound;

    Get_vtable_TaskCore()->ctor((TaskCore *)self, &D_80086D44, (char *)D_800114DC, 0);
    self->methods = GetClass86B60Methods();
    sound = (VabStreamObj *)self->sound;
    sound->methods->setPitchOffset(sound, -1);
    self->dreamSys = dreamSys;
    self->saveCtrl = 0;
    dream = dreamSys;
    self->saveBlock = dream->methods->getSaveBlock(dream, &self->saveBlockSize);
    FormatNumberIntoBuffer(dream->methods->getCurrentDayAndYear(dream, 0));
    self->methods->setTarget(self, &D_80086D44);
    ((Class86B60ResetCallFn)self->methods->resetCounters)(self, dreamSys);
}

void CheckObj866E8CountFlag(Ctx678_3bb8c_c *ctx, Result678_3bb8c_c *out)
{
    Obj866E8 *target = ctx->target;
    s32 flag = 1;

    if (target->unkC > 9999999) {
        flag = (target->unk2F4 == 0);
    }
    out->block[1] = flag;
}

/* FormatFullWidthNumber is GAME code (matched round 38, src/code_2cc8c_f.c -- its
 * own C definition, not a Sony object), which formats a1 as a zero-padded
 * `width`-digit decimal string into `self`, the output buffer (the
 * definition's `u8 *dst`; it was typed as a TextRow view until round 88).
 * This unit's own local view keeps it `void *`. */
extern void FormatFullWidthNumber(void *self, s32 a1, s32 width, s32 unpadded);

/* The 6-byte value formatted into D_8008AA24's buffer by FormatFullWidthNumber
 * above, copied whole into D_8008AA18's buffer at +0x12 as ONE struct
 * assignment. All-`s8` fields (alignment 1, not 2 or 4) is what makes
 * retail's block-move split this way: the leading 4 bytes go via the
 * unaligned lwl/lwr word copy regardless of declared alignment (same
 * idiom as Vec2s16, UpdatePolyBBoxAndCull), but the trailing 2 bytes can no
 * longer be proven 2-byte aligned, so there is no safe halfword move for
 * them and the compiler falls back to two individual signed-byte
 * loads/stores. See docs/match-reports/FormatNumberIntoBuffer.md. */
typedef struct {
    s8 a, b, c, d, e, f;
} Buf6_3bb8c_c;

void FormatNumberIntoBuffer(s32 arg0)
{
    FormatFullWidthNumber(D_8008AA24, arg0, 3, 0);
    *(Buf6_3bb8c_c *)((s8 *)D_8008AA18 + 0x12) = *(Buf6_3bb8c_c *)D_8008AA24;
}
