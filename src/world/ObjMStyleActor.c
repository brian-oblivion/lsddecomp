/*
 * ObjMStyleActor -- the day's scene object and what it runs, with the classes
 * that sit between them in ROM. In address order, each under its own section
 * banner below:
 *  - ItemList's second half (include/ItemList.h; the first half is in
 *    TextEntryItemList.c): setState and tickClosing (close, then report the
 *    result to the parents), handleInputCode (the Pad events it answers) and
 *    playSound, the cursor and scroll methods, the four visible rows
 *    (createRows, releaseRows, refreshRows, and the non-virtual helpers
 *    FormatRowText and SetView), stepCursorInView, getCursorIndex and the
 *    table getter GetItemListMethods;
 *  - ObjM (include/ObjM.h), whole: the TimedTask DayTask starts for a day's
 *    scene;
 *  - the style layer, whose client ObjM is: its setup (RegisterStyleConfig
 *    to ApplyStyleDecorationIfSet), its per-scene objects (StyleFlushDecoration
 *    to StyleScrollVramStrips) and its sound cues (StyleCue00..13);
 *  - StyleEffect (include/StyleEffect.h), whole, the Actor the style layer
 *    keeps at an offset from its target, then SetStyleEffectSources;
 *  - Actor (include/Actor.h), whole, the base of TodActor, DreamSys and
 *    StyleEffect;
 *  - VariantSprite (include/VariantSprite.h), whole;
 *  - GraphRoom (include/GraphRoom.h), whole, the mood graph screen.
 *
 * The game's own files most likely ended after each class's table getter;
 * this file keeps the classes together.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "DayTaskStageMap.h"
#include "TimedTask.h"
#include "TextRow.h"
#include "TimImage.h"
#include "ItemList.h"
#include "ObjM.h"
#include "VabStreamObj.h"
#include "Pad.h"
#include "FadeBox.h"
#include "DreamSys.h"
#include "StageMap.h"
#include "NodeGuardedViewport.h"
#include "TimBlockSrc.h"
#include "WBgm.h"
#include "LbdFile.h"
#include "BoxFill.h"
#include "FrameClock.h"
#include "Actor.h"
#include "StyleEffect.h"
#include "Viewport.h"
#include "SoundCueSet.h"
#include "VariantSprite.h"
#include <rand.h>
#include "TmdModel.h"
#include "GridCell.h"
#include "GraphRoom.h"

void ItemList__SetState(ItemList *self, s32 state) {
    /* MATCHING: the gotos keep retail's branch polarity and block order. */
    self->closeTicks = 0;
    if (state < ITEMLIST_RESULT_CHOSEN) {
        goto end;
    }
    if (state < ITEMLIST_STATE_REPORT) {
        goto case_lt4;
    }
    if (state == ITEMLIST_STATE_REPORT) {
        goto case_eq4;
    }
    goto end;
case_lt4:
    self->methods->removeChild(self, self->inputSource);
    self->methods->releaseResources(self);
    self->result = state;
    goto end;
case_eq4:
    self->methods->notifyParents(self, self->result);
end:
    return;
}

void ItemList__TickClosing(ItemList *self) {
    if (self->result >= ITEMLIST_STATE_REPORT) {
        return;
    }
    if (self->result < ITEMLIST_RESULT_CHOSEN) {
        return;
    }
    if (self->closeTicks++ == 0) {
        return;
    }
    self->methods->setState(self, ITEMLIST_STATE_REPORT);
}

void ItemList__HandleInputCode(ItemList *self, void *source, s32 code) {
    /* MATCHING: the cases stay in this order; retail lays their bodies out in it. */
    switch (code) {
        case PAD_EVENT_PRESSED + PAD_BUTTON_RRIGHT:
            self->methods->playSound(self, 1 << 4); /* VAB program 1, tone 0 */
            self->methods->setState(self, ITEMLIST_RESULT_CHOSEN);
            break;
        case PAD_EVENT_PRESSED + PAD_BUTTON_RDOWN:
            self->methods->playSound(self, 1 << 4); /* VAB program 1, tone 0 */
            self->methods->setState(self, ITEMLIST_RESULT_CANCELLED);
            break;
        case PAD_EVENT_HELD + PAD_BUTTON_LRIGHT:
            self->methods->scrollRight(self);
            break;
        case PAD_EVENT_HELD + PAD_BUTTON_LLEFT:
            self->methods->scrollLeft(self);
            break;
        case PAD_EVENT_PRESSED + PAD_BUTTON_LUP:
            self->methods->cursorUp(self);
            break;
        case PAD_EVENT_PRESSED + PAD_BUTTON_LDOWN:
            self->methods->cursorDown(self);
            break;
    }
}

void ItemList__PlaySound(ItemList *self, s32 tone) {
    struct VabStreamObj *target = self->target;

    if (target != NULL) {
        target->methods->playTone(target, tone, 96, 96);
    }
}

void ItemList__ScrollRight(ItemList *self) {
    ItemListMethods *methods;
    s32 current;
    s32 column;

    if (!self->panelSprite) {
        return;
    }
    current = self->column; /* MATCHING: the double read keeps retail's registers */
    column = current;
    if (column + ITEMLIST_ROW_CHARS >= self->maxTextLen) {
        return;
    }
    methods = self->methods;
    column++;
    self->column = column;
    methods->refreshRows(self, self->topIndex, column, self->cursorIndex, 1);
}

void ItemList__ScrollLeft(ItemList *self) {
    s32 column;

    if (!self->panelSprite) {
        return;
    }
    column = self->column - 1;
    if (column < 0) {
        return;
    }
    self->column = column;
    self->methods->refreshRows(self, self->topIndex, column, self->cursorIndex, 1);
}

void ItemList__CursorUp(ItemList *self, s32 unused1, s32 unused2, s32 forwarded) {
    s32 cursor;
    s32 newTop;
    s32 newCursor;

    if (!self->panelSprite) {
        return;
    }
    cursor = self->cursorIndex;
    if (cursor - 1 < 0) {
        return;
    }
    /* MATCHING: this polarity, and newTop/newCursor, keep retail's block order and registers. */
    if (cursor - self->topIndex > 0) {
        self->methods->stepCursorInView(self, 0, 1, forwarded);
    } else {
        self->topIndex--;
        newTop = self->topIndex;
        self->cursorIndex--;
        newCursor = self->cursorIndex;
        self->methods->refreshRows(self, newTop, self->column, newCursor, 1);
    }
}

void ItemList__CursorDown(ItemList *self, s32 unused1, s32 unused2, s32 forwarded) {
    s32 newTop;
    s32 newCursor;
    s32 prevTop;

    if (!self->panelSprite) {
        return;
    }
    if (self->cursorIndex + 1 >= self->itemCount) {
        return;
    }
    prevTop = self->topIndex - 1; /* MATCHING: its own statement, or cc1 folds the -1 */
    if (self->cursorIndex - prevTop < ARRAY_COUNT(self->rows)) {
        self->methods->stepCursorInView(self, 1, 1, forwarded);
    } else {
        self->topIndex++;
        newTop = self->topIndex; /* MATCHING: newTop/newCursor keep retail's registers */
        self->cursorIndex++;
        newCursor = self->cursorIndex;
        self->methods->refreshRows(self, newTop, self->column, newCursor, 1);
    }
}

/* The first row's position, two sdata words (-92, -15). Read by value into
 * ItemList__CreateRows's `pos`; each further row is ITEMLIST_ROW_SPACING
 * lower. */
extern s32 sItemListRowOriginX;
extern s32 sItemListRowOriginY;

/* The y step from one row to the next (createRows). */
#define ITEMLIST_ROW_SPACING 10

void ItemList__CreateRows(ItemList *self, SceneNode *parent, TimImage *font, s32 top, s32 column,
                          s32 cursor) {
    char buf[32]; /* MATCHING: declared first, or cc1 keeps its address in a register */
    ScreenSpritePos pos;
    TextRow **row;
    s32 count;
    s32 i;

    if (!self->panelSprite) {
        return;
    }

    pos.x = sItemListRowOriginX;
    pos.y = sItemListRowOriginY;
    count = self->itemCount;
    row = &self->rows[0];
    if (count > ARRAY_COUNT(self->rows)) {
        count = ARRAY_COUNT(self->rows);
    }

    for (i = 0; i < count; i++) {
        ItemList__FormatRowText(self, buf, i, top, column);
        *row = New_TextRow(font, ITEMLIST_ROW_CHARS, buf);
        (*row)->methods->attachToParent(*row, parent, (LongVec3 *)&pos);
        (*row)->methods->setColor(*row, &sItemListRowColor);
        pos.y += ITEMLIST_ROW_SPACING;
        row++;
    }

    ItemList__SetView(self, top, column, cursor, 1);
}

void ItemList__ReleaseRows(ItemList *self) {
    s32 count;
    s32 i;
    u8 unused[8]; /* MATCHING: retail's 0x28-byte frame */

    if (!self->panelSprite) {
        return;
    }
    count = self->itemCount;
    i = 0; /* MATCHING: here and a do/while, as retail tests count once */
    if (count > ARRAY_COUNT(self->rows)) {
        count = ARRAY_COUNT(self->rows);
    }
    if (count <= 0) {
        return;
    }
    do {
        self->rows[i]->methods->release(self->rows[i]);
        self->rows[i] = NULL;
        i++;
    } while (i < count);
}

/* Psy-Q's libc2 strlen and memcpy, linked from Sony's objects, typed as
 * ItemList__FormatRowText passes them (memcpy's `void *` as <memory.h>). */
extern s32 strlen(char *s);
extern void *memcpy(char *dest, char *src, s32 n);

void ItemList__RefreshRows(ItemList *self, s32 top, s32 column, s32 cursor, s32 notify) {
    s32 count;
    s32 i;
    char buf[32]; /* MATCHING: retail's frame size */
    TextRow **row;

    if (!self->panelSprite) {
        return;
    }
    count = self->itemCount;
    row = &self->rows[0]; /* MATCHING: before the clamp, in its delay slot */
    if (count > ARRAY_COUNT(self->rows)) {
        count = ARRAY_COUNT(self->rows);
    }
    for (i = 0; i < count; i++) {
        ItemList__FormatRowText(self, buf, i, top, column);
        (*row)->methods->setText(*row, buf);
        row++;
    }
    ItemList__SetView(self, top, column, cursor, 0);
    if (notify) {
        self->methods->playSound(self, 0);
    }
}

char *ItemList__FormatRowText(ItemList *self, char *dest, s32 row, s32 top, s32 column) {
    s32 item = top + row; /* MATCHING: this operand order */
    s32 len;
    s32 i;

    len = strlen(self->texts[item] + column);
    if (len > ITEMLIST_ROW_CHARS) {
        len = ITEMLIST_ROW_CHARS;
    }
    memcpy(dest, self->texts[item] + column, len);
    for (i = len; i < ITEMLIST_ROW_CHARS; i++) {
        dest[i] = ' ';
    }
    dest[ITEMLIST_ROW_CHARS] = '\0';
    return dest;
}

void ItemList__SetView(ItemList *self, s32 top, s32 column, s32 cursor, s32 highlight) {
    TextRow *row;

    self->topIndex = top;
    self->column = column;
    self->cursorIndex = cursor;
    if (highlight == 0) {
        return;
    }
    cursor -= top; /* MATCHING: reuses cursor's register for the index */
    row = self->rows[cursor];
    row->methods->setColor(row, &sItemListCursorColor);
}

void ItemList__StepCursorInView(ItemList *self, s32 dir, s32 notify) {
    TextRow **row;
    s32 idx;

    if (!self->panelSprite) {
        return;
    }
    idx = self->cursorIndex - self->topIndex;
    row = &self->rows[idx]; /* MATCHING: one address, stepped, as retail */
    (*row)->methods->setColor(*row, &sItemListRowColor);
    if (dir) {
        self->cursorIndex++;
        row++;
    } else {
        self->cursorIndex--;
        row--;
    }
    (*row)->methods->setColor(*row, &sItemListCursorColor);
    if (notify) {
        self->methods->playSound(self, 0);
    }
}

s32 ItemList__GetCursorIndex(ItemList *self) {
    return self->cursorIndex;
}

ItemListMethods *GetItemListMethods(void) {
    return &gItemListMethods;
}

/* ---- ObjM (include/ObjM.h) ---------------------------------------------
 *
 * Its allocator, ctor, finalize and onNotify, which dispatches on the
 * sender's class id; then, in the next two sections, its table's slots in
 * order.
 */

ObjM *New_ObjM(BasicClass *sound, struct WBgm *bgm, TimImage *etcTim,
               struct LinkResource *dreamerTmd, s32 stage) {
    ObjM *self;
    ObjMMethods *methods;

    self = BMemPMgrAlloc(sizeof(ObjM));
    if (self != NULL) {
        methods = GetObjMMethods();
        methods->ctor(self, sound, bgm, etcTim, dreamerTmd, stage);
        return self; /* MATCHING: two returns, not one */
    }
    return NULL;
}

void ObjM__ObjM(ObjM *self, BasicClass *sound, struct WBgm *bgm, TimImage *etcTim,
                struct LinkResource *dreamerTmd, s32 stage) {
    GetTimedTaskMethods()->ctor((TimedTask *)self, 0, sound);
    self->methods = GetObjMMethods();
    self->unk64 = 0;
    self->inSession = 0;
    self->timBlockPending = 1;
    self->bgm = bgm;
    self->stage = stage;
    self->ctorSound = sound;
    self->etcTim = etcTim;
    self->dreamerTmd = dreamerTmd;
    self->pauseSetupStep = 0;
    self->closeReady = 0;
    self->methods->resetCounters(self);
}

void ObjM__Finalize(ObjM *self) {
    GetTimedTaskMethods()->finalize((TimedTask *)self);
}

void ObjM__OnNotify(ObjM *self, BasicClass *sender, s32 event) {
    s32 tag;

    GetTimedTaskMethods()->onNotify((TimedTask *)self, sender, event);
    tag = sender->methods->header;
    if ((tag & 0xFFF) == STAGEMAP_CLASS_ID) {
        self->methods->onStageMapNotify(self, sender, event);
    } else if ((tag & 0xFFF) == FADEBOX_CLASS_ID) {
        self->methods->onFadeNotify(self, (struct FadeBox *)sender, event);
    } else if ((tag & 0xFFFF) == DREAMSYS_CLASS_ID) {
        self->methods->onDreamSysNotify(self, sender, event);
    }
}

/* ---- ObjM, resetCounters (+0x040) to enterLinkWall (+0x09C) ---------------
 *
 * In table order.
 *
 *  - init and deinit (AttachTarget, DetachTarget): install
 *    ObjM__GetGridRecord as the StageMap's chunk-record callback and keep
 *    the DreamSys as a child.
 *  - onInit and onDeinit (InitStyleAndWorld, TeardownStyle): pick the
 *    stage's BGM sequence and the day's TIM block, register the stage's
 *    StyleConfig, set the viewport's view and the StageMap's bounds; stop
 *    it all again.
 *  - onDrawSystemEvent's event 2 runs PollTimBlockLoad: once the TIM block has
 *    loaded (or failed) the scene is set up, and once the StageMap has
 *    nothing pending the style session starts.
 *  - onPadEvent (DispatchPadEvent) maps Start, Select and triangle onto the
 *    pause and close slots; update ticks the style, or the pause overlay
 *    while it is being built; togglePause.
 *  - the style scene slots +0x080..+0x08C: SetupSceneStyle,
 *    ExitSceneStyle, EnterStyleSession, TickStyle.
 *  - OnDreamSysNotify turns the DreamSys's link codes into the enter*
 *    slots and notifyLinkTeleport; EnterTimeUp, EnterLinkDynamic and
 *    EnterLinkWall set IntermediateBase::state and start a fade up
 *    (ObjM__StartFadeUp, next section).
 * NoOpSlot40 and NoOpSlot7C are empty.
 */

void ObjM__NoOpSlot40(void) {}

/* init. `args` is the building DayTask's init args: args->lightRig is its
 * StageMap (IntermediateBase__Init keeps it as lightRig), whose callback
 * becomes ObjM__GetGridRecord. */
void ObjM__AttachTarget(ObjM *self, IntermediateBaseInitArgs *args, DreamSys *dreamSys) {
    ((StageMap *)args->lightRig)
        ->methods->setCallback((StageMap *)args->lightRig, (ChunkFileFn)ObjM__GetGridRecord, self);
    self->dreamSys = dreamSys;
    GetTimedTaskMethods()->init((TimedTask *)self, args, 1);
    self->methods->addChild(self, (BasicClass *)dreamSys);
}

/* ObjM__GetGridRecord's grid lookups (src/cd/GameFiles.c): a
 * non-negative code is a linear cell index (GetStageMapChunkRecord(index, code)),
 * a negative one sends x/y to GetStageMapChunkRecordXY. */
extern s32 GetStageMapChunkRecord(s32 index, s32 sub);
extern void GetStageMapChunkRecordXY(s32 index, s32 x, s32 y);

/* The StageMap's chunkFileFn: a chunk's file record, by linear cell index,
 * or by x/y when the index is negative. The record is left as the return
 * value for the StageMap (GetStageMapChunkRecordXY is declared void). */
void ObjM__GetGridRecord(ObjM *self, s32 cell, s32 x, s32 y) {
    if (cell >= 0) {
        GetStageMapChunkRecord(self->stage, cell);
    } else {
        GetStageMapChunkRecordXY(self->stage, x, y);
    }
}

void ObjM__DetachTarget(ObjM *self) {
    self->methods->removeChild(self, (BasicClass *)self->dreamSys);
    GetTimedTaskMethods()->deinit((TimedTask *)self);
}

/* Defined elsewhere, no header: src/cd/GameFiles.c (PickStageBgm and
 * PickStageTexture return a FilePathRecord *, a 0x1C-byte record handed on here as a
 * name), src/graphics/SceneNode.c (GetSetHitHeightGate sets the flag
 * SceneNode__RaycastHullAgainstFaces tests). RegisterStyleConfig, which
 * keeps `sceneRefs` as gStyleSceneRefs, is defined below, after ObjM. */
