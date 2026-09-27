#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <kernel.h>
#include <sys/file.h>
#include "BasicClass.h"
#include "class_3bb8c.h"
#include "TaskObjF.h"

/*
 * class_3bb8c_e (round 14; named round 78, track 3): 19 TaskObjF methods
 * (include/TaskObjF.h, track 4 round 89), carved from the same class_3bb8c
 * remainder segment as class_3bb8c_b/_c/_d/_f. 13 are entries of
 * gTaskObjFMethods --
 *   +0x00C finalize          = TaskObjF__Finalize
 *   +0x010 addChild          = TaskObjF__AddChild
 *   +0x014 removeChild       = TaskObjF__RemoveChild
 *   +0x018 removeAllChildren = TaskObjF__RemoveAllChildren
 *   +0x040..+0x060 (9 contiguous slots) = TaskObjF__SetCardSlot,
 *     TaskObjF__OpenEvents, TaskObjF__CloseEvents, TaskObjF__CheckCardStatus,
 *     TaskObjF__FormatCard, TaskObjF__ProbeMemcardFile,
 *     TaskObjF__FindUnusedMemcardName, TaskObjF__CollectExistingMemcardFiles,
 *     TaskObjF__CheckCardSpace
 * -- and the other six (`TaskObjF__ClearResourceSlots`,
 * `TaskObjF__CardInfoAndLoadStatus`, `TaskObjF__CardInfoStatus`,
 * `TaskObjF__CardLoadStatus`, `TaskObjF__OpenAndReadMemcardFile`,
 * `TaskObjF__ProbeCardFreeSpace`) are private helpers the slotted functions
 * call, mostly wrapping the PS-X memory-card BIOS calls
 * (`_card_info`/`_card_load`/`_card_clear`, `open`/`read`/`close`/`delete`,
 * `format`) behind this class's own retry idiom.
 *
 * Until round 89 this unit read the object through its own view,
 * `Node3bb8cE` (round 78 confirmed it was TaskObjF): its res02/res05/
 * res10/res20 are TaskObjF's inputSource/tickSource/textEntry/itemList,
 * filed by AddChild on the child's class id, and its zero-only `unk68` is
 * spriteParent.
 */

/* Helpers this unit calls into, defined in class_3bb8c_f.c (extern for a
 * function OUTSIDE this unit). */
/* BuildMemcardPath(dest, selector, suffix): 3-parameter, and both of this
 * unit's call sites pass all three. TaskObjF__OpenAndReadMemcardFile emits no $a2 set-up
 * because its own 3rd parameter arrives in $a2 and is forwarded unchanged
 * (round 75; this was an `arity-ok` K&R declaration until then, on the
 * reading that TaskObjF__OpenAndReadMemcardFile made a 2-argument call). */
extern char *BuildMemcardPath(void *dest, s32 cardSlot, char *suffix);

/* PSX thread-table constant walked by TaskObjF__OpenEvents (4 entries, one per
 * OpenTh-style thread it starts). Address-only-derived walk (lui/addiu then
 * plain lw at increasing offsets), never gp-relative, so unaffected by the
 * project's gp_rel blocker. */
extern s32 gCardEventSpecs[4];

/* Literal "TEMP" (asm/data/7B12C.sdata.s) -- the throwaway suffix
 * TaskObjF__ProbeCardFreeSpace passes as BuildMemcardPath's 3rd argument to
 * build a placeholder file name when probing free space. */
extern char sMcTempFileSuffix[];

extern void *BMemPMgrAlloc(s32 size);
extern void *BMemPMgrFree(void *ptr);
extern char *strcpy(char *dest, char *src);
extern char *strcat(char *dest, char *src);

void TaskObjF__ClearResourceSlots(TaskObjF *self) {
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
    if ((classId & 0xF) == 2) {
        self->inputSource = child;
        return;
    }
    if ((classId & 0xF) == 5) {
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
    if ((classId & 0xF) == 2) {
        self->inputSource = NULL;
    } else if ((classId & 0xF) == 5) {
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
    TaskObjF__ForEachEvent(self, (s32 (*)(s32))CloseEvent, 1);
    return 1;
}

s32 TaskObjF__CheckCardStatus(TaskObjF *self, s32 *error, s32 *cardChanged, s32 *formatted) {
    s32 retries;
    s32 firstChanged;
    s32 result;

    retries = MEMCARD_RETRIES;
    *cardChanged = 0;
    result = TaskObjF__CardInfoAndLoadStatus(self, error, &firstChanged, formatted);
    while (result == 0 || *error != 0 || *formatted == 0) {
        result = TaskObjF__CardInfoAndLoadStatus(self, error, cardChanged, formatted);
        if (retries-- == 0) {
            break;
        }
    }
    *cardChanged = *cardChanged | firstChanged;
    return result;
}

s32 TaskObjF__CardInfoAndLoadStatus(TaskObjF *self, s32 *error, s32 *cardChanged, s32 *formatted) {
    if (TaskObjF__CardInfoStatus(self, error, cardChanged) != 0) {
        TaskObjF__CardLoadStatus(self, error, formatted);
    }
}

s32 TaskObjF__CardInfoStatus(TaskObjF *self, s32 *error, s32 *cardChanged) {
    s32 status;
    s32 answer;

    status = 1;
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
        result = format((char *)path);
    } while (result == 0 && retries-- != 0);
    return result;
}

/* TaskObjF__OpenAndReadMemcardFile's 3rd parameter is the file-name suffix, forwarded verbatim
 * by TaskObjF__ProbeMemcardFile (after rejecting NULL/empty) and by TaskObjF__OpenAndReadMemcardFile as
 * BuildMemcardPath's 3rd argument. Round 75 corrected the earlier reading
 * that TaskObjF__OpenAndReadMemcardFile never used it: it never TOUCHES $a2, because the value
 * is already where the call wants it. */
s32 TaskObjF__ProbeMemcardFile(TaskObjF *self, char *destTitle, char *suffix) {
    s32 retries;
    s32 result;

    retries = 0;
    if (suffix == NULL || *suffix == 0) {
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
        strcpy(destTitle, (char *)header + 4);
        BMemPMgrFree(header);
    }
    close(handle);
    return 1;
}

char *TaskObjF__FindUnusedMemcardName(TaskObjF *self, char *buf, char *prefix, char **suffixes) {
    while (*suffixes != NULL) {
        strcpy(buf, prefix);
        strcat(buf, *suffixes);
        if (self->methods->probeMemcardFile(self, 0, buf) == 0) {
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

s32 TaskObjF__ProbeCardFreeSpace(TaskObjF *self, u8 iconFrames, s32 size) {
    char pathBuf[32];
    char *path;
    s32 handle;
    s32 blocks;

    blocks = (u32)(size + MEMCARD_SAVE_HEADER_SIZE + MEMCARD_BLOCK_SIZE - 1) >> MEMCARD_BLOCK_SHIFT;
    path = BuildMemcardPath(pathBuf, self->cardSlot, sMcTempFileSuffix);
    handle = open(path, MEMCARD_OPEN_BLOCKS(blocks) | O_CREAT);
    if (handle == -1) {
        return 0;
    }
    close(handle);
    delete (pathBuf);
    return 1;
}
