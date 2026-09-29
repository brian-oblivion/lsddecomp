/*
 * application.c -- the Application class (include/application.h), the
 * application shell: its ctor (CD init, data source, default screen), its
 * finalize, the screen size setter, initSystems (display, sound and 3D
 * bring-up, and the shared task argument block), a no-op slot, the
 * never-returning main loop that drives the subclass's hooks by
 * runTitleMenu's ApplicationLoopStatus, the table getter and the table.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libcd.h>
#include <libsnd.h>
#include <libgs.h>
#include "application.h"
#include "bmem_pmgr.h"
#include "data_source.h"

extern s32 sCdInitDone;               /* CdInit has been called */
extern ScreenDims sDefaultScreenDims; /* {320, 240} */

void Application__Application(Application *self, s32 dataSource) {
    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetApplicationMethods();
    if (sCdInitDone == 0) {
        CdInit();
        sCdInitDone = 1;
    }
    self->initialized = 0;
    SetActiveDataSource(dataSource);
    self->methods->setScreenDims(self, &sDefaultScreenDims, 0);
}

void Application__Finalize(Application *self) {}

void Application__SetScreenDims(Application *self, ScreenDims *dims, s32 vramMode) {
    self->dims = *dims;
    self->vramMode = vramMode;
}

void Application__InitSystems(Application *self, DrawSystem *drawSystem, struct Pad *pad) {
    if (self->initialized == 0) {
        SetDrawSystem(drawSystem);
        drawSystem->methods->initGraph(drawSystem, &self->dims, self->vramMode);
        SsInit();
        GsInit3D();
        self->aux = BMemPMgrAlloc(sizeof(IntermediateBaseInitArgs));
        self->aux->drawSystem = (BasicClass *)drawSystem;
        self->aux->pad = (BasicClass *)pad;
        self->aux->frameClock = NULL;
        self->aux->lightRig = NULL;
        self->aux->viewport = NULL;
        self->initialized = 1;
    }
}

void Application__NoOpSlot48(Application *self) {}

void Application__RunMainLoop(Application *self) {
    s32 status;

    if (self->initialized) {
        self->methods->showIntroLogos(self);
        for (;;) {
            self->methods->playOpeningMovie(self);
            for (;;) {
                status = self->methods->runTitleMenu(self);
                if (status == APPLICATION_LOOP_REPEAT_MENU) {
                    self->methods->onRepeatMenu(self);
                    continue;
                }
                if (status == APPLICATION_LOOP_DAY) {
                    if (self->methods->runDayTask(self)) {
                        self->methods->playEndingMovie(self);
                    }
                }
                if (status == APPLICATION_LOOP_OPENING) {
                    break;
                }
            }
        }
    }
}

ApplicationMethods *GetApplicationMethods(void) {
    return &gApplicationMethods;
}

/* Application's method table (include/application.h): BasicClass's slots
 * with the ctor and finalize, setScreenDims, initSystems and the main loop;
 * the six sequence hooks are NULL here, each GameApplication's. A (void *)
 * entry is a method declared on another class's type. */
ApplicationMethods gApplicationMethods = {
    APPLICATION_CLASS_ID,
    (void *)BasicClass__Release,
    Application__Application,
    Application__Finalize,
    (void *)BasicClass__AddChild,
    (void *)BasicClass__RemoveChild,
    (void *)BasicClass__RemoveAllChildren,
    (void *)BasicClass__GetNextChild,
    (void *)BasicClass__AddParentRef,
    (void *)BasicClass__RemoveParentRef,
    (void *)BasicClass__ClearParentRefs,
    (void *)BasicClass__GetNextParentRef,
    (void *)BasicClass__NotifyParents,
    BasicClass__NoOpSlot34,
    (void *)BasicClass__OnNotify,
    NULL,
    Application__SetScreenDims,
    (void *)Application__InitSystems,
    Application__NoOpSlot48,
    Application__RunMainLoop,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};
