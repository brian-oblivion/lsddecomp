/**
 * @file task_objf.h
 * @brief TaskObjF, the memory-card save/load controller, and the PS-X save file layout.
 *
 * Declares the TaskObjF class (object and method table), its operation and
 * state enums, the memory-card geometry constants and the structs of a save
 * file's header and icon, and TaskObjF's methods, which are defined in
 * src/ui/task_objf.c.
 */
#ifndef TASK_OBJF_H
#define TASK_OBJF_H

#include "basic_class.h"

struct TextEntry;
struct ItemList;
struct ScreenSprite;
struct VabStreamObj;
struct TimImage;
struct SceneNode;

typedef struct TaskObjF TaskObjF;
typedef struct TaskObjFMethods TaskObjFMethods;

/**
 * @brief TaskObjF's method table: BasicClass's slots, then TaskObjF's own, every one filled.
 *
 * TaskObjF overrides the inherited +0x008 (TaskObjF__TaskObjF), +0x00C
 * (Finalize), +0x010 (AddChild), +0x014 (RemoveChild), +0x018
 * (RemoveAllChildren) and +0x038 (OnNotify).
 */
struct TaskObjFMethods {
    BASICCLASS_SLOTS(TaskObjF, (TaskObjF * self, s32 padEnable, s32 cardSlot));
    /* +0x040 */ void (*setCardSlot)(TaskObjF *self, s32 cardSlot); /**< @see TaskObjF__SetCardSlot; the ctor's last call */
    /* +0x044 */ s32 (*openEvents)(TaskObjF *self);  /**< @see TaskObjF__OpenEvents */
    /* +0x048 */ s32 (*closeEvents)(TaskObjF *self); /**< @see TaskObjF__CloseEvents */
    /* +0x04C */ s32 (*checkCardStatus)(TaskObjF *self, s32 *error, s32 *cardChanged,
                                        s32 *formatted); /**< @see TaskObjF__CheckCardStatus; Validate reads the three */
    /* +0x050 */ s32 (*formatCard)(TaskObjF *self); /**< @see TaskObjF__FormatCard; setState(0x13) */
    /* +0x054 */ s32 (*probeMemcardFile)(TaskObjF *self, char *destTitle,
                                         char *suffix); /**< @see TaskObjF__ProbeMemcardFile */
    /* +0x058 */ char *(*findUnusedMemcardName)(TaskObjF *self, char *buf, char *prefix,
                                                char **suffixes); /**< @see TaskObjF__FindUnusedMemcardName; setState(0x14) */
    /* +0x05C */ s32 (*collectExistingMemcardFiles)(TaskObjF *self, char **destTitles,
                                                    char **outSuffixes, char *prefix,
                                                    char **suffixes); /**< @see TaskObjF__CollectExistingMemcardFiles; beginLoad */
    /* +0x060 */ s32 (*checkCardSpace)(TaskObjF *self, u8 iconFrames,
                                       s32 size); /**< @see TaskObjF__CheckCardSpace; beginSave */
    /* +0x064 */ s32 (*readMemcardFile)(TaskObjF *self, char *suffix, void *outBuf,
                                        s32 outSize); /**< @see TaskObjF__ReadMemcardFile; setState(0x15) */
    /* +0x068 */ s32 (*writeMemcardSaveFile)(TaskObjF *self, char *fileName, char *title,
                                             char iconFrames, struct TimImage *icon, void *data,
                                             s32 size); /**< @see TaskObjF__WriteMemcardSaveFile; setState(0x14) */
    /* +0x06C */ void (*init)(TaskObjF *self, char *namePrefix, char **nameSuffixes,
                              BasicClass *inputSource, BasicClass *tickSource,
                              struct SceneNode *spriteParent,
                              struct VabStreamObj *sound); /**< @see TaskObjF__Init */
    /* +0x070 */ void (*deinit)(TaskObjF *self);           /**< @see TaskObjF__Deinit */
    /* +0x074 */ void (*beginLoad)(TaskObjF *self, char *fileName, char *title, void *data,
                                   s32 size); /**< @see TaskObjF__BeginLoad */
    /* +0x078 */ void (*beginSave)(TaskObjF *self, char *fileName, char *title, s32 titleEditPos,
                                   u8 iconFrames, struct TimImage *icon, void *data,
                                   s32 size);                     /**< @see TaskObjF__BeginSave */
    /* +0x07C */ void (*setState)(TaskObjF *self, s32 state);     /**< @see TaskObjF__SetState */
    /* +0x080 */ void (*loadCardIcon)(TaskObjF *self, s32 index); /**< @see TaskObjF__LoadCardIcon */
    /* +0x084 */ void (*releaseCardIcon)(TaskObjF *self); /**< @see TaskObjF__ReleaseCardIcon */
    /* +0x088 */ void (*onInputEvent)(TaskObjF *self, void *sender, s32 event); /**< @see TaskObjF__OnInputEvent */
    /* +0x08C */ void (*playSound)(TaskObjF *self, s32 index); /**< @see TaskObjF__PlaySound */
    /* +0x090 */ void (*advanceState)(TaskObjF *self); /**< @see TaskObjF__AdvanceState; onInputEvent's circle */
    /* +0x094 */ void (*abortFromState)(TaskObjF *self); /**< @see TaskObjF__AbortFromState; onInputEvent's cross */
    /* +0x098 */ void (*tickStateDelay)(TaskObjF *self, void *sender,
                                        s32 event); /**< @see TaskObjF__TickStateDelay, which reads only self */
    /* +0x09C */ void (*attachTextEntry)(TaskObjF *self); /**< @see TaskObjF__AttachTextEntry; setState(0x11) */
    /* +0x0A0 */ void (*detachTextEntry)(TaskObjF *self); /**< @see TaskObjF__DetachTextEntry */
    /* +0x0A4 */ void (*onTextEntryResult)(TaskObjF *self, void *sender,
                                           s32 result); /**< @see TaskObjF__OnTextEntryResult */
    /* +0x0A8 */ void (*attachItemList)(TaskObjF *self); /**< @see TaskObjF__AttachItemList; setState(0x12) */
    /* +0x0AC */ void (*detachItemList)(TaskObjF *self); /**< @see TaskObjF__DetachItemList */
    /* +0x0B0 */ void (*onItemListResult)(TaskObjF *self, struct ItemList *list,
                                          s32 result); /**< @see TaskObjF__OnItemListResult */
};

