/*
 * title_menu -- the menu between days and its memory-card task:
 * TitleMenu and TaskObjF, led in by the constructors and table methods of
 * two small classes, NodeGuardedViewport and GridCell.
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
 * TitleMenu (include/TitleMenu.h, a TaskCore) is the menu between days:
 * its allocator and ctor with two save-title helpers, then its methods and
 * getter, each run introduced below. Two free functions serve it:
 * UpdateFlashbackLock (called from TitleMenu__RefreshMenu) locks or unlocks
 * the menu's FLASHBACK entry from two words of the save block, and
 * StampSaveTitleDay (called from the ctor with the current day) writes the
 * day as three full-width digits into the save title, "LSD   Day001".
 *
 * TaskObjF (include/TaskObjF.h) is the memory-card task TitleMenu's SAVE
 * and LOAD drive: its allocator and ctor, then its child links, card
 * events, checks and file probes, then its file I/O, buffers and the two
 * operations, and last its state machine, its getter and
 * StampSaveTitleFileLetter.
 *
 * NodeGuardedViewport and GridCell belong with dream_day.c's code:
 * DayTask's ctor makes the NodeGuardedViewport and StageMap is GridCell's
 * only maker.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <strings.h>
#include "dream_sys.h"
#include "scene_node.h"
#include "Actor.h"
#include "Viewport.h"
#include "NodeGuardedViewport.h"
#include "GridCell.h"
#include "TitleMenu.h"
#include "VabStreamObj.h"
#include "TextRow.h"
#include "tim_image.h"
#include "TaskObjF.h"
#include <kernel.h>
#include <sys/file.h>
#include "basic_class.h"
#include "pad.h"
#include "FrameClock.h"
#include "ScreenSprite.h"
#include "TextEntry.h"
#include "ItemList.h"
#include "bmem_pmgr.h"
#include "FullWidthSjis.h"
#include <stdio.h>
#include <convert.h>

/* Per TaskObjF::events slot: the event spec TaskObjF__OpenEvents passes to
 * OpenEvent, and the value WaitForReadyEvent returns for that slot.
 * Unsized: only the four slots are read. */
extern s32 sCardEventSpecs[];

extern McDevicePath sMcDevicePath1; /* "bu10:" */
extern McDevicePath sMcDevicePath0; /* "bu00:" */

/* TaskObjF__TaskObjF's construction count: InitCARD/StartCARD/_bu_init run
 * only on the first construction, when it was 0 before the increment. */
extern s32 sTaskObjFCount;

/* TitleMenu's menu description, a TaskCoreTarget: TitleMenu__TitleMenu
 * passes &sTitleMenuTarget as TaskCore's ctor's `target` and again to setTarget. */
extern TaskCoreTarget sTitleMenuTarget;

/* "ETC\ETCSE", TitleMenu__TitleMenu's soundBankPath for TaskCore's ctor
 * (the ctor casts away the const for its `char *`). */
extern const char sTitleMenuSoundBankPath[];

/* "ETC\TITLE.TIM", TitleMenu__Reset's path for setSubHandle. */
extern const char sTitleTimPath[];

/* "CARD\FILEICN1.TIM", TitleMenu__BeginCardAccess's path for New_TimImage.
 * A string splat already emitted as a symbol: a literal would emit a
 * second copy. */
extern const char sSaveIconTimPath[];

/* The two 320 x 240 display buffers, stacked in VRAM at y 0 and y 240:
 * TitleMenu__OnDeinit clears each with the DrawSystem's clearImage. */
extern DrawRect sDisplayBufferRects[2];

/* TitleMenu__AttachSaveTitle's position for the save title's attachToParent
 * (-4, -23: percent of half the screen from the centre). A TextRow's
 * position is a ScreenSpritePos (include/TextRow.h), passed through
 * SceneNode's LongVec3 slot. */
extern struct ScreenSpritePos sSaveTitleOffset;

/*
 * The save file's name and title, as TaskObjF's beginSave/beginLoad take
 * them (`fileName`, `title`): TitleMenu__SaveToCard and
 * TitleMenu__LoadFromCard pass both. The ROM image points them into the
 * rodata block at D_80011434: sSaveFileName at "BISLPS-01556xxx", which
 * SaveToCard empties on a new game; sSaveTitle at the full-width
 * "LSD   Day001" followed by 19 full-width spaces, which
 * TitleMenu__CreateSaveTitle reblanks from its 12th character on a new game
 * and StampSaveTitleDay writes the day into.
 */
extern char *sSaveFileName;
extern char *sSaveTitle;

/* The buffer StampSaveTitleDay formats the day into
 * (FormatFullWidthNumber) before copying it into sSaveTitle's title. The
 * ROM image points it at the "7654321" string D_8008AA1C. */
extern void *sDayDigits;

/* 19 full-width spaces, the tail TitleMenu__CreateSaveTitle copies over
 * sSaveTitle's on a new game (the ROM image points it just past
 * sSaveFileName's string). */
extern char *sSaveTitleBlanks;

