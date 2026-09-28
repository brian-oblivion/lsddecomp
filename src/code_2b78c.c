/*
 * code_2b78c -- Application (include/Application.h), the application shell:
 * its ctor (CD init, data source, default screen), its finalize, the screen
 * size setter, initSystems (display, sound and 3D bring-up, and the shared
 * task argument block), a no-op slot, the never-returning main loop that
 * drives the subclass's hooks, and the table getter.
 */
#include "common.h"
#include "Application.h"

extern s32 gCdInitDone;               /* CdInit has been called */
extern ScreenDims gDefaultScreenDims; /* {320, 240} */

/* Psy-Q LIBCD.H / LIBSND.H / LIBGS.H prototypes. */
extern int CdInit(void);
extern void SsInit(void);
extern void GsInit3D(void);

extern void SetActiveDataSource(s32 source); /* src/GameApplicationFileResource.c */
extern void *BMemPMgrAlloc(s32 size);

void Application__Application(Application *self, s32 dataSource) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetApplicationMethods();
    if (gCdInitDone == 0) {
        CdInit();
        gCdInitDone = 1;
    }
    self->initialized = 0;
    SetActiveDataSource(dataSource);
    self->methods->setScreenDims(self, &gDefaultScreenDims, 0);
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
        self->aux = BMemPMgrAlloc(0x14);
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
                if (status == 1) {
                    self->methods->slot5C(self);
                    continue;
                }
                if (status == 2) {
                    if (self->methods->runDayTask(self)) {
                        self->methods->playEndingMovie(self);
                    }
                }
                if (status == 0) {
                    break;
                }
            }
        }
    }
}

ApplicationMethods *GetApplicationMethods(void) {
    return &gApplicationMethods;
}
