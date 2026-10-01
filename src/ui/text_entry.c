/*
 * text_entry.c -- TextEntry (include/text_entry.h), the pad-driven
 * editor for the memory card's save title, in ROM order from New_TextEntry
 * to its getter GetTextEntryMethods; its method table and panel cell end
 * the file. ItemList, the list the player picks a save file from, follows
 * in item_list.c.
 *
 * Like ItemList, it keeps a Pad and a FrameClock child by class, draws
 * through a ScreenSprite panel and TextRows built from CARD\ TIMs, and
 * does nothing until its resources are loaded. Its onNotify sends the
 * Pad's button events to handleCommand and the FrameClock's ticks to
 * tickState, which reports the result to the parents on the second tick
 * after the close.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <strings.h>
#include "text_entry.h"
#include "text_row.h"
#include "tim_image.h"
#include "vab_stream_obj.h"
#include "pad.h"
#include "frame_clock.h"
#include "bmem_pmgr.h"
#include "full_width_sjis.h"
#include "data_source.h"

/* The characters nextChar/prevChar step through, NUL-terminated: a space,
 * A to Z, a to z, 0 to 9. */
extern u8 sStrNameChars[];

/* TextEntry's small data, in address order. Positions are percent of half
 * the screen from the centre (include/screen_sprite.h). */

/* LoadCardResources' text row colour, a yellow. */
static ColorRgb sTextEntryTextColor SDATA = {128, 128, 0};
/* Where LoadCardResources attaches the panel and the text row. */
static ScreenSpritePos sTextEntryPanelPos SDATA = {-70, -60};
static ScreenSpritePos sTextEntryTextPos SDATA = {-62, -15};
/* The cursor sprite's position at index 0: loadCardResources attaches the
 * cursor there, setCursorPos moves it to x + index * 7, y. */
static ScreenSpritePos sTextEntryCursorPos SDATA = {-62, -12};
static u8 *sNameCharTable SDATA = sStrNameChars;
/* LoadCardResources' path parts: CARD\<name>.TIM. */
static char sCardPathPrefix[] SDATA = "CARD\\";
static char sTimExt[] SDATA = ".TIM";

TextEntry *New_TextEntry(char *text, s32 mode) {
    TextEntry *self;

    self = BMemPMgrAlloc(sizeof(TextEntry));
    if (self != NULL) {
        GetTextEntryMethods()->ctor(self, text, mode);
        return self;
    }
    return NULL;
}

void TextEntry__TextEntry(TextEntry *self, char *text, s32 mode) {
    u8 *p;
    s32 count;

    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetTextEntryMethods();
    self->textLen = strlen(text);
    self->editBuf = BMemPMgrAlloc(self->textLen + 4);

    /* MATCHING: counted inline; strlen(sNameCharTable) would be a call. */
    p = sNameCharTable;
    count = 0;
    while (*p != 0) {
        p++;
        count++;
    }
    self->charCount = count;

    TextEntry__ClearChildRefs(self);
    self->methods->setText(self, text, mode);
}

void TextEntry__ClearChildRefs(TextEntry *self) {
    self->inputSource = NULL;
    self->tickSource = NULL;
    self->panelSprite = NULL;
}

void TextEntry__Finalize(TextEntry *self) {
    BMemPMgrFree(self->editBuf);
    GetBasicClassMethods()->finalize((BasicClass *)self);
}

void TextEntry__AddChild(TextEntry *self, void *child) {
    s32 kind;

    if (child != NULL) {
        GetBasicClassMethods()->addChild((BasicClass *)self, (BasicClass *)child);
        kind = ((BasicClass *)child)->methods->header & CLASS_ID_ROOT_MASK;
        if (kind == PAD_CLASS_ID) {
            self->inputSource = child;
        } else if (kind == FRAMECLOCK_CLASS_ID) {
            self->tickSource = child;
        }
    }
}

void TextEntry__RemoveChild(TextEntry *self, void *child) {
    s32 kind;

    if (child != NULL) {
        kind = ((BasicClass *)child)->methods->header & CLASS_ID_ROOT_MASK;
        if (kind == PAD_CLASS_ID) {
            self->inputSource = NULL;
        } else if (kind == FRAMECLOCK_CLASS_ID) {
            self->tickSource = NULL;
        }
        GetBasicClassMethods()->removeChild((BasicClass *)self, (BasicClass *)child);
    }
}

