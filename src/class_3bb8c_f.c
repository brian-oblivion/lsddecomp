#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "class_3bb8c.h"
#include "TaskObjF.h"
#include "TimImage.h"
#include <sys/file.h>

/*
 * TaskObjF's file I/O, card events, load buffers and its two operations
 * (include/TaskObjF.h: slots +0x064..+0x078 and the +0x038 onNotify
 * override), in address order:
 *
 * - Memory-card files. TaskObjF__ReadMemcardFile and
 *   TaskObjF__WriteMemcardSaveFile retry TaskObjF__TryReadMemcardFile and
 *   TaskObjF__TryWriteMemcardSaveFile up to MEMCARD_RETRIES more times.
 *   Both open a "bu00:"/"bu10:" path (BuildMemcardPath) with the BIOS file
 *   calls: the write builds the PS-X save header (McSaveHeader) from the
 *   title and the icon TIM, and the read seeks past it.
 * - Card events. TaskObjF__EnableEvents/DisableEvents/TestEvents apply one
 *   kernel call to each of `events` through TaskObjF__ForEachEvent;
 *   WaitForReadyEvent spins until one tests ready.
 * - TaskObjF__Init and TaskObjF__Deinit.
 * - TaskObjF__BeginLoad and TaskObjF__BeginSave store the request, check
 *   the card (TaskObjF__Validate) and choose the next TaskObjFState;
 *   TaskObjF__AllocBuffers/FreeUnusedBuffers/FreeBuffers manage the load's
 *   title buffers.
 * - TaskObjF__OnNotify routes a child's notification by the child's class
 *   id, the way TaskObjF__AddChild files the children.
 */

/* Defined below, called earlier in address order. */
s32 WaitForReadyEvent(s32 *events, s32 count);
char *BuildMemcardPath(McDevicePath *dest, s32 cardSlot, char *suffix);

/* The PS-X BIOS file calls, linked from Sony's libapi. A local view:
 * class_3bb8c_e.c declares `open` with a `void *` path. */
extern s32 open(char *path, s32 mode);             /* B(0x32) */
extern s32 read(s32 handle, void *buf, s32 size);  /* B(0x34) */
extern s32 lseek(s32 handle, s32 pos, s32 whence); /* B(0x33) */
extern s32 close(s32 handle);                      /* B(0x36) */
extern s32 delete (void *path);                    /* B(0x45) */

/* Half of the icon's 16-colour CLUT.
 * MATCHING: all-s16 (alignment 2) makes the whole-struct copy retail's
 * unaligned lwl/lwr + swl/swr pairs. */
typedef struct IconPaletteHalf {
    s16 color[8];
} IconPaletteHalf;

/* One 16x16 4bpp icon frame, one sector.
 * MATCHING: a byte array (alignment 1) makes the whole-struct copy retail's
 * runtime-alignment-checked copy loop. */
typedef struct IconFrame {
    u8 raw[MEMCARD_SECTOR_SIZE];
} IconFrame;

/* The icon TimImage's file buffer, a 4bpp TIM with one 16-colour CLUT: the
 * pads are the TIM header with the CLUT block header, and the pixel block
 * header. Only the CLUT and the first three frames of pixels are read. */
typedef struct McIconSource {
    u8 pad0[0x14];
    IconPaletteHalf palette[2]; /* +0x14 */
    u8 pad34[0x40 - 0x34];
    IconFrame frame0; /* +0x40 */
    IconFrame frame1; /* +0xC0 */
    IconFrame frame2; /* +0x140 */
} McIconSource;

/* The PS-X memory-card save header, MEMCARD_SAVE_HEADER_SIZE bytes: the
 * title sector ('S', 'C', the icon display flag, the file's size in blocks,
 * the title field, the CLUT), then up to three icon frames. */
typedef struct McSaveHeader {
    u8 magic0;
    u8 magic1;
    u8 iconDisplayFlag;
    u8 blockCount;
    char title[92]; /* +0x04..+0x5F: the format's 64-byte title and its reserved bytes */
    IconPaletteHalf palette[2];
    IconFrame frame0;
    IconFrame frame1;
    IconFrame frame2;
} McSaveHeader;

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
    char pathBuf[32];
    char *path;
    s32 handle;
    McSaveHeader *header;
    s32 seekPos;
    u8 iconFlag;

    path = BuildMemcardPath((McDevicePath *)pathBuf, self->cardSlot, suffix);
    handle = open(path, O_RDONLY);
    if (handle == -1) {
        return 0;
    }
    /* Only the title sector: its icon display flag says where the data starts. */
    header = BMemPMgrAlloc(MEMCARD_SECTOR_SIZE);
    read(handle, header, MEMCARD_SECTOR_SIZE);
    iconFlag = header->iconDisplayFlag;
    /* The data follows the title sector and the icon frames.
     * MATCHING: (iconFlag - 0xF) * MEMCARD_SECTOR_SIZE reorders the arithmetic. */
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
    StampSaveTitleFileLetter((s32)title, (s32)fileName);
    do {
        result = TaskObjF__TryWriteMemcardSaveFile(self, fileName, title, iconFrames & 0xFF, icon,
                                                   data, size);
        if (result != 0) {
            break;
        }
    } while (retries-- != 0);
    if (result == 0) {
        StampSaveTitleFileLetter((s32)title, 0);
    }
    return result;
}

