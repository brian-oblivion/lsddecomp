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
 * SceneNode__RaycastHullAgainstFaces tests) and class_3bb8c_m
 * (RegisterStyleConfig, which keeps `sceneRefs` as gStyleSceneRefs). */
extern s32 PickStageBgm(s32 stage, s32 unused);
extern s32 PickStageTexture(s32 stage, s32 unused, s32 day);
extern s32 GetSetHitHeightGate(s32 value);
extern s32 RegisterStyleConfig(void *grid, s32 stage, s32 *sceneRefs, s32 day, s32 arg4);

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
        (StyleConfig *)RegisterStyleConfig(self->unk14, self->stage, (s32 *)&self->ctorSound, day, 0);
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