extern s32 PickStageBgm(s32 stage, s32 unused);
extern s32 PickStageTexture(s32 stage, s32 unused, s32 day);
extern s32 GetSetHitHeightGate(s32 value);
extern s32 RegisterStyleConfig(s32 grid, s32 stage, s32 sceneRefs, s32 day, s32 unreadArg);

/* The viewport's view point and reference point (attachViewChild), the
 * StageMap's bounds on stage 0, and one DreamSys setPendingExtra value per
 * stage. */
extern LongVec3 sObjMViewPoint;
extern LongVec3 sObjMViewRefPoint;
extern s32 sStagePendingExtras[];
extern CellBounds sStage0Bounds;

/* onInit's gridSpan when it is passed 0: sDefaultGridSpan's value, the one
 * the StageMap starts with (10 half-cells: StageMap::gridHalfCells is
 * gridSpan >> 12). */
#define DEFAULT_GRID_SPAN 40960

/* onInit (IntermediateBase__Init passes 0, 0, 0). */
void ObjM__InitStyleAndWorld(ObjM *self, s32 gridSpan, StyleConfig *style, s32 arg3) {
    NodeGuardedViewport *vp = (NodeGuardedViewport *)self->viewport;
    s32 record;
    s32 day;
    s32 flag;

    vp->methods->detachViewChild(vp);
    self->timBlockPending = 1;
    record = PickStageBgm(self->stage, 0);
    self->bgm->methods->setSeq(self->bgm, (char *)record);

    day = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
    record = PickStageTexture(self->stage, 0, day);
    self->timBlockSrc = (TimBlockSrc *)New_TimBlockSrc(record);

    vp->methods->attachViewChild(vp, (BasicClass *)self->dreamSys, &sObjMViewPoint,
                                 &sObjMViewRefPoint, 0);

    self->cachedViewport = vp;
    day = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
    self->styleConfig = (StyleConfig *)RegisterStyleConfig((s32)self->lightRig, self->stage,
                                                           (s32)&self->ctorSound, day, 0);
    if (style != 0) {
        self->styleConfig = style;
    }

    self->unk4C = arg3;
    if (self->stage != 0) {
        s32 stage;
        s32 three;

        /* MATCHING: the volatile read keeps retail's second load of self->stage. */
        stage = *(s32 volatile *)&self->stage;
        self->tickPeriod = 16;
        three = 3;
        /* MATCHING: without the barrier `three`'s li moves past the store above. */
        __asm__("");
        flag = (stage == 5);
        if (stage == 6) {
            flag = 1;
        }
        self->moveMode = three;
        if (stage == three) {
            flag = 1;
        }
        ((StageMap *)self->lightRig)->methods->setBounds((StageMap *)self->lightRig, 0);
    } else {
        self->tickPeriod = 16;
        self->moveMode = 2;
        flag = 1;
        ((StageMap *)self->lightRig)->methods->setBounds((StageMap *)self->lightRig, &sStage0Bounds);
    }

    self->gridSpan = gridSpan;
    if (gridSpan == 0) {
        self->gridSpan = DEFAULT_GRID_SPAN;
    }
    GetSetHitHeightGate(flag);

    self->dreamSys->methods->setPendingExtra(self->dreamSys, sStagePendingExtras[self->stage]);
    self->state = 5;
}

/* ObjM__TeardownStyle's helpers (src/world/DreamAux.c, src/world/ObjMStyleActor.c). */
extern void ReleaseDreamAuxEntities(void);
extern void StyleTeardown(void);

/* onDeinit. */
void ObjM__TeardownStyle(ObjM *self) {
    self->methods->exitSceneStyle(self);
    ReleaseDreamAuxEntities();
    StyleTeardown();
    self->bgm->methods->stop(self->bgm);
}

void ObjM__OnDrawSystemEvent(ObjM *self, void *sender, s32 event) {
    if (event == DRAWSYSTEM_EVENT_VSYNC) {
        ObjM__PollTimBlockLoad(self, self->timBlockSrc);
    }
}

/* `src` is always self->timBlockSrc. Loaded, its CLUT rows fade to a
 * styleConfig colour; failed or loaded, it is released and the scene set
 * up (a failure also adds 30 seconds to the DreamSys's time limit). With
 * nothing pending and the StageMap idle, the style session starts. */
void ObjM__PollTimBlockLoad(ObjM *self, TimBlockSrc *src) {
    s32 timer;
    s32 colorMode;
    TimBlockSrcColor *color;
    TimBlockSrcMethods *m;

    if (self->timBlockPending != 0) {
        if (src->failed != 0) {
            src->methods->release(src);
            self->timBlockPending = 0;
            self->methods->setupSceneStyle(self);
            timer = self->dreamSys->methods->getDreamTimerScaled(self->dreamSys);
            self->dreamSys->methods->getSetDreamTimeLimit(self->dreamSys, timer + 30);
        } else if (src->loaded != 0) {
            colorMode = self->styleConfig->colorMode;
            m = src->methods;
            if (colorMode != 2) {
                color = self->styleConfig->farColor;
            } else {
                color = self->styleConfig->clearColor;
            }
            m->fadeAllEntries(src, color);
            src->methods->release(src);
            self->timBlockPending = 0;
            self->methods->setupSceneStyle(self);
        }
    }
    if (self->timBlockPending == 0) {
        if (((StageMap *)self->lightRig)->pendingLoadCount == 0 && self->inSession == 0) {
            self->unk64 = 1;
            self->methods->enterStyleSession(self);
        }
    }
}

/* onPadEvent, only in session: Start pressed toggles the pause, Select
 * held and released sets and clears the close-ready flag, triangle pressed
 * closes (closeAndNotifyNewGame).
 * MATCHING: the gotos keep retail's compare order; a switch sorts the cases. */
void ObjM__DispatchPadEvent(ObjM *self, void *sender, s32 code) {
    ObjMMethods *m = self->methods;
    void (*fn)(ObjM *);

    if (self->inSession == 0) {
        return;
    }
    if (code == PAD_EVENT_PRESSED + PAD_BUTTON_RUP) {
        goto closeAndNotify;
    }
    if (code <= PAD_EVENT_PRESSED + PAD_BUTTON_RUP) {
        if (code == PAD_EVENT_HELD + PAD_BUTTON_SELECT) {
            goto updateCloseReady;
        }
        return;
    }
    if (code == PAD_EVENT_PRESSED + PAD_BUTTON_START) {
        goto togglePause;
    }
    if (code == PAD_EVENT_RELEASED + PAD_BUTTON_SELECT) {
        goto clearCloseReady;
    }
    return;
togglePause:
    fn = m->togglePause;
    goto call;
updateCloseReady:
    fn = m->updateCloseReadyFlag;
    goto call;
closeAndNotify:
    fn = m->closeAndNotifyNewGame;
    goto call;
clearCloseReady:
    fn = m->clearCloseReadyFlag;
call:
    fn(self);
}

/* update, the FrameClock case of onNotify. */
void ObjM__Update(ObjM *self) {
    void (*fn)(ObjM *);

    if (self->inSession != 0) {
        self->frameCounter++;
        if (self->pauseSetupStep != 0) {
            fn = self->methods->advancePauseSetup;
        } else {
            fn = self->methods->tickStyle;
        }
        fn(self);
    }
}

/* togglePause. */
void ObjM__TogglePause(ObjM *self) {
    ObjMMethods *m = self->methods;

    if (self->pauseSetupStep != 0) {
        m->clearCloseReadyFlag(self);
        m->teardownPauseOverlay(self);
    } else {
        m->advancePauseSetup(self);
    }
}

void ObjM__NoOpSlot7C(void) {}

/* src/world/DreamAux.c's; no header declares it. It keeps the stage, the
 * StageMap, the DreamSys (sDreamAuxWorld), the sound and the FrameClock for
 * the dream's aux entities. */
struct FrameClock;
extern void SetDreamAuxWorld(s32 stage, StageMap *stageMap, DreamSys *world,
                             struct VabStreamObj *sound, struct FrameClock *frameClock);

/* Added to the viewport's projection distance; 0 in the image and never
 * written. */
extern s32 sObjMProjectionBias;

/* The StageMap's accepted tags (setAcceptedTags): the class ids of DreamSys
 * (0x1F34) and Entity (0x1F234), 0-terminated. */
extern s32 sObjMAcceptedClassIds[];

void ObjM__SetupSceneStyle(ObjM *self) {
    NodeGuardedViewport *vp = (NodeGuardedViewport *)self->viewport;
    StyleConfig *style = self->styleConfig;
    DrawSystem *drawSystem;
    s32 width;
    StageMap *rig;

    vp->methods->detachViewChild(vp);

    drawSystem = (DrawSystem *)self->initArgs->drawSystem;
    width = drawSystem->methods->getDims(drawSystem, NULL)->w;
    vp->methods->setProjection(vp, width / 2 * 5 / 3 + sObjMProjectionBias);

    vp->methods->attachViewChild(vp, (BasicClass *)self->dreamSys, &sObjMViewPoint,
                                 &sObjMViewRefPoint, 0);

    SetDreamAuxWorld(self->stage, (StageMap *)self->lightRig, self->dreamSys,
                     (struct VabStreamObj *)self->sound, (struct FrameClock *)self->frameClock);

    rig = (StageMap *)self->lightRig;
    self->methods->addChild(self, (BasicClass *)rig);

    rig->methods->setAmbientColor(rig, (LightRigRgb *)style->ambientColor, 0);
    rig->methods->setChildParams(rig, 3, style->lightDirs, style->lightColors);
    rig->methods->setConfig(rig, GetStageGridDimensions(self->stage));
    ((DreamSysAttachToParentFn)self->dreamSys->methods->attachToParent)(self->dreamSys, rig);
    rig->methods->setGridSpan(rig, self->gridSpan);
    rig->methods->setAcceptedTags(rig, sObjMAcceptedClassIds);
}

void ObjM__ExitSceneStyle(ObjM *self) {
    self->methods->teardownPauseOverlay(self);
    self->dreamSys->methods->blockMovement(self->dreamSys);
    self->dreamSys->methods->detachFromParent(self->dreamSys);
    ((NodeGuardedViewport *)self->viewport)->methods->detachViewChild((NodeGuardedViewport *)self->viewport);
    self->methods->removeChild(self, self->lightRig);
}

void ObjM__EnterStyleSession(ObjM *self) {
    NodeGuardedViewportMethods *m;
    FadeBoxMethods *m2;
    NodeGuardedViewport *vp;
    FadeBox *fade;
    StyleConfig *style;
    DreamColors flashColor;
    s32 flashback;
    s32 channels;
    void *farColor;

    self->inSession = 1;
    self->dreamSys->methods->resetLinkState(self->dreamSys, self->moveMode, self->tickPeriod);
    ((StageMap *)self->lightRig)->methods->enable((StageMap *)self->lightRig);

    vp = (NodeGuardedViewport *)self->viewport;
    style = self->styleConfig;
    vp->methods->setLightMode(vp, 1); /* GsFOG */
    vp->methods->setClearColor(vp, style->clearColor);
    vp->methods->setFogNear(vp, style->fogNear);
    m = vp->methods;
    if (style->colorMode != 1) {
        farColor = style->farColor;
    } else {
        farColor = style->clearColor;
    }
    m->setFarColor(vp, farColor);
    vp->methods->setExtraSwap(vp, 0);
    vp->methods->setDrawEnabled(vp, 1);

    fade = (FadeBox *)vp->methods->getFadeBox(vp);
    self->methods->addChild(self, (BasicClass *)fade);

    flashback = self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, &flashColor, -1);
    fade->methods->setDivisorMode(fade, flashback, (flashback != 0) ? 3 : 0);
    m2 = fade->methods;
    if (flashback == 0) {
        channels = -1;
    } else {
        channels = flashColor;
    }
    m2->startFadeDown(fade, self->frameClock, channels, 0);
}

/* Defined below, in the style layer. */
extern s32 TickStyle(Descriptor10 *cell, void *unused, s32 lastCue);

void ObjM__TickStyle(ObjM *self) {
    TickStyle(((StageMap *)self->lightRig)->methods->getTargetDescriptor((StageMap *)self->lightRig, 0, 0),
              0, 0);
}

/* While ObjM's state is 0 each DreamSys link code runs its enterState slot
 * (DREAMSYS_LINK_DAY_START none); otherwise any code from 9 up clears the
 * DreamSys's own state. */
void ObjM__OnDreamSysNotify(ObjM *self, BasicClass *sender, s32 code) {
    if (self->state == 0) {
        switch (code) {
            case DREAMSYS_TIME_UP:
                self->methods->enterTimeUp(self);
                break;
            case DREAMSYS_LINK_DAY_START:
                break;
            case DREAMSYS_LINK_DYNAMIC:
                self->methods->enterLinkDynamic(self);
                break;
            case DREAMSYS_LINK_WALL:
                self->methods->enterLinkWall(self);
                break;
            case DREAMSYS_LINK_FLASHBACK:
                self->methods->enterLinkFlashback(self);
                break;
            case DREAMSYS_LINK_TUNNEL:
                self->methods->enterLinkTunnel(self);
                break;
            case DREAMSYS_LINK_STAGE_TIMER:
                self->methods->enterLinkStageTimer(self);
                break;
            case DREAMSYS_LINK_TELEPORT:
                self->methods->notifyLinkTeleport(self);
                break;
        }
    } else if (code >= 9) {
        self->dreamSys->state = DREAMSYS_NO_LINK;
    }
}

void ObjM__EnterTimeUp(ObjM *self) {
    DreamColors color;
    s32 phase;
    s32 t;
    s32 step;

    self->state = 4;
    if (self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, &color, -1) == 0) {
        phase = (self->frameCounter + self->stage) & 3;
        t = phase; /* MATCHING: the copy keeps retail's extra move. */
        if (t == 0) {
            self->methods->notifyParents(self, 4);
            return;
        }
        step = 10;
        switch (t) {
            case 1:
                color = DREAM_COLOR_BLACK;
                break;
            case 2:
                color = DREAM_COLOR_RED;
                break;
            case 3:
                color = DREAM_COLOR_WHITE;
                step = 5;
                break;
        }
        ObjM__StartFadeUp(self, color, 0, step, 1);
        return;
    }
    ObjM__StartFadeUp(self, DREAM_COLOR_BLACK, 0, 5, 1);
}

void ObjM__EnterLinkDynamic(ObjM *self) {
    s32 color;

    if (self->dreamSys->currentStage < 0) {
        self->methods->enterLinkWall(self);
    } else {
        self->state = 5;
        color = self->dreamSys->methods->getDreamColor(self->dreamSys);
        ObjM__StartFadeUp(self, color, 0, 10, 1);
        self->dreamSys->methods->blockMovement(self->dreamSys);
    }
}

void ObjM__EnterLinkWall(ObjM *self) {
    s32 color;

    self->state = 6;
    color = self->dreamSys->methods->getDreamColor(self->dreamSys);
    ObjM__StartFadeUp(self, color, 0, 30, 1);
    self->dreamSys->methods->blockMovement(self->dreamSys);
}

/* ---- ObjM, +0x0A0 to its getter; the style layer's setup ----------------
 *
 * ObjM's methods from +0x0A0 to the end of its table (gObjMMethods) and its
 * getter, then the style layer's setup: the four free functions that pick
 * the stage's scene style.
 *
 * ObjM, in ROM order:
 *  - EnterLinkFlashback, EnterLinkTunnel, EnterLinkStageTimer and NotifyLinkTeleport, the DreamSys link codes
 *    ObjM__OnDreamSysNotify hands on (flashback, tunnel, stage timer,
 *    teleport): each sets IntermediateBase::state (enum ObjMState) and
 *    fades up through StartFadeUp, or notifies its parent at once.
 *  - StartFadeUp: the viewport's fade box (a FadeBox), optionally added as
 *    a child, fades up in the given channels.
 *  - OnFadeNotify: fade down done returns ObjM to IDLE; fade up done sets
 *    the viewport's clear colour to the fade's and notifies the state (a
 *    stage-timer link turns into TIME_UP first). OnStageMapNotify runs
 *    CheckAuxTrigger when a slot's data block is ready.
 *  - The pause overlay: AdvancePauseSetup builds the "Pause" TextRow and,
 *    four calls later, hides the viewport and pauses the FrameClock, the
 *    WBgm and the VabStreamObj; TeardownPauseOverlay undoes it. While it is
 *    up and ObjM is IDLE, the close-ready flag arms CloseAndNotify and CloseAndNotifyNewGame,
 *    which tear it down and notify a close (DayTask ends the day).
 *    NoOpSlotBC is empty.
 *
 * The style setup is not ObjM's, but ObjM is its client:
 * ObjM__InitStyleAndWorld (above) calls RegisterStyleConfig once
 * per scene and keeps the result, sStyleConfig, as ObjM::styleConfig.
 * RegisterStyleConfig stores the scene (grid, stage, ObjM's scene
 * references, day) in the gStyle globals the next section reads;
 * ApplyStyleConfig takes the stage's four-byte StyleStageConfig, or
 * PickStyleFallbackConfig's, and FillStyleFromConfig turns it into
 * StyleConfig colours and a fog distance. ApplyStyleDecorationIfSet builds
 * the decoration box, a full-screen semi-transparent BoxFill under the
 * viewport's fade box, when the config asked for one.
 *
 * What the ObjM states and the style configs stand for in the game is not
 * established; the names describe mechanics.
 */

/* StageMap__SplitChunkIndex's output: the chunk's column and row. */
typedef struct ChunkCoord {
    u8 column;
    u8 row;
} ChunkCoord;

/* src/world/DreamAux.c; it reads `coord` as one s16 trigger key. */
extern s32 TryDreamAuxTrigger(s32 data, ChunkCoord *coord, s32 day);

/* ObjM__AdvancePauseSetup's literals, all reached by address: the "Pause"
 * text, the TextRow's position (attachToParent) and its colour (setColor). */
extern char sPauseText[];             /* "Pause" */
extern ScreenSpritePos sPauseTextPos; /* (-20, -50) */
extern SpriteRgb sPauseTextColor;     /* red: (255, 0, 0) */

void ObjM__EnterLinkFlashback(ObjM *self) {
    DreamColors color;
    self->state = OBJM_STATE_LINK_FLASHBACK;
    self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, &color, -1);
    ObjM__StartFadeUp(self, color, 0, 5, 1);
    self->dreamSys->methods->blockMovement(self->dreamSys);
}

