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

/* The pool allocator's pair. BMemPMgrFree returns NULL (its body ends
 * `addu $v0, $zero, $zero`), and ScreenWidgets's caller uses the result. */
extern void *BMemPMgrAlloc(s32 size);
extern void *BMemPMgrFree(void *ptr);

/* Releases each element of a BasicClass array (TaskCore's slot elements,
 * TextRow's children). */
extern void ReleaseBasicClassArray(void *array, void *count);

/* The packed-bitfield accessor SceneNode's attribute setters use
 * (SceneNode.c), reached here by BoxFill's over `&self->boxAttribute`. */
extern u32 GetSetBitField(u32 *word, s32 shift, s32 width, u32 value);

/* LIBGPU.H's `int ResetGraph(int)`, declared with the project's types;
 * Viewport__Flip calls it with 1 and ignores the result. */
extern s32 ResetGraph(s32 mode);

/* Defined in Task.c: &sDefaultMovieFrame. */
extern DrawRect *GetDefaultMovieFrame(void);

/* {x 640, y 0, w 320, h 240}: the default movie frame. TaskCore__OnInit clears
 * it to baseColor when the task has no sub handle. The same three words are
 * StreamTask's default initData and its MoviePlayer's frame, through
 * GetDefaultMovieFrame. */
extern DrawRect sDefaultMovieFrame;

/* Viewport's ctor data: gViewportFadeBoxSize is the (320, 240) it passes
 * New_FadeBox; gFadeBoxAttachPos the (-100, -100) screen position the ctor
 * and SetSubHandle attach the sub handle at; sDefaultViewTwist ({0, 1}) is
 * Viewport__AttachViewChild's twist when its own is NULL. */
extern u8 gViewportFadeBoxSize[];
extern u8 gFadeBoxAttachPos[];
extern Ratio16 sDefaultViewTwist;

/* FadeBox's colour tables, eight RGB entries each, indexed at a 3-byte
 * stride by a channel mask: gFadeBoxMaskColors holds each mask's own
 * channels at 0xFF (0 and 7 white), gFadeBoxBlackColors is all black.
 * sBoxFillDefaultColor ({128, 128, 128}) is BoxFill__Reset's colour when it
 * is given none. */
extern u8 gFadeBoxMaskColors[];
extern u8 gFadeBoxBlackColors[];
extern u8 sBoxFillDefaultColor[3];

#endif
