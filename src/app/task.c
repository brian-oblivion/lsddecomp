/*
 * task.c -- the task classes. In address order:
 *  - StreamTask (include/stream_task.h), whole: it plays one movie stream
 *    through a MoviePlayer inside TaskCore's fade and state machine;
 *  - TaskCore (include/task_core.h), the base of the game's menu and screen
 *    tasks: allocator, ctor, finalize, resetCounters, init and the
 *    onInit/onDeinit hooks, which build and tear down the TileAtlas ->
 *    TileMap -> BgLayer chain, clear the screen and configure the viewport;
 *    then, in sections 1 to 3 below, its pad dispatch, state machine, fades
 *    and menu methods up to its table getter;
 *  - IntermediateBase (include/intermediate_base.h), TaskCore's parent, whole;
 * then the three classes' method tables. Viewport, which they draw
 * through, follows in viewport.c.
 * include/task.h holds the declarations this file shares with
 * screen_widgets.c.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include "task.h"
#include "bmem_pmgr.h"
#include "vab_stream_obj.h"
#include "bg_layer.h"
#include "tile_map.h"
#include "tile_atlas.h"
#include "pad.h"
#include "tim_image.h"
#include "light_rig.h"
#include "frame_clock.h"
#include <strings.h>

/* {x 640, y 0, w 320, h 240}: the default movie frame. TaskCore__OnInit clears
 * it to baseColor when the task has no sub handle. The same three words are
 * StreamTask's default initData and its MoviePlayer's frame, through
 * GetDefaultMovieFrame. */
extern DrawRect sDefaultMovieFrame;

/* resetCounters' colours for setColors, three RGB triples back to back:
 * baseColor {0, 0, 0}, the clear colour {0, 0, 0}, the third {128, 128, 128}. */
extern u8 sTaskCoreDefaultColors[3][3];

/* {0, 0, 0}: TaskCore__OnInit attaches the view with it as both the
 * viewpoint and the reference point. */
extern LongVec3 sTaskCoreViewOrigin;

StreamTask *New_StreamTask(TaskCoreTarget *target, char *soundBankPath, BasicClass *sound,
                           DrawRect *initData) {
    StreamTask *self;

    self = BMemPMgrAlloc(sizeof(StreamTask));
    if (self != NULL) {
        GetStreamTaskMethods()->ctor(self, target, soundBankPath, sound, initData);
        return self;
    }
    return NULL;
}

void StreamTask__StreamTask(StreamTask *self, TaskCoreTarget *target, char *soundBankPath,
                            BasicClass *sound, DrawRect *initData) {
    GetTaskCoreMethods()->ctor((TaskCore *)self, target, soundBankPath, sound);
    self->methods = GetStreamTaskMethods();
    if (initData != NULL) {
        self->initData = *initData;
    } else {
        self->initData = *GetDefaultMovieFrame();
    }
    self->player = New_MoviePlayer(GetDefaultMovieFrame(), 0, 0);
    self->streamName = NULL;
    self->methods->resetCounters(self);
}

void StreamTask__Finalize(StreamTask *self) {
    self->player->methods->release(self->player);
    GetTaskCoreMethods()->finalize((TaskCore *)self);
}

void StreamTask__Reset(StreamTask *self) {
    self->loopCount = -1;
    self->keepActive = 0;
    self->skipOnConfirm = 1;
    self->unkD0 = 0;
    self->abortBeforeFade = 1;
}

void StreamTask__Init(StreamTask *self, IntermediateBaseInitArgs *args, const char *streamName,
                      s32 streamGroup, s32 autoPlay) {
    self->streamName = streamName;
    self->streamGroup = streamGroup;
    self->autoPlay = autoPlay;
    GetTaskCoreMethods()->init((TaskCore *)self, args, INTERMEDIATEBASE_INIT_RUN);
}

void StreamTask__OnInit(StreamTask *self) {
    /* IntermediateBase's onInit slot names init's (0, 0, 0); TaskCore__OnInit
     * takes self alone, and this up-call passes nothing else. */
    ((void (*)(TaskCore *))GetTaskCoreMethods()->onInit)((TaskCore *)self);
    self->playDone = 0;
    self->player->methods->setAutoPlay(self->player, self->autoPlay);
    if (self->player->methods->play(self->player, (char *)self->streamName, self->streamGroup,
                                    self->keepActive, self->loopCount) != 0) {
        self->methods->setFrameBound(self, 0);
    }
}

void StreamTask__Update(StreamTask *self, BasicClass *sender, s32 event) {
    GetTaskCoreMethods()->update((TaskCore *)self, sender, event);
    if (self->playDone != 0) {
        return;
    }
    self->playDone = self->player->methods->advance(self->player);
    if (self->playDone == 0) {
        return;
    }
    if (self->fadingOut != 0) {
        return;
    }
    self->methods->setState(self, TASKCORE_STATE_FADE_OUT);
}

void StreamTask__SetState(StreamTask *self, s32 state) {
    GetTaskCoreMethods()->setState((TaskCore *)self, state);
    switch (state) {
        case TASKCORE_STATE_ACTIVE:
            self->fadingOut = 0;
            break;
        case TASKCORE_STATE_FADE_OUT:
            self->fadingOut = 1;
            break;
        case TASKCORE_STATE_FADED_OUT:
            if (self->abortBeforeFade == 0) {
                self->player->methods->abort(self->player);
            }
            break;
        case STREAMTASK_STATE_SKIPPED:
            self->methods->exit(self);
            break;
    }
}

void StreamTask__SetFrameBound(StreamTask *self, s32 bound) {
    self->frameBound = bound;
    if (bound >= 0) {
        self->frameBound = bound * STREAMTASK_FRAMES_PER_SECOND;
    }
}

void StreamTask__OnPadConfirm(StreamTask *self) {
    GetTaskCoreMethods()->onPadConfirm((TaskCore *)self);
    if (self->skipOnConfirm != 0) {
        self->result = STREAMTASK_RESULT_SKIPPED;
        self->methods->setState(self, STREAMTASK_STATE_SKIPPED);
    }
}

void StreamTask__OnPadPrev(StreamTask *self) {
    GetTaskCoreMethods()->onPadPrev((TaskCore *)self);
}

void StreamTask__OnPadNext(StreamTask *self) {
    GetTaskCoreMethods()->onPadNext((TaskCore *)self);
}

void StreamTask__NoOpSlot88(void) {}

void StreamTask__NoOpSlot8C(void) {}

void StreamTask__Exit(StreamTask *self) {
    if (self->abortBeforeFade != 0) {
        self->player->methods->abort(self->player);
    } else {
        self->methods->setState(self, TASKCORE_STATE_FADE_OUT);
    }
}

void StreamTask__SetKeepActive(StreamTask *self, s32 keepActive) {
    self->keepActive = keepActive;
}

void StreamTask__SetLoopCount(StreamTask *self, s32 count) {
    self->loopCount = count;
}

