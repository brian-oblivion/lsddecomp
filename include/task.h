#ifndef TASK_H
#define TASK_H

#include "common.h"
#include "basic_class.h"
#include "scene_node.h"
#include "box_fill.h"
#include "fade_box.h"
#include "intermediate_base.h"
#include "task_core.h"
#include "stream_task.h"
#include "viewport.h"
#include "movie_player.h"
#include "draw_system.h"
#include "text_row.h"

/*
 * Declarations shared by src/app/task.c and the unit after it,
 * screen_widgets.c (FadeBox, BoxFill, TextRow): the data and outside callees
 * they reach that no class header owns. The classes are in their own headers: stream_task.h,
 * task_core.h, intermediate_base.h, viewport.h, box_fill.h, fade_box.h, text_row.h.
 */

/* Defined in task.c: &sDefaultMovieFrame. */
extern DrawRect *GetDefaultMovieFrame(void);

#endif