/**
 * @brief The memory-card save/load controller: checks the card, edits the title, picks and moves the file.
 *
 * Class id 0xB (TASKOBJF_CLASS_ID), method table gTaskObjFMethods, parent
 * BasicClass; no class below it. All its methods are in src/ui/task_objf.c.
 *
 *  - The ctor runs InitCARD/StartCARD/_bu_init once per boot
 *    (sTaskObjFCount) and setCardSlot(cardSlot); +0x040..+0x068 wrap the
 *    PS-X memory-card BIOS: events (OpenEvent x4), _card_info/_card_load,
 *    format, and open/read/write/delete on "bu00:"/"bu10:" paths
 *    (BuildMemcardPath).
 *  - init (+0x06C) takes the name prefix and suffix table, the two event
 *    sources (added as children; TaskObjF__AddChild files each child by its
 *    class id: low nibble 2 -> inputSource, 5 -> tickSource, 0x10 ->
 *    textEntry, 0x20 -> itemList), the sprite parent and the sound. Its one
 *    caller is TitleMenu__BeginCardAccess: initArgs->pad, frameClock (a
 *    FrameClock, id 0x5), lightRig, and TaskCore's `sound` (a VabStreamObj).
 *  - beginLoad (+0x074, opMode 1) lists the save files that exist
 *    (collectExistingMemcardFiles into `titles`/`foundSuffixes`), lets the
 *    player pick one in an ItemList (state 0x12), then reads it into `data`
 *    (state 0x15, readMemcardFile). beginSave (+0x078, opMode 2) checks for
 *    the file and for space, lets the player edit the title in a TextEntry
 *    (state 0x11), then writes the save file with its icon (state 0x14,
 *    writeMemcardSaveFile).
 *  - setState (+0x07C) runs notifyParents(state) first; the terminal states
 *    0x16/0x17 free the load buffers and return to state 0. TitleMenu's
 *    onCardEvent (the parent) ends the save on them.
 *  - onNotify (+0x038) routes a child's notification by the child's class
 *    id, exactly as its AddChild files them: 2 -> onInputEvent, 5 ->
 *    tickStateDelay, 0x10 -> onTextEntryResult, 0x20 -> onItemListResult.
 *
 * The +0x098 tickStateDelay slot takes (sender, event), which onNotify
 * passes; TaskObjF__TickStateDelay reads only self (TextEntry's tickState
 * is the same case).
 *
 * The object is 0x84 bytes (New_TaskObjF).
 */
