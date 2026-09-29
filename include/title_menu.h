/**
 * @file title_menu.h
 * @brief TitleMenu, the menu the game returns to between days, and the save title's characters.
 *
 * Declares the TitleMenu class (object and method table), its entry enum
 * and GRAPH result, the full-width character types the save title is
 * written in, and TitleMenu's methods and StampSaveTitleDay, which are
 * defined in src/ui/title_menu.c.
 */
#ifndef TITLE_MENU_H
#define TITLE_MENU_H

#include "task_core.h"
#include "draw_system.h"

typedef struct TitleMenu TitleMenu;
typedef struct TitleMenuMethods TitleMenuMethods;

struct DreamSys;
struct TimImage;
struct TextRow;
struct ColorRgb;
struct TaskObjF;

/**
 * @brief TitleMenu's method table: TaskCore's slots, then TitleMenu's own.
 *
 * TitleMenu overrides twelve of TaskCore's slots: the ctor, finalize,
 * onNotify, resetCounters (TitleMenu__Reset), onDeinit, setState,
 * confirmSlot, exit, setTarget (TitleMenu__CreateSaveTitle), releaseTarget
 * (TitleMenu__DestroySaveTitle), updateSlotElements
 * (TitleMenu__AttachSaveTitle) and broadcastToSlots
 * (TitleMenu__CycleSaveTitleColor).
 */
struct TitleMenuMethods {
    TASKCORE_SLOTS(TitleMenu, (TitleMenu * self, struct DreamSys *dreamSys));
    /* +0x124 */ void (*refreshMenu)(TitleMenu *self, s32 arg1); /**< @see TitleMenu__RefreshMenu; both callers
                                                                     pass arg1 (setState: 0, onCardEvent:
                                                                     TASKOBJF_STATE_DONE) and the occupant reads
                                                                     self alone */
    /* +0x128 */ void (*beginCardAccess)(TitleMenu *self); /**< @see TitleMenu__BeginCardAccess */
    /* +0x12C */ void (*endCardAccess)(TitleMenu *self);   /**< @see TitleMenu__EndCardAccess */
    /* +0x130 */ void (*saveToCard)(TitleMenu *self); /**< @see TitleMenu__SaveToCard; confirmSlot's SAVE */
    /* +0x134 */ void (*loadFromCard)(TitleMenu *self); /**< @see TitleMenu__LoadFromCard; confirmSlot's LOAD */
    /* +0x138 */ void (*onCardEvent)(TitleMenu *self, BasicClass *sender,
                                     s32 event); /**< @see TitleMenu__OnCardEvent; onNotify's TaskObjF case */
};

