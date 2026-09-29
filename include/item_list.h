#ifndef ITEM_LIST_H
#define ITEM_LIST_H

#include "basic_class.h"

/**
 * @file item_list.h
 * @brief ItemList, a scrolling list of strings the player picks one from:
 *        the memory card's load-file picker.
 *
 * Methods in src/ui/input_dialogs.c (New_ItemList .. ItemList__DetachTarget)
 * and src/ui/item_list.c (ItemList__SetState .. GetItemListMethods).
 */

/* The objects it holds, by tag (`target` is TaskObjF's `sound`, a
 * VabStreamObj, as TextEntry's is). */
struct SceneNode;
struct ScreenSprite;
struct TextRow;
struct TimImage;
struct ColorRgb;
struct VabStreamObj;

/** ItemList's class id (gItemListMethods word +0x000). Two nibbles, so
 * `(header & CLASS_ID_LEVEL2_MASK) == ITEMLIST_CLASS_ID` is its is-kind-of test (TaskObjF's
 * addChild/removeChild/onNotify). */
#define ITEMLIST_CLASS_ID 0x20

typedef struct ItemList ItemList;
typedef struct ItemListMethods ItemListMethods;

/**
 * @brief The ctor's `mode`, kept in ItemList::mode. TaskObjF passes
 * ITEMLIST_MODE_FULLWIDTH.
 */
enum ItemListMode {
    ITEMLIST_MODE_PLAIN = 0,    /**< Each item string is copied as it is. */
    ITEMLIST_MODE_FULLWIDTH = 1 /**< Full-width SJIS: length halved, decoded by DecodeFullWidthSjis. */
};

/**
 * @brief ItemList::result when the list closes (handleInputCode).
 */
enum ItemListResult {
    ITEMLIST_RESULT_CHOSEN = 2, /**< Circle (pad event 25): the item under the cursor is chosen (getCursorIndex). */
    ITEMLIST_RESULT_CANCELLED = 3 /**< Cross (pad event 23): closed without a choice. */
};

/** setState's last state: notify the parents with `result`. tickClosing
 * enters it on the second tick after a close. */
#define ITEMLIST_STATE_REPORT 4

/** The characters one row shows: New_TextRow's width, and FormatRowText pads
 * or cuts every item's text at `column` to it. */
#define ITEMLIST_ROW_CHARS 26

/** @name Tones
 * playSound's tones, VabStreamObj__PlayTone indices (program << 4 | tone). @{ */
#define ITEMLIST_TONE_CURSOR 0x00 /**< refreshRows, stepCursorInView: the cursor moved */
#define ITEMLIST_TONE_BUTTON 0x10 /**< circle or cross: the list closes */
#define ITEMLIST_TONE_VOLUME 96   /**< playSound's PlayTone vol and endVol */

/** @} */

/**
 * @brief ItemList's method table: BasicClass's fifteen slots, with a ctor
 * that takes (items, mode), then its own.
 *
 * Overrides of BasicClass's slots: +0x008 ItemList__ItemList, +0x00C
 * ItemList__Finalize, +0x010 ItemList__AddChild, +0x014
 * ItemList__RemoveChild, +0x018 ItemList__RemoveAllChildren and +0x038
 * ItemList__OnNotify. +0x064..+0x078 are NULL in the table.
 *
 * Three slots are typed for their CALLERS, not their occupants: tickClosing
 * takes the (sender, event) onNotify passes, and its occupant reads only
 * self; cursorUp and cursorDown take self alone, as handleInputCode calls
 * them, while their occupants take three more words and pass the third on to
 * stepCursorInView, whose occupant reads two.
 */