struct TaskObjF {
    BASICCLASS_FIELDS(TaskObjFMethods);
    /* +0x00C */ s32 cardSlot; /**< setCardSlot; 0 or 1: BuildMemcardPath's "bu00:"/"bu10:" */
    /* +0x010 */ s32 cardHandle; /**< setCardSlot: cardSlot << 4, the _card_info/_card_load/_card_clear channel */
    /* +0x014 */ s32 events[4]; /**< openEvents: OpenEvent per sCardEventSpecs entry; ForEachEvent/WaitForReadyEvent walk them */
    /* +0x024 */ s32 opMode; /**< a TaskObjFOpMode: 1 beginLoad, 2 beginSave; the terminal states clear it. advanceState retries the one that is set */
    /* +0x028 */ s32 state;  /**< a TaskObjFState (setState); init clears it */
    /* +0x02C */ s32 bufCount; /**< collectExistingMemcardFiles's count; how many `titles` buffers are kept */
    /* +0x030 */ char *namePrefix; /**< init: the product code ("BISLPS-01556", sCardFilePrefix); file names are prefix + suffix */
    /* +0x034 */ char **nameSuffixes; /**< init: a NULL-terminated table of candidate suffixes (sSaveFileSuffixes) */
    /* +0x038 */ char **titles; /**< AllocBuffers: 16 pointers, 15 buffers of 0x41; the existing files' titles; the item list's strings */
    /* +0x03C */ char **foundSuffixes; /**< AllocBuffers: 0x40 bytes; the suffix of each file in `titles` */
    /* +0x040 */ char *fileName; /**< beginLoad/beginSave; advanceState builds it from the chosen entry */
    /* +0x044 */ char *title; /**< beginLoad/beginSave; the text entry edits it from titleEditPos */
    /* +0x048 */ s32 titleEditPos; /**< beginSave: first full-width character of `title` the text entry edits */
    /* +0x04C */ u8 iconFrames; /**< beginSave: icon frame count, the save header's 0x10 + n and n * 0x80 bytes */
    /* +0x04D */ u8 pad04D[0x050 - 0x04D];
    /* +0x050 */ struct TimImage *iconImage; /**< beginSave: the icon TIM; writeMemcardSaveFile copies its CLUT and frames */
    /* +0x054 */ void *data;   /**< beginLoad/beginSave: the save block read into or written from */
    /* +0x058 */ s32 dataSize; /**< its size in bytes */
    /* +0x05C */ s32 waitCounter; /**< setState zeroes; tickStateDelay counts to 6 */
    /* +0x060 */ BasicClass *inputSource; /**< AddChild: the child of class PAD_CLASS_ID (a Pad); its events go to onInputEvent */
    /* +0x064 */ BasicClass *tickSource; /**< ... of class FRAMECLOCK_CLASS_ID (a FrameClock); its events go to tickStateDelay */
    /* +0x068 */ struct SceneNode *spriteParent; /**< init; the widgets' and the card icon's sprite parent; with inputSource, gates attach/detach */
    /* +0x06C */ struct VabStreamObj *sound; /**< init; playSound's playTone target, the widgets' target too */
    /* +0x070 */ struct ScreenSprite *cardIcon; /**< loadCardIcon: New_ScreenSprite of a CARD\*.TIM; releaseCardIcon */
    /* +0x074 */ s32 ownsWidget; /**< attachTextEntry/attachItemList set it when they made the widget; detach then releases it */
    /* +0x078 */ struct TextEntry *textEntry; /**< attachTextEntry: New_TextEntry(title + 2 * titleEditPos, 1); AddChild's TEXTENTRY_CLASS_ID child */
    /* +0x07C */ struct ItemList *itemList; /**< attachItemList: New_ItemList(titles, 1); AddChild's ITEMLIST_CLASS_ID child */
    /* +0x080 */ s32 selectedIndex; /**< onItemListResult: the list's getCursorIndex */
};

/** TaskObjF's class id (gTaskObjFMethods word +0x000). A single nibble, so
 * `(header & CLASS_ID_ROOT_MASK) == TASKOBJF_CLASS_ID` is its is-kind-of test
 * (TitleMenu__OnNotify). */
#define TASKOBJF_CLASS_ID 0xB

/** @name Tones
 * playSound's tones, VabStreamObj__PlayTone indices (program << 4 | tone). @{ */
#define TASKOBJF_TONE_PROCEED 0x00 /**< advanceState: circle retries, formats or goes on */
#define TASKOBJF_TONE_BACK 0x10    /**< circle on an error message, or cross: the operation ends */
#define TASKOBJF_TONE_VOLUME 127   /**< playSound's PlayTone vol and endVol */

/** @} */

/** TaskObjF::opMode: the operation beginLoad or beginSave started. */
enum TaskObjFOpMode {
    TASKOBJF_OP_NONE = 0, /**< init, and the terminal states */
    TASKOBJF_OP_LOAD = 1, /**< beginLoad */
    TASKOBJF_OP_SAVE = 2  /**< beginSave */
};

/** TaskObjF::state, setState's argument. 0x02..0x10 show a message:
 * loadCardIcon indexes sCardIconNames by the state, and the quoted name is
 * the CARD\<name>.TIM each one loads. 0x11..0x15 run an entry action in
 * setState. 0x16 and 0x17 are terminal: setState clears state and opMode
 * on either, and frees beginLoad's buffers. No code sets state 1. */
enum TaskObjFState {
    TASKOBJF_STATE_IDLE = 0x00,       /**< no operation */
    TASKOBJF_STATE_NO_CARD = 0x02,    /**< "NOCONECT": checkCardStatus failed (timeout or error) */
    TASKOBJF_STATE_CARD_ERROR = 0x03, /**< "ERROR": the card answered EvSpERROR */
    TASKOBJF_STATE_CARD_CHANGED = 0x04,     /**< "CHANGE": _card_info reported a new card */
    TASKOBJF_STATE_UNFORMATTED_LOAD = 0x05, /**< "UNFORM1": unformatted card, opMode LOAD */
    TASKOBJF_STATE_UNFORMATTED_SAVE = 0x06, /**< "UNFORM2": unformatted card, opMode SAVE; confirming formats it */
    TASKOBJF_STATE_FORMATTING = 0x07,    /**< "FORMING": tickStateDelay, then FORMAT */
    TASKOBJF_STATE_FORMAT_ERROR = 0x08,  /**< "FORMERR": formatCard failed */
    TASKOBJF_STATE_SAVE_NO_SPACE = 0x09, /**< "SAVEEMPT": checkCardSpace failed */
    TASKOBJF_STATE_SAVE_OVERWRITE_WARNING = 0x0A, /**< "SAVEWAR": the file exists; confirming re-runs beginSave */
    TASKOBJF_STATE_SAVING = 0x0B,                 /**< "SAVING": tickStateDelay, then WRITE */
    TASKOBJF_STATE_SAVE_ERROR = 0x0C,     /**< "SAVEERR": writeMemcardSaveFile failed */
    TASKOBJF_STATE_LOAD_NOT_FOUND = 0x0D, /**< "NOTFOUND": collectExistingMemcardFiles found none */
    TASKOBJF_STATE_LOAD_WARNING = 0x0E, /**< "LOADWAR": a file was chosen; confirming re-runs beginLoad */
    TASKOBJF_STATE_LOADING = 0x0F,     /**< "LOADING": tickStateDelay, then READ */
    TASKOBJF_STATE_LOAD_ERROR = 0x10,  /**< "LOADERR": readMemcardFile failed */
    TASKOBJF_STATE_EDIT_TITLE = 0x11,  /**< attachTextEntry: the player edits the save title */
    TASKOBJF_STATE_CHOOSE_FILE = 0x12, /**< attachItemList: the player picks the file to load */
    TASKOBJF_STATE_FORMAT = 0x13,      /**< formatCard, then EDIT_TITLE or FORMAT_ERROR */
    TASKOBJF_STATE_WRITE = 0x14,       /**< writeMemcardSaveFile, then DONE or SAVE_ERROR */
    TASKOBJF_STATE_READ = 0x15,        /**< readMemcardFile, then DONE or LOAD_ERROR */
    TASKOBJF_STATE_DONE = 0x16,        /**< the write or the read succeeded */
    TASKOBJF_STATE_ABORTED = 0x17 /**< cancelled, an error dismissed, or setState to the current state */
};

