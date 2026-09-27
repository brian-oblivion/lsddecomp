/* code_2cc8c_c -- IntermediateBase's methods, the start of Viewport's, and
 * three accessors ahead of them.
 *
 * TaskCore__GetActiveSlotCount, Get_vtable_TaskCore and
 * GetDefaultMovieFrame come first: one TaskCore method and two plain
 * accessors for data used far more widely (code_2c054.c, class_3bb8c_t.c).
 *
 * Then IntermediateBase (include/IntermediateBase.h, whose banner says what
 * the class does): the ctor, onNotify's split by the sender's root class,
 * the counters, init and deinit, the VSync handler that ticks the frame
 * clock and polls the pad, setState with its two state hooks (which start
 * and stop the DrawSystem), and the table getter.
 *
 * Last, Viewport (include/Viewport.h): New_Viewport, the ctor, finalize, and
 * the addChild/removeChild/removeAllChildren overrides, which cache a
 * DrawSystem child and a SceneNode child (the view node, whose coord2 the
 * reference view hangs from) by root class id. code_2cc8c_d.c holds the rest
 * of Viewport's table.
 */

#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "code_2cc8c.h"
#include "Viewport.h"
#include "LightRig.h"
#include "FrameClock.h"
#include "Pad.h"

s32 TaskCore__GetActiveSlotCount(TaskCore *self) {
    return self->slotCounts[self->activeSlot];
}

TaskCoreMethods *Get_vtable_TaskCore(void) {
    return &gTaskCoreMethods;
}

/* The default movie frame, {640, 0, 320, 240}: StreamTask's default initData
 * and the rect TaskCore__OnInit clears (code_2c054.h). */
extern DrawRect gDefaultMovieFrame;

DrawRect *GetDefaultMovieFrame(void) {
    return &gDefaultMovieFrame;
}

void IntermediateBase__IntermediateBase(IntermediateBase *self) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_IntermediateBase();
    self->methods->resetCounters(self);
}

void IntermediateBase__OnNotify(IntermediateBase *self, BasicClass *sender, s32 event) {
    s32 rootClass;

    Get_vtable_BasicClass()->onNotify((BasicClass *)self, sender, event);
    rootClass = sender->methods->header & CLASS_ID_ROOT_MASK;
    if (rootClass == DRAWSYSTEM_CLASS_ID) {
        self->methods->onTag1Notify(self, sender, event);
    } else if (rootClass == PAD_CLASS_ID) {
        self->methods->onPadEvent(self, sender, event);
    } else if (rootClass == FRAMECLOCK_CLASS_ID) {
        self->methods->update(self, sender, event);
    }
}

void IntermediateBase__ResetCounters(IntermediateBase *self) {
    self->frameCounter = 0;
    self->state = 0;
}

