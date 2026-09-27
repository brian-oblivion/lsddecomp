/*
 * class_3bb8c_i -- third carved slice of the class_3bb8c block, 20 functions,
 * carved round 14. All 20 are TextEntry methods (gTextEntryMethods,
 * `D_80086ED0`, 42 slots; `tools/classtable.py gTextEntryMethods`), declared
 * in include/TextEntry.h, whose banner has the evidence for the class name.
 * The other seven own methods (PrevChar .. SetCharAt, GetTextEntryMethods)
 * are class_3bb8c_j's. TextEntry edits a caller-owned string: it loads the
 * `CARD\COMINPUT.TIM`/`CARD\FONTICON.TIM` panel, text row and cursor
 * (LoadCardResources), keeps the caller's buffer (`textBuf`) and its own
 * working copy (`editBuf`, decoded/encoded with DecodeFullWidthSjis/
 * EncodeFullWidthSjis in mode 1), and routes a numeric command switch
 * (HandleCommand) to cursor moves, character stepping, commit (25) and
 * cancel (23).
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <strings.h>
#include "class_3bb8c.h"
#include "TextEntry.h"
#include "CharSprite.h"
#include "TextRow.h"
#include "TimImage.h"
#include "VabStreamObj.h"
#include "Pad.h"
#include "FrameClock.h"

/* Uncarved helper, `code_2cc8c_f`, still INCLUDE_ASM -- TextEntry__SetText's own
 * call. Translates each byte of `src` (a name string) into `dest` (folding a
 * couple of special-case byte ranges) and returns `dest`, same convention as
 * `strcpy`. Typed purely from this call site's own register usage. Declared
 * HERE, not in include/class_3bb8c.h: src/class_3bb8c_j.c types the same
 * (still undefined) function as `void (void *, void *)` from its own call
 * site, and two call-site typings of one function cannot share a header. */
extern char *DecodeFullWidthSjis(char *dest, char *src);

TextEntry *New_TextEntry(char *text, s32 mode) {
    TextEntry *self;

    self = BMemPMgrAlloc(sizeof(TextEntry));
    if (self != NULL) {
        GetTextEntryMethods()->ctor(self, text, mode);
        return self;
    }
    return NULL;
}

/* VALUE-of `%gp_rel`, round 45's own local view -- same global as
 * class_3bb8c_j's `gNameCharTable` (a byte lookup table whose length this
 * function counts by hand rather than via `strlen`, since GCC 2.6.3 with
 * `-fno-builtin` never turns a `strlen` CALL into inline code -- the
 * inline loop below has to be literal source, not a call). */
extern u8 *gNameCharTable;