/* TaskObjF's init `namePrefix` (TitleMenu__BeginCardAccess): the ROM image
 * points it at the product code "BISLPS-01556" in D_80011434. */
extern char *sCardFilePrefix;

/* TaskObjF's init `nameSuffixes` (TitleMenu__BeginCardAccess): the 15 file
 * suffixes "-01" (D_8008AA0C) to "-15" (D_8008A9D4), then NULL. */
extern char *sSaveFileSuffixes[];

/* TitleMenu__CycleSaveTitleColor's index (0, 1, 2) into its 3-byte colour.
 * The storage is a word; every access is a byte (lbu/sb). */
extern u8 sSaveTitleColorChannel;

/* TitleMenu__CycleSaveTitleColor's frame counter (wraps to 0 at 0x101). */
extern s32 sSaveTitleColorFrame;

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

/* SceneNode's handling, then tryAttachNearby for the Actor move events (ACTOR_EVENT_UNSWEPT..MOVED_Y), the body of
 * Actor__OnActorLinkCommand. tryAttachNearby keeps SceneNode's one-parameter
 * slot type; this caller passes the sender and event too. */
void GridCell__OnActorLinkCommand(GridCell *self, void *sender, s32 event) {
    GetSceneNodeMethods()->dispatchLinkCommand((SceneNode *)self, sender, event);
    if (event <= ACTOR_EVENT_MOVED_Y) {
        if (event >= ACTOR_EVENT_UNSWEPT) { /* MATCHING: nested, as && folds to one unsigned test */
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

    GetTaskCoreMethods()->ctor((TaskCore *)self, &sTitleMenuTarget, (char *)sTitleMenuSoundBankPath, NULL);
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

    if (save->totalFlashbackUnlockScore > FLASHBACK_UNLOCK_SCORE) {
        locked = (save->amountFlashbacksAvailable == 0);
    }
    /* A NULL entry is one the cursor can stop on. */
    target->hiddenSlots[TITLEMENU_FLASHBACK] = (void *)locked;
}

/* The save title's day number, full-width characters 9..11 of
 * "LSD   Day001" (StampSaveTitleFileLetter's layout of the title, below). */
#define SAVE_TITLE_DAY 9
#define SAVE_TITLE_DAY_DIGITS 3

/* Formats the day as three full-width digits in sDayDigits's buffer and
 * copies them into the save title's day number, characters 9..11. */
void StampSaveTitleDay(s32 day) {
    FormatFullWidthNumber(sDayDigits, day, SAVE_TITLE_DAY_DIGITS, 0);
    *(FullWidthChars3 *)&((FullWidthChar *)sSaveTitle)[SAVE_TITLE_DAY] = *(FullWidthChars3 *)sDayDigits;
}

/*
 * TitleMenu's methods (include/TitleMenu.h; its allocator and ctor are
 * above) and its getter, then TaskObjF's allocator and ctor
 * (include/TaskObjF.h).
 *
 * TitleMenu is the TaskCore menu between days: START, FLASHBACK, SAVE, LOAD,
 * GRAPH and SHAKE over ETC\TITLE.TIM. In ROM order here: finalize, onNotify,
 * reset, onDeinit, setState and confirmSlot (which acts on the chosen entry),
 * exit (stores SHAKE's setting), the four overrides that manage
 * the save-title TextRow in place of TaskCore's slot widgets, refreshMenu,
 * and the memory-card methods that drive `saveCtrl`, a TaskObjF, for SAVE
 * and LOAD. The header's banner describes the class.
 *
 * The data they share is in include/TitleMenu.h: sSaveTitle, the
 * full-width save title the TextRow shows and the card save carries;
 * sSaveFileName; the card's name prefix and suffix table; and the colour
 * cycle's channel and frame counters.
 */

void TitleMenu__Finalize(TitleMenu *self) {
    if (self->saveCtrl != NULL) {
        self->saveCtrl->methods->release(self->saveCtrl);
        self->saveIcon->methods->release(self->saveIcon);
    }
    GetTaskCoreMethods()->finalize((TaskCore *)self);
}

void TitleMenu__OnNotify(TitleMenu *self, BasicClass *sender, s32 event) {
    GetTaskCoreMethods()->onNotify((TaskCore *)self, sender, event);
    if ((sender->methods->header & CLASS_ID_ROOT_MASK) == TASKOBJF_CLASS_ID) {
        self->methods->onCardEvent(self, sender, event);
    }
}

void TitleMenu__Reset(TitleMenu *self) {
    self->clearOnDeinit = 0;
    self->maxPackets = 400;
    self->methods->setSubHandle(self, sTitleTimPath, 0);
    self->methods->setFrameBound(self, 10);
    self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, 0, 0);
}

/* Replaces TaskCore's onDeinit: clears both display buffers to the colour
 * clearColor. */
void TitleMenu__OnDeinit(TitleMenu *self) {
    u32 i;
    DrawRect *rect;

    i = 0;
    rect = sDisplayBufferRects;
    for (; i < 2; i++) {
        ((DrawSystem *)self->initArgs->drawSystem)
            ->methods->clearImage((DrawSystem *)self->initArgs->drawSystem, self->clearColor, rect);
        rect++;
    }
}

void TitleMenu__SetState(TitleMenu *self, s32 state) {
    GetTaskCoreMethods()->setState((TaskCore *)self, state);
    if (state == TASKCORE_STATE_ACTIVE) {
        self->methods->refreshMenu(self, 0);
    }
    if (state == TASKCORE_STATE_START_PRESSED) {
        self->methods->onPadCancel(self);
        self->methods->setActiveSlot(self, self->target->initialSlot, 1);
        self->methods->onPadConfirm(self);
    }
}

void TitleMenu__ConfirmSlot(TitleMenu *self) {
    void (*fn)(TitleMenu *);

    GetTaskCoreMethods()->confirmSlot((TaskCore *)self);
    switch (self->activeSlot) {
        case TITLEMENU_FLASHBACK:
            self->result = 0;
            self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, 0, 1);
            fn = self->methods->exit;
            break;
        case TITLEMENU_SAVE:
            fn = self->methods->saveToCard;
            break;
        case TITLEMENU_LOAD:
            fn = self->methods->loadFromCard;
            break;
        case TITLEMENU_GRAPH:
            self->result = TITLEMENU_RESULT_GRAPH;
            fn = self->methods->exit;
            break;
        default:
            return;
    }
    fn(self);
}

