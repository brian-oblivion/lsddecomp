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

#endif
