#ifndef CODE_2C054_H
#define CODE_2C054_H

#include "common.h"
#include "TaskCore.h"
#include "StreamTask.h"
#include "Viewport.h"

/*
 * code_2c054: the whole of StreamTask (class id 0x1130, gStreamTaskMethods;
 * include/StreamTask.h, track 4, round 87) and the first seven methods of
 * its parent TaskCore (include/TaskCore.h: allocator, ctor, finalize,
 * reset, init and the onInit/onDeinit hooks). What remains here are this
 * unit's views of the objects both classes hold from classes with no header
 * yet, and the data and allocators only this unit reaches.
 */
typedef struct StreamTaskUnkB4Obj StreamTaskUnkB4Obj;
typedef struct StreamTaskUnkB4Methods StreamTaskUnkB4Methods;
typedef struct TaskTextObj TaskTextObj;
typedef struct TaskTextMethods TaskTextMethods;

/* Defined in code_2cc8c_c.c (as void *): &gDefaultStreamTaskInitData. */
extern StreamTaskInitData *GetDefaultStreamTaskInitData(void);

/* A rodata table TaskCore__Reset reaches only by ADDRESS (`lui`/`addiu`, no
 * `lw`/`sw` here) -- passed to setColors as three pointers 3 bytes apart
 * (base, base+3, base+6). Never decoded further by this unit's queued
 * functions, so typed as a plain byte array. */
extern u8 D_8006E860[];

/* Two more rodata symbols reached only by address (round 2, TaskCore__OnInit):
 * `gDefaultStreamTaskInitData`, passed as `TaskTextMethods::slot78`'s 3rd argument, and
 * `D_8006E86C`, a zero Vec3_d294 passed TWICE (same address) as the
 * viewport's attachViewChild viewpoint AND reference point (Viewport.h
 * +0x070). Neither is dereferenced by this unit. */
extern u8 gDefaultStreamTaskInitData[];
extern Vec3_d294 D_8006E86C;

/* StreamTask::player's class, MoviePlayer (New_MoviePlayer, gMoviePlayerMethods:
 * +0x040 Play, +0x048 Advance, +0x04C Abort, +0x06C SetResult), as
 * StreamTask__Finalize, __OnInit, __Update, __SetState and
 * __RefreshViewValue call it; StreamTask.h types the field `BasicClass *`
 * and they cast. Until round 84 it also stood for TaskCore's
 * sound/subHandle/tileMap/tileAtlas (only their +0x004 release is ever
 * called). The BgLayer (TaskCore::bgLayer,
 * include/BgLayer.h since round 88) is not this class: its +0x04C
 * (Class6B5CC's attachToParent) takes three arguments where this class's
 * takes one. */
struct StreamTaskUnkB4Methods {
    u8 pad00[0x04];
    void (*slot04)(StreamTaskUnkB4Obj *self); /* +0x004 */
    u8 pad08[0x040 - 0x008];
    s32 (*slot40)(StreamTaskUnkB4Obj *self, s32 a1, s32 a2, s32 a3, s32 a4); /* +0x040,
                                        StreamTask__OnInit's forward target; return tested
                                        directly (not stored) there */
    u8 pad44[0x048 - 0x044];
    s32 (*slot48)(StreamTaskUnkB4Obj *self); /* +0x048, StreamTask__Update's forward target;
                                        return stored into StreamTask::playDone there */
    void (*slot4C)(StreamTaskUnkB4Obj *self); /* +0x04C, StreamTask__RefreshViewValue's forward target;
                                        1 argument -- see the struct comment for why this
                                        is NOT the same slot as BgLayer's attachToParent */
    u8 pad50[0x06C - 0x050];
    void (*slot6C)(StreamTaskUnkB4Obj *self, s32 a1); /* +0x06C, StreamTask__OnInit's forward target */
};

struct StreamTaskUnkB4Obj {
    StreamTaskUnkB4Methods *methods; /* +0x000 */
};

/* initArgs->unk0's class (IntermediateBaseInitArgs, BasicClass * there) as
 * TaskCore__OnInit/OnDeinit call its +0x078 with baseColor or unk93. */
struct TaskTextMethods {
    u8 pad00[0x078];
    void (*slot78)(TaskTextObj *self, u8 *buf, u8 *flag); /* +0x078; 3rd param widened from
                                        `s32` to a pointer here (round 2, TaskCore__OnInit):
                                        TaskCore__OnDeinit's call passes literal 0 (valid for
                                        either type, no byte change there), but
                                        TaskCore__OnInit's own call to the same slot passes
                                        `&gDefaultStreamTaskInitData` -- a real address, not an integer */
};

struct TaskTextObj {
    TaskTextMethods *methods; /* +0x000 */
};


/* The viewport (TaskCore::viewport, `BasicClass *` in IntermediateBase.h) is a
 * Viewport: TaskCore__OnInit/OnDeinit cast it to include/Viewport.h's type.
 * Its local view here (StreamTaskUnk18Obj) was merged there in round 85. */


/* Allocates StreamTask::player (a StreamTaskUnkB4Obj); called by
 * StreamTask__StreamTask as `New_MoviePlayer(GetDefaultStreamTaskInitData(), 0, 0)`. Not this unit's
 * own function (no INCLUDE_ASM here), so only the call site's own argument
 * and return types are modeled. */
extern StreamTaskUnkB4Obj *New_MoviePlayer(StreamTaskInitData *a0, s32 a1, s32 a2);

#endif
