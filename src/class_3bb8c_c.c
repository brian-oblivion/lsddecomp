/*
 * class_3bb8c_c -- the constructors and table methods of two small classes,
 * and TitleMenu's allocator and ctor with its two save-title helpers.
 *
 * NodeGuardedViewport (include/NodeGuardedViewport.h) is a Viewport whose
 * update skips the frame while no view node is attached or the ordering
 * table is not ready; its seven table methods, New_ and the table getter are
 * all here. GridCell (include/GridCell.h, a SceneNode) is one cell of
 * StageMap's grid, carrying the model placed there; its five table methods,
 * New_ and the getter are all here too. Each New_X allocates the object and
 * calls the ctor through the class's table; the ctor chains the parent's
 * ctor, then installs its own table.
 *
 * TitleMenu (include/TitleMenu.h, a TaskCore) is the menu between days; its
 * other methods are in class_3bb8c_d.c. Two free functions serve it:
 * UpdateFlashbackLock (called from TitleMenu__RefreshMenu) locks or unlocks
 * the menu's FLASHBACK entry from two words of the save block, and
 * StampSaveTitleDay (called from the ctor with the current day) writes the
 * day as three full-width digits into the save title, "LSD   Day001".
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "class_3bb8c.h"
#include "DreamSys.h"
#include "SceneNode.h"
#include "Actor.h"
#include "Viewport.h"
#include "NodeGuardedViewport.h"
#include "GridCell.h"
#include "TitleMenu.h"
#include "VabStreamObj.h"
#include "TextRow.h"
#include "TimImage.h"
#include "TaskObjF.h"
#include <kernel.h>
#include <sys/file.h>
#include "BasicClass.h"
#include "Pad.h"
#include "FrameClock.h"

NodeGuardedViewport *New_NodeGuardedViewport(void) {
    NodeGuardedViewport *self;

    self = BMemPMgrAlloc(sizeof(NodeGuardedViewport));
    if (self != NULL) {
        GetNodeGuardedViewportMethods()->ctor(self);
        return self;
    }
    return NULL;
}

void NodeGuardedViewport__NodeGuardedViewport(NodeGuardedViewport *self) {
    GetViewportMethods()->ctor((Viewport *)self);
    self->methods = GetNodeGuardedViewportMethods();
    self->methods->initDefaults(self);
}

void NodeGuardedViewport__InitDefaults(void) {}

void NodeGuardedViewport__Update(NodeGuardedViewport *self) {
    if (self->viewNode != NULL && self->otReady != 0) {
        GetViewportMethods()->update((Viewport *)self);
    }
}

void NodeGuardedViewport__NoOpSlotB8(void) {}

void NodeGuardedViewport__NoOpSlotBC(void) {}

void NodeGuardedViewport__NoOpSlotC0(void) {}

void NodeGuardedViewport__NoOpSlotC4(void) {}

NodeGuardedViewportMethods *GetNodeGuardedViewportMethods(void) {
    return &gNodeGuardedViewportMethods;
}

GridCell *New_GridCell(void) {
    GridCell *self;

    self = BMemPMgrAlloc(GRIDCELL_SIZE);
    if (self != NULL) {
        GetGridCellMethods()->ctor(self);
        return self;
    }
    return NULL;
}

void GridCell__GridCell(GridCell *self) {
    GetSceneNodeMethods()->ctor((SceneNode *)self);
    self->methods = GetGridCellMethods();
    self->unk34 = 0;
    self->flags36 = 0;
    self->nextInCell = NULL;
}

void GridCell__Reset(void) {}

/* Passes on only a link command from an Actor or a class below it (the low
 * byte of the sender's class id); Actor__DispatchLinkCommand makes the same
 * test for a GridCell sender. */
void GridCell__DispatchLinkCommand(GridCell *self, BasicClass *sender, s32 event) {
    if ((u8)sender->methods->header == ACTOR_CLASS_ID) {
        self->methods->onActorLinkCommand(self, sender, event);
    }
}

/* SceneNode's handling, then tryAttachNearby for events 5..8, the body of
 * Actor__OnActorLinkCommand. tryAttachNearby keeps SceneNode's one-parameter
 * slot type; this caller passes the sender and event too. */
