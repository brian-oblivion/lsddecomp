/*
 * TaskObjF's methods (include/task_objf.h: the memory-card controller
 * TitleMenu's SAVE and LOAD drive), in ROM order: New_TaskObjF (allocate,
 * then the ctor through the class's table) through GetTaskObjFMethods,
 * with BuildMemcardPath and WaitForReadyEvent among them, then
 * StampSaveTitleFileLetter, which writes a save file's letter into the
 * save title. Its method table and the card's messages close the file; a
 * (void *) entry in the table is a function whose declared type differs
 * from its slot's, a method inherited from a parent class and declared on
 * the parent's type, or an empty method declared (void). TitleMenu is in
 * title_menu.c.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <strings.h>
#include "title_menu.h"
#include "vab_stream_obj.h"
#include "tim_image.h"
#include "task_objf.h"
#include <kernel.h>
#ifdef HOST_BUILD
/* psyz declares the kernel's file, event and card calls in libapi.h. */
#include <libapi.h>
#else
#include <sys/file.h>
#endif
#include "pad.h"
#include "frame_clock.h"
#include "text_entry.h"
#include "item_list.h"
#include "bmem_pmgr.h"
#include <stdio.h>
#include <convert.h>

/* Per TaskObjF::events slot: the event spec TaskObjF__OpenEvents passes to
 * OpenEvent, and the value WaitForReadyEvent returns for that slot. */
extern s32 sCardEventSpecs[4];

/* TaskObjF's read-only data, in the image's order. */

/* The three message icons' names (CARD\<name>.TIM) that are read-only data;
 * sCardIconNames, at the end of the file, lists them with the rest. */
const char sLoadNotFoundIconName[] = "NOTFOUND";
const char sSaveNoSpaceIconName[] = "SAVEEMPT";
const char sNoCardIconName[] = "NOCONECT";

/* TryWriteMemcardSaveFile's message when it cannot create the file. */
const char sFileNotCreatedMsg[] = "File not create in WriteFile\n";

/* The full-width letters a..o, then three full-width spaces and "Day":
 * StampSaveTitleFileLetter's glyphs. FW is one character, its Shift-JIS lead
 * and trail bytes. */
#define FW(lead, trail) {(s8)(lead), (s8)(trail)}
const FullWidthChar sSaveTitleGlyphTable[22] = {
    FW(0x82, 0x81), FW(0x82, 0x82), FW(0x82, 0x83), FW(0x82, 0x84), FW(0x82, 0x85), FW(0x82, 0x86),
    FW(0x82, 0x87), FW(0x82, 0x88), FW(0x82, 0x89), FW(0x82, 0x8A), FW(0x82, 0x8B), FW(0x82, 0x8C),
    FW(0x82, 0x8D), FW(0x82, 0x8E), FW(0x82, 0x8F), FW(0x81, 0x40), FW(0x81, 0x40), FW(0x81, 0x40),
    FW(0x82, 0x63), FW(0x82, 0x81), FW(0x82, 0x99), FW(0x00, 0x00)};

/* TaskObjF's small data, in address order. */

/* TaskObjF__TaskObjF's construction count: InitCARD/StartCARD/_bu_init run
 * only on the first construction, when it was 0 before the increment. */
static s32 sTaskObjFCount SDATA = 0;

/* The message icons' names, CARD\<name>.TIM, that are small data; the
 * other three ("NOCONECT", "SAVEEMPT", "NOTFOUND") are read-only data.
 * sCardIconNames lists them all by state. */
static char sLoadErrorIconName[] SDATA = "LOADERR";
static char sLoadingIconName[] SDATA = "LOADING";
static char sLoadWarningIconName[] SDATA = "LOADWAR";
static char sSaveErrorIconName[] SDATA = "SAVEERR";
static char sSavingIconName[] SDATA = "SAVING";
static char sSaveOverwriteIconName[] SDATA = "SAVEWAR";
static char sFormatErrorIconName[] SDATA = "FORMERR";
static char sFormattingIconName[] SDATA = "FORMING";
static char sUnformattedSaveIconName[] SDATA = "UNFORM2";
static char sUnformattedLoadIconName[] SDATA = "UNFORM1";
static char sCardChangedIconName[] SDATA = "CHANGE";
static char sCardErrorIconName[] SDATA = "ERROR";

/* Where TaskObjF__LoadCardIcon attaches the message icon, percent of half
 * the screen from the centre. */
static ScreenSpritePos sCardIconPos SDATA = {-70, -60};

/* The two card slots' device names. */
static McDevicePath sMcDevicePath1 SDATA = {'b', 'u', '1', '0', ':', '\0'};
static McDevicePath sMcDevicePath0 SDATA = {'b', 'u', '0', '0', ':', '\0'};

/* "TEMP": the name TaskObjF__ProbeCardFreeSpace creates to test for space. */
static char sMcTempFileSuffix[] SDATA = "TEMP";

/* TaskObjF__LoadCardIcon's path parts: CARD\<name>.TIM. */
static char sTitleCardPathPrefix[] SDATA = "CARD\\";
static char sCardPathSuffix[] SDATA = ".TIM";

/* StampSaveTitleFileLetter's glyphs (see SAVE_TITLE_GLYPH_SPACES below). */
static FullWidthChar *sSaveTitleGlyphs SDATA = (FullWidthChar *)sSaveTitleGlyphTable;

TaskObjF *New_TaskObjF(s32 padEnable, s32 cardSlot) {
    TaskObjF *self;

    self = BMemPMgrAlloc(sizeof(TaskObjF));
    if (self != NULL) {
        GetTaskObjFMethods()->ctor(self, padEnable, cardSlot);
        return self;
    }
    return NULL;
}

/* InitCARD, StartCARD and _bu_init are Sony's (libcard): include/psyq/kernel.h. */

/* The card libraries are started once, by the first TaskObjF made. */
void TaskObjF__TaskObjF(TaskObjF *self, s32 padEnable, s32 cardSlot) {
    s32 count;

    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetTaskObjFMethods();
    count = sTaskObjFCount;
    sTaskObjFCount = count + 1;
    if (count == 0) {
        InitCARD(padEnable);
        StartCARD();
        _bu_init();
    }
    TaskObjF__ClearLinks(self);
    self->methods->setCardSlot(self, cardSlot);
}

/*
 * TaskObjF's child links, card events, card checks and file probes
 * (include/task_objf.h: slots +0x00C..+0x018 and +0x040..+0x060, and the
 * helpers they call), in address order: the children filed by class; the
 * card slot and its four SwCARD events; the card checks (_card_info for a
 * new card, _card_load for a formatted one) and format(); and the file
 * probes over "bu00:"/"bu10:" plus a prefix and a suffix
 * (BuildMemcardPath), ending with the free-space test, which creates and
 * deletes a TEMP file of the save's size.
 *
 * A retried call is tried once, then up to MEMCARD_RETRIES more times while
 * it fails (TaskObjF__ProbeMemcardFile's loop: no more times).
 */

