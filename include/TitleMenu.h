#ifndef TITLEMENU_H
#define TITLEMENU_H

#include "TaskCore.h"

/*
 * TitleMenu -- class id 0x1F130, method table gTitleMenuMethods, a TaskCore
 * subclass (`tools/classtable.py gTitleMenuMethods --vs gTaskCoreMethods`:
 * fourteen overrides and six slots of its own). No class derives from it.
 * src/class_3bb8c_c.c holds the allocator and ctor, src/class_3bb8c_d.c every
 * other method and the getter. Named by its table's address: what the class
 * does is readable below, but no name for it is established.
 *
 * Who makes one: GameApplication__PollGraphRoomStatus (src/code_1677c.c), through
 * GameApplication__RunPollTask(New_TitleMenu, self->dreamSys, ...), in a loop that
 * runs GraphRoom again and retries while init returns 2. The ctor's argument,
 * kept at +0x0A4, is therefore the game's DreamSys.
 *
 * Construction, ctor(dreamSys): TaskCore's ctor with (&D_80086D44, "ETC\ETCSE",
 * NULL), this class's table, the sound's +0x09C (VabStreamObj__SetPitchOffset)
 * with -1, dreamSys kept, saveCtrl cleared, the DreamSys's getSaveBlock into
 * saveBlock/saveBlockSize, FormatNumberIntoBuffer(getCurrentDayAndYear(0)),
 * setTarget(&D_80086D44) -- which this class overrides with CreateNameField --
 * and a call of resetCounters (TitleMenu__Reset) that also passes dreamSys,
 * which the slot does not have and Reset does not read ($a1 is loaded in the
 * retail bytes), so the ctor casts the slot to TitleMenuResetCallFn below,
 * as GraphRoom's does.
 *
 * What the overrides do, measured:
 *  - setTarget builds no slot widgets: it makes `nameField`, a TextRow of the
 *    SJIS text in D_8008AA18's buffer (D_8008AA14's string copied in at
 *    +0x18 first when the DreamSys's new-game flag is set), 8 of its cells
 *    visible from cell 4, a gap before cell 9. releaseTarget releases it; updateSlotElements
 *    attaches it; broadcastToSlots blinks one of its three colour channels.
 *  - activeSlot (TaskCore's +0x058) is what tick switches on: 1 and 4 set
 *    `result` to 0 or 2 and refreshViewValue (1 also getSetFlashbackSession
 *    (0, 1)); 2 runs updateMemcardSaveWithIcon, 3 updateMemcardSaveStatus.
 *  - setState(5) calls commitNameEntry; setState(0xA) runs onPadCancel,
 *    setActiveSlot(target->unk8, 1) and onPadConfirm.
 *  - onNotify is the base one, then onTagBValue(sender, event) when the
 *    sender's class id has low nibble 0xB (gTaskObjFMethods, 0xB, is such a
 *    class; saveCtrl is one): events 0x16 and 0x17 run endMemcardSave, 0x16
 *    also clears the new-game flag and calls commitNameEntry.
 *  - beginMemcardSave makes `iconHandle` (New_TimImage("CARD\FILEICN1.TIM"))
 *    and `saveCtrl` (New_TaskObjF(1, 0)) once, calls saveCtrl's init (+0x06C) with
 *    "BISLPS-01556" (D_8008A9D0's value, the memory-card product code), adds
 *    saveCtrl as a child and removes initArgs->unk4 and unk10; endMemcardSave
 *    undoes both and calls saveCtrl's deinit (+0x070). The two update methods pass
 *    D_8008AA10/D_8008AA18 and saveBlock/saveBlockSize to saveCtrl's
 *    beginLoad (+0x074) or beginSave (+0x078, also iconHandle).
 *
 * Accessors that read an inherited field at another type than TaskCore's:
 * the ctor calls `sound` (TaskCore's BasicClass *) past BasicClass's slots
 * and casts it to VabStreamObj; onDeinit calls initArgs->unk0 at +0x078 and
 * casts it to class_3bb8c.h's TitleMenuUnkC0Obj_3bb8c_d (TaskCore__OnDeinit
 * makes the same call through code_2c054.h's TaskTextObj); beginMemcardSave
 * adds `saveCtrl` as a child, upcast to BasicClass.
 *
 * `saveCtrl` is a TaskObjF (include/TaskObjF.h, unified round 89): init
 * (+0x06C) gets the product code, the name suffix table (D_80086D6C),
 * initArgs->unk4 and unk10 as its input and tick sources, unk14 as its
 * sprite parent and `sound`; beginSave (+0x078) and beginLoad (+0x074) get
 * saveBlock/saveBlockSize as the data read or written.
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
    /* +0x124 */ void (*commitNameEntry)(TitleMenu *self, s32 arg1); /* TitleMenu__RefreshMenu; both callers pass
                                                                         arg1 (setState: 0, onTagBValue: 0x16) and the
                                                                         occupant reads self alone */
    /* +0x128 */ void (*beginMemcardSave)(TitleMenu *self); /* TitleMenu__BeginCardAccess; updateMemcardSave* call it first */
    /* +0x12C */ void (*endMemcardSave)(TitleMenu *self); /* TitleMenu__EndCardAccess; onTagBValue's 0x16/0x17 */
    /* +0x130 */ void (*updateMemcardSaveWithIcon)(TitleMenu *self); /* TitleMenu__SaveToCard; tick's activeSlot 2 */
    /* +0x134 */ void (*updateMemcardSaveStatus)(TitleMenu *self); /* TitleMenu__LoadFromCard; tick's activeSlot 3 */
    /* +0x138 */ void (*onTagBValue)(TitleMenu *self, BasicClass *sender,
                                     s32 event); /* TitleMenu__OnCardEvent; onNotify's class-0xB case */
};