/** @name Load buffers
 * beginLoad's title buffers (AllocBuffers): `titles` and `foundSuffixes`
 * hold TASKOBJF_MAX_FILES + 1 pointers, the last one NULL, and each title
 * buffer is TASKOBJF_TITLE_SIZE bytes. beginLoad sets bufCount to
 * TASKOBJF_MAX_FILES when no file was found, so FreeBuffers frees them all.
 * @{ */
#define TASKOBJF_MAX_FILES 15  /**< save files -01..-15 */
#define TASKOBJF_TITLE_SIZE 65 /**< one title buffer's bytes */
/** @} */

/** @name PS-X memory-card geometry
 * As this class's file I/O computes it. A file is read and written in
 * 128-byte sectors and allocated in 8192-byte blocks; its first 512 bytes
 * are the save header (the title sector, then up to three icon frames of
 * one sector each), whose byte 2 is MEMCARD_ICON_FLAG_BASE plus the icon
 * frame count.
 * @{ */
#define MEMCARD_SECTOR_SIZE 128      /**< bytes a sector */
#define MEMCARD_SECTOR_SHIFT 7       /**< log2 of MEMCARD_SECTOR_SIZE */
#define MEMCARD_BLOCK_SIZE 8192      /**< bytes a block */
#define MEMCARD_BLOCK_SHIFT 13       /**< log2 of MEMCARD_BLOCK_SIZE */
#define MEMCARD_SAVE_HEADER_SIZE 512 /**< the title sector and three icon frames */
#define MEMCARD_ICON_FLAG_BASE 0x10  /**< the icon display flag for zero frames */
/** open()'s create mode carries the new file's block count in its high half. */
#define MEMCARD_OPEN_BLOCKS(blocks) ((blocks) << 16)
/** Attempts after the first before a card operation gives up. */
#define MEMCARD_RETRIES 10
/** @} */

/** TaskObjF's method table (class id TASKOBJF_CLASS_ID). */
extern TaskObjFMethods gTaskObjFMethods;

/**
 * @brief Returns TaskObjF's method table.
 * @return &gTaskObjFMethods.
 */
extern TaskObjFMethods *GetTaskObjFMethods(void);

/* The kernel event calls TaskObjF's event methods make (OpenEvent,
 * EnableEvent, DisableEvent, TestEvent; Sony's libapi) are declared in the
 * units that call them. */

/** A 6-byte memory-card device name, "bu00:" or "bu10:" (the BIOS names of
 * the two card slots), as six single bytes. BuildMemcardPath copies one as
 * a whole struct. */
typedef struct McDevicePath {
    s8 b0,  /**< 'b' */
        b1, /**< 'u' */
        b2, /**< the slot digit, '0' or '1' */
        b3, /**< '0' */
        b4, /**< ':' */
        b5; /**< the terminating NUL */
} McDevicePath;

/** The buffer a card file's full path ("bu00:" plus the file name) is built
 * in, on the stack of each file call. */
#define MEMCARD_PATH_SIZE 32

/** Half of the icon's 16-colour CLUT, as eight halfwords; copied whole. */
typedef struct IconPaletteHalf {
    s16 color[8]; /**< eight 15-bit colours */
} IconPaletteHalf;

/** One 16x16 4bpp icon frame, one sector, as bytes; copied whole. */
typedef struct IconFrame {
    u8 raw[MEMCARD_SECTOR_SIZE]; /**< the frame's pixels */
} IconFrame;

/** The icon TimImage's file buffer, a 4bpp TIM with one 16-colour CLUT: the
 * pads are the TIM header with the CLUT block header, and the pixel block
 * header. Only the CLUT and the first three frames of pixels are read. */
typedef struct McIconSource {
    u8 pad0[0x14];
    IconPaletteHalf palette[2]; /**< +0x14: the CLUT */
    u8 pad34[0x40 - 0x34];
    IconFrame frame0; /**< +0x40 */
    IconFrame frame1; /**< +0xC0 */
    IconFrame frame2; /**< +0x140 */
} McIconSource;