/* Defined below, after the file I/O: writes the device name into `dest`,
 * appends `suffix`, and returns `dest`. */
char *BuildMemcardPath(McDevicePath *dest, s32 cardSlot, char *suffix);

void TaskObjF__ClearLinks(TaskObjF *self) {
    self->inputSource = NULL;
    self->tickSource = NULL;
    self->spriteParent = NULL;
    self->textEntry = NULL;
    self->itemList = NULL;
}

void TaskObjF__Finalize(TaskObjF *self) {
    GetBasicClassMethods()->finalize((BasicClass *)self);
}

void TaskObjF__AddChild(TaskObjF *self, BasicClass *child) {
    s32 classId;

    if (child == NULL) {
        return;
    }
    GetBasicClassMethods()->addChild((BasicClass *)self, child);
    classId = child->methods->header;
    if ((classId & CLASS_ID_ROOT_MASK) == PAD_CLASS_ID) {
        self->inputSource = child;
        return;
    }
    if ((classId & CLASS_ID_ROOT_MASK) == FRAMECLOCK_CLASS_ID) {
        self->tickSource = child;
        return;
    }
    if ((classId & CLASS_ID_LEVEL2_MASK) == TEXTENTRY_CLASS_ID) {
        self->textEntry = (struct TextEntry *)child;
        return;
    }
    if ((classId & CLASS_ID_LEVEL2_MASK) == ITEMLIST_CLASS_ID) {
        self->itemList = (struct ItemList *)child;
    }
}

void TaskObjF__RemoveChild(TaskObjF *self, BasicClass *child) {
    s32 classId;

    if (child == NULL) {
        return;
    }
    classId = child->methods->header;
    if ((classId & CLASS_ID_ROOT_MASK) == PAD_CLASS_ID) {
        self->inputSource = NULL;
    } else if ((classId & CLASS_ID_ROOT_MASK) == FRAMECLOCK_CLASS_ID) {
        self->tickSource = NULL;
    } else if ((classId & CLASS_ID_LEVEL2_MASK) == TEXTENTRY_CLASS_ID) {
        self->textEntry = NULL;
    } else if ((classId & CLASS_ID_LEVEL2_MASK) == ITEMLIST_CLASS_ID) {
        self->itemList = NULL;
    }
    GetBasicClassMethods()->removeChild((BasicClass *)self, child);
}

void TaskObjF__RemoveAllChildren(TaskObjF *self) {
    self->inputSource = NULL;
    self->tickSource = NULL;
    self->spriteParent = NULL;
    self->textEntry = NULL;
    self->itemList = NULL;
    GetBasicClassMethods()->removeAllChildren((BasicClass *)self);
}

void TaskObjF__SetCardSlot(TaskObjF *self, s32 cardSlot) {
    self->cardSlot = cardSlot;
    self->cardHandle = cardSlot << 4;
}

s32 TaskObjF__OpenEvents(TaskObjF *self) {
    s32 i;

    EnterCriticalSection();
    i = 0;
    do {
        self->events[i] = OpenEvent(SwCARD, sCardEventSpecs[i], EvMdNOINTR, NULL);
        i++;
    } while (i < ARRAY_COUNT(self->events));
    ExitCriticalSection();
    TaskObjF__EnableEvents(self);
    return 1;
}

s32 TaskObjF__CloseEvents(TaskObjF *self) {
    TaskObjF__DisableEvents(self);
    /* kernel.h spells CloseEvent with `long`, ForEachEvent's callback with s32. */
    TaskObjF__ForEachEvent(self, (s32 (*)(s32))CloseEvent, 1);
    return 1;
}

/* Nonzero when a card answered. *error is set when it answered with an error,
 * *cardChanged when the first or the last try found a new card, *formatted
 * when it is formatted. An error or an unformatted card also counts as a
 * failed try. */
s32 TaskObjF__CheckCardStatus(TaskObjF *self, s32 *error, s32 *cardChanged, s32 *formatted) {
    s32 retries;
    s32 firstChanged;
    s32 result;

    retries = MEMCARD_RETRIES;
    *cardChanged = 0;
    result = TaskObjF__CardInfoAndLoadStatus(self, error, &firstChanged, formatted);
    /* MATCHING: the retry count is tested after the call; testing it first merges the two calls. */
    while (result == 0 || *error != 0 || *formatted == 0) {
        result = TaskObjF__CardInfoAndLoadStatus(self, error, cardChanged, formatted);
        if (retries-- == 0) {
            break;
        }
    }
    *cardChanged = *cardChanged | firstChanged;
    return result;
}

/* CardInfoStatus's 0 when no card answered, else CardLoadStatus's status. */
s32 TaskObjF__CardInfoAndLoadStatus(TaskObjF *self, s32 *error, s32 *cardChanged, s32 *formatted) {
    s32 status;

    status = TaskObjF__CardInfoStatus(self, error, cardChanged);
    if (status != 0) {
        status = TaskObjF__CardLoadStatus(self, error, formatted);
    }
    return status;
}

s32 TaskObjF__CardInfoStatus(TaskObjF *self, s32 *error, s32 *cardChanged) {
    s32 status;
    s32 answer;

    status = 1;
    /* MATCHING: one expression keeps both stores ahead of TestEvents. */
    *cardChanged = *error = 0;
    TaskObjF__TestEvents(self);
    while (_card_info(self->cardHandle) == 0)
        ;
    answer = TaskObjF__WaitForReadyEvent(self);
    if (answer == EvSpTIMOUT) {
        status = 0;
    } else if (answer == EvSpERROR) {
        status = 0;
        *error = 1;
    } else if (answer == EvSpNEW) {
        *cardChanged = 1;
        _card_clear(self->cardHandle);
    }
    return status;
}

s32 TaskObjF__CardLoadStatus(TaskObjF *self, s32 *error, s32 *formatted) {
    s32 status;
    s32 answer;

    status = 1;
    /* MATCHING: one expression keeps both stores ahead of TestEvents. */
    *formatted = (*error = 0, status);
    TaskObjF__TestEvents(self);
    while (_card_load(self->cardHandle) == 0)
        ;
    answer = TaskObjF__WaitForReadyEvent(self);
    if (answer == EvSpTIMOUT) {
        status = 0;
    } else if (answer == EvSpERROR) {
        status = 0;
        *error = 1;
    } else if (answer == EvSpNEW) {
        *formatted = 0;
    }
    return status;
}