void ObjM__EnterLinkTunnel(ObjM *self) {
    self->state = OBJM_STATE_LINK_TUNNEL;
    ObjM__StartFadeUp(self, DREAM_COLOR_BLACK, 0, 6, 1);
    self->dreamSys->methods->setMoveOverride(self->dreamSys, MOVE_OVERRIDE_FORCED);
}

void ObjM__EnterLinkStageTimer(ObjM *self) {
    self->state = OBJM_STATE_LINK_STAGE_TIMER;
    ObjM__StartFadeUp(self, DREAM_COLOR_BLACK, 0, 6, 1);
    self->dreamSys->methods->selectMoveCallback(self->dreamSys, MOVE_CALLBACK_TICK_DRIFT);
    self->dreamSys->methods->setMoveOverride(self->dreamSys, MOVE_OVERRIDE_HELD);
}

void ObjM__NotifyLinkTeleport(ObjM *self) {
    self->methods->notifyParents(self, OBJM_NOTIFY_LINK_TELEPORT);
}

/* The fade box is the viewport's (IntermediateBase::viewport, a
 * NodeGuardedViewport: getFadeBox); IntermediateBase::frameClock, the FrameClock,
 * drives it. A zero step keeps the box's own. */
void ObjM__StartFadeUp(ObjM *self, s32 channels, s32 fadeMode, s32 step, s32 addChild) {
    FadeBox *fade = (FadeBox *)((NodeGuardedViewport *)self->viewport)
                        ->methods->getFadeBox((NodeGuardedViewport *)self->viewport);
    if (step != 0) {
        fade->methods->setStep(fade, step);
    }
    if (addChild != 0) {
        self->methods->addChild(self, (BasicClass *)fade);
    }
    fade->methods->startFadeUp(fade, self->frameClock, channels, fadeMode);
}

void ObjM__OnFadeNotify(ObjM *self, FadeBox *sender, s32 event) {
    void *color;
    switch (event) {
        case FADEBOX_EVENT_FADE_DOWN_DONE:
            self->methods->removeChild(self, (BasicClass *)sender);
            self->dreamSys->methods->setMoveOverride(self->dreamSys, MOVE_OVERRIDE_NONE);
            self->state = OBJM_STATE_IDLE;
            break;
        case FADEBOX_EVENT_FADE_UP_DONE:
            self->methods->removeChild(self, (BasicClass *)sender);
            color = sender->methods->getColor(sender);
            ((NodeGuardedViewport *)self->viewport)
                ->methods->setClearColor((NodeGuardedViewport *)self->viewport, (ViewportRgb *)color);
            /* MATCHING: the two dead `!=` tests are retail's compares. */
            if (self->state != OBJM_STATE_LINK_DYNAMIC && self->state != OBJM_STATE_LINK_TUNNEL &&
                self->state == OBJM_STATE_LINK_STAGE_TIMER) {
                self->dreamSys->methods->stopDrift(self->dreamSys, 1);
                self->dreamSys->methods->setMoveOverride(self->dreamSys, MOVE_OVERRIDE_NONE);
                self->state = OBJM_STATE_TIME_UP;
            }
            self->methods->notifyParents(self, self->state);
            break;
    }
}

void ObjM__OnStageMapNotify(ObjM *self, BasicClass *sender, s32 event) {
    if (event == STAGEMAP_EVENT_SLOT_DATA_READY) {
        self->methods->checkAuxTrigger(self);
    }
}

/* The StageMap's last event slot's data block goes to TryDreamAuxTrigger
 * with the slot's chunk coordinates and the day; the block is released
 * unless that returns an object, which the slot keeps as heldObj. */
s32 ObjM__CheckAuxTrigger(ObjM *self) {
    ChunkCoord coord;
    s32 held;
    ChunkSlot *slot = ((StageMap *)self->lightRig)
                          ->methods->getLastEventSlotChunk((StageMap *)self->lightRig, &coord.column);
    s32 day = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
    held = TryDreamAuxTrigger((s32)slot->loader->dataBuffer, &coord, day);
    slot->heldObj = (BasicClass *)held;
    if (held != 0) {
        return 0;
    }
    slot->loader->methods->releaseDataBlock(slot->loader);
    return 1;
}

void ObjM__NoOpSlotBC(void) {}

void ObjM__UpdateCloseReadyFlag(ObjM *self) {
    if (self->pauseSetupStep != 0 && self->state == OBJM_STATE_IDLE) {
        self->closeReady = 1;
    }
}

void ObjM__ClearCloseReadyFlag(ObjM *self) {
    self->closeReady = 0;
}

void ObjM__CloseAndNotifyNewGame(ObjM *self) {
    if (self->closeReady) {
        self->methods->teardownPauseOverlay(self);
        self->methods->notifyParents(self, OBJM_NOTIFY_CLOSE_NEW_GAME);
    }
}

void ObjM__CloseAndNotify(ObjM *self) {
    if (self->closeReady) {
        self->methods->teardownPauseOverlay(self);
        self->methods->notifyParents(self, OBJM_NOTIFY_CLOSE);
    }
}

/* Called each update while the overlay is up. Step 0 builds the "Pause"
 * TextRow under the StageMap; the fourth call after it hides the viewport
 * and pauses the FrameClock, the WBgm and the VabStreamObj
 * (IntermediateBase::frameClock, bgm, TimedTask::sound). */
void ObjM__AdvancePauseSetup(ObjM *self) {
    s32 step = self->pauseSetupStep;
    if (step == 0) {
        self->pauseText = New_TextRow(self->etcTim, 5, sPauseText);
        self->pauseText->methods->attachToParent(self->pauseText, (SceneNode *)self->lightRig,
                                                 (LongVec3 *)&sPauseTextPos);
        self->pauseText->methods->setColor(self->pauseText, &sPauseTextColor);
        self->pauseSetupStep = step + 1;
        return;
    }
    self->pauseSetupStep = step + 1;
    if (step != 4) {
        return;
    }
    ((NodeGuardedViewport *)self->viewport)->methods->setDrawEnabled((NodeGuardedViewport *)self->viewport, 0);
    ((FrameClock *)self->frameClock)->methods->pause((FrameClock *)self->frameClock);
    self->bgm->methods->pause(self->bgm);
    ((VabStreamObj *)self->sound)->methods->mute((VabStreamObj *)self->sound);
}

void ObjM__TeardownPauseOverlay(ObjM *self) {
    if (self->pauseSetupStep != 0) {
        self->pauseText->methods->release(self->pauseText);
    }
    ((VabStreamObj *)self->sound)->methods->unmute((VabStreamObj *)self->sound);
    self->bgm->methods->resume(self->bgm);
    ((FrameClock *)self->frameClock)->methods->resume((FrameClock *)self->frameClock);
    ((NodeGuardedViewport *)self->viewport)->methods->setDrawEnabled((NodeGuardedViewport *)self->viewport, 1);
    self->pauseSetupStep = 0;
}

ObjMMethods *GetObjMMethods(void) {
    return &gObjMMethods;
}

extern s32 sStyleGrid;
extern s32 gStyleStage;
extern s32 gStyleTickCount;
extern s32 sStyleDay;
extern s32 sStyleUnreadArg;
extern s32 gStyleSceneRefs; /* a StyleSceneRefs * (below) */
extern s32 gStyleVariant;
/* StyleCueSlot is defined with the cue functions below; this only clears the slots. */
typedef struct StyleCueSlot StyleCueSlot;
extern StyleCueSlot *sStyleCueSlots[2];

extern void *ApplyStyleConfig(void);

s32 RegisterStyleConfig(s32 grid, s32 stage, s32 sceneRefs, s32 day, s32 unreadArg) {
    StyleCueSlot **slot;
    s32 i;

    if (sStyleGrid == 0) {
        i = ARRAY_COUNT(sStyleCueSlots) - 1;
        slot = &sStyleCueSlots[ARRAY_COUNT(sStyleCueSlots) - 1];
        sStyleGrid = grid;
        gStyleStage = stage;
        gStyleSceneRefs = sceneRefs;
        gStyleVariant = -1;
        sStyleDay = day;
        sStyleUnreadArg = unreadArg;
        gStyleTickCount = 0;
        do {
            *slot = 0;
            i--;
            slot--;
        } while (i >= 0);
        return (s32)ApplyStyleConfig();
    }
    return 0;
}

/* A stage's style config: four signed bytes, from sStyleStageConfigs (NULL
 * for a stage without a fixed one) or PickStyleFallbackConfig
 * (next section), which FillStyleFromConfig turns into sStyleConfig's last
 * four words. */
typedef struct StyleStageConfig {
    s8 colorMode; /* StyleConfig::colorMode */
    s8 fogLevel; /* sStyleFogNears index; STYLE_DECOR_FOG_LEVEL and up also build the decoration box */
    s8 farColorIndex; /* sStylePalette index: StyleConfig::farColor, and the decoration box's colour */
    s8 clearColorIndex; /* sStylePalette index: StyleConfig::clearColor */
} StyleStageConfig;

/* The fog levels whose config also gets a decoration box (ApplyStyleConfig):
 * this one and up, sStyleFogNears' two nearest (fogNear 4096 and 2048). */
#define STYLE_DECOR_FOG_LEVEL 4

extern StyleConfig sStyleConfig;
extern StyleStageConfig *sStyleStageConfigs[];
extern void *PickStyleFallbackConfig(void);
extern void FillStyleFromConfig(StyleConfig *style, StyleStageConfig *cfg);
extern u8 sStylePalette[][3];
extern const u8 *sStyleDecorColor;

/* The stage's fixed config, or with none PickStyleFallbackConfig's, into
 * sStyleConfig, whose first three words (the StageMap's light settings)
 * are fixed. */
void *ApplyStyleConfig(void) {
    StyleStageConfig *cfg = sStyleStageConfigs[gStyleStage];

    if (cfg == 0) {
        cfg = PickStyleFallbackConfig();
    }
    FillStyleFromConfig(&sStyleConfig, cfg);
    if (cfg->fogLevel >= STYLE_DECOR_FOG_LEVEL) {
        sStyleDecorColor = sStylePalette[cfg->farColorIndex];
    }
    return &sStyleConfig;
}

/* sStylePalette is 24 RGB triples (a greyscale ramp first: 0, 64, 128, 255).
 * MATCHING: indexed as `u8[][3]`, for retail's `i*2 + i + base` stride-3
 * address arithmetic. sStyleFogNears is six fogNear distances, 26624 down to
 * 2048. */
extern s32 sStyleFogNears[];

void FillStyleFromConfig(StyleConfig *style, StyleStageConfig *cfg) {
    style->clearColor = sStylePalette[cfg->clearColorIndex];
    style->farColor = sStylePalette[cfg->farColorIndex];
    style->fogNear = sStyleFogNears[cfg->fogLevel];
    style->colorMode = cfg->colorMode;
}

/* What gStyleSceneRefs points at: ObjM's +0x06C..+0x07B block
 * (ObjM__InitStyleAndWorld passes &ctorSound to RegisterStyleConfig, which
 * keeps it; include/ObjM.h). */
typedef struct StyleSceneRefs {
    void *sound;        /* +0x000, ObjM::ctorSound: the sound object the cue functions take first */
    void *dreamerTmd;   /* +0x004, ObjM::dreamerTmd */
    void *etcTim;       /* +0x008, ObjM::etcTim */
    Viewport *viewport; /* +0x00C, ObjM::cachedViewport */
} StyleSceneRefs;

extern s32 sStyleDecorObj;           /* a BoxFill * */
extern s32 sStyleDecorBoxSize[2];    /* 320 x 240, the screen */
extern BoxFillPos sStyleDecorBoxPos; /* (-100, -100), as Viewport's own fade box */

void ApplyStyleDecorationIfSet(void) {
    SceneNode *fadeBox;

    if (sStyleDecorColor != 0) {
        sStyleDecorObj = (s32)New_BoxFill(sStyleDecorBoxSize, (BoxFillRgb *)sStyleDecorColor, 0);
        ((BoxFill *)sStyleDecorObj)->methods->setSemiTransOn((BoxFill *)sStyleDecorObj, 1);
        ((BoxFill *)sStyleDecorObj)->methods->setSemiTransRate((BoxFill *)sStyleDecorObj, 0);

        fadeBox = ((StyleSceneRefs *)gStyleSceneRefs)
                      ->viewport->methods->getFadeBox(((StyleSceneRefs *)gStyleSceneRefs)->viewport);

        ((BoxFillAttachToParentFn)((BoxFill *)sStyleDecorObj)->methods->attachToParent)(
            (BoxFill *)sStyleDecorObj, fadeBox, &sStyleDecorBoxPos);
    }
}

/* ---- The style layer's per-scene objects ---------------------------------
 *
 * What TickStyle
 * builds on a scene's first tick, updates on every tick, and StyleTeardown
 * releases.
 *
 * RegisterStyleConfig (previous section), called by ObjM__InitStyleAndWorld
 * and a no-op until StyleTeardown clears sStyleGrid, sets the state read here: sStyleGrid (the
 * scene's StageMap), gStyleStage (ObjM's stage), gStyleSceneRefs (ObjM's
 * sound, resources and viewport; StyleSceneRefs below) and sStyleDay (the
 * DreamSys day). ApplyStyleConfig then takes the stage's fixed config or,
 * with none, PickStyleFallbackConfig's: a variant (gStyleVariant, 0..3) and
 * a config record picked from day + stage.
 *
 * What TickStyle keeps, each built on the first tick:
 *  - the decoration box, sStyleDecorObj, when the config has a colour for
 *    it (ApplyStyleDecorationIfSet builds it, StyleFlushDecoration releases it);
 *  - the decor set: STYLE_DECOR_BANDS BoxFill bands (sStyleDecorSlots)
 *    coloured from sStyleDecorColors and attached under the viewport's fade
 *    box; every tick StyleUpdateDecorSet shifts their colours, their
 *    position and the viewport's clear colour by the view point's y offset
 *    from its reference point;
 *  - the effect slots: StyleEffect objects of kinds 0..3 (sStyleEffectSlots)
 *    built from one parameter block, gStyleSpawnOffsetX..gStyleSpawnColors,
 *    that StyleFillEffectKindN and SetupStyleSpawnParamsRandom/B fill in; each
 *    tick updates them with the target position;
 *  - two positional sound cues (sStyleCueSlots, in sStyleCueSlotPool): a
 *    free slot claims the next record of the stage's cue list that lies
 *    within its cue's distance of the target and starts the record's
 *    SoundCueSet callback (sStyleCueCallbacks, next section); a claimed
 *    slot is serviced while the target stays in range and flushed when it
 *    leaves.
 * StyleScrollVramStrips also rotates a VRAM strip one column per tick on
 * stages 2 to 5.
 *
 * The target is the grid's target cell (ObjM__TickStyle passes
 * getTargetDescriptor) turned into a world position. What the style layer
 * is in the game -- what a variant, an effect kind or a cue stands for -- is
 * not established; the names describe mechanics. Evidence and tiers are in
 * each function's match report, `## Naming`.
 */


/* The decoration set: this many BoxFill bands, stacked 3 pixels apart. */
#define STYLE_DECOR_BANDS 18

/* Every band's draw priority: the largest value BoxFill's default 13-bit
 * priority mask admits (BoxFill__Reset calls setMask(13)). */
#define STYLE_DECOR_PRI 0x1FFF

/* How far down (pixels) decor variant 2 draws the set. */
#define STYLE_DECOR_VARIANT2_DROP 30

/* StyleUpdateDecorSet's fade step: the viewport's view-point y less its
 * reference-point y, per step. */
#define STYLE_DECOR_FADE_HEIGHT 600

/* Kind-0 plus kind-1 effects built for style variant 2. */
#define STYLE_VARIANT2_EFFECTS 16

/* The palette entry that, as a variant-0 config's decor colour, selects
 * sStyleDecorColorsB instead of sStyleDecorColorsA (PickStyleFallbackConfig). */
#define STYLE_DECOR_B_PALETTE_INDEX 18


extern const u8 *sStyleDecorColor;
extern s32 sStyleDecorObj; /* a BoxFill */

/* Releases the decoration box, if ApplyStyleDecorationIfSet made one. */
void StyleFlushDecoration(void) {
    if (sStyleDecorColor != 0) {
        ((BoxFill *)sStyleDecorObj)->methods->release((BoxFill *)sStyleDecorObj);
        sStyleDecorColor = 0;
    }
}

extern s32 sStyleDay;
extern s32 gStyleStage;
extern s8 gStyleVariantPicks[];
extern s32 gStyleVariant;
extern s8 gStyleVariantConfigCounts[];
extern s32 sStyleConfigIndex;
extern s8 *gStyleVariantConfigs[];
extern const u8 *sStyleClearColor;
extern u8 sStyleDecorColorsB[];
extern u8 sStylePalette[][3];
extern const u8 *sStyleDecorColors;
extern u8 sStyleDecorColorsA[];
extern s32 sStyleDecorVariant;

/* The config for a stage without a fixed one: the variant from
 * gStyleVariantPicks[(day + stage) & 0xF], then record (day + stage) % count
 * of that variant's table. For variant 0 it also sets the clear colour, the
 * band colours and, for records 0..5, the decor variant (1, or 2 for 4..5). */
void *PickStyleFallbackConfig(void) {
    s32 seed;
    s32 variant;
    s32 count;
    s32 index;
    s8 *config;
    s32 clearIndex;
    s32 decorIndex;
    u8 *decorColors;

    seed = sStyleDay + gStyleStage;
    variant = gStyleVariantPicks[seed & 0xF];
    gStyleVariant = variant;
    count = gStyleVariantConfigCounts[variant];
    index = seed % count;
    sStyleConfigIndex = index;
    config = gStyleVariantConfigs[variant] + index * 4;
    if (variant == 0) {
        clearIndex = config[3];
        sStyleClearColor = sStylePalette[clearIndex];
        decorIndex = config[2];
        decorColors = sStyleDecorColorsB;
        if (decorIndex != STYLE_DECOR_B_PALETTE_INDEX) {
            decorColors = sStyleDecorColorsA;
        }
        sStyleDecorColors = decorColors;
        if (index < 4) {
            sStyleDecorVariant = 1;
        } else if (index < 6) {
            sStyleDecorVariant = 2;
        }
    }
    return config;
}