void GridCell__OnActorLinkCommand(GridCell *self, void *sender, s32 event) {
    GetSceneNodeMethods()->dispatchLinkCommand((SceneNode *)self, sender, event);
    if (event < 9) {
        if (event >= 5) { /* MATCHING: nested, as && folds to one unsigned test */
            ((void (*)(GridCell *, void *, s32))self->methods->tryAttachNearby)(self, sender, event);
        }
    }
}

void *GridCell__ReturnSelf(GridCell *self) {
    return self;
}

GridCellMethods *GetGridCellMethods(void) {
    return &gGridCellMethods;
}

TitleMenu *New_TitleMenu(struct DreamSys *dreamSys) {
    TitleMenu *self;

    self = BMemPMgrAlloc(sizeof(TitleMenu));
    if (self != NULL) {
        GetTitleMenuMethods()->ctor(self, dreamSys);
        return self;
    }
    return NULL;
}

void TitleMenu__TitleMenu(TitleMenu *self, struct DreamSys *dreamSys) {
    VabStreamObj *sound;

    Get_vtable_TaskCore()->ctor((TaskCore *)self, &sTitleMenuTarget,
                                (char *)sTitleMenuSoundBankPath, NULL);
    self->methods = GetTitleMenuMethods();
    sound = (VabStreamObj *)self->sound;
    sound->methods->setPitchOffset(sound, -1); /* 36 semitones down */
    self->dreamSys = dreamSys;
    self->saveCtrl = NULL;
    self->saveBlock = dreamSys->methods->getSaveBlock(dreamSys, &self->saveBlockSize);
    StampSaveTitleDay(dreamSys->methods->getCurrentDayAndYear(dreamSys, NULL));
    self->methods->setTarget(self, &sTitleMenuTarget);
    ((TitleMenuResetCallFn)self->methods->resetCounters)(self, dreamSys);
}

/* The save block's total flashback unlock score must be past this for the
 * menu to offer FLASHBACK. */
#define FLASHBACK_UNLOCK_SCORE 9999999

/* FLASHBACK stays locked unless the unlock score is past the threshold and at
 * least one flashback is stored. `dreamSys` is unread: TitleMenu__RefreshMenu
 * passes self->dreamSys, and retail loads it into $a2 for the call. */
void UpdateFlashbackLock(TitleMenu *self, TaskCoreTarget *target, struct DreamSys *dreamSys) {
    DreamSaveBlock *save = (DreamSaveBlock *)self->saveBlock;
    s32 locked = 1;

    if (save->totalFlasbackUnlockScore > FLASHBACK_UNLOCK_SCORE) {
        locked = (save->amountFlashbacksAvailable == 0);
    }
    /* A NULL entry is one the cursor can stop on. */
    target->registrationSlots[TITLEMENU_FLASHBACK] = (void *)locked;
}

/* src/ScreenWidgets.c: `value` as `width` full-width decimal digits into dst,
 * zero-padded unless `unpadded`. */
extern void FormatFullWidthNumber(u8 *dst, s32 value, s32 width, s32 unpadded);

/* The save title's day number, full-width characters 9..11 of
 * "LSD   Day001" (class_3bb8c_g.c's layout of the title). */
#define SAVE_TITLE_DAY 9
#define SAVE_TITLE_DAY_DIGITS 3

/* Formats the day as three full-width digits in sDayDigits's buffer and
 * copies them into the save title's day number, characters 9..11. */
void StampSaveTitleDay(s32 day) {
    FormatFullWidthNumber(sDayDigits, day, SAVE_TITLE_DAY_DIGITS, 0);
    *(FullWidthChars3 *)&((FullWidthChar *)gSaveTitle)[SAVE_TITLE_DAY] = *(FullWidthChars3 *)sDayDigits;
}

/* ---- merged from class_3bb8c_d ---- */

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

/* Decodes full-width SJIS into one byte a character (src/ScreenWidgets.c);
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
    UpdateFlashbackLock(self, self->target, self->dreamSys);
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

/* InitCARD, StartCARD and _bu_init are Sony's (libcard): include/psyq/kernel.h. */

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

