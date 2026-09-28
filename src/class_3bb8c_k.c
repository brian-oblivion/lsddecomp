/*
 * class_3bb8c_k -- the second half of ItemList's methods and the first of
 * ObjM's.
 *  - ItemList (include/ItemList.h), the list of strings the player picks one
 *    from: setState and tickClosing (close, then report the result to the
 *    parents), handleInputCode (the Pad events it answers) and playSound,
 *    the cursor and scroll methods, the four visible rows (createRows,
 *    releaseRows, refreshRows, and the non-virtual helpers FormatRowText and
 *    SetView), stepCursorInView, getCursorIndex and the table getter
 *    GetItemListMethods. Its ctor and resource methods are in class_3bb8c_i.
 *  - ObjM (include/ObjM.h): its allocator, ctor, finalize and onNotify,
 *    which dispatches on the sender's class id. The rest of ObjM is in
 *    class_3bb8c_l and class_3bb8c_m.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "class_3bb8c.h"
#include "class_39e08.h"
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
extern s32 gItemListRowOriginX;
extern s32 gItemListRowOriginY;

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

    pos.x = gItemListRowOriginX;
    pos.y = gItemListRowOriginY;
    count = self->itemCount;
    row = &self->rows[0];
    if (count > ARRAY_COUNT(self->rows)) {
        count = ARRAY_COUNT(self->rows);
    }

    for (i = 0; i < count; i++) {
        ItemList__FormatRowText(self, buf, i, top, column);
        *row = New_TextRow(font, ITEMLIST_ROW_CHARS, buf);
        (*row)->methods->attachToParent(*row, parent, (LongVec3 *)&pos);
        (*row)->methods->setColor(*row, &gItemListRowColor);
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
    row->methods->setColor(row, &gItemListCursorColor);
}

void ItemList__StepCursorInView(ItemList *self, s32 dir, s32 notify) {
    TextRow **row;
    s32 idx;

    if (!self->panelSprite) {
        return;
    }
    idx = self->cursorIndex - self->topIndex;
    row = &self->rows[idx]; /* MATCHING: one address, stepped, as retail */
    (*row)->methods->setColor(*row, &gItemListRowColor);
    if (dir) {
        self->cursorIndex++;
        row++;
    } else {
        self->cursorIndex--;
        row--;
    }
    (*row)->methods->setColor(*row, &gItemListCursorColor);
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

/* ---- merged from class_3bb8c_l ---- */

/*
 * class_3bb8c_l -- ObjM's methods from resetCounters (+0x040) through
 * enterState6 (+0x09C), in table order. The class is include/ObjM.h.
 *
 *  - init and deinit (AttachTarget, DetachTarget): install
 *    ObjM__GetGridRecord as the StageMap's chunk-record callback and keep
 *    the DreamSys as a child.
 *  - onInit and onDeinit (InitStyleAndWorld, TeardownStyle): pick the
 *    stage's BGM sequence and the day's TIM block, register the stage's
 *    StyleConfig, set the viewport's view and the StageMap's bounds; stop
 *    it all again.
 *  - onTag1Notify's event 2 runs PollTimBlockLoad: once the TIM block has
 *    loaded (or failed) the scene is set up, and once the StageMap has
 *    nothing pending the style session starts.
 *  - onPadEvent (DispatchPadEvent) maps Start, Select and triangle onto the
 *    pause and close slots; update ticks the style, or the pause overlay
 *    while it is being built; togglePause.
 *  - the style scene slots +0x080..+0x08C: SetupSceneStyle,
 *    ExitSceneStyle, EnterStyleSession, TickStyle.
 *  - OnDreamSysNotify turns the DreamSys's link codes into enterState4..B;
 *    EnterState4/5/6 set IntermediateBase::state and start a fade up
 *    (ObjM__StartFadeUp, class_3bb8c_m).
 * NoOpSlot40 and NoOpSlot7C are empty.
 *
 * include/class_3bb8c.h is shared with every other class_3bb8c_* unit;
 * edits to it are additive.
 */

void ObjM__NoOpSlot40(void) {}

/* init. `args` is the building DayTask's init args: args->lightRig is its
 * StageMap (IntermediateBase__Init keeps it as unk14), whose callback
 * becomes ObjM__GetGridRecord. */
void ObjM__AttachTarget(ObjM *self, IntermediateBaseInitArgs *args, DreamSys *dreamSys) {
    ((StageMap *)args->lightRig)
        ->methods->setCallback((StageMap *)args->lightRig, (ChunkFileFn)ObjM__GetGridRecord, self);
    self->dreamSys = dreamSys;
    GetTimedTaskMethods()->init((TimedTask *)self, args, 1);
    self->methods->addChild(self, (BasicClass *)dreamSys);
}

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

/* Defined elsewhere, no header: src/GameFiles.c (PickStageBgm and
 * PickStageTexture return a FilePathRecord *, a 0x1C-byte record handed on here as a
 * name), src/code_d294_c.c (GetSetHitHeightGate sets the flag
 * SceneNode__RaycastHullAgainstFaces tests). RegisterStyleConfig, which
 * keeps `sceneRefs` as gStyleSceneRefs, is defined below, after ObjM. */
extern s32 PickStageBgm(s32 stage, s32 unused);
extern s32 PickStageTexture(s32 stage, s32 unused, s32 day);
extern s32 GetSetHitHeightGate(s32 value);
extern s32 RegisterStyleConfig(s32 grid, s32 stage, s32 sceneRefs, s32 day, s32 unreadArg);

