/*
 * DrawSystem: the game's screen and graphics singleton (include/DrawSystem.h).
 * This file holds all of its methods, the table getter, and the
 * GetDrawSystem/SetDrawSystem accessors for the one instance.
 *
 * main() builds it with New_DrawSystem; Application__InitSystems stores it
 * with SetDrawSystem and sets the screen up through initGraph. Start runs
 * runLoop, a VSync loop that calls the installed callback and notifies the
 * parents every pass until stop clears `running`. The VRAM transfers
 * (load/store/move/clear image) are thin wrappers over libgpu that narrow a
 * DrawRect to libgpu's RECT (ConvertRect); while the loop runs they transfer
 * only when syncMode is set, and then wait for DrawSync(0).
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libetc.h>
#include "DrawSystem.h"
#include "BMemPMgr.h"

extern DrawSystem *sDrawSystem; /* sdata: the singleton GetDrawSystem returns */

void ConvertRect(RECT *dst, DrawRect *src);

DrawSystem *New_DrawSystem(void) {
    DrawSystem *obj = BMemPMgrAlloc(sizeof(DrawSystem));

    if (obj != NULL) {
        GetDrawSystemMethods()->ctor(obj);
        return obj;
    }
    return NULL;
}

void DrawSystem__DrawSystem(DrawSystem *self) {
    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetDrawSystemMethods();
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
    GsInitGraph(size->width, size->height, GsOFSGTE | GsNONINTER, 1, vramMode);
    /* The two display buffers stacked in VRAM: (0, 0) and (0, height). */
    GsDefDispBuff(0, 0, 0, size->height);
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
        out->w = self->size.width;
        out->h = self->size.height * 2;
    }
    return &self->size;
}

void DrawSystem__SetSyncMode(DrawSystem *self, s32 value) {
    self->syncMode = value;
}

void DrawSystem__SetCallback(DrawSystem *self, void (*callback)(void)) {
    self->callback = callback;
}

DrawSystemMethods *GetDrawSystemMethods(void) {
    return &gDrawSystemMethods;
}

DrawSystem *GetDrawSystem(void) {
    return sDrawSystem;
}

void SetDrawSystem(DrawSystem *obj) {
    sDrawSystem = obj;
}
