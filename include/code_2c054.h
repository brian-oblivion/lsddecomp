#ifndef CODE_2C054_H
#define CODE_2C054_H

#include "common.h"
#include "TaskCore.h"
#include "Viewport.h"

/*
 * code_2c054: StreamTaskObj (class id 0x1130, gStreamTaskMethods), the
 * streaming-task class (`New_StreamTask`, 0xDC bytes) used to load and
 * drive named "ETC\*.STR" stream files, and the first seven methods of its
 * parent TaskCore (`New_TaskCore`, 0xA4 bytes: ctor, finalize, reset, init
 * and the onInit/onDeinit hooks). TaskCore is declared once, in
 * include/TaskCore.h (track 4, round 84); StreamTaskObj's own class is not
 * unified yet, and its object and table below expand TaskCore's macros.
 *
 * `classtable.py gStreamTaskMethods --vs gTaskCoreMethods`: every slot
 * StreamTaskObj does not override points at TaskCore's function, and the
 * overrides (008/00C/040/044/04C/05C/060/06C/078/080/084/088/08C/094) up-call
 * `Get_vtable_TaskCore()` at the same slot where they need to.
 *
 * `include/Class6D3C8.h` independently views StreamTaskObj's table as
 * `StreamTask` from code_1677c's call sites. Its +0x044 is `configure`,
 * five arguments: StreamTask__Init's own list. TASKCORE_SLOTS types
 * +0x044 as IntermediateBase's init(args, mode), so this unit's table view
 * and that one disagree there; neither calls +0x044 through this table.
 */
typedef struct StreamTaskObj StreamTaskObj;
typedef struct StreamTaskObjMethods StreamTaskObjMethods;
typedef struct StreamTaskUnkB4Obj StreamTaskUnkB4Obj;
typedef struct StreamTaskUnkB4Methods StreamTaskUnkB4Methods;
typedef struct StreamTaskUnk78Obj StreamTaskUnk78Obj;
typedef struct StreamTaskUnk78Methods StreamTaskUnk78Methods;
typedef struct TaskTextObj TaskTextObj;
typedef struct TaskTextMethods TaskTextMethods;
typedef struct StreamTaskInitData StreamTaskInitData;

/* A 3-word struct: StreamTask__StreamTask's optional 5th (stack) argument, and also
 * GetDefaultStreamTaskInitData()'s return type -- both feed the exact same 3-word copy into
 * StreamTaskObj::unkA8/unkAC/unkB0, so they're the same shape. Real field
 * meanings unknown (never dereferenced beyond word offset by this unit's
 * queued functions). */
struct StreamTaskInitData {
    s32 unk0; /* +0x000 */
    s32 unk4; /* +0x004 */
    s32 unk8; /* +0x008 */
};

extern StreamTaskInitData *GetDefaultStreamTaskInitData(void);

/* TaskCore's 72 slots, then StreamTaskObj's five setters. The ctor takes
 * FIVE parameters: New_StreamTask dispatches it with $a0-$a3 plus a fifth
 * argument stored to 0x10($sp), the o32 stack-argument slot
 * (docs/match-reports/New_StreamTask.md). */
struct StreamTaskObjMethods {
    TASKCORE_SLOTS(StreamTaskObj, (StreamTaskObj *self, s32 a1, s32 a2, s32 a3, StreamTaskInitData *a4));
    /* +0x124 */ void (*setUnkC4)(StreamTaskObj *self, s32 a1); /* StreamTask__SetUnkC4 */
    /* +0x128 */ void (*setUnkC8)(StreamTaskObj *self, s32 a1); /* StreamTask__SetLoopCount */
    /* +0x12C */ void (*setUnkCC)(StreamTaskObj *self, s32 a1); /* StreamTask__SetSkipOnConfirm */
    /* +0x130 */ void (*setUnkD0)(StreamTaskObj *self, s32 a1); /* StreamTask__SetUnkD0 */
    /* +0x134 */ void (*setUnkD4)(StreamTaskObj *self, s32 a1); /* StreamTask__SetAbortBeforeFade */
};

/* Object size is 0xDC (from New_StreamTask's allocator call); TaskCore's
 * 0xA4 bytes, then StreamTaskObj's own. */
struct StreamTaskObj {
    TASKCORE_FIELDS(StreamTaskObjMethods);
    s32 unkA4;                     /* +0x0A4, reset to 0 by StreamTask__OnInit; read
                                        and set to a call result by StreamTask__Update */
    StreamTaskInitData unkA8;         /* +0x0A8, whole-struct-copied by StreamTask__StreamTask from
                                          either its 5th argument or GetDefaultStreamTaskInitData()'s
                                          default (retail batches all 3 loads before all
                                          3 stores -- a struct assignment, not 3 separate
                                          field writes) */
    StreamTaskUnkB4Obj *unkB4;      /* +0x0B4, an object with its own vtable
                                        (see StreamTaskUnkB4Obj below); dispatched
                                        through by StreamTask__Finalize */
    s32 unkB8;                       /* +0x0B8, set by StreamTask__Init's arg2 */
    s32 unkBC;                        /* +0x0BC, set by StreamTask__Init's typeLookup */
    s32 unkC0;                         /* +0x0C0, set by StreamTask__Init's flag (5th, stack-spilled) */
    s32 unkC4;                          /* +0x0C4, get/set by StreamTask__SetUnkC4 */
    s32 unkC8;                           /* +0x0C8, get/set by StreamTask__SetLoopCount */
    s32 unkCC;                            /* +0x0CC, get/set by StreamTask__SetSkipOnConfirm */
    s32 unkD0;                             /* +0x0D0, get/set by StreamTask__SetUnkD0 */
    s32 unkD4;                              /* +0x0D4, get/set by StreamTask__SetAbortBeforeFade */
    s32 unkD8;                               /* +0x0D8, read by StreamTask__Update (object end, 0xDC) */
};