struct ItemListMethods {
    BASICCLASS_SLOTS(ItemList, (ItemList * self, char **items, s32 mode));
    /* +0x040 */ void (*resetView)(ItemList *self); /**< @see ItemList__ResetView */
    /* +0x044 */ void (*loadResources)(ItemList *self, struct SceneNode *parent); /**< @see ItemList__LoadResources */
    /* +0x048 */ void (*releaseResources)(ItemList *self); /**< @see ItemList__ReleaseResources */
    /* +0x04C */ void (*attachTarget)(ItemList *self, void *inputSource, void *tickSource,
                                      struct VabStreamObj *target); /**< @see ItemList__AttachTarget */
    /* +0x050 */ void (*detachTarget)(ItemList *self);        /**< @see ItemList__DetachTarget */
    /* +0x054 */ void (*setState)(ItemList *self, s32 state); /**< @see ItemList__SetState */
    /* +0x058 */ void (*tickClosing)(ItemList *self, void *sender, s32 event); /**< @see ItemList__TickClosing */
    /* +0x05C */ void (*handleInputCode)(ItemList *self, void *source,
                                         s32 code);           /**< @see ItemList__HandleInputCode */
    /* +0x060 */ void (*playSound)(ItemList *self, s32 tone); /**< @see ItemList__PlaySound */
    /* +0x064 */ void *pad64[6];                              /* NULL; nothing calls them */
    /* +0x07C */ void (*scrollRight)(ItemList *self);         /**< @see ItemList__ScrollRight */
    /* +0x080 */ void (*scrollLeft)(ItemList *self);          /**< @see ItemList__ScrollLeft */
    /* +0x084 */ void (*cursorUp)(ItemList *self);            /**< @see ItemList__CursorUp */
    /* +0x088 */ void (*cursorDown)(ItemList *self);          /**< @see ItemList__CursorDown */
    /* +0x08C */ void (*createRows)(ItemList *self, struct SceneNode *parent, struct TimImage *font,
                                    s32 top, s32 column, s32 cursor); /**< @see ItemList__CreateRows */
    /* +0x090 */ void (*releaseRows)(ItemList *self); /**< @see ItemList__ReleaseRows */
    /* +0x094 */ void (*refreshRows)(ItemList *self, s32 top, s32 column, s32 cursor,
                                     s32 notify); /**< @see ItemList__RefreshRows */
    /* +0x098 */ void (*stepCursorInView)(ItemList *self, s32 dir, s32 notify,
                                          s32 forwarded); /**< @see ItemList__StepCursorInView */
    /* +0x09C */ s32 (*getCursorIndex)(ItemList *self);   /**< @see ItemList__GetCursorIndex */
};

/**
 * @brief A list of strings the player picks one from (class id 0x20). A
 * direct BasicClass subclass; no class derives from it.
 *
 * The ctor copies a NULL-terminated list of item strings into buffers of its
 * own. loadResources makes `panelSprite` (CARD\\SELECT.TIM) and, through
 * createRows, up to four TextRows of ITEMLIST_ROW_CHARS characters
 * (CARD\\FONTICON.TIM); every list method does nothing while `panelSprite`
 * is NULL. cursorUp/cursorDown move `cursorIndex`, scrolling `topIndex` at
 * the window's edges, and scrollLeft/scrollRight move `column` inside every
 * string; the cursor row is coloured sItemListCursorColor, the others
 * sItemListRowColor. handleInputCode answers the Pad child's events: Circle
 * closes with ITEMLIST_RESULT_CHOSEN, Cross with ITEMLIST_RESULT_CANCELLED,
 * and two ticks later setState(ITEMLIST_STATE_REPORT) passes `result` to
 * notifyParents; the parent reads the chosen item with getCursorIndex.
 *
 * Lifecycle: its one maker is TaskObjF__AttachItemList (src/ui/title_menu.c):
 * New_ItemList(titles, ITEMLIST_MODE_FULLWIDTH), addChild, loadResources,
 * attachTarget(input source, tick source, sound); TaskObjF__DetachItemList
 * undoes it (detachTarget, releaseResources, release). The strings are the
 * titles of the save files on the memory card, so in the game this is the
 * load-file picker (include/task_objf.h). It is TextEntry's sibling, driven
 * the same way: slots +0x044..+0x060 and fields +0x02C..+0x03C line up one
 * for one (include/text_entry.h).
 */