/* The viewport's view point and reference point (attachViewChild), the
 * StageMap's bounds on stage 0, and one DreamSys setPendingExtra value per
 * stage. */
extern LongVec3 gObjMViewPoint;
extern LongVec3 gObjMViewRefPoint;
extern s32 gStagePendingExtras[];
extern CellBounds gStage0Bounds;

/* onInit's gridSpan when it is passed 0: gDefaultGridSpan's value, the one
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

    vp->methods->attachViewChild(vp, (BasicClass *)self->dreamSys, &gObjMViewPoint,
                                 &gObjMViewRefPoint, 0);

    self->cachedViewport = vp;
    day = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
    self->styleConfig =
        (StyleConfig *)RegisterStyleConfig((s32)self->unk14, self->stage, (s32)&self->ctorSound, day, 0);
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
        ((StageMap *)self->unk14)->methods->setBounds((StageMap *)self->unk14, 0);
    } else {
        self->tickPeriod = 16;
        self->moveMode = 2;
        flag = 1;
        ((StageMap *)self->unk14)->methods->setBounds((StageMap *)self->unk14, &gStage0Bounds);
    }

    self->gridSpan = gridSpan;
    if (gridSpan == 0) {
        self->gridSpan = DEFAULT_GRID_SPAN;
    }
    GetSetHitHeightGate(flag);

    self->dreamSys->methods->setPendingExtra(self->dreamSys, gStagePendingExtras[self->stage]);
    self->state = 5;
}

/* onDeinit. */
void ObjM__TeardownStyle(ObjM *self) {
    self->methods->exitSceneStyle(self);
    ReleaseDreamAuxEntities();
    StyleTeardown();
    self->bgm->methods->stop(self->bgm);
}

void ObjM__OnTag1Notify(ObjM *self, void *sender, s32 event) {
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
        if (((StageMap *)self->unk14)->pendingLoadCount == 0 && self->inSession == 0) {
            self->unk64 = 1;
            self->methods->enterStyleSession(self);
        }
    }
}

/* onPadEvent, only in session: Start pressed toggles the pause, Select
 * held and released sets and clears the close-ready flag, triangle pressed
 * closes (closeAndNotifyD).
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
    fn = m->closeAndNotifyD;
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

/* src/code_4cd08.c's; no header declares it. It keeps the stage, the
 * StageMap, the DreamSys (gDreamAuxWorld), the sound and the FrameClock for
 * the dream's aux entities. */
struct FrameClock;
extern void SetDreamAuxWorld(s32 stage, StageMap *stageMap, DreamSys *world,
                             struct VabStreamObj *sound, struct FrameClock *frameClock);

/* Added to the viewport's projection distance; 0 in the image and never
 * written. */
extern s32 gObjMProjectionBias;

/* The StageMap's accepted tags (setAcceptedTags): the class ids of DreamSys
 * (0x1F34) and Entity (0x1F234), 0-terminated. */
extern s32 gObjMAcceptedClassIds[];

void ObjM__SetupSceneStyle(ObjM *self) {
    NodeGuardedViewport *vp = (NodeGuardedViewport *)self->viewport;
    StyleConfig *style = self->styleConfig;
    DrawSystem *drawSystem;
    s32 width;
    StageMap *rig;

    vp->methods->detachViewChild(vp);

    drawSystem = (DrawSystem *)self->initArgs->drawSystem;
    width = drawSystem->methods->getDims(drawSystem, NULL)->w;
    vp->methods->setProjection(vp, width / 2 * 5 / 3 + gObjMProjectionBias);

    vp->methods->attachViewChild(vp, (BasicClass *)self->dreamSys, &gObjMViewPoint,
                                 &gObjMViewRefPoint, 0);

    SetDreamAuxWorld(self->stage, (StageMap *)self->unk14, self->dreamSys,
                     (struct VabStreamObj *)self->sound, (struct FrameClock *)self->unk10);

    rig = (StageMap *)self->unk14;
    self->methods->addChild(self, (BasicClass *)rig);

    rig->methods->setAmbientColor(rig, (LightRigRgb *)style->ambientColor, 0);
    rig->methods->setChildParams(rig, 3, style->lightDirs, style->lightColors);
    rig->methods->setConfig(rig, GetStageGridDimensions(self->stage));
    ((DreamSysAttachToParentFn)self->dreamSys->methods->attachToParent)(self->dreamSys, rig);
    rig->methods->setGridSpan(rig, self->gridSpan);
    rig->methods->setAcceptedTags(rig, gObjMAcceptedClassIds);
}

void ObjM__ExitSceneStyle(ObjM *self) {
    self->methods->teardownPauseOverlay(self);
    self->dreamSys->methods->blockMovement(self->dreamSys);
    self->dreamSys->methods->detachFromParent(self->dreamSys);
    ((NodeGuardedViewport *)self->viewport)->methods->detachViewChild((NodeGuardedViewport *)self->viewport);
    self->methods->removeChild(self, self->unk14);
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
    ((StageMap *)self->unk14)->methods->enable((StageMap *)self->unk14);

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
    m2->startFadeDown(fade, self->unk10, channels, 0);
}

/* Defined below, in the style layer. */
extern s32 TickStyle(Descriptor10 *cell, void *unused, s32 lastCue);

void ObjM__TickStyle(ObjM *self) {
    TickStyle(((StageMap *)self->unk14)->methods->getTargetDescriptor((StageMap *)self->unk14, 0, 0),
              0, 0);
}

/* While ObjM's state is 0 each DreamSys link code runs its enterState slot
 * (DREAMSYS_LINK_DAY_START none); otherwise any code from 9 up clears the
 * DreamSys's own state. */
