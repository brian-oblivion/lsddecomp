#ifndef DRAWSYSTEM_H
#define DRAWSYSTEM_H

#include "BasicClass.h"

/*
 * DrawSystem -- the game's screen/graphics singleton, class id 0x1, method
 * table gDrawSystemMethods, a direct BasicClass subclass (`tools/classtable.py
 * gDrawSystemMethods --vs gBasicClassMethods`: overrides only the ctor, adds seventeen
 * slots). Methods in src/code_10ee0.c; no class derives from it.
 *
 * main() builds the one instance (New_DrawSystem) and hands it to
 * Application__InitSystems (code_2b78c), which stores it as the singleton
 * (SetDrawSystem -> gDrawSystem) and calls its initGraph (GsInitGraph and
 * GsDefDispBuff for the screen size). Every other unit reaches it through
 * GetDrawSystem(): TimImage and the movie player upload through loadImage,
 * the movie player clears its frame through clearImage, the CD driver
 * installs its service routine with setCallback, and WBgm and StageMap
 * add it as a child. Viewport keeps it as its `drawSystem` child and flips
 * through swapBuffers/getActiveBuffer.
 *
 * The methods are thin wrappers over libgs/libgpu: the VRAM transfers take
 * a DrawRect and convert it to libgpu's RECT (ConvertRect), and while the
 * object is `running` (start: runLoop, a VSync loop that calls `callback`
 * and notifyParents(self, 2) every pass) they only transfer when syncMode is
 * set, then DrawSync(0) after the transfer.
 */

typedef struct DrawSystem DrawSystem;
typedef struct DrawSystemMethods DrawSystemMethods;

/* DrawSystem's class id (gDrawSystemMethods word +0x000). A single nibble, so
 * `(header & CLASS_ID_ROOT_MASK) == DRAWSYSTEM_CLASS_ID` tests for it or a subclass. */
#define DRAWSYSTEM_CLASS_ID 0x1

/* The command runLoop passes its parents on every VSync pass
 * (notifyParents(self, 2)); StageMap__OnNotifyTag1 acts only on it. */
#define DRAWSYSTEM_EVENT_VSYNC 2

/* A {width, height} pair: the screen size initGraph hands to GsInitGraph
 * and getDims returns. Application keeps one (its default is
 * gDefaultScreenDims = {320, 240}) and passes it to initGraph. */
typedef struct ScreenDims {
    /* +0x0 */ s32 w;
    /* +0x4 */ s32 h;
} ScreenDims;

/* A rectangle as DrawSystem's methods take it: 16-bit origin, 32-bit
 * extent. ConvertRect narrows it to libgpu's all-16-bit RECT (halfword
 * loads at +0/+2/+4/+8); getDims fills one with {0, 0, w, h * 2}. */
typedef struct DrawRect {
    /* +0x0 */ s16 x;
    /* +0x2 */ s16 y;
    /* +0x4 */ s32 w;
    /* +0x8 */ s32 h;
} DrawRect;