/* ---- merged from class_3bb8c_e ---- */

/*
 * TaskObjF's child links, card events, card checks and file probes
 * (include/TaskObjF.h: slots +0x00C..+0x018 and +0x040..+0x060, and the
 * helpers they call), in address order:
 *
 * - Children. TaskObjF__AddChild files a child by its class id into one of
 *   four links: a Pad as inputSource, a FrameClock as tickSource, a
 *   TextEntry, an ItemList. TaskObjF__RemoveChild and
 *   TaskObjF__RemoveAllChildren clear them; TaskObjF__ClearLinks clears
 *   them and spriteParent at construction.
 * - The card. TaskObjF__SetCardSlot picks slot 0 or 1 and its BIOS channel.
 *   TaskObjF__OpenEvents opens one SwCARD event per gCardEventSpecs entry;
 *   TaskObjF__CloseEvents closes them.
 * - Card checks. TaskObjF__CheckCardStatus retries
 *   TaskObjF__CardInfoAndLoadStatus, which asks _card_info whether a card is
 *   there and new (TaskObjF__CardInfoStatus), then _card_load whether it is
 *   formatted (TaskObjF__CardLoadStatus). TaskObjF__FormatCard retries
 *   format().
 * - Files, named "bu00:"/"bu10:" plus a prefix and a suffix
 *   (BuildMemcardPath). TaskObjF__ProbeMemcardFile opens one and can copy
 *   out its title (TaskObjF__OpenAndReadMemcardFile);
 *   TaskObjF__FindUnusedMemcardName and
 *   TaskObjF__CollectExistingMemcardFiles probe a list of suffixes through
 *   the probeMemcardFile slot. TaskObjF__CheckCardSpace retries
 *   TaskObjF__ProbeCardFreeSpace, which creates and deletes a TEMP file of
 *   the save's size.
 *
 * A retried call is tried once, then up to MEMCARD_RETRIES more times while
 * it fails (TaskObjF__ProbeMemcardFile's loop: no more times).
 */

/* Defined in class_3bb8c_f.c, which types `dest` as McDevicePath *: it
 * writes the device name, appends `suffix`, and returns `dest`. */
extern char *BuildMemcardPath(void *dest, s32 cardSlot, char *suffix);

/* "TEMP": the name TaskObjF__ProbeCardFreeSpace creates to test for space. */
extern char sMcTempFileSuffix[];

extern void *BMemPMgrAlloc(s32 size);
extern char *strcpy(char *dest, char *src);

void TaskObjF__ClearLinks(TaskObjF *self) {
    self->inputSource = NULL;
    self->tickSource = NULL;
    self->spriteParent = NULL;
    self->textEntry = NULL;
    self->itemList = NULL;
}

