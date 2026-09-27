/*
 * class_3bb8c_d -- TitleMenu's methods (include/TitleMenu.h; its allocator
 * and ctor are in class_3bb8c_c.c) and its getter, then TaskObjF's
 * allocator and ctor (include/TaskObjF.h).
 *
 * TitleMenu is the TaskCore menu between days: START, FLASHBACK, SAVE, LOAD,
 * GRAPH and SHAKE over ETC\TITLE.TIM. In ROM order here: finalize, onNotify,
 * reset, onDeinit, setState and tick (which acts on the chosen entry),
 * refreshViewValue (stores SHAKE's setting), the four overrides that manage
 * the save-title TextRow in place of TaskCore's slot widgets, refreshMenu,
 * and the memory-card methods that drive `saveCtrl`, a TaskObjF, for SAVE
 * and LOAD. The header's banner describes the class.
 *
 * The data they share is in include/class_3bb8c.h: gSaveTitle, the
 * full-width save title the TextRow shows and the card save carries;
 * sSaveFileName; the card's name prefix and suffix table; and the colour
 * cycle's channel and frame counters.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "class_3bb8c.h"
#include "DreamSys.h"
#include "TextRow.h"
#include "TimImage.h"
#include "TitleMenu.h"
#include "TaskObjF.h"

void TitleMenu__Finalize(TitleMenu *self) {
    if (self->saveCtrl != NULL) {
        self->saveCtrl->methods->release(self->saveCtrl);
        self->saveIcon->methods->release(self->saveIcon);
    }
    Get_vtable_TaskCore()->finalize((TaskCore *)self);
}

void TitleMenu__OnNotify(TitleMenu *self, BasicClass *sender, s32 event) {
    Get_vtable_TaskCore()->onNotify((TaskCore *)self, sender, event);
    if ((sender->methods->header & 0xF) == TASKOBJF_CLASS_ID) {
        self->methods->onCardEvent(self, sender, event);
    }
}

void TitleMenu__Reset(TitleMenu *self) {
    self->unk34 = 0;
    self->unk2C = 400;
    self->methods->setSubHandle(self, sTitleTimPath, 0);
    self->methods->setFrameBound(self, 10);
    self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, 0, 0);
}

/* Replaces TaskCore's onDeinit: clears both display buffers to the colour
 * unk93. */
void TitleMenu__OnDeinit(TitleMenu *self) {
    u32 i;
    DrawRect *rect;

    i = 0;
    rect = sDisplayBufferRects;
    for (; i < 2; i++) {
        ((DrawSystem *)self->initArgs->drawSystem)
            ->methods->clearImage((DrawSystem *)self->initArgs->drawSystem, self->unk93, rect);
        rect++;
    }
}

void TitleMenu__SetState(TitleMenu *self, s32 state) {
    Get_vtable_TaskCore()->setState((TaskCore *)self, state);
    if (state == 5) {
        self->methods->refreshMenu(self, 0);
    }
    if (state == 0xA) {
        self->methods->onPadCancel(self);
        self->methods->setActiveSlot(self, self->target->unk8, 1);
        self->methods->onPadConfirm(self);
    }
}

void TitleMenu__Tick(TitleMenu *self) {
    void (*fn)(TitleMenu *);

    Get_vtable_TaskCore()->tick((TaskCore *)self);
    switch (self->activeSlot) {
        case TITLEMENU_FLASHBACK:
            self->result = 0;
            self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, 0, 1);
            fn = self->methods->refreshViewValue;
            break;
        case TITLEMENU_SAVE:
            fn = self->methods->saveToCard;
            break;
        case TITLEMENU_LOAD:
            fn = self->methods->loadFromCard;
            break;
        case TITLEMENU_GRAPH:
            self->result = TITLEMENU_RESULT_GRAPH;
            fn = self->methods->refreshViewValue;
            break;
        default:
            return;
    }
    fn(self);
}

void TitleMenu__RefreshViewValue(TitleMenu *self) {
    s32 shake;

    Get_vtable_TaskCore()->refreshViewValue((TaskCore *)self);
    shake = self->slotCounts[TITLEMENU_SHAKE];
    self->dreamSys->methods->getSetScreenShake(self->dreamSys, &shake);
}

/* Sony's (libc2). */
extern char *strcpy(char *dest, char *src);
extern s32 strlen(char *s);

/* Decodes full-width SJIS into one byte a character (src/code_2cc8c_f.c);
 * nothing here reads its result. */
extern void DecodeFullWidthSjis(void *dst, void *src);

/* gSaveTitle is 2-byte full-width characters; the characters from here on
 * are padding after "LSD   Day001" (class_3bb8c_g.c's SAVE_TITLE_PADDING). */
