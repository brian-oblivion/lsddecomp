/*
 * code_2b78c -- GAME code carved from psyq_2b78c on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x2B78C..0x2BA1C (vram 0x8003AF8C..0x8003B21C). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: methods of D_8006E4F0 and
 * D_8006D3C8, calling SetActiveDataSource; the yaml had called this gap "the
 * game's own libsnd build".
 *
 * Round 81 (delta): all seven functions matched. They are the whole of one
 * class, D_8006E4F0 (local view Class6E4F0 below, class id 0x60): its ctor
 * (`Class6E4F0__Class6E4F0`), empty finalize override, four own slots
 * (`SetScreenDims`, `InitSystems`, a no-op, and `RunMainLoop` -- the
 * subclass Class6D3C8's per-frame dispatcher, first run from `src/main.c`)
 * and the table getter.
 *
 * Round 81 (delta), track 3 naming pass: all seven functions and both
 * unit-local globals (`gCdInitDone`, `gDefaultScreenDims`) named -- tiers
 * and evidence in each function's own match report's `## Naming` section.
 * One exception: `func_8003B20C` (the table getter, proposed
 * `GetClass6E4F0Methods`) was NOT renamed -- `tools/rename.py` cannot apply
 * it because this address already carried an explicit, now-stale, track-2
 * "unidentified" line in the symbols file and the tool's placeholder-name
 * address resolution never finds it to replace; see
 * docs/match-reports/GetClass6E4F0Methods.md and the round-81 broadcast for the
 * head to apply by hand. Round 84 (echo, track 4): applied with rename.py,
 * which now replaces the existing symbols line.
 *
 * Round 84 (echo, track 4): the class is declared once, in
 * include/Class6E4F0.h; this unit's local view of it is gone.
 */
#include "common.h"
#include "Class6E4F0.h"

extern s32 gCdInitDone;               /* CdInit has been called */
extern ScreenDims gDefaultScreenDims; /* {320, 240} */

/* Psy-Q LIBCD.H / LIBSND.H / LIBGS.H prototypes. */
extern int CdInit(void);
extern void SsInit(void);
extern void GsInit3D(void);

extern void SetActiveDataSource(s32 arg0); /* code_171e0 */
extern void *BMemPMgrAlloc(s32 size);

void Class6E4F0__Class6E4F0(Class6E4F0 *self, s32 dataSource) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetClass6E4F0Methods();
    if (gCdInitDone == 0) {
        CdInit();
        gCdInitDone = 1;
    }
    self->initialized = 0;
    SetActiveDataSource(dataSource);
    self->methods->setScreenDims(self, &gDefaultScreenDims, 0);
}

void Class6E4F0__Finalize(Class6E4F0 *self) {}

void Class6E4F0__SetScreenDims(Class6E4F0 *self, ScreenDims *dims, s32 vramMode) {
    self->dims = *dims;
    self->vramMode = vramMode;
}

void Class6E4F0__InitSystems(Class6E4F0 *self, DrawSystem *drawSystem, struct Pad *pad) {
    if (self->initialized == 0) {
        SetDrawSystem(drawSystem);
        drawSystem->methods->initGraph(drawSystem, &self->dims, self->vramMode);
        SsInit();
        GsInit3D();
        self->aux = BMemPMgrAlloc(0x14);
        self->aux->unk0 = (BasicClass *)drawSystem;
        self->aux->unk4 = (BasicClass *)pad;
        self->aux->unk8 = NULL;
        self->aux->unkC = NULL;
        self->aux->viewport = NULL;
        self->initialized = 1;
    }
}

void Class6E4F0__NoOpSlot48(Class6E4F0 *self) {}

void Class6E4F0__RunMainLoop(Class6E4F0 *self) {
    s32 status;

    if (self->initialized) {
        self->methods->loadIntroLogoSequence(self);
        for (;;) {
            self->methods->startWeeklyStreamTask(self);
            for (;;) {
                status = self->methods->pollGraphRoomStatus(self);
                if (status == 1) {
                    self->methods->slot5C(self);
                    continue;
                }
                if (status == 2) {
                    if (self->methods->pollStatusObj(self)) {
                        self->methods->startStreamTaskWithInit(self);
                    }
                }
                if (status == 0) {
                    break;
                }
            }
        }
    }
}

Class6E4F0Methods *GetClass6E4F0Methods(void) {
    return &D_8006E4F0;
}