void TextEntry__RemoveAllChildren(TextEntry *self) {
    self->inputSource = NULL;
    self->tickSource = NULL;
    self->panelSprite = NULL;
    GetBasicClassMethods()->removeAllChildren((BasicClass *)self);
}

void TextEntry__OnNotify(TextEntry *self, void *sender, s32 event) {
    s32 kind;

    GetBasicClassMethods()->onNotify((BasicClass *)self, sender, event);

    kind = ((BasicClass *)sender)->methods->header & CLASS_ID_ROOT_MASK;
    if (kind == PAD_CLASS_ID) {
        self->methods->handleCommand(self, sender, event);
    } else if (kind == FRAMECLOCK_CLASS_ID) {
        /* MATCHING: passes (sender, event) as handleCommand's call does, so the
         * two calls share code. */
        self->methods->tickState(self, sender, event);
    }
}

void TextEntry__SetText(TextEntry *self, char *text, s32 mode) {
    self->mode = mode;
    self->textBuf = text;
    self->cursorIndex = 0;
    self->charIndex = 0;
    if (mode == TEXTENTRY_MODE_FULLWIDTH) {
        DecodeFullWidthSjis(self->editBuf, text);
        self->textLen /= 2;
    } else {
        strcpy(self->editBuf, text);
    }
}

/* LoadCardResources' data. */
extern char sStrComInput[];            /* "COMINPUT" */
extern char sStrFontIcon[];            /* "FONTICON" */
extern SpriteRect sTextEntryPanelRect; /* COMINPUT's cell: 224 x 120 from (0, 0) */

void TextEntry__LoadCardResources(TextEntry *self, void *parent) {
    char path[CARD_TIM_PATH_SIZE];
    char *dir;
    char *ext;
    TimImage *panelTim;
    TimImage *fontTim;

    if (parent == NULL) {
        return;
    }
    if (self->panelSprite != NULL) {
        return;
    }

    dir = sCardPathPrefix;
    ext = sTimExt;

    panelTim = New_TimImage(BuildFileName(path, sStrComInput, dir, ext));
    ((TimImageUploadFn)panelTim->methods->processBuffer)(panelTim);
    self->panelSprite = New_ScreenSprite(panelTim, &sTextEntryPanelRect, 0);
    panelTim->methods->release(panelTim);
    self->panelSprite->methods->attachToParent(self->panelSprite, (SceneNode *)parent,
                                               (LongVec3 *)&sTextEntryPanelPos);

    fontTim = New_TimImage(BuildFileName(path, sStrFontIcon, dir, ext));
    ((TimImageUploadFn)fontTim->methods->processBuffer)(fontTim);
    self->textRow = New_TextRow(fontTim, self->textLen, self->editBuf);
    self->cursorSprite = New_CharSprite(fontTim, '_');
    fontTim->methods->release(fontTim);
    self->textRow->methods->attachToParent(self->textRow, (SceneNode *)parent,
                                           (LongVec3 *)&sTextEntryTextPos);
    self->textRow->methods->setColor(self->textRow, &sTextEntryTextColor);
    self->cursorSprite->methods->attachToParent(self->cursorSprite, (SceneNode *)parent,
                                                (LongVec3 *)&sTextEntryCursorPos);
}

void TextEntry__ReleaseCardResources(TextEntry *self) {
    if (self->panelSprite != NULL) {
        self->panelSprite = self->panelSprite->methods->release(self->panelSprite);
        self->textRow->methods->release(self->textRow);
        self->cursorSprite->methods->release(self->cursorSprite);
    }
}

void TextEntry__AttachTarget(TextEntry *self, void *inputSource, void *tickSource, VabStreamObj *target) {
    self->methods->addChild(self, inputSource);
    self->methods->addChild(self, tickSource);
    self->target = target;
    self->closeState = 0;
    self->actOnHeld = 0;
}

void TextEntry__DetachTarget(TextEntry *self) {
    self->methods->removeChild(self, self->inputSource);
    self->methods->removeChild(self, self->tickSource);
    self->target = NULL;
}

void TextEntry__SetState(TextEntry *self, s32 state) {
    self->closeTickCount = 0;
    if (state < TEXTENTRY_RESULT_ACCEPTED) {
        return;
    }
    switch (state) {
        case TEXTENTRY_RESULT_ACCEPTED:
        case TEXTENTRY_RESULT_CANCELLED:
            self->methods->removeChild(self, self->inputSource);
            self->methods->releaseCardResources(self);
            self->closeState = state;
            break;
        case TEXTENTRY_STATE_REPORT:
            self->methods->notifyParents(self, self->closeState);
            break;
    }
}

