#ifndef DRAW_SYSTEM_H
#define DRAW_SYSTEM_H

#include "basic_class.h"

/**
 * @file draw_system.h
 * @brief DrawSystem, the screen and graphics singleton, and the screen-size,
 *        VRAM-rectangle and colour types the graphics code passes around.
 *
 * main() builds the one instance (New_DrawSystem) and hands it to
 * Application__InitSystems (Application), which stores it as the singleton
 * (SetDrawSystem) and calls its initGraph (GsInitGraph and GsDefDispBuff for
 * the screen size). Every other unit reaches it through GetDrawSystem():
 * TimImage and the movie player upload through loadImage, the movie player
 * clears its frame through clearImage, the CD driver installs its service
 * routine with setCallback, and WBgm and StageMap add it as a child.
 * Viewport keeps it as its `drawSystem` child and flips through
 * swapBuffers/getActiveBuffer.
 *
 * The methods are thin wrappers over libgs/libgpu: the VRAM transfers take
 * a DrawRect and convert it to libgpu's RECT, and while the object is
 * `running` (start: runLoop, a VSync loop that calls `callback` and
 * notifies the parents with DRAWSYSTEM_EVENT_VSYNC every pass) they transfer
 * only when syncMode is set, then wait with DrawSync(0).
 */

typedef struct DrawSystem DrawSystem;
typedef struct DrawSystemMethods DrawSystemMethods;

/** DrawSystem's class id (gDrawSystemMethods word +0x000). A single nibble, so
 * `(header & CLASS_ID_ROOT_MASK) == DRAWSYSTEM_CLASS_ID` tests for it or a subclass. */
#define DRAWSYSTEM_CLASS_ID 0x1

/** The event runLoop sends its parents on every VSync pass;
 * StageMap__OnDrawSystemEvent acts only on it. */
#define DRAWSYSTEM_EVENT_VSYNC 2

/**
 * A {width, height} screen size: the one initGraph hands to GsInitGraph
 * and getDims returns (Application keeps one, default sDefaultScreenDims =
 * {320, 240}, and passes it to initGraph), and a Viewport's screenSize.
 */
typedef struct ScreenDims {
    /* +0x0 */ s32 width;  /**< in pixels */
    /* +0x4 */ s32 height; /**< in pixels, of one display buffer */
} ScreenDims;

/**
 * A VRAM rectangle as DrawSystem's methods take it: 16-bit origin, 32-bit
 * extent. The methods narrow it to libgpu's all-16-bit RECT; getDims fills
 * one with {0, 0, width, height * 2}, both display buffers.
 */
typedef struct DrawRect {
    /* +0x0 */ s16 x; /**< left edge, in VRAM pixels */
    /* +0x2 */ s16 y; /**< top edge, in VRAM lines */
    /* +0x4 */ s32 w; /**< width */
    /* +0x8 */ s32 h; /**< height */
} DrawRect;

/**
 * An r, g, b colour as the game's objects keep and pass it: a sprite's,
 * a box's, the background's, the ambient and flat lights', a viewport's
 * clear and far colours. Three bytes, not Sony's four-byte CVECTOR: byte
 * members give it size 3 and alignment 1, and its users copy it whole.
 */
typedef struct ColorRgb {
    u8 r; /**< red, 0..255 */
    u8 g; /**< green, 0..255 */
    u8 b; /**< blue, 0..255 */
} ColorRgb;