s32 TaskObjF__FormatCard(TaskObjF *self) {
    s32 retries;
    s32 result;
    McDevicePath *path;

    retries = MEMCARD_RETRIES;
    do {
        path = self->cardSlot != 0 ? &sMcDevicePath1 : &sMcDevicePath0;
        result = format((char *)path); /* the device name, "bu00:" or "bu10:" */
    } while (result == 0 && retries-- != 0);
    return result;
}

/* Nonzero when the file prefix+suffix exists; copies its title to destTitle
 * unless that is NULL. An empty suffix names no file. */
s32 TaskObjF__ProbeMemcardFile(TaskObjF *self, char *destTitle, char *suffix) {
    s32 retries;
    s32 result;

    retries = 0; /* the class's retry loop, run once */
    if (suffix == NULL || *suffix == '\0') {
        return 0;
    }
    do {
        result = TaskObjF__OpenAndReadMemcardFile(self, destTitle, suffix);
    } while (result == 0 && retries-- != 0);
    return result;
}

s32 TaskObjF__OpenAndReadMemcardFile(TaskObjF *self, char *destTitle, char *suffix) {
    char pathBuf[MEMCARD_PATH_SIZE];
    char *path;
    s32 handle;
    McSaveHeader *header;

    path = BuildMemcardPath((McDevicePath *)pathBuf, self->cardSlot, suffix);
    handle = open(path, FREAD);
    if (handle == -1) {
        return 0;
    }
    if (destTitle != NULL) {
        header = BMemPMgrAlloc(MEMCARD_SECTOR_SIZE);
        read(handle, header, MEMCARD_SECTOR_SIZE);
        strcpy(destTitle, header->title);
        BMemPMgrFree(header);
    }
    close(handle);
    return 1;
}

char *TaskObjF__FindUnusedMemcardName(TaskObjF *self, char *buf, char *prefix, char **suffixes) {
    while (*suffixes != NULL) {
        strcpy(buf, prefix);
        strcat(buf, *suffixes);
        if (self->methods->probeMemcardFile(self, NULL, buf) == 0) {
            return buf;
        }
        suffixes++;
    }
    return NULL;
}

s32 TaskObjF__CollectExistingMemcardFiles(TaskObjF *self, char **destTitles, char **outSuffixes,
                                          char *prefix, char **suffixes) {
    s32 count;
    char name[32];

    count = 0;
    while (*suffixes != NULL) {
        strcpy(name, prefix);
        strcat(name, *suffixes);
        if (self->methods->probeMemcardFile(self, *destTitles, name) != 0) {
            count++;
            *outSuffixes = *suffixes;
            destTitles++;
            outSuffixes++;
        }
        suffixes++;
    }
    return count;
}

s32 TaskObjF__CheckCardSpace(TaskObjF *self, u8 iconFrames, s32 size) {
    s32 retries;
    s32 result;

    retries = MEMCARD_RETRIES;
    do {
        result = TaskObjF__ProbeCardFreeSpace(self, iconFrames, size);
    } while (result == 0 && retries-- != 0);
    return result;
}

/* Creates, then deletes, a file big enough for `size` bytes of save data
 * after the save header: nonzero when the card has room. */
s32 TaskObjF__ProbeCardFreeSpace(TaskObjF *self, u8 iconFrames, s32 size) {
    char pathBuf[MEMCARD_PATH_SIZE];
    char *path;
    s32 handle;
    s32 blocks;

    /* MATCHING: the u32 cast makes it an unsigned shift, as retail's is. */
    blocks = (u32)(size + MEMCARD_SAVE_HEADER_SIZE + MEMCARD_BLOCK_SIZE - 1) >> MEMCARD_BLOCK_SHIFT;
    path = BuildMemcardPath((McDevicePath *)pathBuf, self->cardSlot, sMcTempFileSuffix);
    handle = open(path, MEMCARD_OPEN_BLOCKS(blocks) | FCREAT);
    if (handle == -1) {
        return 0;
    }
    close(handle);
    /* MATCHING: delete(path), the same address, changes the code. */
#ifdef HOST_BUILD
    erase(pathBuf); /* psyz has the call under its later Psy-Q name */
#else
    delete (pathBuf);
#endif
    return 1;
}

/*
 * TaskObjF's file I/O, card events, load buffers and its two operations
 * (include/task_objf.h: slots +0x064..+0x078 and the +0x038 onNotify
 * override), in address order:
 *
 * - Memory-card files: a read and a write, each retrying a single try up
 *   to MEMCARD_RETRIES more times. The write builds the PS-X save header
 *   (McSaveHeader) from the title and the icon TIM; the read seeks past it.
 * - Card events: enable, disable and test all four through
 *   TaskObjF__ForEachEvent; WaitForReadyEvent spins until one tests ready.
 * - Init and Deinit; BeginLoad and BeginSave, which store the request,
 *   check the card (TaskObjF__Validate) and choose the next TaskObjFState;
 *   and the load's title buffers.
 * - TaskObjF__OnNotify, which routes a child's notification by the child's
 *   class id, the way TaskObjF__AddChild files the children.
 */

/* Defined below, called earlier in address order. */
s32 WaitForReadyEvent(s32 *events, s32 count);

/* The PS-X BIOS file calls (open, read, lseek, close, write, delete) and the
 * kernel event calls are Sony's libapi: include/psyq/kernel.h. */

s32 TaskObjF__ReadMemcardFile(TaskObjF *self, char *suffix, void *outBuf, s32 outSize) {
    s32 retries;
    s32 result;

    retries = MEMCARD_RETRIES;
    do {
        result = TaskObjF__TryReadMemcardFile(self, suffix, outBuf, outSize);
        if (result != 0) {
            break;
        }
    } while (retries-- != 0);
    return result;
}

s32 TaskObjF__TryReadMemcardFile(TaskObjF *self, char *suffix, void *outBuf, s32 outSize) {
    char pathBuf[MEMCARD_PATH_SIZE];
    char *path;
    s32 handle;
    McSaveHeader *header;
    s32 seekPos;
    u8 iconFlag;

    path = BuildMemcardPath((McDevicePath *)pathBuf, self->cardSlot, suffix);
    handle = open(path, FREAD);
    if (handle == -1) {
        return 0;
    }
    /* Only the title sector: its icon display flag says where the data starts. */
    header = BMemPMgrAlloc(MEMCARD_SECTOR_SIZE);
    read(handle, header, MEMCARD_SECTOR_SIZE);
    iconFlag = header->iconDisplayFlag;
    /* The data follows the title sector and the icon frames. */
    /* MATCHING: (iconFlag - 0xF) * MEMCARD_SECTOR_SIZE reorders the arithmetic. */
    seekPos =
        (iconFlag << MEMCARD_SECTOR_SHIFT) - ((MEMCARD_ICON_FLAG_BASE - 1) << MEMCARD_SECTOR_SHIFT);
    BMemPMgrFree(header);
    lseek(handle, seekPos, SEEK_SET);
    read(handle, outBuf, outSize);
    close(handle);
    return 1;
}