extern s32 sStyleDecorPosX;
extern s32 sStyleDecorPosY;
extern s32 sStyleDecorSizeW;
extern s32 sStyleDecorSizeH;
extern BoxFill *sStyleDecorSlots[STYLE_DECOR_BANDS];
extern s32 gStyleSceneRefs; /* a StyleSceneRefs * */

/* sStyleDecorPosX/Y and sStyleDecorSizeW/H are adjacent word pairs.
 * MATCHING: copied whole, never field by field (a BLKmode copy makes cse
 * drop cached memory values; scalar copies lose retail's reloads). */
typedef struct PairXY PairXY;

struct PairXY {
    s32 x; /* +0x000 */
    s32 y; /* +0x004 */
};

/* Builds the bands: band 0 in sStyleDecorColors' first colour, bands 1..17
 * attached under it, each 3 pixels lower and 7 shorter than the one before;
 * band 0 then goes under the viewport's fade box. */
void StyleBuildDecorSet(void) {
    PairXY pos;
    PairXY size;
    s32 i;
    BoxFill *band;
    Viewport *viewport;
    SceneNode *parent;

    if (sStyleDecorVariant == 0) {
        return;
    }
    pos = *(PairXY *)&sStyleDecorPosX;
    if (sStyleDecorVariant == 2) {
        pos.y += STYLE_DECOR_VARIANT2_DROP;
    }
    size = *(PairXY *)&sStyleDecorSizeW;
    sStyleDecorSlots[0] = New_BoxFill(&size, (void *)sStyleDecorColors, STYLE_DECOR_PRI);
    for (i = 1; i < STYLE_DECOR_BANDS; i++) {
        band = New_BoxFill(&size, (void *)(sStyleDecorColors + i * 3), STYLE_DECOR_PRI);
        sStyleDecorSlots[i] = band;
        ((BoxFillAttachToParentFn)band->methods->attachToParent)(
            band, (SceneNode *)sStyleDecorSlots[0], (BoxFillPos *)&pos);
        pos.y += 3;
        size.y -= 7;
    }

    viewport = ((StyleSceneRefs *)gStyleSceneRefs)->viewport;
    parent = viewport->methods->getFadeBox(viewport);
    ((BoxFillAttachToParentFn)sStyleDecorSlots[0]->methods->attachToParent)(
        sStyleDecorSlots[0], parent, (BoxFillPos *)&pos);
}

void AdjustRgbByDelta(u8 *dst, u8 *src, s32 delta);

/* Every tick, once the view point's y exceeds the reference point's by a
 * fade step: each band's colour
 * and the clear colour lose `fade` red and green and gain `fade` blue, and
 * the set moves down 3 pixels per unit of fade. */
void StyleUpdateDecorSet(void) {
    Viewport *viewport;
    s32 height;
    s32 fade;
    u8 rgb[8]; /* MATCHING: 8, not 3 (the frame keeps pos at sp+0x18) */
    PairXY pos;
    s32 colorOfs;
    s32 i;
    BoxFill **slot;
    BoxFill *band;

    if (sStyleDecorVariant == 0) {
        return;
    }
    viewport = ((StyleSceneRefs *)gStyleSceneRefs)->viewport;
    height = viewport->refView.vp.y - viewport->refView.vr.y;
    fade = (height / STYLE_DECOR_FADE_HEIGHT) * 3;
    if (fade <= 0) {
        return;
    }
    pos = *(PairXY *)&sStyleDecorPosX;
    i = 0;
    if (sStyleDecorVariant == 2) {
        pos.y += STYLE_DECOR_VARIANT2_DROP;
    }
    slot = sStyleDecorSlots;
    colorOfs = 0;
    pos.y += fade * 3;
    do {
        AdjustRgbByDelta(rgb, (u8 *)(colorOfs + sStyleDecorColors), fade);
        band = *slot;
        band->methods->setColor(band, 1, rgb);
        band = *slot;
        i++;
        colorOfs += 3;
        band->methods->setPosition(band, (BoxFillPos *)&pos);
        pos.y += 3;
        slot++;
    } while (i < STYLE_DECOR_BANDS);
    AdjustRgbByDelta(rgb, (u8 *)sStyleClearColor, fade);
    viewport->methods->setClearColor(viewport, (ViewportRgb *)rgb);
}

/* dst = src with red and green less `delta`, blue more. */
void AdjustRgbByDelta(u8 *dst, u8 *src, s32 delta) {
    dst[0] = src[0] - delta;
    dst[1] = src[1] - delta;
    dst[2] = src[2] + delta;
}

extern void ReleaseBasicClassArray(void **array, s32 count);
extern s32 sStyleDecorVariant;
extern BoxFill *sStyleDecorSlots[STYLE_DECOR_BANDS];

/* Releases the bands, if StyleBuildDecorSet made them. */
void StyleReleaseDecorSet(void) {
    if (sStyleDecorVariant != 0) {
        ReleaseBasicClassArray((void **)sStyleDecorSlots, ARRAY_COUNT(sStyleDecorSlots));
        sStyleDecorVariant = 0;
    }
}

extern s32 gStyleVariant;
extern s32 gStyleSceneRefs;
extern s32 rand(void);
extern s8 sStyleKind0Counts[];
extern s32 sStyleEffectSlotCount;
extern StyleEffect *sStyleEffectSlots[];
extern StyleEffect **StyleFillEffectKind0(StyleEffect **slots, s32 count, LongVec3 *pos);
extern StyleEffect **StyleFillEffectKind1(StyleEffect **slots, s32 count, LongVec3 *pos);
extern StyleEffect **StyleFillEffectKind3(StyleEffect **slots, LongVec3 *pos);
extern StyleEffect **StyleFillEffectKind2(StyleEffect **slots, LongVec3 *pos);

/* Hands the variant and ObjM's resources to SetStyleEffectSources, then builds
 * the effect slots for the variant: sStyleKind0Counts' pick of
 * kind 0, kind 1 up to STYLE_VARIANT2_EFFECTS for variant 2, then one kind-3
 * (variant 0) or kind-2 (variant 2). */
void StyleBuildEffectSlots(LongVec3 *pos) {
    StyleSceneRefs *refs;
    s32 kind0Count;
    s32 kind1Count;
    StyleEffect **next;

    if (gStyleVariant < 0) {
        return;
    }
    refs = (StyleSceneRefs *)gStyleSceneRefs;
    SetStyleEffectSources(gStyleVariant, (Actor *)refs->dreamerTmd, (s32)refs->etcTim,
                          (s32)refs->viewport);
    kind0Count = sStyleKind0Counts[rand() & 3];
    kind1Count = (gStyleVariant == 2) ? STYLE_VARIANT2_EFFECTS - kind0Count : 0;
    sStyleEffectSlotCount = kind0Count + kind1Count;
    next = StyleFillEffectKind0(sStyleEffectSlots, kind0Count, pos);
    next = StyleFillEffectKind1(next, kind1Count, pos);
    if (gStyleVariant == 0) {
        StyleFillEffectKind3(next, pos);
    } else if (gStyleVariant == 2) {
        StyleFillEffectKind2(next, pos);
    } else {
        return;
    }
    sStyleEffectSlotCount = sStyleEffectSlotCount + 1;
}

extern s32 gStyleVariant;
extern s32 sStyleEffectSlotCount;

/* Each slot's +0x0EC is StyleEffect__Update, called with the position
 * (include/StyleEffect.h: the slot keeps Actor's setPendingExtra type). */
void StyleUpdateEffectSlots(LongVec3 *pos) {
    s32 i;
    StyleEffect *slot;

    if (gStyleVariant < 0) {
        return;
    }
    for (i = 0; i < sStyleEffectSlotCount; i++) {
        slot = sStyleEffectSlots[i];
        ((StyleEffectUpdateFn)slot->methods->setPendingExtra)(slot, pos);
    }
}

extern s32 gStyleVariant;
extern s32 sStyleEffectSlotCount;

/* Releases the effect slots, if StyleBuildEffectSlots ran. */
void StyleReleaseEffectSlots(void) {
    if (gStyleVariant >= 0) {
        ReleaseBasicClassArray((void **)sStyleEffectSlots, sStyleEffectSlotCount);
    }
}

/* A cue record's view here: `cue` is its cue index (the sStyleCueCallbacks
 * and sStyleCueDistanceTable row, InitSoundCueSet's tag), negated while a
 * slot holds the record. EntrySlot, below, is the whole 8-byte record. */
typedef struct StyleCueEntryView StyleCueEntryView;

struct StyleCueEntryView {
    u8 pad0[0x6];
    s8 cue; /* +0x006 */
};

/* One of the two positional cues: the record it holds, the record's world
 * position, the last distance to the target, and its sound cue. */
struct StyleCueSlot {
    StyleCueEntryView *entry; /* +0x000 */
    LongVec3 pos;             /* +0x004 */
    s32 lastDist;             /* +0x010 */
    SoundCueSet cueSet;       /* +0x014 */
}; /* 0x68 bytes */

extern StyleCueSlot *FlushStyleCue(StyleCueSlot *slot);

extern s32 sStyleGrid; /* a StageMap */
extern StyleCueSlot *sStyleCueSlots[2];

/* Releases everything TickStyle built and unregisters the scene. */
void StyleTeardown(void) {
    s32 i;

    StyleFlushDecoration();
    StyleReleaseDecorSet();
    StyleReleaseEffectSlots();
    for (i = 0; i < ARRAY_COUNT(sStyleCueSlots); i++) {
        sStyleCueSlots[i] = FlushStyleCue(sStyleCueSlots[i]);
    }
    if (sStyleGrid != 0) {
        sStyleGrid = 0; /* RegisterStyleConfig registers only while this is 0 */
    }
}

extern Ratio16 gStyleSpawnScales[][3];
extern s32 gStyleSpawnYChoices[];
extern Ratio16 *gStyleSpawnScale;
extern s32 gStyleSpawnTableIndex;
/* The first word of the StyleEffectParams block every effect is built from
 * (gStyleSpawnOffsetX .. gStyleSpawnColors, separate symbols in the image). */
extern s32 gStyleSpawnOffsetX;
extern void SetupStyleSpawnParamsRandom(LongVec3 *pos, s32 offsetY);
extern void SetupStyleSpawnParamsDayMod7(LongVec3 *pos, s32 offsetY);

/* Fills `count` slots with kind-0 effects: a table index and scale for all
 * of them, an offset y (0: each setup picks one; a pick of 4 reads the word
 * after gStyleSpawnYChoices, as retail does), and per slot
 * SetupStyleSpawnParamsRandom, or B on every seventh day. Returns the next slot. */
StyleEffect **StyleFillEffectKind0(StyleEffect **slots, s32 count, LongVec3 *pos) {
    s32 i;
    s32 offsetY;
    void (*setup)(LongVec3 *, s32);

    gStyleSpawnTableIndex = rand() % 7;
    gStyleSpawnScale = gStyleSpawnScales[(u32)rand() % 5];
    offsetY = (u32)rand() % 5;
    if (offsetY != 0) {
        offsetY = gStyleSpawnYChoices[offsetY];
    }
    setup = SetupStyleSpawnParamsDayMod7;
    if (sStyleDay % 7 != 0) {
        setup = SetupStyleSpawnParamsRandom;
    }
    for (i = 0; i < count; i++) {
        setup(pos, offsetY);
        *slots =
            New_StyleEffect(0, (StyleEffectParams *)&gStyleSpawnOffsetX, (SceneNode *)sStyleGrid, pos);
        slots++;
    }
    return slots;
}

extern s32 gStyleSpawnYChoice2;
extern Ratio16 sStyleKind1Scale[];

/* Fills `count` slots with kind-1 effects: sStyleKind1Scale, offset y
 * gStyleSpawnYChoice2. */
StyleEffect **StyleFillEffectKind1(StyleEffect **slots, s32 count, LongVec3 *pos) {
    s32 i;
    s32 offsetY;

    offsetY = gStyleSpawnYChoice2;
    gStyleSpawnScale = sStyleKind1Scale;
    for (i = 0; i < count; i++) {
        SetupStyleSpawnParamsRandom(pos, offsetY);
        *slots =
            New_StyleEffect(1, (StyleEffectParams *)&gStyleSpawnOffsetX, (SceneNode *)sStyleGrid, pos);
        slots++;
    }
    return slots;
}

extern s32 gStyleSpawnYChoice2;
extern void SetupStyleSpawnParamsRandom(LongVec3 *pos, s32 offsetY);
extern s32 gStyleSpawnColors[];
extern Ratio16 *gStyleSpawnRotation;
extern Ratio16 gStyleSpawnRotations[][3];
extern s32 gStyleSpawnOffsetY;
extern s32 gStyleSpawnOffsetZ;
extern u8 sStyleKind3Colors[][3];

/* MATCHING: the rotation store goes through a one-field struct, so the
 * sStyleGrid load may schedule above it (a plain pointer store blocks it). */
typedef struct PtrBoxK3 {
    Ratio16 *p; /* +0x000 */
} PtrBoxK3;

/* Appends one kind-3 effect. With decor variant active and band colours B
 * its offset and colour are fixed; otherwise its z offset is folded to
 * -30720..0 and its colour is random. */
StyleEffect **StyleFillEffectKind3(StyleEffect **slots, LongVec3 *pos) {
    s32 *offsetZ;
    PtrBoxK3 *rotation;

    SetupStyleSpawnParamsRandom(pos, gStyleSpawnYChoice2);
    if (sStyleDecorVariant != 0 && sStyleDecorColors == sStyleDecorColorsB) {
        gStyleSpawnOffsetX = -45056;
        gStyleSpawnOffsetY = -8192;
        gStyleSpawnOffsetZ = 0;
        gStyleSpawnColors[0] = (s32)sStyleKind3Colors[1];
    } else {
        offsetZ = &gStyleSpawnOffsetZ;
        if (*offsetZ > 0) {
            *offsetZ = -*offsetZ;
        }
        if (*offsetZ < -30720) {
            *offsetZ = -30720;
        }
        gStyleSpawnColors[0] = (s32)sStyleKind3Colors[(u32)rand() % 3];
    }
    rotation = (PtrBoxK3 *)&gStyleSpawnRotation;
    rotation->p = gStyleSpawnRotations[0];
    /* MATCHING: the block's address is taken back from its rotation member */
    *slots = New_StyleEffect(
        3, (StyleEffectParams *)((u8 *)rotation - offsetof(StyleEffectParams, rotation)),
        (SceneNode *)sStyleGrid, pos);
    slots++;
    return slots;
}

extern s32 sStyleKind2AltColor;
extern u8 sStyleKind2Colors[][3];
extern s32 gStyleSpawnColors[];
extern Ratio16 *gStyleSpawnRotation;
extern Ratio16 gStyleSpawnRotations[][3];
extern s32 gStyleSpawnTableIndex;

/* MATCHING: the first colour store goes through a one-field struct, as
 * PtrBoxK3's does, so the sStyleDay load may schedule above it. */
typedef struct S32BoxK2 {
    s32 v; /* +0x000 */
} S32BoxK2;

/* Appends one kind-2 effect with a random colour and, except on every
 * twentieth day, sStyleKind2AltColor as its alternate colour. */
StyleEffect **StyleFillEffectKind2(StyleEffect **slots, LongVec3 *pos) {
    s32 r;
    s32 altColor;
    S32BoxK2 *color;
    Ratio16 **rotation;

    r = rand();
    color = (S32BoxK2 *)gStyleSpawnColors;
    color->v = (s32)sStyleKind2Colors[(u32)r % 3];
    color++;
    altColor = (sStyleDay / 20) * 20; /* MATCHING: not `% 20`, which jump.c folds */
    if (sStyleDay != altColor) {
        altColor = sStyleKind2AltColor;
    } else {
        altColor = 0;
    }
    color->v = altColor;
    SetupStyleSpawnParamsRandom(pos, gStyleSpawnYChoice2);
    rotation = &gStyleSpawnRotation;
    *rotation = gStyleSpawnRotations[0];
    gStyleSpawnTableIndex = rand() % 6;
    /* MATCHING: the block's address is taken back from its rotation member */
    *slots = New_StyleEffect(
        2, (StyleEffectParams *)((u8 *)rotation - offsetof(StyleEffectParams, rotation)),
        (SceneNode *)sStyleGrid, pos);
    slots++;
    return slots;
}

extern s32 gStyleSpawnOffsetY;
extern s32 gStyleSpawnOffsetZ;
extern Ratio16 *gStyleSpawnRotation;
extern Ratio16 gStyleSpawnRotations[][3];
extern s32 gStyleSpawnModelLayout;

/* Randomises the spawn parameters: offset y (offsetY, or a random choice
 * when 0), x and z offsets of 0..22 steps of 2048 either side, a rotation
 * and a model layout. `pos` is unused.
 * MATCHING: gStyleSpawnOffsetX is declared a scalar, not an array (an array
 * decay is kept in a saved register across the rand() calls). */
void SetupStyleSpawnParamsRandom(LongVec3 *pos, s32 offsetY) {
    if (offsetY == 0) {
        offsetY = gStyleSpawnYChoices[rand() & 3];
    }
    gStyleSpawnOffsetY = offsetY;
    gStyleSpawnOffsetX = (rand() % 23) << 11;
    if (rand() & 1) {
        gStyleSpawnOffsetX = -gStyleSpawnOffsetX;
    }
    gStyleSpawnOffsetZ = (rand() % 23) << 11;
    if (rand() & 1) {
        gStyleSpawnOffsetZ = -gStyleSpawnOffsetZ;
    }
    gStyleSpawnRotation = gStyleSpawnRotations[(u32)rand() % 7];
    gStyleSpawnModelLayout = rand() % 5;
}

extern s32 gStyleSpawnYChoice1;
extern s32 gStyleSpawnModelLayout;

/* The every-seventh-day setup: fixed offset y, x of 0..19 steps of 2048, z
 * by day % 3 (40960, -40960, 2048), then the same rotation and layout
 * picks as SetupStyleSpawnParamsRandom. Both parameters are unused; it has
 * that function's signature because StyleFillEffectKind0 calls either
 * through one pointer.
 * MATCHING: each rand() is used inline; one local for all three adds a move
 * after every call. */