#define SAVE_TITLE_PADDING 12
/* beginSave's titleEditPos: the player's text goes in from the character
 * after the padding's first space. */
#define SAVE_TITLE_EDIT_POS 13

/* The setTarget override: `target` is the TaskCoreTarget the ctor passes
 * (&sTitleMenuTarget); only its `handle` is read, as the TextRow's texture. On a
 * new game the title's padding is reblanked and its letter field cleared
 * first. The TextRow has a cell a character plus 4, eight of them shown
 * from cell 4, with a gap before cell 9. */
void TitleMenu__CreateSaveTitle(TitleMenu *self, TaskCoreTarget *target) {
    u32 cellCount;
    char *text;

    if (target == NULL) {
        return;
    }
    if (self->dreamSys->methods->getNewGameFlag(self->dreamSys)) {
        strcpy(gSaveTitle + SAVE_TITLE_PADDING * 2, sSaveTitleBlanks);
        StampSaveTitleFileLetter(gSaveTitle, NULL);
    }
    cellCount = strlen(gSaveTitle);
    cellCount = (cellCount >> 1) + 4;
    text = BMemPMgrAlloc(cellCount);
    DecodeFullWidthSjis(text, gSaveTitle);
    self->saveTitle = New_TextRow(target->handle, cellCount, text);
    self->saveTitle->visibleCount = 8;
    self->saveTitle->firstVisible = 4;
    self->saveTitle->gapIndex = 9;
    BMemPMgrFree(text);
}

void TitleMenu__DestroySaveTitle(TitleMenu *self) {
    self->saveTitle->methods->release(self->saveTitle);
    Get_vtable_TaskCore()->releaseTarget((TaskCore *)self);
}

void TitleMenu__AttachSaveTitle(TitleMenu *self, void *parent) {
    Get_vtable_TaskCore()->updateSlotElements((TaskCore *)self, parent);
    self->saveTitle->methods->attachToParent(self->saveTitle, (SceneNode *)parent,
                                             (LongVec3 *)&sSaveTitleOffset);
}

/* The save title's colour cycle: a lit channel's level (or what it adds),
 * the frames of each cycle that light red, and the cycle's length. */
#define SAVE_TITLE_LIT 128
#define SAVE_TITLE_RED_FRAMES 128
#define SAVE_TITLE_CYCLE_FRAMES 257

/* The broadcastToSlots override, once a frame: the save title's colour.
 * While the menu takes input it is black with one channel lit, the channel
 * moving each frame; otherwise `color` with red lifted for the first frames
 * of each cycle and the moving channel after.
 * MATCHING: `channels` is taken before the first call (it lives in $s1)
 * and `rgb = *color` is one struct copy. */
void TitleMenu__CycleSaveTitleColor(TitleMenu *self, SpriteRgb *color) {
    SpriteRgb rgb;
    u8 *channels;

    channels = (u8 *)&rgb;
    Get_vtable_TaskCore()->broadcastToSlots((TaskCore *)self, (u8 *)color);
    if (self->inputMode != 0) {
        channels[0] = 0;
        channels[1] = 0;
        channels[2] = 0;
        channels[sSaveTitleColorChannel] = SAVE_TITLE_LIT;
    } else {
        rgb = *color;
        if (sSaveTitleColorFrame < SAVE_TITLE_RED_FRAMES) {
            channels[0] += SAVE_TITLE_LIT;
        } else {
            channels[sSaveTitleColorChannel] += SAVE_TITLE_LIT;
        }
    }
    if (++sSaveTitleColorChannel >= 3) {
        sSaveTitleColorChannel = 0;
    }
    sSaveTitleColorFrame++;
    if (sSaveTitleColorFrame >= SAVE_TITLE_CYCLE_FRAMES) {
        sSaveTitleColorFrame = 0;
    }
    self->saveTitle->methods->setColor(self->saveTitle, &rgb);
}

/* FLASHBACK's lock (src/class_3bb8c_c.c). */
extern void CheckSaveScoreFlag(TitleMenu *self, TaskCoreTarget *target,
                               struct DreamSys *dreamSys); /* arity-ok: the definition takes 2; retail loads dreamSys into $a2 here */

/* setState(5)'s and a finished card operation's: the save title's text reloaded,
 * FLASHBACK's lock recomputed, the widgets re-attached and SHAKE's cursor
 * set from DreamSys's setting; then the entry that was active is
 * reselected. */
