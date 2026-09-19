#include "common.h"
#include "code_2cc8c.h"

s32 Obj86B60__GetActiveSlotCount(Obj86B60 *self)
{
    return self->unk60[self->unk58];
}

/* TaskCoreMethods table (see code_2c054.h's own richer local view); opaque
 * here since this unit never dereferences it, only returns its address. */
extern u8 gTaskCoreMethods[];

void *Get_vtable_TaskCore(void)
{
    return gTaskCoreMethods;
}

/* A 3-word struct (see code_2c054.h's own StreamTaskInitData local view);
 * opaque here since this unit never dereferences it, only returns its
 * address. */
extern u8 gDefaultStreamTaskInitData[];

void *GetDefaultStreamTaskInitData(void)
{
    return gDefaultStreamTaskInitData;
}

void IntermediateBase__IntermediateBase(Obj86B60 *self)
{
    Get_vtable_BasicClass()->ctor(self);
    self->methods = (Obj86B60Methods *)Get_vtable_IntermediateBase();
    self->methods->resetCounters(self);
}

void Obj86B60__OnNotify(Obj86B60 *self, EventArg *arg1, s32 arg2)
{
    s32 header;

    Get_vtable_BasicClass()->slot38(self, arg1, arg2);
    header = arg1->target->header & 0xF;
    if (header == 1) {
        self->methods->onTag1Notify(self, arg1, arg2);
    } else if (header == 2) {
        self->methods->slot58(self, arg1, arg2);
    } else if (header == 5) {
        self->methods->slot5C(self, arg1, arg2);
    }
}

void Obj86B60__ResetCounters(Obj86B60 *self)
{
    self->unk1C = 0;
    self->unk20 = 0;
}

void Obj86B60__Init(Obj86B60 *self, Obj86B60InitArgs *arg1, s32 arg2)
{
    Obj86B60Methods *methods;
    Unk18Obj *obj18;

    methods = self->methods;
    if (arg1->unk8 != NULL) {
        self->unk10 = (s32)arg1->unk8;
    } else {
        self->unk10 = (s32)func_80042400();
    }
    if (arg1->unkC != NULL) {
        self->unk14 = (s32)arg1->unkC;
    } else {
        self->unk14 = (s32)func_80042694();
    }
    if (arg1->unk10 != NULL) {
        self->viewport = arg1->unk10;
    } else {
        self->viewport = New_Unk18Obj();
    }
    self->initArgs = (Obj86B60UnkC *)arg1;
    obj18 = self->viewport;
    methods->addChild(self, arg1->unk0);
    methods->addChild(self, arg1->unk4);
    methods->addChild(self, (void *)self->unk10);
    methods->slot4C(self, 0, 0, 0);
    self->initMode = arg2;
    if (arg2 == 0) {
        obj18->methods->slot10(obj18, arg1->unk0);
        obj18->methods->slot10(obj18, (void *)self->unk10);
        ((Unk14Obj *)self->unk14)->methods->addChild((Unk14Obj *)self->unk14, (void *)self->unk10);
        methods->slot60(self, 2);
        methods->deinit(self);
    }
}

void Obj86B60__Deinit(Obj86B60 *self)
{
    Obj86B60Methods *methods;
    Unk18Obj *obj18;

    methods = self->methods;
    methods->slot50(self);
    obj18 = self->viewport;
    if (self->initMode == 0) {
        ((Unk14Obj *)self->unk14)->methods->removeChild((Unk14Obj *)self->unk14, (void *)self->unk10);
        obj18->methods->slot14(obj18, (void *)self->unk10);
        obj18->methods->slot14(obj18, ((Obj86B60InitArgs *)self->initArgs)->unk0);
    }
    methods->removeChild(self, (void *)self->unk10);
    methods->removeChild(self, ((Obj86B60InitArgs *)self->initArgs)->unk4);
    methods->removeChild(self, ((Obj86B60InitArgs *)self->initArgs)->unk0);
    if (((Obj86B60InitArgs *)self->initArgs)->unk10 != obj18) {
        self->viewport = obj18->methods->slot4(obj18);
    }
    if ((void *)((Obj86B60InitArgs *)self->initArgs)->unkC != (void *)self->unk14) {
        self->unk14 = (s32)((Unk14Obj *)self->unk14)->methods->release((Unk14Obj *)self->unk14);
    }
    if (((Obj86B60InitArgs *)self->initArgs)->unk8 != (void *)self->unk10) {
        self->unk10 = (s32)((Unk10Obj *)self->unk10)->methods->release((Unk10Obj *)self->unk10);
    }
}

