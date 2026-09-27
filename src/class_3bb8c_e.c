#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <kernel.h>
#include <sys/file.h>
#include "BasicClass.h"
#include "class_3bb8c.h"
#include "TaskObjF.h"
#include "Pad.h"
#include "FrameClock.h"

/*
 * TaskObjF's child links, card events, card checks and file probes
 * (include/TaskObjF.h: slots +0x00C..+0x018 and +0x040..+0x060, and the
 * helpers they call), in address order:
 *
 * - Children. TaskObjF__AddChild files a child by its class id into one of
 *   four links: a Pad as inputSource, a FrameClock as tickSource, a
 *   TextEntry, an ItemList. TaskObjF__RemoveChild and
 *   TaskObjF__RemoveAllChildren clear them; TaskObjF__ClearLinks clears
 *   them and spriteParent at construction.
 * - The card. TaskObjF__SetCardSlot picks slot 0 or 1 and its BIOS channel.
 *   TaskObjF__OpenEvents opens one SwCARD event per gCardEventSpecs entry;
 *   TaskObjF__CloseEvents closes them.
 * - Card checks. TaskObjF__CheckCardStatus retries
 *   TaskObjF__CardInfoAndLoadStatus, which asks _card_info whether a card is
 *   there and new (TaskObjF__CardInfoStatus), then _card_load whether it is
 *   formatted (TaskObjF__CardLoadStatus). TaskObjF__FormatCard retries
 *   format().
 * - Files, named "bu00:"/"bu10:" plus a prefix and a suffix
 *   (BuildMemcardPath). TaskObjF__ProbeMemcardFile opens one and can copy
 *   out its title (TaskObjF__OpenAndReadMemcardFile);
 *   TaskObjF__FindUnusedMemcardName and
 *   TaskObjF__CollectExistingMemcardFiles probe a list of suffixes through
 *   the probeMemcardFile slot. TaskObjF__CheckCardSpace retries
 *   TaskObjF__ProbeCardFreeSpace, which creates and deletes a TEMP file of
 *   the save's size.
 *
 * A retried call is tried once, then up to MEMCARD_RETRIES more times while
 * it fails (TaskObjF__ProbeMemcardFile's loop: no more times).
 */

/* Defined in class_3bb8c_f.c, which types `dest` as McDevicePath *: it
 * writes the device name, appends `suffix`, and returns `dest`. */
extern char *BuildMemcardPath(void *dest, s32 cardSlot, char *suffix);

/* "TEMP": the name TaskObjF__ProbeCardFreeSpace creates to test for space. */
extern char sMcTempFileSuffix[];

extern void *BMemPMgrAlloc(s32 size);
extern char *strcpy(char *dest, char *src);

void TaskObjF__ClearLinks(TaskObjF *self) {
    self->inputSource = NULL;
    self->tickSource = NULL;
    self->spriteParent = NULL;
    self->textEntry = NULL;
    self->itemList = NULL;
}

void TaskObjF__Finalize(TaskObjF *self) {
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void TaskObjF__AddChild(TaskObjF *self, BasicClass *child) {
    s32 classId;

    if (child == NULL) {
        return;
    }
    Get_vtable_BasicClass()->addChild((BasicClass *)self, child);
    classId = child->methods->header;
    if ((classId & CLASS_ID_ROOT_MASK) == PAD_CLASS_ID) {
        self->inputSource = child;
        return;
    }
    if ((classId & CLASS_ID_ROOT_MASK) == FRAMECLOCK_CLASS_ID) {
        self->tickSource = child;
        return;
    }
    if ((classId & 0xFF) == 0x10) {
        self->textEntry = (struct TextEntry *)child;
        return;
    }
    if ((classId & 0xFF) == 0x20) {
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
    } else if ((classId & 0xFF) == 0x10) {
        self->textEntry = NULL;
    } else if ((classId & 0xFF) == 0x20) {
        self->itemList = NULL;
    }
    Get_vtable_BasicClass()->removeChild((BasicClass *)self, child);
}

void TaskObjF__RemoveAllChildren(TaskObjF *self) {
    self->inputSource = NULL;
    self->tickSource = NULL;
    self->spriteParent = NULL;
    self->textEntry = NULL;
    self->itemList = NULL;
    Get_vtable_BasicClass()->removeAllChildren((BasicClass *)self);
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
        self->events[i] = OpenEvent(SwCARD, gCardEventSpecs[i], EvMdNOINTR, NULL);
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
    /* MATCHING: the retry count is tested after the call; testing it first
     * cross-jumps the two calls into one. */
    while (result == 0 || *error != 0 || *formatted == 0) {
        result = TaskObjF__CardInfoAndLoadStatus(self, error, cardChanged, formatted);
        if (retries-- == 0) {
            break;
        }
    }
    *cardChanged = *cardChanged | firstChanged;
    return result;
}

/* Returns whatever the last call left in $v0: CardInfoStatus's 0, or
 * CardLoadStatus's status.
 * MATCHING: an explicit return adds two words. */
s32 TaskObjF__CardInfoAndLoadStatus(TaskObjF *self, s32 *error, s32 *cardChanged, s32 *formatted) {
    if (TaskObjF__CardInfoStatus(self, error, cardChanged) != 0) {
        TaskObjF__CardLoadStatus(self, error, formatted);
    }
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
        path = self->cardSlot != 0 ? &gMcDevicePath1 : &gMcDevicePath0;
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
    char pathBuf[32];
    char *path;
    s32 handle;
    void *header;

    path = BuildMemcardPath(pathBuf, self->cardSlot, suffix);
    handle = open(path, O_RDONLY);
    if (handle == -1) {
        return 0;
    }
    if (destTitle != NULL) {
        header = BMemPMgrAlloc(MEMCARD_SECTOR_SIZE);
        read(handle, header, MEMCARD_SECTOR_SIZE);
        strcpy(destTitle, (char *)header + 4); /* the save header's title */
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
    char pathBuf[32];
    char *path;
    s32 handle;
    s32 blocks;

    /* MATCHING: the u32 cast makes the shift retail's srl. */
    blocks = (u32)(size + MEMCARD_SAVE_HEADER_SIZE + MEMCARD_BLOCK_SIZE - 1) >> MEMCARD_BLOCK_SHIFT;
    path = BuildMemcardPath(pathBuf, self->cardSlot, sMcTempFileSuffix);
    handle = open(path, MEMCARD_OPEN_BLOCKS(blocks) | O_CREAT);
    if (handle == -1) {
        return 0;
    }
    close(handle);
    /* MATCHING: delete(path), the same address, changes the code. */
    delete (pathBuf);
    return 1;
}
