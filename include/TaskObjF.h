#ifndef TASKOBJF_H
#define TASKOBJF_H

#include "BasicClass.h"

/*
 * TaskObjF -- the memory-card save/load controller (class id 0xB, method
 * table gTaskObjFMethods): BasicClass's direct subclass, no class below it.
 * Methods: the allocator and ctor,
 * BasicClass's overrides and the card primitives (+0x00C..+0x060), the file
 * I/O, events, buffers and the two operations (+0x064..+0x078, +0x038) in
 * src/TitleMenuTaskObjF.c, the state machine (+0x07C..+0x0B0) in
 * src/TitleMenuTaskObjF.c.
 *
 *  - The ctor runs InitCARD/StartCARD/_bu_init once per boot (sTaskObjFCount)
 *    and setCardSlot(cardSlot); +0x040..+0x068 wrap the PS-X memory-card
 *    BIOS: events (OpenEvent x4), _card_info/_card_load, format, and
 *    open/read/write/delete on "bu00:"/"bu10:" paths (BuildMemcardPath).
 *  - init (+0x06C) takes the name prefix and suffix table, the two event
 *    sources (added as children; TaskObjF__AddChild files each child by its
 *    class id: low nibble 2 -> inputSource, 5 -> tickSource, 0x10 ->
 *    textEntry, 0x20 -> itemList), the sprite parent and the sound. Its one
 *    caller is TitleMenu__BeginCardAccess: initArgs->unk4, unk10 (a
 *    FrameClock, id 0x5), unk14, and TaskCore's `sound` (a VabStreamObj).
 *  - beginLoad (+0x074, opMode 1) lists the save files that exist
 *    (collectExistingMemcardFiles into `titles`/`foundSuffixes`), lets the
 *    player pick one in a ItemList list (state 0x12), then reads it into
 *    `data` (state 0x15, readMemcardFile). beginSave (+0x078, opMode 2)
 *    checks for the file and for space, lets the player edit the title in a
 *    TextEntry (state 0x11), then writes the save file with its icon
 *    (state 0x14, writeMemcardSaveFile).
 *  - setState (+0x07C) runs notifyParents(state) first; the terminal states
 *    0x16/0x17 free the load buffers and return to state 0. TitleMenu's
 *    onCardEvent (the parent) ends the save on them.
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
struct ItemList;
struct ScreenSprite;
struct VabStreamObj;
struct TimImage;
struct SceneNode;

typedef struct TaskObjF TaskObjF;
typedef struct TaskObjFMethods TaskObjFMethods;

/* BasicClass's slots (overrides: +0x008 TaskObjF__TaskObjF, +0x00C Finalize,
 * +0x010 AddChild, +0x014 RemoveChild, +0x018 RemoveAllChildren, +0x038
 * OnNotify; `tools/classtable.py gTaskObjFMethods --vs gBasicClassMethods`), then
 * this class's own, every one of them filled. */
struct TaskObjFMethods {
    BASICCLASS_SLOTS(TaskObjF, (TaskObjF * self, s32 padEnable, s32 cardSlot));
    /* +0x040 */ void (*setCardSlot)(TaskObjF *self, s32 cardSlot); /* TaskObjF__SetCardSlot; the ctor's last call */
    /* +0x044 */ s32 (*openEvents)(TaskObjF *self);  /* TaskObjF__OpenEvents */
    /* +0x048 */ s32 (*closeEvents)(TaskObjF *self); /* TaskObjF__CloseEvents */
    /* +0x04C */ s32 (*checkCardStatus)(TaskObjF *self, s32 *error, s32 *cardChanged,
                                        s32 *formatted); /* TaskObjF__CheckCardStatus; Validate reads the three */
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
                              struct SceneNode *spriteParent, struct VabStreamObj *sound); /* TaskObjF__Init */
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
    /* +0x094 */ void (*abortFromState)(TaskObjF *self); /* TaskObjF__AbortFromState; onInputEvent's 0x17 */
    /* +0x098 */ void (*tickStateDelay)(TaskObjF *self, void *sender,
                                        s32 event); /* TaskObjF__TickStateDelay (reads only self; see the banner) */
    /* +0x09C */ void (*attachTextEntry)(TaskObjF *self); /* TaskObjF__AttachTextEntry; setState(0x11) */
    /* +0x0A0 */ void (*detachTextEntry)(TaskObjF *self); /* TaskObjF__DetachTextEntry */
    /* +0x0A4 */ void (*onTextEntryResult)(TaskObjF *self, void *sender,
                                           s32 result); /* TaskObjF__OnTextEntryResult */
    /* +0x0A8 */ void (*attachItemList)(TaskObjF *self); /* TaskObjF__AttachItemList; setState(0x12) */
    /* +0x0AC */ void (*detachItemList)(TaskObjF *self); /* TaskObjF__DetachItemList */
    /* +0x0B0 */ void (*onItemListResult)(TaskObjF *self, struct ItemList *sender,
                                          s32 result); /* TaskObjF__OnItemListResult */
};

