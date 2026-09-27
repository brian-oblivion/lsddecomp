#ifndef ITEMLIST_H
#define ITEMLIST_H

#include "BasicClass.h"

/*
 * ItemList -- a list of strings the player picks one from (class id 0x20,
 * method table gItemListMethods): BasicClass's direct subclass, no class
 * below it. Methods in src/class_3bb8c_j.c (New_ItemList .. DetachTarget)
 * and src/class_3bb8c_k.c (SetState .. GetItemListMethods).
 *
 * The name is for what its own methods do:
 *  - the ctor copies a NULL-terminated list of item strings into buffers of
 *    its own (`texts`, lengths in `textLens`, the longest in `maxTextLen`;
 *    mode 1 decodes full-width SJIS and halves each length).
 *  - loadResources makes `panelSprite` (CARD\SELECT.TIM) and, through
 *    createRows, up to four TextRows of 26 characters (CARD\FONTICON.TIM);
 *    every list method does nothing while `panelSprite` is NULL.
 *  - cursorUp/cursorDown move `cursorIndex` (scrolling `topIndex` at the
 *    window's edges), scrollLeft/Right move `column` inside every string; the
 *    cursor row is coloured gItemListCursorColor, the others
 *    gItemListRowColor.
 *  - handleInputCode (the tag-2 child's notifications): 25 closes with
 *    result 2, 23 with result 3; setState(4) then passes `result` to
 *    notifyParents, and the parent reads the chosen item with getCursorIndex.
 *
 * Lifecycle. Its one maker is TaskObjF__AttachItemList (class_3bb8c_g):
 * New_ItemList(titles, 1), addChild, loadResources, attachTarget(input
 * source, tick source, sound); TaskObjF__DetachItemList undoes it
 * (detachTarget, releaseResources, release). TaskObjF's onItemListResult
 * reads the chosen item on result 2 (getCursorIndex into its
 * selectedIndex, then state 0xE) and ends on result 3 (state 0x17, a
 * terminal state). The strings are TaskObjF's `titles`, the titles of the
 * save files already on the memory card, so in the game this is the
 * load-file picker (include/TaskObjF.h, beginLoad). It is TextEntry's
 * sibling, driven the same way: slots +0x044..+0x060 and fields
 * +0x02C..+0x03C line up one for one (include/TextEntry.h).
 *
 * +0x058 tickClosing: the slot passes (sender, event) because onNotify's
 * bytes set $a1/$a2 for it; its occupant reads only self (TextEntry's
 * tickState is the same). +0x084/+0x088 cursorUp/cursorDown and +0x098
 * stepCursorInView keep their CALLERS' argument lists: handleInputCode calls
 * cursorUp/Down with self alone, while the occupants take three more words
 * and forward the third to stepCursorInView, whose occupant reads two.
 *
 * The object is 0x54 bytes (New_ItemList).
 */

/* The objects it holds, by tag (`target` is TaskObjF's `sound`, a
 * VabStreamObj, as TextEntry's is). */
struct SceneNode;
struct ScreenSprite;
struct TextRow;
struct TimImage;
struct SpriteRgb;
struct VabStreamObj;

typedef struct ItemList ItemList;
typedef struct ItemListMethods ItemListMethods;

/* `result` when closing (handleInputCode): command 25 closes with the item
 * under the cursor chosen (getCursorIndex), command 23 without one. */
enum ItemListResult { ITEMLIST_RESULT_CHOSEN = 2, ITEMLIST_RESULT_CANCELLED = 3 };

/* BasicClass's slots (overrides: +0x008 ItemList__ItemList, +0x00C
 * Finalize, +0x010 AddChild, +0x014 RemoveChild, +0x018 RemoveAllChildren,
 * +0x038 OnNotify; `tools/classtable.py gItemListMethods --vs gBasicClassMethods`),
 * then this class's own. +0x064..+0x078 are NULL in the table. */
struct ItemListMethods {
    BASICCLASS_SLOTS(ItemList, (ItemList * self, char **items, s32 mode));
    /* +0x040 */ void (*resetView)(ItemList *self); /* ItemList__ResetView; the ctor's tail */
    /* +0x044 */ void (*loadResources)(ItemList *self, struct SceneNode *parent); /* ItemList__LoadResources */
    /* +0x048 */ void (*releaseResources)(ItemList *self); /* ItemList__ReleaseResources */
    /* +0x04C */ void (*attachTarget)(ItemList *self, void *child1, void *child2,
                                      struct VabStreamObj *target); /* ItemList__AttachTarget */
    /* +0x050 */ void (*detachTarget)(ItemList *self);              /* ItemList__DetachTarget */
    /* +0x054 */ void (*setState)(ItemList *self, s32 state); /* ItemList__SetState: 2/3 close, 4 notifies parents */
    /* +0x058 */ void (*tickClosing)(ItemList *self, void *sender,
                                     s32 event); /* ItemList__TickClosing (reads only self; see the banner) */
    /* +0x05C */ void (*handleInputCode)(ItemList *self, void *source, s32 code); /* ItemList__HandleInputCode */
    /* +0x060 */ void (*playSound)(ItemList *self, s32 tone); /* ItemList__PlaySound */
    /* +0x064 */ void *slot64[6];                             /* NULL */
    /* +0x07C */ void (*scrollRight)(ItemList *self);         /* ItemList__ScrollRight */
    /* +0x080 */ void (*scrollLeft)(ItemList *self);          /* ItemList__ScrollLeft */
    /* +0x084 */ void (*cursorUp)(ItemList *self);   /* ItemList__CursorUp (see the banner) */
    /* +0x088 */ void (*cursorDown)(ItemList *self); /* ItemList__CursorDown (see the banner) */
    /* +0x08C */ void (*createRows)(ItemList *self, struct SceneNode *parent, struct TimImage *font,
                                    s32 top, s32 column, s32 cursor); /* ItemList__CreateRows */
    /* +0x090 */ void (*releaseRows)(ItemList *self);                 /* ItemList__ReleaseRows */
    /* +0x094 */ void (*refreshRows)(ItemList *self, s32 top, s32 column, s32 cursor,
                                     s32 notify); /* ItemList__RefreshRows */
    /* +0x098 */ void (*stepCursorInView)(ItemList *self, s32 dir, s32 notify,
                                          s32 forwarded); /* ItemList__StepCursorInView (see the banner) */
    /* +0x09C */ s32 (*getCursorIndex)(ItemList *self); /* ItemList__GetCursorIndex */
};

