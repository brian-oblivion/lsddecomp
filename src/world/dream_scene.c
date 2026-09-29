/*
 * dream_scene.c -- the day's scene object and what it runs, with the classes
 * that sit between them in ROM. In address order, each under its own section
 * banner below:
 *  - ItemList's second half (include/item_list.h; the first half is in
 *    input_dialogs.c): closing and reporting, the Pad events, the cursor and
 *    scroll methods, the four visible rows and the table getter;
 *  - ObjM (include/objm.h), whole: the TimedTask DayTask starts for a day's
 *    scene;
 *  - the style layer, whose client ObjM is: its setup (RegisterStyleConfig
 *    to ApplyStyleDecorationIfSet), its per-scene objects (StyleFlushDecoration
 *    to StyleScrollVramStrips) and its sound cues (StyleCue00..13).
 * StyleEffect, Actor, VariantSprite and GraphRoom, which follow in ROM, are
 * style_effect.c, actor.c, variant_sprite.c and graph_room.c.
 * The game's own files most likely ended after each class's table getter.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "dream_day.h"
#include "timed_task.h"
#include "text_row.h"
#include "tim_image.h"
#include "item_list.h"
#include "objm.h"
#include "vab_stream_obj.h"
#include "pad.h"
#include "fade_box.h"
#include "dream_sys.h"
#include "stage_map.h"
#include "node_guarded_viewport.h"
#include "tim_block_src.h"
#include "wbgm.h"
#include "lbd_file.h"
#include "box_fill.h"
#include "frame_clock.h"
#include "actor.h"
#include "style_effect.h"
#include "viewport.h"
#include "sound_cue_set.h"
#include "variant_sprite.h"
#include <rand.h>
#include "tmd_model.h"
#include "grid_cell.h"
#include "graph_room.h"
#include "bmem_pmgr.h"
#include "game_files.h"
#include <strings.h>
#include "dream_aux.h"

/* The row colours, two 3-byte RGBs in sdata, 4 bytes apart; only their
 * addresses are taken (setColor). */
extern struct ColorRgb sItemListRowColor;
extern struct ColorRgb sItemListCursorColor;

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

/* ---- ObjM (include/objm.h) ---------------------------------------------
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
    self->loadsComplete = 0;
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
 * In table order; include/objm.h documents each. NoOpResetCounters and
 * NoOpOnTimedOut are empty.
 */

void ObjM__NoOpResetCounters(void) {}

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

/* The StageMap's chunkFileFn: a chunk's file record, by linear cell index,
 * or by x/y when the index is negative. */
CdFileEntry *ObjM__GetGridRecord(ObjM *self, s32 cell, s32 x, s32 y) {
    CdFileEntry *record;

    if (cell >= 0) {
        record = GetStageMapChunkRecord(self->stage, cell);
    } else {
        record = GetStageMapChunkRecordXY(self->stage, x, y);
    }
    return record;
}

void ObjM__DetachTarget(ObjM *self) {
    self->methods->removeChild(self, (BasicClass *)self->dreamSys);
    GetTimedTaskMethods()->deinit((TimedTask *)self);
}

/* Defined elsewhere, no header: src/graphics/scene_node.c (GetSetHitHeightGate
 * sets the flag SceneNode__RaycastHullAgainstFaces tests). RegisterStyleConfig,
 * which keeps `sceneRefs` as sStyleSceneRefs, is defined below, after ObjM. */
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
void ObjM__InitStyleAndWorld(ObjM *self, s32 gridSpan, StyleConfig *style, s32 initOption) {
    NodeGuardedViewport *vp = (NodeGuardedViewport *)self->viewport;
    CdFileEntry *record;
    s32 day;
    s32 flag;

    vp->methods->detachViewChild(vp);
    self->timBlockPending = 1;
    record = PickStageBgm(self->stage, 0);
    self->bgm->methods->setSeq(self->bgm, record->name);

    day = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
    record = PickStageTexture(self->stage, 0, day);
    self->timBlockSrc = New_TimBlockSrc(record->name);

    vp->methods->attachViewChild(vp, (BasicClass *)self->dreamSys, &sObjMViewPoint,
                                 &sObjMViewRefPoint, 0);

    self->cachedViewport = vp;
    day = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
    self->styleConfig = (StyleConfig *)RegisterStyleConfig((s32)self->lightRig, self->stage,
                                                           (s32)&self->ctorSound, day, 0);
    if (style != 0) {
        self->styleConfig = style;
    }

    self->initOption = initOption;
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

/* ObjM__TeardownStyle's helpers (src/world/dream_aux.c, src/world/dream_scene.c). */
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
    ColorRgb *color;
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
            self->loadsComplete = 1;
            self->methods->enterStyleSession(self);
        }
    }
}