struct TaskObjF {
    BASICCLASS_FIELDS(TaskObjFMethods);
    /* +0x00C */ s32 cardSlot; /* setCardSlot; 0 or 1: BuildMemcardPath's "bu00:"/"bu10:" */
    /* +0x010 */ s32 cardHandle; /* setCardSlot: cardSlot << 4, the _card_info/_card_load/_card_clear channel */
    /* +0x014 */ s32 events[4]; /* openEvents: OpenEvent per gCardEventSpecs entry; ForEachEvent/WaitForReadyEvent walk them */
    /* +0x024 */ s32 opMode; /* 1 beginLoad, 2 beginSave; the terminal states clear it. advanceState retries the one that is set */
    /* +0x028 */ s32 state;  /* setState; init clears it */
    /* +0x02C */ s32 bufCount; /* collectExistingMemcardFiles's count; how many `titles` buffers are kept */
    /* +0x030 */ char *namePrefix; /* init: the product code ("BISLPS-01556", sCardFilePrefix); file names are prefix + suffix */
    /* +0x034 */ char **nameSuffixes; /* init: a NULL-terminated table of candidate suffixes (sSaveFileSuffixes) */
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
    /* +0x068 */ struct SceneNode *spriteParent; /* init; the widgets' and the card icon's sprite parent; with inputSource, gates attach/detach */
    /* +0x06C */ struct VabStreamObj *sound; /* init; playSound's playTone target, the widgets' target too */
    /* +0x070 */ struct ScreenSprite *cardIcon; /* loadCardIcon: New_ScreenSprite of a CARD\*.TIM; releaseCardIcon */
    /* +0x074 */ s32 ownsWidget; /* attachTextEntry/attachItemList set it when they made the widget; detach then releases it */
    /* +0x078 */ struct TextEntry *textEntry; /* attachTextEntry: New_TextEntry(title + 2 * titleEditPos, 1); AddChild's class 0x10 */
    /* +0x07C */ struct ItemList *itemList; /* attachItemList: New_ItemList(titles, 1); AddChild's class 0x20 */
    /* +0x080 */ s32 selectedIndex; /* onItemListResult: the list's getCursorIndex */
};

/* TaskObjF's class id (gTaskObjFMethods word +0x000). A single nibble, so
 * `(header & 0xF) == TASKOBJF_CLASS_ID` is its is-kind-of test
 * (TitleMenu__OnNotify). */
#define TASKOBJF_CLASS_ID 0xB

/* TaskObjF::opMode: the operation beginLoad or beginSave started. */
enum TaskObjFOpMode {
    TASKOBJF_OP_NONE = 0, /* init, and the terminal states */
    TASKOBJF_OP_LOAD = 1, /* beginLoad */
    TASKOBJF_OP_SAVE = 2  /* beginSave */
};

/* TaskObjF::state, setState's argument. 0x02..0x10 show a message:
 * loadCardIcon indexes gCardIconNames by the state, and the quoted name is
 * the CARD\<name>.TIM each one loads. 0x11..0x15 run an entry action in
 * setState. 0x16 and 0x17 are terminal: setState clears state and opMode
 * on either, and frees beginLoad's buffers. No code sets state 1. */
