/**
 * @file text_entry.h
 * @brief TextEntry, an on-screen editor for a caller-owned string.
 *
 * Declares the TextEntry class (object and method table), its mode, result
 * and state constants, and its methods, which are defined in
 * src/ui/input_dialogs.c.
 */
#ifndef TEXT_ENTRY_H
#define TEXT_ENTRY_H

#include "basic_class.h"
#include "char_sprite.h"

/* The objects it holds, by tag. */
struct VabStreamObj;
struct TextRow;

/** TextEntry's class id (gTextEntryMethods word +0x000). Two nibbles, so
 * `(u8)header == TEXTENTRY_CLASS_ID` is its is-kind-of test (TaskObjF's
 * addChild/removeChild/onNotify). */
#define TEXTENTRY_CLASS_ID 0x10

typedef struct TextEntry TextEntry;
typedef struct TextEntryMethods TextEntryMethods;

/** setText's `mode`: how the caller's string is stored. */
enum TextEntryMode {
    TEXTENTRY_MODE_PLAIN = 0, /**< one byte a character, copied with strcpy both ways */
    TEXTENTRY_MODE_FULLWIDTH = 1 /**< full-width SJIS, two bytes a character; `editBuf` holds it decoded */
};

/** The result a closing TextEntry passes to notifyParents. setState(result)
 * closes; `closeState` keeps the result. */
enum TextEntryResult {
    TEXTENTRY_RESULT_ACCEPTED = 2, /**< circle: `editBuf` was written back into `textBuf` */
    TEXTENTRY_RESULT_CANCELLED = 3 /**< cross: the caller's string is unchanged */
};

/** setState's last state: notify the parents with `closeState`. tickState
 * enters it on the second tick after a close. */
#define TEXTENTRY_STATE_REPORT 4

/** @name Tones
 * playSound's tones, VabStreamObj__PlayTone indices (program << 4 | tone):
 * the same two TaskCore's menus play (task_core.h). @{ */
#define TEXTENTRY_TONE_CURSOR 0x00 /**< setCursorPos, setCharAt: the cursor or a character moved */
#define TEXTENTRY_TONE_BUTTON 0x10 /**< circle or cross: the entry closes */
#define TEXTENTRY_TONE_VOLUME 96   /**< playSound's PlayTone vol and endVol */

/** @} */

/**
 * @brief TextEntry's method table: BasicClass's slots, then TextEntry's own.
 *
 * TextEntry overrides the inherited +0x008 (TextEntry__TextEntry), +0x00C
 * (Finalize), +0x010 (AddChild), +0x014 (RemoveChild), +0x018
 * (RemoveAllChildren) and +0x038 (OnNotify). +0x064..+0x084 are NULL.
 */
struct TextEntryMethods {
    BASICCLASS_SLOTS(TextEntry, (TextEntry * self, char *text, s32 mode));
    /* +0x040 */ void (*setText)(TextEntry *self, char *text, s32 mode); /**< @see TextEntry__SetText */
    /* +0x044 */ void (*loadCardResources)(TextEntry *self, void *parent); /**< @see TextEntry__LoadCardResources */
    /* +0x048 */ void (*releaseCardResources)(TextEntry *self); /**< @see TextEntry__ReleaseCardResources */
    /* +0x04C */ void (*attachTarget)(TextEntry *self, void *inputSource, void *tickSource,
                                      struct VabStreamObj *target); /**< @see TextEntry__AttachTarget */
    /* +0x050 */ void (*detachTarget)(TextEntry *self);        /**< @see TextEntry__DetachTarget */
    /* +0x054 */ void (*setState)(TextEntry *self, s32 state); /**< @see TextEntry__SetState */
    /* +0x058 */ void (*tickState)(TextEntry *self, void *sender,
                                   s32 event); /**< @see TextEntry__TickState, which reads only self */
    /* +0x05C */ void (*handleCommand)(TextEntry *self, void *sender,
                                       s32 command);           /**< @see TextEntry__HandleCommand */
    /* +0x060 */ void (*playSound)(TextEntry *self, s32 tone); /**< @see TextEntry__PlaySound */
    /* +0x064 */ void *pad64[9];                               /**< NULL; nothing calls them */
    /* +0x088 */ void (*moveCursorRight)(TextEntry *self); /**< @see TextEntry__MoveCursorRight */
    /* +0x08C */ void (*moveCursorLeft)(TextEntry *self);  /**< @see TextEntry__MoveCursorLeft */
    /* +0x090 */ void (*nextChar)(TextEntry *self);        /**< @see TextEntry__NextChar */
    /* +0x094 */ void (*prevChar)(TextEntry *self);        /**< @see TextEntry__PrevChar */
    /* +0x098 */ void (*toggleActOnHeld)(TextEntry *self); /**< @see TextEntry__ToggleActOnHeld */
    /* +0x09C */ void (*resetChar)(TextEntry *self);       /**< @see TextEntry__ResetChar */
    /* +0x0A0 */ void (*resetAllChars)(TextEntry *self);   /**< @see TextEntry__ResetAllChars */
    /* +0x0A4 */ void (*setCursorPos)(TextEntry *self, s32 pos, s32 notify); /**< @see TextEntry__SetCursorPos */
    /* +0x0A8 */ void (*setCharAt)(TextEntry *self, s32 pos, s32 charIndex,
                                   s32 notify); /**< @see TextEntry__SetCharAt */
};

