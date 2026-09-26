#ifndef CLASS86F88_H
#define CLASS86F88_H

#include "BasicClass.h"

/*
 * Class86F88 -- a scrolling list selector (class id 0x20, method table
 * gClass86F88Methods): BasicClass's direct subclass, no class below it.
 * Methods in src/class_3bb8c_j.c (New_Class86F88 .. DetachTarget) and
 * src/class_3bb8c_k.c (SetState .. GetClass86F88Methods). What its own
 * methods do:
 *  - the ctor copies a NULL-terminated list of item strings into buffers of
 *    its own (`texts`, lengths in `textLens`, the longest in `maxTextLen`;
 *    mode 1 decodes full-width SJIS and halves each length).
 *  - loadResources makes `panelSprite` (CARD\SELECT.TIM) and, through
 *    createRows, up to four TextRows of 26 characters (CARD\FONTICON.TIM);
 *    every list method does nothing while `panelSprite` is NULL.
 *  - cursorUp/cursorDown move `cursorIndex` (scrolling `topIndex` at the
 *    window's edges), scrollLeft/Right move `column` inside every string; the
 *    cursor row is coloured gClass86F88CursorColor, the others
 *    gClass86F88RowColor.
 *  - handleInputCode (the tag-2 child's notifications): 25 closes with
 *    result 2, 23 with result 3; setState(4) then passes `result` to
 *    notifyParents, and the parent reads the chosen item with getCursorIndex.
 * Its one maker is TaskObjF__AttachChildB (class_3bb8c_g, mode 1), which
 * drives loadResources/attachTarget/detachTarget/releaseResources exactly as
 * TaskObjF__AttachChildA drives TextEntry's same slots (include/TextEntry.h:
 * slots +0x044..+0x060 and fields +0x02C..+0x03C line up one for one). What
 * the list holds in the game is not established.
 *
 * +0x058 tickClosing: the slot passes (sender, event) because onNotify's
 * bytes set $a1/$a2 for it; its occupant reads only self (TextEntry's
 * tickState is the same). +0x084/+0x088 cursorUp/cursorDown and +0x098
 * stepCursorInView keep their CALLERS' argument lists: handleInputCode calls
 * cursorUp/Down with self alone, while the occupants take three more words
 * and forward the third to stepCursorInView, whose occupant reads two.
 *
 * The object is 0x54 bytes (New_Class86F88).
 */

/* The objects it holds, by tag (TargetObj86ED0 is a helper view in
 * include/class_3bb8c.h: TaskObjF's childC, TextEntry's target too). */
struct Class6B5CC;
struct ScreenSprite;
struct TextRow;
struct TimImage;
struct SpriteRgb;
struct TargetObj86ED0;

typedef struct Class86F88 Class86F88;
typedef struct Class86F88Methods Class86F88Methods;

/* BasicClass's slots (overrides: +0x008 Class86F88__Class86F88, +0x00C
 * Finalize, +0x010 AddChild, +0x014 RemoveChild, +0x018 RemoveAllChildren,
 * +0x038 OnNotify; `tools/classtable.py gClass86F88Methods --vs D_8006B58C`),
 * then this class's own. +0x064..+0x078 are NULL in the table. */
struct Class86F88Methods {
    BASICCLASS_SLOTS(Class86F88, (Class86F88 *self, char **items, s32 mode));
    /* +0x040 */ void (*resetView)(Class86F88 *self);                          /* Class86F88__ResetView; the ctor's tail */
    /* +0x044 */ void (*loadResources)(Class86F88 *self, struct Class6B5CC *parent); /* Class86F88__LoadResources */
    /* +0x048 */ void (*releaseResources)(Class86F88 *self);                   /* Class86F88__ReleaseResources */
    /* +0x04C */ void (*attachTarget)(Class86F88 *self, void *child1, void *child2, struct TargetObj86ED0 *target); /* Class86F88__AttachTarget */
    /* +0x050 */ void (*detachTarget)(Class86F88 *self);                       /* Class86F88__DetachTarget */
    /* +0x054 */ void (*setState)(Class86F88 *self, s32 state);                /* Class86F88__SetState: 2/3 close, 4 notifies parents */
    /* +0x058 */ void (*tickClosing)(Class86F88 *self, void *sender, s32 event); /* Class86F88__TickClosing (reads only self; see the banner) */
    /* +0x05C */ void (*handleInputCode)(Class86F88 *self, void *source, s32 code); /* Class86F88__HandleInputCode */
    /* +0x060 */ void (*forwardToTarget)(Class86F88 *self, s32 code);          /* Class86F88__ForwardToTarget */
    /* +0x064 */ void *slot64[6];                                              /* NULL */
    /* +0x07C */ void (*scrollRight)(Class86F88 *self);                        /* Class86F88__ScrollRight */
    /* +0x080 */ void (*scrollLeft)(Class86F88 *self);                         /* Class86F88__ScrollLeft */
    /* +0x084 */ void (*cursorUp)(Class86F88 *self);                           /* Class86F88__CursorUp (see the banner) */
    /* +0x088 */ void (*cursorDown)(Class86F88 *self);                         /* Class86F88__CursorDown (see the banner) */
    /* +0x08C */ void (*createRows)(Class86F88 *self, struct Class6B5CC *parent, struct TimImage *font, s32 top, s32 column, s32 cursor); /* Class86F88__CreateRows */
    /* +0x090 */ void (*releaseRows)(Class86F88 *self);                        /* Class86F88__ReleaseRows */
    /* +0x094 */ void (*refreshRows)(Class86F88 *self, s32 top, s32 column, s32 cursor, s32 notify); /* Class86F88__RefreshRows */
    /* +0x098 */ void (*stepCursorInView)(Class86F88 *self, s32 dir, s32 notify, s32 arg3); /* Class86F88__StepCursorInView (see the banner) */
    /* +0x09C */ s32 (*getCursorIndex)(Class86F88 *self);                      /* Class86F88__GetCursorIndex */
};