void TitleMenu__RefreshMenu(TitleMenu *self) {
    s32 cellCount;
    s32 origSlot;
    char *text;
    s32 shake;

    cellCount = self->saveTitle->cellCount;
    origSlot = self->activeSlot;
    text = BMemPMgrAlloc(cellCount);
    DecodeFullWidthSjis(text, gSaveTitle);
    self->saveTitle->methods->setText(self->saveTitle, text);
    BMemPMgrFree(text);
    CheckSaveScoreFlag(self, self->target, self->dreamSys);
    self->methods->updateSlotElements(self, self->unk14);
    self->dreamSys->methods->getSetScreenShake(self->dreamSys, &shake);
    self->activeSlot = TITLEMENU_SHAKE;
    self->methods->setState(self, 0xB);
    self->methods->setSlotCursor(self, shake, 1);
    self->methods->setState(self, 0xF);
    self->methods->setActiveSlot(self, origSlot, 0);
    self->dreamSys->methods->getSetScreenShake(self->dreamSys, &shake);
}

void TitleMenu__BeginCardAccess(TitleMenu *self) {
    if (self->saveCtrl == NULL) {
        self->saveIcon = New_TimImage((char *)sSaveIconTimPath);
        self->saveCtrl = New_TaskObjF(1, 0);
    }
    self->saveCtrl->methods->init(self->saveCtrl, sCardFilePrefix, sSaveFileSuffixes,
                                  self->initArgs->pad, self->unk10, (struct SceneNode *)self->unk14,
                                  (struct VabStreamObj *)self->sound);
    self->methods->addChild(self, (BasicClass *)self->saveCtrl);
    self->methods->removeChild(self, self->initArgs->pad);
    self->methods->removeChild(self, self->unk10);
}

void TitleMenu__EndCardAccess(TitleMenu *self) {
    self->methods->addChild(self, self->initArgs->pad);
    self->methods->addChild(self, self->unk10);
    self->methods->removeChild(self, (BasicClass *)self->saveCtrl);
    self->saveCtrl->methods->deinit(self->saveCtrl);
}

void TitleMenu__SaveToCard(TitleMenu *self) {
    s32 shake;

    shake = self->slotCounts[TITLEMENU_SHAKE];
    self->dreamSys->methods->getSetScreenShake(self->dreamSys, &shake);
    self->methods->beginCardAccess(self);
    if (self->dreamSys->methods->getNewGameFlag(self->dreamSys)) {
        sSaveFileName[0] = '\0';
    }
    self->saveCtrl->methods->beginSave(self->saveCtrl, sSaveFileName, gSaveTitle, SAVE_TITLE_EDIT_POS,
                                       3, self->saveIcon, self->saveBlock, self->saveBlockSize);
}

void TitleMenu__LoadFromCard(TitleMenu *self) {
    self->methods->beginCardAccess(self);
    self->saveCtrl->methods->beginLoad(self->saveCtrl, sSaveFileName, gSaveTitle, self->saveBlock,
                                       self->saveBlockSize);
}

/* saveCtrl's terminal states: either gives input back; a completed save or
 * load also clears the new-game flag and refreshes the menu. */
void TitleMenu__OnCardEvent(TitleMenu *self, BasicClass *sender, s32 event) {
    if (event <= TASKOBJF_STATE_ABORTED) {
        if (event >= TASKOBJF_STATE_DONE) {
            self->methods->endCardAccess(self);
            if (event == TASKOBJF_STATE_DONE) {
                self->dreamSys->methods->clearNewGameFlag(self->dreamSys);
                self->methods->refreshMenu(self, TASKOBJF_STATE_DONE);
            }
        }
    }
}

TitleMenuMethods *GetTitleMenuMethods(void) {
    return &gTitleMenuMethods;
}

TaskObjF *New_TaskObjF(s32 padEnable, s32 cardSlot) {
    TaskObjF *self;

    self = BMemPMgrAlloc(sizeof(TaskObjF));
    if (self == NULL) {
        goto fail;
    }
    GetTaskObjFMethods()->ctor(self, padEnable, cardSlot);
    return self;
fail:
    return NULL;
}

/* Sony's (libcard). */
extern void InitCARD(s32 padEnable);
extern void StartCARD(void);
extern void _bu_init(void);

/* The card libraries are started once, by the first TaskObjF made. */
void TaskObjF__TaskObjF(TaskObjF *self, s32 padEnable, s32 cardSlot) {
    s32 count;

    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetTaskObjFMethods();
    count = sTaskObjFCount;
    sTaskObjFCount = count + 1;
    if (count == 0) {
        InitCARD(padEnable);
        StartCARD();
        _bu_init();
    }
    TaskObjF__ClearLinks(self);
    self->methods->setCardSlot(self, cardSlot);
}