/**
 * @brief An on-screen editor for a caller-owned string, driven by the pad.
 *
 * Class id 0x10 (TEXTENTRY_CLASS_ID), method table gTextEntryMethods,
 * parent BasicClass; no class below it. Methods in src/ui/input_dialogs.c,
 * New_TextEntry to GetTextEntryMethods.
 *
 * - setText (the ctor forwards to it) keeps the caller's buffer in
 *   `textBuf` and copies it into its own `editBuf` (mode 1 decodes
 *   full-width SJIS and halves `textLen`, DecodeFullWidthSjis). A circle
 *   press writes `editBuf` back into `textBuf` (EncodeFullWidthSjis in mode
 *   1) and closes with state 2; a cross press closes with state 3 without
 *   writing.
 * - moveCursorRight/Left step `cursorIndex` inside [0, textLen) and move the
 *   cursor sprite (setCursorPos: x = index * 7 from sTextEntryCursorPos).
 * - nextChar/prevChar step `charIndex` through sNameCharTable, whose length
 *   the ctor counts into `charCount`, and setCharAt writes that byte into
 *   `editBuf` at the cursor and into the text row. resetChar/resetAllChars
 *   write byte 0 of the table at the cursor / at every position.
 * - Every editing method does nothing until loadCardResources has made
 *   `panelSprite` (CARD\COMINPUT.TIM), the text row and the '_' cursor
 *   (CARD\FONTICON.TIM).
 *
 * Its one maker is TaskObjF__AttachTextEntry (title_menu.c, mode 1), which
 * edits the memory-card save title and also drives
 * loadCardResources/attachTarget/detachTarget/release.
 *
 * The +0x058 tickState slot takes (sender, event), which onNotify passes;
 * its occupant TextEntry__TickState reads neither.
 *
 * The object is 0x4C bytes (New_TextEntry).
 */
struct TextEntry {
    BASICCLASS_FIELDS(TextEntryMethods);
    /* +0x00C */ s32 mode; /**< a TextEntryMode (setText): 1 = the caller's buffer is full-width SJIS */
    /* +0x010 */ s32 textLen; /**< ctor: strlen(text), halved by setText in mode 1; the cursor's bound */
    /* +0x014 */ s32 charCount; /**< ctor: length of sNameCharTable; nextChar's bound, prevChar's wrap value */
    /* +0x018 */ s32 cursorIndex; /**< setText zeroes; moveCursorRight/Left, setCursorPos, setCharAt */
    /* +0x01C */ s32 charIndex; /**< setText zeroes; index into sNameCharTable: nextChar/prevChar/resetChar, setCharAt */
    /* +0x020 */ s32 actOnHeld; /**< toggleActOnHeld flips it; handleCommand's arrow cases act on Pad presses when 0, on held buttons when set */
    /* +0x024 */ char *textBuf; /**< setText: the caller's buffer; a circle press (handleCommand) writes editBuf back into it */
    /* +0x028 */ char *editBuf; /**< ctor allocates textLen + 4, finalize frees; setText copies into it, setCharAt writes it */
    /* +0x02C */ s32 closeState; /**< setState: 2 or 3; setState(4) notifies parents with it */
    /* +0x030 */ s32 closeTickCount; /**< setState zeroes; tickState counts, and calls setState(4) on the second tick */
    /* +0x034 */ void *inputSource; /**< addChild/removeChild: the child of class PAD_CLASS_ID (a Pad); onNotify sends its events to handleCommand */
    /* +0x038 */ void *tickSource; /**< ... of class FRAMECLOCK_CLASS_ID (a FrameClock); onNotify sends its events to tickState */
    /* +0x03C */ struct VabStreamObj *target; /**< attachTarget; TextEntry__PlaySound plays a tone on it (playTone, volume 96, 96) */
    /* +0x040 */ CharSprite *cursorSprite; /**< loadCardResources: New_CharSprite(FONTICON, '_'); setCursorPos moves it to x = pos * 7 */
    /* +0x044 */ struct TextRow *textRow; /**< a TextRow (include/text_row.h): loadCardResources: New_TextRow(FONTICON, textLen, editBuf); setCharAt sets a cell (+0x0C4) */
    /* +0x048 */ ScreenSprite *panelSprite; /**< loadCardResources: New_ScreenSprite(COMINPUT, 224x120); non-NULL gates every editing method */
};