/* onPadEvent, only in session: Start pressed toggles the pause, Select
 * held and released sets and clears the close-ready flag, triangle pressed
 * closes (closeAndNotifyNewGame). */
/* MATCHING: the gotos keep retail's compare order; a switch sorts the cases. */
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

void ObjM__NoOpOnTimedOut(void) {}

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
    width = drawSystem->methods->getDims(drawSystem, NULL)->width;
    vp->methods->setProjection(vp, width / 2 * 5 / 3 + sObjMProjectionBias);

    vp->methods->attachViewChild(vp, (BasicClass *)self->dreamSys, &sObjMViewPoint,
                                 &sObjMViewRefPoint, 0);

    SetDreamAuxWorld(self->stage, (StageMap *)self->lightRig, self->dreamSys,
                     (struct VabStreamObj *)self->sound, (struct FrameClock *)self->frameClock);

    rig = (StageMap *)self->lightRig;
    self->methods->addChild(self, (BasicClass *)rig);

    rig->methods->setAmbientColor(rig, (ColorRgb *)style->ambientColor, 0);
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
 * ObjM's methods from enterLinkFlashback to the end of its table and its
 * getter (include/objm.h documents each), then the style layer's setup: the
 * four free functions that pick the stage's scene style.
 *
 * The style layer is not ObjM's, but ObjM is its client:
 * ObjM__InitStyleAndWorld calls RegisterStyleConfig once per scene and keeps
 * the result, sStyleConfig, as ObjM::styleConfig. RegisterStyleConfig stores
 * the scene (grid, stage, ObjM's scene references, day) in the sStyle
 * globals the next section reads; ApplyStyleConfig takes the stage's
 * four-byte StyleStageConfig, or PickStyleFallbackConfig's, and
 * FillStyleFromConfig turns it into StyleConfig colours and a fog distance.
 * ApplyStyleDecorationIfSet builds the decoration box, a full-screen
 * semi-transparent BoxFill under the viewport's fade box, when the config
 * asked for one. What the ObjM states and the style configs stand for in the
 * game is not established; the names describe mechanics.
 */


/* ObjM__AdvancePauseSetup's literals, all reached by address: the "Pause"
 * text, the TextRow's position (attachToParent) and its colour (setColor). */
extern char sPauseText[];             /* "Pause" */
extern ScreenSpritePos sPauseTextPos; /* (-20, -50) */
extern ColorRgb sPauseTextColor;      /* red: (255, 0, 0) */

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
                ->methods->setClearColor((NodeGuardedViewport *)self->viewport, (ColorRgb *)color);
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
    struct MapChunk coord; /* SplitChunkIndex writes the column, then the row */
    s32 held;
    ChunkSlot *slot = ((StageMap *)self->lightRig)
                          ->methods->getLastEventSlotChunk((StageMap *)self->lightRig, &coord.col);
    s32 day = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
    held = TryDreamAuxTrigger((s32)slot->loader->dataBuffer, (s16 *)&coord, day); /* the coord is the trigger key */
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
extern s32 sStyleStage;
extern s32 sStyleTickCount;
extern s32 sStyleDay;
extern s32 sStyleUnreadArg;
extern s32 sStyleSceneRefs; /* a StyleSceneRefs * (below) */
extern s32 sStyleVariant;
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
        sStyleStage = stage;
        sStyleSceneRefs = sceneRefs;
        sStyleVariant = -1;
        sStyleDay = day;
        sStyleUnreadArg = unreadArg;
        sStyleTickCount = 0;
        do {
            *slot = 0;
            i--;
            slot--;
        } while (i >= 0);
        return (s32)ApplyStyleConfig();
    }
    return 0;
}

/** @brief A stage's style config: four signed bytes, from
 * sStyleStageConfigs (NULL for a stage without a fixed one) or
 * PickStyleFallbackConfig (next section), which FillStyleFromConfig turns
 * into sStyleConfig's last four words. */
