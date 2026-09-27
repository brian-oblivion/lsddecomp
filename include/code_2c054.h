#ifndef CODE_2C054_H
#define CODE_2C054_H

#include "common.h"
#include "TaskCore.h"
#include "StreamTask.h"
#include "Viewport.h"
#include "MoviePlayer.h"
#include "DrawSystem.h"

/*
 * code_2c054's own declarations: the data, and the one outside accessor,
 * that StreamTask's and TaskCore's methods here reach and no other unit
 * does. The classes themselves are in include/StreamTask.h and
 * include/TaskCore.h.
 */

/* Defined in code_2cc8c_c.c (returning void *): &gDefaultStreamTaskInitData. */
extern StreamTaskInitData *GetDefaultStreamTaskInitData(void);

/* resetCounters' colours for setColors, three RGB triples back to back:
 * baseColor {0, 0, 0}, the clear colour {0, 0, 0}, the third {128, 128, 128}. */
extern u8 sTaskCoreDefaultColors[];

/* {x 640, y 0, w 320, h 240}: the default movie frame. TaskCore__OnInit clears
 * it to baseColor when the task has no sub handle. The same three words are
 * StreamTask's default initData and its MoviePlayer's frame, through
 * GetDefaultStreamTaskInitData. */
extern DrawRect gDefaultStreamTaskInitData;

/* {0, 0, 0}: TaskCore__OnInit attaches the view with it as both the
 * viewpoint and the reference point. */
extern LongVec3 sTaskCoreViewOrigin;

#endif