void SetupStyleSpawnParamsDayMod7(LongVec3 *pos, s32 offsetY) {
    s32 dayMod3;

    rand();
    gStyleSpawnOffsetY = gStyleSpawnYChoice1;
    gStyleSpawnOffsetX = (rand() % 20) << 11;
    dayMod3 = sStyleDay % 3;
    gStyleSpawnOffsetZ = 40960;
    if (dayMod3 == 1) {
        gStyleSpawnOffsetZ = -40960;
    } else if (dayMod3 == 2) {
        gStyleSpawnOffsetZ = 2048;
    }
    gStyleSpawnRotation = gStyleSpawnRotations[(u32)rand() % 7];
    gStyleSpawnModelLayout = rand() % 5;
}

extern s32 gStyleSceneRefs;
extern void *FindNextStyleCueInRange(LongVec3 *pos, s32 *outDist, LongVec3 *target);
extern SoundCueCallbackFn sStyleCueCallbacks[];
extern s32 InitSoundCueSet(void *sound, SoundCueSet *set, s32 tag, void *owner,
                           SoundCueCallbackFn callback);

/* Claims the next record in range for `slot` and starts its cue. A started
 * cue equal to *lastCue is reported back negated. Returns the slot, or NULL. */
StyleCueSlot *TryStartStyleCue(StyleCueSlot *slot, s32 *lastCue, LongVec3 *target, void *unused) {
    StyleCueEntryView *entry;

    entry = (StyleCueEntryView *)FindNextStyleCueInRange(&slot->pos, &slot->lastDist, target);
    if (entry != 0) {
        slot->entry = entry;
        InitSoundCueSet(((StyleSceneRefs *)gStyleSceneRefs)->sound, &slot->cueSet, entry->cue, slot,
                        sStyleCueCallbacks[entry->cue]);
        if (entry->cue == *lastCue) {
            *lastCue = -entry->cue;
        }
        entry->cue = -entry->cue;
        return slot;
    }
    return 0;
}

extern s32 gStyleStage;
extern s32 sStyleCueRecordIndex;
extern u8 *sStyleCueRecordLists[];
extern u8 sStyleCueRecordCounts[];
extern s32 sStyleCueDistanceTable[];

/* The cell-key halves: a record's four cell bytes and sStyleCueOffsets' s16
 * x/y/z, copied whole into a 10-byte cell key (StageMap's Descriptor10
 * shape) for computeCellOffsets. */
typedef struct Pos4 Pos4;

struct Pos4 {
    s16 hi;
    s16 lo;
};

typedef struct TabEntry TabEntry;

struct TabEntry {
    Pos4 head;
    s16 tail;
};

extern TabEntry sStyleCueOffsets[];

typedef struct EntrySlot EntrySlot;

struct EntrySlot {
    Pos4 pos;       /* +0x0 */
    u8 offsetIndex; /* +0x4 */
    u8 pad5;        /* +0x5 */
    s8 cue;         /* +0x6 */
    u8 pad7;        /* +0x7 */
};

typedef struct LocalBuf LocalBuf;

struct LocalBuf {
    Pos4 pos;
    TabEntry tab;
};

/* From sStyleCueRecordIndex on, the first free record of the stage's list
 * whose X+Z distance from the target is under its cue's distance; each
 * record looked at advances the index, so the next slot's search this tick
 * goes on from there.
 * Writes the record's world position and distance. */
void *FindNextStyleCueInRange(LongVec3 *pos, s32 *outDist, LongVec3 *target) {
    s32 j, remaining;
    u8 *records;
    EntrySlot *entry;
    LocalBuf buf;
    s32 dx, dz, dist;
    StageMap *grid;

    if (target == 0) {
        goto fail;
    }
    records = sStyleCueRecordLists[gStyleStage];
    remaining = sStyleCueRecordCounts[gStyleStage] - sStyleCueRecordIndex;
    entry = (EntrySlot *)(sStyleCueRecordIndex * 8 + (s32)records); /* MATCHING: operand order */
    for (j = 0; j < remaining; j++, entry++) {
        sStyleCueRecordIndex++;
        if (entry->cue > 0) {
            buf.pos = entry->pos;
            buf.tab = sStyleCueOffsets[entry->offsetIndex];
            grid = (StageMap *)sStyleGrid;
            grid->methods->computeCellOffsets(grid, pos, &buf);
            dx = pos->x - target->x;
            if (dx < 0) {
                dx = ~dx + 1; /* MATCHING: not -dx (the nor fills the delay slot) */
            }
            dz = pos->z - target->z;
            if (dz >= 0) {
                dist = dx + dz;
            } else {
                dist = dx - dz;
            }
            *outDist = dist;
            if (dist < sStyleCueDistanceTable[entry->cue]) {
                return entry;
            }
        }
    }
fail:
    return 0;
}

extern s32 gStyleSceneRefs;
extern void FlushSoundCueSet(void *sound, SoundCueSet *set);

/* Stops the slot's cue and frees its record. Returns NULL for the slot. */
StyleCueSlot *FlushStyleCue(StyleCueSlot *slot) {
    FlushSoundCueSet(((StyleSceneRefs *)gStyleSceneRefs)->sound, &slot->cueSet);
    slot->entry->cue = -slot->entry->cue;
    return 0;
}

extern s32 IsStyleCueNear(StyleCueSlot *slot, LongVec3 *target);
extern void ServiceSoundCueSet(void *sound, SoundCueSet *set);

/* One service pass of the slot's cue while the target is in range; 0 otherwise. */
s32 ServiceStyleCueIfNear(StyleCueSlot *slot, LongVec3 *target, void *unused) {
    if (IsStyleCueNear(slot, target) != 0) {
        ServiceSoundCueSet(((StyleSceneRefs *)gStyleSceneRefs)->sound, &slot->cueSet);
        return 1;
    }
    return 0;
}

extern s32 sStyleCueDistanceTable[];

/* Whether the target is within the held cue's distance (X+Z); keeps the distance. */
s32 IsStyleCueNear(StyleCueSlot *slot, LongVec3 *target) {
    s32 dx, dz, dist;
    s8 cue;

    if (target == 0) {
        return 0;
    }
    dx = slot->pos.x - target->x;
    if (dx < 0) {
        dx = ~dx + 1; /* MATCHING: not -dx (the nor fills the delay slot) */
    }
    dz = slot->pos.z - target->z;
    if (dz >= 0) {
        dist = dx + dz;
    } else {
        dist = dx - dz;
    }
    slot->lastDist = dist;
    cue = slot->entry->cue;
    if (dist < sStyleCueDistanceTable[-cue]) {
        dist = 1; /* MATCHING: not `return 1` (jump.c folds that to slt) */
        return dist;
    }
    return 0;
}

extern void ApplyStyleDecorationIfSet(void); /* previous section */
extern void StyleBuildDecorSet(void);
extern void StyleUpdateDecorSet(void);
extern void StyleScrollVramStrips(void);
extern s32 gStyleTickCount;
extern s32 sStyleCueRecordIndex;
extern StyleCueSlot sStyleCueSlotPool[];
extern StyleCueSlot *TryStartStyleCue(StyleCueSlot *slot, s32 *lastCue, LongVec3 *target, void *unused);
extern s32 ServiceStyleCueIfNear(StyleCueSlot *slot, LongVec3 *target, void *unused);

/* The per-tick entry (ObjM__TickStyle): the target position from `cell`,
 * first-tick builds, then the updates and the two cue slots. Returns
 * lastCue, negated if a slot started that cue this tick. */
s32 TickStyle(Descriptor10 *cell, void *unused, s32 lastCue) {
    LongVec3 *target;
    LongVec3 targetPos;
    s32 i;

    target = 0;
    if (cell != 0) {
        target = &targetPos;
        ((StageMap *)sStyleGrid)->methods->computeCellOffsets((StageMap *)sStyleGrid, target, cell);
    }
    if (gStyleTickCount++ == 0) {
        ApplyStyleDecorationIfSet();
        StyleBuildDecorSet();
        StyleBuildEffectSlots(target);
    }
    StyleUpdateDecorSet();
    StyleUpdateEffectSlots(target);
    StyleScrollVramStrips();
    sStyleCueRecordIndex = 0;
    for (i = 0; i < ARRAY_COUNT(sStyleCueSlots); i++) {
        if (sStyleCueSlots[i] != 0) {
            if (ServiceStyleCueIfNear(sStyleCueSlots[i], target, unused) == 0) {
                sStyleCueSlots[i] = FlushStyleCue(sStyleCueSlots[i]);
            }
            /* MATCHING: a no-op pair that gives target/i retail's registers */
            i++;
            i--;
        } else {
            sStyleCueSlots[i] = TryStartStyleCue(&sStyleCueSlotPool[i], &lastCue, target, unused);
        }
    }
    return lastCue;
}

extern s32 gStyleStage;
extern void RotateVramRectRight(DrawRect *rect, s32 count, DrawRect *scratch);
extern DrawRect gStyleStripRectA;
extern DrawRect gStyleStripScratchA;
extern DrawRect gStyleStripRectB;
extern DrawRect gStyleStripScratchB;

/* One step of RotateVramRectRight's one-column VRAM rotation: stage 2 on the
 * strip at y 496, stages 3..5 on the one at y 504. */
void StyleScrollVramStrips(void) {
    DrawRect *rect, *scratch;
    s32 count;

    if (gStyleStage == 2) {
        rect = &gStyleStripRectA;
        scratch = &gStyleStripScratchA;
        count = 1; /* MATCHING: a local set in each branch, not a literal argument */
    } else if ((u32)(gStyleStage - 3) < 3) {
        count = 1;
        rect = &gStyleStripRectB;
        scratch = &gStyleStripScratchB;
    } else {
        return;
    }
    RotateVramRectRight(rect, count, scratch);
}

/* ---- The style layer's sound cues; StyleEffect's lifecycle -------------
 *
 * Two groups, with one predicate between them:
 *
 *  - StyleCue00..StyleCue13, the 14 rows of sStyleCueCallbacks, and their
 *    helper ComputeStyleCueFalloff. Each is a SoundCueSet callback
 *    (include/SoundCueSet.h): TryStartStyleCue (previous section) starts a
 *    style-cue slot's embedded set with the claimed cue record's index as the
 *    tag and that row of the table as the callback, as Entity does with its
 *    Entity__MoodCueNN handlers. Every tick a callback sets the set's
 *    attenuation from the slot's distance to the target
 *    (ComputeStyleCueFalloff) and, on the ticks its pattern selects,
 *    requests VAB programs on the three voices; most restart the pattern by
 *    setting `tick` to -1 once it passes a limit.
 *  - IsStyleVariantEven: whether the variant PickStyleFallbackConfig chose
 *    (gStyleVariant) is even.
 *  - StyleEffect (include/StyleEffect.h), the Actor subclass the style layer
 *    keeps at an offset from its target: its ctor, finalize, reset
 *    (StyleEffect__SetParams) and update slot occupants, and the
 *    New_StyleEffect allocator. The per-kind work follows, in the next two
 *    sections.
 */

/* ------------------------------------------------------------------ *
 * sStyleCueCallbacks's 14 slots (StyleCue00..StyleCue13) plus the shared
 * helper ComputeStyleCueFalloff they all call first.
 * ------------------------------------------------------------------ */

/* The owner every StyleCueNN callback receives: one of the style layer's
 * style-cue slots (its `StyleCueSlot`, above, of which this is a local view).
 * TryStartStyleCue passes the slot as InitSoundCueSet's owner and its
 * embedded `cueSet` as the set, so a callback's `set` is `&ctx->cueSet`. */
typedef struct StyleCueParam StyleCueParam;

/* The cue-table record the slot claimed (`StyleCueEntryView`, above). */
typedef struct StyleCueParamMethods {
    u8 pad0[0x6];
    s8 cue; /* +0x006, the record's cue index (its sStyleCueCallbacks row
             * and InitSoundCueSet tag), negated while a slot has it
             * claimed; ComputeStyleCueFalloff indexes sStyleCueDistanceTable
             * with its negative. */
} StyleCueParamMethods;

struct StyleCueParam {
    StyleCueParamMethods *entry; /* +0x000 */
    u8 pad4[0x10 - 0x4];
    s32 lastDist;       /* +0x010, IsStyleCueNear's distance to the target */
    SoundCueSet cueSet; /* +0x014 */
};

/* Every callback calls this first; it is defined after them, in ROM order. */
s32 ComputeStyleCueFalloff(StyleCueParam *ctx);

void StyleCue00(StyleCueParam *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 7;
        set->slots[0].octave = 0;
    } else if (tick == 2) {
        set->slots[1].program = 7;
        set->slots[1].octave = 0;
    } else if (tick == 5) {
        set->slots[2].program = 7;
        set->slots[2].octave = 0;
    } else if (tick >= 8) {
        set->tick = -1;
    }
}

void StyleCue01(StyleCueParam *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 24;
        set->slots[0].octave = -2;
    } else if (tick >= 1025) {
        set->tick = -1;
    }
}

void StyleCue02(StyleCueParam *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 12;
        set->slots[0].octave = 2;
    } else if (tick >= 5) {
        set->tick = -1;
    }
}

void StyleCue03(StyleCueParam *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick % 20 == 0) {
        set->slots[0].program = 30;
        set->slots[0].vol = 32;
        set->slots[0].octave = 0;
        set->slots[0].endVol = 10;
    }
    if (set->tick % 400 == 0) {
        set->slots[1].program = 30;
        set->slots[1].octave = 0;
    }
    set->slots[2].program = 6;
    set->slots[2].vol = 32;
    set->slots[2].octave = 0;
    set->slots[2].endVol = 10;
}

void StyleCue04(StyleCueParam *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick % 3 == 0) {
        set->slots[0].program = 30;
        set->slots[0].octave = 0;
    }
    if (set->tick % 5 == 0) {
        set->slots[1].program = 30;
        set->slots[1].octave = 0;
        set->slots[1].vol = 24;
        set->slots[1].endVol = 24;
    }
    if (set->tick % 7 == 0) {
        set->slots[0].program = 30;
        set->slots[0].octave = 0;
    }
    set->slots[2].program = 6;
    set->slots[2].octave = 1;
    set->slots[2].vol = 42;
    set->slots[2].endVol = 10;
}

void StyleCue05(StyleCueParam *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick == 0) {
        set->slots[0].program = 30;
        set->slots[0].octave = -1;
    } else if (set->tick < 50 && set->tick % 5 == 4) {
        set->slots[1].program = 30;
        set->slots[1].octave = 0;
        set->slots[1].vol = set->slots[1].vol - set->tick * 2;
        set->slots[1].endVol = set->slots[1].vol;
    } else if (set->tick >= 101 && set->tick < 110) {
        set->slots[2].program = 13;
        set->slots[2].octave = 1;
    } else if (set->tick >= 201) {
        set->tick = -1;
    }
}

void StyleCue06(StyleCueParam *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 7;
        set->slots[0].octave = 2;
    } else if (tick >= 27) {
        set->tick = -1;
    }
}

void StyleCue07(StyleCueParam *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 20;
        set->slots[0].octave = 1;
    } else if (tick == 3) {
        set->slots[0].octave = 2;
        set->slots[0].vol = 24;
        set->slots[0].program = 3;
        set->slots[0].endVol = 20;
    } else if (tick >= 51) {
        set->tick = -1;
    }
}

void StyleCue08(StyleCueParam *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick % 20 == 0) {
        set->slots[0].program = 9;
        set->slots[0].octave = 0;
        set->slots[0].vol = 64;
        set->slots[0].endVol = 64;
    }
}

void StyleCue09(StyleCueParam *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick % 20 == 0) {
        set->slots[0].program = 9;
        set->slots[0].octave = -2;
    }
}

void StyleCue10(StyleCueParam *ctx, SoundCueSet *set) {
    s32 rem;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    rem = set->tick % 20;
    if (rem == 1) {
        set->slots[0].program = 9;
        set->slots[0].octave = -2;
    } else if (rem == 16) {
        set->slots[1].program = 9;
        set->slots[1].octave = -2;
    }
}

void StyleCue11(StyleCueParam *ctx, SoundCueSet *set) {
    s32 rem;

    StyleCue10(ctx, set);
    rem = set->tick % 70;
    if (rem == 50) {
        set->slots[2].program = 20;
        set->slots[2].octave = 1;
    } else if (rem >= 54 && rem < 59) {
        set->slots[2].program = 13;
        set->slots[2].octave = 1;
    } else if (rem == 61) {
        set->slots[2].program = 9;
        set->slots[2].octave = -1;
    }
}

void StyleCue12(StyleCueParam *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 20;
        set->slots[0].octave = -2;
        set->slots[1].program = 20;
        set->slots[1].octave = -2;
    } else if (tick == 4) {
        set->slots[1].program = 20;
        set->slots[1].octave = -2;
    } else if (tick == 20) {
        set->slots[0].program = 16;
        set->slots[0].octave = -2;
        set->slots[1].program = 18;
        set->slots[1].octave = -2;
    } else if (tick >= 201) {
        set->tick = -1;
    }
}

void StyleCue13(StyleCueParam *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick == 0) {
        set->slots[0].program = 24;
        set->slots[0].octave = 0;
    }
}

/* One range per cue record (15), indexed by the record's cue index: the
 * negative of `cue` while a slot has the record claimed. IsStyleCueNear
 * tests the slot's distance against the same row. */
extern s32 sStyleCueDistanceTable[];

s32 ComputeStyleCueFalloff(StyleCueParam *ctx) {
    s32 range = sStyleCueDistanceTable[-ctx->entry->cue];
    s32 stepDist = range / ctx->cueSet.attenuationSteps;

    return ctx->lastDist / stepDist;
}

extern s32 gStyleVariant;

s32 IsStyleVariantEven(void) {
    return (gStyleVariant & 1) ^ 1;
}

/* ------------------------------------------------------------------ *
 * StyleEffect's slot occupants (ctor, finalize, reset = SetParams, +0x0EC =
 * Update), plus its `New_` allocator. The class: include/StyleEffect.h.
 * ------------------------------------------------------------------ */