typedef struct StyleStageConfig {
    s8 colorMode; /**< StyleConfig::colorMode */
    s8 fogLevel; /**< sStyleFogNears index; STYLE_DECOR_FOG_LEVEL and up also build the decoration box */
    s8 farColorIndex; /**< sStylePalette index: StyleConfig::farColor, and the decoration box's colour */
    s8 clearColorIndex; /**< sStylePalette index: StyleConfig::clearColor */
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
    StyleStageConfig *cfg = sStyleStageConfigs[sStyleStage];

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
 * sStyleFogNears is six fogNear distances, 26624 down to 2048. */
/* MATCHING: sStylePalette is indexed as u8[][3], for the stride-3 address arithmetic. */
extern s32 sStyleFogNears[];

void FillStyleFromConfig(StyleConfig *style, StyleStageConfig *cfg) {
    style->clearColor = sStylePalette[cfg->clearColorIndex];
    style->farColor = sStylePalette[cfg->farColorIndex];
    style->fogNear = sStyleFogNears[cfg->fogLevel];
    style->colorMode = cfg->colorMode;
}

/** @brief What sStyleSceneRefs points at: ObjM's +0x06C..+0x07B block
 * (ObjM__InitStyleAndWorld passes &ctorSound to RegisterStyleConfig, which
 * keeps it; include/objm.h). */
typedef struct StyleSceneRefs {
    void *sound;      /**< +0x000, ObjM::ctorSound: the sound object the cue functions take first */
    void *dreamerTmd; /**< +0x004, ObjM::dreamerTmd */
    void *etcTim;     /**< +0x008, ObjM::etcTim */
    Viewport *viewport; /**< +0x00C, ObjM::cachedViewport */
} StyleSceneRefs;

extern s32 sStyleDecorObj;           /* a BoxFill * */
extern s32 sStyleDecorBoxSize[2];    /* 320 x 240, the screen */
extern BoxFillPos sStyleDecorBoxPos; /* (-100, -100), as Viewport's own fade box */

void ApplyStyleDecorationIfSet(void) {
    SceneNode *fadeBox;

    if (sStyleDecorColor != 0) {
        sStyleDecorObj = (s32)New_BoxFill(sStyleDecorBoxSize, (ColorRgb *)sStyleDecorColor, 0);
        ((BoxFill *)sStyleDecorObj)->methods->setSemiTransOn((BoxFill *)sStyleDecorObj, 1);
        ((BoxFill *)sStyleDecorObj)->methods->setSemiTransRate((BoxFill *)sStyleDecorObj, 0);

        fadeBox = ((StyleSceneRefs *)sStyleSceneRefs)
                      ->viewport->methods->getFadeBox(((StyleSceneRefs *)sStyleSceneRefs)->viewport);

        ((BoxFillAttachToParentFn)((BoxFill *)sStyleDecorObj)->methods->attachToParent)(
            (BoxFill *)sStyleDecorObj, fadeBox, &sStyleDecorBoxPos);
    }
}

/* ---- The style layer's per-scene objects ---------------------------------
 *
 * What TickStyle builds on a scene's first tick, updates every tick and
 * StyleTeardown releases, in sStyle globals RegisterStyleConfig set (the
 * scene's StageMap, ObjM's stage, its StyleSceneRefs and the day):
 *  - the decoration box (sStyleDecorObj), when the config has a colour;
 *  - the decor set: STYLE_DECOR_BANDS BoxFill bands (sStyleDecorSlots) under
 *    the viewport's fade box, shaded every tick by the view point's height;
 *  - the effect slots: StyleEffects of kinds 0..3 (sStyleEffectSlots), each
 *    built from the one parameter block the StyleFillEffectKindN functions
 *    fill, updated every tick with the target position;
 *  - two positional sound cues (sStyleCueSlots): a free slot claims the next
 *    record of the stage's cue list within its cue's distance of the target
 *    and starts the record's SoundCueSet callback; a claimed slot is
 *    serviced while the target stays in range and flushed when it leaves.
 * The target is the grid's target cell as a world position; on stages 2 to
 * 5 StyleScrollVramStrips also rotates a VRAM strip. What a variant, an
 * effect kind or a cue stands for in the game is not established; the names
 * describe mechanics.
 */

/* The decoration set: this many BoxFill bands, stacked 3 pixels apart. */
#define STYLE_DECOR_BANDS 18

/* Every band's draw priority, which Viewport__DrawNode hands to
 * GsSortBoxFill unmasked. */
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

/* Releases the decoration box, if ApplyStyleDecorationIfSet made one. */
void StyleFlushDecoration(void) {
    if (sStyleDecorColor != 0) {
        ((BoxFill *)sStyleDecorObj)->methods->release((BoxFill *)sStyleDecorObj);
        sStyleDecorColor = 0;
    }
}

extern s8 sStyleVariantPicks[];
extern s8 sStyleVariantConfigCounts[];
extern s32 sStyleConfigIndex;
extern s8 *sStyleVariantConfigs[];
extern const u8 *sStyleClearColor;
extern u8 sStyleDecorColorsB[];
extern const u8 *sStyleDecorColors;
extern u8 sStyleDecorColorsA[];
extern s32 sStyleDecorVariant;

/* The config for a stage without a fixed one: the variant from
 * sStyleVariantPicks[(day + stage) & 0xF], then record (day + stage) % count
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

    seed = sStyleDay + sStyleStage;
    variant = sStyleVariantPicks[seed & 0xF];
    sStyleVariant = variant;
    count = sStyleVariantConfigCounts[variant];
    index = seed % count;
    sStyleConfigIndex = index;
    config = sStyleVariantConfigs[variant] + index * 4;
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

/* sStyleDecorPosX/Y and sStyleDecorSizeW/H are adjacent word pairs, a
 * BoxFillPos and a BoxFillSize. */
/* MATCHING: both are copied whole, never field by field. */

/* Builds the bands: band 0 in sStyleDecorColors' first colour, bands 1..17
 * attached under it, each 3 pixels lower and 7 shorter than the one before;
 * band 0 then goes under the viewport's fade box. */
void StyleBuildDecorSet(void) {
    BoxFillPos pos;
    BoxFillSize size;
    s32 i;
    BoxFill *band;
    Viewport *viewport;
    SceneNode *parent;

    if (sStyleDecorVariant == 0) {
        return;
    }
    pos = *(BoxFillPos *)&sStyleDecorPosX;
    if (sStyleDecorVariant == 2) {
        pos.y += STYLE_DECOR_VARIANT2_DROP;
    }
    size = *(BoxFillSize *)&sStyleDecorSizeW;
    sStyleDecorSlots[0] = New_BoxFill(&size, (void *)sStyleDecorColors, STYLE_DECOR_PRI);
    for (i = 1; i < STYLE_DECOR_BANDS; i++) {
        band = New_BoxFill(&size, (void *)(sStyleDecorColors + i * 3), STYLE_DECOR_PRI);
        sStyleDecorSlots[i] = band;
        ((BoxFillAttachToParentFn)band->methods->attachToParent)(
            band, (SceneNode *)sStyleDecorSlots[0], &pos);
        pos.y += 3;
        size.h -= 7;
    }

    viewport = ((StyleSceneRefs *)sStyleSceneRefs)->viewport;
    parent = viewport->methods->getFadeBox(viewport);
    ((BoxFillAttachToParentFn)sStyleDecorSlots[0]->methods->attachToParent)(sStyleDecorSlots[0],
                                                                            parent, &pos);
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
    BoxFillPos pos;
    s32 colorOfs;
    s32 i;
    BoxFill **slot;
    BoxFill *band;

    if (sStyleDecorVariant == 0) {
        return;
    }
    viewport = ((StyleSceneRefs *)sStyleSceneRefs)->viewport;
    height = viewport->refView.vp.y - viewport->refView.vr.y;
    fade = (height / STYLE_DECOR_FADE_HEIGHT) * 3;
    if (fade <= 0) {
        return;
    }
    pos = *(BoxFillPos *)&sStyleDecorPosX;
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
        band->methods->setPosition(band, &pos);
        pos.y += 3;
        slot++;
    } while (i < STYLE_DECOR_BANDS);
    AdjustRgbByDelta(rgb, (u8 *)sStyleClearColor, fade);
    viewport->methods->setClearColor(viewport, (ColorRgb *)rgb);
}