s32 TaskObjF__WriteMemcardSaveFile(TaskObjF *self, char *fileName, char *title, char iconFrames,
                                   struct TimImage *icon, void *data, s32 size) {
    s32 retries;
    s32 result;

    retries = MEMCARD_RETRIES;
    StampSaveTitleFileLetter(title, fileName);
    do {
        result = TaskObjF__TryWriteMemcardSaveFile(self, fileName, title, iconFrames & 0xFF, icon,
                                                   data, size);
        if (result != 0) {
            break;
        }
    } while (retries-- != 0);
    if (result == 0) {
        StampSaveTitleFileLetter(title, NULL);
    }
    return result;
}

/* Deletes the file, creates it at its full size in blocks, then reopens it
 * to write the save header and the data, each rounded up to whole sectors. */
/* MATCHING: iconFrames is u8, so the incoming word and its zero-extended copy are both kept. */
s32 TaskObjF__TryWriteMemcardSaveFile(TaskObjF *self, char *fileName, char *title, u8 iconFrames,
                                      struct TimImage *icon, void *data, s32 size) {
    char pathBuf[MEMCARD_PATH_SIZE];
    char *path;
    s32 fileHandle;
    s32 openMode;
    McIconSource *iconSrc;
    McSaveHeader *header;

    path = BuildMemcardPath((McDevicePath *)pathBuf, self->cardSlot, fileName);
#ifdef HOST_BUILD
    erase(path); /* psyz has the call under its later Psy-Q name */
#else
    delete (path);
#endif
    openMode = MEMCARD_OPEN_BLOCKS(((u32)size + (MEMCARD_SAVE_HEADER_SIZE + MEMCARD_BLOCK_SIZE - 1)) >>
                                   MEMCARD_BLOCK_SHIFT) |
               FCREAT;
    fileHandle = open(path, openMode);
    if (fileHandle == -1) {
        printf((char *)sFileNotCreatedMsg);
        return 0;
    }
    close(fileHandle);
    fileHandle = open(path, FWRITE);
    if (fileHandle == -1) {
        return 0;
    }
    iconSrc = (McIconSource *)icon->buffer;
    header = (McSaveHeader *)BMemPMgrAlloc(sizeof(McSaveHeader));
    header->magic0 = 'S';
    header->magic1 = 'C';
    header->iconDisplayFlag = iconFrames + MEMCARD_ICON_FLAG_BASE;
    header->blockCount = ((u32)size + (MEMCARD_BLOCK_SIZE - 1)) >> MEMCARD_BLOCK_SHIFT;
    strcpy(header->title, title);
    /* MATCHING: IconPaletteHalf (halfwords) and IconFrame (bytes) set how each
     * whole-struct copy is done. */
    header->palette[0] = iconSrc->palette[0];
    header->palette[1] = iconSrc->palette[1];
    header->frame0 = iconSrc->frame0;
    header->frame1 = iconSrc->frame1;
    header->frame2 = iconSrc->frame2;
    /* The title sector and the icon frames. */
    /* MATCHING: (iconFrames + 1) * MEMCARD_SECTOR_SIZE reorders the arithmetic. */
    write(fileHandle, header, (iconFrames << MEMCARD_SECTOR_SHIFT) + MEMCARD_SECTOR_SIZE);
    BMemPMgrFree(header);
    write(fileHandle, data,
          (((u32)size + (MEMCARD_SECTOR_SIZE - 1)) >> MEMCARD_SECTOR_SHIFT) << MEMCARD_SECTOR_SHIFT);
    close(fileHandle);
    return 1;
}

char *BuildMemcardPath(McDevicePath *dest, s32 cardSlot, char *suffix) {
    McDevicePath *device;

    if (cardSlot) {
        device = &sMcDevicePath1;
    } else {
        device = &sMcDevicePath0;
    }
    /* MATCHING: McDevicePath's byte members (alignment 1) set how the copy is done. */
    *dest = *device;
    strcat((char *)dest, suffix);
    return (char *)dest;
}

/* Sony's kernel event calls (libapi/a11..a13, include/psyq/kernel.h). Each
 * takes an event descriptor and returns a status word, which is what lets
 * TaskObjF__ForEachEvent take them as its callback; kernel.h spells them with
 * `long`, hence the casts. */

s32 TaskObjF__EnableEvents(TaskObjF *self) {
    return TaskObjF__ForEachEvent(self, (s32 (*)(s32))EnableEvent, 1);
}

s32 TaskObjF__DisableEvents(TaskObjF *self) {
    return TaskObjF__ForEachEvent(self, (s32 (*)(s32))DisableEvent, 1);
}

s32 TaskObjF__TestEvents(TaskObjF *self) {
    return TaskObjF__ForEachEvent(self, (s32 (*)(s32))TestEvent, 0);
}

/* Calls `callback` on each event until one returns 0, inside a critical
 * section when `critical` is set; returns the last result. */
s32 TaskObjF__ForEachEvent(TaskObjF *self, s32 (*callback)(s32), s32 critical) {
    s32 i;
    s32 result;

    if (critical) {
        EnterCriticalSection();
    }
    for (i = 0; i < ARRAY_COUNT(self->events); i++) {
        result = callback(self->events[i]);
        if (result == 0) {
            break;
        }
    }
    if (critical) {
        ExitCriticalSection();
    }
    return result;
}

s32 TaskObjF__WaitForReadyEvent(TaskObjF *self) {
    return WaitForReadyEvent(self->events, ARRAY_COUNT(self->events));
}

/* Spins until one of `count` events tests ready and returns that slot's
 * sCardEventSpecs entry: which answer the card gave. */
s32 WaitForReadyEvent(s32 *events, s32 count) {
    s32 i;

    for (;;) {
        for (i = 0; i < count; i++) {
            if (TestEvent(events[i]) != 0) {
                return sCardEventSpecs[i];
            }
        }
    }
}

void TaskObjF__Init(TaskObjF *self, char *namePrefix, char **nameSuffixes, BasicClass *inputSource,
                    BasicClass *tickSource, struct SceneNode *spriteParent, struct VabStreamObj *sound) {
    self->namePrefix = namePrefix;
    self->nameSuffixes = nameSuffixes;
    self->titles = NULL;
    self->spriteParent = spriteParent;
    self->sound = sound;
    self->methods->addChild(self, inputSource);
    self->methods->addChild(self, tickSource);
    self->cardIcon = NULL;
    self->state = TASKOBJF_STATE_IDLE;
    self->opMode = TASKOBJF_OP_NONE;
}

