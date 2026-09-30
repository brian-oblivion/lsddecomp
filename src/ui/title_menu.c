/*
 * title_menu.c -- TitleMenu (include/title_menu.h), the TaskCore menu
 * between days, in ROM order: its allocator (allocate, then the ctor
 * through the class's table) and ctor, with UpdateFlashbackLock, which
 * locks or unlocks its FLASHBACK entry from the save block, and
 * StampSaveTitleDay, which writes the day into the save title
 * ("LSD   Day001"); then its methods, introduced below, to its getter. Its
 * method table and the menu's layout close the file; a (void *) entry in
 * the table is a method inherited from a parent class and declared on the
 * parent's type, or an empty method declared (void). TaskObjF, the
 * memory-card controller its SAVE and LOAD drive, follows in task_objf.c.
 */
#include "common.h"
#include <libgte.h>
#include <strings.h>
#include "dream_sys.h"
#include "title_menu.h"
#include "vab_stream_obj.h"
#include "text_row.h"
#include "tim_image.h"
#include "task_objf.h"
#include "bmem_pmgr.h"
#include "full_width_sjis.h"

/* TitleMenu's menu description, a TaskCoreTarget: TitleMenu__TitleMenu
 * passes &sTitleMenuTarget as TaskCore's ctor's `target` and again to
 * setTarget. Its six slots, START to SHAKE: */
#define TITLE_MENU_SLOT_COUNT (TITLEMENU_SHAKE + 1)
extern TaskCoreTarget sTitleMenuTarget;

/* "ETC\ETCSE", TitleMenu__TitleMenu's soundBankPath for TaskCore's ctor
 * (the ctor casts away the const for its `char *`). */
extern const char sTitleMenuSoundBankPath[];

/* "ETC\TITLE.TIM", TitleMenu__Reset's path for setSubHandle. */
extern const char sTitleTimPath[];

/* "CARD\FILEICN1.TIM", TitleMenu__BeginCardAccess's path for New_TimImage. */
extern const char sSaveIconTimPath[];

/* The two 320 x 240 display buffers, stacked in VRAM at y 0 and y 240:
 * TitleMenu__OnDeinit clears each with the DrawSystem's clearImage. */
extern DrawRect sDisplayBufferRects[2];

/* TitleMenu__AttachSaveTitle's position for the save title's attachToParent
 * (-4, -23: percent of half the screen from the centre). A TextRow's
 * position is a ScreenSpritePos (include/text_row.h), passed through
 * SceneNode's LongVec3 slot. */
extern struct ScreenSpritePos sSaveTitleOffset;

/*
 * The save file's name and title, as TaskObjF's beginSave/beginLoad take
 * them (`fileName`, `title`): TitleMenu__SaveToCard and
 * TitleMenu__LoadFromCard pass both. sSaveFileName points at
 * "BISLPS-01556xxx", which SaveToCard empties on a new game; sSaveTitle at
 * the full-width "LSD   Day001" followed by 19 full-width spaces, which
 * TitleMenu__CreateSaveTitle reblanks from its 12th character on a new game
 * and StampSaveTitleDay writes the day into.
 */
extern char *sSaveFileName;
extern char *sSaveTitle;

/* The buffer StampSaveTitleDay formats the day into
 * (FormatFullWidthNumber) before copying it into sSaveTitle's title; it
 * starts as the string "7654321". */
extern void *sDayDigits;

/* 19 full-width spaces, the tail TitleMenu__CreateSaveTitle copies over
 * sSaveTitle's on a new game (the ROM image points it just past
 * sSaveFileName's string). */
extern char *sSaveTitleBlanks;

/* TaskObjF's init `namePrefix` (TitleMenu__BeginCardAccess): the product
 * code "BISLPS-01556", a string of its own 16 bytes before sSaveFileName's. */
extern char *sCardFilePrefix;