/* dst = src with red and green less `delta`, blue more. */
void AdjustRgbByDelta(u8 *dst, u8 *src, s32 delta) {
    dst[0] = src[0] - delta;
    dst[1] = src[1] - delta;
    dst[2] = src[2] + delta;
}

/* Releases the bands, if StyleBuildDecorSet made them. */
void StyleReleaseDecorSet(void) {
    if (sStyleDecorVariant != 0) {
        ReleaseBasicClassArray((BasicClass **)sStyleDecorSlots, ARRAY_COUNT(sStyleDecorSlots));
        sStyleDecorVariant = 0;
    }
}

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

    if (sStyleVariant < 0) {
        return;
    }
    refs = (StyleSceneRefs *)sStyleSceneRefs;
    SetStyleEffectSources(sStyleVariant, (Actor *)refs->dreamerTmd, (s32)refs->etcTim,
                          (s32)refs->viewport);
    kind0Count = sStyleKind0Counts[rand() & 3];
    kind1Count = (sStyleVariant == 2) ? STYLE_VARIANT2_EFFECTS - kind0Count : 0;
    sStyleEffectSlotCount = kind0Count + kind1Count;
    next = StyleFillEffectKind0(sStyleEffectSlots, kind0Count, pos);
    next = StyleFillEffectKind1(next, kind1Count, pos);
    if (sStyleVariant == 0) {
        StyleFillEffectKind3(next, pos);
    } else if (sStyleVariant == 2) {
        StyleFillEffectKind2(next, pos);
    } else {
        return;
    }
    sStyleEffectSlotCount = sStyleEffectSlotCount + 1;
}

/* Each slot's +0x0EC is StyleEffect__Update, called with the position
 * (include/style_effect.h: the slot keeps Actor's setPendingExtra type). */