void TaskObjF__Deinit(TaskObjF *self) {
    self->sound = NULL;
    self->spriteParent = NULL;
    self->methods->removeChild(self, self->inputSource);
    self->methods->removeChild(self, self->tickSource);
}

/* Re-entered from LOAD_WARNING once the player has chosen a file, which it
 * then loads; otherwise it offers the files it found. */
void TaskObjF__BeginLoad(TaskObjF *self, char *fileName, char *title, void *data, s32 size) {
    s32 found;
    s32 state;

    self->fileName = fileName;
    self->title = title;
    self->data = data;
    self->opMode = TASKOBJF_OP_LOAD;
    self->dataSize = size;
    if (TaskObjF__Validate(self)) {
        TaskObjF__FreeBuffers(self);
        TaskObjF__AllocBuffers(self);
        found = self->methods->collectExistingMemcardFiles(self, self->titles, self->foundSuffixes,
                                                           self->namePrefix, self->nameSuffixes);
        self->bufCount = found;
        if (found != 0) {
            TaskObjF__FreeUnusedBuffers(self);
            if (self->state == TASKOBJF_STATE_LOAD_WARNING) {
                state = TASKOBJF_STATE_LOADING;
            } else {
                state = TASKOBJF_STATE_CHOOSE_FILE;
            }
        } else {
            state = TASKOBJF_STATE_LOAD_NOT_FOUND;
            self->bufCount = TASKOBJF_MAX_FILES;
        }
        self->methods->setState(self, state);
    }
}

void TaskObjF__AllocBuffers(TaskObjF *self) {
    s32 i;

    if (self->titles == NULL) {
        self->titles = BMemPMgrAlloc((TASKOBJF_MAX_FILES + 1) * sizeof(char *));
        for (i = 0; i < TASKOBJF_MAX_FILES; i++) {
            self->titles[i] = BMemPMgrAlloc(TASKOBJF_TITLE_SIZE);
        }
        self->foundSuffixes = BMemPMgrAlloc((TASKOBJF_MAX_FILES + 1) * sizeof(char *));
    }
}

void TaskObjF__FreeUnusedBuffers(TaskObjF *self) {
    s32 i;

    for (i = self->bufCount; i < TASKOBJF_MAX_FILES; i++) {
        self->titles[i] = BMemPMgrFree(self->titles[i]);
    }
    self->titles[i] = NULL;
}

void TaskObjF__FreeBuffers(TaskObjF *self) {
    s32 i;

    if (self->titles != NULL) {
        BMemPMgrFree(self->foundSuffixes);
        for (i = 0; i < self->bufCount; i++) {
            BMemPMgrFree(self->titles[i]);
        }
        BMemPMgrFree(self->titles);
        self->titles = NULL;
    }
}

/* An existing file first asks to overwrite; confirming re-enters from
 * SAVE_OVERWRITE_WARNING to edit the title, and the edited title re-enters
 * from EDIT_TITLE to save. */
/* MATCHING: one setState call per branch; they compile to one shared call, as retail's. */
void TaskObjF__BeginSave(TaskObjF *self, char *fileName, char *title, s32 titleEditPos,
                         u8 iconFrames, struct TimImage *icon, void *data, s32 size) {
    s32 state;
    TaskObjFMethods *methods;

    self->fileName = fileName;
    self->title = title;
    self->titleEditPos = titleEditPos;
    self->opMode = TASKOBJF_OP_SAVE;
    self->iconFrames = iconFrames;
    self->iconImage = icon;
    self->data = data;
    self->dataSize = size;
    if (TaskObjF__Validate(self)) {
        if (self->methods->probeMemcardFile(self, NULL, fileName) != 0) {
            state = TASKOBJF_STATE_SAVE_OVERWRITE_WARNING;
            if (self->state == state) {
                state = TASKOBJF_STATE_EDIT_TITLE;
            } else if (self->state == TASKOBJF_STATE_EDIT_TITLE) {
                state = TASKOBJF_STATE_SAVING;
            }
            self->methods->setState(self, state);
        } else if (!self->methods->checkCardSpace(self, iconFrames, size)) {
            self->methods->setState(self, TASKOBJF_STATE_SAVE_NO_SPACE);
        } else {
            methods = self->methods;
            state = TASKOBJF_STATE_EDIT_TITLE;
            if (self->state == state) {
                state = TASKOBJF_STATE_SAVING;
            }
            methods->setState(self, state);
        }
    }
}

/* Returns 1 when checkCardStatus succeeds on a formatted, unchanged card;
 * otherwise sets the state that says why and returns 0. */
/* MATCHING: the `formatted` test stays though a formatted, unchanged card has
 * already returned 1; without it the tests are laid out differently. */
s32 TaskObjF__Validate(TaskObjF *self) {
    s32 error;
    s32 cardChanged;
    s32 formatted;
    s32 ok;
    s32 state;

    self->methods->openEvents(self);
    ok = self->methods->checkCardStatus(self, &error, &cardChanged, &formatted);
    self->methods->closeEvents(self);

    if (ok != 0) {
        if (cardChanged == 0 && formatted != 0) {
            return 1;
        }
    }

    if (ok == 0) {
        state = TASKOBJF_STATE_NO_CARD;
    } else if (error != 0) {
        state = TASKOBJF_STATE_CARD_ERROR;
    } else if (cardChanged != 0) {
        state = TASKOBJF_STATE_CARD_CHANGED;
    } else if (formatted == 0) {
        if (self->opMode == TASKOBJF_OP_LOAD) {
            state = TASKOBJF_STATE_UNFORMATTED_LOAD;
        } else {
            state = TASKOBJF_STATE_UNFORMATTED_SAVE;
        }
    }
    self->methods->setState(self, state);
    return 0;
}

void TaskObjF__OnNotify(TaskObjF *self, void *sender, s32 event) {
    TaskObjFMethods *methods;
    BasicClassMethods *base;
    s32 tag;
    s32 kind;

    methods = self->methods;
    base = GetBasicClassMethods();
    base->onNotify((BasicClass *)self, sender, event);

    tag = ((BasicClass *)sender)->methods->header;
    kind = tag & CLASS_ID_ROOT_MASK;
    if (kind == PAD_CLASS_ID) {
        methods->onInputEvent(self, sender, event);
    } else if (kind == FRAMECLOCK_CLASS_ID) {
        methods->tickStateDelay(self, sender, event);
    } else {
        kind = tag & CLASS_ID_LEVEL2_MASK;
        if (kind == TEXTENTRY_CLASS_ID) {
            methods->onTextEntryResult(self, sender, event);
        } else if (kind == ITEMLIST_CLASS_ID) {
            methods->onItemListResult(self, sender, event);
        }
    }
}

