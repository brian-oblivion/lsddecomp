/*
 * class_3bb8c_g -- TaskObjF methods, slots +0x07C..+0x0B0 of gTaskObjFMethods
 * (0x80086DC4; class_3bb8c_f holds +0x064..+0x078), plus one standalone
 * helper (CopyMemcardIconTemplate) reused by class_3bb8c_m's memcard save
 * writer. The local view type is still called Class86E00_3bb8c_g, after a
 * mid-table address an earlier round took for a separate class table (see
 * include/class_3bb8c.h).
 *
 * TaskObjF runs a `state` machine: SetState/AdvanceState fire a fixed
 * set of transition-entry callbacks and either commit a new `state` or
 * tear the object down; ForceIdleFromState and TickStateDelay drive two
 * more paths into the same shared slot7C/slot8C tail; OnCommand and
 * OnItemSelected forward an externally supplied 2/3 dispatch code the
 * same way. Two lazily-attached child sub-objects, childA (a TextEntry,
 * New_TextEntry, include/TextEntry.h) and childB (a Class86F88,
 * New_Class86F88), are attached/detached in mirrored pairs through the
 * slots +0x044..+0x050 both classes put at the same offsets. A third,
 * independent child (cardIcon) is a memcard-icon TIM image, loaded once
 * by LoadCardIcon and stepped by TickCardIcon.
 *
 * Every function in the unit is matched C. What each numeric `state`
 * code and each OnCommand/OnItemSelected dispatch code means in game
 * terms is not established -- names below describe mechanics, not
 * purpose (tier B throughout except the pure getter GetTaskObjFMethods). See each function's own match report for its evidence.
 */
#include "common.h"
#include "class_3bb8c.h"
#include "ScreenSprite.h"
#include "TextEntry.h"
#include "TimImage.h"

void TaskObjF__SetState(Class86E00_3bb8c_g *self, s32 arg1)
{
    Class86E00Methods_3bb8c_g *methods = self->methods;
    s32 ret;
    s32 i;

    if (self->state == arg1) {
        arg1 = 0x17;
    }

    methods->slot30(self, arg1);
    methods->slot84(self);
    methods->slot80(self, arg1);

    self->waitCounter = 0;
    switch (arg1) {
    case 0x13:
        arg1 = methods->slot50(self) ? 0x11 : 8;
        methods->slot7C(self, arg1);
        break;
    case 0x14:
        if (*(u8 *)self->unk40 == 0) {
            methods->slot58(self, self->unk40, self->unk30, self->unk34);
        }
        ret = methods->slot68(self, self->unk40, self->unk44, self->unk4C,
                              self->unk50, self->unk54, self->unk58);
        arg1 = ret ? 0x16 : 0xC;
        methods->slot7C(self, arg1);
        break;
    case 0x15:
        ret = methods->slot64(self, self->unk40, self->unk54, self->unk58);
        arg1 = ret ? 0x16 : 0x10;
        methods->slot7C(self, arg1);
        break;
    case 0x11:
        methods->slot9C(self);
        break;
    case 0x12:
        methods->slotA8(self);
        break;
    }

    if ((u32)(arg1 - 0x16) < 2) {
        if (self->secondaryMode == 1 && self->unk38 != NULL) {
            BMemPMgrFree(self->unk3C);
            for (i = 0; i < self->unk2C; i++) {
                BMemPMgrFree(((void **)self->unk38)[i]);
            }
            BMemPMgrFree(self->unk38);
            self->unk38 = NULL;
        }
        self->state = 0;
        self->secondaryMode = 0;
    } else {
        self->state = arg1;
    }
}

/* 0x11 (17) entries, indexed by `arg1` (range-checked `< 0x11` below);
 * mostly `char *` string pointers into rodata, a few raw literal words at
 * indices never reached from this call site. `asm/data/76DC8.data.s`. */
extern char *gCardIconNames[];
extern const char gCardPathPrefix[]; /* "CARD\\" */
extern const char gCardPathSuffix[]; /* ".TIM" */
/* 3 words, `New_ScreenSprite`'s rect: a SpriteRect {0, 0, 160, 120}. */
extern s32 D_80086EC4;
/* opaque block, the fresh `cardIcon`'s own `slot4C` arg2, address-only here. */
extern s32 D_8008AA94;

void TaskObjF__LoadCardIcon(Class86E00_3bb8c_g *self, s32 arg1)
{
    char path[0x20];
    char *buf;
    char *name;
    TimImage *handle;
    Class86E00Unk70Obj_3bb8c_g *newVal;

    if (arg1 >= 0x11) {
        return;
    }
    if (self->childReady == 0) {
        return;
    }
    if (self->cardIcon != NULL) {
        return;
    }

    buf = path;
    name = gCardIconNames[arg1];
    buf[0] = '\0';
    strcat(buf, gCardPathPrefix);
    strcat(buf, name);
    strcat(buf, gCardPathSuffix);

    handle = New_TimImage(buf);
    ((TimImageUploadFn)handle->methods->slot78)(handle);
    newVal = (Class86E00Unk70Obj_3bb8c_g *)New_ScreenSprite(handle, (SpriteRect *)&D_80086EC4, 0);
    self->cardIcon = newVal;
    handle->methods->release(handle);
    newVal->methods->slot4C(newVal, self->childReady, (void *)&D_8008AA94);
}