void IntermediateBase__Init(IntermediateBase *self, IntermediateBaseInitArgs *args, s32 mode) {
    IntermediateBaseMethods *methods;
    BasicClass *viewport;

    methods = self->methods;
    if (args->frameClock != NULL) {
        self->unk10 = args->frameClock;
    } else {
        self->unk10 = (BasicClass *)New_FrameClock();
    }
    if (args->lightRig != NULL) {
        self->unk14 = args->lightRig;
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
    methods->addChild(self, args->drawSystem);
    methods->addChild(self, args->pad);
    methods->addChild(self, self->unk10);
    methods->onInit(self, 0, 0, 0);
    self->initMode = mode;
    if (mode == 0) {
        viewport->methods->addChild(viewport, args->drawSystem);
        viewport->methods->addChild(viewport, self->unk10);
        self->unk14->methods->addChild(self->unk14, self->unk10);
        methods->setState(self, 2);
        methods->deinit(self);
    }
}

void IntermediateBase__Deinit(IntermediateBase *self) {
    IntermediateBaseMethods *methods;
    BasicClass *viewport;

    methods = self->methods;
    methods->onDeinit(self);
    viewport = self->viewport;
    if (self->initMode == 0) {
        self->unk14->methods->removeChild(self->unk14, self->unk10);
        viewport->methods->removeChild(viewport, self->unk10);
        viewport->methods->removeChild(viewport, self->initArgs->drawSystem);
    }
    methods->removeChild(self, self->unk10);
    methods->removeChild(self, self->initArgs->pad);
    methods->removeChild(self, self->initArgs->drawSystem);
    if (self->initArgs->viewport != viewport) {
        self->viewport = viewport->methods->release(viewport);
    }
    if (self->initArgs->lightRig != self->unk14) {
        self->unk14 = self->unk14->methods->release(self->unk14);
    }
    if (self->initArgs->frameClock != self->unk10) {
        self->unk10 = self->unk10->methods->release(self->unk10);
    }
}

void IntermediateBase__OnTag1Notify(IntermediateBase *self, BasicClass *sender, s32 event) {
    Pad *pad;

    if (event == DRAWSYSTEM_EVENT_VSYNC) {
        ((FrameClock *)self->unk10)->methods->tick((FrameClock *)self->unk10);
        pad = (Pad *)self->initArgs->pad;
        pad->methods->updateMasks(pad);
        pad->methods->dispatchEvents(pad);
    }
}

void IntermediateBase__IncrementFrameCounter(IntermediateBase *self) {
    self->frameCounter++;
}

/* MATCHING: the two state hooks are ONE call through a slot picked per arm;
 * two direct calls give self a sixth reference and swap $s0/$s1. */
void IntermediateBase__SetState(IntermediateBase *self, s32 state) {
    IntermediateBaseMethods *methods;
    void (*fn)(IntermediateBase *);

    methods = self->methods;
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

void IntermediateBase__OnState2(IntermediateBase *self) {
    DrawSystem *drawSystem;

    self->frameCounter = 0;
    drawSystem = (DrawSystem *)self->initArgs->drawSystem;
    drawSystem->methods->start(drawSystem);
}

void IntermediateBase__OnState3(IntermediateBase *self) {
    DrawSystem *drawSystem;

    drawSystem = (DrawSystem *)self->initArgs->drawSystem;
    drawSystem->methods->stop(drawSystem);
    self->frameCounter = 0;
}

IntermediateBaseMethods *Get_vtable_IntermediateBase(void) {
    return &gIntermediateBaseMethods;
}

Viewport *New_Viewport(void) {
    Viewport *self;

    self = BMemPMgrAlloc(sizeof(Viewport));
    if (self != NULL) {
        GetViewportMethods()->ctor(self);
        return self;
    }
    return NULL;
}

void Viewport__Viewport(Viewport *self) {
    SceneNode *fadeBox;

    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetViewportMethods();
    self->drawSystem = NULL;
    self->viewNode = NULL;
    self->sceneRoot = New_SceneNode();
    fadeBox = (SceneNode *)New_FadeBox(gViewportFadeBoxSize, 0, 0);
    self->fadeBox = fadeBox;
    fadeBox->methods->attachToParent(fadeBox, self->sceneRoot, (LongVec3 *)gFadeBoxAttachPos);
    self->methods->initDefaults(self);
}

void Viewport__Finalize(Viewport *self) {
    self->methods->deinitOt(self);
    self->methods->detachViewChild(self);
    self->sceneRoot->methods->release(self->sceneRoot);
    self->methods->setFadeBox(self, 0);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void Viewport__AddChild(Viewport *self, BasicClass *child) {
    s32 rootClass;

    Get_vtable_BasicClass()->addChild((BasicClass *)self, child);
    rootClass = child->methods->header & CLASS_ID_ROOT_MASK;
    if (rootClass == SCENENODE_CLASS_ID) {
        self->viewNode = (SceneNode *)child;
        self->refView.super = ((SceneNode *)child)->coord2;
    } else if (rootClass == DRAWSYSTEM_CLASS_ID) {
        self->drawSystem = (DrawSystem *)child;
    }
}

void Viewport__RemoveChild(Viewport *self, BasicClass *child) {
    s32 rootClass;

    rootClass = child->methods->header & CLASS_ID_ROOT_MASK;
    if (rootClass == SCENENODE_CLASS_ID) {
        self->refView.super = NULL;
        self->viewNode = NULL;
    } else if (rootClass == DRAWSYSTEM_CLASS_ID) {
        self->drawSystem = NULL;
    }
    Get_vtable_BasicClass()->removeChild((BasicClass *)self, child);
}

/* Viewport's removeAllChildren override (+0x018 of gViewportMethods and of
 * gNodeGuardedViewportMethods): clears the three child caches AddChild fills, then
 * the base. */
void Viewport__RemoveAllChildren(Viewport *self) {
    self->refView.super = NULL;
    self->viewNode = NULL;
    self->drawSystem = NULL;
    Get_vtable_BasicClass()->removeAllChildren((BasicClass *)self);
}
