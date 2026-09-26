#ifndef TASKOBJF_H
#define TASKOBJF_H

#include "BasicClass.h"

/*
 * TaskObjF -- the memory-card save/load controller (class id 0xB, method
 * table gTaskObjFMethods): BasicClass's direct subclass, no class below it.
 * The name is the old unit view's (FINISHING-PLAN track 4 step 2); what the
 * class does is measured below. Methods: the allocator and ctor in
 * src/class_3bb8c_d.c, BasicClass's overrides and the card primitives
 * (+0x00C..+0x060) in src/class_3bb8c_e.c, the file I/O, events, buffers and
 * the two operations (+0x064..+0x078, +0x038) in src/class_3bb8c_f.c, the
 * state machine (+0x07C..+0x0B0) in src/class_3bb8c_g.c.
 *
 *  - The ctor runs InitCARD/StartCARD/_bu_init once per boot (D_8008AA30)
 *    and setCardSlot(cardSlot); +0x040..+0x068 wrap the PS-X memory-card
 *    BIOS: events (OpenEvent x4), _card_info/_card_load, format, and
 *    open/read/write/delete on "bu00:"/"bu10:" paths (BuildMemcardPath).
 *  - init (+0x06C) takes the name prefix and suffix table, the two event
 *    sources (added as children; TaskObjF__AddChild files each child by its
 *    class id: low nibble 2 -> inputSource, 5 -> tickSource, 0x10 ->
 *    textEntry, 0x20 -> itemList), the sprite parent and the sound. Its one
 *    caller is Class86B60__BeginMemcardSave: initArgs->unk4, unk10 (a
 *    FrameClock, id 0x5), unk14, and TaskCore's `sound` (a VabStreamObj).
 *  - beginLoad (+0x074, opMode 1) lists the save files that exist
 *    (collectExistingMemcardFiles into `titles`/`foundSuffixes`), lets the
 *    player pick one in a Class86F88 list (state 0x12), then reads it into
 *    `data` (state 0x15, readMemcardFile). beginSave (+0x078, opMode 2)
 *    checks for the file and for space, lets the player edit the title in a
 *    TextEntry (state 0x11), then writes the save file with its icon
 *    (state 0x14, writeMemcardSaveFile).
 *  - setState (+0x07C) runs notifyParents(state) first; the terminal states
 *    0x16/0x17 free the load buffers and return to state 0. Class86B60's
 *    onTagBValue (the parent) ends the save on them.
 *  - onNotify (+0x038) routes a child's notification by the child's class
 *    id, exactly as its AddChild files them: 2 -> onInputEvent, 5 ->
 *    tickStateDelay, 0x10 -> onTextEntryResult, 0x20 -> onItemListResult.
 *
 * +0x098 tickStateDelay keeps its caller's argument list: onNotify passes
 * (sender, event) and TaskObjF__TickStateDelay reads only self (TextEntry's
 * tickState is the same case).
 *
 * The object is 0x84 bytes (New_TaskObjF).
 */

struct TextEntry;
struct Class86F88;
struct ScreenSprite;
struct VabStreamObj;
struct TimImage;
struct Class6B5CC;

typedef struct TaskObjF TaskObjF;
typedef struct TaskObjFMethods TaskObjFMethods;

/* BasicClass's slots (overrides: +0x008 TaskObjF__TaskObjF, +0x00C Finalize,
 * +0x010 AddChild, +0x014 RemoveChild, +0x018 RemoveAllChildren, +0x038
 * OnNotify; `tools/classtable.py gTaskObjFMethods --vs D_8006B58C`), then
 * this class's own, every one of them filled. */
