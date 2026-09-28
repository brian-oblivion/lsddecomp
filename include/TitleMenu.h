#ifndef TITLEMENU_H
#define TITLEMENU_H

#include "TaskCore.h"
#include "DrawSystem.h"

/*
 * TitleMenu -- the menu over ETC\TITLE.TIM that the game returns to between
 * days: START, FLASHBACK, SAVE, LOAD, GRAPH and SHAKE (the six `names` of its
 * TaskCoreTarget, sTitleMenuTarget), with the memory-card save title ("LSD   Day001")
 * shown as a TextRow. A TaskCore (class id 0x1F130, table gTitleMenuMethods;
 * fourteen overrides and six slots of its own); no class derives from it.
 * src/ui/TitleMenuTaskObjF.c holds the allocator and ctor, every other method
 * and the getter.
 *
 * Who makes one: GameApplication__RunTitleMenu (src/app/GameApplicationFileResource.c) runs
 * it with GameApplication__RunTask(New_TitleMenu, dreamSys, ...) after the
 * day's GraphRoom; while it returns 2 (GRAPH) it runs GraphRoom again and then
 * the menu again.
 *
 * Construction, ctor(dreamSys): TaskCore's ctor with the menu, "ETC\ETCSE" and
 * no sound object; `sound`'s pitch offset set to -1; `saveCtrl` cleared;
 * `saveBlock`/`saveBlockSize` from DreamSys's getSaveBlock;
 * StampSaveTitleDay writes the current day into the save title;
 * setTarget, then resetCounters (TitleMenu__Reset: the TITLE.TIM backdrop via
 * setSubHandle, frame bound 10, the flashback session cleared).
 *
 * The menu. The six entries are TaskCore's slots, `activeSlot` their index:
 *  - 0 START is the target's confirm slot (unkC): TaskCore ends the menu with
 *    refreshViewValue, which this class extends to store SHAKE's setting
 *    (slotCounts[5]) through DreamSys's getSetScreenShake.
 *  - 1 FLASHBACK: result 0 and a flashback session opened; 4 GRAPH: result 2.
 *    Both then end the menu through refreshViewValue.
 *  - 2 SAVE runs saveToCard, 3 LOAD loadFromCard.
 *  - 5 SHAKE is the one entry with an item list (the target's unk24[5]).
 *  - FLASHBACK starts locked (registrationSlots[1] = 1); refreshMenu clears
 *    the lock through UpdateFlashbackLock when the save block allows it.
 * setState(5), the menu becoming active, runs refreshMenu: the save title's
 * text reloaded, FLASHBACK's lock recomputed, the widgets re-attached and
 * SHAKE's cursor set from DreamSys. setState(0xA) cancels, reselects the
 * target's first slot and confirms.
 *
 * The save title replaces TaskCore's slot widgets as what setTarget,
 * releaseTarget, updateSlotElements and broadcastToSlots manage:
 * createSaveTitle makes `saveTitle` from the SJIS title in gSaveTitle's buffer
 * (blanked from +0x18 on a new game), 8 cells visible from cell 4 with a gap
 * before cell 9; cycleSaveTitleColor lights one colour channel a frame.
 *
 * The memory card. beginCardAccess makes `saveIcon` (CARD\FILEICN1.TIM) and
 * `saveCtrl` (a TaskObjF, include/TaskObjF.h) on first use, inits saveCtrl
 * with the product code "BISLPS-01556", the file suffix table, the pad and
 * clock sources (initArgs->pad, unk10), unk14 as sprite parent and `sound`,
 * and hands input to it: saveCtrl is added as a child and the pad and
 * clock are removed. saveToCard stores SHAKE's setting and calls saveCtrl's beginSave
 * with the save title, `saveIcon` and the save block; loadFromCard calls
 * beginLoad with the same block. saveCtrl's terminal events reach onNotify
 * (the sender's class id ends in 0xB, as TaskObjF's does) and onCardEvent:
 * 0x16 (the operation completed) and 0x17 both run endCardAccess, which gives input back; 0x16
 * also clears the new-game flag and runs refreshMenu. Finalize releases
 * saveCtrl and saveIcon if they were made.
 *
 * Accessors that read an inherited field at another type than TaskCore's:
 * the ctor casts `sound` to VabStreamObj; onDeinit casts initArgs->drawSystem
 * to DrawSystem to call its clearImage (TaskCore__OnDeinit makes the same call
 * through DrawSystem as well); beginCardAccess
 * adds `saveCtrl` as a child, upcast to BasicClass.
 *
 * The object is 0xC4 bytes (New_TitleMenu's allocation).
 */

