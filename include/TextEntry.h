#ifndef TEXTENTRY_H
#define TEXTENTRY_H

#include "BasicClass.h"
#include "CharSprite.h"

/*
 * TextEntry -- an editor for a caller-owned string (class id 0x10, method
 * table gTextEntryMethods): BasicClass's direct subclass, no class below it.
 * Methods in src/ui/TextEntryItemList.c, New_TextEntry to GetTextEntryMethods.
 * The name is for
 * what its own methods do, and the evidence is this:
 *  - setText (the ctor forwards to it) keeps the caller's buffer in `textBuf`
 *    and copies it into its own `editBuf` (mode 1 decodes full-width SJIS
 *    and halves `textLen`, DecodeFullWidthSjis); a circle press writes `editBuf`
 *    back into `textBuf` (EncodeFullWidthSjis in mode 1) and closes with
 *    state 2, a cross press closes with state 3 without writing.
 *  - moveCursorRight/Left step `cursorIndex` inside [0, textLen) and move the
 *    cursor sprite (setCursorPos: x = index * 7 from sTextEntryCursorPos).
 *  - nextChar/prevChar step `charIndex` through sNameCharTable, whose length
 *    the ctor counts into `charCount`, and setCharAt writes that byte into
 *    `editBuf` at the cursor and into the text row. resetChar/resetAllChars
 *    write byte 0 of the table at the cursor / at every position.
 *  - Every editing method does nothing until loadCardResources has made
 *    `panelSprite` (CARD\COMINPUT.TIM), the text row and the '_' cursor
 *    (CARD\FONTICON.TIM).
 * Its one maker is TaskObjF__AttachTextEntry (TitleMenuTaskObjF, mode 1), which
 * also drives loadCardResources/attachTarget/detachTarget/release.
 * What the string is in the game is not established.
 *
 * +0x058 tickState: the slot passes (sender, event) because onNotify's bytes
 * set $a1/$a2 for it (the tag-5 call cross-jumps with handleCommand's); its
 * occupant TextEntry__TickState reads neither.
 *
 * The object is 0x4C bytes (New_TextEntry).
 */

/* The objects it holds, by tag. */
struct VabStreamObj;
struct TextRow;

/* TextEntry's class id (gTextEntryMethods word +0x000). Two nibbles, so
 * `(u8)header == TEXTENTRY_CLASS_ID` is its is-kind-of test (TaskObjF's
 * addChild/removeChild/onNotify). */
#define TEXTENTRY_CLASS_ID 0x10

typedef struct TextEntry TextEntry;
typedef struct TextEntryMethods TextEntryMethods;

/* setText's `mode`: how the caller's string is stored. In FULLWIDTH it is
 * full-width SJIS, two bytes a character, and `editBuf` holds it decoded to
 * one byte a character (DecodeFullWidthSjis; EncodeFullWidthSjis writes it
 * back). PLAIN copies it with strcpy both ways. */
enum TextEntryMode { TEXTENTRY_MODE_PLAIN = 0, TEXTENTRY_MODE_FULLWIDTH = 1 };

/* The result a closing TextEntry passes to notifyParents: a circle press
 * writes `editBuf` back into `textBuf` first, a cross press does not.
 * setState(result) closes; `closeState` keeps the result. */
enum TextEntryResult { TEXTENTRY_RESULT_ACCEPTED = 2, TEXTENTRY_RESULT_CANCELLED = 3 };

/* setState's last state: notify the parents with `closeState`. tickState
 * enters it on the second tick after a close. */
#define TEXTENTRY_STATE_REPORT 4

/* BasicClass's slots (overrides: +0x008 TextEntry__TextEntry, +0x00C
 * Finalize, +0x010 AddChild, +0x014 RemoveChild, +0x018 RemoveAllChildren,
 * +0x038 OnNotify; `tools/classtable.py gTextEntryMethods --vs gBasicClassMethods`),
 * then this class's own. +0x064..+0x084 are NULL in the table. */
struct TextEntryMethods {
    BASICCLASS_SLOTS(TextEntry, (TextEntry * self, char *text, s32 mode));
    /* +0x040 */ void (*setText)(TextEntry *self, char *text, s32 mode); /* TextEntry__SetText */
    /* +0x044 */ void (*loadCardResources)(TextEntry *self, void *parent); /* TextEntry__LoadCardResources */
    /* +0x048 */ void (*releaseCardResources)(TextEntry *self); /* TextEntry__ReleaseCardResources */
    /* +0x04C */ void (*attachTarget)(TextEntry *self, void *child1, void *child2,
                                      struct VabStreamObj *target); /* TextEntry__AttachTarget */
    /* +0x050 */ void (*detachTarget)(TextEntry *self);             /* TextEntry__DetachTarget */
    /* +0x054 */ void (*setState)(TextEntry *self, s32 state); /* TextEntry__SetState: 2/3 close, 4 notifies parents */
    /* +0x058 */ void (*tickState)(TextEntry *self, void *sender,
                                   s32 event); /* TextEntry__TickState (reads only self; see the banner) */
    /* +0x05C */ void (*handleCommand)(TextEntry *self, void *sender, s32 command); /* TextEntry__HandleCommand */
    /* +0x060 */ void (*playSound)(TextEntry *self, s32 tone); /* TextEntry__PlaySound */
    /* +0x064 */ void *slot64[9];                              /* NULL */
    /* +0x088 */ void (*moveCursorRight)(TextEntry *self);     /* TextEntry__MoveCursorRight */
    /* +0x08C */ void (*moveCursorLeft)(TextEntry *self);      /* TextEntry__MoveCursorLeft */
    /* +0x090 */ void (*nextChar)(TextEntry *self);            /* TextEntry__NextChar */
    /* +0x094 */ void (*prevChar)(TextEntry *self);            /* TextEntry__PrevChar */
    /* +0x098 */ void (*toggleActOnHeld)(TextEntry *self);     /* TextEntry__ToggleActOnHeld */
    /* +0x09C */ void (*resetChar)(TextEntry *self);           /* TextEntry__ResetChar */
    /* +0x0A0 */ void (*resetAllChars)(TextEntry *self);       /* TextEntry__ResetAllChars */
    /* +0x0A4 */ void (*setCursorPos)(TextEntry *self, s32 pos, s32 notify); /* TextEntry__SetCursorPos */
    /* +0x0A8 */ void (*setCharAt)(TextEntry *self, s32 pos, s32 charIndex, s32 notify); /* TextEntry__SetCharAt */
};

