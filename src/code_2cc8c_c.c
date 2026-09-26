/* code_2cc8c_c -- third slice of the 0x2CC8C block (0x8003DFA0..0x8003E874,
 * 19 functions plus one stall), continuing directly from code_2cc8c_b.
 *
 * After TaskCore's TaskCore__GetActiveSlotCount and the two getters come
 * IntermediateBase's own methods, the ctor through OnState3 and its getter
 * Get_vtable_IntermediateBase (class id 0x30, gIntermediateBaseMethods, the
 * parent of TaskCore and Class86668). The class is declared once, in
 * include/IntermediateBase.h, whose banner says what it does (track 4,
 * round 82); these functions take `IntermediateBase *self`.
 *
 * The remaining functions are Viewport's (class id 0x7, gViewportMethods,
 * include/Viewport.h; track 4, round 85): New_Viewport, the ctor, finalize
 * and the addChild/removeChild/removeAllChildren overrides, which cache a
 * child by its class-id nibble. code_2cc8c_d.c holds the rest of its table. `Get_vtable_TaskCore`/`GetDefaultStreamTaskInitData`
 * are plain accessors for tables SHARED far more widely (code_2c054.c,
 * class_3bb8c_t.c) that simply happen to live in this unit's address range.
 *
 * Round 55 (runner alpha): full track-3 naming pass. Every definition named;
 * see each function's own match report for the `## Naming` evidence.
 * Unk18Obj (now Viewport, include/Viewport.h) and until round 84 the
 * TaskCore view Obj86B60 (now include/TaskCore.h) were
 * SHARED with one or more of code_2cc8c.c, code_2cc8c_b.c and
 * code_2cc8c_d.c (same classes, split by address range across sibling
 * units), so most field/slot renames on those particular structs are
 * PROPOSALS in this round's report, not direct edits -- only the
 * fields/slots this unit's own functions touch AND no sibling reaches were
 * renamed here.
 */

#include "common.h"
#include "code_2cc8c.h"
#include "Viewport.h"
#include "LightRig.h"

s32 TaskCore__GetActiveSlotCount(TaskCore *self)
{
    return self->slotCounts[self->activeSlot];
}

TaskCoreMethods *Get_vtable_TaskCore(void)
{
    return &gTaskCoreMethods;
}

/* A 3-word struct (see code_2c054.h's own StreamTaskInitData local view);
 * opaque here since this unit never dereferences it, only returns its
 * address. */
extern u8 gDefaultStreamTaskInitData[];

void *GetDefaultStreamTaskInitData(void)
{
    return gDefaultStreamTaskInitData;
}

/* One local reading of the objects IntermediateBase calls outside
 * BasicClass's slots: initArgs->unk0 (+0x048 in onState2, +0x04C in
 * onState3), initArgs->unk4 (+0x044, +0x048 in onTag1Notify) and unk10
 * (+0x044 in onTag1Notify). Their classes are not established; every call
 * passes the object alone. */
typedef struct IntermediateBaseLinked IntermediateBaseLinked;
typedef struct IntermediateBaseLinkedMethods {
    u8 pad000[0x044];
    void (*slot44)(IntermediateBaseLinked *self); /* +0x044 */
    void (*slot48)(IntermediateBaseLinked *self); /* +0x048 */
    void (*slot4C)(IntermediateBaseLinked *self); /* +0x04C */
} IntermediateBaseLinkedMethods;
struct IntermediateBaseLinked {
    IntermediateBaseLinkedMethods *methods; /* +0x000 */
};

void IntermediateBase__IntermediateBase(IntermediateBase *self)
{
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_IntermediateBase();
    self->methods->resetCounters(self);
}

void IntermediateBase__OnNotify(IntermediateBase *self, BasicClass *sender, s32 event)
{
    s32 header;

    Get_vtable_BasicClass()->onNotify((BasicClass *)self, sender, event);
    header = sender->methods->header & 0xF;
    if (header == 1) {
        self->methods->onTag1Notify(self, sender, event);
    } else if (header == 2) {
        self->methods->onPadEvent(self, sender, event);
    } else if (header == 5) {
        self->methods->update(self, sender, event);
    }
}

void IntermediateBase__ResetCounters(IntermediateBase *self)
{
    self->frameCounter = 0;
    self->state = 0;
}

void IntermediateBase__Init(IntermediateBase *self, IntermediateBaseInitArgs *args, s32 mode)
{
    IntermediateBaseMethods *methods;
    BasicClass *viewport;

    methods = self->methods;
    if (args->unk8 != NULL) {
        self->unk10 = args->unk8;
    } else {
        self->unk10 = New_FrameClock();
    }
    if (args->unkC != NULL) {
        self->unk14 = args->unkC;
    } else {
        self->unk14 = (BasicClass *)New_LightRig();
    }
    if (args->viewport != NULL) {
        self->viewport = args->viewport;
    } else {
        self->viewport = (BasicClass *)New_Viewport();
    }
    self->initArgs = args;
    viewport = self->viewport;
    methods->addChild(self, args->unk0);
    methods->addChild(self, args->unk4);
    methods->addChild(self, self->unk10);
    methods->onInit(self, 0, 0, 0);
    self->initMode = mode;
    if (mode == 0) {
        viewport->methods->addChild(viewport, args->unk0);
        viewport->methods->addChild(viewport, self->unk10);
        self->unk14->methods->addChild(self->unk14, self->unk10);
        methods->setState(self, 2);
        methods->deinit(self);
    }
}

