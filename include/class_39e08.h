#ifndef CLASS_39E08_H
#define CLASS_39E08_H

#include "common.h"
#include "Class865C8.h"

/*
 * Unit class_39e08: the methods of two classes, in ROM order.
 *  - Class865C8 (gClass865C8Methods, 0x1F230), New_Class865C8 through
 *    GetClass865C8Methods: include/Class865C8.h (track 4, round 88).
 *  - TimedTask (gTimedTaskMethods, 0x230), its parent, New_TimedTask
 *    through TimedTask__SetTimeout: include/TimedTask.h. Its last two
 *    functions, TimedTask__PlaySound and GetTimedTaskMethods, open
 *    class_3ac78. NoOpSlot58, CheckTimeout, SetState and SetTimeout are
 *    TimedTask's own methods that Class865C8 inherits unchanged.
 * plus func_8004A070, called once from Class865C8's ctor and once from
 * code_1677c: it manages a pair of file-scope globals (D_8008A978/
 * D_8008A97C) and loops on RegisterFileTableEntries; nothing pins down
 * what it registers.
 *
 * What stays here are the call-site views of objects this unit reaches
 * without a unified class to type them, each with only the slot or word its
 * one caller touches. ObjM (Class865C8::objM) is include/ObjM.h (track 4,
 * round 89).
 */

/* Opaque view of whatever object Class865C8__OnInit reaches through
 * IntermediateBaseInitArgs::unk0: its +0x07C returns the size it hands the
 * viewport's setScreenSize. */
typedef struct SubObjE SubObjE;

typedef struct SubObjEMethods {
    u8 pad00[0x7C];
    struct ViewportSize *(*slot7C)(SubObjE *self, s32 arg1);
} SubObjEMethods;

struct SubObjE {
    SubObjEMethods *methods;
};

/* New_ObjM and ObjM, the class of Class865C8::objM: include/ObjM.h. */

/* BasicClass-family allocator; see code_171e0.h / code_55dd4.h / Entity.h /
 * class_16334.h for the other units that also declare it locally. */
extern void *BMemPMgrAlloc(s32 size);

/* Matched in code_4cd08.c. No return value used. */
extern void TickDreamAuxSlots(void);

/* MATCHED, src/code_39094.c (`char *GetSoundEffectDir(void)`): returns the
 * "SND\\SE" directory string pointer; Class865C8's ctor passes it as
 * TimedTask's soundBankPath. */
extern s32 GetSoundEffectDir(s32 arg1); /* arity-ok: the definition takes no parameter and reads no argument register, but this dead argument IS byte-load-bearing -- retail emits `move a0,zero` at 0x800496A8 ahead of the jal at 0x800496B0 */

/* Matched in code_4cd08.c (still called `InitDreamAux` there, STALLED at
 * 40/56 -- see docs/match-reports/InitDreamAux.md). Takes no arguments,
 * return value (if any) unused here. */
extern void InitDreamAux(void);

/* Filenames right next to each other in the same rodata blob
 * (asm/data/1A90.rodata.s): "ETC\\ETC.TIM" and "ETC\\DREAMER.TMD". */
extern const char D_800113EC[];
extern const char D_800113F8[];

/* Same request-block shape src/code_1677c.c established at
 * `GameApplication__GameApplication`'s call site (`LoadModelRequest`, learned there to be
 * 0x10 bytes even though only the first two fields are ever written --
 * "local struct SIZE matters, not shape"). New_LinkResource
 * (include/LinkResource.h) takes it cast to its descriptor, struct Src6F240. */
typedef struct LoadRequest {
    s32 type;
    const char *path;
    s32 unk08;
    s32 unk0C;
} LoadRequest;

/* src/code_39094.c: one of the seven gWeeklyGroupTable words, each a VAB
 * path string ("SND\\AMBIENT" ... "SND\\STANDERD"). Its return value is
 * forwarded as `New_WBgm`'s own 1st argument (include/WBgm.h). */
extern s32 PickWeeklyGroup(s32 arg1);

/* Also declared in code_1677c.c as `extern s32 func_8004A070(s32 a0)`.
 * Return value discarded at this call site. */
extern s32 func_8004A070(s32 arg1);

/* Also declared in src/code_1677c.c with this exact signature. Return
 * value discarded at this call site. */
extern s32 SetActiveDataSourceDriverMode(s32 arg1, s32 arg2, s32 arg3);

/* New_Class866E8 is declared in include/Class866E8.h. */

#endif