struct TextEntry {
    BASICCLASS_FIELDS(TextEntryMethods);
    /* +0x00C */ s32 mode;    /* setText: 1 = the caller's buffer is full-width SJIS */
    /* +0x010 */ s32 textLen; /* ctor: strlen(text), halved by setText in mode 1; the cursor's bound */
    /* +0x014 */ s32 charCount; /* ctor: length of sNameCharTable; nextChar's bound, prevChar's wrap value */
    /* +0x018 */ s32 cursorIndex; /* setText zeroes; moveCursorRight/Left, setCursorPos, setCharAt */
    /* +0x01C */ s32 charIndex; /* setText zeroes; index into sNameCharTable: nextChar/prevChar/resetChar, setCharAt */
    /* +0x020 */ s32 actOnHeld; /* toggleActOnHeld flips it; handleCommand's arrow cases act on Pad presses when 0, on held buttons when set */
    /* +0x024 */ char *textBuf; /* setText: the caller's buffer; a circle press (handleCommand) writes editBuf back into it */
    /* +0x028 */ char *editBuf; /* ctor allocates textLen + 4, finalize frees; setText copies into it, setCharAt writes it */
    /* +0x02C */ s32 closeState; /* setState: 2 or 3; setState(4) notifies parents with it */
    /* +0x030 */ s32 closeTickCount; /* setState zeroes; tickState counts, and calls setState(4) on the second tick */
    /* +0x034 */ void *inputSource; /* addChild/removeChild: the child of class PAD_CLASS_ID (a Pad); onNotify sends its events to handleCommand */
    /* +0x038 */ void *tickSource; /* ... of class FRAMECLOCK_CLASS_ID (a FrameClock); onNotify sends its events to tickState */
    /* +0x03C */ struct VabStreamObj *target; /* attachTarget; TextEntry__PlaySound plays a tone on it (playTone, volume 96, 96) */
    /* +0x040 */ CharSprite *cursorSprite; /* loadCardResources: New_CharSprite(FONTICON, '_'); setCursorPos moves it to x = pos * 7 */
    /* +0x044 */ struct TextRow *textRow; /* a TextRow (include/TextRow.h): loadCardResources: New_TextRow(FONTICON, textLen, editBuf); setCharAt sets a cell (+0x0C4) */
    /* +0x048 */ ScreenSprite *panelSprite; /* loadCardResources: New_ScreenSprite(COMINPUT, 224x120); non-NULL gates every editing method */
};

extern TextEntryMethods gTextEntryMethods;
extern TextEntryMethods *GetTextEntryMethods(void); /* returns &gTextEntryMethods */

/* The class's own methods, in address order. */
TextEntry *New_TextEntry(char *text, s32 mode);
void TextEntry__TextEntry(TextEntry *self, char *text, s32 mode);
void TextEntry__ClearChildRefs(TextEntry *self);
void TextEntry__Finalize(TextEntry *self);
void TextEntry__AddChild(TextEntry *self, void *child);
void TextEntry__RemoveChild(TextEntry *self, void *child);
void TextEntry__RemoveAllChildren(TextEntry *self);
void TextEntry__OnNotify(TextEntry *self, void *sender, s32 event);
void TextEntry__SetText(TextEntry *self, char *text, s32 mode);
void TextEntry__LoadCardResources(TextEntry *self, void *parent);
void TextEntry__ReleaseCardResources(TextEntry *self);
void TextEntry__AttachTarget(TextEntry *self, void *child1, void *child2, struct VabStreamObj *target);
void TextEntry__DetachTarget(TextEntry *self);
void TextEntry__SetState(TextEntry *self, s32 state);
void TextEntry__TickState(TextEntry *self);
void TextEntry__HandleCommand(TextEntry *self, void *sender, s32 command);
void TextEntry__PlaySound(TextEntry *self, s32 tone);
void TextEntry__MoveCursorRight(TextEntry *self);
void TextEntry__MoveCursorLeft(TextEntry *self);
void TextEntry__NextChar(TextEntry *self);
void TextEntry__PrevChar(TextEntry *self);
void TextEntry__ToggleActOnHeld(TextEntry *self);
void TextEntry__ResetChar(TextEntry *self);
void TextEntry__ResetAllChars(TextEntry *self);
void TextEntry__SetCursorPos(TextEntry *self, s32 pos, s32 notify);
void TextEntry__SetCharAt(TextEntry *self, s32 pos, s32 charIndex, s32 notify);

#endif