/** The PS-X memory-card save header, MEMCARD_SAVE_HEADER_SIZE bytes: the
 * title sector ('S', 'C', the icon display flag, the file's size in blocks,
 * the title field, the CLUT), then up to three icon frames. */
typedef struct McSaveHeader {
    u8 magic0;          /**< 'S' */
    u8 magic1;          /**< 'C' */
    u8 iconDisplayFlag; /**< MEMCARD_ICON_FLAG_BASE plus the icon frame count */
    u8 blockCount;      /**< the save data's size in blocks */
    char title[92];     /**< +0x04..+0x5F: the format's 64-byte title and its reserved bytes */
    IconPaletteHalf palette[2]; /**< the icon's CLUT */
    IconFrame frame0;           /**< icon frame 1 */
    IconFrame frame1;           /**< icon frame 2 */
    IconFrame frame2;           /**< icon frame 3 */
} McSaveHeader;

/* The card format's header and the icon TIM it is built from. */
COMPILE_ASSERT(sizeof(McSaveHeader) == MEMCARD_SAVE_HEADER_SIZE, McSaveHeader_size);
COMPILE_ASSERT(offsetof(McSaveHeader, palette) == 0x60, McSaveHeader_palette);
COMPILE_ASSERT(offsetof(McSaveHeader, frame0) == 0x80, McSaveHeader_frame0);
COMPILE_ASSERT(offsetof(McIconSource, palette) == 0x14, McIconSource_palette);
COMPILE_ASSERT(offsetof(McIconSource, frame0) == 0x40, McIconSource_frame0);

/**
 * @brief Writes a save file's letter into a full-width save title, or blanks it.
 *
 * With a file name, the title's letter field becomes a space, the letter
 * for the file name's -NN (a for -01 .. o for -15) and a space, followed by
 * "Day", and a space goes after the day number. With none it only blanks
 * the letter field. TaskObjF__WriteMemcardSaveFile calls it before its
 * retry loop, and with no name when the loop gives up. Defined in
 * src/ui/task_objf.c.
 * @param titleText the full-width save title, "LSD   Day001" in layout.
 * @param fileName  the save file's name (namePrefix + "-NN"), or NULL.
 * @return a pointer into sSaveTitleGlyphs, which no caller reads.
 */
extern void *StampSaveTitleFileLetter(char *titleText, char *fileName);

/* The class's own methods, in address order. */

/**
 * @brief Allocates a TaskObjF and runs its ctor through the method table.
 * @param padEnable passed to InitCARD on the first construction.
 * @param cardSlot  the memory-card slot, 0 or 1.
 * @return the new controller, or NULL when the allocation fails.
 */
TaskObjF *New_TaskObjF(s32 padEnable, s32 cardSlot);

/**
 * @brief Constructs a TaskObjF, starting the card libraries on the first one made.
 *
 * On the first construction runs InitCARD(padEnable), StartCARD and
 * _bu_init; then clears the links and calls setCardSlot.
 * @param self      the object to construct.
 * @param padEnable InitCARD's argument.
 * @param cardSlot  the memory-card slot, 0 or 1.
 */
void TaskObjF__TaskObjF(TaskObjF *self, s32 padEnable, s32 cardSlot);

/**
 * @brief Clears the input, tick, sprite-parent, text-entry and item-list links.
 * @param self the controller.
 */
void TaskObjF__ClearLinks(TaskObjF *self);

/**
 * @brief Runs BasicClass's finalize.
 * @param self the controller.
 */
void TaskObjF__Finalize(TaskObjF *self);

/**
 * @brief Adds a child and files it by class: Pad, FrameClock, TextEntry or ItemList.
 * @param self  the controller.
 * @param child the child, or NULL for none.
 */
void TaskObjF__AddChild(TaskObjF *self, BasicClass *child);

/**
 * @brief Removes a child, clearing the link its class is filed under.
 * @param self  the controller.
 * @param child the child, or NULL for none.
 */
void TaskObjF__RemoveChild(TaskObjF *self, BasicClass *child);

/**
 * @brief Clears every link, then removes every child.
 * @param self the controller.
 */
void TaskObjF__RemoveAllChildren(TaskObjF *self);

/**
 * @brief Selects the memory-card slot and its card channel (slot << 4).
 * @param self     the controller.
 * @param cardSlot the slot, 0 or 1.
 */
void TaskObjF__SetCardSlot(TaskObjF *self, s32 cardSlot);

/**
 * @brief Opens and enables the four SwCARD events, one per sCardEventSpecs entry.
 * @param self the controller.
 * @return 1.
 */
s32 TaskObjF__OpenEvents(TaskObjF *self);

/**
 * @brief Disables and closes the four card events.
 * @param self the controller.
 * @return 1.
 */
s32 TaskObjF__CloseEvents(TaskObjF *self);

/**
 * @brief Asks the card whether it is there, new and formatted, retrying while it is not usable.
 *
 * Retries CardInfoAndLoadStatus up to MEMCARD_RETRIES more times while it
 * fails, answers with an error, or finds the card unformatted.
 * @param self        the controller.
 * @param error       set nonzero when the card answered with an error.
 * @param cardChanged set nonzero when the first or the last try found a new card.
 * @param formatted   set nonzero when the card is formatted.
 * @return nonzero when a card answered.
 */