void TaskObjF__Finalize(TaskObjF *self) {
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void TaskObjF__AddChild(TaskObjF *self, BasicClass *child) {
    s32 classId;

    if (child == NULL) {
        return;
    }
    Get_vtable_BasicClass()->addChild((BasicClass *)self, child);
    classId = child->methods->header;
    if ((classId & CLASS_ID_ROOT_MASK) == PAD_CLASS_ID) {
        self->inputSource = child;
        return;
    }
    if ((classId & CLASS_ID_ROOT_MASK) == FRAMECLOCK_CLASS_ID) {
        self->tickSource = child;
        return;
    }
    if ((classId & 0xFF) == 0x10) {
        self->textEntry = (struct TextEntry *)child;
        return;
    }
    if ((classId & 0xFF) == 0x20) {
        self->itemList = (struct ItemList *)child;
    }
}

void TaskObjF__RemoveChild(TaskObjF *self, BasicClass *child) {
    s32 classId;

    if (child == NULL) {
        return;
    }
    classId = child->methods->header;
    if ((classId & CLASS_ID_ROOT_MASK) == PAD_CLASS_ID) {
        self->inputSource = NULL;
    } else if ((classId & CLASS_ID_ROOT_MASK) == FRAMECLOCK_CLASS_ID) {
        self->tickSource = NULL;
    } else if ((classId & 0xFF) == 0x10) {
        self->textEntry = NULL;
    } else if ((classId & 0xFF) == 0x20) {
        self->itemList = NULL;
    }
    Get_vtable_BasicClass()->removeChild((BasicClass *)self, child);
}

void TaskObjF__RemoveAllChildren(TaskObjF *self) {
    self->inputSource = NULL;
    self->tickSource = NULL;
    self->spriteParent = NULL;
    self->textEntry = NULL;
    self->itemList = NULL;
    Get_vtable_BasicClass()->removeAllChildren((BasicClass *)self);
}

void TaskObjF__SetCardSlot(TaskObjF *self, s32 cardSlot) {
    self->cardSlot = cardSlot;
    self->cardHandle = cardSlot << 4;
}

s32 TaskObjF__OpenEvents(TaskObjF *self) {
    s32 i;

    EnterCriticalSection();
    i = 0;
    do {
        self->events[i] = OpenEvent(SwCARD, gCardEventSpecs[i], EvMdNOINTR, NULL);
        i++;
    } while (i < ARRAY_COUNT(self->events));
    ExitCriticalSection();
    TaskObjF__EnableEvents(self);
    return 1;
}

s32 TaskObjF__CloseEvents(TaskObjF *self) {
    TaskObjF__DisableEvents(self);
    /* kernel.h spells CloseEvent with `long`, ForEachEvent's callback with s32. */
    TaskObjF__ForEachEvent(self, (s32 (*)(s32))CloseEvent, 1);
    return 1;
}

/* Nonzero when a card answered. *error is set when it answered with an error,
 * *cardChanged when the first or the last try found a new card, *formatted
 * when it is formatted. An error or an unformatted card also counts as a
 * failed try. */
s32 TaskObjF__CheckCardStatus(TaskObjF *self, s32 *error, s32 *cardChanged, s32 *formatted) {
    s32 retries;
    s32 firstChanged;
    s32 result;

    retries = MEMCARD_RETRIES;
    *cardChanged = 0;
    result = TaskObjF__CardInfoAndLoadStatus(self, error, &firstChanged, formatted);
    /* MATCHING: the retry count is tested after the call; testing it first
     * cross-jumps the two calls into one. */
    while (result == 0 || *error != 0 || *formatted == 0) {
        result = TaskObjF__CardInfoAndLoadStatus(self, error, cardChanged, formatted);
        if (retries-- == 0) {
            break;
        }
    }
    *cardChanged = *cardChanged | firstChanged;
    return result;
}

/* Returns whatever the last call left in $v0: CardInfoStatus's 0, or
 * CardLoadStatus's status.
 * MATCHING: an explicit return adds two words. */
s32 TaskObjF__CardInfoAndLoadStatus(TaskObjF *self, s32 *error, s32 *cardChanged, s32 *formatted) {
    if (TaskObjF__CardInfoStatus(self, error, cardChanged) != 0) {
        TaskObjF__CardLoadStatus(self, error, formatted);
    }
}

s32 TaskObjF__CardInfoStatus(TaskObjF *self, s32 *error, s32 *cardChanged) {
    s32 status;
    s32 answer;

    status = 1;
    /* MATCHING: one expression keeps both stores ahead of TestEvents. */
    *cardChanged = *error = 0;
    TaskObjF__TestEvents(self);
    while (_card_info(self->cardHandle) == 0)
        ;
    answer = TaskObjF__WaitForReadyEvent(self);
    if (answer == EvSpTIMOUT) {
        status = 0;
    } else if (answer == EvSpERROR) {
        status = 0;
        *error = 1;
    } else if (answer == EvSpNEW) {
        *cardChanged = 1;
        _card_clear(self->cardHandle);
    }
    return status;
}

s32 TaskObjF__CardLoadStatus(TaskObjF *self, s32 *error, s32 *formatted) {
    s32 status;
    s32 answer;

    status = 1;
    /* MATCHING: one expression keeps both stores ahead of TestEvents. */
    *formatted = (*error = 0, status);
    TaskObjF__TestEvents(self);
    while (_card_load(self->cardHandle) == 0)
        ;
    answer = TaskObjF__WaitForReadyEvent(self);
    if (answer == EvSpTIMOUT) {
        status = 0;
    } else if (answer == EvSpERROR) {
        status = 0;
        *error = 1;
    } else if (answer == EvSpNEW) {
        *formatted = 0;
    }
    return status;
}

s32 TaskObjF__FormatCard(TaskObjF *self) {
    s32 retries;
    s32 result;
    McDevicePath *path;

    retries = MEMCARD_RETRIES;
    do {
        path = self->cardSlot != 0 ? &gMcDevicePath1 : &gMcDevicePath0;
        result = format((char *)path); /* the device name, "bu00:" or "bu10:" */
    } while (result == 0 && retries-- != 0);
    return result;
}

/* Nonzero when the file prefix+suffix exists; copies its title to destTitle
 * unless that is NULL. An empty suffix names no file. */
s32 TaskObjF__ProbeMemcardFile(TaskObjF *self, char *destTitle, char *suffix) {
    s32 retries;
    s32 result;

    retries = 0; /* the class's retry loop, run once */
    if (suffix == NULL || *suffix == '\0') {
        return 0;
    }
    do {
        result = TaskObjF__OpenAndReadMemcardFile(self, destTitle, suffix);
    } while (result == 0 && retries-- != 0);
    return result;
}

s32 TaskObjF__OpenAndReadMemcardFile(TaskObjF *self, char *destTitle, char *suffix) {
    char pathBuf[32];
    char *path;
    s32 handle;
    void *header;

    path = BuildMemcardPath(pathBuf, self->cardSlot, suffix);
    handle = open(path, O_RDONLY);
    if (handle == -1) {
        return 0;
    }
    if (destTitle != NULL) {
        header = BMemPMgrAlloc(MEMCARD_SECTOR_SIZE);
        read(handle, header, MEMCARD_SECTOR_SIZE);
        strcpy(destTitle, (char *)header + 4); /* the save header's title */
        BMemPMgrFree(header);
    }
    close(handle);
    return 1;
}

char *TaskObjF__FindUnusedMemcardName(TaskObjF *self, char *buf, char *prefix, char **suffixes) {
    while (*suffixes != NULL) {
        strcpy(buf, prefix);
        strcat(buf, *suffixes);
        if (self->methods->probeMemcardFile(self, NULL, buf) == 0) {
            return buf;
        }
        suffixes++;
    }
    return NULL;
}

s32 TaskObjF__CollectExistingMemcardFiles(TaskObjF *self, char **destTitles, char **outSuffixes,
                                          char *prefix, char **suffixes) {
    s32 count;
    char name[32];

    count = 0;
    while (*suffixes != NULL) {
        strcpy(name, prefix);
        strcat(name, *suffixes);
        if (self->methods->probeMemcardFile(self, *destTitles, name) != 0) {
            count++;
            *outSuffixes = *suffixes;
            destTitles++;
            outSuffixes++;
        }
        suffixes++;
    }
    return count;
}

s32 TaskObjF__CheckCardSpace(TaskObjF *self, u8 iconFrames, s32 size) {
    s32 retries;
    s32 result;

    retries = MEMCARD_RETRIES;
    do {
        result = TaskObjF__ProbeCardFreeSpace(self, iconFrames, size);
    } while (result == 0 && retries-- != 0);
    return result;
}

/* Creates, then deletes, a file big enough for `size` bytes of save data
 * after the save header: nonzero when the card has room. */
s32 TaskObjF__ProbeCardFreeSpace(TaskObjF *self, u8 iconFrames, s32 size) {
    char pathBuf[32];
    char *path;
    s32 handle;
    s32 blocks;

    /* MATCHING: the u32 cast makes the shift retail's srl. */
    blocks = (u32)(size + MEMCARD_SAVE_HEADER_SIZE + MEMCARD_BLOCK_SIZE - 1) >> MEMCARD_BLOCK_SHIFT;
    path = BuildMemcardPath(pathBuf, self->cardSlot, sMcTempFileSuffix);
    handle = open(path, MEMCARD_OPEN_BLOCKS(blocks) | O_CREAT);
    if (handle == -1) {
        return 0;
    }
    close(handle);
    /* MATCHING: delete(path), the same address, changes the code. */
    delete (pathBuf);
    return 1;
}