/* This class's own "GetMethods" accessor (compare `Get_vtable_Entity` in
 * Entity.h) -- returns &gStreamTaskMethods with no other side effect. Called by
 * New_StreamTask/StreamTask__StreamTask (both still INCLUDE_ASM, not this batch) to
 * fetch the ctor at slot +0x008. */
extern StreamTaskObjMethods *Get_vtable_StreamTask(void);
extern StreamTaskObjMethods gStreamTaskMethods;

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

/* self->unkB4's class (New_MoviePlayer), dispatched through by
 * StreamTask__Finalize, __func_8003BAB4, __func_8003BB5C and
 * __func_8003BDF4. Until round 84 it also stood for TaskCore's
 * sound/subHandle/tileMap/tileAtlas, which TaskCore.h types `BasicClass *`
 * (only their +0x004 release is ever called). The BgLayer (TaskCore::bgLayer)
 * is StreamTaskUnk78Obj below: its +0x04C takes three arguments where this
 * class's takes one. */
struct StreamTaskUnkB4Methods {
    u8 pad00[0x04];
    void (*slot04)(StreamTaskUnkB4Obj *self); /* +0x004 */
    u8 pad08[0x040 - 0x008];
    s32 (*slot40)(StreamTaskUnkB4Obj *self, s32 a1, s32 a2, s32 a3, s32 a4); /* +0x040,
                                        StreamTask__OnInit's forward target; return tested
                                        directly (not stored) there */
    u8 pad44[0x048 - 0x044];
    s32 (*slot48)(StreamTaskUnkB4Obj *self); /* +0x048, StreamTask__Update's forward target;
                                        return stored into StreamTaskObj::unkA4 there */
    void (*slot4C)(StreamTaskUnkB4Obj *self); /* +0x04C, StreamTask__RefreshViewValue's forward target;
                                        1 argument -- see the struct comment for why this
                                        is NOT the same slot as StreamTaskUnk78Methods::slot4C */
    u8 pad50[0x06C - 0x050];
    void (*slot6C)(StreamTaskUnkB4Obj *self, s32 a1); /* +0x06C, StreamTask__OnInit's forward target */
};

struct StreamTaskUnkB4Obj {
    StreamTaskUnkB4Methods *methods; /* +0x000 */
};

/* TaskCore::bgLayer's class, BgLayer (D_8006F2C4: +0x04C/+0x050 are
 * Class6B5CC__AttachToParent/DetachFromParent, +0x0B8 BgLayer__SetColor), as
 * TaskCore__OnInit/OnDeinit call it. */
struct StreamTaskUnk78Methods {
    u8 pad00[0x04];
    void (*slot04)(StreamTaskUnk78Obj *self); /* +0x004, TaskCore__Finalize's forward target */
    u8 pad08[0x04C - 0x008];
    void (*slot4C)(StreamTaskUnk78Obj *self, BasicClass *parent, s32 a2); /* +0x04C, TaskCore__OnInit's
                                        forward target -- 3 arguments, see above */
    void (*slot50)(StreamTaskUnk78Obj *self); /* +0x050, TaskCore__OnDeinit's forward target */
    u8 pad54[0x0B8 - 0x054];
    void (*slotB8)(StreamTaskUnk78Obj *self, s32 a1, u8 *a2); /* +0x0B8, TaskCore__OnInit's
                                        forward target */
};

struct StreamTaskUnk78Obj {
    StreamTaskUnk78Methods *methods; /* +0x000 */
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


/* Allocates/initializes self->unkB4 (a StreamTaskUnkB4Obj); called by
 * StreamTask__StreamTask as `New_MoviePlayer(GetDefaultStreamTaskInitData(), 0, 0)`. Not this unit's
 * own function (no INCLUDE_ASM here), so only the call site's own argument
 * and return types are modeled. */
extern StreamTaskUnkB4Obj *New_MoviePlayer(StreamTaskInitData *a0, s32 a1, s32 a2);

/* Three more externs reached only by TaskCore__TaskCore, none of them in
 * this unit; local views of their allocators. The ctor stores each result
 * into a `BasicClass *` field (tileAtlas, tileMap, bgLayer); the fourth,
 * New_VabStreamObj (sound), is include/VabStreamObj.h's. */
extern StreamTaskUnkB4Obj *New_TileAtlas(s32 a0);
extern StreamTaskUnkB4Obj *New_TileMap(s32 a0, StreamTaskUnkB4Obj *a1);
extern StreamTaskUnk78Obj *New_BgLayer(StreamTaskUnkB4Obj *a0, s32 a1);

#endif