struct TaskObjFMethods {
    BASICCLASS_SLOTS(TaskObjF, (TaskObjF * self, s32 padEnable, s32 cardSlot));
    /* +0x040 */ void (*setCardSlot)(TaskObjF *self, s32 cardSlot); /* TaskObjF__SetCardSlot; the ctor's last call */
    /* +0x044 */ s32 (*openEvents)(TaskObjF *self);  /* TaskObjF__OpenEvents */
    /* +0x048 */ s32 (*closeEvents)(TaskObjF *self); /* TaskObjF__CloseEvents */
    /* +0x04C */ s32 (*checkCardStatus)(TaskObjF *self, s32 *outA, s32 *outB,
                                        s32 *outC); /* TaskObjF__CheckCardStatus; Validate reads the three */
    /* +0x050 */ s32 (*formatCard)(TaskObjF *self); /* TaskObjF__FormatCard; setState(0x13) */
    /* +0x054 */ s32 (*probeMemcardFile)(TaskObjF *self, char *destTitle,
                                         char *suffix); /* TaskObjF__ProbeMemcardFile: nonzero when the file opens */
    /* +0x058 */ char *(*findUnusedMemcardName)(TaskObjF *self, char *buf, char *prefix,
                                                char **suffixes); /* TaskObjF__FindUnusedMemcardName; setState(0x14) */
    /* +0x05C */ s32 (*collectExistingMemcardFiles)(TaskObjF *self, char **destTitles,
                                                    char **outSuffixes, char *prefix,
                                                    char **suffixes); /* TaskObjF__CollectExistingMemcardFiles; beginLoad */
    /* +0x060 */ s32 (*checkCardSpace)(TaskObjF *self, u8 iconFrames,
                                       s32 size); /* TaskObjF__CheckCardSpace; beginSave */
    /* +0x064 */ s32 (*readMemcardFile)(TaskObjF *self, char *suffix, void *outBuf,
                                        s32 outSize); /* TaskObjF__ReadMemcardFile; setState(0x15) */
    /* +0x068 */ s32 (*writeMemcardSaveFile)(TaskObjF *self, char *fileName, char *title,
                                             char iconFrames, struct TimImage *icon, void *data,
                                             s32 size); /* TaskObjF__WriteMemcardSaveFile; setState(0x14) */
    /* +0x06C */ void (*init)(TaskObjF *self, char *namePrefix, char **nameSuffixes,
                              BasicClass *inputSource, BasicClass *tickSource,
                              struct Class6B5CC *spriteParent, struct VabStreamObj *sound); /* TaskObjF__Init */
    /* +0x070 */ void (*deinit)(TaskObjF *self); /* TaskObjF__Deinit */
    /* +0x074 */ void (*beginLoad)(TaskObjF *self, char *fileName, char *title, void *data,
                                   s32 size); /* TaskObjF__BeginLoad */
    /* +0x078 */ void (*beginSave)(TaskObjF *self, char *fileName, char *title, s32 titleEditPos,
                                   u8 iconFrames, struct TimImage *icon, void *data,
                                   s32 size);                 /* TaskObjF__BeginSave */
    /* +0x07C */ void (*setState)(TaskObjF *self, s32 state); /* TaskObjF__SetState */
    /* +0x080 */ void (*loadCardIcon)(TaskObjF *self, s32 index); /* TaskObjF__LoadCardIcon: CARD\<gCardIconNames[index]>.TIM */
    /* +0x084 */ void (*releaseCardIcon)(TaskObjF *self); /* TaskObjF__ReleaseCardIcon */
    /* +0x088 */ void (*onInputEvent)(TaskObjF *self, void *sender,
                                      s32 event); /* TaskObjF__OnInputEvent (reads self and event) */
    /* +0x08C */ void (*playSound)(TaskObjF *self, s32 index); /* TaskObjF__PlaySound */
    /* +0x090 */ void (*advanceState)(TaskObjF *self); /* TaskObjF__AdvanceState; onInputEvent's 0x19 */
    /* +0x094 */ void (*forceIdleFromState)(TaskObjF *self); /* TaskObjF__ForceIdleFromState; onInputEvent's 0x17 */
    /* +0x098 */ void (*tickStateDelay)(TaskObjF *self, void *sender,
                                        s32 event); /* TaskObjF__TickStateDelay (reads only self; see the banner) */
    /* +0x09C */ void (*attachTextEntry)(TaskObjF *self); /* TaskObjF__AttachTextEntry; setState(0x11) */
    /* +0x0A0 */ void (*detachTextEntry)(TaskObjF *self); /* TaskObjF__DetachTextEntry */
    /* +0x0A4 */ void (*onTextEntryResult)(TaskObjF *self, void *sender,
                                           s32 result); /* TaskObjF__OnTextEntryResult */
    /* +0x0A8 */ void (*attachItemList)(TaskObjF *self); /* TaskObjF__AttachItemList; setState(0x12) */
    /* +0x0AC */ void (*detachItemList)(TaskObjF *self); /* TaskObjF__DetachItemList */
    /* +0x0B0 */ void (*onItemListResult)(TaskObjF *self, struct Class86F88 *sender,
                                          s32 result); /* TaskObjF__OnItemListResult */
};