struct ItemList {
    BASICCLASS_FIELDS(ItemListMethods);
    /* +0x00C */ s32 mode; /* the ctor's mode: 1 = the item strings are full-width SJIS */
    /* +0x010 */ s32 itemCount; /* the ctor counts the NULL-terminated list; rows shown = min(itemCount, 4) */
    /* +0x014 */ s32 maxTextLen;  /* the ctor: the longest item; bounds `column` in scrollRight */
    /* +0x018 */ char **texts;    /* the ctor: one buffer per item */
    /* +0x01C */ s32 *textLens;   /* the ctor: each item's length; finalize frees it */
    /* +0x020 */ s32 topIndex;    /* the item shown in row 0; resetView zeroes, setView sets */
    /* +0x024 */ s32 column;      /* character offset into every item (horizontal scroll) */
    /* +0x028 */ s32 cursorIndex; /* the highlighted item; getCursorIndex returns it */
    /* +0x02C */ s32 result; /* setState: 2 or 3 when closing, passed to notifyParents by state 4; attachTarget zeroes */
    /* +0x030 */ s32 closeTicks;    /* tickClosing's call counter; setState zeroes */
    /* +0x034 */ void *inputSource; /* addChild/removeChild: the child whose class id's low nibble is 2; onNotify sends its events to handleInputCode */
    /* +0x038 */ void *tickSource; /* ... whose low nibble is 5; onNotify sends its events to tickClosing */
    /* +0x03C */ struct VabStreamObj *target; /* attachTarget; ItemList__PlaySound plays a tone on it (playTone, volume 96, 96) */
    /* +0x040 */ struct TextRow *rows[4]; /* createRows: New_TextRow(font, 26, ...); index = item - topIndex */
    /* +0x050 */ struct ScreenSprite *panelSprite; /* loadResources: New_ScreenSprite(SELECT); non-NULL gates every list method */
};

extern ItemListMethods gItemListMethods;
extern ItemListMethods *GetItemListMethods(void); /* returns &gItemListMethods */

/* The row colours, two 3-byte RGBs in sdata, 4 bytes apart; only their
 * addresses are taken (setColor). */
extern struct SpriteRgb gItemListRowColor;
extern struct SpriteRgb gItemListCursorColor;

/* The overrides whose parameter lists differ from their slot's (see the
 * banner), as a caller that passes the extra arguments casts them: attachTarget
 * reaches its own addChild override with all four of its words once. */
typedef void (*ItemListAddChildWideFn)(ItemList *self, void *child1, void *child2,
                                       struct VabStreamObj *target);

/* The class's own methods, in address order. */
ItemList *New_ItemList(char **items, s32 mode);
void ItemList__ItemList(ItemList *self, char **items, s32 mode);
void ItemList__ClearCachedRefs(ItemList *self);
void ItemList__Finalize(ItemList *self);
void ItemList__AddChild(ItemList *self, void *child);
void ItemList__RemoveChild(ItemList *self, void *child);
void ItemList__RemoveAllChildren(ItemList *self);
void ItemList__OnNotify(ItemList *self, void *sender, s32 event);
void ItemList__ResetView(ItemList *self);
void ItemList__LoadResources(ItemList *self, struct SceneNode *parent);
void ItemList__ReleaseResources(ItemList *self);
void ItemList__AttachTarget(ItemList *self, void *child1, void *child2, struct VabStreamObj *target);
void ItemList__DetachTarget(ItemList *self);
void ItemList__SetState(ItemList *self, s32 state);
void ItemList__TickClosing(ItemList *self);
void ItemList__HandleInputCode(ItemList *self, void *source, s32 code);
void ItemList__PlaySound(ItemList *self, s32 tone);
void ItemList__ScrollRight(ItemList *self);
void ItemList__ScrollLeft(ItemList *self);
void ItemList__CursorUp(ItemList *self, s32 unused1, s32 unused2, s32 forwarded);
void ItemList__CursorDown(ItemList *self, s32 unused1, s32 unused2, s32 forwarded);
void ItemList__CreateRows(ItemList *self, struct SceneNode *parent, struct TimImage *font, s32 top,
                          s32 column, s32 cursor);
void ItemList__ReleaseRows(ItemList *self);
void ItemList__RefreshRows(ItemList *self, s32 top, s32 column, s32 cursor, s32 notify);
char *ItemList__FormatRowText(ItemList *self, char *dest, s32 row, s32 top, s32 column);
void ItemList__SetView(ItemList *self, s32 top, s32 column, s32 cursor, s32 highlight);
void ItemList__StepCursorInView(ItemList *self, s32 dir, s32 notify);
s32 ItemList__GetCursorIndex(ItemList *self);

#endif
