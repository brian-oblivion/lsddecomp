/*
 * IntermediateBase's methods (include/intermediate_base.h, whose banner
 * says what the class does: a job, run by the game loop to a result, that
 * TaskCore and TimedTask derive from), in ROM order: the ctor, onNotify's
 * split by the sender's root class, the counters, init and deinit, the
 * VSync handler that ticks the frame clock and polls the pad,
 * IncrementFrameCounter, setState
 * with its two state hooks (which start and stop the DrawSystem), ending
 * with its getter GetIntermediateBaseMethods; its method table closes the
 * file. A (void *) entry in it is a function whose declared type differs
 * from its slot's: a method inherited from BasicClass and declared on its
 * type, or an empty method declared (void).
 */
#include "common.h"
#include <libgte.h>
#include "intermediate_base.h"
#include "viewport.h"
#include "pad.h"
#include "light_rig.h"
#include "frame_clock.h"

void IntermediateBase__IntermediateBase(IntermediateBase *self) {
    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetIntermediateBaseMethods();
    self->methods->resetCounters(self);
}

void IntermediateBase__OnNotify(IntermediateBase *self, BasicClass *sender, s32 event) {
    s32 rootClass;

    GetBasicClassMethods()->onNotify((BasicClass *)self, sender, event);
    rootClass = sender->methods->header & CLASS_ID_ROOT_MASK;
    if (rootClass == DRAWSYSTEM_CLASS_ID) {
        self->methods->onDrawSystemEvent(self, sender, event);
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
        self->frameClock = args->frameClock;
    } else {
        self->frameClock = (BasicClass *)New_FrameClock();
    }
    if (args->lightRig != NULL) {
        self->lightRig = args->lightRig;
    } else {
        self->lightRig = (BasicClass *)New_LightRig();
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
    methods->addChild(self, self->frameClock);
    methods->onInit(self, 0, 0, 0);
    self->initMode = mode;
    if (mode == INTERMEDIATEBASE_INIT_RUN) {
        viewport->methods->addChild(viewport, args->drawSystem);
        viewport->methods->addChild(viewport, self->frameClock);
        self->lightRig->methods->addChild(self->lightRig, self->frameClock);
        methods->setState(self, INTERMEDIATEBASE_STATE_START);
        methods->deinit(self);
    }
}

void IntermediateBase__Deinit(IntermediateBase *self) {
    IntermediateBaseMethods *methods;
    BasicClass *viewport;

    methods = self->methods;
    methods->onDeinit(self);
    viewport = self->viewport;
    if (self->initMode == INTERMEDIATEBASE_INIT_RUN) {
        self->lightRig->methods->removeChild(self->lightRig, self->frameClock);
        viewport->methods->removeChild(viewport, self->frameClock);
        viewport->methods->removeChild(viewport, self->initArgs->drawSystem);
    }
    methods->removeChild(self, self->frameClock);
    methods->removeChild(self, self->initArgs->pad);
    methods->removeChild(self, self->initArgs->drawSystem);
    if (self->initArgs->viewport != viewport) {
        self->viewport = viewport->methods->release(viewport);
    }
    if (self->initArgs->lightRig != self->lightRig) {
        self->lightRig = self->lightRig->methods->release(self->lightRig);
    }
    if (self->initArgs->frameClock != self->frameClock) {
        self->frameClock = self->frameClock->methods->release(self->frameClock);
    }
}

void IntermediateBase__OnDrawSystemEvent(IntermediateBase *self, BasicClass *sender, s32 event) {
    Pad *pad;

    if (event == DRAWSYSTEM_EVENT_VSYNC) {
        ((FrameClock *)self->frameClock)->methods->tick((FrameClock *)self->frameClock);
        pad = (Pad *)self->initArgs->pad;
        pad->methods->updateMasks(pad);
        pad->methods->dispatchEvents(pad);
    }
}

void IntermediateBase__IncrementFrameCounter(IntermediateBase *self) {
    self->frameCounter++;
}

/* MATCHING: the two state hooks are one call through a slot picked per arm; a call in each arm
 * compiles differently. */
void IntermediateBase__SetState(IntermediateBase *self, s32 state) {
    IntermediateBaseMethods *methods;
    void (*fn)(IntermediateBase *);

    methods = self->methods;
    self->state = state;
    methods->notifyParents(self, state);
    if (state == INTERMEDIATEBASE_STATE_START) {
        fn = methods->onStart;
    } else if (state == INTERMEDIATEBASE_STATE_STOP) {
        fn = methods->onStop;
    } else {
        return;
    }
    fn(self);
}

void IntermediateBase__OnStart(IntermediateBase *self) {
    DrawSystem *drawSystem;

    self->frameCounter = 0;
    drawSystem = (DrawSystem *)self->initArgs->drawSystem;
    drawSystem->methods->start(drawSystem);
}

void IntermediateBase__OnStop(IntermediateBase *self) {
    DrawSystem *drawSystem;

    drawSystem = (DrawSystem *)self->initArgs->drawSystem;
    drawSystem->methods->stop(drawSystem);
    self->frameCounter = 0;
}

IntermediateBaseMethods *GetIntermediateBaseMethods(void) {
    return &gIntermediateBaseMethods;
}

/* IntermediateBase (include/intermediate_base.h): BasicClass's slots with
 * the task lifecycle: resetCounters, init, deinit, the DrawSystem event
 * handler, the frame counter, setState, onStart and onStop. */
IntermediateBaseMethods gIntermediateBaseMethods = {
    /* +0x000 header */ INTERMEDIATEBASE_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ IntermediateBase__IntermediateBase,
    /* +0x00C finalize */ (void *)BasicClass__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)IntermediateBase__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 resetCounters */ IntermediateBase__ResetCounters,
    /* +0x044 init */ (void *)IntermediateBase__Init,
    /* +0x048 deinit */ IntermediateBase__Deinit,
    /* +0x04C onInit */ NULL,
    /* +0x050 onDeinit */ NULL,
    /* +0x054 onDrawSystemEvent */ IntermediateBase__OnDrawSystemEvent,
    /* +0x058 onPadEvent */ NULL,
    /* +0x05C update */ (void *)IntermediateBase__IncrementFrameCounter,
    /* +0x060 setState */ IntermediateBase__SetState,
    /* +0x064 onStart */ IntermediateBase__OnStart,
    /* +0x068 onStop */ IntermediateBase__OnStop,
};