/** TextEntry's method table (class id TEXTENTRY_CLASS_ID). */
extern TextEntryMethods gTextEntryMethods;

/**
 * @brief Returns TextEntry's method table.
 * @return &gTextEntryMethods.
 */
extern TextEntryMethods *GetTextEntryMethods(void);

/* The class's own methods, in address order. */

/**
 * @brief Allocates a TextEntry and runs its ctor through the method table.
 * @param text the caller's string to edit, kept and later written back.
 * @param mode a TextEntryMode: how `text` is stored.
 * @return the new editor, or NULL when the allocation fails.
 */
TextEntry *New_TextEntry(char *text, s32 mode);

/**
 * @brief Constructs a TextEntry: BasicClass's ctor, an edit buffer, then setText.
 *
 * Allocates `editBuf` for strlen(text) + 4 bytes, counts sNameCharTable into
 * `charCount` and clears the child references.
 * @param self the object to construct.
 * @param text the caller's string to edit.
 * @param mode a TextEntryMode.
 */
void TextEntry__TextEntry(TextEntry *self, char *text, s32 mode);

/**
 * @brief Clears the input and tick child references and the panel sprite.
 * @param self the editor.
 */
void TextEntry__ClearChildRefs(TextEntry *self);

/**
 * @brief Frees the edit buffer, then runs BasicClass's finalize.
 * @param self the editor.
 */
void TextEntry__Finalize(TextEntry *self);

/**
 * @brief Adds a child, keeping a Pad as `inputSource` and a FrameClock as `tickSource`.
 * @param self  the editor.
 * @param child the child, or NULL for none.
 */
void TextEntry__AddChild(TextEntry *self, void *child);

/**
 * @brief Removes a child, clearing `inputSource` or `tickSource` when it is that one's class.
 * @param self  the editor.
 * @param child the child, or NULL for none.
 */
void TextEntry__RemoveChild(TextEntry *self, void *child);

/**
 * @brief Clears the child references and the panel sprite, then removes every child.
 * @param self the editor.
 */
void TextEntry__RemoveAllChildren(TextEntry *self);

/**
 * @brief Routes a child's notification: a Pad's to handleCommand, a FrameClock's to tickState.
 *
 * Runs BasicClass's onNotify first.
 * @param self   the editor.
 * @param sender the child that notified.
 * @param event  the notification (a pad event for a Pad).
 */
void TextEntry__OnNotify(TextEntry *self, void *sender, s32 event);

/**
 * @brief Starts editing `text`: keeps the buffer and copies it into `editBuf`.
 *
 * Zeroes the cursor and character indices. In TEXTENTRY_MODE_FULLWIDTH it
 * decodes the string to one byte a character and halves `textLen`.
 * @param self the editor.
 * @param text the caller's string, written back on a circle press.
 * @param mode a TextEntryMode.
 */
void TextEntry__SetText(TextEntry *self, char *text, s32 mode);

/**
 * @brief Loads the panel, text row and cursor and attaches them under `parent`.
 *
 * The panel is CARD\COMINPUT.TIM (224 x 120) at (-70, -60); the text row
 * shows `editBuf` in CARD\FONTICON.TIM at (-62, -15) in colour
 * (128, 128, 0); the cursor is the font's '_' at sTextEntryCursorPos. Does
 * nothing without a parent or when already loaded.
 * @param self   the editor.
 * @param parent the SceneNode to attach the sprites under.
 */
void TextEntry__LoadCardResources(TextEntry *self, void *parent);

/**
 * @brief Releases the panel, text row and cursor, when they are loaded.
 * @param self the editor.
 */
void TextEntry__ReleaseCardResources(TextEntry *self);