void TaskObjF__TickCardIcon(Class86E00_3bb8c_g *self)
{
    if (self->cardIcon != NULL) {
        self->cardIcon = self->cardIcon->methods->slot4(self->cardIcon);
    }
}

void TaskObjF__OnNotify(Class86E00_3bb8c_g *self, s32 arg1, s32 arg2)
{
    if (self->state != 0) {
        if (arg2 == 0x19) {
            self->methods->slot90(self);
        } else if (arg2 == 0x17) {
            self->methods->slot94(self);
        }
    }
}

void TaskObjF__SetChildFlag8(Class86E00_3bb8c_g *self, s32 arg1)
{
    if (self->childC != NULL) {
        self->childC->methods->slot80(self->childC, arg1, 0x7F, 0x7F);
    }
}

void TaskObjF__AdvanceState(Class86E00_3bb8c_g *self)
{
    Class86E00Methods_3bb8c_g *methods = self->methods;

    switch (self->state) {
    case 2:
    case 4:
    case 0xA:
    case 0xE:
        methods->slot8C(self, 0);
        if (self->state == 0xE) {
            strcpy((char *)self->unk40, self->unk30);
            strcat((char *)self->unk40, ((char **)self->unk3C)[(s32)self->selectedItem]);
            strcpy((char *)self->unk44, ((char **)self->unk38)[(s32)self->selectedItem]);
        }
        if (self->secondaryMode == 2) {
            methods->slot78(self, self->unk40, self->unk44, self->unk48,
                             self->unk4C, self->unk50, self->unk54, self->unk58);
        } else if (self->secondaryMode == 1) {
            methods->slot74(self, self->unk40, self->unk44, self->unk54, self->unk58);
        }
        break;
    case 6:
        methods->slot8C(self, 0);
        methods->slot7C(self, 7);
        break;
    case 3:
    case 5:
    case 8:
    case 9:
    case 0xC:
    case 0xD:
    case 0x10:
        methods->slot8C(self, 0x10);
        methods->slot7C(self, 0x17);
        break;
    }
}

void TaskObjF__ForceIdleFromState(Class86E00_3bb8c_g *self)
{
    switch (self->state) {
    case 4:
    case 6:
    case 0xA:
    case 0xE:
        self->methods->slot8C(self, 0x10);
        self->methods->slot7C(self, 0x17);
        break;
    default:
        break;
    }
}

void TaskObjF__TickStateDelay(Class86E00_3bb8c_g *self)
{
    s32 old;
    s32 newVal;

    if (self->state == 7) {
        old = self->waitCounter;
        newVal = old + 1;
        self->waitCounter = newVal;
        if (old < 6) {
            return;
        }
        self->methods->slot7C(self, 0x13);
    } else if (self->state == 0xB) {
        old = self->waitCounter;
        newVal = old + 1;
        self->waitCounter = newVal;
        if (old < 6) {
            return;
        }
        self->methods->slot7C(self, 0x14);
    } else if (self->state == 0xF) {
        old = self->waitCounter;
        newVal = old + 1;
        self->waitCounter = newVal;
        if (old < 6) {
            return;
        }
        self->methods->slot7C(self, 0x15);
    }
}

void TaskObjF__AttachChildA(Class86E00_3bb8c_g *self)
{
    if (self->childReady != 0 && self->unk60 != 0) {
        if (self->childA == NULL) {
            self->childA = New_TextEntry((char *)((self->unk48 << 1) + self->unk44), 1);
            self->childAttached = 1;
        }
        self->methods->slot10(self, (Class86E00SubObj_3bb8c_g *)self->childA);
        self->childA->methods->loadCardResources(self->childA, (void *)self->childReady);
        self->childA->methods->attachTarget(self->childA, (void *)self->unk60, (void *)self->unk64,
                                           (struct TargetObj86ED0 *)self->childC);
    }
}

void TaskObjF__DetachChildA(Class86E00_3bb8c_g *self)
{
    if (self->childReady != 0 && self->unk60 != 0 && self->childA != NULL) {
        self->childA->methods->detachTarget(self->childA);
        self->childA->methods->releaseCardResources(self->childA);
        if (self->childAttached != 0) {
            self->childA->methods->release(self->childA);
            self->childA = NULL;
        }
    }
}

