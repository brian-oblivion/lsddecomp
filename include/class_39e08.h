#ifndef CLASS_39E08_H
#define CLASS_39E08_H

#include "common.h"
#include "DayTask.h"

/*
 * Declarations src/class_39e08.c uses (DayTask, TimedTask and
 * RegisterRecordTableFiles; the .c's banner says what it holds): the
 * functions it calls that no header it includes declares, and the call-site
 * view of the one object it reaches without its class's header. DayTask and
 * TimedTask themselves are include/DayTask.h and include/TimedTask.h.
 */

/* The init args' drawSystem as DayTask__OnInit calls it: its +0x07C (the
 * DrawSystem's getDims) returns the size it hands the viewport's
 * setScreenSize. */
typedef struct SubObjE SubObjE;

typedef struct SubObjEMethods {
    u8 pad00[0x7C];
    struct ViewportSize *(*getDims)(SubObjE *self, s32 out); /* DrawSystem__GetDims; its out is NULL here */
} SubObjEMethods;

struct SubObjE {
    SubObjEMethods *methods;
};

/* The object allocator every New_<Class> calls. */
extern void *BMemPMgrAlloc(s32 size);

/* src/code_4cd08.c; DayTask's finalize calls it after releasing its
 * resources. */
extern void ReleaseDreamAuxModels(void);

/* src/GameFiles.c: the "SND\\SE" sound bank path, which DayTask's ctor
 * passes as TimedTask's soundBankPath. */
extern char *GetSoundEffectDir(s32 unused); /* arity-ok: the definition takes no parameter and reads no argument register, but this dead argument IS byte-load-bearing -- retail emits `move a0,zero` at 0x800496A8 ahead of the jal at 0x800496B0 */

/* src/code_4cd08.c; DayTask's ctor calls it, and its finalize
 * ReleaseDreamAuxModels. */
extern void InitDreamAux(void);

/* "ETC\\ETC.TIM" and "ETC\\DREAMER.TMD", the files DayTask's ctor loads. */
extern const char sEtcTimPath[];
extern const char sDreamerTmdPath[];

/* src/GameFiles.c: one of the seven gSoundBankPaths words, each a VAB
 * path string ("SND\\AMBIENT" ... "SND\\STANDERD"), which DayTask's ctor
 * passes to New_WBgm as its vabPath. */
extern s32 PickSoundBank(s32 unused);

/* Defined in src/class_39e08.c, after DayTask's methods; code_1677c.c
 * declares it too. */
extern s32 RegisterRecordTableFiles(s32 all);

/* src/code_171e0.c (a void function); code_1677c.c declares it the same
 * way. */
extern s32 SetActiveDataSourceDriverMode(s32 async, s32 mode2, s32 useVSyncCallback);

#endif
