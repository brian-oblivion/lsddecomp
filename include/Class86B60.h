#ifndef CLASS86B60_H
#define CLASS86B60_H

#include "TaskCore.h"

/*
 * Class86B60 -- class id 0x1F130, method table gClass86B60Methods, a TaskCore
 * subclass (`tools/classtable.py gClass86B60Methods --vs gTaskCoreMethods`:
 * fourteen overrides and six slots of its own). No class derives from it.
 * src/class_3bb8c_c.c holds the allocator and ctor, src/class_3bb8c_d.c every
 * other method and the getter. Named by its table's address: what the class
 * does is readable below, but no name for it is established.
 *
 * Who makes one: Class6D3C8__PollGraphRoomStatus (src/code_1677c.c), through
 * Class6D3C8__RunPollTask(New_Class86B60, self->dreamSys, ...), in a loop that
 * runs GraphRoom again and retries while init returns 2. The ctor's argument,
 * kept at +0x0A4, is therefore the game's DreamSys.
 *
 * Construction, ctor(dreamSys): TaskCore's ctor with (&D_80086D44, "ETC\ETCSE",
 * NULL), this class's table, the sound's +0x09C (VabStreamObj__SetPitchOffset)
 * with -1, dreamSys kept, saveCtrl cleared, the DreamSys's getSaveBlock into
 * saveBlock/saveBlockSize, FormatNumberIntoBuffer(getCurrentDayAndYear(0)),
 * setTarget(&D_80086D44) -- which this class overrides with CreateNameField --
 * and a call of resetCounters (Class86B60__Reset) that also passes dreamSys,
 * which the slot does not have and Reset does not read ($a1 is loaded in the
 * retail bytes), so the ctor casts the slot to Class86B60ResetCallFn below,
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
 * casts it to class_3bb8c.h's Class86B60UnkC0Obj_3bb8c_d (TaskCore__OnDeinit
 * makes the same call through code_2c054.h's TaskTextObj); beginMemcardSave
 * adds `saveCtrl` as a child, upcast to BasicClass.
 *
 * `saveCtrl` is a TaskObjF (include/TaskObjF.h, unified round 89): init
 * (+0x06C) gets the product code, the name suffix table (D_80086D6C),
 * initArgs->unk4 and unk10 as its input and tick sources, unk14 as its
 * sprite parent and `sound`; beginSave (+0x078) and beginLoad (+0x074) get
 * saveBlock/saveBlockSize as the data read or written.
 *
 * The object is 0xC4 bytes (New_Class86B60's allocation).
 */

typedef struct Class86B60 Class86B60;
typedef struct Class86B60Methods Class86B60Methods;

struct DreamSys;
struct TimImage;
struct TextRow;
struct SpriteRgb;
struct TaskObjF;

struct Class86B60Methods {
    TASKCORE_SLOTS(Class86B60, (Class86B60 *self, struct DreamSys *dreamSys));
    /* +0x124 */ void (*commitNameEntry)(Class86B60 *self, s32 arg1); /* Class86B60__CommitNameEntry; both callers pass
                                                                         arg1 (setState: 0, onTagBValue: 0x16) and the
                                                                         occupant reads self alone */
    /* +0x128 */ void (*beginMemcardSave)(Class86B60 *self);          /* Class86B60__BeginMemcardSave; updateMemcardSave* call it first */
    /* +0x12C */ void (*endMemcardSave)(Class86B60 *self);            /* Class86B60__EndMemcardSave; onTagBValue's 0x16/0x17 */
    /* +0x130 */ void (*updateMemcardSaveWithIcon)(Class86B60 *self); /* Class86B60__UpdateMemcardSaveWithIcon; tick's activeSlot 2 */
    /* +0x134 */ void (*updateMemcardSaveStatus)(Class86B60 *self);   /* Class86B60__UpdateMemcardSaveStatus; tick's activeSlot 3 */
    /* +0x138 */ void (*onTagBValue)(Class86B60 *self, BasicClass *sender, s32 event); /* Class86B60__OnTagBValue; onNotify's class-0xB case */
};

struct Class86B60 {
    TASKCORE_FIELDS(Class86B60Methods);
    /* +0x0A4 */ struct DreamSys *dreamSys;     /* the ctor's; its +0x0F0/+0x19C/+0x1A0/+0x1A8/+0x1AC/+0x1B0 are called */
    /* +0x0A8 */ struct TimImage *iconHandle;   /* beginMemcardSave: New_TimImage("CARD\FILEICN1.TIM"); finalize releases it */
    /* +0x0AC */ struct TaskObjF *saveCtrl;               /* beginMemcardSave: New_TaskObjF(1, 0); the ctor clears it;
                                                                 finalize releases it and iconHandle when it is set */
    /* +0x0B0 */ struct TextRow *nameField;     /* setTarget (CreateNameField): New_TextRow; releaseTarget releases it */
    /* +0x0B4 */ u8 pad0B4[0x0BC - 0x0B4];
    /* +0x0BC */ s32 *saveBlock;                /* the ctor: DreamSys getSaveBlock's result (&saveMagic); saveCtrl's beginLoad/beginSave data */
    /* +0x0C0 */ s32 saveBlockSize;             /* the ctor: getSaveBlock's *outSize (0x700); the object is 0xC4 bytes */
};

/* The ctor's resetCounters call, as the retail bytes make it: the slot is
 * (self), the call also passes dreamSys (see the banner). No code. */
typedef void (*Class86B60ResetCallFn)(Class86B60 *self, struct DreamSys *dreamSys);

extern Class86B60Methods gClass86B60Methods;
extern Class86B60Methods *GetClass86B60Methods(void); /* returns &gClass86B60Methods */

/* The class's own methods, in ROM order (class_3bb8c_c, then class_3bb8c_d). */
Class86B60 *New_Class86B60(struct DreamSys *dreamSys);
void Class86B60__Class86B60(Class86B60 *self, struct DreamSys *dreamSys);
void Class86B60__Finalize(Class86B60 *self);
void Class86B60__OnNotify(Class86B60 *self, BasicClass *sender, s32 event);
void Class86B60__Reset(Class86B60 *self);
void Class86B60__OnDeinit(Class86B60 *self);
void Class86B60__SetState(Class86B60 *self, s32 state);
void Class86B60__Tick(Class86B60 *self);
void Class86B60__RefreshViewValue(Class86B60 *self);
void Class86B60__CreateNameField(Class86B60 *self, TaskCoreTarget *target);
void Class86B60__DestroyNameField(Class86B60 *self);
void Class86B60__ForwardToNameField(Class86B60 *self, void *parent);
void Class86B60__TickNameFieldCursor(Class86B60 *self, struct SpriteRgb *color);
void Class86B60__CommitNameEntry(Class86B60 *self);
void Class86B60__BeginMemcardSave(Class86B60 *self);
void Class86B60__EndMemcardSave(Class86B60 *self);
void Class86B60__UpdateMemcardSaveWithIcon(Class86B60 *self);
void Class86B60__UpdateMemcardSaveStatus(Class86B60 *self);
void Class86B60__OnTagBValue(Class86B60 *self, BasicClass *sender, s32 event);

#endif