s32 TaskObjF__CheckCardStatus(TaskObjF *self, s32 *error, s32 *cardChanged, s32 *formatted);

/**
 * @brief One card check: CardInfoStatus, then CardLoadStatus when a card answered.
 * @param self        the controller.
 * @param error       set nonzero on an error answer.
 * @param cardChanged set nonzero when the card is new.
 * @param formatted   set nonzero when the card is formatted.
 * @return CardLoadStatus's result, or 0 when _card_info got no usable answer.
 */
s32 TaskObjF__CardInfoAndLoadStatus(TaskObjF *self, s32 *error, s32 *cardChanged, s32 *formatted);

/**
 * @brief Runs _card_info and waits for the card's answer.
 *
 * A new card is acknowledged with _card_clear.
 * @param self        the controller.
 * @param error       set to 1 on EvSpERROR, else 0.
 * @param cardChanged set to 1 on EvSpNEW, else 0.
 * @return 0 on a timeout or an error, else 1.
 */
s32 TaskObjF__CardInfoStatus(TaskObjF *self, s32 *error, s32 *cardChanged);

/**
 * @brief Runs _card_load and waits for the card's answer.
 * @param self      the controller.
 * @param error     set to 1 on EvSpERROR, else 0.
 * @param formatted set to 0 on EvSpNEW (unformatted), else 1.
 * @return 0 on a timeout or an error, else 1.
 */
s32 TaskObjF__CardLoadStatus(TaskObjF *self, s32 *error, s32 *formatted);

/**
 * @brief Formats the card in the selected slot, retrying up to MEMCARD_RETRIES more times.
 * @param self the controller.
 * @return format()'s result: nonzero on success.
 */
s32 TaskObjF__FormatCard(TaskObjF *self);

/**
 * @brief Tests whether a save file exists, and copies out its title.
 * @param self      the controller.
 * @param destTitle receives the file's title, or NULL for none.
 * @param suffix    the file's name after the device; empty or NULL names no file.
 * @return nonzero when the file opens.
 */
s32 TaskObjF__ProbeMemcardFile(TaskObjF *self, char *destTitle, char *suffix);

/**
 * @brief Opens a save file and copies out the title from its header, once.
 * @param self      the controller.
 * @param destTitle receives the file's title, or NULL for none.
 * @param suffix    the file's name after the device.
 * @return 1 when the file opened, else 0.
 */
s32 TaskObjF__OpenAndReadMemcardFile(TaskObjF *self, char *destTitle, char *suffix);

/**
 * @brief Finds the first `prefix` + suffix file name that does not exist on the card.
 * @param self     the controller.
 * @param buf      receives the name.
 * @param prefix   the name's prefix (the product code).
 * @param suffixes a NULL-terminated table of candidate suffixes.
 * @return `buf`, or NULL when every name exists.
 */
char *TaskObjF__FindUnusedMemcardName(TaskObjF *self, char *buf, char *prefix, char **suffixes);

/**
 * @brief Lists the `prefix` + suffix files that exist, with their titles.
 * @param self        the controller.
 * @param destTitles  title buffers, filled in order for each file found.
 * @param outSuffixes receives each found file's suffix, in the same order.
 * @param prefix      the names' prefix (the product code).
 * @param suffixes    a NULL-terminated table of candidate suffixes.
 * @return the number of files found.
 */
s32 TaskObjF__CollectExistingMemcardFiles(TaskObjF *self, char **destTitles, char **outSuffixes,
                                          char *prefix, char **suffixes);

/**
 * @brief Tests for room for a save, retrying up to MEMCARD_RETRIES more times.
 * @param self       the controller.
 * @param iconFrames the icon frame count (unused by the test).
 * @param size       the save data's size in bytes.
 * @return nonzero when the card has room.
 */
s32 TaskObjF__CheckCardSpace(TaskObjF *self, u8 iconFrames, s32 size);

/**
 * @brief Creates, then deletes, a TEMP file big enough for the save header and `size` bytes.
 * @param self       the controller.
 * @param iconFrames the icon frame count (unused).
 * @param size       the save data's size in bytes.
 * @return 1 when the file could be created, else 0.
 */
s32 TaskObjF__ProbeCardFreeSpace(TaskObjF *self, u8 iconFrames, s32 size);

/**
 * @brief Reads a save file's data, retrying up to MEMCARD_RETRIES more times.
 * @param self    the controller.
 * @param suffix  the file's name after the device.
 * @param outBuf  receives the data.
 * @param outSize the bytes to read.
 * @return nonzero on success.
 */
s32 TaskObjF__ReadMemcardFile(TaskObjF *self, char *suffix, void *outBuf, s32 outSize);

/**
 * @brief Reads a save file's data once, from after its header's title sector and icon frames.
 * @param self    the controller.
 * @param suffix  the file's name after the device.
 * @param outBuf  receives the data.
 * @param outSize the bytes to read.
 * @return 1 when the file opened, else 0.
 */
s32 TaskObjF__TryReadMemcardFile(TaskObjF *self, char *suffix, void *outBuf, s32 outSize);