extern void *BMemPMgrAlloc(s32 size);
extern void *BMemPMgrFree(void *ptr);

StyleEffect *New_StyleEffect(s32 kind, StyleEffectParams *params, SceneNode *parent, LongVec3 *pos) {
    StyleEffect *self = BMemPMgrAlloc(sizeof(StyleEffect));

    if (self != NULL) {
        if (GetStyleEffectMethods()->ctor(self, kind, params, parent, pos) != NULL) {
            return self;
        }
        BMemPMgrFree(self);
        return NULL;
    }
    return NULL;
}

/* `kind` goes into Actor's pendingExtra (+0x054): see include/StyleEffect.h. */
StyleEffect *StyleEffect__StyleEffect(StyleEffect *self, s32 kind, StyleEffectParams *params,
                                      SceneNode *parent, LongVec3 *pos) {
    if (GetActorMethods()->ctor((Actor *)self) == NULL) {
        goto fail;
    }
    self->methods = GetStyleEffectMethods();
    self->state = 0;
    self->pendingExtra = kind;
    ((StyleEffectSetParamsFn)self->methods->reset)(self, params);
    StyleEffect__InitByKind(self, parent, pos);
    return self;
fail:
    return NULL;
}

/* Actor's finalize (SceneNode__Finalize) returns nothing, so neither does this. */
void StyleEffect__Finalize(StyleEffect *self) {
    StyleEffect__ReleaseByKind(self);
    GetActorMethods()->finalize((Actor *)self);
}

void StyleEffect__SetParams(StyleEffect *self, StyleEffectParams *params) {
    self->params = *params;
    self->tick = 0;
}

/* `pos` arrives from StyleUpdateEffectSlots and is forwarded untouched. */
void StyleEffect__Update(StyleEffect *self, LongVec3 *pos) {
    self->tick = self->tick + 1;
    StyleEffect__UpdateByKind(self, pos);
}

/* ---- StyleEffect's per-kind work (include/StyleEffect.h) ---------------
 *
 * The three switches on `kind` (StyleEffectKind) that its ctor, update and
 * finalize run: InitByKind (attach at pos + offset, link the model, build
 * the kind's children), UpdateByKind (follow pos + offset and the
 * viewport's viewpoint y, then the kind's per-frame step) and
 * ReleaseByKind. Then what they call for the model-row kind -- lay out,
 * drift and release the two model children -- and for the sprite kinds,
 * build the five sprites. Two small helpers every kind uses sit among
 * them: AddVec3 and AttachWithRotScale (attach, then set the rotation and
 * scale). The slot occupants themselves are in the previous section; the
 * rest of the sprite helpers, and SetStyleEffectSources, in the next.
 */

/* The class and its children: include/StyleEffect.h (the owner),
 * include/Actor.h (modelChildren) and include/VariantSprite.h (sprites). */

extern void ReleaseBasicClassArray(void **array, s32 count);
extern s32 sSpriteShiftX[];
extern Ratio16 sSpriteScaleLarge[3];
extern Ratio16 sSpriteScaleHalf[3];
extern Ratio16 sSpriteScaleSmall[3];
extern LongVec3 sSpriteShiftScratch;

void AddVec3(LongVec3 *dst, LongVec3 *a, LongVec3 *b);
void AttachWithRotScale(Actor *node, void *parent, void *trans, void *rotation, void *scale);

/* MATCHING: StyleEffect__SpawnPlainSprites, __RandomizeSprites,
 * __BuildRandomSprites and __DriftModelChildren read only `self`, but the
 * calls below pass a second, dead argument that retail loads, so
 * include/StyleEffect.h declares them without a prototype. NoOpIgnoreArgs
 * (next section, empty) is declared the same way here. */
extern void NoOpIgnoreArgs();

/* New_VariantSprite: include/VariantSprite.h. */

/* What SetStyleEffectSources (next section) recorded, declared there
 * with the same types: the DREAMER.TMD Actor the model kinds fetch their
 * model from (setBackClip), the TIM image New_VariantSprite is handed, and
 * the scene's Viewport, whose viewpoint y (refView.vp.y) InitByKind
 * snapshots into sStyleEffectBaseViewY and UpdateByKind follows. The
 * viewport stays `void *` because that is how the next section declares it. */
extern Actor *sStyleEffectTmd; /* the Actor SetStyleEffectSources ran on */
extern void *sStyleEffectTim;
extern Viewport *sStyleEffectViewport;
extern s32 sStyleEffectBaseViewY;
extern s32 sStyleEffectModelIds[];

/* Called once, from the class's ctor (StyleEffect__StyleEffect): snapshot the
 * viewpoint y, place self under `parent` at pos + offset, then build the
 * per-kind parts. The two model kinds link a model fetched from
 * sStyleEffectTmd by sStyleEffectModelIds[kind], and STYLE_EFFECT_MODEL_ROW
 * also gets its two model children; STYLE_EFFECT_SPRITES gets five
 * randomised sprites, STYLE_EFFECT_JITTER_SPRITES five plain ones
 * (StyleEffect__SpawnPlainSprites is StyleEffect__SpawnSprites(self, 0, 0,
 * NULL)). */
void StyleEffect__InitByKind(StyleEffect *self, SceneNode *parent, LongVec3 *pos) {
    LongVec3 placed;
    s32 kind;

    sStyleEffectBaseViewY = sStyleEffectViewport->refView.vp.y;
    AddVec3(&placed, pos, &self->params.offset);
    AttachWithRotScale((Actor *)self, parent, &placed, self->params.rotation, self->params.scale);

    kind = self->pendingExtra;
    if (kind <= STYLE_EFFECT_MODEL) {
        s32 model = sStyleEffectTmd->methods->setBackClip(sStyleEffectTmd, sStyleEffectModelIds[kind]);
        SceneNode__LinkModel((SceneNode *)self, (void *)model);
        kind = self->pendingExtra;
    }

    switch (kind) {
        case STYLE_EFFECT_MODEL_ROW:
            StyleEffect__PlaceModelChildren(self, 0);
            break;
        case STYLE_EFFECT_SPRITES:
            StyleEffect__BuildRandomSprites(self, 0);
            break;
        case STYLE_EFFECT_JITTER_SPRITES:
            StyleEffect__SpawnPlainSprites(self, 0);
            break;
        default:
            break;
    }
}

/* Called every frame from the class's slot +0x0EC (StyleEffect__Update, right
 * after it increments `tick`): set self's translation (Actor's
 * setTranslation) to pos + offset, plus however far the viewpoint y has moved
 * since StyleEffect__InitByKind snapshotted it, then run the
 * per-kind update. */
void StyleEffect__UpdateByKind(StyleEffect *self, LongVec3 *pos) {
    LongVec3 placed;

    AddVec3(&placed, pos, &self->params.offset);
    placed.y += sStyleEffectViewport->refView.vp.y - sStyleEffectBaseViewY;
    self->methods->setTranslation(self, &placed);

    switch (self->pendingExtra) {
        case STYLE_EFFECT_MODEL_ROW:
            StyleEffect__DriftModelChildren(self, pos);
            break;
        case STYLE_EFFECT_SPRITES:
            NoOpIgnoreArgs(self, pos);
            break;
        case STYLE_EFFECT_JITTER_SPRITES:
            StyleEffect__RandomizeSprites(self, pos);
            break;
        default:
            break;
    }
}

/* Called from the class's dtor (StyleEffect__Finalize): release whichever child
 * array this kind built (both sprite kinds release `sprites`, through two
 * identical functions in the next section). */
void StyleEffect__ReleaseByKind(StyleEffect *self) {
    switch (self->pendingExtra) {
        case STYLE_EFFECT_MODEL_ROW:
            StyleEffect__ReleaseModelChildren(self);
            break;
        case STYLE_EFFECT_SPRITES:
            StyleEffect__ReleaseSprites(self);
            break;
        case STYLE_EFFECT_JITTER_SPRITES:
            StyleEffect__ReleaseJitterSprites(self);
            break;
        default:
            break;
    }
}

/* Plain Vec3 add: dst = a + b. Frameless -- no self/vtable involved. */
void AddVec3(LongVec3 *dst, LongVec3 *a, LongVec3 *b) {
    dst->x = a->x + b->x;
    dst->y = a->y + b->y;
    dst->z = a->z + b->z;
}

/* Attach `node` under `parent` at translation `trans`, then assign (set = 1)
 * its rotation and scale. Used on the owner itself and on each model child. */
void AttachWithRotScale(Actor *node, void *parent, void *trans, void *rotation, void *scale) {
    node->methods->attachToParent(node, parent, trans);
    node->methods->updateRotation(node, 1, rotation);
    node->methods->updateScale(node, 1, scale);
}

/* Lay the two model children out in a row: child i sits at (i + 1) *
 * sModelChildSpacing[modelChildLayout], along x (scaled by scale's x
 * numerator) for layouts 1-2 and along y for 3-4. reuse = 0 creates them
 * (New_Actor, sharing the owner's model, attached to the owner);
 * reuse = 1 only resets their translation (setTranslation). */
extern LongVec3 sModelChildOffsetInit;
extern s32 sModelChildSpacing[];

void StyleEffect__PlaceModelChildren(StyleEffect *self, s32 reuse) {
    LongVec3 childPos;
    Actor **slot;
    s32 i;
    s32 layout = self->params.modelChildLayout;

    if (layout == 0) {
        return;
    }
    childPos = sModelChildOffsetInit;
    slot = self->modelChildren;
    for (i = 0; i < ARRAY_COUNT(self->modelChildren); i++, slot++) {
        if (layout < 3) {
            childPos.x += self->params.scale[0].num * sModelChildSpacing[layout];
        } else {
            childPos.y += sModelChildSpacing[layout];
        }
        if (reuse) {
            Actor *child = *slot;
            child->methods->setTranslation(child, &childPos);
        } else {
            Actor *child = New_Actor();
            *slot = child;
            SceneNode__LinkModel((SceneNode *)child, self->model);
            AttachWithRotScale(*slot, self, &childPos, self->params.rotation, self->params.scale);
        }
    }
}

/* Per-`tableIndex` z step for the model children (0 = no drift), also the
 * divisor of MODEL_CHILD_DRIFT_RANGE for the reset period below. Same index space as
 * sSpriteShiftX. */
extern s32 sModelChildDriftZ[];
/* All-zero LongVec3, the start value of each child's per-frame z delta. */
extern LongVec3 sModelChildDriftInit;
/* Ratio triple {0/1, 1/10, 0/1}: the per-frame rotation increment
 * updateRotation(.., 0, ..) adds to self and to each model child. */
extern Ratio16 sSpinRotStep[3];

/* Ticks (StyleEffect::tick) before the model children start to drift. */
#define MODEL_CHILD_DRIFT_DELAY 500
/* The z distance a child drifts before it snaps back: the reset period is
 * this over the child's per-tick step, sModelChildDriftZ[tableIndex]. */
#define MODEL_CHILD_DRIFT_RANGE 24500

/* Once tick passes MODEL_CHILD_DRIFT_DELAY, for a layout with model
 * children and a nonzero sModelChildDriftZ step: spin self and both
 * children, move child i along z by step + 3 * i, and every
 * MODEL_CHILD_DRIFT_RANGE / step ticks (the period's magnitude, whatever
 * the step's sign) snap them back to their layout. Always marks self's
 * coord2 for recompute. */
void StyleEffect__DriftModelChildren(StyleEffect *self) {
    s32 tableIndex;
    s32 extraZ;
    s32 period;
    s32 tick;
    Actor **slot;
    s32 i;
    s32 *stepZ;

    tableIndex = self->params.tableIndex;
    if (self->params.modelChildLayout != 0 && sModelChildDriftZ[tableIndex] != 0 &&
        (u32)self->tick > MODEL_CHILD_DRIFT_DELAY) {
        slot = self->modelChildren;
        self->methods->updateRotation(self, 0, sSpinRotStep);
        i = 0;
        /* MATCHING: the guard reads the step from the table and the pointer
         * is taken only here, after the call; either held earlier in a
         * local swaps two registers. */
        stepZ = &sModelChildDriftZ[tableIndex];
        extraZ = 0;
        for (; i < ARRAY_COUNT(self->modelChildren); i++) {
            LongVec3 delta = sModelChildDriftInit;
            delta.z += extraZ + *stepZ;
            (*slot)->methods->addTranslation(*slot, &delta);
            extraZ += 3;
            (*slot)->methods->updateRotation(*slot, 0, sSpinRotStep);
            slot++;
        }

        period = MODEL_CHILD_DRIFT_RANGE / sModelChildDriftZ[tableIndex];
        tick = self->tick;
        if (period >= 0) {
            if ((u32)tick % (u32)period == 0) {
                StyleEffect__PlaceModelChildren(self, 1);
            }
        } else {
            u32 absPeriod = ~period + 1;
            if ((u32)tick % absPeriod == 0) {
                StyleEffect__PlaceModelChildren(self, 1);
            }
        }
    }
    self->coord2->flg = 0;
}

/* Release the two model children, if this layout made any. */
void StyleEffect__ReleaseModelChildren(StyleEffect *self) {
    if (self->params.modelChildLayout != 0) {
        ReleaseBasicClassArray((void **)self->modelChildren, ARRAY_COUNT(self->modelChildren));
    }
}

/* Kind 2's init: five sprites, all scaled by sSpriteScaleHalf on an even
 * rand(); then sprites[1] is either shifted along x by
 * sSpriteShiftX[tableIndex] and recoloured (tableIndex >= 2) or made
 * semi-transparent (rate 0) and rescaled, and sprites[2] is hidden. */
void StyleEffect__BuildRandomSprites(StyleEffect *self) {
    s32 parity = rand() % 2;
    void *scale = parity ? NULL : sSpriteScaleHalf;
    VariantSprite *sprite;
    SpriteRgb *color;

    StyleEffect__SpawnSprites(self, 0, 0, scale);

    if (self->params.tableIndex >= 2) {
        VariantSpriteMethods *methods;

        sprite = self->sprites[1];
        sSpriteShiftScratch.x = sSpriteShiftX[self->params.tableIndex];
        /* Called directly, not through the sprite's table: a VariantSprite
         * is a Sprite, not an Actor, and the function only touches the
         * SceneNode coord2 both share. */
        Actor__AddTranslation((Actor *)sprite, &sSpriteShiftScratch);
        methods = sprite->methods;
        color = (self->params.altColor != NULL) ? self->params.altColor : self->params.color;
        methods->setColor(sprite, color);
    } else {
        sprite = self->sprites[1];
        sprite->methods->setSemiTransOn(sprite, 1);
        sprite->methods->setSemiTransRate(sprite, 0);
        sprite->methods->updateScale(sprite, 1, (parity != 0) ? sSpriteScaleLarge : sSpriteScaleSmall);
    }

    self->sprites[2]->methods->setDisplay(self->sprites[2], 0);
}

/* Create the five sprites (New_VariantSprite), attach each to self at no offset,
 * give each self's colour (Sprite's setColor sets GsSPRITE r,g,b), and
 * assign `scale` as their scale when non-NULL. `self` stays `void *`: it is
 * the prototype the next section calls through, and a typed local alias of
 * it costs a callee-saved register (see this function's report). */
void StyleEffect__SpawnSprites(void *self, s32 unused, s32 variant, void *scale) {
    VariantSprite **slot = ((StyleEffect *)self)->sprites;
    VariantSprite *sprite;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(((StyleEffect *)self)->sprites); i++, slot++) {
        sprite = New_VariantSprite(variant, 0, sStyleEffectTim);
        *slot = sprite;
        sprite->methods->attachToParent(sprite, self, 0);
        (*slot)->methods->setColor(*slot, ((StyleEffect *)self)->params.color);
        if (scale != 0) {
            (*slot)->methods->updateScale(*slot, 1, scale);
        }
    }
}

/* ---- StyleEffect's sprite helpers and getter; Actor, first half --------
 *
 * The tail of StyleEffect's sprite helpers, then the first half of Actor's
 * own methods (the second half is the next section):
 *
 *  - StyleEffect (include/StyleEffect.h): the per-kind pieces for its two
 *    sprite kinds that StyleEffect__UpdateByKind and ReleaseByKind
 *    (previous section) call. SpawnPlainSprites builds the five sprites,
 *    RandomizeSprites re-shapes four of them every frame, NoOpIgnoreArgs is
 *    the empty per-frame step, and ReleaseSprites / ReleaseJitterSprites are two
 *    identical functions that release them. Behind them, by address, sit the
 *    class's table getter and SetStyleEffectSources, which records the
 *    TMD resource, TIM image and viewport every StyleEffect draws from.
 *  - Actor (include/Actor.h), the base of TodActor, DreamSys and
 *    StyleEffect: New_Actor and the constructor, the child bookkeeping that
 *    keeps the grid manager and the frame clock in `grid` and `ticker`,
 *    Reset, NotifyMove (the hull sweep sent after a move),
 *    DispatchLinkCommand, and the translation setters that end in
 *    MoveLocalZ.
 */

/* STYLE_EFFECT_SPRITES' per-frame step: nothing. Its caller passes
 * (self, pos), which it does not read. */
void NoOpIgnoreArgs(void) {}

/* ------------------------------------------------------------------ *
 * StyleEffect's sprite kinds (STYLE_EFFECT_SPRITES, _JITTER_SPRITES).
 * ------------------------------------------------------------------ */

extern void ReleaseBasicClassArray(void **array, s32 count);

/* Six scale tables, each three Ratio16s (x, y, z), for the jittering
 * sprites: a thin streak along y or along x, {1/16, 7/1}, {7/1, 1/16}, then
 * the same with 3 and 2; z is 1/1. VariantSprite__UpdateScale reads x and y. */
extern Ratio16 sStyleEffectJitterScales[6][3];

/* Kind 2's release; ReleaseJitterSprites, kind 3's, is the same body. */
void StyleEffect__ReleaseSprites(StyleEffect *self) {
    ReleaseBasicClassArray((void **)self->sprites, ARRAY_COUNT(self->sprites));
}

/* Five sprites of variant 0 at their default scale. */
void StyleEffect__SpawnPlainSprites(StyleEffect *self) {
    StyleEffect__SpawnSprites(self, 0, 0, 0);
}