void StreamTask__SetSkipOnConfirm(StreamTask *self, s32 enable) {
    self->skipOnConfirm = enable;
}

void StreamTask__SetUnkD0(StreamTask *self, s32 value) {
    self->unkD0 = value;
}

void StreamTask__SetAbortBeforeFade(StreamTask *self, s32 enable) {
    self->abortBeforeFade = enable;
}

StreamTaskMethods *GetStreamTaskMethods(void) {
    return &gStreamTaskMethods;
}

TaskCore *New_TaskCore(TaskCoreTarget *target, char *soundBankPath, BasicClass *sound) {
    TaskCore *self;

    self = BMemPMgrAlloc(sizeof(TaskCore));
    if (self != NULL) {
        GetTaskCoreMethods()->ctor(self, target, soundBankPath, sound);
        return self;
    }
    return NULL;
}

void TaskCore__TaskCore(TaskCore *self, TaskCoreTarget *target, char *soundBankPath, BasicClass *sound) {
    struct TileAtlas *atlas;
    struct TileMap *tileMap;
    TaskCoreMethods *methods;

    GetIntermediateBaseMethods()->ctor((IntermediateBase *)self);
    /* MATCHING: one GetTaskCoreMethods() call, kept for setTarget; retail calls it once. */
    methods = GetTaskCoreMethods();
    self->methods = methods;
    methods->setTarget(self, target);
    if (soundBankPath != NULL) {
        self->sound = (BasicClass *)New_VabStreamObj(soundBankPath);
    } else {
        self->sound = sound;
    }
    self->soundBankPath = soundBankPath;
    self->methods->setSubHandle(self, NULL, NULL);
    atlas = New_TileAtlas(0);
    self->tileAtlas = atlas;
    tileMap = New_TileMap(0, atlas);
    self->tileMap = tileMap;
    self->bgLayer = New_BgLayer(tileMap, BGLAYER_MODE_SCREEN);
    self->methods->resetCounters(self);
}

void TaskCore__Finalize(TaskCore *self) {
    self->bgLayer->methods->release(self->bgLayer);
    self->tileMap->methods->release(self->tileMap);
    self->tileAtlas->methods->release(self->tileAtlas);
    if (self->soundBankPath != NULL) {
        self->sound->methods->release(self->sound);
    }
    if (self->subHandlePath != NULL) {
        self->subHandle->methods->release(self->subHandle);
    }
    self->methods->releaseTarget(self);
    GetIntermediateBaseMethods()->finalize((IntermediateBase *)self);
}

void TaskCore__Reset(TaskCore *self) {
    /* MATCHING: the table read once; reading self->methods at each call is a word longer. */
    TaskCoreMethods *methods = self->methods;
    methods->setFrameBound(self, -1);
    methods->setColors(self, sTaskCoreDefaultColors[0], sTaskCoreDefaultColors[1],
                       sTaskCoreDefaultColors[2]);
    methods->setFadeInCallbackEnabled(self, 1);
    methods->setFadeOutCallbackEnabled(self, 1);
    self->fadeRate = 9;
    self->otLength = 3;
    self->maxPackets = 300;
    self->packetSize = 64;
    self->exitCallback = NULL;
    self->exitCallbackCtx = NULL;
    self->clearOnDeinit = 1;
    self->inputMode = TASKCORE_INPUT_NONE;
}

s32 TaskCore__Init(TaskCore *self, IntermediateBaseInitArgs *args, s32 mode) {
    GetIntermediateBaseMethods()->init((IntermediateBase *)self, args, mode);
    return self->result;
}

/* Hangs the slot widgets and the BgLayer under the light rig, sets the
 * fade-in colour, clears the default movie frame (no sub handle) and then the
 * screen to baseColor, and configures and opens the viewport's OT.
 * initArgs->drawSystem and the viewport are `BasicClass *` fields, cast to
 * their classes, DrawSystem and Viewport. */
void TaskCore__OnInit(TaskCore *self) {
    Viewport *viewport;
    ViewportMethods *viewportMethods;

    /* MATCHING: retail loads both before the first call and keeps them to the end. */
    viewport = (Viewport *)self->viewport;
    viewportMethods = viewport->methods;
    self->methods->updateSlotElements(self, self->lightRig);
    self->bgLayer->methods->attachToParent(self->bgLayer, (SceneNode *)self->lightRig, NULL);
    if (self->fadeInCallback != NULL) {
        self->methods->broadcastToSlots(self, self->baseColor);
        self->bgLayer->methods->setColor(self->bgLayer, 1, (ColorRgb *)self->baseColor);
    }
    if (self->subHandle == NULL) {
        ((DrawSystem *)self->initArgs->drawSystem)
            ->methods->clearImage((DrawSystem *)self->initArgs->drawSystem, self->baseColor,
                                  &sDefaultMovieFrame);
    }
    ((DrawSystem *)self->initArgs->drawSystem)
        ->methods->clearImage((DrawSystem *)self->initArgs->drawSystem, self->baseColor, NULL);
    viewportMethods->setOtLength(viewport, self->otLength);
    viewportMethods->setMaxPackets(viewport, self->maxPackets);
    viewportMethods->setPacketSize(viewport, self->packetSize);
    viewportMethods->attachViewChild(viewport, self->lightRig, &sTaskCoreViewOrigin,
                                     &sTaskCoreViewOrigin, NULL);
    viewportMethods->initOt(viewport);
    self->result = TASKCORE_RESULT_DONE;
}

/* Closes the viewport's OT, detaches the view and the BgLayer, and clears the
 * screen to clearColor while clearOnDeinit is set. */
void TaskCore__OnDeinit(TaskCore *self) {
    /* MATCHING: a local; reading self->viewport at each call compiles differently. */
    Viewport *viewport = (Viewport *)self->viewport;
    viewport->methods->deinitOt(viewport);
    viewport->methods->detachViewChild(viewport);
    self->bgLayer->methods->detachFromParent(self->bgLayer);
    if (self->clearOnDeinit != 0) {
        ((DrawSystem *)self->initArgs->drawSystem)
            ->methods->clearImage((DrawSystem *)self->initArgs->drawSystem, self->clearColor, NULL);
    }
}

/* Section 1. TaskCore's own methods from +0x058 to +0x0C0: the pad
 * dispatch and its five button handlers, the state machine (update and
 * setState), the frame bound, the sound call, the exit callback and the
 * fade-in/fade-out pair. Each is the default for its slot in
 * gTaskCoreMethods, which StreamTask, TitleMenu and GraphRoom inherit or
 * override; include/task_core.h's banner describes the class and its states.
 * Section 2 follows with the slot and item-list methods.
 *
 * Input: while inputMode is not NONE, onPadEvent maps a press to a handler;
 * each handler plays the button tone (or moves a cursor) and reports what
 * happened as a state, which setState passes to the parents and then folds
 * back into ACTIVE. Fades: tickFadeIn adds frameCounter * fadeRate to
 * baseColor and pushes the result to every slot widget and the BgLayer,
 * until it passes TASKCORE_FADE_FULL.
 */