struct TaskObjF {
    BASICCLASS_FIELDS(TaskObjFMethods);
    /* +0x00C */ s32 cardSlot; /* setCardSlot; 0 or 1: BuildMemcardPath's "bu00:"/"bu10:" */
    /* +0x010 */ s32 cardHandle; /* setCardSlot: cardSlot << 4, the _card_info/_card_load/_card_clear channel */
    /* +0x014 */ s32 events[4]; /* openEvents: OpenEvent per D_80086E78 entry; ForEachEvent/WaitForReadyEvent walk them */
    /* +0x024 */ s32 opMode; /* 1 beginLoad, 2 beginSave; the terminal states clear it. advanceState retries the one that is set */
    /* +0x028 */ s32 state;  /* setState; init clears it */
    /* +0x02C */ s32 bufCount; /* collectExistingMemcardFiles's count; how many `titles` buffers are kept */
    /* +0x030 */ char *namePrefix; /* init: the product code ("BISLPS-01556", D_8008A9D0); file names are prefix + suffix */
    /* +0x034 */ char **nameSuffixes; /* init: a NULL-terminated table of candidate suffixes (D_80086D6C) */
    /* +0x038 */ char **titles; /* AllocBuffers: 16 pointers, 15 buffers of 0x41; the existing files' titles; the item list's strings */
    /* +0x03C */ char **foundSuffixes; /* AllocBuffers: 0x40 bytes; the suffix of each file in `titles` */
    /* +0x040 */ char *fileName; /* beginLoad/beginSave; advanceState builds it from the chosen entry */
    /* +0x044 */ char *title; /* beginLoad/beginSave; the text entry edits it from titleEditPos */
    /* +0x048 */ s32 titleEditPos; /* beginSave: first full-width character of `title` the text entry edits */
    /* +0x04C */ u8 iconFrames; /* beginSave: icon frame count, the save header's 0x10 + n and n * 0x80 bytes */
    /* +0x04D */ u8 pad04D[0x050 - 0x04D];
    /* +0x050 */ struct TimImage *iconImage; /* beginSave: the icon TIM; writeMemcardSaveFile copies its CLUT and frames */
    /* +0x054 */ void *data;   /* beginLoad/beginSave: the save block read into or written from */
    /* +0x058 */ s32 dataSize; /* its size in bytes */
    /* +0x05C */ s32 waitCounter; /* setState zeroes; tickStateDelay counts to 6 */
    /* +0x060 */ BasicClass *inputSource; /* AddChild: the child whose class id's low nibble is 2; its events go to onInputEvent */
    /* +0x064 */ BasicClass *tickSource; /* ... low nibble 5; its events go to tickStateDelay */
    /* +0x068 */ struct Class6B5CC *spriteParent; /* init; the widgets' and the card icon's sprite parent; with inputSource, gates attach/detach */
    /* +0x06C */ struct VabStreamObj *sound; /* init; playSound's playTone target, the widgets' target too */
    /* +0x070 */ struct ScreenSprite *cardIcon; /* loadCardIcon: New_ScreenSprite of a CARD\*.TIM; releaseCardIcon */
    /* +0x074 */ s32 ownsWidget; /* attachTextEntry/attachItemList set it when they made the widget; detach then releases it */
    /* +0x078 */ struct TextEntry *textEntry; /* attachTextEntry: New_TextEntry(title + 2 * titleEditPos, 1); AddChild's class 0x10 */
    /* +0x07C */ struct Class86F88 *itemList; /* attachItemList: New_Class86F88(titles, 1); AddChild's class 0x20 */
    /* +0x080 */ s32 selectedIndex; /* onItemListResult: the list's getCursorIndex */
};

extern TaskObjFMethods gTaskObjFMethods;
extern TaskObjFMethods *GetTaskObjFMethods(void); /* returns &gTaskObjFMethods */