/* STYLE_EFFECT_JITTER_SPRITES' per-frame step: every sprite but the first
 * takes a random streak shape and a random whole-degree rotation. */
void StyleEffect__RandomizeSprites(StyleEffect *self) {
    VariantSprite **sprite = &self->sprites[1];
    s32 i;

    for (i = 0; i < ARRAY_COUNT(self->sprites) - 1; i++, sprite++) {
        u32 pick = rand();

        (*sprite)->methods->updateScale(
            *sprite, 1, sStyleEffectJitterScales[pick % ARRAY_COUNT(sStyleEffectJitterScales)]);
        (*sprite)->sprite.rotate = (rand() % 360) * ONE; /* 4096ths of a degree */
    }
}

void StyleEffect__ReleaseJitterSprites(StyleEffect *self) {
    ReleaseBasicClassArray((void **)self->sprites, ARRAY_COUNT(self->sprites));
}

/* StyleEffect's table getter. */
StyleEffectMethods *GetStyleEffectMethods(void) {
    return &gStyleEffectMethods;
}

/* What StyleEffect's methods (previous sections, which declare the same
 * globals) draw from: the scene's TMD resource, its TIM image and the
 * viewport. The TMD resource's getModel slot sits where Actor has
 * setBackClip, hence the Actor view. */
extern Actor *sStyleEffectTmd;
extern void *sStyleEffectTim;
extern Viewport *sStyleEffectViewport;
extern s32 sStyleEffectModelIds[3];
extern s16 sStyleEffectClutPos[2];

extern void TmdModel__SetFirstPrimClut(TmdModel *self, s16 *xy);

/* Records the three sources, then points the first primitive of the TMD's
 * models 0 and 2 (sStyleEffectModelIds) at the CLUT at sStyleEffectClutPos. */
void SetStyleEffectSources(s32 unused, Actor *tmd, s32 tim, s32 viewport) {
    s32 i;
    TmdModel *model;

    sStyleEffectTmd = tmd;
    sStyleEffectTim = (void *)tim;
    sStyleEffectViewport = (Viewport *)viewport;
    i = 0;
    do {
        model = (TmdModel *)tmd->methods->setBackClip(tmd, sStyleEffectModelIds[i]);
        TmdModel__SetFirstPrimClut(model, sStyleEffectClutPos);
        i++;
    } while (i < 2);
}

extern void *BMemPMgrAlloc(s32 size);
extern void *BMemPMgrFree(void *ptr);

void *New_Actor(void) {
    Actor *self = BMemPMgrAlloc(sizeof(Actor));

    if (self != NULL) {
        if (GetActorMethods()->ctor(self) != NULL) {
            return self;
        }
        BMemPMgrFree(self);
        return NULL;
    }
    return NULL;
}

Actor *Actor__Actor(Actor *self) {
    if (GetSceneNodeMethods()->ctor((SceneNode *)self) == NULL) {
        goto fail;
    }
    self->methods = GetActorMethods();
    self->state = 0;
    self->grid = NULL;
    self->ticker = NULL;
    self->methods->reset(self);
    return self;
fail:
    return NULL;
}

/* addChild, removeChild and removeAllChildren chain SceneNode's and keep
 * two companions: a StageMap child (class id 0x114, three nibbles) in
 * `grid`, a FrameClock child in `ticker`. */
void Actor__AddChild(Actor *self, BasicClass *child) {
    s32 classId;

    GetSceneNodeMethods()->addChild((SceneNode *)self, child);
    classId = child->methods->header;
    if ((classId & 0xFFF) == STAGEMAP_CLASS_ID) {
        self->grid = (struct StageMap *)child;
    } else if ((classId & CLASS_ID_ROOT_MASK) == FRAMECLOCK_CLASS_ID) {
        self->ticker = child;
    }
}

void Actor__RemoveChild(Actor *self, BasicClass *child) {
    s32 classId = child->methods->header;

    if ((classId & 0xFFF) == STAGEMAP_CLASS_ID) {
        self->grid = NULL;
    } else if ((classId & CLASS_ID_ROOT_MASK) == FRAMECLOCK_CLASS_ID) {
        self->ticker = NULL;
    }
    GetSceneNodeMethods()->removeChild((SceneNode *)self, child);
}

void Actor__RemoveAllChildren(Actor *self) {
    self->grid = NULL;
    self->ticker = NULL;
    GetSceneNodeMethods()->removeAllChildren((SceneNode *)self);
}

/* The distance NotifyMove stretches the hull by until a move or
 * setLastOffsetValue sets one; no extra. */
void Actor__Reset(Actor *self) {
    self->lastOffsetValue = 300;
    self->pendingExtra = 0;
}

extern void RotateAndOffsetHullList(TmdHull *hull, s32 turn, s32 back, s32 delta);

/* notifyWithHull: SceneNode's, then, for events ACTOR_EVENT_UNSWEPT to
 * ACTOR_EVENT_MOVED_Y on a model with bounds, the model's hull goes to the
 * parents through transformAndNotifyParents. For a move it is first
 * stretched: one face pushed out by the last move's distance plus
 * pendingExtra, an x face for ACTOR_EVENT_MOVED_X (RotateAndOffsetHullList
 * turns the box a quarter first), a z face otherwise; the max face after a
 * forward move, the min face after a backward one. An Actor linkTarget then
 * gets slotE8, which every Actor class leaves empty. */
void Actor__NotifyMove(Actor *self, s32 event) {
    GetSceneNodeMethods()->notifyWithHull((SceneNode *)self, event);
    /* MATCHING: two nested ifs; `&&` folds into one unsigned compare */
    if (event <= ACTOR_EVENT_MOVED_Y) {
        if (event >= ACTOR_EVENT_UNSWEPT) {
            TmdHull hull;

            if (self->model != NULL && TmdModel__GetBoundsCount(self->model)) {
                self->methods->getModelHull(self, &hull);
                if (event != ACTOR_EVENT_UNSWEPT) {
                    s16 offset = self->lastOffsetValue;
                    s32 alongX = (event == ACTOR_EVENT_MOVED_X);
                    s32 forward = (offset >= 0);
                    s32 delta;

                    /* MATCHING: goto, not if/else, for retail's branch order */
                    if (offset < 0) {
                        goto backward;
                    }
                    delta = offset + self->pendingExtra;
                    goto offsetHull;
                backward:
                    delta = offset - self->pendingExtra;
                offsetHull:
                    RotateAndOffsetHullList(&hull, alongX, forward, delta);
                }
                self->methods->transformAndNotifyParents(self, &hull, event);
                if (self->linkTarget != NULL) {
                    if ((u8)self->linkTarget->methods->header == ACTOR_CLASS_ID) {
                        ((Actor *)self->linkTarget)->methods->slotE8((Actor *)self->linkTarget);
                    }
                }
            }
        }
    }
}

/* Routes a link command by the sender's class id byte: from an Actor (or
 * any class below it) to onActorLinkCommand, from a GridCell to
 * onGridCellLinkCommand, from anything else nowhere. */
void Actor__DispatchLinkCommand(Actor *self, BasicClass *sender, s32 event) {
    if ((u8)sender->methods->header == ACTOR_CLASS_ID) {
        self->methods->onActorLinkCommand(self, sender, event);
    } else if ((u8)sender->methods->header == GRIDCELL_CLASS_ID) {
        self->methods->onGridCellLinkCommand(self, sender, event);
    }
}

void Actor__SetTranslation(Actor *self, LongVec3 *v) {
    Actor__UpdateTranslation(self, 1, v);
}

void Actor__AddTranslation(Actor *self, LongVec3 *delta) {
    Actor__UpdateTranslation(self, 0, delta);
}

/* Sets (set != 0) or adds to the offset from the parent, coord2->coord.t,
 * then clears coord2->flg so libgs recomputes the matrix. */
void Actor__UpdateTranslation(Actor *self, s32 set, LongVec3 *v) {
    Actor *actor = self; /* MATCHING: a second name for self; without it $a0 is used, not $t0 */
    GsCOORDINATE2 *coord = actor->coord2;

    if (set) {
        *(LongVec3 *)coord->coord.t = *v; /* MATCHING: one struct copy, loads before stores */
    } else {
        coord->coord.t[0] += v->x;
        coord->coord.t[1] += v->y;
        coord->coord.t[2] += v->z;
    }
    actor->coord2->flg = 0;
}

/* Moves by `local` turned by the actor's own rotation. */
void Actor__AddLocalTranslation(Actor *self, s16 *local) {
    LongVec3 delta;

    SceneNode__RotateLocalVector((SceneNode *)self, &delta, local);
    self->methods->addTranslation(self, &delta);
}

/* The z of the s16 local move vector whose x and y are sActorLocalMove
 * (next section, with MoveLocalX/Y and MoveAlongLocalAxis). */
extern s16 sActorLocalMoveZ;

void Actor__MoveLocalZ(Actor *self, s32 val, void *notify) {
    Actor__MoveAlongLocalAxis(self, &sActorLocalMoveZ, val, notify, ACTOR_EVENT_MOVED_Z);
}

/* ---- Actor's movement and link search; VariantSprite's ctor -------------
 *
 * Actor's movement and link-search methods (include/Actor.h, occupants of
 * the base table gActorMethods, +0x0C8..+0x0EC), and VariantSprite's
 * allocator and ctor.
 *
 *  - Local-axis moves. Actor__MoveLocalX/Y put `val` into one component of
 *    the local move vector sActorLocalMove, apply it through
 *    addLocalTranslation and clear it again (Actor__MoveAlongLocalAxis;
 *    MoveLocalZ is in the previous section).
 *  - Move, else find a link. Actor__MoveLocalZOrFindLink/XOrFindLink clear
 *    linkTarget and move; when the move set no linkTarget,
 *    Actor__FindNearbyLink searches the StageMap grid round the actor's
 *    position for a GridCell whose model a vertical ray hits
 *    (BuildLinkQueries, ScanLinkCandidates, ScanGridWindow,
 *    AcceptGridElem), links to it and moves onto the hit.
 *  - The link-command pair (Actor__OnActorLinkCommand,
 *    Actor__OnGridCellLinkCommand): SceneNode's dispatchLinkCommand and,
 *    for an Actor sender's events 5..8, tryAttachNearby.
 *  - Actor__SetLastOffsetValue/SetPendingExtra and GetActorMethods.
 *  - New_VariantSprite and VariantSprite__VariantSprite, of an unrelated
 *    class (include/VariantSprite.h) that happens to follow in ROM.
 */

/* The local move vector's x and y (s16; the z, sActorLocalMoveZ, is the next
 * halfword, previous section). All three stay 0 between moves: a move sets
 * one component, addLocalTranslation rotates the whole vector by the
 * actor's orientation, and the component is cleared again. */
extern s16 sActorLocalMove[2];

void Actor__MoveLocalX(Actor *self, s32 val, void *notify) {
    Actor__MoveAlongLocalAxis(self, &sActorLocalMove[0], val, notify, ACTOR_EVENT_MOVED_X);
}

void Actor__MoveLocalY(Actor *self, s32 val, void *notify) {
    Actor__MoveAlongLocalAxis(self, &sActorLocalMove[1], val, notify, ACTOR_EVENT_MOVED_Y);
}

/* Moves the actor by `val` along one local axis (`axis` is that component of
 * sActorLocalMove), keeps `val` in lastOffsetValue and, when `notify` is
 * non-NULL, sends `event` (6, 7, 8 for z, x, y) through notifyWithHull. */
void Actor__MoveAlongLocalAxis(Actor *self, s16 *axis, s32 val, void *notify, s32 event) {
    s16 val16 = (s16)val; /* MATCHING: truncating at each store does not match */
    *axis = val16;
    self->lastOffsetValue = val16;
    self->methods->addLocalTranslation(self, &sActorLocalMove[0]);
    *axis = 0;
    if (notify != NULL) {
        self->methods->notifyWithHull(self, event);
    }
}

void Actor__MoveLocalZOrFindLink(Actor *self, s32 val, void *notify) {
    Actor__MoveOrFindNearbyLink(self, self->methods->moveLocalZ, val, notify);
}

void Actor__MoveLocalXOrFindLink(Actor *self, s32 val, void *notify) {
    Actor__MoveOrFindNearbyLink(self, self->methods->moveLocalX, val, notify);
}

void Actor__NoOpSlotD8(void) {}

/* Clears linkTarget, moves; if the move did not link, looks for a link. */
void Actor__MoveOrFindNearbyLink(Actor *self, void (*move)(Actor *, s32, void *), s32 val, void *notify) {
    self->linkTarget = NULL;
    move(self, val, notify);
    if (self->linkTarget == NULL) {
        Actor__FindNearbyLink(self);
    }
}

/* A window of one chunk slot's cells, in cells: from (startCol, startRow),
 * numCols to the right and numRows DOWN (to lower row indices;
 * Actor__ScanGridWindow). */
typedef struct GridQuery {
    s16 startCol;
    s16 startRow;
    s32 numCols;
    s32 numRows;
} GridQuery;

void *AcceptGridElem(void *cell, void *offset, void *pos);
s32 Actor__BuildLinkQueries(Actor *self, GridQuery *queries, ChunkSlot **slots,
                            Descriptor10Ext *desc, s32 span);
void *Actor__ScanLinkCandidates(Actor *self, void *offset, void *pos, s32 count, GridQuery *queries,
                                ChunkSlot **slots);

/* Looks in the actor's grid for a GridCell a vertical ray from the actor's
 * position hits: in the actor's own cell of its chunk slot and, in a
 * vertical grid, the same cell in the slots above and below
 * (BuildLinkQueries with span 1). On a hit the cell becomes linkTarget, the
 * actor moves by the ray's offset (addTranslation) and notifyWithHull gets
 * -1; with no hit, linkTarget is NULL and it gets -2. Returns whether it
 * linked; 0 as well when the actor has no grid or no slot holds its
 * position. */
s32 Actor__FindNearbyLink(Actor *self) {
    Descriptor10Ext desc;
    GridQuery queries[3];
    u8 pad48Tail[8]; /* MATCHING: retail leaves these 8 bytes between queries and slots */
    ChunkSlot *slots[3];
    LongVec3 offset;

    if (self->grid != NULL) {
        void *pos = self->coord2->coord.t;

        if (self->grid->methods->computeFootprintDescriptor(self->grid, &desc, pos) == 0) {
            s32 count = Actor__BuildLinkQueries(self, queries, slots, &desc, 1);
            void *result = Actor__ScanLinkCandidates(self, &offset, pos, count, queries, slots);

            self->linkTarget = result;
            if (result != NULL) {
                self->methods->addTranslation(self, &offset);
                self->methods->notifyWithHull(self, -1);
                return 1;
            }
            self->methods->notifyWithHull(self, -2);
            return 0;
        }
    }
    return 0;
}

/* Fills queries[]/slots[] with the cell windows to search round the cell
 * `desc` locates, and returns how many. An even span is rounded up to odd.
 * Span 1: the cell itself in desc's slot and, when the grid is vertical,
 * the same cell in the slots whose elemKey is one more and one less (as far
 * as the grid has rows), up to 3. A wider span: one window in desc's slot,
 * starting one column lower and one row higher than the cell, clipped at the
 * chunk's edges (FindNearbyLink, the one caller, passes 1). */
s32 Actor__BuildLinkQueries(Actor *self, GridQuery *queries, ChunkSlot **slots,
                            Descriptor10Ext *desc, s32 span) {
    s32 cellCol = desc->base.b2;
    s32 cellRow = desc->base.b3;
    s32 numCols;
    s32 numRows;
    s32 col;
    s32 row;
    s32 count;

    numRows = !(span & 1) ? ++span : span; /* MATCHING: the ternary, inverted, into numRows */
    numCols = numRows;
    col = cellCol;
    row = cellRow;
    count = 1;
    if (span == 1) {
        StageMap *map;
        StageGridDimensions *dims;
        ChunkSlot *slot;
        s16 slotKey;
        s32 key;

        queries[0].startCol = col;
        queries[0].startRow = row;
        queries[0].numCols = numRows;
        queries[0].numRows = numRows;
        slot = desc->slot;
        slots[0] = slot;
        map = self->grid;
        dims = map->config;
        if (dims->isVertical != 1) {
            return 1;
        }
        slotKey = slot->loader->elemKey;
        key = slotKey + 1;
        if (key < dims->rows) {
            slots[1] = map->methods->findSlotByNeighbour(map, key);
            count = 2; /* MATCHING: after the call, for the delay-slot fill */
            queries[1] = queries[0];
        }
        key = slotKey - 1;
        if (key >= 0) {
            slots[count] = map->methods->findSlotByNeighbour(map, key);
            queries[count] = queries[0];
            count++;
        }
        return count;
    }

    if (col == 0) {
        numCols = numRows - 1;
    } else {
        col--;
    }
    if (cellCol == STAGE_CHUNK_CELLS - 1) {
        numCols--;
    }
    if (cellRow == STAGE_CHUNK_CELLS - 1) {
        numRows--;
    } else {
        row++;
    }
    queries[0].startCol = col;
    if (cellRow == 0) {
        numRows--;
    }
    queries[0].startRow = row;
    queries[0].numCols = numCols;
    queries[0].numRows = numRows;
    slots[0] = desc->slot;
    return 1;
}

void *Actor__ScanGridWindow(Actor *self, void *offset, void *pos, GridQuery *query, ChunkSlot *slot);

/* Scans each of the `count` windows in its slot, skipping a slot whose
 * LbdFile header is not in yet; returns the first cell AcceptGridElem
 * accepts, or NULL. */
void *Actor__ScanLinkCandidates(Actor *self, void *offset, void *pos, s32 count, GridQuery *queries,
                                ChunkSlot **slots) {
    s32 i;

    for (i = 0; i < count;) {
        ChunkSlot *slot = *slots;
        i++; /* MATCHING: here, not in the for header */
        if (slot->loader->headerReady != 0) {
            void *result = Actor__ScanGridWindow(self, offset, pos, queries, slot);
            if (result != NULL) {
                return result;
            }
        }
        queries++;
        slots++;
    }
    return NULL;
}