void StyleUpdateEffectSlots(LongVec3 *pos) {
    s32 i;
    StyleEffect *slot;

    if (sStyleVariant < 0) {
        return;
    }
    for (i = 0; i < sStyleEffectSlotCount; i++) {
        slot = sStyleEffectSlots[i];
        ((StyleEffectUpdateFn)slot->methods->setPendingExtra)(slot, pos);
    }
}

/* Releases the effect slots, if StyleBuildEffectSlots ran. */
void StyleReleaseEffectSlots(void) {
    if (sStyleVariant >= 0) {
        ReleaseBasicClassArray((BasicClass **)sStyleEffectSlots, sStyleEffectSlotCount);
    }
}

/** @brief One 8-byte record of a stage's cue list (sStyleCueRecordLists):
 * the cell it sits in, its in-cell offset (a sStyleCueOffsets index) and
 * `cue`, its cue index (the sStyleCueCallbacks and sStyleCueDistanceTable
 * row, InitSoundCueSet's tag), negated while a slot holds the record. */
typedef struct StyleCueRecord {
    CellKey key;    /**< +0x0 the cell the cue sits in */
    u8 offsetIndex; /**< +0x4 its offset in the cell, a sStyleCueOffsets index */
    u8 pad5;        /* +0x5 */
    s8 cue;         /**< +0x6 the cue index; negated while a slot holds the record */
    u8 pad7;        /* +0x7 */
} StyleCueRecord;

/** @brief One of the two positional cues: the record it holds, the
 * record's world position, the last distance to the target, and its sound
 * cue. */
struct StyleCueSlot {
    StyleCueRecord *entry; /**< +0x000 the record the slot plays */
    LongVec3 pos;          /**< +0x004 the record's world position */
    s32 lastDist;          /**< +0x010 the last distance measured to the target */
    SoundCueSet cueSet;    /**< +0x014 the slot's sound cue */
}; /* 0x68 bytes */

extern StyleCueSlot *FlushStyleCue(StyleCueSlot *slot);

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

extern Ratio16 sStyleSpawnScales[][3];
extern s32 sStyleSpawnYChoices[];
extern Ratio16 *sStyleSpawnScale;
extern s32 sStyleSpawnTableIndex;
/* The first word of the StyleEffectParams block every effect is built from
 * (sStyleSpawnOffsetX .. sStyleSpawnColors, separate symbols in the image). */
extern s32 sStyleSpawnOffsetX;
extern void SetupStyleSpawnParamsRandom(LongVec3 *pos, s32 offsetY);
extern void SetupStyleSpawnParamsDayMod7(LongVec3 *pos, s32 offsetY);

/* Fills `count` slots with kind-0 effects: a table index and scale for all
 * of them, an offset y (0: each setup picks one; a pick of 4 reads the word
 * after sStyleSpawnYChoices), and per slot SetupStyleSpawnParamsRandom, or
 * SetupStyleSpawnParamsDayMod7 on every seventh day. Returns the next slot. */
StyleEffect **StyleFillEffectKind0(StyleEffect **slots, s32 count, LongVec3 *pos) {
    s32 i;
    s32 offsetY;
    void (*setup)(LongVec3 *, s32);

    sStyleSpawnTableIndex = rand() % 7;
    sStyleSpawnScale = sStyleSpawnScales[(u32)rand() % 5];
    offsetY = (u32)rand() % 5;
    if (offsetY != 0) {
        offsetY = sStyleSpawnYChoices[offsetY];
    }
    setup = SetupStyleSpawnParamsDayMod7;
    if (sStyleDay % 7 != 0) {
        setup = SetupStyleSpawnParamsRandom;
    }
    for (i = 0; i < count; i++) {
        setup(pos, offsetY);
        *slots =
            New_StyleEffect(0, (StyleEffectParams *)&sStyleSpawnOffsetX, (SceneNode *)sStyleGrid, pos);
        slots++;
    }
    return slots;
}

extern s32 sStyleSpawnYChoice2;
extern Ratio16 sStyleKind1Scale[];

/* Fills `count` slots with kind-1 effects: sStyleKind1Scale, offset y
 * sStyleSpawnYChoice2. */
StyleEffect **StyleFillEffectKind1(StyleEffect **slots, s32 count, LongVec3 *pos) {
    s32 i;
    s32 offsetY;

    offsetY = sStyleSpawnYChoice2;
    sStyleSpawnScale = sStyleKind1Scale;
    for (i = 0; i < count; i++) {
        SetupStyleSpawnParamsRandom(pos, offsetY);
        *slots =
            New_StyleEffect(1, (StyleEffectParams *)&sStyleSpawnOffsetX, (SceneNode *)sStyleGrid, pos);
        slots++;
    }
    return slots;
}