void TaskCore__OnPadEvent(TaskCore *self, BasicClass *sender, s32 event) {
    TaskCoreMethods *methods;

    methods = self->methods;
    if (self->inputMode != TASKCORE_INPUT_NONE) {
        /* MATCHING: the case order is retail's arm order (its jump table). */
        switch (event) {
            case PAD_EVENT_PRESSED + PAD_BUTTON_LUP:
                methods->onPadPrev(self);
                break;
            case PAD_EVENT_PRESSED + PAD_BUTTON_LDOWN:
                methods->onPadNext(self);
                break;
            case PAD_EVENT_PRESSED + PAD_BUTTON_START:
                methods->onPadStart(self);
                break;
            case PAD_EVENT_PRESSED + PAD_BUTTON_RDOWN:
                methods->onPadCancel(self);
                break;
            case PAD_EVENT_PRESSED + PAD_BUTTON_RRIGHT:
                methods->onPadConfirm(self);
                break;
        }
    }
}

void TaskCore__Update(TaskCore *self, BasicClass *sender, s32 event) {
    TaskCoreMethods *methods;

    methods = self->methods;
    GetIntermediateBaseMethods()->update((IntermediateBase *)self, sender, event);
    if (self->inputMode != TASKCORE_INPUT_NONE) {
        u32 frames;

        /* MATCHING: frameCounter is loaded before frameBound. */
        frames = self->frameCounter;
        if ((u32)self->frameBound < frames) {
            methods->setState(self, TASKCORE_STATE_TIMED_OUT);
        }
    }
    switch (self->state) {
        case INTERMEDIATEBASE_STATE_START:
            methods->setState(self, TASKCORE_STATE_FADE_IN);
            break;
        case TASKCORE_STATE_FADE_IN:
            methods->tickFadeInCallback(self);
            break;
        case TASKCORE_STATE_FADE_OUT:
            methods->tickFadeOutCallback(self);
            break;
        case TASKCORE_STATE_FADED_OUT:
            methods->setState(self, INTERMEDIATEBASE_STATE_STOP);
            break;
    }
}

void TaskCore__SetState(TaskCore *self, s32 state) {
    TaskCoreMethods *methods;

    methods = self->methods;
    GetIntermediateBaseMethods()->setState((IntermediateBase *)self, state);
    switch (state) {
        case TASKCORE_STATE_ACTIVE:
            methods->broadcastToSlots(self, self->target->unselectedColor);
            methods->setActiveSlot(self, self->target->initialSlot, 0);
            self->frameCounter = 0;
            self->inputMode = TASKCORE_INPUT_CHOOSING_SLOT;
            break;
        case TASKCORE_STATE_TIMED_OUT:
            self->result = TASKCORE_RESULT_TIMED_OUT;
            methods->exit(self);
            break;
        case TASKCORE_STATE_FADE_IN:
        case TASKCORE_STATE_FADE_OUT:
            self->frameCounter = 0;
            self->inputMode = TASKCORE_INPUT_NONE;
            break;
        case TASKCORE_STATE_FADED_OUT:
            self->frameCounter = 0;
            break;
        case TASKCORE_STATE_CURSOR_MOVED:
        case TASKCORE_STATE_START_PRESSED:
        case TASKCORE_STATE_SLOT_CONFIRMED:
        case TASKCORE_STATE_SCROLL_OPENED:
        case TASKCORE_STATE_ITEM_CONFIRMED:
        case TASKCORE_STATE_SCROLL_COMMITTED:
        case TASKCORE_STATE_SCROLL_CANCELLED:
            self->state = TASKCORE_STATE_ACTIVE;
            self->frameCounter = 0;
            switch (state) {
                case TASKCORE_STATE_SLOT_CONFIRMED:
                    methods->confirmSlot(self);
                    break;
                case TASKCORE_STATE_ITEM_CONFIRMED:
                    methods->commitElementScroll(self);
                    break;
                case TASKCORE_STATE_SCROLL_CANCELLED:
                    methods->cancelElementScroll(self);
                    break;
            }
            break;
    }
}

void TaskCore__SetFrameBound(TaskCore *self, s32 bound) {
    self->frameBound = bound;
    if (bound >= 0) {
        self->frameBound = bound * TASKCORE_FRAMES_PER_SECOND;
    }
}

void TaskCore__PlaySound(TaskCore *self, s32 tone) {
    VabStreamObj *sound;

    sound = (VabStreamObj *)self->sound;
    if (sound != NULL) {
        sound->methods->playTone(sound, tone, TASKCORE_TONE_VOLUME, TASKCORE_TONE_VOLUME);
    }
}

void TaskCore__OnPadStart(TaskCore *self) {
    if (self->target != NULL) {
        self->methods->playSound(self, TASKCORE_TONE_BUTTON);
        self->methods->setState(self, TASKCORE_STATE_START_PRESSED);
    }
}

void TaskCore__OnPadConfirm(TaskCore *self) {
    s32 state;

    if (self->target != NULL) {
        self->methods->playSound(self, TASKCORE_TONE_BUTTON);
        state = TASKCORE_STATE_ITEM_CONFIRMED;
        if (self->inputMode == TASKCORE_INPUT_CHOOSING_SLOT) {
            state = TASKCORE_STATE_SLOT_CONFIRMED;
        }
        self->methods->setState(self, state);
    }
}

void TaskCore__OnPadCancel(TaskCore *self) {
    if (self->target != NULL && self->inputMode != TASKCORE_INPUT_CHOOSING_SLOT) {
        self->methods->playSound(self, TASKCORE_TONE_BUTTON);
        self->methods->setState(self, TASKCORE_STATE_SCROLL_CANCELLED);
    }
}

void TaskCore__OnPadPrev(TaskCore *self) {
    void (*handler)(TaskCore *self); /* MATCHING: one call site, not one per arm */

    if (self->target == NULL) {
        return;
    }
    if (self->inputMode == TASKCORE_INPUT_CHOOSING_SLOT) {
        handler = self->methods->findPrevFreeSlot;
    } else if (self->inputMode == TASKCORE_INPUT_SCROLLING) {
        handler = self->methods->retreatSlotCursor;
    } else {
        return;
    }
    handler(self);
}

void TaskCore__OnPadNext(TaskCore *self) {
    void (*handler)(TaskCore *self); /* MATCHING: one call site, not one per arm */

    if (self->target == NULL) {
        return;
    }
    if (self->inputMode == TASKCORE_INPUT_CHOOSING_SLOT) {
        handler = self->methods->findNextFreeSlot;
    } else if (self->inputMode == TASKCORE_INPUT_SCROLLING) {
        handler = self->methods->advanceSlotCursor;
    } else {
        return;
    }
    handler(self);
}

void TaskCore__ConfirmSlot(TaskCore *self) {
    TaskCoreTarget *target;
    s32 slot;

    target = self->target;
    slot = self->activeSlot;
    if (target->slotLists[slot] != NULL) {
        self->methods->beginElementScroll(self);
    } else if (slot == target->exitSlot) {
        self->methods->exit(self);
    }
}

