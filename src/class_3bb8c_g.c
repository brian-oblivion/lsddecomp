/*
 * class_3bb8c_g -- TaskObjF's state machine (include/TaskObjF.h): slots
 * +0x07C..+0x0B0 of gTaskObjFMethods and the table getter, plus
 * StampSaveTitleFileLetter, which writes a save file's letter into its
 * title. TaskObjF's other methods are in TitleMenuTaskObjF.c, and this unit
 * joins that file (FINISHING-PLAN track 8) once its .rodata line (the two
 * jump tables) is renamed to it: unitfile.py leaves that yaml edit to the
 * head.
 *
 * setState (enum TaskObjFState) notifies the parent, swaps the message icon
 * (a ScreenSprite of CARD\<name>.TIM: loadCardIcon, releaseCardIcon) and
 * runs the new state's entry action: format, write or read the card, or
 * attach the title editor (a TextEntry) or the file chooser (an ItemList).
 * A circle press on the Pad reaches advanceState (retry, format, go on) and
 * a cross press abortFromState (abort); the tick source counts down
 * FORMATTING, SAVING and LOADING before their action runs. The two widgets
 * are made on first use, driven through the slots +0x044..+0x050 both
 * classes put at the same offsets, and report back through
 * onTextEntryResult and onItemListResult.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "class_3bb8c.h"
#include "ScreenSprite.h"
#include "TextEntry.h"
#include "ItemList.h"
#include "TimImage.h"
#include "VabStreamObj.h"
#include "TaskObjF.h"
#include "TitleMenu.h"
#include "Pad.h"

/* MATCHING: `methods` is cached, and the cases are in retail's code order
 * (the entry actions before the widgets). */
void TaskObjF__SetState(TaskObjF *self, s32 state) {
    TaskObjFMethods *methods = self->methods;
    s32 ok;
    s32 i;

    if (self->state == state) {
        state = TASKOBJF_STATE_ABORTED;
    }

    methods->notifyParents(self, state);
    methods->releaseCardIcon(self);
    methods->loadCardIcon(self, state);

    self->waitCounter = 0;
    switch (state) {
        case TASKOBJF_STATE_FORMAT:
            state = methods->formatCard(self) ? TASKOBJF_STATE_EDIT_TITLE : TASKOBJF_STATE_FORMAT_ERROR;
            methods->setState(self, state);
            break;
        case TASKOBJF_STATE_WRITE:
            if (self->fileName[0] == '\0') {
                methods->findUnusedMemcardName(self, self->fileName, self->namePrefix, self->nameSuffixes);
            }
            ok = methods->writeMemcardSaveFile(self, self->fileName, self->title, self->iconFrames,
                                               self->iconImage, self->data, self->dataSize);
            state = ok ? TASKOBJF_STATE_DONE : TASKOBJF_STATE_SAVE_ERROR;
            methods->setState(self, state);
            break;
        case TASKOBJF_STATE_READ:
            ok = methods->readMemcardFile(self, self->fileName, self->data, self->dataSize);
            state = ok ? TASKOBJF_STATE_DONE : TASKOBJF_STATE_LOAD_ERROR;
            methods->setState(self, state);
            break;
        case TASKOBJF_STATE_EDIT_TITLE:
            methods->attachTextEntry(self);
            break;
        case TASKOBJF_STATE_CHOOSE_FILE:
            methods->attachItemList(self);
            break;
    }

    if ((u32)(state - TASKOBJF_STATE_DONE) < 2) { /* DONE or ABORTED */
        if (self->opMode == TASKOBJF_OP_LOAD && self->titles != NULL) {
            BMemPMgrFree(self->foundSuffixes);
            for (i = 0; i < self->bufCount; i++) {
                BMemPMgrFree(self->titles[i]);
            }
            BMemPMgrFree(self->titles);
            self->titles = NULL;
        }
        self->state = TASKOBJF_STATE_IDLE;
        self->opMode = TASKOBJF_OP_NONE;
    } else {
        self->state = state;
    }
}