/**
 * @brief The menu between days: START, FLASHBACK, SAVE, LOAD, GRAPH and SHAKE over ETC\TITLE.TIM.
 *
 * A TaskCore (class id 0x1F130, table gTitleMenuMethods, twelve overrides
 * and six slots of its own); no class derives from it. The six entries are
 * the `names` of its TaskCoreTarget, sTitleMenuTarget, and the memory-card
 * save title ("LSD   Day001") is shown as a TextRow. src/ui/title_menu.c
 * holds the allocator, the ctor, every other method and the getter.
 *
 * Who makes one: GameApplication__RunTitleMenu (src/app/game_shell.c) runs
 * it with GameApplication__RunTask(New_TitleMenu, dreamSys, ...) after the
 * day's GraphRoom; while it returns 2 (GRAPH) it runs GraphRoom again and
 * then the menu again.
 *
 * Construction, ctor(dreamSys): TaskCore's ctor with the menu, "ETC\ETCSE"
 * and no sound object; `sound`'s pitch offset set to -1; `saveCtrl`
 * cleared; `saveBlock`/`saveBlockSize` from DreamSys's getSaveBlock;
 * StampSaveTitleDay writes the current day into the save title; setTarget,
 * then resetCounters (TitleMenu__Reset: the TITLE.TIM backdrop via
 * setSubHandle, frame bound 10, the flashback session cleared).
 *
 * The menu. The six entries are TaskCore's slots, `activeSlot` their index:
 *  - 0 START is the target's exitSlot: TaskCore ends the menu with exit,
 *    which this class extends to store SHAKE's setting (itemCursors[5])
 *    through DreamSys's getSetScreenShake.
 *  - 1 FLASHBACK: result TASKCORE_RESULT_DONE and a flashback session
 *    opened; 4 GRAPH: result TITLEMENU_RESULT_GRAPH. Both then end the menu
 *    through exit.
 *  - 2 SAVE runs saveToCard, 3 LOAD loadFromCard.
 *  - 5 SHAKE is the one entry with an item list (the target's slotLists[5]).
 *  - FLASHBACK starts locked (hiddenSlots[1] = 1); refreshMenu clears the
 *    lock through UpdateFlashbackLock when the save block allows it.
 * setState(TASKCORE_STATE_ACTIVE), the menu becoming active, runs
 * refreshMenu: the save title's text reloaded, FLASHBACK's lock recomputed,
 * the widgets re-attached and SHAKE's cursor set from DreamSys.
 * setState(TASKCORE_STATE_START_PRESSED) cancels, reselects the target's
 * first slot and confirms.
 *
 * The save title replaces TaskCore's slot widgets as what setTarget,
 * releaseTarget, updateSlotElements and broadcastToSlots manage:
 * createSaveTitle makes `saveTitle` from the SJIS title in sSaveTitle's
 * buffer (blanked from +0x18 on a new game), 8 cells visible from cell 4
 * with a gap before cell 9; cycleSaveTitleColor lights one colour channel a
 * frame.
 *
 * The memory card. beginCardAccess makes `saveIcon` (CARD\FILEICN1.TIM) and
 * `saveCtrl` (a TaskObjF, include/task_objf.h) on first use, inits saveCtrl
 * with the product code "BISLPS-01556", the file suffix table, the pad and
 * clock sources (initArgs->pad, frameClock), lightRig as sprite parent and
 * `sound`, and hands input to it: saveCtrl is added as a child and the pad
 * and clock are removed. saveToCard stores SHAKE's setting and calls
 * saveCtrl's beginSave with the save title, `saveIcon` and the save block;
 * loadFromCard calls beginLoad with the same block. saveCtrl's terminal
 * events reach onNotify (the sender's class id ends in 0xB, as TaskObjF's
 * does) and onCardEvent: 0x16 (the operation completed) and 0x17 both run
 * endCardAccess, which gives input back; 0x16 also clears the new-game flag
 * and runs refreshMenu. Finalize releases saveCtrl and saveIcon if they
 * were made.
 *
 * Accessors that read an inherited field at another type than TaskCore's:
 * the ctor casts `sound` to VabStreamObj; onDeinit casts
 * initArgs->drawSystem to DrawSystem to call its clearImage
 * (TaskCore__OnDeinit makes the same call through DrawSystem as well);
 * beginCardAccess adds `saveCtrl` as a child, upcast to BasicClass.
 *
 * The object is 0xC4 bytes (New_TitleMenu's allocation).
 */
struct TitleMenu {
    TASKCORE_FIELDS(TitleMenuMethods);
    /* +0x0A4 */ struct DreamSys *dreamSys; /**< the ctor's: its save block, day, screen shake, flashback session and new-game flag */
    /* +0x0A8 */ struct TimImage *saveIcon; /**< beginCardAccess: New_TimImage("CARD\FILEICN1.TIM"); beginSave's icon */
    /* +0x0AC */ struct TaskObjF *saveCtrl; /**< beginCardAccess: New_TaskObjF(1, 0); the ctor clears it;
                                               finalize releases it and saveIcon when it is set */
    /* +0x0B0 */ struct TextRow *saveTitle; /**< setTarget (createSaveTitle): the save title; releaseTarget releases it */
    /* +0x0B4 */ u8 pad0B4[0x0BC - 0x0B4];
    /* +0x0BC */ s32 *saveBlock; /**< the ctor: DreamSys getSaveBlock's result (&saveMagic); saveCtrl's beginLoad/beginSave data */
    /* +0x0C0 */ s32 saveBlockSize; /**< the ctor: getSaveBlock's *outSize (0x700); the object is 0xC4 bytes */
};