void TaskCore__Exit(TaskCore *self) {
    if (self->exitCallback != NULL) {
        self->exitCallback(self->exitCallbackCtx);
    }
    self->methods->setState(self, TASKCORE_STATE_FADE_OUT);
}

void TaskCore__SetExitCallback(TaskCore *self, void (*callback)(void *ctx), void *ctx) {
    self->exitCallback = callback;
    self->exitCallbackCtx = ctx;
}

void TaskCore__SetFadeInCallbackEnabled(TaskCore *self, s32 enable) {
    TaskCoreMethods *methods;

    methods = self->methods; /* MATCHING: loaded before the switch, on every path */
    switch (enable) {
        case 0:
            self->fadeInCallback = NULL;
            break;
        case 1:
            self->fadeInCallback = methods->tickFadeIn;
            break;
    }
}

void TaskCore__SetFadeOutCallbackEnabled(TaskCore *self, s32 enable) {
    TaskCoreMethods *methods;

    methods = self->methods; /* MATCHING: loaded before the switch, on every path */
    switch (enable) {
        case 0:
            self->fadeOutCallback = NULL;
            break;
        case 1:
            self->fadeOutCallback = methods->tickFadeOut;
            break;
    }
}

/* baseColor is where the fade-in starts; clearColor is what onDeinit clears the
 * screen to (TaskCore__Reset's defaults: black, black, 128 grey). */
/* MATCHING: each colour copied as one ColorRgb; byte-by-byte copies compile differently. */
void TaskCore__SetColors(TaskCore *self, u8 *base, u8 *clear, u8 *color96) {
    *(ColorRgb *)self->baseColor = *(ColorRgb *)base;
    *(ColorRgb *)self->clearColor = *(ColorRgb *)clear;
    *(ColorRgb *)self->unk96 = *(ColorRgb *)color96;
}

void TaskCore__SetFadeRate(TaskCore *self, s32 rate) {
    self->fadeRate = rate;
}

s32 TaskCore__TickFadeInCallback(TaskCore *self) {
    s32 done;

    done = 1;
    if (self->fadeInCallback != NULL) {
        done = self->fadeInCallback(self);
    }
    if (done != 0) {
        self->methods->setState(self, TASKCORE_STATE_ACTIVE);
    }
    return done;
}

s32 TaskCore__TickFadeIn(TaskCore *self) {
    s32 level;
    u8 color[3];

    level = self->frameCounter * self->fadeRate;
    /* MATCHING: level first; base + level puts each add's operands the other way round. */
    color[0] = level + self->baseColor[0];
    color[1] = level + self->baseColor[1];
    color[2] = level + self->baseColor[2];
    self->methods->broadcastToSlots(self, color);
    self->bgLayer->methods->setColor(self->bgLayer, 1, (ColorRgb *)color);
    return (u8)level > TASKCORE_FADE_FULL;
}

s32 TaskCore__TickFadeOutCallback(TaskCore *self) {
    s32 done;

    done = 1;
    if (self->fadeOutCallback != NULL) {
        done = self->fadeOutCallback(self);
        if (done == 0) {
            goto epilogue; /* MATCHING: an early return here returns a constant 0, not done */
        }
    }
    self->methods->setState(self, TASKCORE_STATE_FADED_OUT);
epilogue:
    return done;
}

/*
 * Section 2. TaskCore's menu methods, gTaskCoreMethods +0x0C4 to +0x11C: the
 * fade-out tick, the sub handle, and the two-level picker over the menu
 * description in `target` (include/task_core.h, TaskCore's "picker").
 */

/* New_BoxFill's size and colour for listView: (320, 240), (32, 32, 64). */
extern s32 sListViewSize[2];
extern ColorRgb sListViewColor;

/* The scrolled list's layout (RefreshSlotView, CommitElementScroll): item
 * rows are SLOT_LIST_ROW_PITCH apart, and listView, the frame behind them,
 * is SLOT_LIST_FRAME_WIDTH wide and SLOT_LIST_FRAME_ROW_HEIGHT tall a row. */
#define SLOT_LIST_ROW_PITCH 10
#define SLOT_LIST_FRAME_WIDTH 40
#define SLOT_LIST_FRAME_ROW_HEIGHT 12

s32 TaskCore__TickFadeOut(TaskCore *self) {
    s32 level = TASKCORE_FADE_FULL - (self->frameCounter * self->fadeRate);
    u8 color[3];

    color[0] = level;
    color[1] = level;
    color[2] = level;
    self->methods->broadcastToSlots(self, color);
    self->bgLayer->methods->setColor(self->bgLayer, 1, (ColorRgb *)color);
    return (u8)level > TASKCORE_FADE_FULL;
}

void TaskCore__SetSubHandle(TaskCore *self, const char *path, BasicClass *handle) {
    if (path != NULL) {
        if (self->subHandlePath != NULL) {
            self->subHandle->methods->release(self->subHandle);
        }
        self->subHandle = (BasicClass *)New_TimImage((char *)path);
        ((TimImageUploadFn)((TimImage *)self->subHandle)->methods->processBuffer)(
            (TimImage *)self->subHandle);
        ((TimImage *)self->subHandle)->methods->freeBuffer((TimImage *)self->subHandle);
    } else {
        self->subHandle = handle;
    }
    self->subHandlePath = path;
}

void TaskCore__SetTarget(TaskCore *self, TaskCoreTarget *target) {
    char **names;
    s32 count;
    s32 size;
    TextRow **widget;
    TimImage *texture;
    s32 i;

    self->target = target;
    if (target == NULL) {
        return;
    }

    names = target->names;
    count = 0;
    while (*names++ != NULL) {
        count++;
    }
    size = count * sizeof(void *);
    widget = BMemPMgrAlloc(size);
    self->slotElements = (BasicClass **)widget;
    self->itemCounts = BMemPMgrAlloc(size);
    self->itemCursors = BMemPMgrAlloc(size);
    self->itemLists = BMemPMgrAlloc(size);
    self->slotCount = count;

    if (target->path != NULL) {
        texture = New_TimImage((char *)target->path);
        ((TimImageUploadFn)texture->methods->processBuffer)(texture);
        texture->methods->freeBuffer(texture);
    } else {
        texture = (TimImage *)target->handle;
    }

    names = target->names;
    i = 0;
    if (*names != NULL) {
        do {
            TaskCoreItemList *itemList = target->slotLists[i];
            s32 len = strlen(*names);

            *widget = New_TextRow(texture, len, *names);
            widget++;
            if (itemList != NULL) {
                self->activeSlot = i;
                self->methods->createSlotElements(self, itemList, texture);
            }
            names++;
            i++;
        } while (*names != NULL);
    }

    self->listView = New_BoxFill(sListViewSize, &sListViewColor, 0);
    target->handle = (BasicClass *)texture;
}

