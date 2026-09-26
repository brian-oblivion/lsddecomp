#include "common.h"
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
extern void *BuildMemcardPath(void *dest, s32 selector, void *suffix);

/* PSX thread-table constant walked by TaskObjF__OpenEvents (4 entries, one per
 * OpenTh-style thread it starts). Address-only-derived walk (lui/addiu then
 * plain lw at increasing offsets), never gp-relative, so unaffected by the
 * project's gp_rel blocker. */
extern s32 D_80086E78[4];
extern s32 OpenEvent(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Literal "TEMP" (asm/data/7B12C.sdata.s) -- the throwaway suffix
 * TaskObjF__ProbeCardFreeSpace passes as BuildMemcardPath's 3rd argument to
 * build a placeholder file name when probing free space. */
extern s32 sMcTempFileSuffix;

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
    s32 tag;

    if (child == NULL) {
        return;
    }
    Get_vtable_BasicClass()->addChild((BasicClass *)self, child);
    tag = child->methods->header;
    if ((tag & 0xF) == 2) {
        self->inputSource = child;
        return;
    }
    if ((tag & 0xF) == 5) {
        self->tickSource = child;
        return;
    }
    if ((tag & 0xFF) == 0x10) {
        self->textEntry = (struct TextEntry *)child;
        return;
    }
    if ((tag & 0xFF) == 0x20) {
        self->itemList = (struct ItemList *)child;
    }
}