/** One full-width (2-byte Shift-JIS) character of the save title, as two
 * single bytes. Runs of 3 and 6 of them are written into the title whole:
 * StampSaveTitleDay's day number (characters 9..11),
 * StampSaveTitleFileLetter's letter field (3..5) and letter field plus
 * "Day" (3..8). */
typedef struct {
    s8 lead,   /**< the Shift-JIS lead byte */
        trail; /**< the Shift-JIS trail byte */
} FullWidthChar;

/** Three full-width characters, copied whole. */
typedef struct {
    FullWidthChar chars[3]; /**< the characters */
} FullWidthChars3;

/** Six full-width characters, copied whole. */
typedef struct {
    FullWidthChar chars[6]; /**< the characters */
} FullWidthChars6;

/** TitleMenu::activeSlot and the index into itemCursors: the six entries,
 * in the order of the target's `names`. */
enum TitleMenuEntry {
    TITLEMENU_START = 0,     /**< ends the menu and starts the day */
    TITLEMENU_FLASHBACK = 1, /**< opens a flashback session, then ends the menu */
    TITLEMENU_SAVE = 2,      /**< saves to the memory card */
    TITLEMENU_LOAD = 3,      /**< loads from the memory card */
    TITLEMENU_GRAPH = 4,     /**< ends the menu with TITLEMENU_RESULT_GRAPH */
    TITLEMENU_SHAKE = 5      /**< the screen-shake setting's item list */
};

/** TitleMenu's `result` for GRAPH: GameApplication__RunTitleMenu runs
 * GraphRoom again, then the menu again, while the menu returns it. */
#define TITLEMENU_RESULT_GRAPH 2

/** The ctor's resetCounters call: the slot takes (self), and the ctor
 * passes dreamSys as well. */
typedef void (*TitleMenuResetCallFn)(TitleMenu *self, struct DreamSys *dreamSys);

/** TitleMenu's method table (class id 0x1F130). */
extern TitleMenuMethods gTitleMenuMethods;

/**
 * @brief Returns TitleMenu's method table.
 * @return &gTitleMenuMethods.
 */
extern TitleMenuMethods *GetTitleMenuMethods(void);

/**
 * @brief Writes the day into the save title as three full-width digits (characters 9..11).
 *
 * TitleMenu__TitleMenu calls it with DreamSys's getCurrentDayAndYear.
 * @param day the day number, zero-padded to three digits.
 */
extern void StampSaveTitleDay(s32 day);

/* The save block DreamSys's getSaveBlock returns (TitleMenu::saveBlock),
 * which UpdateFlashbackLock reads, is DreamSaveBlock in include/dream_sys.h. */

/* The class's own methods, in ROM order (src/ui/title_menu.c). */

/**
 * @brief Allocates a TitleMenu and runs its ctor through the method table.
 * @param dreamSys the DreamSys whose save block and settings the menu uses.
 * @return the new menu, or NULL when the allocation fails.
 */
TitleMenu *New_TitleMenu(struct DreamSys *dreamSys);

/**
 * @brief Constructs the menu: TaskCore's ctor, the save block, the day in the save title, target and counters.
 * @param self     the object to construct.
 * @param dreamSys the DreamSys whose save block and settings the menu uses.
 */
void TitleMenu__TitleMenu(TitleMenu *self, struct DreamSys *dreamSys);

/**
 * @brief Releases the card controller and save icon if they were made, then runs TaskCore's finalize.
 * @param self the menu.
 */
void TitleMenu__Finalize(TitleMenu *self);

/**
 * @brief TaskCore's onNotify, then onCardEvent for a notification from the card controller.
 * @param self   the menu.
 * @param sender the child that notified.
 * @param event  the notification.
 */
void TitleMenu__OnNotify(TitleMenu *self, BasicClass *sender, s32 event);

/**
 * @brief The resetCounters override: TITLE.TIM backdrop, 400 packets, frame bound 10, no flashback session.
 * @param self the menu.
 */
void TitleMenu__Reset(TitleMenu *self);

