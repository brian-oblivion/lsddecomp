/*
 * DrawSystem: the game's screen and graphics singleton (include/draw_system.h).
 * This file holds all of its methods, the table getter, and the
 * GetDrawSystem/SetDrawSystem accessors for the one instance, then the
 * method table.
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
#include "draw_system.h"
#include "bmem_pmgr.h"

static DrawSystem *sDrawSystem SDATA = NULL; /* the singleton GetDrawSystem returns */

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

/* DrawSystem's method table (include/draw_system.h): BasicClass's slots with
 * DrawSystem's ctor, then its own: set-up, the VSync loop, the VRAM
 * transfers and the frame counters. A (void *) entry is a base method,
 * declared on BasicClass *. */
DrawSystemMethods gDrawSystemMethods = {
    /* +0x000 header */ DRAWSYSTEM_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ DrawSystem__DrawSystem,
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
    /* +0x038 onNotify */ (void *)BasicClass__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 init */ DrawSystem__Init,
    /* +0x044 initGraph */ DrawSystem__InitGraph,
    /* +0x048 start */ DrawSystem__Start,
    /* +0x04C stop */ DrawSystem__Stop,
    /* +0x050 swapBuffers */ DrawSystem__SwapBuffers,
    /* +0x054 getActiveBuffer */ DrawSystem__GetActiveBuffer,
    /* +0x058 loadImage */ DrawSystem__LoadImage,
    /* +0x05C storeImage */ DrawSystem__StoreImage,
    /* +0x060 slot60 */ DrawSystem__NoOpSlot60,
    /* +0x064 moveImage */ (void *)DrawSystem__MoveImage,
    /* +0x068 runLoop */ DrawSystem__RunLoop,
    /* +0x06C countFrames */ DrawSystem__CountFrames,
    /* +0x070 setVSyncCount */ DrawSystem__SetVSyncCount,
    /* +0x074 getVSyncCount */ DrawSystem__GetVSyncCount,
    /* +0x078 clearImage */ DrawSystem__ClearImage,
    /* +0x07C getDims */ DrawSystem__GetDims,
    /* +0x080 setSyncMode */ DrawSystem__SetSyncMode,
    /* +0x084 setCallback */ DrawSystem__SetCallback,
};