void TaskCore__ReleaseTarget(TaskCore *self) {
    TextRow **widget;
    s32 i;

    if (self->target == NULL) {
        return;
    }
    if (self->target->path != NULL) {
        TimImage *texture = (TimImage *)self->target->handle;
        texture->methods->release(texture);
    }
    self->listView->methods->release(self->listView);
    widget = (TextRow **)self->slotElements;
    for (i = 0; i < self->slotCount; widget++) {
        TextRow *row;

        if (self->target->slotLists[i] != NULL) {
            self->activeSlot = i;
            self->methods->releaseSlotElements(self);
        }
        row = *widget;
        row->methods->release(row);
        i++;
    }
    BMemPMgrFree(self->itemLists);
    BMemPMgrFree(self->itemCursors);
    BMemPMgrFree(self->itemCounts);
    BMemPMgrFree(self->slotElements);
}

void TaskCore__UpdateSlotElements(TaskCore *self, void *parent) {
    TextRow **widget;
    ScreenSpritePos *position;
    s32 i;

    if (self->target == NULL) {
        return;
    }
    widget = (TextRow **)self->slotElements;
    position = self->target->slotPositions;
    for (i = 0; i < self->slotCount; i++, widget++, position++) {
        if (self->target->hiddenSlots[i] == NULL) {
            TextRow *row = *widget;

            row->methods->attachToParent(row, parent, (LongVec3 *)position);
            if (self->target->slotLists[i] != NULL) {
                self->activeSlot = i;
                self->methods->refreshSlotView(self, parent, 0);
            }
        } else {
            TextRow *row = *widget;

            row->methods->detachFromParent(row);
        }
    }
}

void TaskCore__BroadcastToSlots(TaskCore *self, void *color) {
    s32 savedSlot;
    TextRow **widget;
    s32 i;

    if (self->target == NULL) {
        return;
    }
    widget = (TextRow **)self->slotElements;
    savedSlot = self->activeSlot;
    for (i = 0; i < self->slotCount;) {
        TextRow *row = *widget;

        widget++;
        row->methods->setColor(row, color);
        if (self->target->slotLists[i] != NULL) {
            self->activeSlot = i;
            self->methods->broadcastToSlotElements(self, color);
        }
        i++;
        /* MATCHING: an ordering barrier; without it i++ is scheduled beside the slotCount read. */
        __asm__("");
    }
    self->activeSlot = savedSlot;
}

void TaskCore__FindNextFreeSlot(TaskCore *self) {
    s32 i;

    if (self->target == NULL) {
        return;
    }
    i = self->activeSlot;
    i++;
    for (;;) {
        if (i >= self->slotCount) {
            i = 0;
        }
        if (i == self->activeSlot) {
            break;
        }
        if (self->target->hiddenSlots[i++] != NULL) {
            continue;
        }
        i--;
        break;
    }
    self->methods->setActiveSlot(self, i, 1);
}

void TaskCore__FindPrevFreeSlot(TaskCore *self) {
    s32 i;

    if (self->target == NULL) {
        return;
    }
    i = self->activeSlot;
    i--;
    for (;;) {
        if (i < 0) {
            i = self->slotCount - 1;
        }
        if (i == self->activeSlot) {
            break;
        }
        if (self->target->hiddenSlots[i--] != NULL) {
            continue;
        }
        i++;
        break;
    }
    self->methods->setActiveSlot(self, i, 1);
}

void TaskCore__SetActiveSlot(TaskCore *self, s32 slot, s32 withSound) {
    s32 prev;
    TextRow *prevWidget;
    TextRow *nextWidget;

    if (self->target == NULL) {
        return;
    }
    prev = self->activeSlot;
    prevWidget = ((TextRow **)self->slotElements)[prev];
    nextWidget = ((TextRow **)self->slotElements)[slot];
    if (prev >= 0) {
        prevWidget->methods->setColor(prevWidget, (ColorRgb *)self->target->unselectedColor);
    }
    nextWidget->methods->setColor(nextWidget, (ColorRgb *)self->target->selectedColor);
    self->activeSlot = slot;
    if (withSound != 0) {
        self->methods->playSound(self, TASKCORE_TONE_CURSOR);
    }
    self->methods->setState(self, TASKCORE_STATE_CURSOR_MOVED);
}

s32 TaskCore__GetActiveSlot(TaskCore *self) {
    return self->activeSlot;
}

void TaskCore__CreateSlotElements(TaskCore *self, TaskCoreItemList *list, void *texture) {
    char **names;
    s32 slot;
    s32 count;
    TextRow **item;

    names = list->itemNames;
    slot = self->activeSlot;
    count = 0;
    while (*names++ != NULL) {
        count++;
    }
    item = BMemPMgrAlloc(count * sizeof(TextRow *));
    self->itemLists[slot] = (void *)item;
    self->itemCursors[slot] = list->savedCursor;
    self->itemCounts[slot] = count;

    names = list->itemNames;
    if (*names != NULL) {
        do {
            s32 len = strlen(*names);

            *item = New_TextRow(texture, len, *names);
            names++;
            item++;
        } while (*names != NULL);
    }
}

void TaskCore__ReleaseSlotElements(TaskCore *self) {
    ReleaseBasicClassArray(self->itemLists[self->activeSlot], self->itemCounts[self->activeSlot]);
    BMemPMgrFree(self->itemLists[self->activeSlot]);
}

void TaskCore__RefreshSlotView(TaskCore *self, void *parent, s32 show) {
    s32 slot;
    TextRow **item;
    s32 count;
    s32 cursor;
    ScreenSpritePos pos;
    s32 i;

    slot = self->activeSlot;
    item = (TextRow **)self->itemLists[slot];
    {
        TaskCoreItemList *list = self->target->slotLists[slot];

        count = self->itemCounts[slot];
        cursor = list->savedCursor;
    }

    for (i = 0; i < count; i++) {
        (*item)->methods->detachFromParent(*item);
        item++;
    }

    pos = self->target->slotLists[slot]->pos;
    pos.y -= cursor * SLOT_LIST_ROW_PITCH;

    if (show != 0) {
        s32 size[2];

        ((BoxFillAttachToParentFn)((BoxFill *)self->listView)->methods->attachToParent)(
            (BoxFill *)self->listView, (SceneNode *)self->lightRig, (BoxFillPos *)&pos);
        size[0] = SLOT_LIST_FRAME_WIDTH;
        size[1] = count * SLOT_LIST_FRAME_ROW_HEIGHT;
        ((BoxFill *)self->listView)->methods->setSize((BoxFill *)self->listView, size);
    } else {
        ((BoxFill *)self->listView)->methods->detachFromParent((BoxFill *)self->listView);
    }

    item = (TextRow **)self->itemLists[slot];
    for (i = 0; i < count; i++) {
        (*item)->methods->attachToParent(*item, parent, (LongVec3 *)&pos);
        (*item)->methods->setDisplay(*item, show);
        pos.y += SLOT_LIST_ROW_PITCH;
        item++;
    }

    item = (TextRow **)self->itemLists[slot];
    item[cursor]->methods->setDisplay(item[cursor], 1);
}