void TextEntry__TickState(TextEntry *self) {
    s32 state;
    s32 old;

    state = self->closeState;
    if (state >= TEXTENTRY_STATE_REPORT) {
        return;
    }
    if (state < TEXTENTRY_RESULT_ACCEPTED) {
        return;
    }
    old = self->closeTickCount;
    self->closeTickCount = old + 1;
    if (old != 0) {
        self->methods->setState(self, TEXTENTRY_STATE_REPORT);
    }
}

/* With actOnHeld clear the arrows act on presses, with it set on held
 * buttons. */
/* MATCHING: arms in this order, default first; retail's jump table and tests follow it. */
void TextEntry__HandleCommand(TextEntry *self, void *sender, s32 command) {
    switch (command) {
        default:
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_RRIGHT:
            if (self->mode == TEXTENTRY_MODE_FULLWIDTH) {
                EncodeFullWidthSjis(self->textBuf, self->editBuf);
            } else {
                strcpy(self->textBuf, self->editBuf);
            }
            self->methods->playSound(self, TEXTENTRY_TONE_BUTTON);
            self->methods->setState(self, TEXTENTRY_RESULT_ACCEPTED);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_RDOWN:
            self->methods->playSound(self, TEXTENTRY_TONE_BUTTON);
            self->methods->setState(self, TEXTENTRY_RESULT_CANCELLED);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_L2:
            self->methods->resetAllChars(self);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_L1:
            self->methods->resetChar(self);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_SELECT:
            self->methods->toggleActOnHeld(self);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_LRIGHT:
            if (self->actOnHeld != 0) {
                return;
            }
            self->methods->moveCursorRight(self);
            return;
        case PAD_EVENT_HELD + PAD_BUTTON_LRIGHT:
            if (self->actOnHeld == 0) {
                return;
            }
            self->methods->moveCursorRight(self);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_LLEFT:
            if (self->actOnHeld != 0) {
                return;
            }
            self->methods->moveCursorLeft(self);
            return;
        case PAD_EVENT_HELD + PAD_BUTTON_LLEFT:
            if (self->actOnHeld == 0) {
                return;
            }
            self->methods->moveCursorLeft(self);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_LUP:
            if (self->actOnHeld != 0) {
                return;
            }
            self->methods->nextChar(self);
            return;
        case PAD_EVENT_HELD + PAD_BUTTON_LUP:
            if (self->actOnHeld == 0) {
                return;
            }
            self->methods->nextChar(self);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_LDOWN:
            if (self->actOnHeld != 0) {
                return;
            }
            self->methods->prevChar(self);
            return;
        case PAD_EVENT_HELD + PAD_BUTTON_LDOWN:
            if (self->actOnHeld == 0) {
                return;
            }
            self->methods->prevChar(self);
            return;
    }
}

void TextEntry__PlaySound(TextEntry *self, s32 tone) {
    VabStreamObj *target;

    target = self->target;
    if (target != NULL) {
        target->methods->playTone(target, tone, TEXTENTRY_TONE_VOLUME, TEXTENTRY_TONE_VOLUME);
    }
}

void TextEntry__MoveCursorRight(TextEntry *self) {
    s32 old;
    s32 next;

    if (self->panelSprite != NULL) {
        old = self->cursorIndex;
        next = old + 1;
        self->cursorIndex = next;
        if (next < self->textLen) {
            self->methods->setCursorPos(self, next, 1);
        } else {
            self->cursorIndex = old;
        }
    }
}

void TextEntry__MoveCursorLeft(TextEntry *self) {
    s32 old;
    s32 next;

    if (self->panelSprite != NULL) {
        old = self->cursorIndex;
        next = old - 1;
        self->cursorIndex = next;
        if (next >= 0) {
            self->methods->setCursorPos(self, next, 1);
        } else {
            self->cursorIndex = old;
        }
    }
}

void TextEntry__NextChar(TextEntry *self) {
    s32 next;

    if (self->panelSprite != NULL) {
        next = self->charIndex + 1;
        self->charIndex = next;
        if (next < self->charCount) {
            self->methods->setCharAt(self, self->cursorIndex, next, 1);
        } else {
            self->charIndex = 0;
        }
    }
}

void TextEntry__PrevChar(TextEntry *self) {
    s32 count;

    if (self->panelSprite) {
        count = self->charIndex - 1;
        self->charIndex = count;
        if (count > 0) {
            self->methods->setCharAt(self, self->cursorIndex, count, 1);
        } else {
            self->charIndex = self->charCount;
        }
    }
}