void TextEntry__TextEntry(TextEntry *self, char *text, s32 mode) {
    u8 *p;
    s32 count;

    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetTextEntryMethods();
    self->textLen = strlen(text);
    self->editBuf = BMemPMgrAlloc(self->textLen + 4);

    p = gNameCharTable;
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
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void TextEntry__AddChild(TextEntry *self, void *child) {
    s32 kind;

    if (child != NULL) {
        Get_vtable_BasicClass()->addChild((BasicClass *)self, (BasicClass *)child);
        kind = ((BasicClass *)child)->methods->header & 0xF;
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
        kind = ((BasicClass *)child)->methods->header & 0xF;
        if (kind == PAD_CLASS_ID) {
            self->inputSource = NULL;
        } else if (kind == FRAMECLOCK_CLASS_ID) {
            self->tickSource = NULL;
        }
        Get_vtable_BasicClass()->removeChild((BasicClass *)self, (BasicClass *)child);
    }
}

void TextEntry__RemoveAllChildren(TextEntry *self) {
    self->inputSource = NULL;
    self->tickSource = NULL;
    self->panelSprite = NULL;
    Get_vtable_BasicClass()->removeAllChildren((BasicClass *)self);
}

void TextEntry__OnNotify(TextEntry *self, void *sender, s32 event) {
    s32 kind;

    Get_vtable_BasicClass()->onNotify((BasicClass *)self, sender, event);

    kind = ((BasicClass *)sender)->methods->header & 0xF;
    if (kind == PAD_CLASS_ID) {
        self->methods->handleCommand(self, sender, event);
    } else if (kind == FRAMECLOCK_CLASS_ID) {
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

/*
 * TextEntry__LoadCardResources's own helpers/data -- builds two "CARD\\<name>.TIM"
 * paths (BuildFileName, code_171e0.c), loads each through New_TimImage, and
 * makes panelSprite (New_ScreenSprite), textRow (New_TextRow) and
 * cursorSprite (New_CharSprite) from the loaded handles.
 */
extern char *BuildFileName(char *dest, const char *name, const char *dir, const char *ext);

extern const char sStrComInput[];          /* "COMINPUT" */
extern const char sStrFontIcon[];          /* "FONTICON" */
extern const char sCardPathPrefix[];       /* "CARD\\" */
extern const char sTimExt[];               /* ".TIM" */
extern SpriteRect gTextEntryPanelRect;     /* COMINPUT's cell: 224 x 120 from (0, 0) */
extern SpriteRgb gTextEntryTextColor;      /* the text row's colour: (128, 128, 0) */
extern ScreenSpritePos gTextEntryPanelPos; /* (-70, -60) */
extern ScreenSpritePos gTextEntryTextPos;  /* (-62, -15) */
extern ScreenSpritePos gTextEntryCursorPos; /* (-62, -12), y at D_8008AAE0: SetCursorPos adds pos * 7 to x */

void TextEntry__LoadCardResources(TextEntry *self, void *parent) {
    char path[32];
    const char *dir;
    const char *ext;
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
    self->panelSprite = New_ScreenSprite(panelTim, &gTextEntryPanelRect, 0);
    panelTim->methods->release(panelTim);
    self->panelSprite->methods->attachToParent(self->panelSprite, (SceneNode *)parent,
                                               (LongVec3 *)&gTextEntryPanelPos);

    fontTim = New_TimImage(BuildFileName(path, sStrFontIcon, dir, ext));
    ((TimImageUploadFn)fontTim->methods->processBuffer)(fontTim);
    self->textRow = New_TextRow(fontTim, self->textLen, self->editBuf);
    self->cursorSprite = New_CharSprite(fontTim, '_');
    fontTim->methods->release(fontTim);
    self->textRow->methods->attachToParent(self->textRow, (SceneNode *)parent,
                                           (LongVec3 *)&gTextEntryTextPos);
    self->textRow->methods->setColor(self->textRow, &gTextEntryTextColor);
    self->cursorSprite->methods->attachToParent(self->cursorSprite, (SceneNode *)parent,
                                                (LongVec3 *)&gTextEntryCursorPos);
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
    self->altCommands = 0;
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

/* TextEntry__HandleCommand's own name-copy helper -- uncarved elsewhere (`code_2cc8c_f`,
 * still `INCLUDE_ASM`), typed purely from this call site's own register
 * usage: `a0`/`a1` are `self->textBuf`/`self->editBuf` (both `char *`, the same
 * pair `strcpy` is fed in the other arm), return value unused. Same
 * declare-locally convention as `DecodeFullWidthSjis` above (a different unit
 * types this same-shaped function with a different signature from its own
 * call site). */
extern void EncodeFullWidthSjis(char *dest, char *src);

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
            self->methods->notifyTarget(self, 1 << 4);
            self->methods->setState(self, TEXTENTRY_RESULT_ACCEPTED);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_RDOWN:
            self->methods->notifyTarget(self, 1 << 4);
            self->methods->setState(self, TEXTENTRY_RESULT_CANCELLED);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_L2:
            self->methods->resetAllChars(self);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_L1:
            self->methods->resetChar(self);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_SELECT:
            self->methods->toggleAltCommands(self);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_LRIGHT:
            if (self->altCommands != 0) {
                return;
            }
            self->methods->moveCursorRight(self);
            return;
        case PAD_EVENT_HELD + PAD_BUTTON_LRIGHT:
            if (self->altCommands == 0) {
                return;
            }
            self->methods->moveCursorRight(self);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_LLEFT:
            if (self->altCommands != 0) {
                return;
            }
            self->methods->moveCursorLeft(self);
            return;
        case PAD_EVENT_HELD + PAD_BUTTON_LLEFT:
            if (self->altCommands == 0) {
                return;
            }
            self->methods->moveCursorLeft(self);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_LUP:
            if (self->altCommands != 0) {
                return;
            }
            self->methods->nextChar(self);
            return;
        case PAD_EVENT_HELD + PAD_BUTTON_LUP:
            if (self->altCommands == 0) {
                return;
            }
            self->methods->nextChar(self);
            return;
        case PAD_EVENT_PRESSED + PAD_BUTTON_LDOWN:
            if (self->altCommands == 0) {
                goto callPrevChar;
            }
            return;
        case PAD_EVENT_HELD + PAD_BUTTON_LDOWN:
            if (self->altCommands == 0) {
                return;
            }
        callPrevChar:
            self->methods->prevChar(self);
            return;
    }
}

void TextEntry__PlaySound(TextEntry *self, s32 tone) {
    VabStreamObj *target;

    target = self->target;
    if (target != NULL) {
        target->methods->playTone(target, tone, 96, 96);
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