void TaskCore__BroadcastToSlotElements(TaskCore *self, void *color) {
    s32 slot = self->activeSlot;
    TextRow **item = (TextRow **)self->itemLists[slot];
    s32 count = self->itemCounts[slot];
    s32 i;

    for (i = 0; i < count; i++) {
        TextRow *row = *item;
        item++;
        row->methods->setColor(row, color);
    }
}

void TaskCore__BeginElementScroll(TaskCore *self) {
    s32 slot;
    TextRow *item;
    ColorRgb *cursorColor;

    if (self->inputMode != TASKCORE_INPUT_CHOOSING_SLOT) {
        return;
    }
    slot = self->activeSlot;
    self->methods->refreshSlotView(self, self->lightRig, 1);
    item = ((TextRow **)self->itemLists[slot])[self->itemCursors[slot]];
    cursorColor = &self->target->slotLists[slot]->cursorColor;
    item->methods->setColor(item, cursorColor);
    self->inputMode = TASKCORE_INPUT_SCROLLING;
    self->methods->setState(self, TASKCORE_STATE_SCROLL_OPENED);
}

void TaskCore__CommitElementScroll(TaskCore *self) {
    s32 slot;
    s32 cursor;
    ScreenSpritePos pos;
    TextRow **item;
    s32 count;
    s32 i;

    if (self->inputMode != TASKCORE_INPUT_SCROLLING) {
        return;
    }
    slot = self->activeSlot;
    cursor = self->itemCursors[slot];
    pos = self->target->slotLists[slot]->pos;
    pos.y -= cursor * SLOT_LIST_ROW_PITCH;

    item = (TextRow **)self->itemLists[slot];
    count = self->itemCounts[slot];
    for (i = 0; i < count; i++) {
        (*item)->methods->setDisplay(*item, 0);
        (*item)->methods->setPosition(*item, &pos);
        pos.y += SLOT_LIST_ROW_PITCH;
        item++;
    }

    {
        TextRow *row = ((TextRow **)self->itemLists[slot])[cursor];

        row->methods->setDisplay(row, 1);
        row->methods->setColor(row, (ColorRgb *)self->target->unselectedColor);
    }

    self->target->slotLists[slot]->savedCursor = cursor;

    ((BoxFill *)self->listView)->methods->detachFromParent((BoxFill *)self->listView);

    self->inputMode = TASKCORE_INPUT_CHOOSING_SLOT;
    self->methods->setState(self, TASKCORE_STATE_SCROLL_COMMITTED);
}

void TaskCore__CancelElementScroll(TaskCore *self) {
    s32 slot;
    s32 cursor;
    TextRow **items;
    TextRow *prevItem;
    TextRow *savedItem;
    s32 saved;

    if (self->inputMode != TASKCORE_INPUT_SCROLLING) {
        return;
    }
    slot = self->activeSlot;
    cursor = self->itemCursors[slot];
    self->methods->refreshSlotView(self, self->lightRig, 0);
    items = (TextRow **)self->itemLists[slot];
    prevItem = items[cursor];
    prevItem->methods->setColor(prevItem, (ColorRgb *)self->target->unselectedColor);
    saved = self->target->slotLists[slot]->savedCursor;
    self->itemCursors[slot] = saved;
    savedItem = items[saved];
    savedItem->methods->setDisplay(savedItem, 1);
    self->inputMode = TASKCORE_INPUT_CHOOSING_SLOT;
    self->methods->setState(self, TASKCORE_STATE_SCROLL_CANCELLED);
}

void TaskCore__AdvanceSlotCursor(TaskCore *self) {
    s32 slot = self->activeSlot;
    s32 cursor = self->itemCursors[slot];

    cursor++;
    if (cursor >= self->itemCounts[slot]) {
        cursor = 0;
    }
    self->methods->setSlotCursor(self, cursor, 1);
}

void TaskCore__RetreatSlotCursor(TaskCore *self) {
    s32 slot = self->activeSlot;
    s32 cursor = self->itemCursors[slot];

    cursor--;
    if (cursor < 0) {
        cursor = self->itemCounts[slot] - 1;
    }
    self->methods->setSlotCursor(self, cursor, 1);
}

void TaskCore__SetSlotCursor(TaskCore *self, s32 cursor, s32 withSound) {
    s32 slot;
    s32 prev;
    TextRow **items;
    TextRow *prevItem;
    TextRow *nextItem;
    ColorRgb *cursorColor;

    slot = self->activeSlot;
    prev = self->itemCursors[slot];
    items = (TextRow **)self->itemLists[slot];
    prevItem = items[prev];
    nextItem = items[cursor];
    prevItem->methods->setColor(prevItem, (ColorRgb *)self->target->unselectedColor);
    cursorColor = &self->target->slotLists[slot]->cursorColor;
    nextItem->methods->setColor(nextItem, cursorColor);
    self->itemCursors[slot] = cursor;
    if (withSound != 0) {
        self->methods->playSound(self, TASKCORE_TONE_CURSOR);
    }
    self->methods->setState(self, TASKCORE_STATE_CURSOR_MOVED);
}

/* Section 3. IntermediateBase's methods, and three accessors ahead of
 * them.
 *
 * TaskCore__GetActiveItemCursor, GetTaskCoreMethods and
 * GetDefaultMovieFrame come first: one TaskCore method and two plain
 * accessors for data used far more widely (task.c, graph_room.c).
 *
 * Then IntermediateBase (include/intermediate_base.h, whose banner says what
 * the class does): the ctor, onNotify's split by the sender's root class,
 * the counters, init and deinit, the VSync handler that ticks the frame
 * clock and polls the pad, setState with its two state hooks (which start
 * and stop the DrawSystem), and the table getter.
 */

s32 TaskCore__GetActiveItemCursor(TaskCore *self) {
    return self->itemCursors[self->activeSlot];
}

TaskCoreMethods *GetTaskCoreMethods(void) {
    return &gTaskCoreMethods;
}

DrawRect *GetDefaultMovieFrame(void) {
    return &sDefaultMovieFrame;
}

void IntermediateBase__IntermediateBase(IntermediateBase *self) {
    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetIntermediateBaseMethods();
    self->methods->resetCounters(self);
}

void IntermediateBase__OnNotify(IntermediateBase *self, BasicClass *sender, s32 event) {
    s32 rootClass;

    GetBasicClassMethods()->onNotify((BasicClass *)self, sender, event);
    rootClass = sender->methods->header & CLASS_ID_ROOT_MASK;
    if (rootClass == DRAWSYSTEM_CLASS_ID) {
        self->methods->onDrawSystemEvent(self, sender, event);
    } else if (rootClass == PAD_CLASS_ID) {
        self->methods->onPadEvent(self, sender, event);
    } else if (rootClass == FRAMECLOCK_CLASS_ID) {
        self->methods->update(self, sender, event);
    }
}