/* TaskObjF's init `nameSuffixes` (TitleMenu__BeginCardAccess): the 15 file
 * suffixes "-01" to "-15", then NULL. */
extern char *sSaveFileSuffixes[];

/* TitleMenu__CycleSaveTitleColor's index (0, 1, 2) into its 3-byte colour,
 * read and written as a byte of a word. */
extern u8 sSaveTitleColorChannel;

/* TitleMenu__CycleSaveTitleColor's frame counter (wraps to 0 at 0x101). */
extern s32 sSaveTitleColorFrame;

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
    /* MATCHING: resetCounters is called with dreamSys too, which its (self) occupant ignores. */
    ((TitleMenuResetCallFn)self->methods->resetCounters)(self, dreamSys);
}

/* The save block's total flashback unlock score must be past this for the
 * menu to offer FLASHBACK. */
#define FLASHBACK_UNLOCK_SCORE 9999999

/* FLASHBACK stays locked unless the unlock score is past the threshold and at
 * least one flashback is stored. TitleMenu__RefreshMenu passes
 * self->dreamSys, which is unread. */
/* MATCHING: the unread dreamSys parameter; RefreshMenu's call loads it. */
void UpdateFlashbackLock(TitleMenu *self, TaskCoreTarget *target, struct DreamSys *dreamSys) {
    DreamSaveBlock *save = (DreamSaveBlock *)self->saveBlock;
    s32 locked = 1;

    if (save->totalFlashbackUnlockScore > FLASHBACK_UNLOCK_SCORE) {
        locked = (save->amountFlashbacksAvailable == 0);
    }
    /* A NULL entry is one the cursor can stop on. */
    target->hiddenSlots[TITLEMENU_FLASHBACK] = (void *)locked;
}

/* Formats the day as three full-width digits in sDayDigits's buffer and
 * copies them into the save title's day number, characters 9..11. */
void StampSaveTitleDay(s32 day) {
    FormatFullWidthNumber(sDayDigits, day, SAVE_TITLE_DAY_DIGITS, 0);
    /* MATCHING: FullWidthChar's byte members (alignment 1) set how the copy is done. */
    *(FullWidthChars3 *)&((FullWidthChar *)sSaveTitle)[SAVE_TITLE_DAY] = *(FullWidthChars3 *)sDayDigits;
}

/*
 * TitleMenu's methods (include/title_menu.h; its allocator and ctor are
 * above) and its getter.
 *
 * TitleMenu is the TaskCore menu between days: START, FLASHBACK, SAVE, LOAD,
 * GRAPH and SHAKE over ETC\TITLE.TIM. In ROM order here: finalize, onNotify,
 * reset, onDeinit, setState and confirmSlot (which acts on the chosen entry),
 * exit (stores SHAKE's setting), the four overrides that manage
 * the save-title TextRow in place of TaskCore's slot widgets, refreshMenu,
 * and the memory-card methods that drive `saveCtrl`, a TaskObjF, for SAVE
 * and LOAD. The header's class doc describes the class.
 *
 * The data they share is declared at the top of this file: sSaveTitle, the
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
    for (; i < ARRAY_COUNT(sDisplayBufferRects); i++) {
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
            self->result = TASKCORE_RESULT_DONE;
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

/* beginSave's titleEditPos: the player's text goes in from the character
 * after the padding's first space. */
#define SAVE_TITLE_EDIT_POS 13
/* beginSave's iconFrames: CARD\FILEICN1.TIM's three icon frames, the save
 * header's frame0..frame2. */
#define SAVE_ICON_FRAMES 3

/* The setTarget override: `target` is the TaskCoreTarget the ctor passes
 * (&sTitleMenuTarget); only its `handle` is read, as the TextRow's texture. On a
 * new game the title's padding is reblanked and its letter field cleared
 * first. The TextRow has a cell a character plus 4 and shows the file's
 * letter, "Day" and the day number (characters 4..11), with a gap before
 * the number. */
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
    self->saveTitle->visibleCount = SAVE_TITLE_PADDING - SAVE_TITLE_LETTER;
    self->saveTitle->firstVisible = SAVE_TITLE_LETTER;
    self->saveTitle->gapIndex = SAVE_TITLE_DAY;
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
 * of each cycle and the moving channel after. */