struct ItemList {
    BASICCLASS_FIELDS(ItemListMethods);
    /* +0x00C */ s32 mode; /**< The ctor's mode (enum ItemListMode). */
    /* +0x010 */ s32 itemCount; /**< Entries in the ctor's NULL-terminated list; rows shown = min(itemCount, 4). */
    /* +0x014 */ s32 maxTextLen; /**< The longest item's length; bounds `column` in scrollRight. */
    /* +0x018 */ char **texts;   /**< One buffer per item, the list's own copies. */
    /* +0x01C */ s32 *textLens;  /**< Each item's length in characters. */
    /* +0x020 */ s32 topIndex; /**< The item shown in row 0; resetView zeroes it, setView sets it. */
    /* +0x024 */ s32 column;   /**< Character offset into every item (horizontal scroll). */
    /* +0x028 */ s32 cursorIndex; /**< The highlighted item; getCursorIndex returns it. */
    /* +0x02C */ s32 result; /**< enum ItemListResult once closing, passed to notifyParents; attachTarget zeroes it. */
    /* +0x030 */ s32 closeTicks; /**< tickClosing's call counter; setState zeroes it. */
    /* +0x034 */ void *inputSource; /**< The Pad child (class id low nibble 2); onNotify sends its events to handleInputCode. */
    /* +0x038 */ void *tickSource; /**< The FrameClock child (low nibble 5); onNotify sends its events to tickClosing. */
    /* +0x03C */ struct VabStreamObj *target; /**< attachTarget's sound; playSound plays a tone on it at ITEMLIST_TONE_VOLUME. */
    /* +0x040 */ struct TextRow *rows[4]; /**< The visible rows (createRows); index = item - topIndex. */
    /* +0x050 */ struct ScreenSprite *panelSprite; /**< The SELECT panel (loadResources); non-NULL gates every list method. */
}; /* 0x54 bytes: New_ItemList */

/** @brief ItemList's method table (see ItemListMethods). */
extern ItemListMethods gItemListMethods;

/**
 * @brief Returns ItemList's method table.
 * @return &gItemListMethods.
 */
extern ItemListMethods *GetItemListMethods(void);

/**
 * @brief The addChild slot as attachTarget calls it once, with all four of
 * its own words; the occupant reads only the child.
 */
typedef void (*ItemListAddChildWideFn)(ItemList *self, void *inputSource, void *tickSource,
                                       struct VabStreamObj *target);

/**
 * @brief Allocates an ItemList from the BMemPMgr pool and constructs it.
 * @param items NULL-terminated array of item strings, copied by the ctor.
 * @param mode  enum ItemListMode.
 * @return The new list, or NULL when the pool is exhausted.
 */
ItemList *New_ItemList(char **items, s32 mode);

/**
 * @brief Constructor (slot +0x008): BasicClass's ctor, then copies each item
 * into a buffer of its own (decoding full-width SJIS in that mode), records
 * the lengths and the longest, clears the cached children and resets the
 * view.
 * @param self  The object being constructed.
 * @param items NULL-terminated array of item strings.
 * @param mode  enum ItemListMode.
 */
void ItemList__ItemList(ItemList *self, char **items, s32 mode);

/**
 * @brief Clears `inputSource`, `tickSource` and `panelSprite`.
 * @param self The list.
 */
void ItemList__ClearCachedRefs(ItemList *self);

/**
 * @brief Finalizer (slot +0x00C): frees every item buffer and the two
 * arrays, then runs BasicClass's finalizer.
 * @param self The list.
 */
void ItemList__Finalize(ItemList *self);

/**
 * @brief addChild (slot +0x010): BasicClass's, then keeps a Pad child as
 * `inputSource` or a FrameClock child as `tickSource`. NULL is ignored.
 * @param self  The list.
 * @param child The child to add.
 */