extern const char sFileNotCreatedMsg[]; /* "File not create in WriteFile\n" */
/* The BIOS write, B(0x35), linked from Sony's libapi/a53. */
extern s32 write(s32 handle, void *buf, s32 size);
/* Sony's libc2 functions. A local view of printf: the call here passes
 * only the format string. */
extern char *strcpy(char *dest, char *src);
extern void printf(const char *fmt);

/* Deletes the file, creates it at its full size in blocks, then reopens it
 * to write the save header and the data, each rounded up to whole sectors.
 * MATCHING: iconFrames is u8: retail keeps the incoming word and its
 * zero-extended copy in two registers. */

s32 TaskObjF__TryWriteMemcardSaveFile(TaskObjF *self, char *fileName, char *title, u8 iconFrames,
                                      struct TimImage *icon, void *data, s32 size) {
    char pathBuf[32];
    char *path;
    s32 fileHandle;
    s32 openMode;
    McIconSource *iconSrc;
    McSaveHeader *header;

    path = BuildMemcardPath((McDevicePath *)pathBuf, self->cardSlot, fileName);
    delete (path);
    openMode = MEMCARD_OPEN_BLOCKS(((u32)size + (MEMCARD_SAVE_HEADER_SIZE + MEMCARD_BLOCK_SIZE - 1)) >>
                                   MEMCARD_BLOCK_SHIFT) |
               O_CREAT;
    fileHandle = open(path, openMode);
    if (fileHandle == -1) {
        printf(sFileNotCreatedMsg);
        return 0;
    }
    close(fileHandle);
    fileHandle = open(path, O_WRONLY);
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
    header->palette[0] = iconSrc->palette[0];
    header->palette[1] = iconSrc->palette[1];
    header->frame0 = iconSrc->frame0;
    header->frame1 = iconSrc->frame1;
    header->frame2 = iconSrc->frame2;
    /* The title sector and the icon frames.
     * MATCHING: (iconFrames + 1) * MEMCARD_SECTOR_SIZE reorders the arithmetic. */
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
        device = &gMcDevicePath1;
    } else {
        device = &gMcDevicePath0;
    }
    *dest = *device;
    strcat((char *)dest, suffix);
    return (char *)dest;
}

/* Sony's kernel event calls (libapi/a11..a13). Each takes an event
 * descriptor and returns a status word, which is what lets
 * TaskObjF__ForEachEvent take them as its callback. */
extern s32 EnableEvent(s32 event);
extern s32 DisableEvent(s32 event);
extern s32 TestEvent(s32 event);

s32 TaskObjF__EnableEvents(TaskObjF *self) {
    return TaskObjF__ForEachEvent(self, EnableEvent, 1);
}

s32 TaskObjF__DisableEvents(TaskObjF *self) {
    return TaskObjF__ForEachEvent(self, DisableEvent, 1);
}

s32 TaskObjF__TestEvents(TaskObjF *self) {
    return TaskObjF__ForEachEvent(self, TestEvent, 0);
}

/* Sony's libapi/a36 and a37. EnterCriticalSection returns int; nothing
 * here reads it. */
extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);

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
 * gCardEventSpecs entry: which answer the card gave. */
s32 WaitForReadyEvent(s32 *events, s32 count) {
    s32 i;

    for (;;) {
        for (i = 0; i < count; i++) {
            if (TestEvent(events[i]) != 0) {
                return gCardEventSpecs[i];
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
 * from EDIT_TITLE to save.
 * MATCHING: each branch makes its own setState call, which GCC cross-jumps
 * to one; the function returns nothing. */
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
 * otherwise sets the state that says why and returns 0.
 * MATCHING: the unreachable `formatted` branch and the one setState call
 * reached by goto keep retail's branch layout. */
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
    } else if (formatted != 0) {
        goto dispatch;
    } else if (self->opMode == TASKOBJF_OP_LOAD) {
        state = TASKOBJF_STATE_UNFORMATTED_LOAD;
    } else {
        state = TASKOBJF_STATE_UNFORMATTED_SAVE;
    }

dispatch:
    self->methods->setState(self, state);
    return 0;
}

void TaskObjF__OnNotify(TaskObjF *self, void *sender, s32 event) {
    TaskObjFMethods *methods;
    BasicClassMethods *base;
    s32 tag;
    s32 kind;

    methods = self->methods;
    base = Get_vtable_BasicClass();
    base->onNotify((BasicClass *)self, sender, event);

    /* Pad's class id is 0x2 and FrameClock's 0x5, matched with their
     * subclasses on the low nibble; TextEntry's is 0x10, ItemList's 0x20. */
    tag = ((BasicClass *)sender)->methods->header;
    kind = tag & 0xF;
    if (kind == 0x2) {
        methods->onInputEvent(self, sender, event);
    } else if (kind == 0x5) {
        methods->tickStateDelay(self, sender, event);
    } else {
        kind = tag & 0xFF;
        if (kind == 0x10) {
            methods->onTextEntryResult(self, sender, event);
        } else if (kind == 0x20) {
            methods->onItemListResult(self, sender, event);
        }
    }
}