enum TaskObjFState {
    TASKOBJF_STATE_IDLE = 0x00,
    TASKOBJF_STATE_NO_CARD = 0x02,      /* "NOCONECT": checkCardStatus failed (timeout or error) */
    TASKOBJF_STATE_CARD_ERROR = 0x03,   /* "ERROR": the card answered EvSpERROR */
    TASKOBJF_STATE_CARD_CHANGED = 0x04, /* "CHANGE": _card_info reported a new card */
    TASKOBJF_STATE_UNFORMATTED_LOAD = 0x05, /* "UNFORM1": unformatted card, opMode LOAD */
    TASKOBJF_STATE_UNFORMATTED_SAVE = 0x06, /* "UNFORM2": unformatted card, opMode SAVE; confirming formats it */
    TASKOBJF_STATE_FORMATTING = 0x07,    /* "FORMING": tickStateDelay, then FORMAT */
    TASKOBJF_STATE_FORMAT_ERROR = 0x08,  /* "FORMERR": formatCard failed */
    TASKOBJF_STATE_SAVE_NO_SPACE = 0x09, /* "SAVEEMPT": checkCardSpace failed */
    TASKOBJF_STATE_SAVE_OVERWRITE_WARNING = 0x0A, /* "SAVEWAR": the file exists; confirming re-runs beginSave */
    TASKOBJF_STATE_SAVING = 0x0B,                 /* "SAVING": tickStateDelay, then WRITE */
    TASKOBJF_STATE_SAVE_ERROR = 0x0C,             /* "SAVEERR": writeMemcardSaveFile failed */
    TASKOBJF_STATE_LOAD_NOT_FOUND = 0x0D, /* "NOTFOUND": collectExistingMemcardFiles found none */
    TASKOBJF_STATE_LOAD_WARNING = 0x0E, /* "LOADWAR": a file was chosen; confirming re-runs beginLoad */
    TASKOBJF_STATE_LOADING = 0x0F,      /* "LOADING": tickStateDelay, then READ */
    TASKOBJF_STATE_LOAD_ERROR = 0x10,  /* "LOADERR": readMemcardFile failed */
    TASKOBJF_STATE_EDIT_TITLE = 0x11,  /* attachTextEntry: the player edits the save title */
    TASKOBJF_STATE_CHOOSE_FILE = 0x12, /* attachItemList: the player picks the file to load */
    TASKOBJF_STATE_FORMAT = 0x13,      /* formatCard, then EDIT_TITLE or FORMAT_ERROR */
    TASKOBJF_STATE_WRITE = 0x14,       /* writeMemcardSaveFile, then DONE or SAVE_ERROR */
    TASKOBJF_STATE_READ = 0x15,        /* readMemcardFile, then DONE or LOAD_ERROR */
    TASKOBJF_STATE_DONE = 0x16,        /* the write or the read succeeded */
    TASKOBJF_STATE_ABORTED = 0x17 /* cancelled, an error dismissed, or setState to the current state */
};

/* beginLoad's title buffers (AllocBuffers): `titles` and `foundSuffixes`
 * hold TASKOBJF_MAX_FILES + 1 pointers, the last one NULL, and each title
 * buffer is TASKOBJF_TITLE_SIZE bytes. beginLoad sets bufCount to
 * TASKOBJF_MAX_FILES when no file was found, so FreeBuffers frees them all. */
#define TASKOBJF_MAX_FILES 15
#define TASKOBJF_TITLE_SIZE 65

/* PS-X memory-card geometry, as this class's file I/O computes it. A file
 * is read and written in 128-byte sectors and allocated in 8192-byte
 * blocks; its first 512 bytes are the save header (the title sector, then
 * up to three icon frames of one sector each), whose byte 2 is
 * MEMCARD_ICON_FLAG_BASE plus the icon frame count. */
#define MEMCARD_SECTOR_SIZE 128
#define MEMCARD_SECTOR_SHIFT 7
#define MEMCARD_BLOCK_SIZE 8192
#define MEMCARD_BLOCK_SHIFT 13
#define MEMCARD_SAVE_HEADER_SIZE 512
#define MEMCARD_ICON_FLAG_BASE 0x10
/* open()'s create mode carries the new file's block count in its high half. */
#define MEMCARD_OPEN_BLOCKS(blocks) ((blocks) << 16)
/* Attempts after the first before a card operation gives up. */
#define MEMCARD_RETRIES 10

extern TaskObjFMethods gTaskObjFMethods;
extern TaskObjFMethods *GetTaskObjFMethods(void); /* returns &gTaskObjFMethods */

/* TaskObjF__TaskObjF's construction count: InitCARD/StartCARD/_bu_init run
 * only on the first construction, when it was 0 before the increment. */