void ItemList__AddChild(ItemList *self, void *child);

/**
 * @brief removeChild (slot +0x014): forgets the child if it is the input or
 * tick source, then BasicClass's removeChild. NULL is ignored.
 * @param self  The list.
 * @param child The child to remove.
 */
void ItemList__RemoveChild(ItemList *self, void *child);

/**
 * @brief removeAllChildren (slot +0x018): clears the cached children and the
 * panel sprite, then BasicClass's removeAllChildren.
 * @param self The list.
 */
void ItemList__RemoveAllChildren(ItemList *self);

/**
 * @brief onNotify (slot +0x038): BasicClass's, then a Pad sender's event goes
 * to handleInputCode and a FrameClock sender's to tickClosing.
 * @param self   The list.
 * @param sender The notifying object.
 * @param event  The event code.
 */
void ItemList__OnNotify(ItemList *self, void *sender, s32 event);

/**
 * @brief resetView (slot +0x040): zeroes `topIndex`, `column` and
 * `cursorIndex`.
 * @param self The list.
 */
void ItemList__ResetView(ItemList *self);

/**
 * @brief loadResources (slot +0x044): loads CARD\\SELECT.TIM as the panel
 * sprite under `parent` and has createRows build the rows from
 * CARD\\FONTICON.TIM. Does nothing without a parent or when already loaded.
 * @param self   The list.
 * @param parent The node the panel and rows attach to.
 */
void ItemList__LoadResources(ItemList *self, struct SceneNode *parent);

/**
 * @brief releaseResources (slot +0x048): releases the rows and the panel
 * sprite, if loaded.
 * @param self The list.
 */
void ItemList__ReleaseResources(ItemList *self);

/**
 * @brief attachTarget (slot +0x04C): adds the input and tick children, keeps
 * the sound as `target` and zeroes `result`.
 * @param self        The list.
 * @param inputSource The Pad object whose events drive the list.
 * @param tickSource  The FrameClock that times the close.
 * @param target      The sound object playSound plays on.
 */
void ItemList__AttachTarget(ItemList *self, void *inputSource, void *tickSource,
                            struct VabStreamObj *target);

/**
 * @brief detachTarget (slot +0x050): removes the input and tick children and
 * clears `target`.
 * @param self The list.
 */
void ItemList__DetachTarget(ItemList *self);

/**
 * @brief setState (slot +0x054). A result state removes the input child,
 * releases the resources and keeps the state in `result`;
 * ITEMLIST_STATE_REPORT passes `result` to notifyParents; any other state
 * does nothing. Always zeroes `closeTicks`.
 * @param self  The list.
 * @param state enum ItemListResult, or ITEMLIST_STATE_REPORT.
 */
void ItemList__SetState(ItemList *self, s32 state);

/**
 * @brief tickClosing (slot +0x058): while the list is closing, counts ticks
 * and on the second enters ITEMLIST_STATE_REPORT.
 * @param self The list.
 */
void ItemList__TickClosing(ItemList *self);

/**
 * @brief handleInputCode (slot +0x05C): Circle chooses and Cross cancels,
 * each with a sound; left/right held scroll, up/down move the cursor. Other
 * codes are ignored.
 * @param self   The list.
 * @param source The Pad object (unused).
 * @param code   The pad event code (PAD_EVENT_* + PAD_BUTTON_*).
 */
void ItemList__HandleInputCode(ItemList *self, void *source, s32 code);

/**
 * @brief playSound (slot +0x060): plays `tone` on `target` at
 * ITEMLIST_TONE_VOLUME, when there is a target.
 * @param self The list.
 * @param tone ITEMLIST_TONE_CURSOR or ITEMLIST_TONE_BUTTON.
 */
void ItemList__PlaySound(ItemList *self, s32 tone);

/**
 * @brief scrollRight (slot +0x07C): moves `column` one character right,
 * unless the longest item already fits, and redraws the rows with a click.
 * @param self The list.
 */
