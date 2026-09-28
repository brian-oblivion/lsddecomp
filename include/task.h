#ifndef TASK_H
#define TASK_H

#include "common.h"
#include "basic_class.h"
#include "scene_node.h"
#include "box_fill.h"
#include "FadeBox.h"
#include "IntermediateBase.h"
#include "TaskCore.h"
#include "StreamTask.h"
#include "Viewport.h"
#include "MoviePlayer.h"
#include "draw_system.h"
#include "TextRow.h"

/*
 * Declarations shared by src/app/task.c and the unit after it,
 * screen_widgets.c (FadeBox, BoxFill, TextRow): the data and outside callees
 * they reach that no class header owns. The classes are in their own headers: StreamTask.h,
 * TaskCore.h, IntermediateBase.h, Viewport.h, box_fill.h, FadeBox.h, TextRow.h.
 */

/* Defined in task.c: &sDefaultMovieFrame. */
extern DrawRect *GetDefaultMovieFrame(void);

#endif
