#ifndef TASK_H
#define TASK_H

#include "common.h"
#include "BasicClass.h"
#include "SceneNode.h"
#include "BoxFill.h"
#include "FadeBox.h"
#include "IntermediateBase.h"
#include "TaskCore.h"
#include "StreamTask.h"
#include "Viewport.h"
#include "MoviePlayer.h"
#include "DrawSystem.h"
#include "TextRow.h"

/*
 * Declarations shared by src/app/Task.c and the unit after it,
 * ScreenWidgets.c (FadeBox, BoxFill, TextRow): the data and outside callees
 * they reach that no class header owns. The classes are in their own headers: StreamTask.h,
 * TaskCore.h, IntermediateBase.h, Viewport.h, BoxFill.h, FadeBox.h, TextRow.h.
 */

/* Defined in Task.c: &sDefaultMovieFrame. */
extern DrawRect *GetDefaultMovieFrame(void);

#endif