extern s32 sStyleSpawnColors[];
extern Ratio16 *sStyleSpawnRotation;
extern Ratio16 sStyleSpawnRotations[][3];
extern s32 sStyleSpawnOffsetY;
extern s32 sStyleSpawnOffsetZ;
extern u8 sStyleKind3Colors[][3];

/** @brief sStyleSpawnRotation seen as a one-field struct, through which
 * StyleFillEffectKind3 stores the effect's rotation. */
/* MATCHING: the rotation store goes through a one-field struct, so the
 * sStyleGrid load may schedule above it (a plain pointer store blocks it). */
typedef struct PtrBoxK3 {
    Ratio16 *p; /**< +0x000 the spawn rotation */
} PtrBoxK3;

/* Appends one kind-3 effect. With decor variant active and band colours B
 * its offset and colour are fixed; otherwise its z offset is folded to
 * -30720..0 and its colour is random. */
StyleEffect **StyleFillEffectKind3(StyleEffect **slots, LongVec3 *pos) {
    s32 *offsetZ;
    PtrBoxK3 *rotation;

    SetupStyleSpawnParamsRandom(pos, sStyleSpawnYChoice2);
    if (sStyleDecorVariant != 0 && sStyleDecorColors == sStyleDecorColorsB) {
        sStyleSpawnOffsetX = -45056;
        sStyleSpawnOffsetY = -8192;
        sStyleSpawnOffsetZ = 0;
        sStyleSpawnColors[0] = (s32)sStyleKind3Colors[1];
    } else {
        offsetZ = &sStyleSpawnOffsetZ;
        if (*offsetZ > 0) {
            *offsetZ = -*offsetZ;
        }
        if (*offsetZ < -30720) {
            *offsetZ = -30720;
        }
        sStyleSpawnColors[0] = (s32)sStyleKind3Colors[(u32)rand() % 3];
    }
    rotation = (PtrBoxK3 *)&sStyleSpawnRotation;
    rotation->p = sStyleSpawnRotations[0];
    /* MATCHING: the block's address is taken back from its rotation member */
    *slots = New_StyleEffect(
        3, (StyleEffectParams *)((u8 *)rotation - offsetof(StyleEffectParams, rotation)),
        (SceneNode *)sStyleGrid, pos);
    slots++;
    return slots;
}

extern s32 sStyleKind2AltColor;
extern u8 sStyleKind2Colors[][3];

/** @brief One word of sStyleSpawnColors seen as a one-field struct, through
 * which StyleFillEffectKind2 stores the effect's two colours. */
/* MATCHING: the first colour store goes through a one-field struct, as
 * PtrBoxK3's does, so the sStyleDay load may schedule above it. */
typedef struct S32BoxK2 {
    s32 v; /**< +0x000 a colour: first an RGB triple's address, then sStyleKind2AltColor or 0 */
} S32BoxK2;

/* Appends one kind-2 effect with a random colour and, except on every
 * twentieth day, sStyleKind2AltColor as its alternate colour. */
StyleEffect **StyleFillEffectKind2(StyleEffect **slots, LongVec3 *pos) {
    s32 r;
    s32 altColor;
    S32BoxK2 *color;
    Ratio16 **rotation;

    r = rand();
    color = (S32BoxK2 *)sStyleSpawnColors;
    color->v = (s32)sStyleKind2Colors[(u32)r % 3];
    color++;
    altColor = (sStyleDay / 20) * 20; /* MATCHING: not `% 20`, which jump.c folds */
    if (sStyleDay != altColor) {
        altColor = sStyleKind2AltColor;
    } else {
        altColor = 0;
    }
    color->v = altColor;
    SetupStyleSpawnParamsRandom(pos, sStyleSpawnYChoice2);
    rotation = &sStyleSpawnRotation;
    *rotation = sStyleSpawnRotations[0];
    sStyleSpawnTableIndex = rand() % 6;
    /* MATCHING: the block's address is taken back from its rotation member */
    *slots = New_StyleEffect(
        2, (StyleEffectParams *)((u8 *)rotation - offsetof(StyleEffectParams, rotation)),
        (SceneNode *)sStyleGrid, pos);
    slots++;
    return slots;
}

extern s32 sStyleSpawnModelLayout;

/* Randomises the spawn parameters: offset y (offsetY, or a random choice
 * when 0), x and z offsets of 0..22 steps of 2048 either side, a rotation
 * and a model layout. `pos` is unused. */
/* MATCHING: sStyleSpawnOffsetX is declared a scalar, not an array. */
void SetupStyleSpawnParamsRandom(LongVec3 *pos, s32 offsetY) {
    if (offsetY == 0) {
        offsetY = sStyleSpawnYChoices[rand() & 3];
    }
    sStyleSpawnOffsetY = offsetY;
    sStyleSpawnOffsetX = (rand() % 23) << 11;
    if (rand() & 1) {
        sStyleSpawnOffsetX = -sStyleSpawnOffsetX;
    }
    sStyleSpawnOffsetZ = (rand() % 23) << 11;
    if (rand() & 1) {
        sStyleSpawnOffsetZ = -sStyleSpawnOffsetZ;
    }
    sStyleSpawnRotation = sStyleSpawnRotations[(u32)rand() % 7];
    sStyleSpawnModelLayout = rand() % 5;
}