/* The message icon's name per state, CARD\<name>.TIM ("NOCONECT" ..
 * "LOADERR" for states 2..16). Entries 0 and 1 are not names; no setState
 * call passes 0 or 1. */
extern char *gCardIconNames[TASKOBJF_STATE_EDIT_TITLE];
extern const char gCardPathPrefix[]; /* "CARD\\" */
extern const char gCardPathSuffix[]; /* ".TIM" */
/* {0, 0, 160, 120} */
extern SpriteRect gCardIconRect;
/* (-70, -60), percent of half the screen from the centre */
extern ScreenSpritePos gCardIconPos;

void TaskObjF__LoadCardIcon(TaskObjF *self, s32 index) {
    char pathBuf[32];
    char *path;
    char *name;
    TimImage *tim;
    ScreenSprite *icon;

    /* MATCHING: `path` and `icon` keep the buffer and the sprite in saved
     * registers across the calls. */
    if (index >= ARRAY_COUNT(gCardIconNames)) {
        return;
    }
    if (self->spriteParent == 0) {
        return;
    }
    if (self->cardIcon != NULL) {
        return;
    }

    path = pathBuf;
    name = gCardIconNames[index];
    path[0] = '\0';
    strcat(path, gCardPathPrefix);
    strcat(path, name);
    strcat(path, gCardPathSuffix);

    tim = New_TimImage(path);
    ((TimImageUploadFn)tim->methods->processBuffer)(tim);
    icon = New_ScreenSprite(tim, &gCardIconRect, 0);
    self->cardIcon = icon;
    tim->methods->release(tim);
    icon->methods->attachToParent(icon, self->spriteParent, (LongVec3 *)&gCardIconPos);
}

void TaskObjF__ReleaseCardIcon(TaskObjF *self) {
    if (self->cardIcon != NULL) {
        self->cardIcon = self->cardIcon->methods->release(self->cardIcon);
    }
}

void TaskObjF__OnInputEvent(TaskObjF *self, void *sender, s32 event) {
    if (self->state != TASKOBJF_STATE_IDLE) {
        if (event == PAD_EVENT_PRESSED + PAD_BUTTON_RRIGHT) {
            self->methods->advanceState(self);
        } else if (event == PAD_EVENT_PRESSED + PAD_BUTTON_RDOWN) {
            self->methods->abortFromState(self);
        }
    }
}

void TaskObjF__PlaySound(TaskObjF *self, s32 index) {
    if (self->sound != NULL) {
        self->sound->methods->playTone(self->sound, index, 127, 127);
    }
}

void TaskObjF__AdvanceState(TaskObjF *self) {
    TaskObjFMethods *methods = self->methods;

    switch (self->state) {
        case TASKOBJF_STATE_NO_CARD:
        case TASKOBJF_STATE_CARD_CHANGED:
        case TASKOBJF_STATE_SAVE_OVERWRITE_WARNING:
        case TASKOBJF_STATE_LOAD_WARNING:
            methods->playSound(self, 0 << 4);
            if (self->state == TASKOBJF_STATE_LOAD_WARNING) {
                strcpy(self->fileName, self->namePrefix);
                strcat(self->fileName, self->foundSuffixes[self->selectedIndex]);
                strcpy(self->title, self->titles[self->selectedIndex]);
            }
            /* MATCHING: an if chain; a switch tests LOAD first. */
            if (self->opMode == TASKOBJF_OP_SAVE) {
                methods->beginSave(self, self->fileName, self->title, self->titleEditPos,
                                   self->iconFrames, self->iconImage, self->data, self->dataSize);
            } else if (self->opMode == TASKOBJF_OP_LOAD) {
                methods->beginLoad(self, self->fileName, self->title, self->data, self->dataSize);
            }
            break;
        case TASKOBJF_STATE_UNFORMATTED_SAVE:
            methods->playSound(self, 0 << 4);
            methods->setState(self, TASKOBJF_STATE_FORMATTING);
            break;
        case TASKOBJF_STATE_CARD_ERROR:
        case TASKOBJF_STATE_UNFORMATTED_LOAD:
        case TASKOBJF_STATE_FORMAT_ERROR:
        case TASKOBJF_STATE_SAVE_NO_SPACE:
        case TASKOBJF_STATE_SAVE_ERROR:
        case TASKOBJF_STATE_LOAD_NOT_FOUND:
        case TASKOBJF_STATE_LOAD_ERROR:
            methods->playSound(self, 1 << 4);
            methods->setState(self, TASKOBJF_STATE_ABORTED);
            break;
    }
}

