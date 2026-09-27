/*
 * class_3bb8c_g -- TaskObjF methods (include/TaskObjF.h), slots +0x07C..
 * +0x0B0 of gTaskObjFMethods (class_3bb8c_f holds +0x064..+0x078), the
 * table getter, plus one standalone helper (StampSaveTitleFileLetter) reused
 * by class_3bb8c_m's memcard save writer.
 *
 * TaskObjF runs a `state` machine: SetState notifies the parent, swaps the
 * card icon and runs the entry action of the new state (format, write, read,
 * or open a widget); AdvanceState and ForceIdleFromState answer the input
 * source's events, TickStateDelay the tick source's. The two widgets are
 * lazily attached in mirrored pairs: `textEntry` (a TextEntry, to edit the
 * title) and `itemList` (a ItemList, to choose among the existing files),
 * driven through the slots +0x044..+0x050 both classes put at the same
 * offsets; their results come back through OnTextEntryResult and
 * OnItemListResult. `cardIcon` is a ScreenSprite of a CARD\*.TIM message,
 * made by LoadCardIcon and released by ReleaseCardIcon.
 *
 * Every function in the unit is matched C. What each numeric `state` code
 * means in game terms is not established. See each function's own match
 * report for its evidence.
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

void TaskObjF__SetState(TaskObjF *self, s32 state) {
    TaskObjFMethods *methods = self->methods;
    s32 ok;
    s32 i;

    if (self->state == state) {
        state = 0x17;
    }

    methods->notifyParents(self, state);
    methods->releaseCardIcon(self);
    methods->loadCardIcon(self, state);

    self->waitCounter = 0;
    switch (state) {
        case 0x13:
            state = methods->formatCard(self) ? 0x11 : 8;
            methods->setState(self, state);
            break;
        case 0x14:
            if (self->fileName[0] == '\0') {
                methods->findUnusedMemcardName(self, self->fileName, self->namePrefix, self->nameSuffixes);
            }
            ok = methods->writeMemcardSaveFile(self, self->fileName, self->title, self->iconFrames,
                                               self->iconImage, self->data, self->dataSize);
            state = ok ? 0x16 : 0xC;
            methods->setState(self, state);
            break;
        case 0x15:
            ok = methods->readMemcardFile(self, self->fileName, self->data, self->dataSize);
            state = ok ? 0x16 : 0x10;
            methods->setState(self, state);
            break;
        case 0x11:
            methods->attachTextEntry(self);
            break;
        case 0x12:
            methods->attachItemList(self);
            break;
    }

    if ((u32)(state - 0x16) < 2) {
        if (self->opMode == 1 && self->titles != NULL) {
            BMemPMgrFree(self->foundSuffixes);
            for (i = 0; i < self->bufCount; i++) {
                BMemPMgrFree(self->titles[i]);
            }
            BMemPMgrFree(self->titles);
            self->titles = NULL;
        }
        self->state = 0;
        self->opMode = 0;
    } else {
        self->state = state;
    }
}

/* 0x11 (17) entries, indexed by `arg1` (range-checked `< 0x11` below);
 * mostly `char *` string pointers into rodata, a few raw literal words at
 * indices never reached from this call site. `asm/data/76DC8.data.s`. */
extern char *gCardIconNames[];
extern const char gCardPathPrefix[]; /* "CARD\\" */
extern const char gCardPathSuffix[]; /* ".TIM" */
/* 3 words, `New_ScreenSprite`'s rect: a SpriteRect {0, 0, 160, 120}. */
extern SpriteRect gCardIconRect;
/* opaque block, the fresh `cardIcon`'s own `slot4C` arg2, address-only here. */
extern ScreenSpritePos gCardIconPos;