void IntermediateBase__ResetCounters(IntermediateBase *self) {
    self->frameCounter = 0;
    self->state = 0;
}

void IntermediateBase__Init(IntermediateBase *self, IntermediateBaseInitArgs *args, s32 mode) {
    IntermediateBaseMethods *methods;
    BasicClass *viewport;

    methods = self->methods;
    if (args->frameClock != NULL) {
        self->frameClock = args->frameClock;
    } else {
        self->frameClock = (BasicClass *)New_FrameClock();
    }
    if (args->lightRig != NULL) {
        self->lightRig = args->lightRig;
    } else {
        self->lightRig = (BasicClass *)New_LightRig();
    }
    if (args->viewport != NULL) {
        self->viewport = args->viewport;
    } else {
        self->viewport = (BasicClass *)New_Viewport();
    }
    self->initArgs = args;
    viewport = self->viewport;
    methods->addChild(self, args->drawSystem);
    methods->addChild(self, args->pad);
    methods->addChild(self, self->frameClock);
    methods->onInit(self, 0, 0, 0);
    self->initMode = mode;
    if (mode == INTERMEDIATEBASE_INIT_RUN) {
        viewport->methods->addChild(viewport, args->drawSystem);
        viewport->methods->addChild(viewport, self->frameClock);
        self->lightRig->methods->addChild(self->lightRig, self->frameClock);
        methods->setState(self, INTERMEDIATEBASE_STATE_START);
        methods->deinit(self);
    }
}

void IntermediateBase__Deinit(IntermediateBase *self) {
    IntermediateBaseMethods *methods;
    BasicClass *viewport;

    methods = self->methods;
    methods->onDeinit(self);
    viewport = self->viewport;
    if (self->initMode == INTERMEDIATEBASE_INIT_RUN) {
        self->lightRig->methods->removeChild(self->lightRig, self->frameClock);
        viewport->methods->removeChild(viewport, self->frameClock);
        viewport->methods->removeChild(viewport, self->initArgs->drawSystem);
    }
    methods->removeChild(self, self->frameClock);
    methods->removeChild(self, self->initArgs->pad);
    methods->removeChild(self, self->initArgs->drawSystem);
    if (self->initArgs->viewport != viewport) {
        self->viewport = viewport->methods->release(viewport);
    }
    if (self->initArgs->lightRig != self->lightRig) {
        self->lightRig = self->lightRig->methods->release(self->lightRig);
    }
    if (self->initArgs->frameClock != self->frameClock) {
        self->frameClock = self->frameClock->methods->release(self->frameClock);
    }
}

void IntermediateBase__OnDrawSystemEvent(IntermediateBase *self, BasicClass *sender, s32 event) {
    Pad *pad;

    if (event == DRAWSYSTEM_EVENT_VSYNC) {
        ((FrameClock *)self->frameClock)->methods->tick((FrameClock *)self->frameClock);
        pad = (Pad *)self->initArgs->pad;
        pad->methods->updateMasks(pad);
        pad->methods->dispatchEvents(pad);
    }
}

void IntermediateBase__IncrementFrameCounter(IntermediateBase *self) {
    self->frameCounter++;
}

/* MATCHING: the two state hooks are one call through a slot picked per arm; a call in each arm
 * compiles differently. */
void IntermediateBase__SetState(IntermediateBase *self, s32 state) {
    IntermediateBaseMethods *methods;
    void (*fn)(IntermediateBase *);

    methods = self->methods;
    self->state = state;
    methods->notifyParents(self, state);
    if (state == INTERMEDIATEBASE_STATE_START) {
        fn = methods->onStart;
    } else if (state == INTERMEDIATEBASE_STATE_STOP) {
        fn = methods->onStop;
    } else {
        return;
    }
    fn(self);
}

void IntermediateBase__OnStart(IntermediateBase *self) {
    DrawSystem *drawSystem;

    self->frameCounter = 0;
    drawSystem = (DrawSystem *)self->initArgs->drawSystem;
    drawSystem->methods->start(drawSystem);
}

void IntermediateBase__OnStop(IntermediateBase *self) {
    DrawSystem *drawSystem;

    drawSystem = (DrawSystem *)self->initArgs->drawSystem;
    drawSystem->methods->stop(drawSystem);
    self->frameCounter = 0;
}

IntermediateBaseMethods *GetIntermediateBaseMethods(void) {
    return &gIntermediateBaseMethods;
}

/* The task classes' method tables and TaskCore's three constants, in the order the image keeps them. A (void *) entry is a
 * function whose declared type differs from its slot's: a method inherited
 * from a parent class and declared on the parent's type, or an empty method
 * declared (void). */

/* StreamTask (include/stream_task.h): TaskCore's table with the movie
 * player's reset, init, onInit, update, state machine and pad handlers, then
 * its five setters. */
StreamTaskMethods gStreamTaskMethods = {
    /* +0x000 header */ STREAMTASK_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ StreamTask__StreamTask,
    /* +0x00C finalize */ StreamTask__Finalize,
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
    /* +0x038 onNotify */ (void *)IntermediateBase__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 resetCounters */ StreamTask__Reset,
    /* +0x044 init */ (void *)StreamTask__Init,
    /* +0x048 deinit */ (void *)IntermediateBase__Deinit,
    /* +0x04C onInit */ (void *)StreamTask__OnInit,
    /* +0x050 onDeinit */ (void *)TaskCore__OnDeinit,
    /* +0x054 onDrawSystemEvent */ (void *)IntermediateBase__OnDrawSystemEvent,
    /* +0x058 onPadEvent */ (void *)TaskCore__OnPadEvent,
    /* +0x05C update */ StreamTask__Update,
    /* +0x060 setState */ StreamTask__SetState,
    /* +0x064 onStart */ (void *)IntermediateBase__OnStart,
    /* +0x068 onStop */ (void *)IntermediateBase__OnStop,
    /* +0x06C setFrameBound */ StreamTask__SetFrameBound,
    /* +0x070 playSound */ (void *)TaskCore__PlaySound,
    /* +0x074 onPadStart */ (void *)TaskCore__OnPadStart,
    /* +0x078 onPadConfirm */ StreamTask__OnPadConfirm,
    /* +0x07C onPadCancel */ (void *)TaskCore__OnPadCancel,
    /* +0x080 onPadPrev */ StreamTask__OnPadPrev,
    /* +0x084 onPadNext */ StreamTask__OnPadNext,
    /* +0x088 slot88 */ StreamTask__NoOpSlot88,
    /* +0x08C slot8C */ StreamTask__NoOpSlot8C,
    /* +0x090 confirmSlot */ (void *)TaskCore__ConfirmSlot,
    /* +0x094 exit */ StreamTask__Exit,
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
    /* +0x0D8 setTarget */ (void *)TaskCore__SetTarget,
    /* +0x0DC releaseTarget */ (void *)TaskCore__ReleaseTarget,
    /* +0x0E0 updateSlotElements */ (void *)TaskCore__UpdateSlotElements,
    /* +0x0E4 broadcastToSlots */ (void *)TaskCore__BroadcastToSlots,
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
    /* +0x124 setKeepActive */ StreamTask__SetKeepActive,
    /* +0x128 setLoopCount */ StreamTask__SetLoopCount,
    /* +0x12C setSkipOnConfirm */ StreamTask__SetSkipOnConfirm,
    /* +0x130 setUnkD0 */ StreamTask__SetUnkD0,
    /* +0x134 setAbortBeforeFade */ StreamTask__SetAbortBeforeFade,
};