void ObjM__OnDreamSysNotify(ObjM *self, BasicClass *sender, s32 code) {
    if (self->state == 0) {
        switch (code) {
            case DREAMSYS_TIME_UP:
                self->methods->enterState4(self);
                break;
            case DREAMSYS_LINK_DAY_START:
                break;
            case DREAMSYS_LINK_DYNAMIC:
                self->methods->enterState5(self);
                break;
            case DREAMSYS_LINK_WALL:
                self->methods->enterState6(self);
                break;
            case DREAMSYS_LINK_FLASHBACK:
                self->methods->enterState7(self);
                break;
            case DREAMSYS_LINK_TUNNEL:
                self->methods->enterState8(self);
                break;
            case DREAMSYS_LINK_STAGE_TIMER:
                self->methods->enterStateA(self);
                break;
            case DREAMSYS_LINK_TELEPORT:
                self->methods->notifyParentsCodeB(self);
                break;
        }
    } else if (code >= 9) {
        self->dreamSys->state = DREAMSYS_NO_LINK;
    }
}

void ObjM__EnterState4(ObjM *self) {
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

void ObjM__EnterState5(ObjM *self) {
    s32 color;

    if (self->dreamSys->currentStage < 0) {
        self->methods->enterState6(self);
    } else {
        self->state = 5;
        color = self->dreamSys->methods->getDreamColor(self->dreamSys);
        ObjM__StartFadeUp(self, color, 0, 10, 1);
        self->dreamSys->methods->blockMovement(self->dreamSys);
    }
}

void ObjM__EnterState6(ObjM *self) {
    s32 color;

    self->state = 6;
    color = self->dreamSys->methods->getDreamColor(self->dreamSys);
    ObjM__StartFadeUp(self, color, 0, 30, 1);
    self->dreamSys->methods->blockMovement(self->dreamSys);
}

/* ---- merged from class_3bb8c_m ---- */

/*
 * class_3bb8c_m -- ObjM's methods from +0x0A0 to the end of its table
 * (gObjMMethods, include/ObjM.h) and its getter, then the style layer's
 * setup: the four free functions that pick the stage's scene style.
 *
 * ObjM, in ROM order:
 *  - EnterState7/8/A and NotifyParentsCodeB, the DreamSys link codes
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
 *    up and ObjM is IDLE, the close-ready flag arms CloseAndNotifyC/D,
 *    which tear it down and notify a close (DayTask ends the day).
 *    NoOpSlotBC is empty.
 *
 * The style setup is not ObjM's, but ObjM is its client:
 * ObjM__InitStyleAndWorld (class_3bb8c_k) calls RegisterStyleConfig once
 * per scene and keeps the result, sStyleConfig, as ObjM::styleConfig.
 * RegisterStyleConfig stores the scene (grid, stage, ObjM's scene
 * references, day) in the gStyle globals class_3bb8c_n reads;
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

/* src/code_4cd08.c; it reads `coord` as one s16 trigger key. */
extern s32 TryDreamAuxTrigger(s32 data, ChunkCoord *coord, s32 day);

/* ObjM__AdvancePauseSetup's literals, all reached by address: the "Pause"
 * text, the TextRow's position (attachToParent) and its colour (setColor). */
extern char sPauseText[];             /* "Pause" */
extern ScreenSpritePos sPauseTextPos; /* (-20, -50) */
extern SpriteRgb sPauseTextColor;     /* red: (255, 0, 0) */

void ObjM__EnterState7(ObjM *self) {
    DreamColors color;
    self->state = OBJM_STATE_LINK_FLASHBACK;
    self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, &color, -1);
    ObjM__StartFadeUp(self, color, 0, 5, 1);
    self->dreamSys->methods->blockMovement(self->dreamSys);
}

void ObjM__EnterState8(ObjM *self) {
    self->state = OBJM_STATE_LINK_TUNNEL;
    ObjM__StartFadeUp(self, DREAM_COLOR_BLACK, 0, 6, 1);
    self->dreamSys->methods->setMoveOverride(self->dreamSys, MOVE_OVERRIDE_FORCED);
}

void ObjM__EnterStateA(ObjM *self) {
    self->state = OBJM_STATE_LINK_STAGE_TIMER;
    ObjM__StartFadeUp(self, DREAM_COLOR_BLACK, 0, 6, 1);
    self->dreamSys->methods->selectCallback98(self->dreamSys, MOVE_CALLBACK_TICK_DRIFT);
    self->dreamSys->methods->setMoveOverride(self->dreamSys, MOVE_OVERRIDE_HELD);
}

void ObjM__NotifyParentsCodeB(ObjM *self) {
    self->methods->notifyParents(self, OBJM_NOTIFY_LINK_TELEPORT);
}

/* The fade box is the viewport's (IntermediateBase::viewport, a
 * NodeGuardedViewport: getFadeBox); IntermediateBase::unk10, the FrameClock,
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
    fade->methods->startFadeUp(fade, self->unk10, channels, fadeMode);
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
    ChunkSlot *slot =
        ((StageMap *)self->unk14)->methods->getLastEventSlotChunk((StageMap *)self->unk14, &coord.column);
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

void ObjM__CloseAndNotifyD(ObjM *self) {
    if (self->closeReady) {
        self->methods->teardownPauseOverlay(self);
        self->methods->notifyParents(self, OBJM_NOTIFY_CLOSE_NEW_GAME);
    }
}

void ObjM__CloseAndNotifyC(ObjM *self) {
    if (self->closeReady) {
        self->methods->teardownPauseOverlay(self);
        self->methods->notifyParents(self, OBJM_NOTIFY_CLOSE);
    }
}

/* Called each update while the overlay is up. Step 0 builds the "Pause"
 * TextRow under the StageMap; the fourth call after it hides the viewport
 * and pauses the FrameClock, the WBgm and the VabStreamObj
 * (IntermediateBase::unk10, bgm, TimedTask::sound). */