void TaskObjF__RemoveChild(TaskObjF *self, BasicClass *child) {
    s32 tag;

    if (child == NULL) {
        return;
    }
    tag = child->methods->header;
    if ((tag & 0xF) == 2) {
        self->inputSource = NULL;
    } else if ((tag & 0xF) == 5) {
        self->tickSource = NULL;
    } else if ((tag & 0xFF) == 0x10) {
        self->textEntry = NULL;
    } else if ((tag & 0xFF) == 0x20) {
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

void TaskObjF__SetCardSlot(TaskObjF *self, s32 val) {
    self->cardSlot = val;
    self->cardHandle = val << 4;
}

extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);

s32 TaskObjF__OpenEvents(TaskObjF *self) {
    s32 i;
    TaskObjF *cur;

    EnterCriticalSection();
    i = 0;
    cur = self;
    do {
        cur->events[0] = OpenEvent(0xF4000001, D_80086E78[i], 0x2000, 0);
        i++;
        cur = (TaskObjF *)((u8 *)cur + 4);
    } while (i < 4);
    ExitCriticalSection();
    TaskObjF__EnableEvents(self);
    return 1;
}

/* Psy-Q's CloseEvent (libapi), passed as TaskObjF__ForEachEvent's callback. */
extern s32 CloseEvent(s32 event);

s32 TaskObjF__CloseEvents(TaskObjF *self) {
    TaskObjF__DisableEvents(self);
    TaskObjF__ForEachEvent(self, CloseEvent, 1);
    return 1;
}

s32 TaskObjF__CheckCardStatus(TaskObjF *self, s32 *p1, s32 *p2, s32 *p3) {
    s32 retries;
    s32 localFlag;
    s32 result;

    retries = 10;
    *p2 = 0;
    result = TaskObjF__CardInfoAndLoadStatus(self, p1, &localFlag, p3);
    while (result == 0 || *p1 != 0 || *p3 == 0) {
        result = TaskObjF__CardInfoAndLoadStatus(self, p1, p2, p3);
        if (retries-- == 0) {
            break;
        }
    }
    *p2 = *p2 | localFlag;
    return result;
}

s32 TaskObjF__CardInfoAndLoadStatus(TaskObjF *self, s32 *p1, s32 *p2, s32 *p3) {
    if (TaskObjF__CardInfoStatus(self, p1, p2) != 0) {
        TaskObjF__CardLoadStatus(self, p1, p3);
    }
}

extern s32 _card_info(s32 arg0);
extern s32 _card_clear(s32 arg0);

s32 TaskObjF__CardInfoStatus(TaskObjF *self, s32 *p1, s32 *p2) {
    s32 status;
    s32 code;

    status = 1;
    *p2 = *p1 = 0;
    TaskObjF__TestEvents(self);
    while (_card_info(self->cardHandle) == 0)
        ;
    code = TaskObjF__WaitForReadyEvent(self);
    if (code == 0x100) {
        status = 0;
    } else if (code == 0x8000) {
        status = 0;
        *p1 = 1;
    } else if (code == 0x2000) {
        *p2 = 1;
        _card_clear(self->cardHandle);
    }
    return status;
}

extern s32 _card_load(s32 arg0);

s32 TaskObjF__CardLoadStatus(TaskObjF *self, s32 *p1, s32 *p2) {
    s32 status;
    s32 code;

    status = 1;
    *p2 = (*p1 = 0, status);
    TaskObjF__TestEvents(self);
    while (_card_load(self->cardHandle) == 0)
        ;
    code = TaskObjF__WaitForReadyEvent(self);
    if (code == 0x100) {
        status = 0;
    } else if (code == 0x8000) {
        status = 0;
        *p1 = 1;
    } else if (code == 0x2000) {
        *p2 = 0;
    }
    return status;
}

/* The PS-X BIOS format(), which takes a device name. gMcDevicePath0/1 are
 * the "bu00:"/"bu10:" templates include/class_3bb8c.h declares (track 4b,
 * round 85: this unit had its own `s32` view for this address-only use). */
extern s32 format(char *fs);

s32 TaskObjF__FormatCard(TaskObjF *self) {
    s32 retries;
    s32 result;
    McDevicePath *path;

    retries = 10;
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
s32 TaskObjF__ProbeMemcardFile(TaskObjF *self, char *destBuf, char *suffix) {
    s32 retries;
    s32 result;

    retries = 0;
    if (suffix == NULL || *suffix == 0) {
        return 0;
    }
    do {
        result = TaskObjF__OpenAndReadMemcardFile(self, destBuf, suffix);
    } while (result == 0 && retries-- != 0);
    return result;
}

extern s32 open(void *arg0, s32 arg1);
extern s32 read(s32 arg0, void *arg1, s32 arg2);
extern s32 close(s32 arg0);

s32 TaskObjF__OpenAndReadMemcardFile(TaskObjF *self, char *destBuf, char *suffix) {
    s32 pathBuf[8];
    void *path;
    s32 handle;
    void *buf;

    path = BuildMemcardPath(pathBuf, self->cardSlot, suffix);
    handle = open(path, 1);
    if (handle == -1) {
        return 0;
    }
    if (destBuf != NULL) {
        buf = BMemPMgrAlloc(0x80);
        read(handle, buf, 0x80);
        strcpy(destBuf, (char *)buf + 4);
        BMemPMgrFree(buf);
    }
    close(handle);
    return 1;
}

char *TaskObjF__FindUnusedMemcardName(TaskObjF *self, char *buf, char *middle, char **entries) {
    while (*entries != NULL) {
        strcpy(buf, middle);
        strcat(buf, *entries);
        if (self->methods->probeMemcardFile(self, 0, buf) == 0) {
            return buf;
        }
        entries++;
    }
    return NULL;
}

s32 TaskObjF__CollectExistingMemcardFiles(TaskObjF *self, char **destBufs, char **outArr,
                                          char *middle, char **entries) {
    s32 count;
    char buf[0x20];

    count = 0;
    while (*entries != NULL) {
        strcpy(buf, middle);
        strcat(buf, *entries);
        if (self->methods->probeMemcardFile(self, *destBufs, buf) != 0) {
            count++;
            *outArr = *entries;
            destBufs++;
            outArr++;
        }
        entries++;
    }
    return count;
}

s32 TaskObjF__CheckCardSpace(TaskObjF *self, u8 id, s32 sizeArg) {
    s32 retries;
    s32 result;

    retries = 10;
    do {
        result = TaskObjF__ProbeCardFreeSpace(self, id, sizeArg);
    } while (result == 0 && retries-- != 0);
    return result;
}

extern s32 delete (void *arg0);

s32 TaskObjF__ProbeCardFreeSpace(TaskObjF *self, u8 id, s32 sizeArg) {
    s32 pathBuf[8];
    void *path;
    s32 handle;
    s32 sectors;

    sectors = (u32)(sizeArg + 0x21FF) >> 13;
    path = BuildMemcardPath(pathBuf, self->cardSlot, &sMcTempFileSuffix);
    handle = open(path, (sectors << 16) | 0x200);
    if (handle == -1) {
        return 0;
    }
    close(handle);
    delete (pathBuf);
    return 1;
}