extern s32 sTaskObjFCount;

/* The kernel event calls TaskObjF's event methods make (OpenEvent,
 * EnableEvent, DisableEvent, TestEvent; Sony's libapi) are declared in the
 * units that call them. */

/* Per TaskObjF::events slot: the event spec TaskObjF__OpenEvents passes to
 * OpenEvent, and the value WaitForReadyEvent returns for that slot.
 * Unsized: only the four slots are read. */
extern s32 gCardEventSpecs[];

/* A 6-byte memory-card device name, "bu00:" or "bu10:" (the BIOS names of
 * the two card slots). BuildMemcardPath copies one as a whole struct.
 * MATCHING: all-s8 members (alignment 1) make that copy retail's unaligned
 * lwl/lwr plus byte stores. */
typedef struct McDevicePath {
    s8 b0, b1, b2, b3, b4, b5;
} McDevicePath;

extern McDevicePath gMcDevicePath1; /* "bu10:" */
extern McDevicePath gMcDevicePath0; /* "bu00:" */

/* Game code (src/TitleMenuTaskObjF.c). TaskObjF__WriteMemcardSaveFile calls it
 * around its retry loop, and with (arg, 0) when the loop gives up. The BIOS
 * file calls (open, read, lseek, close, delete; Sony's libapi) are declared
 * in the units that call them. */
extern s32 StampSaveTitleFileLetter(char *titleText, char *fileName);

/* The class's own methods, in address order. */
TaskObjF *New_TaskObjF(s32 padEnable, s32 cardSlot);
void TaskObjF__TaskObjF(TaskObjF *self, s32 padEnable, s32 cardSlot);
void TaskObjF__ClearLinks(TaskObjF *self);
void TaskObjF__Finalize(TaskObjF *self);
void TaskObjF__AddChild(TaskObjF *self, BasicClass *child);
void TaskObjF__RemoveChild(TaskObjF *self, BasicClass *child);
void TaskObjF__RemoveAllChildren(TaskObjF *self);
void TaskObjF__SetCardSlot(TaskObjF *self, s32 cardSlot);
s32 TaskObjF__OpenEvents(TaskObjF *self);
s32 TaskObjF__CloseEvents(TaskObjF *self);
s32 TaskObjF__CheckCardStatus(TaskObjF *self, s32 *error, s32 *cardChanged, s32 *formatted);
s32 TaskObjF__CardInfoAndLoadStatus(TaskObjF *self, s32 *error, s32 *cardChanged, s32 *formatted);
s32 TaskObjF__CardInfoStatus(TaskObjF *self, s32 *error, s32 *cardChanged);
s32 TaskObjF__CardLoadStatus(TaskObjF *self, s32 *error, s32 *formatted);
s32 TaskObjF__FormatCard(TaskObjF *self);
s32 TaskObjF__ProbeMemcardFile(TaskObjF *self, char *destTitle, char *suffix);
s32 TaskObjF__OpenAndReadMemcardFile(TaskObjF *self, char *destTitle, char *suffix);
char *TaskObjF__FindUnusedMemcardName(TaskObjF *self, char *buf, char *prefix, char **suffixes);
s32 TaskObjF__CollectExistingMemcardFiles(TaskObjF *self, char **destTitles, char **outSuffixes,
                                          char *prefix, char **suffixes);
s32 TaskObjF__CheckCardSpace(TaskObjF *self, u8 iconFrames, s32 size);
s32 TaskObjF__ProbeCardFreeSpace(TaskObjF *self, u8 iconFrames, s32 size);
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
                    BasicClass *tickSource, struct SceneNode *spriteParent, struct VabStreamObj *sound);
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
void TaskObjF__AbortFromState(TaskObjF *self);
void TaskObjF__TickStateDelay(TaskObjF *self);
void TaskObjF__AttachTextEntry(TaskObjF *self);
void TaskObjF__DetachTextEntry(TaskObjF *self);
void TaskObjF__OnTextEntryResult(TaskObjF *self, void *sender, s32 result);
void TaskObjF__AttachItemList(TaskObjF *self);
void TaskObjF__DetachItemList(TaskObjF *self);
void TaskObjF__OnItemListResult(TaskObjF *self, struct ItemList *sender, s32 result);

#endif