/* TaskCore (include/task_core.h): IntermediateBase's table with the pad
 * dispatch, the state machine and the fades, then the slot and cursor
 * methods of a menu. */
TaskCoreMethods gTaskCoreMethods = {
    /* +0x000 header */ TASKCORE_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ TaskCore__TaskCore,
    /* +0x00C finalize */ TaskCore__Finalize,
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
    /* +0x038 onNotify */ (void *)IntermediateBase__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 resetCounters */ TaskCore__Reset,
    /* +0x044 init */ TaskCore__Init,
    /* +0x048 deinit */ (void *)IntermediateBase__Deinit,
    /* +0x04C onInit */ (void *)TaskCore__OnInit,
    /* +0x050 onDeinit */ TaskCore__OnDeinit,
    /* +0x054 onDrawSystemEvent */ (void *)IntermediateBase__OnDrawSystemEvent,
    /* +0x058 onPadEvent */ TaskCore__OnPadEvent,
    /* +0x05C update */ TaskCore__Update,
    /* +0x060 setState */ TaskCore__SetState,
    /* +0x064 onStart */ (void *)IntermediateBase__OnStart,
    /* +0x068 onStop */ (void *)IntermediateBase__OnStop,
    /* +0x06C setFrameBound */ TaskCore__SetFrameBound,
    /* +0x070 playSound */ TaskCore__PlaySound,
    /* +0x074 onPadStart */ TaskCore__OnPadStart,
    /* +0x078 onPadConfirm */ TaskCore__OnPadConfirm,
    /* +0x07C onPadCancel */ TaskCore__OnPadCancel,
    /* +0x080 onPadPrev */ TaskCore__OnPadPrev,
    /* +0x084 onPadNext */ TaskCore__OnPadNext,
    /* +0x088 slot88 */ NULL,
    /* +0x08C slot8C */ NULL,
    /* +0x090 confirmSlot */ TaskCore__ConfirmSlot,
    /* +0x094 exit */ TaskCore__Exit,
    /* +0x098 setExitCallback */ TaskCore__SetExitCallback,
    /* +0x09C setFadeInCallbackEnabled */ TaskCore__SetFadeInCallbackEnabled,
    /* +0x0A0 setFadeOutCallbackEnabled */ TaskCore__SetFadeOutCallbackEnabled,
    /* +0x0A4 setColors */ TaskCore__SetColors,
    /* +0x0A8 setFadeRate */ TaskCore__SetFadeRate,
    /* +0x0AC tickFadeInCallback */ TaskCore__TickFadeInCallback,
    /* +0x0B0 tickFadeIn */ TaskCore__TickFadeIn,
    /* +0x0B4 slotB4 */ NULL,
    /* +0x0B8 slotB8 */ NULL,
    /* +0x0BC slotBC */ NULL,
    /* +0x0C0 tickFadeOutCallback */ TaskCore__TickFadeOutCallback,
    /* +0x0C4 tickFadeOut */ TaskCore__TickFadeOut,
    /* +0x0C8 slotC8 */ NULL,
    /* +0x0CC slotCC */ NULL,
    /* +0x0D0 slotD0 */ NULL,
    /* +0x0D4 setSubHandle */ TaskCore__SetSubHandle,
    /* +0x0D8 setTarget */ TaskCore__SetTarget,
    /* +0x0DC releaseTarget */ TaskCore__ReleaseTarget,
    /* +0x0E0 updateSlotElements */ TaskCore__UpdateSlotElements,
    /* +0x0E4 broadcastToSlots */ (void *)TaskCore__BroadcastToSlots,
    /* +0x0E8 findNextFreeSlot */ TaskCore__FindNextFreeSlot,
    /* +0x0EC findPrevFreeSlot */ TaskCore__FindPrevFreeSlot,
    /* +0x0F0 setActiveSlot */ TaskCore__SetActiveSlot,
    /* +0x0F4 getActiveSlot */ TaskCore__GetActiveSlot,
    /* +0x0F8 createSlotElements */ TaskCore__CreateSlotElements,
    /* +0x0FC releaseSlotElements */ TaskCore__ReleaseSlotElements,
    /* +0x100 refreshSlotView */ TaskCore__RefreshSlotView,
    /* +0x104 broadcastToSlotElements */ TaskCore__BroadcastToSlotElements,
    /* +0x108 beginElementScroll */ TaskCore__BeginElementScroll,
    /* +0x10C commitElementScroll */ TaskCore__CommitElementScroll,
    /* +0x110 cancelElementScroll */ TaskCore__CancelElementScroll,
    /* +0x114 advanceSlotCursor */ TaskCore__AdvanceSlotCursor,
    /* +0x118 retreatSlotCursor */ TaskCore__RetreatSlotCursor,
    /* +0x11C setSlotCursor */ TaskCore__SetSlotCursor,
    /* +0x120 getActiveItemCursor */ TaskCore__GetActiveItemCursor,
};

DrawRect sDefaultMovieFrame = {640, 0, 320, 240};

u8 sTaskCoreDefaultColors[3][3] = {{0, 0, 0}, {0, 0, 0}, {128, 128, 128}};

LongVec3 sTaskCoreViewOrigin = {0, 0, 0};

/* IntermediateBase (include/intermediate_base.h): BasicClass's slots with
 * the task lifecycle: resetCounters, init, deinit, the DrawSystem event
 * handler, the frame counter, setState, onStart and onStop. */
IntermediateBaseMethods gIntermediateBaseMethods = {
    /* +0x000 header */ INTERMEDIATEBASE_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ IntermediateBase__IntermediateBase,
    /* +0x00C finalize */ (void *)BasicClass__Finalize,
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
    /* +0x038 onNotify */ (void *)IntermediateBase__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 resetCounters */ IntermediateBase__ResetCounters,
    /* +0x044 init */ (void *)IntermediateBase__Init,
    /* +0x048 deinit */ IntermediateBase__Deinit,
    /* +0x04C onInit */ NULL,
    /* +0x050 onDeinit */ NULL,
    /* +0x054 onDrawSystemEvent */ IntermediateBase__OnDrawSystemEvent,
    /* +0x058 onPadEvent */ NULL,
    /* +0x05C update */ (void *)IntermediateBase__IncrementFrameCounter,
    /* +0x060 setState */ IntermediateBase__SetState,
    /* +0x064 onStart */ IntermediateBase__OnStart,
    /* +0x068 onStop */ IntermediateBase__OnStop,
};