typedef struct TitleMenu TitleMenu;
typedef struct TitleMenuMethods TitleMenuMethods;

struct DreamSys;
struct TimImage;
struct TextRow;
struct SpriteRgb;
struct TaskObjF;

struct TitleMenuMethods {
    TASKCORE_SLOTS(TitleMenu, (TitleMenu * self, struct DreamSys *dreamSys));
    /* +0x124 */ void (*refreshMenu)(TitleMenu *self, s32 arg1); /* TitleMenu__RefreshMenu; both callers pass
                                                                     arg1 (setState: 0, onCardEvent: 0x16) and the
                                                                     occupant reads self alone */
    /* +0x128 */ void (*beginCardAccess)(TitleMenu *self); /* TitleMenu__BeginCardAccess; saveToCard/loadFromCard call it first */
    /* +0x12C */ void (*endCardAccess)(TitleMenu *self); /* TitleMenu__EndCardAccess; onCardEvent's 0x16/0x17 */
    /* +0x130 */ void (*saveToCard)(TitleMenu *self); /* TitleMenu__SaveToCard; tick's activeSlot 2, SAVE */
    /* +0x134 */ void (*loadFromCard)(TitleMenu *self); /* TitleMenu__LoadFromCard; tick's activeSlot 3, LOAD */
    /* +0x138 */ void (*onCardEvent)(TitleMenu *self, BasicClass *sender,
                                     s32 event); /* TitleMenu__OnCardEvent; onNotify's class-0xB case */
};

struct TitleMenu {
    TASKCORE_FIELDS(TitleMenuMethods);
    /* +0x0A4 */ struct DreamSys *dreamSys; /* the ctor's; its +0x0F0/+0x19C/+0x1A0/+0x1A8/+0x1AC/+0x1B0 are called */
    /* +0x0A8 */ struct TimImage *saveIcon; /* beginCardAccess: New_TimImage("CARD\FILEICN1.TIM"); beginSave's icon */
    /* +0x0AC */ struct TaskObjF *saveCtrl; /* beginCardAccess: New_TaskObjF(1, 0); the ctor clears it;
                                             finalize releases it and saveIcon when it is set */
    /* +0x0B0 */ struct TextRow *saveTitle; /* setTarget (createSaveTitle): the save title; releaseTarget releases it */
    /* +0x0B4 */ u8 pad0B4[0x0BC - 0x0B4];
    /* +0x0BC */ s32 *saveBlock; /* the ctor: DreamSys getSaveBlock's result (&saveMagic); saveCtrl's beginLoad/beginSave data */
    /* +0x0C0 */ s32 saveBlockSize; /* the ctor: getSaveBlock's *outSize (0x700); the object is 0xC4 bytes */
};

/* One full-width (2-byte Shift-JIS) character of the save title, and runs of
 * 3 and 6 of them that are written into it whole: StampSaveTitleDay's
 * day number (characters 9..11), StampSaveTitleFileLetter's letter field
 * (3..5) and letter field plus "Day" (3..8).
 * MATCHING: all-s8 (alignment 1), so a copy is lwl/lwr words plus single
 * bytes; alignment 2 would merge a two-byte tail into a halfword. */
typedef struct {
    s8 lead, trail;
} FullWidthChar;

