#ifndef CLASS_39E08_H
#define CLASS_39E08_H

#include "common.h"
#include "DayTask.h"

/*
 * Unit class_39e08: the methods of two classes, in ROM order.
 *  - DayTask (gDayTaskMethods, 0x1F230), New_DayTask through
 *    GetDayTaskMethods: the task that runs one dream day
 *    (include/DayTask.h).
 *  - TimedTask (gTimedTaskMethods, 0x230), its parent, New_TimedTask
 *    through TimedTask__SetTimeout: include/TimedTask.h. Its last two
 *    functions, TimedTask__PlaySound and GetTimedTaskMethods, open
 *    class_3ac78. NoOpSlot58, CheckTimeout, SetState and SetTimeout are
 *    TimedTask's own methods that DayTask inherits unchanged.
 * plus RegisterRecordTableFiles, called once from DayTask's ctor and once from
 * code_1677c: it manages a pair of file-scope globals (sRecordRegisterCalls/
 * sRecordFirstBatchCount) and loops on RegisterFileTableEntries; nothing pins down
 * what it registers.
 *
 * What stays here are the call-site views of objects this unit reaches
 * without a unified class to type them, each with only the slot or word its
 * one caller touches. ObjM (DayTask::objM) is include/ObjM.h.
 */

/* Opaque view of whatever object DayTask__OnInit reaches through
 * IntermediateBaseInitArgs::unk0: its +0x07C returns the size it hands the
 * viewport's setScreenSize. */
typedef struct SubObjE SubObjE;

typedef struct SubObjEMethods {
    u8 pad00[0x7C];
    struct ViewportSize *(*getDims)(SubObjE *self, s32 out); /* DrawSystem__GetDims; its out is NULL here */
} SubObjEMethods;

struct SubObjE {
    SubObjEMethods *methods;
};

/* New_ObjM and ObjM, the class of DayTask::objM: include/ObjM.h. */

/* BasicClass-family allocator; see code_171e0.h / code_55dd4.h / Entity.h /
 * class_16334.h for the other units that also declare it locally. */
extern void *BMemPMgrAlloc(s32 size);

/* Matched in code_4cd08.c. No return value used. */
extern void TickDreamAuxSlots(void);

/* MATCHED, src/code_39094.c (`char *GetSoundEffectDir(void)`): returns the
 * "SND\\SE" directory string pointer; DayTask's ctor passes it as
 * TimedTask's soundBankPath. */
extern s32 GetSoundEffectDir(s32 arg1); /* arity-ok: the definition takes no parameter and reads no argument register, but this dead argument IS byte-load-bearing -- retail emits `move a0,zero` at 0x800496A8 ahead of the jal at 0x800496B0 */

/* Matched in code_4cd08.c (still called `InitDreamAux` there, STALLED at
 * 40/56 -- see docs/match-reports/InitDreamAux.md). Takes no arguments,
 * return value (if any) unused here. */
extern void InitDreamAux(void);

/* Filenames right next to each other in the same rodata blob
 * (asm/data/1A90.rodata.s): "ETC\\ETC.TIM" and "ETC\\DREAMER.TMD". */
extern const char sEtcTimPath[];
extern const char sDreamerTmdPath[];

/* src/code_39094.c: one of the seven gWeeklyGroupTable words, each a VAB
 * path string ("SND\\AMBIENT" ... "SND\\STANDERD"). Its return value is
 * forwarded as `New_WBgm`'s own 1st argument (include/WBgm.h). */
extern s32 PickWeeklyGroup(s32 arg1);

/* Also declared in code_1677c.c as `extern s32 RegisterRecordTableFiles(s32 a0)`.
 * Return value discarded at this call site. */
extern s32 RegisterRecordTableFiles(s32 arg1);

/* Also declared in src/code_1677c.c with this exact signature. Return
 * value discarded at this call site. */
extern s32 SetActiveDataSourceDriverMode(s32 arg1, s32 arg2, s32 arg3);

/* New_StageMap is declared in include/StageMap.h. */

#endif