/*
 * TaskObjF's state machine: slots +0x07C..+0x0B0 of gTaskObjFMethods and
 * the table getter, plus StampSaveTitleFileLetter, which writes a save
 * file's letter into its title.
 *
 * setState (enum TaskObjFState) notifies the parent, swaps the message icon
 * (a ScreenSprite of CARD\<name>.TIM: loadCardIcon, releaseCardIcon) and
 * runs the new state's entry action: format, write or read the card, or
 * attach the title editor (a TextEntry) or the file chooser (an ItemList).
 * A circle press on the Pad reaches advanceState (retry, format, go on) and
 * a cross press abortFromState (abort); the tick source counts down
 * FORMATTING, SAVING and LOADING before their action runs. The two widgets
 * are made on first use, driven through the slots +0x044..+0x050 both
 * classes put at the same offsets, and report back through
 * onTextEntryResult and onItemListResult.
 */

/* MATCHING: `methods` is cached, and the cases stay in this order (entry actions
 * before widgets). */
void TaskObjF__SetState(TaskObjF *self, s32 state) {
    TaskObjFMethods *methods = self->methods;
    s32 ok;
    s32 i;

    if (self->state == state) {
        state = TASKOBJF_STATE_ABORTED;
    }

    methods->notifyParents(self, state);
    methods->releaseCardIcon(self);
    methods->loadCardIcon(self, state);

    self->waitCounter = 0;
    switch (state) {
        case TASKOBJF_STATE_FORMAT:
            state = methods->formatCard(self) ? TASKOBJF_STATE_EDIT_TITLE : TASKOBJF_STATE_FORMAT_ERROR;
            methods->setState(self, state);
            break;
        case TASKOBJF_STATE_WRITE:
            if (self->fileName[0] == '\0') {
                methods->findUnusedMemcardName(self, self->fileName, self->namePrefix, self->nameSuffixes);
            }
            ok = methods->writeMemcardSaveFile(self, self->fileName, self->title, self->iconFrames,
                                               self->iconImage, self->data, self->dataSize);
            state = ok ? TASKOBJF_STATE_DONE : TASKOBJF_STATE_SAVE_ERROR;
            methods->setState(self, state);
            break;
        case TASKOBJF_STATE_READ:
            ok = methods->readMemcardFile(self, self->fileName, self->data, self->dataSize);
            state = ok ? TASKOBJF_STATE_DONE : TASKOBJF_STATE_LOAD_ERROR;
            methods->setState(self, state);
            break;
        case TASKOBJF_STATE_EDIT_TITLE:
            methods->attachTextEntry(self);
            break;
        case TASKOBJF_STATE_CHOOSE_FILE:
            methods->attachItemList(self);
            break;
    }

    if ((u32)(state - TASKOBJF_STATE_DONE) < 2) { /* DONE or ABORTED */
        if (self->opMode == TASKOBJF_OP_LOAD && self->titles != NULL) {
            BMemPMgrFree(self->foundSuffixes);
            for (i = 0; i < self->bufCount; i++) {
                BMemPMgrFree(self->titles[i]);
            }
            BMemPMgrFree(self->titles);
            self->titles = NULL;
        }
        self->state = TASKOBJF_STATE_IDLE;
        self->opMode = TASKOBJF_OP_NONE;
    } else {
        self->state = state;
    }
}

/* The message icon's name per state from NO_CARD, CARD\<name>.TIM
 * ("NOCONECT" .. "LOADERR" for states 2..16). */
#define CARD_ICON_COUNT (TASKOBJF_STATE_EDIT_TITLE - TASKOBJF_STATE_NO_CARD)
extern char *sCardIconNames[CARD_ICON_COUNT];
/* {0, 0, 160, 120} */
extern SpriteRect sCardIconRect;

void TaskObjF__LoadCardIcon(TaskObjF *self, s32 index) {
    char pathBuf[32];
    char *path;
    char *name;
    TimImage *tim;
    ScreenSprite *icon;

    /* MATCHING: `path` and `icon` hold the buffer and the sprite across the calls. */
    if (index >= TASKOBJF_STATE_EDIT_TITLE) {
        return;
    }
    if (self->spriteParent == 0) {
        return;
    }
    if (self->cardIcon != NULL) {
        return;
    }

    path = pathBuf;
    name = sCardIconNames[index - TASKOBJF_STATE_NO_CARD];
    path[0] = '\0';
    strcat(path, sTitleCardPathPrefix);
    strcat(path, name);
    strcat(path, sCardPathSuffix);

    tim = New_TimImage(path);
    ((TimImageUploadFn)tim->methods->processBuffer)(tim);
    icon = New_ScreenSprite(tim, &sCardIconRect, 0);
    self->cardIcon = icon;
    tim->methods->release(tim);
    icon->methods->attachToParent(icon, self->spriteParent, (LongVec3 *)&sCardIconPos);
}

void TaskObjF__ReleaseCardIcon(TaskObjF *self) {
    if (self->cardIcon != NULL) {
        self->cardIcon = self->cardIcon->methods->release(self->cardIcon);
    }
}

void TaskObjF__OnInputEvent(TaskObjF *self, void *sender, s32 event) {
    if (self->state != TASKOBJF_STATE_IDLE) {
        if (event == PAD_EVENT_PRESSED + PAD_BUTTON_RRIGHT) {
            self->methods->advanceState(self);
        } else if (event == PAD_EVENT_PRESSED + PAD_BUTTON_RDOWN) {
            self->methods->abortFromState(self);
        }
    }
}

void TaskObjF__PlaySound(TaskObjF *self, s32 index) {
    if (self->sound != NULL) {
        self->sound->methods->playTone(self->sound, index, TASKOBJF_TONE_VOLUME, TASKOBJF_TONE_VOLUME);
    }
}

