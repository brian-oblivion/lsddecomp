#ifndef CLASS_3BB8C_H
#define CLASS_3BB8C_H

#include "common.h"
#include "BasicClass.h"
#include "TaskCore.h"
#include "StageMap.h"
#include "DrawSystem.h"

/*
 * class_3bb8c.h -- the data and shared helper declarations of the four
 * units below. The classes those units hold
 * each have their own header, and nothing here redefines them:
 *
 *   DayTaskStageMap.c    StageMap (include/StageMap.h), the chunk grid manager
 *   TitleMenuTaskObjF.c  NodeGuardedViewport, GridCell, TitleMenu
 *                        (include/TitleMenu.h, a TaskCore) and TaskObjF
 *                        (include/TaskObjF.h), the memory-card task
 *   TextEntryItemList.c  TextEntry (include/TextEntry.h), ItemList's first half
 *   ObjMStyleActor.c     ItemList's second half (include/ItemList.h),
 *                        ObjM (include/ObjM.h) and the style layer
 *
 * What is here, in that order: StageMap's lookup tables and SplitCoord2,
 * the view of a slot's origin its methods read; TitleMenu's data (menu
 * description, paths, the save title's buffers, the colour cycle); TaskObjF's
 * event table, the memory-card device names (McDevicePath) and helpers;
 * ObjM's StyleConfig record and the helpers its methods call.
 *
 * A prototype for a Sony library function, or for a function one unit
 * alone calls under its own reading, lives in that unit instead: every
 * unit that includes this header would otherwise inherit it, and a second
 * reading of the same name in any of them would collide.
 */

/* ---- ObjM -------------------------------------------------------- */

/* ObjM::styleConfig's pointee (include/ObjM.h): the day's scene style, a
 * plain record. RegisterStyleConfig returns sStyleConfig after
 * FillStyleFromConfig fills its last four words from the stage's config
 * bytes (ObjMStyleActor, whose local StyleM views the same words), or
 * InitStyleAndWorld's caller supplies one. ObjM__SetupSceneStyle hands the
 * first three to the StageMap's lights, ObjM__EnterStyleSession the rest to
 * the viewport, ObjM__PollTimBlockLoad a colour to the TimBlockSrc. */
typedef struct StyleConfig {
    s32 lightDirs;    /* +0x000, SetupSceneStyle: the StageMap's setChildParams `dirs` */
    s32 lightColors;  /* +0x004, SetupSceneStyle: setChildParams `colors` */
    s32 ambientColor; /* +0x008, SetupSceneStyle: setAmbientColor's rgb (a pointer) */
    void *clearColor; /* +0x00C, EnterStyleSession: the viewport's setClearColor; a gStylePalette entry */
    u8 pad10[0x014 - 0x010];
    s32 colorMode; /* +0x014, EnterStyleSession: 1 makes the far colour clearColor; PollTimBlockLoad: 2 fades to clearColor, else farColor */
    void *farColor; /* +0x018, EnterStyleSession: setFarColor unless colorMode is 1; a gStylePalette entry */
    s32 fogNear; /* +0x01C, EnterStyleSession: the viewport's setFogNear; a sStyleFogNears value */
} StyleConfig;

/* ObjM__GetGridRecord's grid lookups (src/GameFiles.c): a
 * non-negative code is a linear cell index (GetStageMapChunkRecord(index, code)),
 * a negative one sends x/y to GetStageMapChunkRecordXY. */
extern s32 GetStageMapChunkRecord(s32 index, s32 sub);
extern void GetStageMapChunkRecordXY(s32 index, s32 x, s32 y);

/* ObjM__TeardownStyle's helpers (src/DreamAux.c, src/ObjMStyleActor.c). */
extern void ReleaseDreamAuxEntities(void);
extern void StyleTeardown(void);

#endif