/**
 * @brief The onDeinit override: clears both display buffers to `clearColor`.
 * @param self the menu.
 */
void TitleMenu__OnDeinit(TitleMenu *self);

/**
 * @brief TaskCore's setState, then the menu's own reactions.
 *
 * TASKCORE_STATE_ACTIVE runs refreshMenu; TASKCORE_STATE_START_PRESSED
 * cancels, reselects the target's initial slot and confirms it.
 * @param self  the menu.
 * @param state a TaskCore state.
 */
void TitleMenu__SetState(TitleMenu *self, s32 state);

/**
 * @brief TaskCore's confirmSlot, then acts on the chosen entry.
 *
 * FLASHBACK opens a flashback session and exits with result
 * TASKCORE_RESULT_DONE, GRAPH exits with TITLEMENU_RESULT_GRAPH, SAVE runs
 * saveToCard and LOAD loadFromCard.
 * @param self the menu.
 */
void TitleMenu__ConfirmSlot(TitleMenu *self);

/**
 * @brief TaskCore's exit, then stores the SHAKE entry's setting in DreamSys.
 * @param self the menu.
 */
void TitleMenu__Exit(TitleMenu *self);

/**
 * @brief The setTarget override: makes the save title's TextRow in place of the slot widgets.
 *
 * On a new game first reblanks the title's padding and clears its letter
 * field. The row has a cell a character plus 4, eight of them shown from
 * cell 4, with a gap before cell 9. Does nothing for a NULL target.
 * @param self   the menu.
 * @param target the menu's TaskCoreTarget; only its `handle` (the font) is read.
 */
void TitleMenu__CreateSaveTitle(TitleMenu *self, TaskCoreTarget *target);

/**
 * @brief The releaseTarget override: releases the save title, then TaskCore's releaseTarget.
 * @param self the menu.
 */
void TitleMenu__DestroySaveTitle(TitleMenu *self);

/**
 * @brief The updateSlotElements override: TaskCore's, then the save title attached under `parent`.
 * @param self   the menu.
 * @param parent the SceneNode the widgets attach under.
 */
void TitleMenu__AttachSaveTitle(TitleMenu *self, void *parent);

/**
 * @brief The broadcastToSlots override, once a frame: TaskCore's, then the save title's colour.
 *
 * While the menu takes input the title is black with one channel lit, the
 * channel moving each frame; otherwise it is `color` with red lifted for
 * the first 128 frames of each 257-frame cycle and the moving channel after.
 * @param self  the menu.
 * @param color the colour TaskCore broadcasts to the slots.
 */
void TitleMenu__CycleSaveTitleColor(TitleMenu *self, struct ColorRgb *color);

/**
 * @brief Reloads the menu after it becomes active or a card operation completes.
 *
 * Reloads the save title's text, recomputes FLASHBACK's lock, re-attaches
 * the widgets and sets SHAKE's cursor from DreamSys's setting, then
 * reselects the entry that was active.
 * @param self the menu.
 */
void TitleMenu__RefreshMenu(TitleMenu *self);

/**
 * @brief Hands input to the card controller, making it and the save icon on first use.
 * @param self the menu.
 */
void TitleMenu__BeginCardAccess(TitleMenu *self);

/**
 * @brief Takes input back from the card controller and deinits it.
 * @param self the menu.
 */
void TitleMenu__EndCardAccess(TitleMenu *self);

/**
 * @brief Stores SHAKE's setting and starts a save of the save block under the save title.
 *
 * On a new game the file name is emptied first, so the save picks an
 * unused one.
 * @param self the menu.
 */
void TitleMenu__SaveToCard(TitleMenu *self);

/**
 * @brief Starts a load into the save block.
 * @param self the menu.
 */
void TitleMenu__LoadFromCard(TitleMenu *self);

/**
 * @brief Ends card access on the controller's DONE or ABORTED; DONE also clears the new-game flag and refreshes.
 * @param self   the menu.
 * @param sender the TaskObjF (unused).
 * @param event  the TaskObjFState the controller entered.
 */
void TitleMenu__OnCardEvent(TitleMenu *self, BasicClass *sender, s32 event);

#endif