void TaskObjF__AbortFromState(TaskObjF *self) {
    switch (self->state) {
        case TASKOBJF_STATE_CARD_CHANGED:
        case TASKOBJF_STATE_UNFORMATTED_SAVE:
        case TASKOBJF_STATE_SAVE_OVERWRITE_WARNING:
        case TASKOBJF_STATE_LOAD_WARNING:
            self->methods->playSound(self, 1 << 4);
            self->methods->setState(self, TASKOBJF_STATE_ABORTED);
            break;
        default:
            break;
    }
}

/* Waits until `waitCounter` (zeroed by setState) passes 6, then runs the
 * state's action. MATCHING: `old` and `count` apart, and one call per
 * branch. */
void TaskObjF__TickStateDelay(TaskObjF *self) {
    s32 old;
    s32 count;

    if (self->state == TASKOBJF_STATE_FORMATTING) {
        old = self->waitCounter;
        count = old + 1;
        self->waitCounter = count;
        if (old < 6) {
            return;
        }
        self->methods->setState(self, TASKOBJF_STATE_FORMAT);
    } else if (self->state == TASKOBJF_STATE_SAVING) {
        old = self->waitCounter;
        count = old + 1;
        self->waitCounter = count;
        if (old < 6) {
            return;
        }
        self->methods->setState(self, TASKOBJF_STATE_WRITE);
    } else if (self->state == TASKOBJF_STATE_LOADING) {
        old = self->waitCounter;
        count = old + 1;
        self->waitCounter = count;
        if (old < 6) {
            return;
        }
        self->methods->setState(self, TASKOBJF_STATE_READ);
    }
}

void TaskObjF__AttachTextEntry(TaskObjF *self) {
    if (self->spriteParent != 0 && self->inputSource != 0) {
        if (self->textEntry == NULL) {
            self->textEntry = New_TextEntry(&self->title[self->titleEditPos * 2], 1);
            self->ownsWidget = 1;
        }
        self->methods->addChild(self, (BasicClass *)self->textEntry);
        self->textEntry->methods->loadCardResources(self->textEntry, self->spriteParent);
        self->textEntry->methods->attachTarget(self->textEntry, self->inputSource, self->tickSource,
                                               self->sound);
    }
}

void TaskObjF__DetachTextEntry(TaskObjF *self) {
    if (self->spriteParent != 0 && self->inputSource != 0 && self->textEntry != NULL) {
        self->textEntry->methods->detachTarget(self->textEntry);
        self->textEntry->methods->releaseCardResources(self->textEntry);
        if (self->ownsWidget != 0) {
            self->textEntry->methods->release(self->textEntry);
            self->textEntry = NULL;
        }
    }
}

void TaskObjF__OnTextEntryResult(TaskObjF *self, void *sender, s32 result) {
    switch (result) {
        case TEXTENTRY_RESULT_ACCEPTED:
            self->methods->detachTextEntry(self);
            self->methods->beginSave(self, self->fileName, self->title, self->titleEditPos,
                                     self->iconFrames, self->iconImage, self->data, self->dataSize);
            break;
        case TEXTENTRY_RESULT_CANCELLED:
            self->methods->detachTextEntry(self);
            self->methods->setState(self, TASKOBJF_STATE_ABORTED);
            break;
    }
}

void TaskObjF__AttachItemList(TaskObjF *self) {
    if (self->spriteParent != 0 && self->inputSource != 0) {
        if (self->itemList == NULL) {
            self->itemList = New_ItemList(self->titles, ITEMLIST_MODE_FULLWIDTH);
            self->ownsWidget = 1;
        }
        self->methods->addChild(self, (BasicClass *)self->itemList);
        self->itemList->methods->loadResources(self->itemList, self->spriteParent);
        self->itemList->methods->attachTarget(self->itemList, self->inputSource, self->tickSource,
                                              self->sound);
    }
}

