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

#endif