void IntermediateBase__Deinit(IntermediateBase *self)
{
    IntermediateBaseMethods *methods;
    BasicClass *viewport;

    methods = self->methods;
    methods->onDeinit(self);
    viewport = self->viewport;
    if (self->initMode == 0) {
        self->unk14->methods->removeChild(self->unk14, self->unk10);
        viewport->methods->removeChild(viewport, self->unk10);
        viewport->methods->removeChild(viewport, self->initArgs->unk0);
    }
    methods->removeChild(self, self->unk10);
    methods->removeChild(self, self->initArgs->unk4);
    methods->removeChild(self, self->initArgs->unk0);
    if (self->initArgs->viewport != viewport) {
        self->viewport = viewport->methods->release(viewport);
    }
    if (self->initArgs->unkC != self->unk14) {
        self->unk14 = self->unk14->methods->release(self->unk14);
    }
    if (self->initArgs->unk8 != self->unk10) {
        self->unk10 = self->unk10->methods->release(self->unk10);
    }
}

void IntermediateBase__OnTag1Notify(IntermediateBase *self, BasicClass *sender, s32 event)
{
    IntermediateBaseLinked *obj4;

    if (event == 2) {
        ((IntermediateBaseLinked *)self->unk10)->methods->slot44((IntermediateBaseLinked *)self->unk10);
        obj4 = (IntermediateBaseLinked *)self->initArgs->unk4;
        obj4->methods->slot44(obj4);
        obj4->methods->slot48(obj4);
    }
}

void IntermediateBase__IncrementFrameCounter(IntermediateBase *self)
{
    self->frameCounter++;
}

/* Matched round 72: the two state-dependent calls are ONE call through a
 * slot picked per arm (self then has 5 refs, not 6, so global-alloc ranks
 * state above it: state -> $s0, self -> $s1).  The barrier only moves state's
 * copy into the prologue (instruction order, not register identity). */
void IntermediateBase__SetState(IntermediateBase *self, s32 state)
{
    IntermediateBaseMethods *methods;
    void (*fn)(IntermediateBase *);

    methods = self->methods;
    __asm__("" ::: "memory");
    self->state = state;
    methods->notifyParents(self, state);
    if (state == 2) {
        fn = methods->onState2;
    } else if (state == 3) {
        fn = methods->onState3;
    } else {
        return;
    }
    fn(self);
}

void IntermediateBase__OnState2(IntermediateBase *self)
{
    IntermediateBaseLinked *obj0;

    self->frameCounter = 0;
    obj0 = (IntermediateBaseLinked *)self->initArgs->unk0;
    obj0->methods->slot48(obj0);
}

void IntermediateBase__OnState3(IntermediateBase *self)
{
    IntermediateBaseLinked *obj0;

    obj0 = (IntermediateBaseLinked *)self->initArgs->unk0;
    obj0->methods->slot4C(obj0);
    self->frameCounter = 0;
}

IntermediateBaseMethods *Get_vtable_IntermediateBase(void)
{
    return &gIntermediateBaseMethods;
}

Viewport *New_Viewport(void)
{
    Viewport *self;

    self = BMemPMgrAlloc(0xBC);
    if (self != NULL) {
        GetViewportMethods()->ctor(self);
        return self;
    }
    return NULL;
}

void Viewport__Viewport(Viewport *self)
{
    Class6B5CC *obj;

    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetViewportMethods();
    self->drawSystem = 0;
    self->viewNode = 0;
    self->sceneRoot = New_Class6B5CC();
    obj = (Class6B5CC *)New_Class6E99C(D_8008A90C, 0, 0);
    self->subHandle = obj;
    obj->methods->attachToParent(obj, self->sceneRoot, (Vec3_d294 *)D_8008A904);
    self->methods->initDefaults(self);
}

void Viewport__Finalize(Viewport *self)
{
    self->methods->deinitOt(self);
    self->methods->detachViewChild(self);
    self->sceneRoot->methods->release(self->sceneRoot);
    self->methods->setSubHandle(self, 0);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void Viewport__AddChild(Viewport *self, BasicClass *child)
{
    s32 header;

    Get_vtable_BasicClass()->addChild((BasicClass *)self, child);
    header = child->methods->header & 0xF;
    if (header == 4) {
        self->viewNode = (Class6B5CC *)child;
        self->refView.super = ((Class6B5CC *)child)->coord2;
    } else if (header == 1) {
        self->drawSystem = (DrawSystem *)child;
    }
}

void Viewport__RemoveChild(Viewport *self, BasicClass *child)
{
    s32 header;

    header = child->methods->header & 0xF;
    if (header == 4) {
        self->refView.super = 0;
        self->viewNode = NULL;
    } else if (header == 1) {
        self->drawSystem = NULL;
    }
    Get_vtable_BasicClass()->removeChild((BasicClass *)self, child);
}

/* Viewport's removeAllChildren override (+0x018 of gViewportMethods and of
 * gClass869D8Methods): clears the three child caches AddChild fills, then
 * the base. */
void Viewport__RemoveAllChildren(Viewport *self)
{
    self->refView.super = 0;
    self->viewNode = 0;
    self->drawSystem = NULL;
    Get_vtable_BasicClass()->removeAllChildren((BasicClass *)self);
}