void TaskObjF__AdvanceState(TaskObjF *self) {
    TaskObjFMethods *methods = self->methods;

    switch (self->state) {
        case TASKOBJF_STATE_NO_CARD:
        case TASKOBJF_STATE_CARD_CHANGED:
        case TASKOBJF_STATE_SAVE_OVERWRITE_WARNING:
        case TASKOBJF_STATE_LOAD_WARNING:
            methods->playSound(self, TASKOBJF_TONE_PROCEED);
            if (self->state == TASKOBJF_STATE_LOAD_WARNING) {
                strcpy(self->fileName, self->namePrefix);
                strcat(self->fileName, self->foundSuffixes[self->selectedIndex]);
                strcpy(self->title, self->titles[self->selectedIndex]);
            }
            /* MATCHING: an if chain; a switch tests LOAD first. */
            if (self->opMode == TASKOBJF_OP_SAVE) {
                methods->beginSave(self, self->fileName, self->title, self->titleEditPos,
                                   self->iconFrames, self->iconImage, self->data, self->dataSize);
            } else if (self->opMode == TASKOBJF_OP_LOAD) {
                methods->beginLoad(self, self->fileName, self->title, self->data, self->dataSize);
            }
            break;
        case TASKOBJF_STATE_UNFORMATTED_SAVE:
            methods->playSound(self, TASKOBJF_TONE_PROCEED);
            methods->setState(self, TASKOBJF_STATE_FORMATTING);
            break;
        case TASKOBJF_STATE_CARD_ERROR:
        case TASKOBJF_STATE_UNFORMATTED_LOAD:
        case TASKOBJF_STATE_FORMAT_ERROR:
        case TASKOBJF_STATE_SAVE_NO_SPACE:
        case TASKOBJF_STATE_SAVE_ERROR:
        case TASKOBJF_STATE_LOAD_NOT_FOUND:
        case TASKOBJF_STATE_LOAD_ERROR:
            methods->playSound(self, TASKOBJF_TONE_BACK);
            methods->setState(self, TASKOBJF_STATE_ABORTED);
            break;
    }
}

void TaskObjF__AbortFromState(TaskObjF *self) {
    switch (self->state) {
        case TASKOBJF_STATE_CARD_CHANGED:
        case TASKOBJF_STATE_UNFORMATTED_SAVE:
        case TASKOBJF_STATE_SAVE_OVERWRITE_WARNING:
        case TASKOBJF_STATE_LOAD_WARNING:
            self->methods->playSound(self, TASKOBJF_TONE_BACK);
            self->methods->setState(self, TASKOBJF_STATE_ABORTED);
            break;
        default:
            break;
    }
}

/* Waits until `waitCounter` (zeroed by setState) passes 6, then runs the
 * state's action. */
/* MATCHING: `old` and `count` kept apart, and one setState call per branch. */
void TaskObjF__TickStateDelay(TaskObjF *self) {
    s32 old;
    s32 count;

    if (self->state == TASKOBJF_STATE_FORMATTING) {
        old = self->waitCounter;
        count = old + 1;
        self->waitCounter = count;
        if (old < 6) {
            return;
        }
        self->methods->setState(self, TASKOBJF_STATE_FORMAT);
    } else if (self->state == TASKOBJF_STATE_SAVING) {
        old = self->waitCounter;
        count = old + 1;
        self->waitCounter = count;
        if (old < 6) {
            return;
        }
        self->methods->setState(self, TASKOBJF_STATE_WRITE);
    } else if (self->state == TASKOBJF_STATE_LOADING) {
        old = self->waitCounter;
        count = old + 1;
        self->waitCounter = count;
        if (old < 6) {
            return;
        }
        self->methods->setState(self, TASKOBJF_STATE_READ);
    }
}

void TaskObjF__AttachTextEntry(TaskObjF *self) {
    if (self->spriteParent != 0 && self->inputSource != 0) {
        if (self->textEntry == NULL) {
            self->textEntry =
                New_TextEntry(&self->title[self->titleEditPos * 2], TEXTENTRY_MODE_FULLWIDTH);
            self->ownsWidget = 1;
        }
        self->methods->addChild(self, (BasicClass *)self->textEntry);
        self->textEntry->methods->loadCardResources(self->textEntry, self->spriteParent);
        self->textEntry->methods->attachTarget(self->textEntry, self->inputSource, self->tickSource,
                                               self->sound);
    }
}

void TaskObjF__DetachTextEntry(TaskObjF *self) {
    if (self->spriteParent != 0 && self->inputSource != 0 && self->textEntry != NULL) {
        self->textEntry->methods->detachTarget(self->textEntry);
        self->textEntry->methods->releaseCardResources(self->textEntry);
        if (self->ownsWidget != 0) {
            self->textEntry->methods->release(self->textEntry);
            self->textEntry = NULL;
        }
    }
}

void TaskObjF__OnTextEntryResult(TaskObjF *self, void *sender, s32 result) {
    switch (result) {
        case TEXTENTRY_RESULT_ACCEPTED:
            self->methods->detachTextEntry(self);
            self->methods->beginSave(self, self->fileName, self->title, self->titleEditPos,
                                     self->iconFrames, self->iconImage, self->data, self->dataSize);
            break;
        case TEXTENTRY_RESULT_CANCELLED:
            self->methods->detachTextEntry(self);
            self->methods->setState(self, TASKOBJF_STATE_ABORTED);
            break;
    }
}

void TaskObjF__AttachItemList(TaskObjF *self) {
    if (self->spriteParent != 0 && self->inputSource != 0) {
        if (self->itemList == NULL) {
            self->itemList = New_ItemList(self->titles, ITEMLIST_MODE_FULLWIDTH);
            self->ownsWidget = 1;
        }
        self->methods->addChild(self, (BasicClass *)self->itemList);
        self->itemList->methods->loadResources(self->itemList, self->spriteParent);
        self->itemList->methods->attachTarget(self->itemList, self->inputSource, self->tickSource,
                                              self->sound);
    }
}

void TaskObjF__DetachItemList(TaskObjF *self) {
    if (self->spriteParent != 0 && self->inputSource != 0 && self->itemList != NULL) {
        self->itemList->methods->detachTarget(self->itemList);
        self->itemList->methods->releaseResources(self->itemList);
        if (self->ownsWidget != 0) {
            self->itemList->methods->release(self->itemList);
            self->itemList = NULL;
        }
    }
}

void TaskObjF__OnItemListResult(TaskObjF *self, ItemList *list, s32 result) {
    switch (result) {
        case ITEMLIST_RESULT_CHOSEN:
            self->selectedIndex = list->methods->getCursorIndex(list);
            self->methods->detachItemList(self);
            self->methods->setState(self, TASKOBJF_STATE_LOAD_WARNING);
            break;
        case ITEMLIST_RESULT_CANCELLED:
            self->methods->detachItemList(self);
            self->methods->setState(self, TASKOBJF_STATE_ABORTED);
            break;
    }
}

TaskObjFMethods *GetTaskObjFMethods(void) {
    return &gTaskObjFMethods;
}

/* The save title's layout (SAVE_TITLE_LETTER_FIELD and the rest) is in
 * include/title_menu.h. */
/* sSaveTitleGlyphs: the full-width letters a..o (0..14), one per save file
 * -01..-15, then three full-width spaces and "Day" (15..20). */
#define SAVE_TITLE_GLYPH_SPACES 15
/* A save file name is namePrefix ("BISLPS-01556", 12 characters) + "-NN";
 * the first digit of NN. */
#define SAVE_FILE_NAME_NUMBER 13