void ItemList__ScrollRight(ItemList *self);

/**
 * @brief scrollLeft (slot +0x080): moves `column` one character left, down
 * to 0, and redraws the rows with a click.
 * @param self The list.
 */
void ItemList__ScrollLeft(ItemList *self);

/**
 * @brief cursorUp (slot +0x084): moves the cursor to the previous item,
 * inside the window or by scrolling it up one row.
 * @param self      The list.
 * @param unused1   Not read.
 * @param unused2   Not read.
 * @param forwarded Passed on to stepCursorInView, which ignores it.
 */
void ItemList__CursorUp(ItemList *self, s32 unused1, s32 unused2, s32 forwarded);

/**
 * @brief cursorDown (slot +0x088): moves the cursor to the next item, inside
 * the window or by scrolling it down one row.
 * @param self      The list.
 * @param unused1   Not read.
 * @param unused2   Not read.
 * @param forwarded Passed on to stepCursorInView, which ignores it.
 */
void ItemList__CursorDown(ItemList *self, s32 unused1, s32 unused2, s32 forwarded);

/**
 * @brief createRows (slot +0x08C): makes up to four TextRows from `font`,
 * one per visible item, stacked under `parent`, then sets the view with the
 * cursor highlighted.
 * @param self   The list.
 * @param parent The node the rows attach to.
 * @param font   The font image the rows draw with.
 * @param top    The item shown in row 0.
 * @param column The horizontal scroll offset.
 * @param cursor The highlighted item.
 */
void ItemList__CreateRows(ItemList *self, struct SceneNode *parent, struct TimImage *font, s32 top,
                          s32 column, s32 cursor);

/**
 * @brief releaseRows (slot +0x090): releases the visible TextRows.
 * @param self The list.
 */
void ItemList__ReleaseRows(ItemList *self);

/**
 * @brief refreshRows (slot +0x094): rewrites every visible row's text for a
 * new window and scroll, records it with setView (no highlight change), and
 * clicks when `notify` is set.
 * @param self   The list.
 * @param top    The item shown in row 0.
 * @param column The horizontal scroll offset.
 * @param cursor The highlighted item.
 * @param notify Non-zero to play the click.
 */
void ItemList__RefreshRows(ItemList *self, s32 top, s32 column, s32 cursor, s32 notify);

/**
 * @brief Writes row `row`'s text into `dest`: the item's text from `column`,
 * cut or padded with spaces to ITEMLIST_ROW_CHARS and terminated.
 * @param self   The list.
 * @param dest   At least ITEMLIST_ROW_CHARS + 1 bytes.
 * @param row    The visible row, 0..3.
 * @param top    The item shown in row 0.
 * @param column The horizontal scroll offset.
 * @return dest.
 */
char *ItemList__FormatRowText(ItemList *self, char *dest, s32 row, s32 top, s32 column);

/**
 * @brief Records the window, scroll and cursor, and when `highlight` is set
 * colours the cursor's row.
 * @param self      The list.
 * @param top       The item shown in row 0.
 * @param column    The horizontal scroll offset.
 * @param cursor    The highlighted item.
 * @param highlight Non-zero to colour the cursor row.
 */
void ItemList__SetView(ItemList *self, s32 top, s32 column, s32 cursor, s32 highlight);

/**
 * @brief stepCursorInView (slot +0x098): moves the highlight one row down
 * (`dir` non-zero) or up inside the window, and clicks when `notify` is set.
 * @param self   The list.
 * @param dir    Non-zero for down, zero for up.
 * @param notify Non-zero to play the click.
 */
void ItemList__StepCursorInView(ItemList *self, s32 dir, s32 notify);

/**
 * @brief getCursorIndex (slot +0x09C).
 * @param self The list.
 * @return The item under the cursor.
 */
s32 ItemList__GetCursorIndex(ItemList *self);

#endif