void TitleMenu__Exit(TitleMenu *self) {
    s32 shake;

    GetTaskCoreMethods()->exit((TaskCore *)self);
    shake = self->itemCursors[TITLEMENU_SHAKE];
    self->dreamSys->methods->getSetScreenShake(self->dreamSys, &shake);
}

/* sSaveTitle is 2-byte full-width characters; the characters from here on
 * are padding after "LSD   Day001". */
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
        strcpy(sSaveTitle + SAVE_TITLE_PADDING * 2, sSaveTitleBlanks);
        StampSaveTitleFileLetter(sSaveTitle, NULL);
    }
    cellCount = strlen(sSaveTitle);
    cellCount = (cellCount >> 1) + 4;
    text = BMemPMgrAlloc(cellCount);
    DecodeFullWidthSjis(text, sSaveTitle);
    self->saveTitle = New_TextRow(target->handle, cellCount, text);
    self->saveTitle->visibleCount = 8;
    self->saveTitle->firstVisible = 4;
    self->saveTitle->gapIndex = 9;
    BMemPMgrFree(text);
}

void TitleMenu__DestroySaveTitle(TitleMenu *self) {
    self->saveTitle->methods->release(self->saveTitle);
    GetTaskCoreMethods()->releaseTarget((TaskCore *)self);
}