void Obj86B60__OnTag1Notify(Obj86B60 *self, EventArg *arg1, s32 arg2)
{
    Unk4ArgObj *obj4;

    if (arg2 == 2) {
        ((Unk10Obj *)self->unk10)->methods->slot44((Unk10Obj *)self->unk10);
        obj4 = ((Obj86B60InitArgs *)self->initArgs)->unk4;
        obj4->methods->slot44(obj4);
        obj4->methods->slot48(obj4);
    }
}

void Obj86B60__IncrementFrameCounter(Obj86B60 *self)
{
    self->unk1C++;
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_c", Obj86B60__NotifyParents);

void Obj86B60__NotifyTargetReset(Obj86B60 *self)
{
    Obj86B60UnkCTarget *target;

    self->unk1C = 0;
    target = self->initArgs->target;
    target->methods->slot48(target);
}

void Obj86B60__NotifyChildReset(Obj86B60 *self)
{
    Unk0ArgObj *obj0;

    obj0 = ((Obj86B60InitArgs *)self->initArgs)->unk0;
    obj0->methods->slot4C(obj0);
    self->unk1C = 0;
}

IntermediateBaseMethods *Get_vtable_IntermediateBase(void)
{
    return &gIntermediateBaseMethods;
}

Unk18Obj *New_Unk18Obj(void)
{
    Unk18Obj *self;

    self = func_80017B34(0xBC);
    if (self != NULL) {
        func_8003F24C()->ctor(self);
        return self;
    }
    return NULL;
}

void Unk18Obj__Unk18Obj(Unk18Obj *self)
{
    SubHandleObj *obj;

    Get_vtable_BasicClass()->ctor(self);
    self->methods = func_8003F24C();
    self->unkC = 0;
    self->unk10 = 0;
    self->unkAC = func_8001CA94();
    obj = func_8003FDB0(D_8008A90C, 0, 0);
    self->unkB0 = obj;
    obj->methods->slot4C(obj, self->unkAC, D_8008A904);
    self->methods->slot40(self);
}

void Unk18Obj__Finalize(Unk18Obj *self)
{
    self->methods->slot90(self);
    self->methods->slot74(self);
    self->unkAC->methods->slot4(self->unkAC);
    self->methods->slotA8(self, 0);
    Get_vtable_BasicClass()->finalize(self);
}

void Unk18Obj__AddChild(Unk18Obj *self, GenericObj *arg1)
{
    s32 header;

    Get_vtable_BasicClass()->addChild(self, arg1);
    header = arg1->methods->header & 0xF;
    if (header == 4) {
        self->unk10 = arg1;
        self->unk30 = arg1->unk14;
    } else if (header == 1) {
        self->unkC = arg1;
    }
}

void Unk18Obj__RemoveChild(Unk18Obj *self, GenericObj *arg1)
{
    s32 header;

    header = arg1->methods->header & 0xF;
    if (header == 4) {
        self->unk30 = 0;
        self->unk10 = NULL;
    } else if (header == 1) {
        self->unkC = NULL;
    }
    Get_vtable_BasicClass()->removeChild(self, arg1);
}

void Obj86B60__ResetAndRemoveAllChildren(Obj86B60 *self)
{
    self->unk30 = 0;
    self->unk10 = 0;
    self->initArgs = NULL;
    Get_vtable_BasicClass()->removeAllChildren(self);
}