/** DrawSystem's method table: BasicClass's slots, then seventeen of its own. */
struct DrawSystemMethods {
    BASICCLASS_SLOTS(DrawSystem, (DrawSystem * self)); /* +0x008: DrawSystem__DrawSystem */
    /* +0x040 */ void (*init)(DrawSystem *self);       /**< @see DrawSystem__Init */
    /* +0x044 */ void (*initGraph)(DrawSystem *self, ScreenDims *size,
                                   s32 vramMode);          /**< @see DrawSystem__InitGraph */
    /* +0x048 */ void (*start)(DrawSystem *self);          /**< @see DrawSystem__Start */
    /* +0x04C */ void (*stop)(DrawSystem *self);           /**< @see DrawSystem__Stop */
    /* +0x050 */ void (*swapBuffers)(DrawSystem *self);    /**< @see DrawSystem__SwapBuffers */
    /* +0x054 */ s32 (*getActiveBuffer)(DrawSystem *self); /**< @see DrawSystem__GetActiveBuffer */
    /* +0x058 */ void (*loadImage)(DrawSystem *self, DrawRect *rect, u32 *pixels); /**< @see DrawSystem__LoadImage */
    /* +0x05C */ void (*storeImage)(DrawSystem *self, u32 *pixels,
                                    DrawRect *rect); /**< @see DrawSystem__StoreImage */
    /* +0x060 */ s32 (*slot60)(DrawSystem *self);    /**< @see DrawSystem__NoOpSlot60 */
    /* +0x064 */ void (*moveImage)(DrawSystem *self, DrawRect *rect, s32 x,
                                   s32 y); /**< @see DrawSystem__MoveImage; the slot passes x, y as s32, the occupant takes s16 */
    /* +0x068 */ void (*runLoop)(DrawSystem *self);     /**< @see DrawSystem__RunLoop */
    /* +0x06C */ void (*countFrames)(DrawSystem *self); /**< @see DrawSystem__CountFrames */
    /* +0x070 */ void (*setVSyncCount)(DrawSystem *self, s32 value); /**< @see DrawSystem__SetVSyncCount */
    /* +0x074 */ s32 (*getVSyncCount)(DrawSystem *self); /**< @see DrawSystem__GetVSyncCount */
    /* +0x078 */ void (*clearImage)(DrawSystem *self, u8 *color, DrawRect *rect); /**< @see DrawSystem__ClearImage */
    /* +0x07C */ ScreenDims *(*getDims)(DrawSystem *self, DrawRect *out); /**< @see DrawSystem__GetDims */
    /* +0x080 */ void (*setSyncMode)(DrawSystem *self, s32 value); /**< @see DrawSystem__SetSyncMode */
    /* +0x084 */ void (*setCallback)(DrawSystem *self, void (*callback)(void)); /**< @see DrawSystem__SetCallback */
};

/**
 * DrawSystem: the game's screen and graphics singleton. Class id 0x1
 * (DRAWSYSTEM_CLASS_ID), table gDrawSystemMethods, a direct BasicClass
 * subclass that overrides only the ctor and adds seventeen slots; methods in
 * src/graphics/draw_system.c; no class derives from it. The object is 0x34
 * bytes (New_DrawSystem), built once and never released.
 */
struct DrawSystem {
    BASICCLASS_FIELDS(DrawSystemMethods);
    /* +0x00C */ s32 countReached; /**< countFrames sets it to 1 (and restarts frameCount) once frameCount reaches vsyncCount while it is 0; no code reads or clears it */
    /* +0x010 */ s32 running;      /**< set by start, cleared by stop; runLoop's condition */
    /* +0x014 */ ScreenDims size;  /**< initGraph stores it, getDims returns its address */
    /* +0x01C */ s32 vramMode;     /**< initGraph: GsInitGraph's vram mode */
    /* +0x020 */ s32 vsyncCount; /**< setVSyncCount (only while not running) / getVSyncCount; runLoop's VSync() argument, countFrames's threshold */
    /* +0x024 */ s32 frameCount; /**< countFrames counts it up to vsyncCount */
    /* +0x028 */ u8 pad28[4];
    /* +0x02C */ s32 syncMode; /**< setSyncMode; gates the post-transfer DrawSync(0) and the running bypass */
    /* +0x030 */ void (*callback)(void); /**< setCallback; runLoop calls it every VSync */
};

/** DrawSystem's method table (class id 0x1). */
extern DrawSystemMethods gDrawSystemMethods;

/**
 * @brief The DrawSystem method table.
 * @return &gDrawSystemMethods.
 */
extern DrawSystemMethods *GetDrawSystemMethods(void);

/**
 * @brief The singleton, as SetDrawSystem stored it.
 * @return The game's DrawSystem.
 */
DrawSystem *GetDrawSystem(void);

/**
 * @brief Stores the singleton GetDrawSystem returns (Application__InitSystems).
 * @param obj The DrawSystem.
 */
void SetDrawSystem(DrawSystem *obj);

/**
 * @brief Allocates a DrawSystem and runs its ctor through the table.
 * @return The new DrawSystem, or NULL when the allocation fails.
 */
DrawSystem *New_DrawSystem(void);

/**
 * @brief Constructor (slot +0x008): BasicClass's ctor, installs
 *        gDrawSystemMethods, then calls init.
 * @param self The DrawSystem.
 */
void DrawSystem__DrawSystem(DrawSystem *self);

/**
 * @brief Defaults: not running, 3 vertical blanks per runLoop pass, syncMode 1
 *        (transfers wait for DrawSync), no callback.
 * @param self The DrawSystem.
 */
void DrawSystem__Init(DrawSystem *self);

/**
 * @brief Sets the screen up: GsInitGraph non-interlaced with GTE offsets and
 *        dithering on, and the two display buffers stacked in VRAM at (0, 0)
 *        and (0, height) (GsDefDispBuff). Keeps the size and the mode.
 * @param self The DrawSystem.
 * @param size The screen size.
 * @param vramMode GsInitGraph's vram mode.
 */
