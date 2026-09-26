#ifndef CLASS6E4F0_H
#define CLASS6E4F0_H

#include "BasicClass.h"
#include "DrawSystem.h"
#include "IntermediateBase.h"

/*
 * Class6E4F0 -- class id 0x60, method table D_8006E4F0, a direct BasicClass
 * subclass (its ctor calls BasicClass's ctor first). Methods in
 * src/code_2b78c.c. Its one subclass is Class6D3C8 (D_8006D3C8, 0x1F60,
 * include/Class6D3C8.h), the game's root object, whose ctor calls this
 * class's ctor first (GetClass6E4F0Methods()->ctor).
 *
 * What its own methods do: the ctor runs CdInit once and selects the active
 * data source (SetActiveDataSource); initSystems (+0x044) takes the draw
 * system and pad main() builds, runs the draw system's initGraph with the
 * stored screen size and vram mode, then SsInit and GsInit3D; runMainLoop
 * (+0x04C) is the game's outer loop and, once initialized, never returns.
 * The name stays the table-address identity: nothing yet names what the
 * class IS.
 *
 * The table is 0x68 bytes, not the 19 slots classtable.py prints: six NULL
 * words at +0x050..+0x064 follow +0x04C in the data, and runMainLoop calls
 * all six through self->methods. They are abstract here; the subclass fills
 * them. Their names are the subclass's occupants (as FileResource's interface
 * slots are named for the CD driver's).
 *
 * Object size 0x20: no allocator, but the subclass's first own field
 * (Class6D3C8's ctor stores its `arg` at +0x020) bounds it.
 */

typedef struct Class6E4F0 Class6E4F0;
typedef struct Class6E4F0Methods Class6E4F0Methods;
struct Pad; /* initSystems's pad: main()'s New_Pad(0, 0) */

/* ScreenDims ({w, h}; the ctor's default is gDefaultScreenDims = {320, 240})
 * and initSystems's drawSystem argument are DrawSystem's: include/DrawSystem.h
 * (round 87). */

/* The 0x14-byte block initSystems allocates is an IntermediateBaseInitArgs
 * (include/IntermediateBase.h): {drawSystem, pad, NULL, NULL, NULL}. Every
 * task the subclass starts receives it as its init argument; NULL in the last
 * three makes IntermediateBase__Init create its own FrameClock, LightRig and
 * Viewport. */

/* clang-format off */
#define CLASS6E4F0_SLOTS(Self, CtorParams)                                                         \
    BASICCLASS_SLOTS(Self, CtorParams);                                                            \
    /* +0x040 */ void (*setScreenDims)(Self *self, ScreenDims *dims, s32 vramMode); /* Class6E4F0__SetScreenDims */ \
    /* +0x044: the fourth argument is the caller's: Class6D3C8__InitSystems  \
     * passes 0 (`move a3,zero` in the jalr's delay slot); the occupant never reads $a3. */      \
    /* +0x044 */ void (*initSystems)(Self *self, DrawSystem *drawSystem, struct Pad *pad, s32 arg3); /* Class6E4F0__InitSystems */ \
    /* +0x048 */ void (*slot48)(Self *self);                    /* Class6E4F0__NoOpSlot48, empty */ \
    /* +0x04C */ void (*runMainLoop)(Self *self);               /* Class6E4F0__RunMainLoop */      \
    /* +0x050..+0x064: NULL here, called by runMainLoop; named for Class6D3C8's occupants */      \
    /* +0x050 */ void (*loadIntroLogoSequence)(Self *self);     /* once, before the loop */        \
    /* +0x054 */ void (*startWeeklyStreamTask)(Self *self);     /* each outer iteration */         \
    /* +0x058 */ s32 (*pollGraphRoomStatus)(Self *self);        /* 0 ends the inner loop, 1 and 2 dispatch */ \
    /* +0x05C */ void (*slot5C)(Self *self);                    /* on status 1; Class6D3C8__NoOpSlot5C */ \
    /* +0x060 */ s32 (*pollStatusObj)(Self *self);              /* on status 2; nonzero runs +0x064 */ \
    /* +0x064 */ void (*startStreamTaskWithInit)(Self *self)    /* Class6D3C8__StartStreamTaskWithInit */
/* clang-format on */

/* clang-format off */
#define CLASS6E4F0_FIELDS(Methods)                                                                 \
    BASICCLASS_FIELDS(Methods);                                                                    \
    /* +0x00C */ ScreenDims dims;      /* setScreenDims; initGraph's size */                       \
    /* +0x014 */ s32 vramMode;         /* setScreenDims; initGraph's GsInitGraph vram mode */      \
    /* +0x018 */ s32 initialized;      /* cleared by the ctor, set by initSystems; runMainLoop runs only once set */ \
    /* +0x01C */ IntermediateBaseInitArgs *aux /* initSystems's allocation: every task's init argument */
/* clang-format on */

struct Class6E4F0Methods {
    CLASS6E4F0_SLOTS(Class6E4F0, (Class6E4F0 * self, s32 dataSource));
};

struct Class6E4F0 {
    CLASS6E4F0_FIELDS(Class6E4F0Methods);
};

extern Class6E4F0Methods D_8006E4F0;
extern Class6E4F0Methods *GetClass6E4F0Methods(void); /* returns &D_8006E4F0 */

void Class6E4F0__Class6E4F0(Class6E4F0 *self, s32 dataSource);
void Class6E4F0__Finalize(Class6E4F0 *self);
void Class6E4F0__SetScreenDims(Class6E4F0 *self, ScreenDims *dims, s32 vramMode);
void Class6E4F0__InitSystems(Class6E4F0 *self, DrawSystem *drawSystem, struct Pad *pad);
void Class6E4F0__NoOpSlot48(Class6E4F0 *self);
void Class6E4F0__RunMainLoop(Class6E4F0 *self);

#endif