extern s32 sStyleSpawnYChoice1;

/* The every-seventh-day setup: fixed offset y, x of 0..19 steps of 2048, z
 * by day % 3 (40960, -40960, 2048), then the same rotation and layout
 * picks as SetupStyleSpawnParamsRandom. Both parameters are unused; it has
 * that function's signature because StyleFillEffectKind0 calls either
 * through one pointer. */
/* MATCHING: each rand() is used inline; one local for all three adds a move after every call. */
void SetupStyleSpawnParamsDayMod7(LongVec3 *pos, s32 offsetY) {
    s32 dayMod3;

    rand();
    sStyleSpawnOffsetY = sStyleSpawnYChoice1;
    sStyleSpawnOffsetX = (rand() % 20) << 11;
    dayMod3 = sStyleDay % 3;
    sStyleSpawnOffsetZ = 40960;
    if (dayMod3 == 1) {
        sStyleSpawnOffsetZ = -40960;
    } else if (dayMod3 == 2) {
        sStyleSpawnOffsetZ = 2048;
    }
    sStyleSpawnRotation = sStyleSpawnRotations[(u32)rand() % 7];
    sStyleSpawnModelLayout = rand() % 5;
}

extern StyleCueRecord *FindNextStyleCueInRange(LongVec3 *pos, s32 *outDist, LongVec3 *target);
extern SoundCueCallbackFn sStyleCueCallbacks[];

/* Claims the next record in range for `slot` and starts its cue. A started
 * cue equal to *lastCue is reported back negated. Returns the slot, or NULL. */
StyleCueSlot *TryStartStyleCue(StyleCueSlot *slot, s32 *lastCue, LongVec3 *target, void *unused) {
    StyleCueRecord *entry;

    entry = FindNextStyleCueInRange(&slot->pos, &slot->lastDist, target);
    if (entry != 0) {
        slot->entry = entry;
        InitSoundCueSet(((StyleSceneRefs *)sStyleSceneRefs)->sound, &slot->cueSet, entry->cue, slot,
                        sStyleCueCallbacks[entry->cue]);
        if (entry->cue == *lastCue) {
            *lastCue = -entry->cue;
        }
        entry->cue = -entry->cue;
        return slot;
    }
    return 0;
}

extern s32 sStyleCueRecordIndex;
extern u8 *sStyleCueRecordLists[];
extern u8 sStyleCueRecordCounts[];
extern s32 sStyleCueDistanceTable[];

/* sStyleCueOffsets: the in-cell offsets a record's offsetIndex picks. */
extern CellOffset sStyleCueOffsets[];

/* From sStyleCueRecordIndex on, the first free record of the stage's list
 * whose X+Z distance from the target is under its cue's distance; each
 * record looked at advances the index, so the next slot's search this tick
 * goes on from there.
 * Writes the record's world position and distance. */