void TaskObjF__LoadCardIcon(TaskObjF *self, s32 index) {
    char pathBuf[0x20];
    char *path;
    char *name;
    TimImage *tim;
    ScreenSprite *icon;

    if (index >= 0x11) {
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
    if (self->state != 0) {
        if (event == 0x19) {
            self->methods->advanceState(self);
        } else if (event == 0x17) {
            self->methods->forceIdleFromState(self);
        }
    }
}

void TaskObjF__PlaySound(TaskObjF *self, s32 index) {
    if (self->sound != NULL) {
        self->sound->methods->playTone(self->sound, index, 0x7F, 0x7F);
    }
}

void TaskObjF__AdvanceState(TaskObjF *self) {
    TaskObjFMethods *methods = self->methods;

    switch (self->state) {
        case 2:
        case 4:
        case 0xA:
        case 0xE:
            methods->playSound(self, 0);
            if (self->state == 0xE) {
                strcpy(self->fileName, self->namePrefix);
                strcat(self->fileName, self->foundSuffixes[self->selectedIndex]);
                strcpy(self->title, self->titles[self->selectedIndex]);
            }
            if (self->opMode == 2) {
                methods->beginSave(self, self->fileName, self->title, self->titleEditPos,
                                   self->iconFrames, self->iconImage, self->data, self->dataSize);
            } else if (self->opMode == 1) {
                methods->beginLoad(self, self->fileName, self->title, self->data, self->dataSize);
            }
            break;
        case 6:
            methods->playSound(self, 0);
            methods->setState(self, 7);
            break;
        case 3:
        case 5:
        case 8:
        case 9:
        case 0xC:
        case 0xD:
        case 0x10:
            methods->playSound(self, 0x10);
            methods->setState(self, 0x17);
            break;
    }
}

void TaskObjF__ForceIdleFromState(TaskObjF *self) {
    switch (self->state) {
        case 4:
        case 6:
        case 0xA:
        case 0xE:
            self->methods->playSound(self, 0x10);
            self->methods->setState(self, 0x17);
            break;
        default:
            break;
    }
}

void TaskObjF__TickStateDelay(TaskObjF *self) {
    s32 old;
    s32 count;

    if (self->state == 7) {
        old = self->waitCounter;
        count = old + 1;
        self->waitCounter = count;
        if (old < 6) {
            return;
        }
        self->methods->setState(self, 0x13);
    } else if (self->state == 0xB) {
        old = self->waitCounter;
        count = old + 1;
        self->waitCounter = count;
        if (old < 6) {
            return;
        }
        self->methods->setState(self, 0x14);
    } else if (self->state == 0xF) {
        old = self->waitCounter;
        count = old + 1;
        self->waitCounter = count;
        if (old < 6) {
            return;
        }
        self->methods->setState(self, 0x15);
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
        case 2:
            self->methods->detachTextEntry(self);
            self->methods->beginSave(self, self->fileName, self->title, self->titleEditPos,
                                     self->iconFrames, self->iconImage, self->data, self->dataSize);
            break;
        case 3:
            self->methods->detachTextEntry(self);
            self->methods->setState(self, 0x17);
            break;
    }
}

void TaskObjF__AttachItemList(TaskObjF *self) {
    if (self->spriteParent != 0 && self->inputSource != 0) {
        if (self->itemList == NULL) {
            self->itemList = New_ItemList(self->titles, 1);
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
        case 2:
            self->selectedIndex = list->methods->getCursorIndex(list);
            self->methods->detachItemList(self);
            self->methods->setState(self, 0xE);
            break;
        case 3:
            self->methods->detachItemList(self);
            self->methods->setState(self, 0x17);
            break;
    }
}

TaskObjFMethods *GetTaskObjFMethods(void) {
    return &gTaskObjFMethods;
}

/* Sony's, from libc2 (round 45's own local view -- this unit's first use). */
extern s32 atoi(char *s);

/* VALUE-of `%gp_rel`, round 45's own local view -- a fixed rodata template
 * (ROM image still-uncarved, `asm/data/1C34.rodata.s` region) this
 * function copies raw byte ranges out of; also read by
 * `class_3bb8c_d.c`'s own (differently-typed) local view. */

/* Struct-copy helper types for round 45's StampSaveTitleFileLetter, all deliberately
 * all-`s8` (alignment 1) per this round's FormatNumberIntoBuffer lever: retail
 * copies these ranges as one unaligned `lwl`/`lwr` word chunk per 4 bytes,
 * with any non-multiple-of-4 remainder as INDIVIDUAL byte loads/stores,
 * never merged into a halfword -- alignment 2 would let GCC trust a
 * halfword move retail does not have. */
typedef struct {
    s8 raw[6];
} Buf6_3bb8c_g;

typedef struct {
    s8 raw[12];
} Buf12_3bb8c_g;

typedef struct {
    s8 a, b;
} Pair2_3bb8c_g;

extern Pair2_3bb8c_g *gSaveTitleGlyphs;

/* Signature is `include/class_3bb8c.h`'s ALREADY-shared
 * `extern s32 StampSaveTitleFileLetter(s32 arg0, s32 arg1);` (class_3bb8c_m's own
 * caller, TaskObjF__WriteMemcardSaveFile), matched exactly -- this unit's own definition
 * must agree with that declaration since both are visible in this
 * translation unit. Cast to `u8 *` internally; retail's own register
 * content at exit (`$v0` left holding a pointer into the `gSaveTitleGlyphs`
 * template in every path) confirms the real return type is a pointer,
 * loosely read as `s32` by the caller that never dereferences it. */
s32 StampSaveTitleFileLetter(s32 titleAddr, s32 fileNameAddr) {
    Pair2_3bb8c_g *title = (Pair2_3bb8c_g *)titleAddr;
    char *fileName = (char *)fileNameAddr;
    s32 numberPos;
    s32 letter;
    Pair2_3bb8c_g *glyph;

    if (fileName != NULL) {
        numberPos = ((u32)(fileName[0xE] - 0x38) < 2) ? 0xE : 0xD;

        title[12] = gSaveTitleGlyphs[15];
        *(Buf12_3bb8c_g *)&title[3] = *(Buf12_3bb8c_g *)&gSaveTitleGlyphs[15];

        letter = atoi(fileName + numberPos) - 1;
        glyph = &gSaveTitleGlyphs[letter];
        title[4] = *glyph;
        return (s32)glyph;
    } else {
        Pair2_3bb8c_g *glyphs = gSaveTitleGlyphs;

        *(Buf6_3bb8c_g *)&title[3] = *(Buf6_3bb8c_g *)&glyphs[15];
        return (s32)glyphs;
    }
}