typedef struct {
    FullWidthChar chars[3];
} FullWidthChars3;

typedef struct {
    FullWidthChar chars[6];
} FullWidthChars6;

/* TitleMenu::activeSlot and the index into slotCounts: the six entries,
 * in the order of the target's `names` (see the banner). */
enum TitleMenuEntry {
    TITLEMENU_START = 0,
    TITLEMENU_FLASHBACK = 1,
    TITLEMENU_SAVE = 2,
    TITLEMENU_LOAD = 3,
    TITLEMENU_GRAPH = 4,
    TITLEMENU_SHAKE = 5
};

/* TitleMenu's `result` for GRAPH: GameApplication__RunTitleMenu runs
 * GraphRoom again, then the menu again, while the menu returns it. */
#define TITLEMENU_RESULT_GRAPH 2

/* The ctor's resetCounters call, as the retail bytes make it: the slot is
 * (self), the call also passes dreamSys (see the banner). No code. */
typedef void (*TitleMenuResetCallFn)(TitleMenu *self, struct DreamSys *dreamSys);

extern TitleMenuMethods gTitleMenuMethods;
extern TitleMenuMethods *GetTitleMenuMethods(void); /* returns &gTitleMenuMethods */

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
 * SaveToCard empties on a new game; gSaveTitle at the full-width
 * "LSD   Day001" followed by 19 full-width spaces, which
 * TitleMenu__CreateSaveTitle reblanks from its 12th character on a new game
 * and StampSaveTitleDay writes the day into.
 */
extern char *sSaveFileName;
extern char *gSaveTitle;

/* The buffer StampSaveTitleDay formats the day into
 * (FormatFullWidthNumber) before copying it into gSaveTitle's title. The
 * ROM image points it at the "7654321" string D_8008AA1C. */
extern void *sDayDigits;

/* 19 full-width spaces, the tail TitleMenu__CreateSaveTitle copies over
 * gSaveTitle's on a new game (the ROM image points it just past
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
/* Formats the current day into the save title (src/ui/TitleMenuTaskObjF.c);
 * TitleMenu__TitleMenu calls it with DreamSys's getCurrentDayAndYear. */
extern void StampSaveTitleDay(s32 day);

/* The save block DreamSys's getSaveBlock returns (TitleMenu::saveBlock),
 * which UpdateFlashbackLock reads, is DreamSaveBlock in include/DreamSys.h. */

/* The class's own methods, in ROM order (src/ui/TitleMenuTaskObjF.c). */
TitleMenu *New_TitleMenu(struct DreamSys *dreamSys);
void TitleMenu__TitleMenu(TitleMenu *self, struct DreamSys *dreamSys);
void TitleMenu__Finalize(TitleMenu *self);
void TitleMenu__OnNotify(TitleMenu *self, BasicClass *sender, s32 event);
void TitleMenu__Reset(TitleMenu *self);
void TitleMenu__OnDeinit(TitleMenu *self);
void TitleMenu__SetState(TitleMenu *self, s32 state);
void TitleMenu__ConfirmSlot(TitleMenu *self);
void TitleMenu__Exit(TitleMenu *self);
void TitleMenu__CreateSaveTitle(TitleMenu *self, TaskCoreTarget *target);
void TitleMenu__DestroySaveTitle(TitleMenu *self);
void TitleMenu__AttachSaveTitle(TitleMenu *self, void *parent);
void TitleMenu__CycleSaveTitleColor(TitleMenu *self, struct SpriteRgb *color);
void TitleMenu__RefreshMenu(TitleMenu *self);
void TitleMenu__BeginCardAccess(TitleMenu *self);
void TitleMenu__EndCardAccess(TitleMenu *self);
void TitleMenu__SaveToCard(TitleMenu *self);
void TitleMenu__LoadFromCard(TitleMenu *self);
void TitleMenu__OnCardEvent(TitleMenu *self, BasicClass *sender, s32 event);

#endif
