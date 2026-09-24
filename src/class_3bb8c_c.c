/*
 * class_3bb8c_c -- three small sibling classes, each built by its own
 * New_X/ctor pair (allocate, chain a base ctor, install the class's own
 * vtable): Class869D8, Class86AA0 and Class86B60. All three follow the
 * same class-framework shape documented in docs/research/class-framework.md
 * and already used elsewhere in this codebase (e.g. class_3ac78.c's
 * Class866E8). Class86B60 is the largest of the three -- its own vtable
 * (gClass86B60Methods, 78 slots) is mostly dispatched from the sibling
 * unit class_3bb8c_d.c, which shares this file's header
 * (include/class_3bb8c.h) and struct definitions; this unit contributes
 * only the allocator and ctor.
 *
 * Two free functions round out the unit: CheckObj866E8CountFlag, called
 * directly (not through any vtable) from the still-uncarved
 * func_8004DE08, computes a 0/1 flag from an Obj866E8's own fields; and
 * FormatNumberIntoBuffer, called from Class86B60's own ctor, formats a
 * number into a shared buffer whose broader role (nearby rodata strings
 * hint at a memory-card save label) is not established from this unit
 * alone.
 *
 * All 20 definitions here are matched, 0 INCLUDE_ASM.
 */
#include "common.h"
#include "class_3bb8c.h"

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
    GetUnk18ObjMethods()->ctor(self);
    self->methods = GetClass869D8Methods();
    self->methods->onConstruct(self);
}

void func_8004D2F8(void) {
}

void Class869D8__ForwardIfUnk10AndUnk70(Class869D8 *self)
{
    if (self->unk10 != 0 && self->unk70 != 0) {
        GetUnk18ObjMethods()->slot9C(self);
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
    GetClass6B5CCMethods(self)->ctor(self);
    self->methods = GetClass86AA0Methods();
    self->unk34 = 0;
    self->unk36 = 0;
    self->unk38 = 0;
}

void func_8004D42C(void) {
}

void Class86AA0__ForwardIfTag34(Class86AA0 *self, GenericTagInst_3bb8c_c *arg1)
{
    if (arg1->methods->tag == 0x34) {
        self->methods->slotB8(self);
    }
}

void Class86AA0__ForwardIfArg2InRange(Class86AA0 *self, GenericTagInst_3bb8c_c *arg1, s32 arg2)
{
    GetClass6B5CCMethods(self)->slot9C(self, arg1, arg2);
    if (arg2 >= 9) {
        return;
    }
    do {
        if (arg2 < 5) {
            return;
        }
    } while (0);
    self->methods->slotA0(self, arg1, arg2);
}

void *Class86AA0__ReturnSelf(void *self)
{
    return self;
}

Class86AA0Methods *GetClass86AA0Methods(void)
{
    return &gClass86AA0Methods;
}

Class86B60 *New_Class86B60(void *dreamSys)
{
    Class86B60 *self;

    self = BMemPMgrAlloc(0xC4);
    if (self != NULL) {
        func_8004E2D0()->ctor(self, dreamSys);
        return self;
    }
    return NULL;
}

void Class86B60__Class86B60(Class86B60 *self, void *dreamSys)
{
    DreamSysView_3bb8c_c *dream;
    Class86B60Unk48Obj *obj;

    Get_vtable_TaskCore()->slot08(self, &D_80086D44, &D_800114DC, 0);
    self->methods = func_8004E2D0();
    obj = self->unk48;
    obj->methods->slot9C(obj, -1);
    self->unkA4 = dreamSys;
    self->unkAC = 0;
    dream = dreamSys;
    self->unkBC = dream->methods->slot1B0(dream, &self->unkC0);
    FormatNumberIntoBuffer(dream->methods->slot1A0(dream, 0));
    self->methods->slotD8(self, &D_80086D44);
    self->methods->onConstruct(self, dreamSys);
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
 * `width`-digit decimal string into `self`. This unit's own local view
 * keeps `self` opaque (`void *`) since nothing here touches Obj6EAC0's
 * fields -- the pointer is only passed through. */
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