/* Writes a save file's letter into the full-width `title`: the letter
 * field becomes a space, the letter for the file name's -NN (a for -01 ..
 * o for -15) and a space, followed by "Day", and a space goes after the day
 * number. With no file name it only blanks the letter field. -08 and -09
 * are parsed from their second digit, which atoi would otherwise read as
 * octal. Returns a pointer into sSaveTitleGlyphs that no caller reads. */
/* MATCHING: `glyphs` is the return value, not a second read of the global. */
s32 StampSaveTitleFileLetter(char *titleText, char *fileName) {
    FullWidthChar *title = (FullWidthChar *)titleText;
    s32 numberPos;
    s32 letter;
    FullWidthChar *glyph;

    if (fileName != NULL) {
        numberPos = ((u32)(fileName[SAVE_FILE_NAME_NUMBER + 1] - '8') < 2) ? SAVE_FILE_NAME_NUMBER + 1
                                                                           : SAVE_FILE_NAME_NUMBER;

        /* MATCHING: FullWidthChar's byte members (alignment 1) set how these copies are done. */
        title[SAVE_TITLE_PADDING] = sSaveTitleGlyphs[SAVE_TITLE_GLYPH_SPACES];
        *(FullWidthChars6 *)&title[SAVE_TITLE_LETTER_FIELD] =
            *(FullWidthChars6 *)&sSaveTitleGlyphs[SAVE_TITLE_GLYPH_SPACES];

        letter = atoi(fileName + numberPos) - 1;
        glyph = &sSaveTitleGlyphs[letter];
        title[SAVE_TITLE_LETTER] = *glyph;
        return (s32)glyph;
    } else {
        FullWidthChar *glyphs = sSaveTitleGlyphs;

        *(FullWidthChars3 *)&title[SAVE_TITLE_LETTER_FIELD] =
            *(FullWidthChars3 *)&glyphs[SAVE_TITLE_GLYPH_SPACES];
        return (s32)glyphs;
    }
}

/* TaskObjF (include/task_objf.h): BasicClass's slots with the memory-card
 * controller's card, file, state, icon, input and dialog slots. */
TaskObjFMethods gTaskObjFMethods = {
    /* +0x000 header */ TASKOBJF_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ TaskObjF__TaskObjF,
    /* +0x00C finalize */ TaskObjF__Finalize,
    /* +0x010 addChild */ TaskObjF__AddChild,
    /* +0x014 removeChild */ TaskObjF__RemoveChild,
    /* +0x018 removeAllChildren */ TaskObjF__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ TaskObjF__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 setCardSlot */ TaskObjF__SetCardSlot,
    /* +0x044 openEvents */ TaskObjF__OpenEvents,
    /* +0x048 closeEvents */ TaskObjF__CloseEvents,
    /* +0x04C checkCardStatus */ TaskObjF__CheckCardStatus,
    /* +0x050 formatCard */ TaskObjF__FormatCard,
    /* +0x054 probeMemcardFile */ TaskObjF__ProbeMemcardFile,
    /* +0x058 findUnusedMemcardName */ TaskObjF__FindUnusedMemcardName,
    /* +0x05C collectExistingMemcardFiles */ TaskObjF__CollectExistingMemcardFiles,
    /* +0x060 checkCardSpace */ TaskObjF__CheckCardSpace,
    /* +0x064 readMemcardFile */ TaskObjF__ReadMemcardFile,
    /* +0x068 writeMemcardSaveFile */ TaskObjF__WriteMemcardSaveFile,
    /* +0x06C init */ TaskObjF__Init,
    /* +0x070 deinit */ TaskObjF__Deinit,
    /* +0x074 beginLoad */ TaskObjF__BeginLoad,
    /* +0x078 beginSave */ TaskObjF__BeginSave,
    /* +0x07C setState */ TaskObjF__SetState,
    /* +0x080 loadCardIcon */ TaskObjF__LoadCardIcon,
    /* +0x084 releaseCardIcon */ TaskObjF__ReleaseCardIcon,
    /* +0x088 onInputEvent */ TaskObjF__OnInputEvent,
    /* +0x08C playSound */ TaskObjF__PlaySound,
    /* +0x090 advanceState */ TaskObjF__AdvanceState,
    /* +0x094 abortFromState */ TaskObjF__AbortFromState,
    /* +0x098 tickStateDelay */ (void *)TaskObjF__TickStateDelay,
    /* +0x09C attachTextEntry */ TaskObjF__AttachTextEntry,
    /* +0x0A0 detachTextEntry */ TaskObjF__DetachTextEntry,
    /* +0x0A4 onTextEntryResult */ TaskObjF__OnTextEntryResult,
    /* +0x0A8 attachItemList */ TaskObjF__AttachItemList,
    /* +0x0AC detachItemList */ TaskObjF__DetachItemList,
    /* +0x0B0 onItemListResult */ TaskObjF__OnItemListResult,
};

s32 sCardEventSpecs[4] = {EvSpIOE, EvSpERROR, EvSpTIMOUT, EvSpNEW};

/* The message icons' names, CARD\<name>.TIM, for states NO_CARD to
 * LOAD_ERROR: three are read-only data (at the top of the file), the rest
 * small data. */
/* clang-format off */
char *sCardIconNames[CARD_ICON_COUNT] = {
    (char *)sNoCardIconName,       /* TASKOBJF_STATE_NO_CARD */
    sCardErrorIconName,            /* TASKOBJF_STATE_CARD_ERROR */
    sCardChangedIconName,          /* TASKOBJF_STATE_CARD_CHANGED */
    sUnformattedLoadIconName,      /* TASKOBJF_STATE_UNFORMATTED_LOAD */
    sUnformattedSaveIconName,      /* TASKOBJF_STATE_UNFORMATTED_SAVE */
    sFormattingIconName,           /* TASKOBJF_STATE_FORMATTING */
    sFormatErrorIconName,          /* TASKOBJF_STATE_FORMAT_ERROR */
    (char *)sSaveNoSpaceIconName,  /* TASKOBJF_STATE_SAVE_NO_SPACE */
    sSaveOverwriteIconName,        /* TASKOBJF_STATE_SAVE_OVERWRITE_WARNING */
    sSavingIconName,               /* TASKOBJF_STATE_SAVING */
    sSaveErrorIconName,            /* TASKOBJF_STATE_SAVE_ERROR */
    (char *)sLoadNotFoundIconName, /* TASKOBJF_STATE_LOAD_NOT_FOUND */
    sLoadWarningIconName,          /* TASKOBJF_STATE_LOAD_WARNING */
    sLoadingIconName,              /* TASKOBJF_STATE_LOADING */
    sLoadErrorIconName,            /* TASKOBJF_STATE_LOAD_ERROR */
};
/* clang-format on */

SpriteRect sCardIconRect = {0, 0, 160, 120};