/**
 * @brief Adds the input and tick sources as children and keeps the sound object.
 *
 * Also clears `closeState` and `actOnHeld`.
 * @param self        the editor.
 * @param inputSource the Pad whose button events edit the text.
 * @param tickSource  the FrameClock whose ticks time the close.
 * @param target      the VabStreamObj sounds are played on.
 */
void TextEntry__AttachTarget(TextEntry *self, void *inputSource, void *tickSource,
                             struct VabStreamObj *target);

/**
 * @brief Removes the input and tick children and forgets the sound object.
 * @param self the editor.
 */
void TextEntry__DetachTarget(TextEntry *self);

/**
 * @brief Closes the editor with a result, or reports the result to the parents.
 *
 * Zeroes `closeTickCount`. A TextEntryResult removes the input child,
 * releases the sprites and keeps the result in `closeState`;
 * TEXTENTRY_STATE_REPORT notifies the parents with it. Values below 2 do
 * nothing else.
 * @param self  the editor.
 * @param state TEXTENTRY_RESULT_ACCEPTED, TEXTENTRY_RESULT_CANCELLED or
 *              TEXTENTRY_STATE_REPORT.
 */
void TextEntry__SetState(TextEntry *self, s32 state);

/**
 * @brief Counts frame ticks after a close and reports on the second one.
 *
 * Only while `closeState` holds a TextEntryResult: the second tick calls
 * setState(TEXTENTRY_STATE_REPORT).
 * @param self the editor.
 */
void TextEntry__TickState(TextEntry *self);

/**
 * @brief Acts on a pad event.
 *
 * Circle writes the edit back and closes accepted, cross closes cancelled
 * (both with a sound), L2 resets every character, L1 the one under the
 * cursor, Select toggles `actOnHeld`. Left/right move the cursor and
 * up/down step the character under it: on a press while `actOnHeld` is
 * clear, on a held button while it is set. Other events are ignored.
 * @param self    the editor.
 * @param sender  the Pad (unused).
 * @param command the pad event, PAD_EVENT_PRESSED or PAD_EVENT_HELD plus a button.
 */
void TextEntry__HandleCommand(TextEntry *self, void *sender, s32 command);

/**
 * @brief Plays a tone on the sound object at volume 96, 96, when there is one.
 * @param self the editor.
 * @param tone the tone: VAB program * 16 + tone number.
 */
void TextEntry__PlaySound(TextEntry *self, s32 tone);

/**
 * @brief Moves the cursor one character right, stopping at the last one.
 * @param self the editor.
 */
void TextEntry__MoveCursorRight(TextEntry *self);

/**
 * @brief Moves the cursor one character left, stopping at the first one.
 * @param self the editor.
 */
void TextEntry__MoveCursorLeft(TextEntry *self);

/**
 * @brief Writes the next character of sNameCharTable under the cursor.
 *
 * Past the table's end it wraps `charIndex` to 0 without writing.
 * @param self the editor.
 */
void TextEntry__NextChar(TextEntry *self);

/**
 * @brief Writes the previous character of sNameCharTable under the cursor.
 *
 * On reaching index 0 it wraps `charIndex` to `charCount` without writing.
 * @param self the editor.
 */
void TextEntry__PrevChar(TextEntry *self);

/**
 * @brief Switches the arrows between acting on presses and on held buttons.
 * @param self the editor.
 */
void TextEntry__ToggleActOnHeld(TextEntry *self);

/**
 * @brief Writes the table's first character under the cursor.
 * @param self the editor.
 */
void TextEntry__ResetChar(TextEntry *self);

/**
 * @brief Writes the table's first character at every position and moves the cursor to the start.
 * @param self the editor.
 */
void TextEntry__ResetAllChars(TextEntry *self);

/**
 * @brief Moves the cursor sprite to character `pos` and makes it the cursor position.
 * @param self   the editor.
 * @param pos    the character index; the sprite goes to x + pos * 7.
 * @param notify nonzero to play tone 0.
 */
void TextEntry__SetCursorPos(TextEntry *self, s32 pos, s32 notify);

/**
 * @brief Writes character `charIndex` of sNameCharTable at `pos`, in `editBuf` and the text row.
 *
 * Also moves the cursor and character indices there.
 * @param self      the editor.
 * @param pos       the character position.
 * @param charIndex the index into sNameCharTable.
 * @param notify    nonzero to play tone 0.
 */
void TextEntry__SetCharAt(TextEntry *self, s32 pos, s32 charIndex, s32 notify);

#endif