struct DrawSystemMethods {
    BASICCLASS_SLOTS(DrawSystem, (DrawSystem * self)); /* +0x008: DrawSystem__DrawSystem */
    /* +0x040 */ void (*init)(DrawSystem *self);       /* DrawSystem__Init */
    /* +0x044 */ void (*initGraph)(DrawSystem *self, ScreenDims *size, s32 vramMode); /* DrawSystem__InitGraph */
    /* +0x048 */ void (*start)(DrawSystem *self);          /* DrawSystem__Start */
    /* +0x04C */ void (*stop)(DrawSystem *self);           /* DrawSystem__Stop */
    /* +0x050 */ void (*swapBuffers)(DrawSystem *self);    /* DrawSystem__SwapBuffers */
    /* +0x054 */ s32 (*getActiveBuffer)(DrawSystem *self); /* DrawSystem__GetActiveBuffer */
    /* +0x058 */ void (*loadImage)(DrawSystem *self, DrawRect *rect, u32 *pixels); /* DrawSystem__LoadImage */
    /* +0x05C */ void (*storeImage)(DrawSystem *self, u32 *pixels, DrawRect *rect); /* DrawSystem__StoreImage */
    /* +0x060 */ s32 (*slot60)(DrawSystem *self); /* DrawSystem__func_80020A1C, always returns 0 */
    /* +0x064: the occupant takes s16 x, y and sign-extends them itself; the slot passes
     * s32 because its caller's bytes need it (RotateVramRectRight, code_2bb9c: an s16
     * prototype adds a caller-side sll/sra per argument). */
    /* +0x064 */ void (*moveImage)(DrawSystem *self, DrawRect *rect, s32 x, s32 y); /* DrawSystem__MoveImage */
    /* +0x068 */ void (*runLoop)(DrawSystem *self);                  /* DrawSystem__RunLoop */
    /* +0x06C */ void (*countFrames)(DrawSystem *self);              /* DrawSystem__CountFrames */
    /* +0x070 */ void (*setVSyncCount)(DrawSystem *self, s32 value); /* DrawSystem__SetVSyncCount */
    /* +0x074 */ s32 (*getVSyncCount)(DrawSystem *self);             /* DrawSystem__GetVSyncCount */
    /* +0x078 */ void (*clearImage)(DrawSystem *self, u8 *color,
                                    DrawRect *rect); /* DrawSystem__ClearImage: NULL rect clears getDims's */
    /* +0x07C */ ScreenDims *(*getDims)(DrawSystem *self, DrawRect *out); /* DrawSystem__GetDims */
    /* +0x080 */ void (*setSyncMode)(DrawSystem *self, s32 value); /* DrawSystem__SetSyncMode */
    /* +0x084 */ void (*setCallback)(DrawSystem *self, void (*callback)(void)); /* DrawSystem__SetCallback */
};

struct DrawSystem {
    BASICCLASS_FIELDS(DrawSystemMethods);
    /* +0x00C */ s32 unkC; /* set to 1 by countFrames when frameCount reaches vsyncCount and it is 0; no reader in C */
    /* +0x010 */ s32 running;     /* set by start, cleared by stop; runLoop's condition */
    /* +0x014 */ ScreenDims size; /* initGraph stores it, getDims returns its address */
    /* +0x01C */ s32 vramMode;    /* initGraph: GsInitGraph's vram mode */
    /* +0x020 */ s32 vsyncCount; /* setVSyncCount (only while not running) / getVSyncCount; runLoop's VSync() argument, countFrames's threshold */
    /* +0x024 */ s32 frameCount; /* countFrames counts it up to vsyncCount */
    /* +0x028 */ u8 pad28[4];
    /* +0x02C */ s32 syncMode; /* setSyncMode; gates the post-transfer DrawSync(0) and the running bypass */
    /* +0x030 */ void (*callback)(void); /* setCallback; runLoop calls it every VSync. The object is 0x34 bytes (New_DrawSystem) */
};

extern DrawSystemMethods gDrawSystemMethods; /* DrawSystem's method table */
extern DrawSystemMethods *Get_vtable_DrawSystem(void);

DrawSystem *GetDrawSystem(void); /* returns gDrawSystem, the singleton */
void SetDrawSystem(DrawSystem *obj);

DrawSystem *New_DrawSystem(void);
void DrawSystem__DrawSystem(DrawSystem *self);
void DrawSystem__Init(DrawSystem *self);
void DrawSystem__InitGraph(DrawSystem *self, ScreenDims *size, s32 vramMode);
void DrawSystem__Start(DrawSystem *self);
void DrawSystem__Stop(DrawSystem *self);
void DrawSystem__SwapBuffers(DrawSystem *self);
s32 DrawSystem__GetActiveBuffer(DrawSystem *self);
void DrawSystem__LoadImage(DrawSystem *self, DrawRect *rect, u32 *pixels);
void DrawSystem__StoreImage(DrawSystem *self, u32 *pixels, DrawRect *rect);
s32 DrawSystem__func_80020A1C(DrawSystem *self);
void DrawSystem__MoveImage(DrawSystem *self, DrawRect *rect, s16 x, s16 y);
void DrawSystem__RunLoop(DrawSystem *self);
void DrawSystem__CountFrames(DrawSystem *self);
void DrawSystem__SetVSyncCount(DrawSystem *self, s32 value);
s32 DrawSystem__GetVSyncCount(DrawSystem *self);
void DrawSystem__ClearImage(DrawSystem *self, u8 *color, DrawRect *rect);
ScreenDims *DrawSystem__GetDims(DrawSystem *self, DrawRect *out);
void DrawSystem__SetSyncMode(DrawSystem *self, s32 value);
void DrawSystem__SetCallback(DrawSystem *self, void (*callback)(void));

#endif