/* MATCHING: `channels` is taken before the first call, and `rgb = *color` is one
 * struct copy. */
void TitleMenu__CycleSaveTitleColor(TitleMenu *self, ColorRgb *color) {
    ColorRgb rgb;
    u8 *channels;

    channels = (u8 *)&rgb;
    GetTaskCoreMethods()->broadcastToSlots((TaskCore *)self, (u8 *)color);
    if (self->inputMode != TASKCORE_INPUT_NONE) {
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
    self->saveCtrl->methods->beginSave(self->saveCtrl, sSaveFileName, sSaveTitle,
                                       SAVE_TITLE_EDIT_POS, SAVE_ICON_FRAMES, self->saveIcon,
                                       self->saveBlock, self->saveBlockSize);
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

/* TitleMenu (include/title_menu.h): TaskCore's table with the title menu's
 * reset, state, confirm, exit and save-title overrides, then its menu and
 * memory-card slots. */
TitleMenuMethods gTitleMenuMethods = {
    /* +0x000 header */ TITLEMENU_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ TitleMenu__TitleMenu,
    /* +0x00C finalize */ TitleMenu__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)TitleMenu__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 resetCounters */ TitleMenu__Reset,
    /* +0x044 init */ (void *)TaskCore__Init,
    /* +0x048 deinit */ (void *)IntermediateBase__Deinit,
    /* +0x04C onInit */ (void *)TaskCore__OnInit,
    /* +0x050 onDeinit */ TitleMenu__OnDeinit,
    /* +0x054 onDrawSystemEvent */ (void *)IntermediateBase__OnDrawSystemEvent,
    /* +0x058 onPadEvent */ (void *)TaskCore__OnPadEvent,
    /* +0x05C update */ (void *)TaskCore__Update,
    /* +0x060 setState */ TitleMenu__SetState,
    /* +0x064 onStart */ (void *)IntermediateBase__OnStart,
    /* +0x068 onStop */ (void *)IntermediateBase__OnStop,
    /* +0x06C setFrameBound */ (void *)TaskCore__SetFrameBound,
    /* +0x070 playSound */ (void *)TaskCore__PlaySound,
    /* +0x074 onPadStart */ (void *)TaskCore__OnPadStart,
    /* +0x078 onPadConfirm */ (void *)TaskCore__OnPadConfirm,
    /* +0x07C onPadCancel */ (void *)TaskCore__OnPadCancel,
    /* +0x080 onPadPrev */ (void *)TaskCore__OnPadPrev,
    /* +0x084 onPadNext */ (void *)TaskCore__OnPadNext,
    /* +0x088 slot88 */ NULL,
    /* +0x08C slot8C */ NULL,
    /* +0x090 confirmSlot */ TitleMenu__ConfirmSlot,
    /* +0x094 exit */ TitleMenu__Exit,
    /* +0x098 setExitCallback */ (void *)TaskCore__SetExitCallback,
    /* +0x09C setFadeInCallbackEnabled */ (void *)TaskCore__SetFadeInCallbackEnabled,
    /* +0x0A0 setFadeOutCallbackEnabled */ (void *)TaskCore__SetFadeOutCallbackEnabled,
    /* +0x0A4 setColors */ (void *)TaskCore__SetColors,
    /* +0x0A8 setFadeRate */ (void *)TaskCore__SetFadeRate,
    /* +0x0AC tickFadeInCallback */ (void *)TaskCore__TickFadeInCallback,
    /* +0x0B0 tickFadeIn */ (void *)TaskCore__TickFadeIn,
    /* +0x0B4 slotB4 */ NULL,
    /* +0x0B8 slotB8 */ NULL,
    /* +0x0BC slotBC */ NULL,
    /* +0x0C0 tickFadeOutCallback */ (void *)TaskCore__TickFadeOutCallback,
    /* +0x0C4 tickFadeOut */ (void *)TaskCore__TickFadeOut,
    /* +0x0C8 slotC8 */ NULL,
    /* +0x0CC slotCC */ NULL,
    /* +0x0D0 slotD0 */ NULL,
    /* +0x0D4 setSubHandle */ (void *)TaskCore__SetSubHandle,
    /* +0x0D8 setTarget */ TitleMenu__CreateSaveTitle,
    /* +0x0DC releaseTarget */ TitleMenu__DestroySaveTitle,
    /* +0x0E0 updateSlotElements */ TitleMenu__AttachSaveTitle,
    /* +0x0E4 broadcastToSlots */ (void *)TitleMenu__CycleSaveTitleColor,
    /* +0x0E8 findNextFreeSlot */ (void *)TaskCore__FindNextFreeSlot,
    /* +0x0EC findPrevFreeSlot */ (void *)TaskCore__FindPrevFreeSlot,
    /* +0x0F0 setActiveSlot */ (void *)TaskCore__SetActiveSlot,
    /* +0x0F4 getActiveSlot */ (void *)TaskCore__GetActiveSlot,
    /* +0x0F8 createSlotElements */ (void *)TaskCore__CreateSlotElements,
    /* +0x0FC releaseSlotElements */ (void *)TaskCore__ReleaseSlotElements,
    /* +0x100 refreshSlotView */ (void *)TaskCore__RefreshSlotView,
    /* +0x104 broadcastToSlotElements */ (void *)TaskCore__BroadcastToSlotElements,
    /* +0x108 beginElementScroll */ (void *)TaskCore__BeginElementScroll,
    /* +0x10C commitElementScroll */ (void *)TaskCore__CommitElementScroll,
    /* +0x110 cancelElementScroll */ (void *)TaskCore__CancelElementScroll,
    /* +0x114 advanceSlotCursor */ (void *)TaskCore__AdvanceSlotCursor,
    /* +0x118 retreatSlotCursor */ (void *)TaskCore__RetreatSlotCursor,
    /* +0x11C setSlotCursor */ (void *)TaskCore__SetSlotCursor,
    /* +0x120 getActiveItemCursor */ (void *)TaskCore__GetActiveItemCursor,
    /* +0x124 refreshMenu */ (void *)TitleMenu__RefreshMenu,
    /* +0x128 beginCardAccess */ TitleMenu__BeginCardAccess,
    /* +0x12C endCardAccess */ TitleMenu__EndCardAccess,
    /* +0x130 saveToCard */ TitleMenu__SaveToCard,
    /* +0x134 loadFromCard */ TitleMenu__LoadFromCard,
    /* +0x138 onCardEvent */ TitleMenu__OnCardEvent,
};

/* The title menu's strings. They are the image's read-only data and small
 * data, declared plain char because the tables' fields are char *. */
extern char sTitleMenuStartName[];      /* "START" */
extern char sTitleMenuFlashbackName[];  /* "FLASHBACK" */
extern char sTitleMenuSaveName[];       /* "SAVE" */
extern char sTitleMenuLoadName[];       /* "LOAD" */
extern char sTitleMenuGraphName[];      /* "GRAPH" */
extern char sTitleMenuShakeName[];      /* "SHAKE" */
extern char sShakeOffName[];            /* "Off" */
extern char sShakeOnName[];             /* "On" */
extern const char sTitleMenuFontPath[]; /* "ETC\FONTICON.TIM" */

/* SHAKE's two settings, the item list its slot opens. */
char *sShakeItemNames[] = {sShakeOffName, sShakeOnName, NULL};

/* SHAKE's item list: the cursor starts on Off, drawn in yellow, at (53, 57). */
TaskCoreItemList sShakeItemList = {{0}, 0, {128, 128, 0}, {0}, {53, 57}, sShakeItemNames};

/* The slots' tables, per slot START, FLASHBACK, SAVE, LOAD, GRAPH, SHAKE. */
/* clang-format off */

/* The item list each slot opens: only SHAKE has one. */
TaskCoreItemList *sTitleMenuSlotLists[TITLE_MENU_SLOT_COUNT] = {
    NULL, NULL, NULL, NULL, NULL, &sShakeItemList,
};

/* Non-NULL for the slots the cursor skips: FLASHBACK, locked until
 * UpdateFlashbackLock unlocks it from the save. One entry more than there
 * are slots. */
void *sTitleMenuHiddenSlots[TITLE_MENU_SLOT_COUNT + 1] = {
    NULL, (void *)1, NULL, NULL, NULL, NULL, NULL,
};

/* The menu, top to bottom, NULL-terminated. */
char *sTitleMenuSlotNames[TITLE_MENU_SLOT_COUNT + 1] = {
    sTitleMenuStartName,
    sTitleMenuFlashbackName,
    sTitleMenuSaveName,
    sTitleMenuLoadName,
    sTitleMenuGraphName,
    sTitleMenuShakeName,
    NULL,
};

/* Each slot's position, percent of half the screen from the centre: one
 * column, 12 apart. */
ScreenSpritePos sTitleMenuSlotPositions[TITLE_MENU_SLOT_COUNT] = {
    {8, -3}, {8, 9}, {8, 21}, {8, 33}, {8, 45}, {8, 57},
};
/* clang-format on */

/* The title menu: ETC\FONTICON.TIM's font, START selected first, grey
 * slots with the selected one yellow. */
TaskCoreTarget sTitleMenuTarget = {
    sTitleMenuFontPath,
    NULL,
    0,
    0,
    {80, 80, 80},
    {128, 128, 0},
    {0},
    sTitleMenuHiddenSlots,
    sTitleMenuSlotNames,
    sTitleMenuSlotPositions,
    sTitleMenuSlotLists,
};

/* The save files' suffixes, "-01" to "-15". */
extern char sSaveFileSuffix01[]; /* "-01" */
extern char sSaveFileSuffix02[]; /* "-02" */
extern char sSaveFileSuffix03[]; /* "-03" */
extern char sSaveFileSuffix04[]; /* "-04" */
extern char sSaveFileSuffix05[]; /* "-05" */
extern char sSaveFileSuffix06[]; /* "-06" */
extern char sSaveFileSuffix07[]; /* "-07" */
extern char sSaveFileSuffix08[]; /* "-08" */
extern char sSaveFileSuffix09[]; /* "-09" */
extern char sSaveFileSuffix10[]; /* "-10" */
extern char sSaveFileSuffix11[]; /* "-11" */
extern char sSaveFileSuffix12[]; /* "-12" */
extern char sSaveFileSuffix13[]; /* "-13" */
extern char sSaveFileSuffix14[]; /* "-14" */
extern char sSaveFileSuffix15[]; /* "-15" */

/* clang-format off */
char *sSaveFileSuffixes[] = {
    sSaveFileSuffix01,
    sSaveFileSuffix02,
    sSaveFileSuffix03,
    sSaveFileSuffix04,
    sSaveFileSuffix05,
    sSaveFileSuffix06,
    sSaveFileSuffix07,
    sSaveFileSuffix08,
    sSaveFileSuffix09,
    sSaveFileSuffix10,
    sSaveFileSuffix11,
    sSaveFileSuffix12,
    sSaveFileSuffix13,
    sSaveFileSuffix14,
    sSaveFileSuffix15,
    NULL,
};
/* clang-format on */

DrawRect sDisplayBufferRects[2] = {{0, 0, 320, 240}, {0, 240, 320, 240}};