struct Class86F88 {
    BASICCLASS_FIELDS(Class86F88Methods);
    /* +0x00C */ s32 mode;           /* the ctor's mode: 1 = the item strings are full-width SJIS */
    /* +0x010 */ s32 itemCount;      /* the ctor counts the NULL-terminated list; rows shown = min(itemCount, 4) */
    /* +0x014 */ s32 maxTextLen;     /* the ctor: the longest item; bounds `column` in scrollRight */
    /* +0x018 */ char **texts;       /* the ctor: one buffer per item */
    /* +0x01C */ s32 *textLens;      /* the ctor: each item's length; finalize frees it */
    /* +0x020 */ s32 topIndex;       /* the item shown in row 0; resetView zeroes, setView sets */
    /* +0x024 */ s32 column;         /* character offset into every item (horizontal scroll) */
    /* +0x028 */ s32 cursorIndex;    /* the highlighted item; getCursorIndex returns it */
    /* +0x02C */ s32 result;         /* setState: 2 or 3 when closing, passed to notifyParents by state 4; attachTarget zeroes */
    /* +0x030 */ s32 closeTicks;     /* tickClosing's call counter; setState zeroes */
    /* +0x034 */ void *inputSource;  /* addChild/removeChild: the child whose class id's low nibble is 2; onNotify sends its events to handleInputCode */
    /* +0x038 */ void *tickSource;   /* ... whose low nibble is 5; onNotify sends its events to tickClosing */
    /* +0x03C */ struct TargetObj86ED0 *target; /* attachTarget; forwardToTarget calls its +0x080 with (code, 0x60, 0x60) */
    /* +0x040 */ struct TextRow *rows[4];       /* createRows: New_TextRow(font, 26, ...); index = item - topIndex */
    /* +0x050 */ struct ScreenSprite *panelSprite; /* loadResources: New_ScreenSprite(SELECT); non-NULL gates every list method */
};

extern Class86F88Methods gClass86F88Methods;
extern Class86F88Methods *GetClass86F88Methods(void); /* returns &gClass86F88Methods */

/* The row colours, two 3-byte RGBs in sdata, 4 bytes apart; only their
 * addresses are taken (setColor). */
extern struct SpriteRgb gClass86F88RowColor;
extern struct SpriteRgb gClass86F88CursorColor;

/* The overrides whose parameter lists differ from their slot's (see the
 * banner), as a caller that passes the extra arguments casts them: attachTarget
 * reaches its own addChild override with all four of its words once. */
typedef void (*Class86F88AddChildWideFn)(Class86F88 *self, void *child1, void *child2, struct TargetObj86ED0 *target);

/* The class's own methods, in address order. */
Class86F88 *New_Class86F88(char **items, s32 mode);
void Class86F88__Class86F88(Class86F88 *self, char **items, s32 mode);
void Class86F88__ClearCachedRefs(Class86F88 *self);
void Class86F88__Finalize(Class86F88 *self);
void Class86F88__AddChild(Class86F88 *self, void *child);
void Class86F88__RemoveChild(Class86F88 *self, void *child);
void Class86F88__RemoveAllChildren(Class86F88 *self);
void Class86F88__OnNotify(Class86F88 *self, void *sender, s32 event);
void Class86F88__ResetView(Class86F88 *self);
void Class86F88__LoadResources(Class86F88 *self, struct Class6B5CC *parent);
void Class86F88__ReleaseResources(Class86F88 *self);
void Class86F88__AttachTarget(Class86F88 *self, void *child1, void *child2, struct TargetObj86ED0 *target);
void Class86F88__DetachTarget(Class86F88 *self);
void Class86F88__SetState(Class86F88 *self, s32 state);
void Class86F88__TickClosing(Class86F88 *self);
void Class86F88__HandleInputCode(Class86F88 *self, void *source, s32 code);
void Class86F88__ForwardToTarget(Class86F88 *self, s32 code);
void Class86F88__ScrollRight(Class86F88 *self);
void Class86F88__ScrollLeft(Class86F88 *self);
void Class86F88__CursorUp(Class86F88 *self, s32 arg1, s32 arg2, s32 arg3);
void Class86F88__CursorDown(Class86F88 *self, s32 arg1, s32 arg2, s32 arg3);
void Class86F88__CreateRows(Class86F88 *self, struct Class6B5CC *parent, struct TimImage *font, s32 top, s32 column, s32 cursor);
void Class86F88__ReleaseRows(Class86F88 *self);
void Class86F88__RefreshRows(Class86F88 *self, s32 top, s32 column, s32 cursor, s32 notify);
char *Class86F88__FormatRowText(Class86F88 *self, char *dest, s32 row, s32 top, s32 column);
void Class86F88__SetView(Class86F88 *self, s32 top, s32 column, s32 cursor, s32 highlight);
void Class86F88__StepCursorInView(Class86F88 *self, s32 dir, s32 notify);
s32 Class86F88__GetCursorIndex(Class86F88 *self);

#endif