void ObjM__AdvancePauseSetup(ObjM *self) {
    s32 step = self->pauseSetupStep;
    if (step == 0) {
        self->pauseText = New_TextRow(self->etcTim, 5, sPauseText);
        self->pauseText->methods->attachToParent(self->pauseText, (SceneNode *)self->unk14,
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
    ((FrameClock *)self->unk10)->methods->pause((FrameClock *)self->unk10);
    self->bgm->methods->pause(self->bgm);
    ((VabStreamObj *)self->sound)->methods->mute((VabStreamObj *)self->sound);
}

void ObjM__TeardownPauseOverlay(ObjM *self) {
    if (self->pauseSetupStep != 0) {
        self->pauseText->methods->release(self->pauseText);
    }
    ((VabStreamObj *)self->sound)->methods->unmute((VabStreamObj *)self->sound);
    self->bgm->methods->resume(self->bgm);
    ((FrameClock *)self->unk10)->methods->resume((FrameClock *)self->unk10);
    ((NodeGuardedViewport *)self->viewport)->methods->setDrawEnabled((NodeGuardedViewport *)self->viewport, 1);
    self->pauseSetupStep = 0;
}

ObjMMethods *GetObjMMethods(void) {
    return &gObjMMethods;
}

extern s32 gStyleGrid;
extern s32 gStyleStage;
extern s32 gStyleTickCount;
extern s32 gStyleDay;
extern s32 sStyleUnreadArg;
extern s32 gStyleSceneRefs; /* a StyleSceneRefs * (below) */
extern s32 gStyleVariant;
/* class_3bb8c_n.c defines StyleCueSlot; this unit only clears the slots. */
typedef struct StyleCueSlot StyleCueSlot;
extern StyleCueSlot *gStyleCueSlots[2];

extern void *ApplyStyleConfig(void);

s32 RegisterStyleConfig(s32 grid, s32 stage, s32 sceneRefs, s32 day, s32 unreadArg) {
    StyleCueSlot **slot;
    s32 i;

    if (gStyleGrid == 0) {
        i = ARRAY_COUNT(gStyleCueSlots) - 1;
        slot = &gStyleCueSlots[ARRAY_COUNT(gStyleCueSlots) - 1];
        gStyleGrid = grid;
        gStyleStage = stage;
        gStyleSceneRefs = sceneRefs;
        gStyleVariant = -1;
        gStyleDay = day;
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
 * (class_3bb8c_n), which FillStyleFromConfig turns into sStyleConfig's last
 * four words. */
typedef struct StyleStageConfig {
    s8 colorMode; /* StyleConfig::colorMode */
    s8 fogLevel; /* sStyleFogNears index; STYLE_DECOR_FOG_LEVEL and up also build the decoration box */
    s8 farColorIndex; /* gStylePalette index: StyleConfig::farColor, and the decoration box's colour */
    s8 clearColorIndex; /* gStylePalette index: StyleConfig::clearColor */
} StyleStageConfig;

/* The fog levels whose config also gets a decoration box (ApplyStyleConfig):
 * this one and up, sStyleFogNears' two nearest (fogNear 4096 and 2048). */
#define STYLE_DECOR_FOG_LEVEL 4

extern StyleConfig sStyleConfig;
extern StyleStageConfig *sStyleStageConfigs[];
extern void *PickStyleFallbackConfig(void);
extern void FillStyleFromConfig(StyleConfig *style, StyleStageConfig *cfg);
extern u8 gStylePalette[][3];
extern const u8 *gStyleDecorColor;

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
        gStyleDecorColor = gStylePalette[cfg->farColorIndex];
    }
    return &sStyleConfig;
}

/* gStylePalette is 24 RGB triples (a greyscale ramp first: 0, 64, 128, 255).
 * MATCHING: indexed as `u8[][3]`, for retail's `i*2 + i + base` stride-3
 * address arithmetic. sStyleFogNears is six fogNear distances, 26624 down to
 * 2048. */
extern s32 sStyleFogNears[];

void FillStyleFromConfig(StyleConfig *style, StyleStageConfig *cfg) {
    style->clearColor = gStylePalette[cfg->clearColorIndex];
    style->farColor = gStylePalette[cfg->farColorIndex];
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

extern s32 gStyleDecorObj;           /* a BoxFill *; class_3bb8c_n.c declares it s32 too */
extern s32 sStyleDecorBoxSize[2];    /* 320 x 240, the screen */
extern BoxFillPos sStyleDecorBoxPos; /* (-100, -100), as Viewport's own fade box */

void ApplyStyleDecorationIfSet(void) {
    SceneNode *fadeBox;

    if (gStyleDecorColor != 0) {
        gStyleDecorObj = (s32)New_BoxFill(sStyleDecorBoxSize, (BoxFillRgb *)gStyleDecorColor, 0);
        ((BoxFill *)gStyleDecorObj)->methods->setSemiTransOn((BoxFill *)gStyleDecorObj, 1);
        ((BoxFill *)gStyleDecorObj)->methods->setSemiTransRate((BoxFill *)gStyleDecorObj, 0);

        fadeBox = ((StyleSceneRefs *)gStyleSceneRefs)
                      ->viewport->methods->getFadeBox(((StyleSceneRefs *)gStyleSceneRefs)->viewport);

        ((BoxFillAttachToParentFn)((BoxFill *)gStyleDecorObj)->methods->attachToParent)(
            (BoxFill *)gStyleDecorObj, fadeBox, &sStyleDecorBoxPos);
    }
}

/* ---- merged from class_3bb8c_n ---- */

/*
 * class_3bb8c_n -- the style layer's per-scene objects: what TickStyle
 * builds on a scene's first tick, updates on every tick, and StyleTeardown
 * releases.
 *
 * RegisterStyleConfig (class_3bb8c_k.c), called by ObjM__InitStyleAndWorld
 * and a no-op until StyleTeardown clears gStyleGrid, sets the state read here: gStyleGrid (the
 * scene's StageMap), gStyleStage (ObjM's stage), gStyleSceneRefs (ObjM's
 * sound, resources and viewport; StyleSceneRefs below) and gStyleDay (the
 * DreamSys day). ApplyStyleConfig then takes the stage's fixed config or,
 * with none, PickStyleFallbackConfig's: a variant (gStyleVariant, 0..3) and
 * a config record picked from day + stage.
 *
 * What TickStyle keeps, each built on the first tick:
 *  - the decoration box, gStyleDecorObj, when the config has a colour for
 *    it (class_3bb8c_k builds it, StyleFlushDecoration releases it);
 *  - the decor set: STYLE_DECOR_BANDS BoxFill bands (gStyleDecorSlots)
 *    coloured from gStyleDecorColors and attached under the viewport's fade
 *    box; every tick StyleUpdateDecorSet shifts their colours, their
 *    position and the viewport's clear colour by the view point's y offset
 *    from its reference point;
 *  - the effect slots: StyleEffect objects of kinds 0..3 (gStyleEffectSlots)
 *    built from one parameter block, gStyleSpawnOffsetX..gStyleSpawnColors,
 *    that StyleFillEffectKindN and SetupStyleSpawnParamsA/B fill in; each
 *    tick updates them with the target position;
 *  - two positional sound cues (gStyleCueSlots, in gStyleCueSlotPool): a
 *    free slot claims the next record of the stage's cue list that lies
 *    within its cue's distance of the target and starts the record's
 *    SoundCueSet callback (gStyleCueCallbacks, class_3bb8c_r.c); a claimed
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
 * gStyleDecorColorsB instead of gStyleDecorColorsA (PickStyleFallbackConfig). */
#define STYLE_DECOR_B_PALETTE_INDEX 18


extern const u8 *gStyleDecorColor;
extern s32 gStyleDecorObj; /* a BoxFill; class_3bb8c_k.c declares it s32 too */

/* Releases the decoration box, if ApplyStyleDecorationIfSet made one. */
void StyleFlushDecoration(void) {
    if (gStyleDecorColor != 0) {
        ((BoxFill *)gStyleDecorObj)->methods->release((BoxFill *)gStyleDecorObj);
        gStyleDecorColor = 0;
    }
}

extern s32 gStyleDay;
extern s32 gStyleStage;
extern s8 gStyleVariantPicks[];
extern s32 gStyleVariant;
extern s8 gStyleVariantConfigCounts[];
extern s32 gStyleConfigIndex;
extern s8 *gStyleVariantConfigs[];
extern const u8 *gStyleClearColor;
extern u8 gStyleDecorColorsB[];
extern u8 gStylePalette[][3];
extern const u8 *gStyleDecorColors;
extern u8 gStyleDecorColorsA[];
extern s32 gStyleDecorVariant;

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

    seed = gStyleDay + gStyleStage;
    variant = gStyleVariantPicks[seed & 0xF];
    gStyleVariant = variant;
    count = gStyleVariantConfigCounts[variant];
    index = seed % count;
    gStyleConfigIndex = index;
    config = gStyleVariantConfigs[variant] + index * 4;
    if (variant == 0) {
        clearIndex = config[3];
        gStyleClearColor = gStylePalette[clearIndex];
        decorIndex = config[2];
        decorColors = gStyleDecorColorsB;
        if (decorIndex != STYLE_DECOR_B_PALETTE_INDEX) {
            decorColors = gStyleDecorColorsA;
        }
        gStyleDecorColors = decorColors;
        if (index < 4) {
            gStyleDecorVariant = 1;
        } else if (index < 6) {
            gStyleDecorVariant = 2;
        }
    }
    return config;
}

extern s32 gStyleDecorPosX;
extern s32 gStyleDecorPosY;
extern s32 gStyleDecorSizeW;
extern s32 gStyleDecorSizeH;
extern BoxFill *gStyleDecorSlots[STYLE_DECOR_BANDS];
extern s32 gStyleSceneRefs; /* a StyleSceneRefs *; class_3bb8c_k.c declares it s32 too */

/* gStyleDecorPosX/Y and gStyleDecorSizeW/H are adjacent word pairs.
 * MATCHING: copied whole, never field by field (a BLKmode copy makes cse
 * drop cached memory values; scalar copies lose retail's reloads). */
typedef struct PairXY PairXY;

struct PairXY {
    s32 x; /* +0x000 */
    s32 y; /* +0x004 */
};

/* Builds the bands: band 0 in gStyleDecorColors' first colour, bands 1..17
 * attached under it, each 3 pixels lower and 7 shorter than the one before;
 * band 0 then goes under the viewport's fade box. */
void StyleBuildDecorSet(void) {
    PairXY pos;
    PairXY size;
    s32 i;
    BoxFill *band;
    Viewport *viewport;
    SceneNode *parent;

    if (gStyleDecorVariant == 0) {
        return;
    }
    pos = *(PairXY *)&gStyleDecorPosX;
    if (gStyleDecorVariant == 2) {
        pos.y += STYLE_DECOR_VARIANT2_DROP;
    }
    size = *(PairXY *)&gStyleDecorSizeW;
    gStyleDecorSlots[0] = New_BoxFill(&size, (void *)gStyleDecorColors, STYLE_DECOR_PRI);
    for (i = 1; i < STYLE_DECOR_BANDS; i++) {
        band = New_BoxFill(&size, (void *)(gStyleDecorColors + i * 3), STYLE_DECOR_PRI);
        gStyleDecorSlots[i] = band;
        ((BoxFillAttachToParentFn)band->methods->attachToParent)(
            band, (SceneNode *)gStyleDecorSlots[0], (BoxFillPos *)&pos);
        pos.y += 3;
        size.y -= 7;
    }

    viewport = ((StyleSceneRefs *)gStyleSceneRefs)->viewport;
    parent = viewport->methods->getFadeBox(viewport);
    ((BoxFillAttachToParentFn)gStyleDecorSlots[0]->methods->attachToParent)(
        gStyleDecorSlots[0], parent, (BoxFillPos *)&pos);
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

    if (gStyleDecorVariant == 0) {
        return;
    }
    viewport = ((StyleSceneRefs *)gStyleSceneRefs)->viewport;
    height = viewport->refView.vp.y - viewport->refView.vr.y;
    fade = (height / STYLE_DECOR_FADE_HEIGHT) * 3;
    if (fade <= 0) {
        return;
    }
    pos = *(PairXY *)&gStyleDecorPosX;
    i = 0;
    if (gStyleDecorVariant == 2) {
        pos.y += STYLE_DECOR_VARIANT2_DROP;
    }
    slot = gStyleDecorSlots;
    colorOfs = 0;
    pos.y += fade * 3;
    do {
        AdjustRgbByDelta(rgb, (u8 *)(colorOfs + gStyleDecorColors), fade);
        band = *slot;
        band->methods->setColor(band, 1, rgb);
        band = *slot;
        i++;
        colorOfs += 3;
        band->methods->setPosition(band, (BoxFillPos *)&pos);
        pos.y += 3;
        slot++;
    } while (i < STYLE_DECOR_BANDS);
    AdjustRgbByDelta(rgb, (u8 *)gStyleClearColor, fade);
    viewport->methods->setClearColor(viewport, (ViewportRgb *)rgb);
}

/* dst = src with red and green less `delta`, blue more. */
void AdjustRgbByDelta(u8 *dst, u8 *src, s32 delta) {
    dst[0] = src[0] - delta;
    dst[1] = src[1] - delta;
    dst[2] = src[2] + delta;
}

extern void ReleaseBasicClassArray(void **array, s32 count);
extern s32 gStyleDecorVariant;
extern BoxFill *gStyleDecorSlots[STYLE_DECOR_BANDS];

/* Releases the bands, if StyleBuildDecorSet made them. */
void StyleReleaseDecorSet(void) {
    if (gStyleDecorVariant != 0) {
        ReleaseBasicClassArray((void **)gStyleDecorSlots, ARRAY_COUNT(gStyleDecorSlots));
        gStyleDecorVariant = 0;
    }
}

extern s32 gStyleVariant;
extern s32 gStyleSceneRefs;
extern s32 rand(void);
extern s8 gStyleKind0Counts[];
extern s32 gStyleEffectSlotCount;
extern StyleEffect *gStyleEffectSlots[];
extern StyleEffect **StyleFillEffectKind0(StyleEffect **slots, s32 count, LongVec3 *pos);
extern StyleEffect **StyleFillEffectKind1(StyleEffect **slots, s32 count, LongVec3 *pos);
extern StyleEffect **StyleFillEffectKind3(StyleEffect **slots, LongVec3 *pos);
extern StyleEffect **StyleFillEffectKind2(StyleEffect **slots, LongVec3 *pos);

/* Hands the variant and ObjM's resources to SetStyleEffectSources, then builds
 * the effect slots for the variant: gStyleKind0Counts' pick of
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
    kind0Count = gStyleKind0Counts[rand() & 3];
    kind1Count = (gStyleVariant == 2) ? STYLE_VARIANT2_EFFECTS - kind0Count : 0;
    gStyleEffectSlotCount = kind0Count + kind1Count;
    next = StyleFillEffectKind0(gStyleEffectSlots, kind0Count, pos);
    next = StyleFillEffectKind1(next, kind1Count, pos);
    if (gStyleVariant == 0) {
        StyleFillEffectKind3(next, pos);
    } else if (gStyleVariant == 2) {
        StyleFillEffectKind2(next, pos);
    } else {
        return;
    }
    gStyleEffectSlotCount = gStyleEffectSlotCount + 1;
}

extern s32 gStyleVariant;
extern s32 gStyleEffectSlotCount;

/* Each slot's +0x0EC is StyleEffect__Update, called with the position
 * (include/StyleEffect.h: the slot keeps Actor's setPendingExtra type). */
void StyleUpdateEffectSlots(LongVec3 *pos) {
    s32 i;
    StyleEffect *slot;

    if (gStyleVariant < 0) {
        return;
    }
    for (i = 0; i < gStyleEffectSlotCount; i++) {
        slot = gStyleEffectSlots[i];
        ((StyleEffectUpdateFn)slot->methods->setPendingExtra)(slot, pos);
    }
}

extern s32 gStyleVariant;
extern s32 gStyleEffectSlotCount;

/* Releases the effect slots, if StyleBuildEffectSlots ran. */
void StyleReleaseEffectSlots(void) {
    if (gStyleVariant >= 0) {
        ReleaseBasicClassArray((void **)gStyleEffectSlots, gStyleEffectSlotCount);
    }
}

/* A cue record's view here: `cue` is its cue index (the gStyleCueCallbacks
 * and gStyleCueDistanceTable row, InitSoundCueSet's tag), negated while a
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

extern s32 gStyleGrid; /* a StageMap; class_3bb8c_k.c declares it s32 too */
extern StyleCueSlot *gStyleCueSlots[2];

/* Releases everything TickStyle built and unregisters the scene. */
void StyleTeardown(void) {
    s32 i;

    StyleFlushDecoration();
    StyleReleaseDecorSet();
    StyleReleaseEffectSlots();
    for (i = 0; i < ARRAY_COUNT(gStyleCueSlots); i++) {
        gStyleCueSlots[i] = FlushStyleCue(gStyleCueSlots[i]);
    }
    if (gStyleGrid != 0) {
        gStyleGrid = 0; /* RegisterStyleConfig registers only while this is 0 */
    }
}

extern Ratio16 gStyleSpawnScales[][3];
extern s32 gStyleSpawnYChoices[];
extern Ratio16 *gStyleSpawnScale;
extern s32 gStyleSpawnTableIndex;
/* The first word of the StyleEffectParams block every effect is built from
 * (gStyleSpawnOffsetX .. gStyleSpawnColors, separate symbols in the image). */
extern s32 gStyleSpawnOffsetX;
extern void SetupStyleSpawnParamsA(LongVec3 *pos, s32 offsetY);
extern void SetupStyleSpawnParamsB(LongVec3 *pos, s32 offsetY);

/* Fills `count` slots with kind-0 effects: a table index and scale for all
 * of them, an offset y (0: each setup picks one; a pick of 4 reads the word
 * after gStyleSpawnYChoices, as retail does), and per slot
 * SetupStyleSpawnParamsA, or B on every seventh day. Returns the next slot. */
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
    setup = SetupStyleSpawnParamsB;
    if (gStyleDay % 7 != 0) {
        setup = SetupStyleSpawnParamsA;
    }
    for (i = 0; i < count; i++) {
        setup(pos, offsetY);
        *slots =
            New_StyleEffect(0, (StyleEffectParams *)&gStyleSpawnOffsetX, (SceneNode *)gStyleGrid, pos);
        slots++;
    }
    return slots;
}

extern s32 gStyleSpawnYChoice2;
extern Ratio16 gStyleKind1Scale[];

/* Fills `count` slots with kind-1 effects: gStyleKind1Scale, offset y
 * gStyleSpawnYChoice2. */
StyleEffect **StyleFillEffectKind1(StyleEffect **slots, s32 count, LongVec3 *pos) {
    s32 i;
    s32 offsetY;

    offsetY = gStyleSpawnYChoice2;
    gStyleSpawnScale = gStyleKind1Scale;
    for (i = 0; i < count; i++) {
        SetupStyleSpawnParamsA(pos, offsetY);
        *slots =
            New_StyleEffect(1, (StyleEffectParams *)&gStyleSpawnOffsetX, (SceneNode *)gStyleGrid, pos);
        slots++;
    }
    return slots;
}

extern s32 gStyleSpawnYChoice2;
extern void SetupStyleSpawnParamsA(LongVec3 *pos, s32 offsetY);
extern s32 gStyleSpawnColors[];
extern Ratio16 *gStyleSpawnRotation;
extern Ratio16 gStyleSpawnRotations[][3];
extern s32 gStyleSpawnOffsetY;
extern s32 gStyleSpawnOffsetZ;
extern u8 gStyleKind3Colors[][3];

/* MATCHING: the rotation store goes through a one-field struct, so the
 * gStyleGrid load may schedule above it (a plain pointer store blocks it). */
typedef struct PtrBoxK3 {
    Ratio16 *p; /* +0x000 */
} PtrBoxK3;

/* Appends one kind-3 effect. With decor variant active and band colours B
 * its offset and colour are fixed; otherwise its z offset is folded to
 * -30720..0 and its colour is random. */
StyleEffect **StyleFillEffectKind3(StyleEffect **slots, LongVec3 *pos) {
    s32 *offsetZ;
    PtrBoxK3 *rotation;

    SetupStyleSpawnParamsA(pos, gStyleSpawnYChoice2);
    if (gStyleDecorVariant != 0 && gStyleDecorColors == gStyleDecorColorsB) {
        gStyleSpawnOffsetX = -45056;
        gStyleSpawnOffsetY = -8192;
        gStyleSpawnOffsetZ = 0;
        gStyleSpawnColors[0] = (s32)gStyleKind3Colors[1];
    } else {
        offsetZ = &gStyleSpawnOffsetZ;
        if (*offsetZ > 0) {
            *offsetZ = -*offsetZ;
        }
        if (*offsetZ < -30720) {
            *offsetZ = -30720;
        }
        gStyleSpawnColors[0] = (s32)gStyleKind3Colors[(u32)rand() % 3];
    }
    rotation = (PtrBoxK3 *)&gStyleSpawnRotation;
    rotation->p = gStyleSpawnRotations[0];
    /* MATCHING: the block's address is taken back from its rotation member */
    *slots = New_StyleEffect(
        3, (StyleEffectParams *)((u8 *)rotation - offsetof(StyleEffectParams, rotation)),
        (SceneNode *)gStyleGrid, pos);
    slots++;
    return slots;
}

extern s32 gStyleKind2AltColor;
extern u8 gStyleKind2Colors[][3];
extern s32 gStyleSpawnColors[];
extern Ratio16 *gStyleSpawnRotation;
extern Ratio16 gStyleSpawnRotations[][3];
extern s32 gStyleSpawnTableIndex;

/* MATCHING: the first colour store goes through a one-field struct, as
 * PtrBoxK3's does, so the gStyleDay load may schedule above it. */
typedef struct S32BoxK2 {
    s32 v; /* +0x000 */
} S32BoxK2;

/* Appends one kind-2 effect with a random colour and, except on every
 * twentieth day, gStyleKind2AltColor as its alternate colour. */
StyleEffect **StyleFillEffectKind2(StyleEffect **slots, LongVec3 *pos) {
    s32 r;
    s32 altColor;
    S32BoxK2 *color;
    Ratio16 **rotation;

    r = rand();
    color = (S32BoxK2 *)gStyleSpawnColors;
    color->v = (s32)gStyleKind2Colors[(u32)r % 3];
    color++;
    altColor = (gStyleDay / 20) * 20; /* MATCHING: not `% 20`, which jump.c folds */
    if (gStyleDay != altColor) {
        altColor = gStyleKind2AltColor;
    } else {
        altColor = 0;
    }
    color->v = altColor;
    SetupStyleSpawnParamsA(pos, gStyleSpawnYChoice2);
    rotation = &gStyleSpawnRotation;
    *rotation = gStyleSpawnRotations[0];
    gStyleSpawnTableIndex = rand() % 6;
    /* MATCHING: the block's address is taken back from its rotation member */
    *slots = New_StyleEffect(
        2, (StyleEffectParams *)((u8 *)rotation - offsetof(StyleEffectParams, rotation)),
        (SceneNode *)gStyleGrid, pos);
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
void SetupStyleSpawnParamsA(LongVec3 *pos, s32 offsetY) {
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
 * picks as A. Both parameters are unused; it has A's signature because
 * StyleFillEffectKind0 calls either through one pointer.
 * MATCHING: each rand() is used inline; one local for all three adds a move
 * after every call. */
void SetupStyleSpawnParamsB(LongVec3 *pos, s32 offsetY) {
    s32 dayMod3;

    rand();
    gStyleSpawnOffsetY = gStyleSpawnYChoice1;
    gStyleSpawnOffsetX = (rand() % 20) << 11;
    dayMod3 = gStyleDay % 3;
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
extern SoundCueCallbackFn gStyleCueCallbacks[];
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
                        gStyleCueCallbacks[entry->cue]);
        if (entry->cue == *lastCue) {
            *lastCue = -entry->cue;
        }
        entry->cue = -entry->cue;
        return slot;
    }
    return 0;
}

extern s32 gStyleStage;
extern s32 gStyleCueRecordIndex;
extern u8 *gStyleCueRecordLists[];
extern u8 gStyleCueRecordCounts[];
extern s32 gStyleCueDistanceTable[];

/* The cell-key halves: a record's four cell bytes and gStyleCueOffsets' s16
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

extern TabEntry gStyleCueOffsets[];

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

/* From gStyleCueRecordIndex on, the first free record of the stage's list
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
    records = gStyleCueRecordLists[gStyleStage];
    remaining = gStyleCueRecordCounts[gStyleStage] - gStyleCueRecordIndex;
    entry = (EntrySlot *)(gStyleCueRecordIndex * 8 + (s32)records); /* MATCHING: operand order */
    for (j = 0; j < remaining; j++, entry++) {
        gStyleCueRecordIndex++;
        if (entry->cue > 0) {
            buf.pos = entry->pos;
            buf.tab = gStyleCueOffsets[entry->offsetIndex];
            grid = (StageMap *)gStyleGrid;
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
            if (dist < gStyleCueDistanceTable[entry->cue]) {
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

extern s32 gStyleCueDistanceTable[];

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
    if (dist < gStyleCueDistanceTable[-cue]) {
        dist = 1; /* MATCHING: not `return 1` (jump.c folds that to slt) */
        return dist;
    }
    return 0;
}

extern void ApplyStyleDecorationIfSet(void); /* class_3bb8c_k.c */
extern void StyleBuildDecorSet(void);
extern void StyleUpdateDecorSet(void);
extern void StyleScrollVramStrips(void);
extern s32 gStyleTickCount;
extern s32 gStyleCueRecordIndex;
extern StyleCueSlot gStyleCueSlotPool[];
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
        ((StageMap *)gStyleGrid)->methods->computeCellOffsets((StageMap *)gStyleGrid, target, cell);
    }
    if (gStyleTickCount++ == 0) {
        ApplyStyleDecorationIfSet();
        StyleBuildDecorSet();
        StyleBuildEffectSlots(target);
    }
    StyleUpdateDecorSet();
    StyleUpdateEffectSlots(target);
    StyleScrollVramStrips();
    gStyleCueRecordIndex = 0;
    for (i = 0; i < ARRAY_COUNT(gStyleCueSlots); i++) {
        if (gStyleCueSlots[i] != 0) {
            if (ServiceStyleCueIfNear(gStyleCueSlots[i], target, unused) == 0) {
                gStyleCueSlots[i] = FlushStyleCue(gStyleCueSlots[i]);
            }
            /* MATCHING: a no-op pair that gives target/i retail's registers */
            i++;
            i--;
        } else {
            gStyleCueSlots[i] = TryStartStyleCue(&gStyleCueSlotPool[i], &lastCue, target, unused);
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
