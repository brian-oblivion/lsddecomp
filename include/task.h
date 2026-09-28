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

/**
 * @file task.h
 * @brief What src/app/task.c shares with the unit after it,
 * src/ui/screen_widgets.c (FadeBox, BoxFill, TextRow), that no class header
 * owns: the default movie frame's getter, and the class headers both use.
 *
 * The classes themselves are in their own headers: stream_task.h,
 * task_core.h, intermediate_base.h, viewport.h, box_fill.h, fade_box.h and
 * text_row.h.
 */

/** @brief The default movie frame, {x 640, y 0, w 320, h 240}: StreamTask's
 * default initData and its MoviePlayer's frame, and the rectangle
 * TaskCore__OnInit clears when the task has no sub handle.
 * @return &sDefaultMovieFrame */
extern DrawRect *GetDefaultMovieFrame(void);

#endif