/* Tries every cell of the window, each lattice cell and then the cells
 * chained from it (nextInCell), and returns the first AcceptGridElem
 * accepts, or NULL. `self` is not used. */
void *Actor__ScanGridWindow(Actor *self, void *offset, void *pos, GridQuery *query, ChunkSlot *slot) {
    s32 row, col;
    GridCell **bucket;

    bucket = slot->cells + query->startRow * STAGE_CHUNK_CELLS + query->startCol;
    for (row = 0; row < query->numRows; row++) {
        for (col = 0; col < query->numCols; col++) {
            GridCell *node;

            /* MATCHING: *bucket re-read at each use; a local costs a register */
            if (AcceptGridElem(*bucket, offset, pos) != NULL) {
                return *bucket;
            }
            for (node = (*bucket)->nextInCell; node != NULL; node = node->nextInCell) {
                if (AcceptGridElem(node, offset, pos) != NULL) {
                    return node;
                }
            }
            bucket++;
        }
        bucket -= query->numCols + STAGE_CHUNK_CELLS;
    }
    return NULL;
}

/* SceneNode.c: casts a vertical ray from `pos` against the node's model,
 * one way and then the other; on a hit writes the hit less the ray's start
 * to `offset` and returns 1. */
extern s32 SceneNode__RaycastVertical(void *self, void *offset, void *pos);

/* `cell` if it is non-NULL and a vertical ray from `pos` hits its model
 * (the offset to the hit into `offset`), else NULL. */
void *AcceptGridElem(void *cell, void *offset, void *pos) {
    if (cell != NULL) {
        if (SceneNode__RaycastVertical(cell, offset, pos) != 0) {
            return cell;
        }
    }
    return NULL;
}

/* gActorMethods +0x0DC: a link command from another Actor. SceneNode's
 * handling, then, for events 5..8, tryAttachNearby with (self, sender,
 * event): SceneNode's slot declares self alone, hence the cast. */
void Actor__OnActorLinkCommand(Actor *self, void *sender, s32 event) {
    GetSceneNodeMethods()->dispatchLinkCommand((SceneNode *)self, sender, event);
    if (event < 9) {
        if (event >= 5) { /* MATCHING: nested, as && folds to one unsigned test */
            ((void (*)(Actor *, void *, s32))self->methods->tryAttachNearby)(self, sender, event);
        }
    }
}

/* gActorMethods +0x0E0: a link command from a GridCell; SceneNode's handling. */
void Actor__OnGridCellLinkCommand(Actor *self, void *sender, s32 event) {
    GetSceneNodeMethods()->dispatchLinkCommand((SceneNode *)self, sender, event);
}

void Actor__SetLastOffsetValue(Actor *self, s16 val) {
    self->lastOffsetValue = val;
}

void Actor__NoOpSlotE8(void) {}

void Actor__SetPendingExtra(Actor *self, s32 extra) {
    self->pendingExtra = extra;
}

ActorMethods *GetActorMethods(void) {
    return &gActorMethods;
}

extern void *BMemPMgrAlloc(s32 size);

/* VariantSprite's allocator; its other methods follow in the next two
 * sections. */
VariantSprite *New_VariantSprite(s32 variant, void *resetArg, void *texture) {
    void *obj = BMemPMgrAlloc(sizeof(VariantSprite));
    if (obj != NULL) {
        GetVariantSpriteMethods()->ctor(obj, variant, resetArg, texture);
        return obj;
    }
    return NULL;
}

/* VariantSprite's two texture cells, forwarded as the Sprite ctor's `rect`
 * (Sprite__Reset copies it into Sprite.rect): u,v = (0x00,0x20) and
 * (0x10,0x20), 16x16. */
extern SpriteRect gVariantSpriteCells[2];

/* Sprite's ctor with the variant's cell, then this class's table, and the
 * reset slot (VariantSprite__SetVariantClut) with the variant, through
 * VariantSpriteResetFn. Returns nothing: it ends in that call and sets no
 * $v0. */
void VariantSprite__VariantSprite(VariantSprite *self, s32 variant, void *resetArg, void *texture) {
    GetSpriteMethods()->ctor((Sprite *)self, texture, 0, &gVariantSpriteCells[variant], resetArg, 0);
    self->methods = GetVariantSpriteMethods();
    self->unkA4 = 0;
    ((VariantSpriteResetFn)self->methods->reset)(self, variant);
}

/* ---- VariantSprite (include/VariantSprite.h) ---------------------------
 *
 * A Sprite whose variant, 0 or 1, picks its texture cell and CLUT. Its ctor
 * and allocator are just above, the empty leaves and the table getter at the
 * head of the next section.
 *
 * - VariantSprite__SetVariantClut (reset, +0x040, called last by the ctor
 *   with the variant): records the variant and points the GsSPRITE's CLUT
 *   at that variant's row, replacing the one Sprite's reset took from the
 *   texture.
 * - VariantSprite__UpdateScale (updateScale, +0x048): two num/den ratios
 *   into GsSPRITE scalex/scaley.
 */


/*
 * The two variants' CLUT positions, one {x, y} table in VRAM:
 * {976, 511} and {992, 511}, adjacent 16-colour rows on the bottom line.
 * MATCHING: two externs, as retail takes one %hi/%lo base for x, one for y.
 */
extern const s16 gVariantSpriteClutX[];
extern const s16 gVariantSpriteClutY[];

/* One {x, y} entry of that table, in s16s: the stride both lookups index by. */
#define VARIANT_CLUT_STRIDE 2

void VariantSprite__SetVariantClut(VariantSprite *self, s32 variant) {
    self->variant = variant;
    self->sprite.cx = gVariantSpriteClutX[variant * VARIANT_CLUT_STRIDE];
    self->sprite.cy = gVariantSpriteClutY[variant * VARIANT_CLUT_STRIDE];
}

/*
 * Overrides SceneNode__UpdateScale. `ratios` is two num/den pairs, x then y
 * (the callers' tables hold three, sSpriteScaleLarge's {6,5} and
 * sSpriteScaleSmall's {4,6}; the third is not read), each turned into 20.12
 * by the split division RatioToFixed12 uses. The ratios go to the GsSPRITE's
 * scalex/scaley, or, while Sprite's accumulateScale is set, multiply accumScaleX/Y
 * instead. `set` is not read.
 */
void VariantSprite__UpdateScale(VariantSprite *self, s32 set, Ratio16 *ratios) {
    s32 xWhole, xRem, xFrac, xRatio;
    s32 yWhole, yRem, yFrac, yRatio;
    s16 xScale, yScale;

    xWhole = ratios[0].num / ratios[0].den;
    xRem = ratios[0].num % ratios[0].den;
    xFrac = (xRem << FIX12_SHIFT) / ratios[0].den;
    xRatio = (xWhole << FIX12_SHIFT) + xFrac;
    xScale = (s16)xRatio; /* MATCHING: here; in the else arm it loses a move */

    yWhole = ratios[1].num / ratios[1].den;
    yRem = ratios[1].num % ratios[1].den;
    yFrac = (yRem << FIX12_SHIFT) / ratios[1].den;
    yRatio = (yWhole << FIX12_SHIFT) + yFrac;
    yScale = (s16)yRatio;

    if (self->accumulateScale != 0) {
        self->accumScaleX = ((s16)xRatio * self->accumScaleX) >> FIX12_SHIFT;
        self->accumScaleY = ((s16)yRatio * self->accumScaleY) >> FIX12_SHIFT;
    } else {
        self->sprite.scalex = xScale;
        self->sprite.scaley = yScale;
    }
}

/* ---- VariantSprite's tail; GraphRoom -----------------------------------
 *
 * - VariantSprite: four empty methods and the table getter.
 * - GraphRoom (include/GraphRoom.h, whose banner has the slots and fields),
 *   a TaskCore subclass, whole: allocator, ctor, every override, ScoreDayLog
 *   and the getter.
 *
 * GraphRoom loads "ETC\HGRAPH.TIM" and plots up to 100 days of the
 * DreamSys's mood ring (moodPreviousDays, read from the save block
 * DreamSys__GetSaveBlock returns), newest first, as 10-pixel BoxFill dots:
 * a day's two signed mood bytes, times 10, are its dot's centre, the upper
 * axis pointing up the screen. The newest dot is red and blinks while
 * inputMode is 1; the others fade from white. ScoreDayLog checks, once per
 * save, that four fixed moods (sGraphScoreMoods) all appear among the
 * plotted days, and TickHighlight then turns each one's dot green, one
 * every 24 frames once frameCounter passes 30.
 */

extern void *BMemPMgrAlloc(s32 size);

/* sGraphScoreMoods' length: ScoreDayLog's targets, one matchedDayIndices
 * byte and one highlight each. */
#define GRAPH_SCORE_MOOD_COUNT 4

/* A plotted dot's side, sGraphPointSize's {10, 10}: PopulateGraphPoints
 * subtracts half of it so each dot is centred on its mood. */
#define GRAPH_POINT_SIZE 10

/* Screen pixels per step of a mood axis (PopulateGraphPoints). */
#define GRAPH_PIXELS_PER_MOOD 10

/* VariantSprite's (include/VariantSprite.h) four empty leaves and its table
 * getter. VariantSprite__Update is the +0x098 update override of
 * Sprite__Update, typed as that slot; the other three occupy the class's own
 * slots +0x0BC/+0x0C0/+0x0C4, which nothing calls. */
void VariantSprite__Update(VariantSprite *self, void *sender, s32 event) {}

void VariantSprite__NoOpSlotBC(void) {}

void VariantSprite__NoOpSlotC0(void) {}

void VariantSprite__NoOpSlotC4(void) {}

VariantSpriteMethods *GetVariantSpriteMethods(void) {
    return &gVariantSpriteMethods;
}

/* GraphRoom's object, table and methods: include/GraphRoom.h. The base
 * implementations are reached through GetTaskCoreMethods() with `self`
 * upcast. */

/* DreamSaveBlock, the save block GraphRoom plots, is include/DreamSys.h's. */

GraphRoom *New_GraphRoom(struct DreamSys *dreamSys) {
    GraphRoom *obj = BMemPMgrAlloc(sizeof(GraphRoom));
    if (obj != NULL) {
        GetGraphRoomMethods()->ctor(obj, dreamSys);
        return obj;
    }
    return NULL;
}

extern char sGraphSoundBankPath[];

void GraphRoom__GraphRoom(GraphRoom *self, struct DreamSys *dreamSys) {
    GetTaskCoreMethods()->ctor((TaskCore *)self, NULL, sGraphSoundBankPath, NULL);
    self->methods = GetGraphRoomMethods();
    ((VabStreamObj *)self->sound)->methods->setPitchOffset((VabStreamObj *)self->sound, -1); /* TaskCore::sound is a VabStreamObj */
    self->dreamSys = dreamSys;
    self->methods->setTarget(self, NULL);
    ((GraphRoomResetCallFn)self->methods->resetCounters)(self, dreamSys);
}

extern char sGraphTimPath[];

void GraphRoom__Reset(GraphRoom *self) {
    self->fadeRate = 5;
    self->unk2C = 400;
    self->methods->setSubHandle(self, sGraphTimPath, NULL);
    self->methods->setFrameBound(self, 10);
}

void GraphRoom__Update(GraphRoom *self, BasicClass *sender, s32 event) {
    GetTaskCoreMethods()->update((TaskCore *)self, sender, event);
    if (self->inputMode == 1) {
        DreamSaveBlock *save =
            (DreamSaveBlock *)self->dreamSys->methods->getSaveBlock(self->dreamSys, 0);
        if (save->currentYear != 0 || save->currentDay != 0) {
            self->points[0]->methods->setDisplay(self->points[0], self->frameCounter & 1);
        }
    }
    self->methods->tickHighlight(self);
}

void GraphRoom__OnPadConfirm(GraphRoom *self) {
    if (self->scored == 0) {
        self->methods->playSound(self, 1 << 4); /* VAB program 1, tone 0 */
        self->methods->exit(self);
    }
}

/* A graph point's colour is New_BoxFill's colour argument, a BoxFillRgb
 * (include/BoxFill.h).
 * MATCHING: signed, and exactly three bytes -- the whole-struct copy of `rgb`
 * below is three lb/sb pairs. */
extern s32 sGraphPointSize[2];
extern BoxFillRgb sGraphPointNewestColor;
extern BoxFillRgb sGraphPointBaseColor;

void GraphRoom__BuildGraphPoints(GraphRoom *self) {
    BoxFillRgb rgb;
    s32 i;

    self->points[0] = New_BoxFill(sGraphPointSize, &sGraphPointNewestColor, 0);
    rgb = sGraphPointBaseColor;
    for (i = 1; i < ARRAY_COUNT(self->points); i++) {
        s32 step;

        self->points[i] = New_BoxFill(sGraphPointSize, &rgb, 0);
        step = 1;
        if (i < 7) {
            step = 20;
        }
        rgb.r -= step;
        rgb.g -= step;
        rgb.b -= step;
    }
    self->matchedDayIndices = BMemPMgrAlloc(GRAPH_SCORE_MOOD_COUNT * sizeof(s8));
}

void GraphRoom__ReleaseGraphPoints(GraphRoom *self) {
    s32 i;

    BMemPMgrFree(self->matchedDayIndices);
    for (i = 0; i < ARRAY_COUNT(self->points); i++) {
        self->points[i]->methods->release(self->points[i]);
    }
    GetTaskCoreMethods()->releaseTarget((TaskCore *)self);
}

s32 GraphRoom__Init(GraphRoom *self, IntermediateBaseInitArgs *args, s32 mode) {
    s32 result;
    GetTaskCoreMethods()->init((TaskCore *)self, args, mode);
    result = 2;
    if (self->scored == 0) {
        result = self->result;
    }
    return result;
}

void GraphRoom__PopulateGraphPoints(GraphRoom *self, void *parent) {
    DreamSaveBlock *save;
    s32 count;
    s32 i;
    s32 day;
    s32 haveNewest;
    BoxFillPos point;
    BoxFillPos firstPoint;

    GetTaskCoreMethods()->updateSlotElements((TaskCore *)self, parent);
    save = (DreamSaveBlock *)self->dreamSys->methods->getSaveBlock(self->dreamSys, 0);
    self->scored = GraphRoom__ScoreDayLog(self, save);

    haveNewest = 0;
    if (save->currentYear != 0) {
        count = ARRAY_COUNT(self->points);
    } else {
        count = save->currentDay;
        if (count > ARRAY_COUNT(self->points)) {
            count = ARRAY_COUNT(self->points);
        }
    }

    day = save->currentDay - 1;
    for (i = 0; i < count; i++, day--) {
        s8 dx, dy;
        s32 ndy;

        if (day < 0) {
            day = DAYS_PER_YEAR - 1;
        }
        /* MATCHING: indexed twice; through a `MoodGraphPoint *` to the day,
         * cc1 adds the array's +0x018 to the pointer first. */
        dx = save->moodPreviousDays[day].axis.dynamic;
        point.x = dx * GRAPH_PIXELS_PER_MOOD - GRAPH_POINT_SIZE / 2;
        dy = save->moodPreviousDays[day].axis.upper;
        ndy = -dy;
        point.y = ndy * GRAPH_PIXELS_PER_MOOD - GRAPH_POINT_SIZE / 2;

        if (i == 0) {
            firstPoint = point;
            haveNewest = 1;
        } else {
            self->points[i]->methods->attachAbsolute(self->points[i], parent, &point, 0);
        }
    }

    if (haveNewest) {
        self->points[0]->methods->attachAbsolute(self->points[0], parent, &firstPoint, 0);
    }
}

/* The four moods ScoreDayLog looks for, as (dynamic, upper): (-1, 1),
 * (1, 1), (0, 0), (0, -3). */
extern MoodGraphPoint sGraphScoreMoods[GRAPH_SCORE_MOOD_COUNT];

/* Whether every sGraphScoreMoods entry appears among the plotted days (the
 * window PopulateGraphPoints walks, newest first), recording in
 * matchedDayIndices the oldest dot holding each. Fails at once when
 * graphScored is set, and sets it on success.
 * MATCHING: the `targets`/`days` caches, the dead else branch and the
 * chained assignment are all inert; without any one, cc1 strength-reduces
 * sGraphScoreMoods[i] into a pointer hoisted across the outer loop. */
s32 GraphRoom__ScoreDayLog(GraphRoom *self, DreamSaveBlock *log) {
    u32 i;
    MoodGraphPoint *days;
    s32 dot;
    MoodGraphPoint *targets;
    s32 day;
    s32 matches;
    s32 limit;

    if (log->graphScored != 0) {
        goto fail;
    }

    if (log->currentYear != 0) {
        limit = ARRAY_COUNT(self->points);
    } else {
        limit = log->currentDay;
        if (limit > ARRAY_COUNT(self->points)) {
            limit = ARRAY_COUNT(self->points);
        }
    }

    for (i = 0; i < GRAPH_SCORE_MOOD_COUNT; i++) {
        matches = 0;
        day = log->currentDay - 1;
        for (dot = 0; dot < limit; dot++) {
            if (day < 0) {
                day = DAYS_PER_YEAR - 1;
            } else {
                targets = sGraphScoreMoods;
            }
            targets = (days = sGraphScoreMoods);
            days = log->moodPreviousDays;
            if (targets[i].value == days[day].value) {
                self->matchedDayIndices[i] = dot;
                matches++;
            }
            day--;
        }
        if (matches == 0) {
            goto fail;
        }
    }

    log->graphScored = 1;
    self->highlightCount = 0;
    return 1;

fail:
    return 0;
}

extern BoxFillRgb sGraphPointHighlightColor;

void GraphRoom__TickHighlight(GraphRoom *self) {
    if (self->scored != 0) {
        if ((u32)self->frameCounter >= 31) {
            if (self->highlightCount < GRAPH_SCORE_MOOD_COUNT) {
                if (((u32)self->frameCounter % 24) == 0) {
                    s8 dot = self->matchedDayIndices[self->highlightCount];
                    self->points[dot]->methods->setColor(self->points[dot], 1, &sGraphPointHighlightColor);
                    self->highlightCount += 1;
                }
            }
        }
    }
}

GraphRoomMethods *GetGraphRoomMethods(void) {
    return &gGraphRoomMethods;
}