/**
 * @brief Writes a save file, retrying up to MEMCARD_RETRIES more times.
 *
 * Stamps the file's letter into `title` first, and blanks it again when
 * every try fails.
 * @param self       the controller.
 * @param fileName   the file's name after the device.
 * @param title      the full-width save title.
 * @param iconFrames the icon frame count.
 * @param icon       the icon TIM (its CLUT and first frames are written).
 * @param data       the save data.
 * @param size       the save data's size in bytes.
 * @return nonzero on success.
 */
s32 TaskObjF__WriteMemcardSaveFile(TaskObjF *self, char *fileName, char *title, char iconFrames,
                                   struct TimImage *icon, void *data, s32 size);

/**
 * @brief Writes a save file once: header with title and icon, then the data.
 *
 * Deletes the file, creates it at its full size in blocks, then reopens it
 * to write the save header and the data, each rounded up to whole sectors.
 * @param self       the controller.
 * @param fileName   the file's name after the device.
 * @param title      the full-width save title.
 * @param iconFrames the icon frame count.
 * @param icon       the icon TIM.
 * @param data       the save data.
 * @param size       the save data's size in bytes.
 * @return 1 on success, 0 when the file cannot be created or opened.
 */
s32 TaskObjF__TryWriteMemcardSaveFile(TaskObjF *self, char *fileName, char *title, u8 iconFrames,
                                      struct TimImage *icon, void *data, s32 size);

/**
 * @brief Enables the four card events, in a critical section.
 * @param self the controller.
 * @return EnableEvent's last result.
 */
s32 TaskObjF__EnableEvents(TaskObjF *self);

/**
 * @brief Disables the four card events, in a critical section.
 * @param self the controller.
 * @return DisableEvent's last result.
 */
s32 TaskObjF__DisableEvents(TaskObjF *self);

/**
 * @brief Tests the four card events.
 * @param self the controller.
 * @return TestEvent's last result.
 */
s32 TaskObjF__TestEvents(TaskObjF *self);

/**
 * @brief Calls `callback` on each card event until one returns 0.
 * @param self     the controller.
 * @param callback the kernel event call to apply.
 * @param critical nonzero to run the loop in a critical section.
 * @return the last callback's result.
 */
s32 TaskObjF__ForEachEvent(TaskObjF *self, long (*callback)(long), s32 critical);

/**
 * @brief Spins until a card event tests ready.
 * @param self the controller.
 * @return that event's sCardEventSpecs entry: which answer the card gave.
 */
s32 TaskObjF__WaitForReadyEvent(TaskObjF *self);

/**
 * @brief Prepares for an operation: file names, input and tick children, sprite parent and sound.
 *
 * Also forgets the load buffers and the card icon (both NULL) and clears
 * the state and the operation.
 * @param self         the controller.
 * @param namePrefix   the save file names' prefix.
 * @param nameSuffixes a NULL-terminated table of save file suffixes.
 * @param inputSource  the Pad, added as a child.
 * @param tickSource   the FrameClock, added as a child.
 * @param spriteParent the node the card icon and the widgets attach under.
 * @param sound        the VabStreamObj tones are played on.
 */
void TaskObjF__Init(TaskObjF *self, char *namePrefix, char **nameSuffixes, BasicClass *inputSource,
                    BasicClass *tickSource, struct SceneNode *spriteParent, struct VabStreamObj *sound);

/**
 * @brief Forgets the sound and sprite parent and removes the input and tick children.
 * @param self the controller.
 */
void TaskObjF__Deinit(TaskObjF *self);

/**
 * @brief Starts, or continues, a load.
 *
 * Stores the request and checks the card (Validate). Then lists the files
 * that exist: none gives LOAD_NOT_FOUND; otherwise CHOOSE_FILE, or LOADING
 * when re-entered from LOAD_WARNING after the player chose one.
 * @param self     the controller.
 * @param fileName receives the chosen file's name.
 * @param title    receives the chosen file's title.
 * @param data     the buffer the save data is read into.
 * @param size     its size in bytes.
 */
void TaskObjF__BeginLoad(TaskObjF *self, char *fileName, char *title, void *data, s32 size);

/**
 * @brief Allocates the title and suffix pointer arrays and the title buffers, when not allocated.
 * @param self the controller.
 */
void TaskObjF__AllocBuffers(TaskObjF *self);

/**
 * @brief Frees the title buffers past the `bufCount` found and NULL-terminates the list.
 * @param self the controller.
 */
void TaskObjF__FreeUnusedBuffers(TaskObjF *self);

/**
 * @brief Frees the suffix array, the `bufCount` title buffers and the title array, when allocated.
 * @param self the controller.
 */
void TaskObjF__FreeBuffers(TaskObjF *self);

/**
 * @brief Starts, or continues, a save.
 *
 * Stores the request and checks the card (Validate). An existing file asks
 * to overwrite (SAVE_OVERWRITE_WARNING); confirming re-enters to edit the
 * title (EDIT_TITLE), and the edited title re-enters to save (SAVING). A
 * new file needs room (else SAVE_NO_SPACE), then goes to EDIT_TITLE and
 * SAVING the same way.
 * @param self         the controller.
 * @param fileName     the file's name; empty to pick an unused one.
 * @param title        the full-width save title.
 * @param titleEditPos the first full-width character the player edits.
 * @param iconFrames   the icon frame count.
 * @param icon         the icon TIM.
 * @param data         the save data.
 * @param size         its size in bytes.
 */