void TextEntry__ToggleActOnHeld(TextEntry *self) {
    if (self->panelSprite) {
        self->actOnHeld ^= 1;
    }
}

void TextEntry__ResetChar(TextEntry *self) {
    if (self->panelSprite) {
        self->charIndex = 0;
        self->methods->setCharAt(self, self->cursorIndex, 0, 1);
    }
}

void TextEntry__ResetAllChars(TextEntry *self) {
    s32 i;

    if (self->panelSprite) {
        self->charIndex = 0;
        for (i = self->textLen - 1; i >= 0; i--) {
            self->cursorIndex = i;
            self->methods->setCharAt(self, i, self->charIndex, 0);
        }
        self->methods->setCursorPos(self, self->cursorIndex, 1);
    }
}

void TextEntry__SetCursorPos(TextEntry *self, s32 pos, s32 notify) {
    ScreenSpritePos screenPos;
    CharSprite *cursor;

    if (self->panelSprite) {
        screenPos.y = sTextEntryCursorPos.y;
        screenPos.x = pos * TEXTROW_DEFAULT_PITCH + sTextEntryCursorPos.x;
        cursor = self->cursorSprite;
        cursor->methods->setPosition(cursor, &screenPos);
        self->cursorIndex = pos;
        if (notify) {
            self->methods->playSound(self, TEXTENTRY_TONE_CURSOR);
        }
    }
}

void TextEntry__SetCharAt(TextEntry *self, s32 pos, s32 charIndex, s32 notify) {
    TextRow *row;

    if (self->panelSprite) {
        self->editBuf[pos] = sNameCharTable[charIndex];
        row = self->textRow;
        ((TextRowSetCellAtFn)row->methods->setCell)(row, sNameCharTable[charIndex], pos);
        self->cursorIndex = pos;
        self->charIndex = charIndex;
        if (notify) {
            self->methods->playSound(self, TEXTENTRY_TONE_CURSOR);
        }
    }
}

TextEntryMethods *GetTextEntryMethods(void) {
    return &gTextEntryMethods;
}

/* TextEntry's method table (include/text_entry.h): BasicClass's slots with
 * the ctor, finalize and onNotify, then the editor's setText, card
 * resources, attach, state, command and sound slots, and its cursor and
 * character slots; nothing calls the nine pad64 slots. A (void *) entry is
 * a method declared on another type than its slot's. */
TextEntryMethods gTextEntryMethods = {
    /* +0x000 header */ TEXTENTRY_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ TextEntry__TextEntry,
    /* +0x00C finalize */ TextEntry__Finalize,
    /* +0x010 addChild */ (void *)TextEntry__AddChild,
    /* +0x014 removeChild */ (void *)TextEntry__RemoveChild,
    /* +0x018 removeAllChildren */ TextEntry__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ TextEntry__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 setText */ TextEntry__SetText,
    /* +0x044 loadCardResources */ TextEntry__LoadCardResources,
    /* +0x048 releaseCardResources */ TextEntry__ReleaseCardResources,
    /* +0x04C attachTarget */ TextEntry__AttachTarget,
    /* +0x050 detachTarget */ TextEntry__DetachTarget,
    /* +0x054 setState */ TextEntry__SetState,
    /* +0x058 tickState */ (void *)TextEntry__TickState,
    /* +0x05C handleCommand */ TextEntry__HandleCommand,
    /* +0x060 playSound */ TextEntry__PlaySound,
    /* +0x064 pad64 */ {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    /* +0x088 moveCursorRight */ TextEntry__MoveCursorRight,
    /* +0x08C moveCursorLeft */ TextEntry__MoveCursorLeft,
    /* +0x090 nextChar */ TextEntry__NextChar,
    /* +0x094 prevChar */ TextEntry__PrevChar,
    /* +0x098 toggleActOnHeld */ TextEntry__ToggleActOnHeld,
    /* +0x09C resetChar */ TextEntry__ResetChar,
    /* +0x0A0 resetAllChars */ TextEntry__ResetAllChars,
    /* +0x0A4 setCursorPos */ TextEntry__SetCursorPos,
    /* +0x0A8 setCharAt */ TextEntry__SetCharAt,
};

/* COMINPUT's cell, the text-entry panel: 224 x 120 from (0, 0). */
SpriteRect sTextEntryPanelRect = {0, 0, 224, 120};