void TaskObjF__DetachItemList(TaskObjF *self) {
    if (self->spriteParent != 0 && self->inputSource != 0 && self->itemList != NULL) {
        self->itemList->methods->detachTarget(self->itemList);
        self->itemList->methods->releaseResources(self->itemList);
        if (self->ownsWidget != 0) {
            self->itemList->methods->release(self->itemList);
            self->itemList = NULL;
        }
    }
}

void TaskObjF__OnItemListResult(TaskObjF *self, ItemList *list, s32 result) {
    switch (result) {
        case ITEMLIST_RESULT_CHOSEN:
            self->selectedIndex = list->methods->getCursorIndex(list);
            self->methods->detachItemList(self);
            self->methods->setState(self, TASKOBJF_STATE_LOAD_WARNING);
            break;
        case ITEMLIST_RESULT_CANCELLED:
            self->methods->detachItemList(self);
            self->methods->setState(self, TASKOBJF_STATE_ABORTED);
            break;
    }
}

TaskObjFMethods *GetTaskObjFMethods(void) {
    return &gTaskObjFMethods;
}

/* The save title is full-width (2-byte SJIS) characters. TitleMenu's (the
 * buffer gSaveTitle points at) starts as "LSD   Day001", all full-width:
 * "LSD" (0..2), the letter field (3..5), "Day" (6..8), the day number
 * (9..11), then padding. */
#define SAVE_TITLE_LETTER_FIELD 3
#define SAVE_TITLE_LETTER 4
#define SAVE_TITLE_PADDING 12
/* gSaveTitleGlyphs: the full-width letters a..o (0..14), one per save file
 * -01..-15, then three full-width spaces and "Day" (15..20). */
#define SAVE_TITLE_GLYPH_SPACES 15
/* A save file name is namePrefix ("BISLPS-01556", 12 characters) + "-NN";
 * the first digit of NN. */
#define SAVE_FILE_NAME_NUMBER 13

/* Sony's (libc2). A leading 0 makes it parse octal. */
extern s32 atoi(char *s);

extern FullWidthChar *gSaveTitleGlyphs;

/* Writes a save file's letter into the full-width `title`: the letter
 * field becomes a space, the letter for the file name's -NN (a for -01 ..
 * o for -15) and a space, followed by "Day", and a space goes after the day
 * number. With no file name it only blanks the letter field. -08 and -09
 * are parsed from their second digit, which atoi would otherwise read as
 * octal. Returns a pointer into gSaveTitleGlyphs that no caller reads.
 * MATCHING:
 * `glyphs` is the return value, not a second read of the global. */
s32 StampSaveTitleFileLetter(char *titleText, char *fileName) {
    FullWidthChar *title = (FullWidthChar *)titleText;
    s32 numberPos;
    s32 letter;
    FullWidthChar *glyph;

    if (fileName != NULL) {
        numberPos = ((u32)(fileName[SAVE_FILE_NAME_NUMBER + 1] - '8') < 2) ? SAVE_FILE_NAME_NUMBER + 1
                                                                           : SAVE_FILE_NAME_NUMBER;

        title[SAVE_TITLE_PADDING] = gSaveTitleGlyphs[SAVE_TITLE_GLYPH_SPACES];
        *(FullWidthChars6 *)&title[SAVE_TITLE_LETTER_FIELD] =
            *(FullWidthChars6 *)&gSaveTitleGlyphs[SAVE_TITLE_GLYPH_SPACES];

        letter = atoi(fileName + numberPos) - 1;
        glyph = &gSaveTitleGlyphs[letter];
        title[SAVE_TITLE_LETTER] = *glyph;
        return (s32)glyph;
    } else {
        FullWidthChar *glyphs = gSaveTitleGlyphs;

        *(FullWidthChars3 *)&title[SAVE_TITLE_LETTER_FIELD] =
            *(FullWidthChars3 *)&glyphs[SAVE_TITLE_GLYPH_SPACES];
        return (s32)glyphs;
    }
}
