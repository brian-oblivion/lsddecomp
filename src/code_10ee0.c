/*
 * code_10ee0 -- GAME code carved from the head of psyq_10ee0 on 2026-09-25
 * (FINISHING-PLAN revision 18). 0x10EE0..0x11474 (vram
 * 0x800206E0..0x80020C74). It was counted as Psy-Q SDK by segment name;
 * tools/gameinsdk.py measured it as game (a call into game code, a method-
 * table entry beside game methods, or contiguity with those, and no Sony
 * fingerprint). What it holds: the 19 methods of gDrawSystemMethods, the game's
 * screen/graphics singleton, matched as DrawSystem this round. `main.c`
 * builds the one instance (`New_DrawSystem`) and hands it into the game's
 * startup chain, which lands it in `code_2b78c.c`'s `Application__InitSystems`
 * as its `source` argument -- that unit dispatches `source`'s own +0x044
 * slot, the address this unit's table lists as `initGraph`
 * (`DrawSystem__InitGraph`, GsInitGraph setup), confirming the two units see
 * the same object. Three OTHER units independently called
 * `GetDrawSystem()`'s return "the draw singleton" in their own comments
 * before this rename, and two of them (`TimImage.c`, `code_179d8_q.c`)
 * independently chose the names `loadImage`/`moveImage` for the exact same
 * slots this unit matched as LoadImage/MoveImage -- three-way convergent
 * naming evidence, not a guess. libgpu/sys starts right after, at
 * ResetGraph (now psyq_11474).
 *
 * Round 81 (bravo) matched the ten small methods/accessors; round 81
 * (alpha) matched ten more (the allocator, ctor, init and the RECT/VRAM
 * helpers). Round 82 (alpha) matched the last four (DrawSystem__InitGraph,
 * DrawSystem__StoreImage, DrawSystem__RunLoop, DrawSystem__ClearImage); the
 * unit is complete. Round 82 (bravo): naming pass -- class named DrawSystem,
 * every function and both gp-variable accessors renamed via
 * tools/rename.py, method-table slots named for the methods they hold. See
 * each function's report `## Naming` for tier and evidence.
 *
 * Round 87 (bravo, track 4): the class is unified. Its one definition is
 * include/DrawSystem.h (object, table, both value types); the SDK types and
 * prototypes it uses come from Sony's <libgpu.h>, <libgs.h> and <libetc.h>.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libetc.h>
#include "DrawSystem.h"

extern void *BMemPMgrAlloc(s32 size);

extern DrawSystem *gDrawSystem; /* sdata: the singleton GetDrawSystem returns */

void ConvertRect(RECT *dst, DrawRect *src);

DrawSystem *New_DrawSystem(void) {
    DrawSystem *obj = BMemPMgrAlloc(sizeof(DrawSystem));

    if (obj != NULL) {
        Get_vtable_DrawSystem()->ctor(obj);
        return obj;
    }
    return NULL;
}

void DrawSystem__DrawSystem(DrawSystem *self) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_DrawSystem();
    self->methods->init(self);
}

void DrawSystem__Init(DrawSystem *self) {
    self->running = 0;
    /* runLoop waits 3 vertical blanks per pass; transfers wait for DrawSync. */
    self->methods->setVSyncCount(self, 3);
    self->methods->setSyncMode(self, 1);
    self->callback = NULL;
}

void DrawSystem__InitGraph(DrawSystem *self, ScreenDims *size, s32 vramMode) {
    /* Non-interlaced, GTE offsets; the 1 turns dithering on. */
    GsInitGraph(size->w, size->h, GsOFSGTE | GsNONINTER, 1, vramMode);
    /* The two display buffers stacked in VRAM: (0, 0) and (0, h). */
    GsDefDispBuff(0, 0, 0, size->h);
    self->size = *size;
    self->vramMode = vramMode;
}

void DrawSystem__Start(DrawSystem *self) {
    if (self->running == 0) {
        self->running = 1;
        self->methods->runLoop(self);
    }
}

void DrawSystem__Stop(DrawSystem *self) {
    if (self->running != 0) {
        self->running = 0;
    }
}

void DrawSystem__SwapBuffers(DrawSystem *self) {
    GsSwapDispBuff();
}

s32 DrawSystem__GetActiveBuffer(DrawSystem *self) {
    return GsGetActiveBuff();
}

void DrawSystem__LoadImage(DrawSystem *self, DrawRect *rect, u32 *pixels) {
    RECT gpuRect;

    if (self->running == 0 || self->syncMode != 0) {
        ConvertRect(&gpuRect, rect);
        LoadImage(&gpuRect, pixels);
        if (self->syncMode != 0) {
            DrawSync(0);
        }
    }
}

void ConvertRect(RECT *dst, DrawRect *src) {
    dst->x = src->x;
    dst->y = src->y;
    dst->w = src->w;
    dst->h = src->h;
}

void DrawSystem__StoreImage(DrawSystem *self, u32 *pixels, DrawRect *rect) {
    RECT gpuRect;

    if (self->running == 0 || self->syncMode != 0) {
        ConvertRect(&gpuRect, rect);
        StoreImage(&gpuRect, pixels);
        if (self->syncMode != 0) {
            DrawSync(0);
        }
    }
}

s32 DrawSystem__NoOpSlot60(DrawSystem *self) {
    return 0;
}

void DrawSystem__MoveImage(DrawSystem *self, DrawRect *rect, s16 x, s16 y) {
    RECT gpuRect;

    ConvertRect(&gpuRect, rect);
    MoveImage(&gpuRect, x, y);
}

void DrawSystem__RunLoop(DrawSystem *self) {
    while (self->running != 0) {
        VSync(self->vsyncCount);
        if (self->callback != NULL) {
            self->callback();
        }
        self->methods->notifyParents(self, DRAWSYSTEM_EVENT_VSYNC);
    }
}

void DrawSystem__CountFrames(DrawSystem *self) {
    DrawSystem *drawSystem = GetDrawSystem();

    drawSystem->frameCount++;
    if (drawSystem->frameCount >= drawSystem->vsyncCount && drawSystem->countReached == 0) {
        drawSystem->countReached = 1;
        drawSystem->frameCount = 0;
    }
}

void DrawSystem__SetVSyncCount(DrawSystem *self, s32 value) {
    if (self->running == 0) {
        self->vsyncCount = value;
    }
}

s32 DrawSystem__GetVSyncCount(DrawSystem *self) {
    return self->vsyncCount;
}

void DrawSystem__ClearImage(DrawSystem *self, u8 *color, DrawRect *rect) {
    DrawRect screen;
    RECT gpuRect;

    if (rect == NULL) {
        self->methods->getDims(self, &screen);
        self->methods->clearImage(self, color, &screen);
    } else {
        ConvertRect(&gpuRect, rect);
        ClearImage(&gpuRect, color[0], color[1], color[2]);
    }
}

ScreenDims *DrawSystem__GetDims(DrawSystem *self, DrawRect *out) {
    if (out != NULL) {
        out->x = 0;
        out->y = 0;
        out->w = self->size.w;
        out->h = self->size.h * 2;
    }
    return &self->size;
}

void DrawSystem__SetSyncMode(DrawSystem *self, s32 value) {
    self->syncMode = value;
}

void DrawSystem__SetCallback(DrawSystem *self, void (*callback)(void)) {
    self->callback = callback;
}

DrawSystemMethods *Get_vtable_DrawSystem(void) {
    return &gDrawSystemMethods;
}

DrawSystem *GetDrawSystem(void) {
    return gDrawSystem;
}

void SetDrawSystem(DrawSystem *obj) {
    gDrawSystem = obj;
}