void DrawSystem__InitGraph(DrawSystem *self, ScreenDims *size, s32 vramMode);

/**
 * @brief Sets `running` and enters runLoop, which returns only once stop
 *        clears it. Does nothing when already running.
 * @param self The DrawSystem.
 */
void DrawSystem__Start(DrawSystem *self);

/**
 * @brief Clears `running`, which ends runLoop after its current pass.
 * @param self The DrawSystem.
 */
void DrawSystem__Stop(DrawSystem *self);

/**
 * @brief Flips the display buffers (GsSwapDispBuff).
 * @param self The DrawSystem.
 */
void DrawSystem__SwapBuffers(DrawSystem *self);

/**
 * @brief The display buffer being drawn (GsGetActiveBuff).
 * @param self The DrawSystem.
 * @return 0 or 1.
 */
s32 DrawSystem__GetActiveBuffer(DrawSystem *self);

/**
 * @brief Uploads pixels to a VRAM rectangle (LoadImage), then waits for
 *        DrawSync(0) when syncMode is set. While running with syncMode 0 it
 *        transfers nothing.
 * @param self The DrawSystem.
 * @param rect The VRAM rectangle.
 * @param pixels The pixel data.
 */
void DrawSystem__LoadImage(DrawSystem *self, DrawRect *rect, u32 *pixels);

/**
 * @brief Downloads a VRAM rectangle into memory (StoreImage), with
 *        LoadImage's syncMode and running rules.
 * @param self The DrawSystem.
 * @param pixels The buffer to fill.
 * @param rect The VRAM rectangle.
 */
void DrawSystem__StoreImage(DrawSystem *self, u32 *pixels, DrawRect *rect);

/**
 * @brief Slot +0x060: does nothing.
 * @param self The DrawSystem.
 * @return 0.
 */
s32 DrawSystem__NoOpSlot60(DrawSystem *self);

/**
 * @brief Copies a VRAM rectangle to (x, y) in VRAM (MoveImage), whether or
 *        not the loop is running.
 * @param self The DrawSystem.
 * @param rect The source rectangle.
 * @param x The destination's left edge.
 * @param y The destination's top edge.
 */
void DrawSystem__MoveImage(DrawSystem *self, DrawRect *rect, s16 x, s16 y);

/**
 * @brief The frame loop: while `running`, waits vsyncCount vertical blanks
 *        (VSync), calls `callback` when set and notifies the parents with
 *        DRAWSYSTEM_EVENT_VSYNC.
 * @param self The DrawSystem.
 */
void DrawSystem__RunLoop(DrawSystem *self);

/**
 * @brief Counts a frame on the singleton (GetDrawSystem(), not `self`): once
 *        frameCount reaches vsyncCount while countReached is 0, sets
 *        countReached and restarts frameCount.
 * @param self Not read.
 */
void DrawSystem__CountFrames(DrawSystem *self);

/**
 * @brief Sets the vertical blanks per runLoop pass; ignored while running.
 * @param self The DrawSystem.
 * @param value The VSync() argument.
 */
void DrawSystem__SetVSyncCount(DrawSystem *self, s32 value);

/**
 * @brief The vertical blanks per runLoop pass.
 * @param self The DrawSystem.
 * @return vsyncCount.
 */
s32 DrawSystem__GetVSyncCount(DrawSystem *self);

/**
 * @brief Fills a VRAM rectangle with one colour (ClearImage), whether or not
 *        the loop is running. A NULL `rect` clears both display buffers
 *        (getDims's rectangle).
 * @param self The DrawSystem.
 * @param color Three bytes: r, g, b.
 * @param rect The rectangle, or NULL for the whole display area.
 */
void DrawSystem__ClearImage(DrawSystem *self, u8 *color, DrawRect *rect);

/**
 * @brief The screen size, and optionally the VRAM rectangle both display
 *        buffers cover.
 * @param self The DrawSystem.
 * @param out Filled with {0, 0, width, height * 2} when non-NULL.
 * @return &self->size.
 */
ScreenDims *DrawSystem__GetDims(DrawSystem *self, DrawRect *out);

/**
 * @brief Sets syncMode: non-zero makes the transfers run while the loop runs
 *        and wait for DrawSync(0) after each.
 * @param self The DrawSystem.
 * @param value The new syncMode.
 */
void DrawSystem__SetSyncMode(DrawSystem *self, s32 value);

/**
 * @brief Installs the function runLoop calls every pass.
 * @param self The DrawSystem.
 * @param callback The function, or NULL for none.
 */
void DrawSystem__SetCallback(DrawSystem *self, void (*callback)(void));

#endif