void TaskObjF__BeginSave(TaskObjF *self, char *fileName, char *title, s32 titleEditPos,
                         u8 iconFrames, struct TimImage *icon, void *data, s32 size);

/**
 * @brief Checks the card around its own open and close of the events.
 * @param self the controller.
 * @return 1 for a formatted, unchanged card; otherwise 0, after setting the
 *         state that says why (NO_CARD, CARD_ERROR, CARD_CHANGED or an
 *         UNFORMATTED state).
 */
s32 TaskObjF__Validate(TaskObjF *self);

/**
 * @brief Routes a child's notification by the child's class.
 *
 * Runs BasicClass's onNotify, then: a Pad's to onInputEvent, a FrameClock's
 * to tickStateDelay, a TextEntry's to onTextEntryResult, an ItemList's to
 * onItemListResult.
 * @param self   the controller.
 * @param sender the child that notified.
 * @param event  the notification.
 */
void TaskObjF__OnNotify(TaskObjF *self, void *sender, s32 event);

/**
 * @brief Enters a state: notifies the parent, shows its message and runs its entry action.
 *
 * Setting the current state again aborts. Swaps the message icon, zeroes
 * `waitCounter`, then FORMAT, WRITE and READ run their card operation and
 * set the result state, EDIT_TITLE attaches the text entry and CHOOSE_FILE
 * the item list. DONE and ABORTED free a load's buffers and return to idle
 * with no operation.
 * @param self  the controller.
 * @param state a TaskObjFState.
 */
void TaskObjF__SetState(TaskObjF *self, s32 state);

/**
 * @brief Shows the message sprite CARD\<sCardIconNames[index]>.TIM under the sprite parent.
 *
 * Does nothing for an index past the table, without a sprite parent, or
 * while an icon is shown.
 * @param self  the controller.
 * @param index the state whose message to show.
 */
void TaskObjF__LoadCardIcon(TaskObjF *self, s32 index);

/**
 * @brief Releases the message sprite, when one is shown.
 * @param self the controller.
 */
void TaskObjF__ReleaseCardIcon(TaskObjF *self);

/**
 * @brief Acts on a pad event while an operation runs: circle advances, cross aborts.
 * @param self   the controller.
 * @param sender the Pad (unused).
 * @param event  the pad event.
 */
void TaskObjF__OnInputEvent(TaskObjF *self, void *sender, s32 event);

/**
 * @brief Plays a tone on the sound object at volume 127, 127, when there is one.
 * @param self  the controller.
 * @param index the tone: VAB program * 16 + tone number.
 */
void TaskObjF__PlaySound(TaskObjF *self, s32 index);

/**
 * @brief Confirms the message shown.
 *
 * NO_CARD, CARD_CHANGED and the two warnings re-run the operation (after
 * LOAD_WARNING, with the chosen file's name and title); UNFORMATTED_SAVE
 * starts formatting; the errors, SAVE_NO_SPACE, UNFORMATTED_LOAD and
 * LOAD_NOT_FOUND abort.
 * Each plays a tone.
 * @param self the controller.
 */
void TaskObjF__AdvanceState(TaskObjF *self);

/**
 * @brief Cancels from CARD_CHANGED, UNFORMATTED_SAVE or a warning, with a tone.
 * @param self the controller.
 */
void TaskObjF__AbortFromState(TaskObjF *self);

/**
 * @brief Counts frame ticks in FORMATTING, SAVING or LOADING, then runs the operation.
 *
 * The seventh tick sets FORMAT, WRITE or READ.
 * @param self the controller.
 */
void TaskObjF__TickStateDelay(TaskObjF *self);

/**
 * @brief Shows the save-title editor and hands it input.
 *
 * Makes the TextEntry over `title` from `titleEditPos` on first use, adds
 * it as a child, loads its sprites and attaches the input, tick and sound.
 * Needs a sprite parent and an input source.
 * @param self the controller.
 */
void TaskObjF__AttachTextEntry(TaskObjF *self);

/**
 * @brief Takes input back from the save-title editor and hides it, releasing it if this made it.
 * @param self the controller.
 */
void TaskObjF__DetachTextEntry(TaskObjF *self);

/**
 * @brief Acts on the save-title editor's result: accepted saves, cancelled aborts.
 * @param self   the controller.
 * @param sender the TextEntry (unused).
 * @param result a TextEntryResult.
 */
void TaskObjF__OnTextEntryResult(TaskObjF *self, void *sender, s32 result);

/**
 * @brief Shows the file chooser over the found titles and hands it input.
 *
 * Makes the ItemList on first use, adds it as a child, loads its sprites
 * and attaches the input, tick and sound. Needs a sprite parent and an
 * input source.
 * @param self the controller.
 */
void TaskObjF__AttachItemList(TaskObjF *self);

/**
 * @brief Takes input back from the file chooser and hides it, releasing it if this made it.
 * @param self the controller.
 */
void TaskObjF__DetachItemList(TaskObjF *self);

/**
 * @brief Acts on the file chooser's result: a choice goes to LOAD_WARNING, a cancel aborts.
 * @param self   the controller.
 * @param list   the ItemList; its cursor index becomes `selectedIndex`.
 * @param result ITEMLIST_RESULT_CHOSEN or ITEMLIST_RESULT_CANCELLED.
 */
void TaskObjF__OnItemListResult(TaskObjF *self, struct ItemList *list, s32 result);

#endif