/* The class's own methods, in address order. */
TaskObjF *New_TaskObjF(s32 padEnable, s32 cardSlot);
void TaskObjF__TaskObjF(TaskObjF *self, s32 padEnable, s32 cardSlot);
void TaskObjF__ClearResourceSlots(TaskObjF *self);
void TaskObjF__Finalize(TaskObjF *self);
void TaskObjF__AddChild(TaskObjF *self, BasicClass *child);
void TaskObjF__RemoveChild(TaskObjF *self, BasicClass *child);
void TaskObjF__RemoveAllChildren(TaskObjF *self);
void TaskObjF__SetCardSlot(TaskObjF *self, s32 cardSlot);
s32 TaskObjF__OpenEvents(TaskObjF *self);
s32 TaskObjF__CloseEvents(TaskObjF *self);
s32 TaskObjF__CheckCardStatus(TaskObjF *self, s32 *p1, s32 *p2, s32 *p3);
s32 TaskObjF__CardInfoAndLoadStatus(TaskObjF *self, s32 *p1, s32 *p2, s32 *p3);
s32 TaskObjF__CardInfoStatus(TaskObjF *self, s32 *p1, s32 *p2);
s32 TaskObjF__CardLoadStatus(TaskObjF *self, s32 *p1, s32 *p2);
s32 TaskObjF__FormatCard(TaskObjF *self);
s32 TaskObjF__ProbeMemcardFile(TaskObjF *self, char *destBuf, char *suffix);
s32 TaskObjF__OpenAndReadMemcardFile(TaskObjF *self, char *destBuf, char *suffix);
char *TaskObjF__FindUnusedMemcardName(TaskObjF *self, char *buf, char *middle, char **entries);
s32 TaskObjF__CollectExistingMemcardFiles(TaskObjF *self, char **destBufs, char **outArr,
                                          char *middle, char **entries);
s32 TaskObjF__CheckCardSpace(TaskObjF *self, u8 id, s32 sizeArg);
s32 TaskObjF__ProbeCardFreeSpace(TaskObjF *self, u8 id, s32 sizeArg);
s32 TaskObjF__ReadMemcardFile(TaskObjF *self, char *suffix, void *outBuf, s32 outSize);
s32 TaskObjF__TryReadMemcardFile(TaskObjF *self, char *suffix, void *outBuf, s32 outSize);
s32 TaskObjF__WriteMemcardSaveFile(TaskObjF *self, char *fileName, char *title, char a3,
                                   struct TimImage *icon, void *data, s32 size);
s32 TaskObjF__TryWriteMemcardSaveFile(TaskObjF *self, char *fileName, char *title, u8 a3,
                                      struct TimImage *icon, void *data, s32 size);
s32 TaskObjF__EnableEvents(TaskObjF *self);
s32 TaskObjF__DisableEvents(TaskObjF *self);
s32 TaskObjF__TestEvents(TaskObjF *self);
s32 TaskObjF__ForEachEvent(TaskObjF *self, s32 (*callback)(s32), s32 flag);
s32 TaskObjF__WaitForReadyEvent(TaskObjF *self);
void TaskObjF__Init(TaskObjF *self, char *namePrefix, char **nameSuffixes, BasicClass *inputSource,
                    BasicClass *tickSource, struct Class6B5CC *spriteParent, struct VabStreamObj *sound);
void TaskObjF__Deinit(TaskObjF *self);
void TaskObjF__BeginLoad(TaskObjF *self, char *fileName, char *title, void *data, s32 size);
void TaskObjF__AllocBuffers(TaskObjF *self);
void TaskObjF__FreeUnusedBuffers(TaskObjF *self);
void TaskObjF__FreeBuffers(TaskObjF *self);
void TaskObjF__BeginSave(TaskObjF *self, char *fileName, char *title, s32 titleEditPos,
                         u8 iconFrames, struct TimImage *icon, void *data, s32 size);
s32 TaskObjF__Validate(TaskObjF *self);
void TaskObjF__OnNotify(TaskObjF *self, void *sender, s32 event);
void TaskObjF__SetState(TaskObjF *self, s32 state);
void TaskObjF__LoadCardIcon(TaskObjF *self, s32 index);
void TaskObjF__ReleaseCardIcon(TaskObjF *self);
void TaskObjF__OnInputEvent(TaskObjF *self, void *sender, s32 event);
void TaskObjF__PlaySound(TaskObjF *self, s32 index);
void TaskObjF__AdvanceState(TaskObjF *self);
void TaskObjF__ForceIdleFromState(TaskObjF *self);
void TaskObjF__TickStateDelay(TaskObjF *self);
void TaskObjF__AttachTextEntry(TaskObjF *self);
void TaskObjF__DetachTextEntry(TaskObjF *self);
void TaskObjF__OnTextEntryResult(TaskObjF *self, void *sender, s32 result);
void TaskObjF__AttachItemList(TaskObjF *self);
void TaskObjF__DetachItemList(TaskObjF *self);
void TaskObjF__OnItemListResult(TaskObjF *self, struct Class86F88 *sender, s32 result);

#endif