struct TitleMenu {
    TASKCORE_FIELDS(TitleMenuMethods);
    /* +0x0A4 */ struct DreamSys *dreamSys; /* the ctor's; its +0x0F0/+0x19C/+0x1A0/+0x1A8/+0x1AC/+0x1B0 are called */
    /* +0x0A8 */ struct TimImage *iconHandle; /* beginMemcardSave: New_TimImage("CARD\FILEICN1.TIM"); finalize releases it */
    /* +0x0AC */ struct TaskObjF *saveCtrl; /* beginMemcardSave: New_TaskObjF(1, 0); the ctor clears it;
                                                                 finalize releases it and iconHandle when it is set */
    /* +0x0B0 */ struct TextRow *nameField; /* setTarget (CreateNameField): New_TextRow; releaseTarget releases it */
    /* +0x0B4 */ u8 pad0B4[0x0BC - 0x0B4];
    /* +0x0BC */ s32 *saveBlock; /* the ctor: DreamSys getSaveBlock's result (&saveMagic); saveCtrl's beginLoad/beginSave data */
    /* +0x0C0 */ s32 saveBlockSize; /* the ctor: getSaveBlock's *outSize (0x700); the object is 0xC4 bytes */
};

/* The ctor's resetCounters call, as the retail bytes make it: the slot is
 * (self), the call also passes dreamSys (see the banner). No code. */
typedef void (*TitleMenuResetCallFn)(TitleMenu *self, struct DreamSys *dreamSys);

extern TitleMenuMethods gTitleMenuMethods;
extern TitleMenuMethods *GetTitleMenuMethods(void); /* returns &gTitleMenuMethods */

/* The class's own methods, in ROM order (class_3bb8c_c, then class_3bb8c_d). */
TitleMenu *New_TitleMenu(struct DreamSys *dreamSys);
void TitleMenu__TitleMenu(TitleMenu *self, struct DreamSys *dreamSys);
void TitleMenu__Finalize(TitleMenu *self);
void TitleMenu__OnNotify(TitleMenu *self, BasicClass *sender, s32 event);
void TitleMenu__Reset(TitleMenu *self);
void TitleMenu__OnDeinit(TitleMenu *self);
void TitleMenu__SetState(TitleMenu *self, s32 state);
void TitleMenu__Tick(TitleMenu *self);
void TitleMenu__RefreshViewValue(TitleMenu *self);
void TitleMenu__CreateNameField(TitleMenu *self, TaskCoreTarget *target);
void TitleMenu__DestroyNameField(TitleMenu *self);
void TitleMenu__ForwardToNameField(TitleMenu *self, void *parent);
void TitleMenu__TickNameFieldCursor(TitleMenu *self, struct SpriteRgb *color);
void TitleMenu__RefreshMenu(TitleMenu *self);
void TitleMenu__BeginCardAccess(TitleMenu *self);
void TitleMenu__EndCardAccess(TitleMenu *self);
void TitleMenu__SaveToCard(TitleMenu *self);
void TitleMenu__LoadFromCard(TitleMenu *self);
void TitleMenu__OnCardEvent(TitleMenu *self, BasicClass *sender, s32 event);

#endif