StyleCueRecord *FindNextStyleCueInRange(LongVec3 *pos, s32 *outDist, LongVec3 *target) {
    s32 j, remaining;
    u8 *records;
    StyleCueRecord *entry;
    CellKeyDesc buf;
    s32 dx, dz, dist;
    StageMap *grid;

    if (target == 0) {
        goto fail;
    }
    records = sStyleCueRecordLists[sStyleStage];
    remaining = sStyleCueRecordCounts[sStyleStage] - sStyleCueRecordIndex;
    entry = (StyleCueRecord *)(sStyleCueRecordIndex * 8 + (s32)records); /* MATCHING: operand order */
    for (j = 0; j < remaining; j++, entry++) {
        sStyleCueRecordIndex++;
        if (entry->cue > 0) {
            buf.key = entry->key;
            buf.offset = sStyleCueOffsets[entry->offsetIndex];
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

/* Stops the slot's cue and frees its record. Returns NULL for the slot. */
StyleCueSlot *FlushStyleCue(StyleCueSlot *slot) {
    FlushSoundCueSet(((StyleSceneRefs *)sStyleSceneRefs)->sound, &slot->cueSet);
    slot->entry->cue = -slot->entry->cue;
    return 0;
}

extern s32 IsStyleCueNear(StyleCueSlot *slot, LongVec3 *target);

/* One service pass of the slot's cue while the target is in range; 0 otherwise. */
s32 ServiceStyleCueIfNear(StyleCueSlot *slot, LongVec3 *target, void *unused) {
    if (IsStyleCueNear(slot, target) != 0) {
        ServiceSoundCueSet(((StyleSceneRefs *)sStyleSceneRefs)->sound, &slot->cueSet);
        return 1;
    }
    return 0;
}

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
    if (sStyleTickCount++ == 0) {
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

extern DrawRect sStyleStripRectA;
extern DrawPoint sStyleStripScratchA;
extern DrawRect sStyleStripRectB;
extern DrawPoint sStyleStripScratchB;

/* One step of RotateVramRectRight's one-column VRAM rotation: stage 2 on the
 * strip at y 496, stages 3..5 on the one at y 504. */
void StyleScrollVramStrips(void) {
    DrawRect *rect;
    DrawPoint *scratch;
    s32 count;

    if (sStyleStage == 2) {
        rect = &sStyleStripRectA;
        scratch = &sStyleStripScratchA;
        count = 1; /* MATCHING: a local set in each branch, not a literal argument */
    } else if ((u32)(sStyleStage - 3) < 3) {
        count = 1;
        rect = &sStyleStripRectB;
        scratch = &sStyleStripScratchB;
    } else {
        return;
    }
    RotateVramRectRight(rect, count, scratch);
}

/* ---- The style layer's sound cues ---------------------------------------
 *
 *  - StyleCue00..StyleCue13, the 14 rows of sStyleCueCallbacks, and their
 *    helper ComputeStyleCueFalloff. Each is a SoundCueSet callback
 *    (include/sound_cue_set.h): TryStartStyleCue starts a style-cue slot's
 *    embedded set with the claimed cue record's index as the tag and that
 *    row of the table as the callback, as Entity does with the
 *    Entity__Cue* handlers of sEntityMoodTable's rows. Every tick a callback sets the set's
 *    attenuation from the slot's distance to the target and, on the ticks
 *    its pattern selects, requests VAB programs on the three voices; most
 *    restart the pattern by setting `tick` to -1 once it passes a limit.
 *  - IsStyleVariantEven (include/dream_aux.h).
 */

/* ------------------------------------------------------------------ *
 * sStyleCueCallbacks's 14 slots (StyleCue00..StyleCue13) plus the shared
 * helper ComputeStyleCueFalloff they all call first.
 * ------------------------------------------------------------------ */

/* The owner every StyleCueNN callback receives is its StyleCueSlot:
 * TryStartStyleCue passes the slot as InitSoundCueSet's owner and its
 * embedded `cueSet` as the set, so a callback's `set` is `&ctx->cueSet`. */

/* Every callback calls this first; it is defined after them, in ROM order. */
s32 ComputeStyleCueFalloff(StyleCueSlot *ctx);

void StyleCue00(StyleCueSlot *ctx, SoundCueSet *set) {
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

void StyleCue01(StyleCueSlot *ctx, SoundCueSet *set) {
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

void StyleCue02(StyleCueSlot *ctx, SoundCueSet *set) {
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

void StyleCue03(StyleCueSlot *ctx, SoundCueSet *set) {
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

void StyleCue04(StyleCueSlot *ctx, SoundCueSet *set) {
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

void StyleCue05(StyleCueSlot *ctx, SoundCueSet *set) {
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

void StyleCue06(StyleCueSlot *ctx, SoundCueSet *set) {
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

void StyleCue07(StyleCueSlot *ctx, SoundCueSet *set) {
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

void StyleCue08(StyleCueSlot *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick % 20 == 0) {
        set->slots[0].program = 9;
        set->slots[0].octave = 0;
        set->slots[0].vol = 64;
        set->slots[0].endVol = 64;
    }
}

void StyleCue09(StyleCueSlot *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick % 20 == 0) {
        set->slots[0].program = 9;
        set->slots[0].octave = -2;
    }
}

void StyleCue10(StyleCueSlot *ctx, SoundCueSet *set) {
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

void StyleCue11(StyleCueSlot *ctx, SoundCueSet *set) {
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

void StyleCue12(StyleCueSlot *ctx, SoundCueSet *set) {
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

void StyleCue13(StyleCueSlot *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick == 0) {
        set->slots[0].program = 24;
        set->slots[0].octave = 0;
    }
}

/* One range per cue record (15), indexed by the record's cue index: the
 * negative of `cue` while a slot has the record claimed. IsStyleCueNear
 * tests the slot's distance against the same row. */

s32 ComputeStyleCueFalloff(StyleCueSlot *ctx) {
    s32 range = sStyleCueDistanceTable[-ctx->entry->cue];
    s32 stepDist = range / ctx->cueSet.attenuationSteps;

    return ctx->lastDist / stepDist;
}

s32 IsStyleVariantEven(void) {
    return (sStyleVariant & 1) ^ 1;
}