void TitleMenu__AttachSaveTitle(TitleMenu *self, void *parent) {
    GetTaskCoreMethods()->updateSlotElements((TaskCore *)self, parent);
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
void TitleMenu__CycleSaveTitleColor(TitleMenu *self, ColorRgb *color) {
    ColorRgb rgb;
    u8 *channels;

    channels = (u8 *)&rgb;
    GetTaskCoreMethods()->broadcastToSlots((TaskCore *)self, (u8 *)color);
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

/* setState(ACTIVE)'s and a finished card operation's: the save title's text reloaded,
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
    DecodeFullWidthSjis(text, sSaveTitle);
    self->saveTitle->methods->setText(self->saveTitle, text);
    BMemPMgrFree(text);
    UpdateFlashbackLock(self, self->target, self->dreamSys);
    self->methods->updateSlotElements(self, self->lightRig);
    self->dreamSys->methods->getSetScreenShake(self->dreamSys, &shake);
    self->activeSlot = TITLEMENU_SHAKE;
    self->methods->setState(self, TASKCORE_STATE_SLOT_CONFIRMED);
    self->methods->setSlotCursor(self, shake, 1);
    self->methods->setState(self, TASKCORE_STATE_ITEM_CONFIRMED);
    self->methods->setActiveSlot(self, origSlot, 0);
    self->dreamSys->methods->getSetScreenShake(self->dreamSys, &shake);
}

void TitleMenu__BeginCardAccess(TitleMenu *self) {
    if (self->saveCtrl == NULL) {
        self->saveIcon = New_TimImage((char *)sSaveIconTimPath);
        self->saveCtrl = New_TaskObjF(1, 0);
    }
    self->saveCtrl->methods->init(
        self->saveCtrl, sCardFilePrefix, sSaveFileSuffixes, self->initArgs->pad, self->frameClock,
        (struct SceneNode *)self->lightRig, (struct VabStreamObj *)self->sound);
    self->methods->addChild(self, (BasicClass *)self->saveCtrl);
    self->methods->removeChild(self, self->initArgs->pad);
    self->methods->removeChild(self, self->frameClock);
}

void TitleMenu__EndCardAccess(TitleMenu *self) {
    self->methods->addChild(self, self->initArgs->pad);
    self->methods->addChild(self, self->frameClock);
    self->methods->removeChild(self, (BasicClass *)self->saveCtrl);
    self->saveCtrl->methods->deinit(self->saveCtrl);
}

void TitleMenu__SaveToCard(TitleMenu *self) {
    s32 shake;

    shake = self->itemCursors[TITLEMENU_SHAKE];
    self->dreamSys->methods->getSetScreenShake(self->dreamSys, &shake);
    self->methods->beginCardAccess(self);
    if (self->dreamSys->methods->getNewGameFlag(self->dreamSys)) {
        sSaveFileName[0] = '\0';
    }
    self->saveCtrl->methods->beginSave(self->saveCtrl, sSaveFileName, sSaveTitle, SAVE_TITLE_EDIT_POS,
                                       3, self->saveIcon, self->saveBlock, self->saveBlockSize);
}

void TitleMenu__LoadFromCard(TitleMenu *self) {
    self->methods->beginCardAccess(self);
    self->saveCtrl->methods->beginLoad(self->saveCtrl, sSaveFileName, sSaveTitle, self->saveBlock,
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

    GetBasicClassMethods()->ctor((BasicClass *)self);
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
 *   TaskObjF__OpenEvents opens one SwCARD event per sCardEventSpecs entry;
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

/* Defined below, after the file I/O: writes the device name into `dest`,
 * appends `suffix`, and returns `dest`. */
char *BuildMemcardPath(McDevicePath *dest, s32 cardSlot, char *suffix);

/* "TEMP": the name TaskObjF__ProbeCardFreeSpace creates to test for space. */
extern char sMcTempFileSuffix[];

void TaskObjF__ClearLinks(TaskObjF *self) {
    self->inputSource = NULL;
    self->tickSource = NULL;
    self->spriteParent = NULL;
    self->textEntry = NULL;
    self->itemList = NULL;
}

void TaskObjF__Finalize(TaskObjF *self) {
    GetBasicClassMethods()->finalize((BasicClass *)self);
}

void TaskObjF__AddChild(TaskObjF *self, BasicClass *child) {
    s32 classId;

    if (child == NULL) {
        return;
    }
    GetBasicClassMethods()->addChild((BasicClass *)self, child);
    classId = child->methods->header;
    if ((classId & CLASS_ID_ROOT_MASK) == PAD_CLASS_ID) {
        self->inputSource = child;
        return;
    }
    if ((classId & CLASS_ID_ROOT_MASK) == FRAMECLOCK_CLASS_ID) {
        self->tickSource = child;
        return;
    }
    if ((u8)classId == TEXTENTRY_CLASS_ID) {
        self->textEntry = (struct TextEntry *)child;
        return;
    }
    if ((u8)classId == ITEMLIST_CLASS_ID) {
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
    } else if ((u8)classId == TEXTENTRY_CLASS_ID) {
        self->textEntry = NULL;
    } else if ((u8)classId == ITEMLIST_CLASS_ID) {
        self->itemList = NULL;
    }
    GetBasicClassMethods()->removeChild((BasicClass *)self, child);
}

void TaskObjF__RemoveAllChildren(TaskObjF *self) {
    self->inputSource = NULL;
    self->tickSource = NULL;
    self->spriteParent = NULL;
    self->textEntry = NULL;
    self->itemList = NULL;
    GetBasicClassMethods()->removeAllChildren((BasicClass *)self);
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
        self->events[i] = OpenEvent(SwCARD, sCardEventSpecs[i], EvMdNOINTR, NULL);
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
        path = self->cardSlot != 0 ? &sMcDevicePath1 : &sMcDevicePath0;
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
    char pathBuf[MEMCARD_PATH_SIZE];
    char *path;
    s32 handle;
    McSaveHeader *header;

    path = BuildMemcardPath((McDevicePath *)pathBuf, self->cardSlot, suffix);
    handle = open(path, O_RDONLY);
    if (handle == -1) {
        return 0;
    }
    if (destTitle != NULL) {
        header = BMemPMgrAlloc(MEMCARD_SECTOR_SIZE);
        read(handle, header, MEMCARD_SECTOR_SIZE);
        strcpy(destTitle, header->title);
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
    char pathBuf[MEMCARD_PATH_SIZE];
    char *path;
    s32 handle;
    s32 blocks;

    /* MATCHING: the u32 cast makes the shift retail's srl. */
    blocks = (u32)(size + MEMCARD_SAVE_HEADER_SIZE + MEMCARD_BLOCK_SIZE - 1) >> MEMCARD_BLOCK_SHIFT;
    path = BuildMemcardPath((McDevicePath *)pathBuf, self->cardSlot, sMcTempFileSuffix);
    handle = open(path, MEMCARD_OPEN_BLOCKS(blocks) | O_CREAT);
    if (handle == -1) {
        return 0;
    }
    close(handle);
    /* MATCHING: delete(path), the same address, changes the code. */
    delete (pathBuf);
    return 1;
}

/*
 * TaskObjF's file I/O, card events, load buffers and its two operations
 * (include/TaskObjF.h: slots +0x064..+0x078 and the +0x038 onNotify
 * override), in address order:
 *
 * - Memory-card files. TaskObjF__ReadMemcardFile and
 *   TaskObjF__WriteMemcardSaveFile retry TaskObjF__TryReadMemcardFile and
 *   TaskObjF__TryWriteMemcardSaveFile up to MEMCARD_RETRIES more times.
 *   Both open a "bu00:"/"bu10:" path (BuildMemcardPath) with the BIOS file
 *   calls: the write builds the PS-X save header (McSaveHeader) from the
 *   title and the icon TIM, and the read seeks past it.
 * - Card events. TaskObjF__EnableEvents/DisableEvents/TestEvents apply one
 *   kernel call to each of `events` through TaskObjF__ForEachEvent;
 *   WaitForReadyEvent spins until one tests ready.
 * - TaskObjF__Init and TaskObjF__Deinit.
 * - TaskObjF__BeginLoad and TaskObjF__BeginSave store the request, check
 *   the card (TaskObjF__Validate) and choose the next TaskObjFState;
 *   TaskObjF__AllocBuffers/FreeUnusedBuffers/FreeBuffers manage the load's
 *   title buffers.
 * - TaskObjF__OnNotify routes a child's notification by the child's class
 *   id, the way TaskObjF__AddChild files the children.
 */

/* Defined below, called earlier in address order. */
s32 WaitForReadyEvent(s32 *events, s32 count);

/* The PS-X BIOS file calls (open, read, lseek, close, write, delete) and the
 * kernel event calls are Sony's libapi: include/psyq/kernel.h. */

s32 TaskObjF__ReadMemcardFile(TaskObjF *self, char *suffix, void *outBuf, s32 outSize) {
    s32 retries;
    s32 result;

    retries = MEMCARD_RETRIES;
    do {
        result = TaskObjF__TryReadMemcardFile(self, suffix, outBuf, outSize);
        if (result != 0) {
            break;
        }
    } while (retries-- != 0);
    return result;
}

s32 TaskObjF__TryReadMemcardFile(TaskObjF *self, char *suffix, void *outBuf, s32 outSize) {
    char pathBuf[MEMCARD_PATH_SIZE];
    char *path;
    s32 handle;
    McSaveHeader *header;
    s32 seekPos;
    u8 iconFlag;

    path = BuildMemcardPath((McDevicePath *)pathBuf, self->cardSlot, suffix);
    handle = open(path, O_RDONLY);
    if (handle == -1) {
        return 0;
    }
    /* Only the title sector: its icon display flag says where the data starts. */
    header = BMemPMgrAlloc(MEMCARD_SECTOR_SIZE);
    read(handle, header, MEMCARD_SECTOR_SIZE);
    iconFlag = header->iconDisplayFlag;
    /* The data follows the title sector and the icon frames.
     * MATCHING: (iconFlag - 0xF) * MEMCARD_SECTOR_SIZE reorders the arithmetic. */
    seekPos =
        (iconFlag << MEMCARD_SECTOR_SHIFT) - ((MEMCARD_ICON_FLAG_BASE - 1) << MEMCARD_SECTOR_SHIFT);
    BMemPMgrFree(header);
    lseek(handle, seekPos, SEEK_SET);
    read(handle, outBuf, outSize);
    close(handle);
    return 1;
}

s32 TaskObjF__WriteMemcardSaveFile(TaskObjF *self, char *fileName, char *title, char iconFrames,
                                   struct TimImage *icon, void *data, s32 size) {
    s32 retries;
    s32 result;

    retries = MEMCARD_RETRIES;
    StampSaveTitleFileLetter(title, fileName);
    do {
        result = TaskObjF__TryWriteMemcardSaveFile(self, fileName, title, iconFrames & 0xFF, icon,
                                                   data, size);
        if (result != 0) {
            break;
        }
    } while (retries-- != 0);
    if (result == 0) {
        StampSaveTitleFileLetter(title, NULL);
    }
    return result;
}

extern char sFileNotCreatedMsg[]; /* "File not create in WriteFile\n" */

/* Deletes the file, creates it at its full size in blocks, then reopens it
 * to write the save header and the data, each rounded up to whole sectors.
 * MATCHING: iconFrames is u8: retail keeps the incoming word and its
 * zero-extended copy in two registers. */

s32 TaskObjF__TryWriteMemcardSaveFile(TaskObjF *self, char *fileName, char *title, u8 iconFrames,
                                      struct TimImage *icon, void *data, s32 size) {
    char pathBuf[MEMCARD_PATH_SIZE];
    char *path;
    s32 fileHandle;
    s32 openMode;
    McIconSource *iconSrc;
    McSaveHeader *header;

    path = BuildMemcardPath((McDevicePath *)pathBuf, self->cardSlot, fileName);
    delete (path);
    openMode = MEMCARD_OPEN_BLOCKS(((u32)size + (MEMCARD_SAVE_HEADER_SIZE + MEMCARD_BLOCK_SIZE - 1)) >>
                                   MEMCARD_BLOCK_SHIFT) |
               O_CREAT;
    fileHandle = open(path, openMode);
    if (fileHandle == -1) {
        printf(sFileNotCreatedMsg);
        return 0;
    }
    close(fileHandle);
    fileHandle = open(path, O_WRONLY);
    if (fileHandle == -1) {
        return 0;
    }
    iconSrc = (McIconSource *)icon->buffer;
    header = (McSaveHeader *)BMemPMgrAlloc(sizeof(McSaveHeader));
    header->magic0 = 'S';
    header->magic1 = 'C';
    header->iconDisplayFlag = iconFrames + MEMCARD_ICON_FLAG_BASE;
    header->blockCount = ((u32)size + (MEMCARD_BLOCK_SIZE - 1)) >> MEMCARD_BLOCK_SHIFT;
    strcpy(header->title, title);
    header->palette[0] = iconSrc->palette[0];
    header->palette[1] = iconSrc->palette[1];
    header->frame0 = iconSrc->frame0;
    header->frame1 = iconSrc->frame1;
    header->frame2 = iconSrc->frame2;
    /* The title sector and the icon frames.
     * MATCHING: (iconFrames + 1) * MEMCARD_SECTOR_SIZE reorders the arithmetic. */
    write(fileHandle, header, (iconFrames << MEMCARD_SECTOR_SHIFT) + MEMCARD_SECTOR_SIZE);
    BMemPMgrFree(header);
    write(fileHandle, data,
          (((u32)size + (MEMCARD_SECTOR_SIZE - 1)) >> MEMCARD_SECTOR_SHIFT) << MEMCARD_SECTOR_SHIFT);
    close(fileHandle);
    return 1;
}

char *BuildMemcardPath(McDevicePath *dest, s32 cardSlot, char *suffix) {
    McDevicePath *device;

    if (cardSlot) {
        device = &sMcDevicePath1;
    } else {
        device = &sMcDevicePath0;
    }
    *dest = *device;
    strcat((char *)dest, suffix);
    return (char *)dest;
}

/* Sony's kernel event calls (libapi/a11..a13, include/psyq/kernel.h). Each
 * takes an event descriptor and returns a status word, which is what lets
 * TaskObjF__ForEachEvent take them as its callback; kernel.h spells them with
 * `long`, hence the casts. */

s32 TaskObjF__EnableEvents(TaskObjF *self) {
    return TaskObjF__ForEachEvent(self, (s32 (*)(s32))EnableEvent, 1);
}

s32 TaskObjF__DisableEvents(TaskObjF *self) {
    return TaskObjF__ForEachEvent(self, (s32 (*)(s32))DisableEvent, 1);
}

s32 TaskObjF__TestEvents(TaskObjF *self) {
    return TaskObjF__ForEachEvent(self, (s32 (*)(s32))TestEvent, 0);
}

/* Calls `callback` on each event until one returns 0, inside a critical
 * section when `critical` is set; returns the last result. */
s32 TaskObjF__ForEachEvent(TaskObjF *self, s32 (*callback)(s32), s32 critical) {
    s32 i;
    s32 result;

    if (critical) {
        EnterCriticalSection();
    }
    for (i = 0; i < ARRAY_COUNT(self->events); i++) {
        result = callback(self->events[i]);
        if (result == 0) {
            break;
        }
    }
    if (critical) {
        ExitCriticalSection();
    }
    return result;
}

s32 TaskObjF__WaitForReadyEvent(TaskObjF *self) {
    return WaitForReadyEvent(self->events, ARRAY_COUNT(self->events));
}

/* Spins until one of `count` events tests ready and returns that slot's
 * sCardEventSpecs entry: which answer the card gave. */
s32 WaitForReadyEvent(s32 *events, s32 count) {
    s32 i;

    for (;;) {
        for (i = 0; i < count; i++) {
            if (TestEvent(events[i]) != 0) {
                return sCardEventSpecs[i];
            }
        }
    }
}

void TaskObjF__Init(TaskObjF *self, char *namePrefix, char **nameSuffixes, BasicClass *inputSource,
                    BasicClass *tickSource, struct SceneNode *spriteParent, struct VabStreamObj *sound) {
    self->namePrefix = namePrefix;
    self->nameSuffixes = nameSuffixes;
    self->titles = NULL;
    self->spriteParent = spriteParent;
    self->sound = sound;
    self->methods->addChild(self, inputSource);
    self->methods->addChild(self, tickSource);
    self->cardIcon = NULL;
    self->state = TASKOBJF_STATE_IDLE;
    self->opMode = TASKOBJF_OP_NONE;
}

void TaskObjF__Deinit(TaskObjF *self) {
    self->sound = NULL;
    self->spriteParent = NULL;
    self->methods->removeChild(self, self->inputSource);
    self->methods->removeChild(self, self->tickSource);
}

/* Re-entered from LOAD_WARNING once the player has chosen a file, which it
 * then loads; otherwise it offers the files it found. */
void TaskObjF__BeginLoad(TaskObjF *self, char *fileName, char *title, void *data, s32 size) {
    s32 found;
    s32 state;

    self->fileName = fileName;
    self->title = title;
    self->data = data;
    self->opMode = TASKOBJF_OP_LOAD;
    self->dataSize = size;
    if (TaskObjF__Validate(self)) {
        TaskObjF__FreeBuffers(self);
        TaskObjF__AllocBuffers(self);
        found = self->methods->collectExistingMemcardFiles(self, self->titles, self->foundSuffixes,
                                                           self->namePrefix, self->nameSuffixes);
        self->bufCount = found;
        if (found != 0) {
            TaskObjF__FreeUnusedBuffers(self);
            if (self->state == TASKOBJF_STATE_LOAD_WARNING) {
                state = TASKOBJF_STATE_LOADING;
            } else {
                state = TASKOBJF_STATE_CHOOSE_FILE;
            }
        } else {
            state = TASKOBJF_STATE_LOAD_NOT_FOUND;
            self->bufCount = TASKOBJF_MAX_FILES;
        }
        self->methods->setState(self, state);
    }
}

void TaskObjF__AllocBuffers(TaskObjF *self) {
    s32 i;

    if (self->titles == NULL) {
        self->titles = BMemPMgrAlloc((TASKOBJF_MAX_FILES + 1) * sizeof(char *));
        for (i = 0; i < TASKOBJF_MAX_FILES; i++) {
            self->titles[i] = BMemPMgrAlloc(TASKOBJF_TITLE_SIZE);
        }
        self->foundSuffixes = BMemPMgrAlloc((TASKOBJF_MAX_FILES + 1) * sizeof(char *));
    }
}

void TaskObjF__FreeUnusedBuffers(TaskObjF *self) {
    s32 i;

    for (i = self->bufCount; i < TASKOBJF_MAX_FILES; i++) {
        self->titles[i] = BMemPMgrFree(self->titles[i]);
    }
    self->titles[i] = NULL;
}

void TaskObjF__FreeBuffers(TaskObjF *self) {
    s32 i;

    if (self->titles != NULL) {
        BMemPMgrFree(self->foundSuffixes);
        for (i = 0; i < self->bufCount; i++) {
            BMemPMgrFree(self->titles[i]);
        }
        BMemPMgrFree(self->titles);
        self->titles = NULL;
    }
}

/* An existing file first asks to overwrite; confirming re-enters from
 * SAVE_OVERWRITE_WARNING to edit the title, and the edited title re-enters
 * from EDIT_TITLE to save.
 * MATCHING: each branch makes its own setState call, which GCC cross-jumps
 * to one; the function returns nothing. */
void TaskObjF__BeginSave(TaskObjF *self, char *fileName, char *title, s32 titleEditPos,
                         u8 iconFrames, struct TimImage *icon, void *data, s32 size) {
    s32 state;
    TaskObjFMethods *methods;

    self->fileName = fileName;
    self->title = title;
    self->titleEditPos = titleEditPos;
    self->opMode = TASKOBJF_OP_SAVE;
    self->iconFrames = iconFrames;
    self->iconImage = icon;
    self->data = data;
    self->dataSize = size;
    if (TaskObjF__Validate(self)) {
        if (self->methods->probeMemcardFile(self, NULL, fileName) != 0) {
            state = TASKOBJF_STATE_SAVE_OVERWRITE_WARNING;
            if (self->state == state) {
                state = TASKOBJF_STATE_EDIT_TITLE;
            } else if (self->state == TASKOBJF_STATE_EDIT_TITLE) {
                state = TASKOBJF_STATE_SAVING;
            }
            self->methods->setState(self, state);
        } else if (!self->methods->checkCardSpace(self, iconFrames, size)) {
            self->methods->setState(self, TASKOBJF_STATE_SAVE_NO_SPACE);
        } else {
            methods = self->methods;
            state = TASKOBJF_STATE_EDIT_TITLE;
            if (self->state == state) {
                state = TASKOBJF_STATE_SAVING;
            }
            methods->setState(self, state);
        }
    }
}

/* Returns 1 when checkCardStatus succeeds on a formatted, unchanged card;
 * otherwise sets the state that says why and returns 0.
 * MATCHING: the unreachable `formatted` branch and the one setState call
 * reached by goto keep retail's branch layout. */
s32 TaskObjF__Validate(TaskObjF *self) {
    s32 error;
    s32 cardChanged;
    s32 formatted;
    s32 ok;
    s32 state;

    self->methods->openEvents(self);
    ok = self->methods->checkCardStatus(self, &error, &cardChanged, &formatted);
    self->methods->closeEvents(self);

    if (ok != 0) {
        if (cardChanged == 0 && formatted != 0) {
            return 1;
        }
    }

    if (ok == 0) {
        state = TASKOBJF_STATE_NO_CARD;
    } else if (error != 0) {
        state = TASKOBJF_STATE_CARD_ERROR;
    } else if (cardChanged != 0) {
        state = TASKOBJF_STATE_CARD_CHANGED;
    } else if (formatted != 0) {
        goto dispatch;
    } else if (self->opMode == TASKOBJF_OP_LOAD) {
        state = TASKOBJF_STATE_UNFORMATTED_LOAD;
    } else {
        state = TASKOBJF_STATE_UNFORMATTED_SAVE;
    }

dispatch:
    self->methods->setState(self, state);
    return 0;
}

void TaskObjF__OnNotify(TaskObjF *self, void *sender, s32 event) {
    TaskObjFMethods *methods;
    BasicClassMethods *base;
    s32 tag;
    s32 kind;

    methods = self->methods;
    base = GetBasicClassMethods();
    base->onNotify((BasicClass *)self, sender, event);

    tag = ((BasicClass *)sender)->methods->header;
    kind = tag & CLASS_ID_ROOT_MASK;
    if (kind == PAD_CLASS_ID) {
        methods->onInputEvent(self, sender, event);
    } else if (kind == FRAMECLOCK_CLASS_ID) {
        methods->tickStateDelay(self, sender, event);
    } else {
        kind = (u8)tag;
        if (kind == TEXTENTRY_CLASS_ID) {
            methods->onTextEntryResult(self, sender, event);
        } else if (kind == ITEMLIST_CLASS_ID) {
            methods->onItemListResult(self, sender, event);
        }
    }
}

/*
 * TaskObjF's state machine: slots +0x07C..+0x0B0 of gTaskObjFMethods and
 * the table getter, plus StampSaveTitleFileLetter, which writes a save
 * file's letter into its title.
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
extern char *sCardIconNames[TASKOBJF_STATE_EDIT_TITLE];
extern char sTitleCardPathPrefix[]; /* "CARD\\" */
extern char sCardPathSuffix[];      /* ".TIM" */
/* {0, 0, 160, 120} */
extern SpriteRect sCardIconRect;
/* (-70, -60), percent of half the screen from the centre */
extern ScreenSpritePos sCardIconPos;

void TaskObjF__LoadCardIcon(TaskObjF *self, s32 index) {
    char pathBuf[32];
    char *path;
    char *name;
    TimImage *tim;
    ScreenSprite *icon;

    /* MATCHING: `path` and `icon` keep the buffer and the sprite in saved
     * registers across the calls. */
    if (index >= ARRAY_COUNT(sCardIconNames)) {
        return;
    }
    if (self->spriteParent == 0) {
        return;
    }
    if (self->cardIcon != NULL) {
        return;
    }

    path = pathBuf;
    name = sCardIconNames[index];
    path[0] = '\0';
    strcat(path, sTitleCardPathPrefix);
    strcat(path, name);
    strcat(path, sCardPathSuffix);

    tim = New_TimImage(path);
    ((TimImageUploadFn)tim->methods->processBuffer)(tim);
    icon = New_ScreenSprite(tim, &sCardIconRect, 0);
    self->cardIcon = icon;
    tim->methods->release(tim);
    icon->methods->attachToParent(icon, self->spriteParent, (LongVec3 *)&sCardIconPos);
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
 * buffer sSaveTitle points at) starts as "LSD   Day001", all full-width:
 * "LSD" (0..2), the letter field (3..5), "Day" (6..8), the day number
 * (9..11), then padding. */
#define SAVE_TITLE_LETTER_FIELD 3
#define SAVE_TITLE_LETTER 4
/* SAVE_TITLE_PADDING (12) is defined above, with sSaveTitle. */
/* sSaveTitleGlyphs: the full-width letters a..o (0..14), one per save file
 * -01..-15, then three full-width spaces and "Day" (15..20). */
#define SAVE_TITLE_GLYPH_SPACES 15
/* A save file name is namePrefix ("BISLPS-01556", 12 characters) + "-NN";
 * the first digit of NN. */
#define SAVE_FILE_NAME_NUMBER 13

extern FullWidthChar *sSaveTitleGlyphs;

/* Writes a save file's letter into the full-width `title`: the letter
 * field becomes a space, the letter for the file name's -NN (a for -01 ..
 * o for -15) and a space, followed by "Day", and a space goes after the day
 * number. With no file name it only blanks the letter field. -08 and -09
 * are parsed from their second digit, which atoi would otherwise read as
 * octal. Returns a pointer into sSaveTitleGlyphs that no caller reads.
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

        title[SAVE_TITLE_PADDING] = sSaveTitleGlyphs[SAVE_TITLE_GLYPH_SPACES];
        *(FullWidthChars6 *)&title[SAVE_TITLE_LETTER_FIELD] =
            *(FullWidthChars6 *)&sSaveTitleGlyphs[SAVE_TITLE_GLYPH_SPACES];

        letter = atoi(fileName + numberPos) - 1;
        glyph = &sSaveTitleGlyphs[letter];
        title[SAVE_TITLE_LETTER] = *glyph;
        return (s32)glyph;
    } else {
        FullWidthChar *glyphs = sSaveTitleGlyphs;

        *(FullWidthChars3 *)&title[SAVE_TITLE_LETTER_FIELD] =
            *(FullWidthChars3 *)&glyphs[SAVE_TITLE_GLYPH_SPACES];
        return (s32)glyphs;
    }
}