void TaskObjF__OnCommand(Class86E00_3bb8c_g *self, void *arg1, s32 arg2)
{
    switch (arg2) {
    case 2:
        self->methods->slotA0(self);
        self->methods->slot78(self, self->unk40, self->unk44, self->unk48,
                               self->unk4C, self->unk50, self->unk54, self->unk58);
        break;
    case 3:
        self->methods->slotA0(self);
        self->methods->slot7C(self, 0x17);
        break;
    }
}

void TaskObjF__AttachChildB(Class86E00_3bb8c_g *self)
{
    if (self->childReady != 0 && self->unk60 != 0) {
        if (self->childB == NULL) {
            self->childB = New_Class86F88(self->unk38, 1);
            self->childAttached = 1;
        }
        self->methods->slot10(self, self->childB);
        self->childB->methods->slot44(self->childB, self->childReady);
        self->childB->methods->slot4C(self->childB, self->unk60, self->unk64, self->childC);
    }
}

void TaskObjF__DetachChildB(Class86E00_3bb8c_g *self)
{
    if (self->childReady != 0 && self->unk60 != 0 && self->childB != NULL) {
        self->childB->methods->slot50(self->childB);
        self->childB->methods->slot48(self->childB);
        if (self->childAttached != 0) {
            self->childB->methods->release(self->childB);
            self->childB = NULL;
        }
    }
}

void TaskObjF__OnItemSelected(Class86E00_3bb8c_g *self, GenericSlot9CObj_3bb8c_g *arg1, s32 arg2)
{
    switch (arg2) {
    case 2:
        self->selectedItem = arg1->methods->slot9C(arg1);
        self->methods->slotAC(self);
        self->methods->slot7C(self, 0xE);
        break;
    case 3:
        self->methods->slotAC(self);
        self->methods->slot7C(self, 0x17);
        break;
    }
}

GenericCtorTable_3bb8c_d *GetTaskObjFMethods(void)
{
    return &gTaskObjFMethods;
}

/* Sony's, from libc2 (round 45's own local view -- this unit's first use). */
extern s32 atoi(char *s);

/* VALUE-of `%gp_rel`, round 45's own local view -- a fixed rodata template
 * (ROM image still-uncarved, `asm/data/1C34.rodata.s` region) this
 * function copies raw byte ranges out of; also read by
 * `class_3bb8c_d.c`'s own (differently-typed) local view. */
extern u8 *gMemcardIconTemplate;

/* Struct-copy helper types for round 45's CopyMemcardIconTemplate, all deliberately
 * all-`s8` (alignment 1) per this round's FormatNumberIntoBuffer lever: retail
 * copies these ranges as one unaligned `lwl`/`lwr` word chunk per 4 bytes,
 * with any non-multiple-of-4 remainder as INDIVIDUAL byte loads/stores,
 * never merged into a halfword -- alignment 2 would let GCC trust a
 * halfword move retail does not have. */
typedef struct {
    s8 raw[6];
} Buf6_3bb8c_g;
typedef struct {
    s8 raw[12];
} Buf12_3bb8c_g;
typedef struct {
    s8 a, b;
} Pair2_3bb8c_g;

/* Signature is `include/class_3bb8c.h`'s ALREADY-shared
 * `extern s32 CopyMemcardIconTemplate(s32 arg0, s32 arg1);` (class_3bb8c_m's own
 * caller, TaskObjF__WriteMemcardSaveFile), matched exactly -- this unit's own definition
 * must agree with that declaration since both are visible in this
 * translation unit. Cast to `u8 *` internally; retail's own register
 * content at exit (`$v0` left holding a pointer into the `gMemcardIconTemplate`
 * template in every path) confirms the real return type is a pointer,
 * loosely read as `s32` by the caller that never dereferences it. */
s32 CopyMemcardIconTemplate(s32 arg0, s32 arg1)
{
    u8 *self = (u8 *)arg0;
    u8 *src = (u8 *)arg1;
    s32 t0;
    s32 idx;
    u8 *p;

    if (src != NULL) {
        t0 = ((u32)(src[0xE] - 0x38) < 2) ? 0xE : 0xD;

        *(Pair2_3bb8c_g *)(self + 0x18) = *(Pair2_3bb8c_g *)(gMemcardIconTemplate + 0x1E);
        *(Buf12_3bb8c_g *)(self + 0x6) = *(Buf12_3bb8c_g *)(gMemcardIconTemplate + 0x1E);

        idx = atoi((char *)(src + t0)) - 1;
        p = gMemcardIconTemplate + idx * 2;
        *(Pair2_3bb8c_g *)(self + 0x8) = *(Pair2_3bb8c_g *)p;
        return (s32)p;
    } else {
        u8 *q = gMemcardIconTemplate;

        *(Buf6_3bb8c_g *)(self + 0x6) = *(Buf6_3bb8c_g *)(q + 0x1E);
        return (s32)q;
    }
}
