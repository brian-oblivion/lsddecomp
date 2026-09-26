#ifndef CODE_2C054_H
#define CODE_2C054_H

#include "common.h"
#include "TaskCore.h"
#include "StreamTask.h"
#include "Viewport.h"
#include "MoviePlayer.h"

/*
 * code_2c054: the whole of StreamTask (class id 0x1130, gStreamTaskMethods;
 * include/StreamTask.h, track 4, round 87) and the first seven methods of
 * its parent TaskCore (include/TaskCore.h: allocator, ctor, finalize,
 * reset, init and the onInit/onDeinit hooks). What remains here are this
 * unit's views of the objects both classes hold from classes with no header
 * yet, and the data and allocators only this unit reaches.
 */
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
 * Its local view here (StreamTaskUnk18Obj) was merged there in round 85.
 * StreamTask::player is a MoviePlayer (include/MoviePlayer.h); its local
 * view here (StreamTaskUnkB4Obj) was merged there in round 89. */


#endif
