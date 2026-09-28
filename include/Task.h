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
 * Declarations shared by src/app/Task.c and the two units after it,
 * ScreenWidgets.c (FadeBox, BoxFill's first methods) and ScreenWidgets.c (the
 * rest of BoxFill, TextRow): the data and outside callees they reach that no
 * class header owns. The classes are in their own headers: StreamTask.h,
 * TaskCore.h, IntermediateBase.h, Viewport.h, BoxFill.h, FadeBox.h, TextRow.h.
 */

/* Defined in Task.c: &gDefaultMovieFrame. */
extern DrawRect *GetDefaultMovieFrame(void);

/* resetCounters' colours for setColors, three RGB triples back to back:
 * baseColor {0, 0, 0}, the clear colour {0, 0, 0}, the third {128, 128, 128}. */
extern u8 sTaskCoreDefaultColors[3][3];

/* {x 640, y 0, w 320, h 240}: the default movie frame. TaskCore__OnInit clears
 * it to baseColor when the task has no sub handle. The same three words are
 * StreamTask's default initData and its MoviePlayer's frame, through
 * GetDefaultMovieFrame. */
extern DrawRect gDefaultMovieFrame;

/* {0, 0, 0}: TaskCore__OnInit attaches the view with it as both the
 * viewpoint and the reference point. */
extern LongVec3 sTaskCoreViewOrigin;

/* Viewport's ctor data: gViewportFadeBoxSize is the (320, 240) it passes
 * New_FadeBox; gFadeBoxAttachPos the (-100, -100) screen position the ctor
 * and SetSubHandle attach the sub handle at; gDefaultViewTwist ({0, 1}) is
 * Viewport__AttachViewChild's twist when its own is NULL. */
extern u8 gViewportFadeBoxSize[];
extern u8 gFadeBoxAttachPos[];
extern Ratio16 gDefaultViewTwist;

/* FadeBox's colour tables, eight RGB entries each, indexed at a 3-byte
 * stride by a channel mask: gFadeBoxMaskColors holds each mask's own
 * channels at 0xFF (0 and 7 white), gFadeBoxBlackColors is all black.
 * gBoxFillDefaultColor ({128, 128, 128}) is BoxFill__Reset's colour when it
 * is given none. */
extern u8 gFadeBoxMaskColors[];
extern u8 gFadeBoxBlackColors[];
extern u8 gBoxFillDefaultColor[3];

#endif
