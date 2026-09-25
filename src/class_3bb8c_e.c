#include "common.h"
#include "BasicClass.h"
#include "class_3bb8c.h"

/*
 * class_3bb8c_e (round 14; named round 78, track 3): 19 functions carved
 * from the same 305-function class_3bb8c remainder segment as
 * class_3bb8c_b/_c/_d/_f. These declarations are kept local to this .c
 * rather than moved into include/class_3bb8c.h, per the project's
 * cross-unit-prototype rule.
 *
 * ROUND 78: `Node3bb8cE` IS `TaskObjF` (include/class_3bb8c.h,
 * class_3bb8c_d.c/_f.c/_g.c) -- CONFIRMED, not just suspected. Round 60
 * found `TaskObjF__OpenEvents` (this unit) filling the identical `events[4]`
 * offset `TaskObjF::events` occupies and handing the same pointer straight
 * to class_3bb8c_f.c's `TaskObjF__EnableEvents`, and flagged it as a
 * track-4 lead (docs/match-reports/TaskObjF__EnableEvents.md). Round 78
 * cross-checked it against `gTaskObjFMethods` itself
 * (asm/data/76DC8.data.s): 9 of this unit's 19 functions are LITERAL
 * entries of that table, at the exact offsets this unit had already
 * derived independently for its own `SelfMethods3bb8cE` view (and a base-table
 * view since replaced by include/BasicClass.h) --
 *   +0x00C finalize          = TaskObjF__Finalize
 *   +0x010 addChild          = TaskObjF__AddChild
 *   +0x014 removeChild       = TaskObjF__RemoveChild
 *   +0x018 removeAllChildren = TaskObjF__RemoveAllChildren
 *   +0x040..+0x060 (9 contiguous slots) = TaskObjF__SetCardSlot,
 *     TaskObjF__OpenEvents, TaskObjF__CloseEvents, TaskObjF__CheckCardStatus,
 *     TaskObjF__FormatCard, TaskObjF__ProbeMemcardFile,
 *     TaskObjF__FindUnusedMemcardName, TaskObjF__CollectExistingMemcardFiles,
 *     TaskObjF__CheckCardSpace
 * and `TaskObjF__SetCardSlot` (+0x040) is the exact function
 * `TaskObjF__TaskObjF`'s ctor calls directly as `self->methods->slot40(self,
 * arg2)` (class_3bb8c_d.c) -- the same real object, same real vtable, two
 * independent local views. `include/class_3bb8c.h`'s own `TaskObjFMethods`
 * struct mis-marks +0x040 as padding (`pad3C[0x044-0x03C]`); it is a real
 * slot -- proposed for track 4, see this round's match reports and the
 * broadcast. The type stays `Node3bb8cE` here regardless: unifying it with
 * `TaskObjF` is track 4's job, not track 3's, and the project's
 * multiple-independent-local-views convention
 * (docs/DECOMPILATION_LEARNINGS.md) is what makes keeping two names
 * legitimate until then.
 *
 * The other five functions (`TaskObjF__ClearResourceSlots`,
 * `TaskObjF__CardInfoAndLoadStatus`, `TaskObjF__CardInfoStatus`,
 * `TaskObjF__CardLoadStatus`, `TaskObjF__OpenAndReadMemcardFile`,
 * `TaskObjF__ProbeCardFreeSpace`) are NOT vtable entries (checked against
 * the full `gTaskObjFMethods` table) -- private helpers the slotted
 * functions above call, mostly wrapping the PS-X memory-card BIOS calls
 * (`_card_info`/`_card_load`/`_card_clear`, `open`/`read`/`close`/`delete`,
 * `format`) behind this class's own retry idiom.
 *
 */

/*
 * A typed child resource attached to a Node3bb8cE. Only the resource's own
 * header/type-tag word (methods->header, whose low byte(s) this unit tests
 * against literals 2/5/0x10/0x20) is read here -- TaskObjF__AddChild/TaskObjF__RemoveChild
 * never dereference the resource beyond that one word.
 */
typedef struct ResHeader3bb8cE ResHeader3bb8cE;
struct ResHeader3bb8cE {
    s32 header;   /* +0x000, low byte(s): type tag (2, 5, 0x10 or 0x20) */
};
typedef struct Res3bb8cE Res3bb8cE;
struct Res3bb8cE {
    ResHeader3bb8cE *methods;   /* +0x000 */
};

/*
 * The class itself. Offsets established purely from this unit's own 19
 * functions (see each field's comment). res02/res05/res10/res20 are four
 * typed resource slots, one per tag value TaskObjF__AddChild/TaskObjF__RemoveChild
 * dispatch on; unk68 is zeroed alongside them by TaskObjF__ClearResourceSlots/TaskObjF__RemoveAllChildren
 * but has no setter anywhere in this unit, so its pointee type is unproven.
 */
typedef struct Node3bb8cE Node3bb8cE;

/* The object's OWN vtable, at offset 0 -- distinct from the separately
 * fetched base-class table (`Get_vtable_BasicClass()`, include/BasicClass.h).
 * Only the one slot this unit's functions reach is typed. */
typedef struct SelfMethods3bb8cE SelfMethods3bb8cE;
struct SelfMethods3bb8cE {
    u8 pad000[0x054];
    /* TaskObjF__FindUnusedMemcardName: called (self, 0, buf) per candidate string; 0 return
     * means "match" (buf is returned as the winning string). */
    s32 (*slot54)(Node3bb8cE *self, s32 arg1, char *arg2); /* +0x054 */
};

struct Node3bb8cE {
    SelfMethods3bb8cE *methods;   /* +0x000, TaskObjF__FindUnusedMemcardName */
    u8 pad04[0x00C - 0x004];
    s32 cardSlot;              /* +0x00C, TaskObjF__SetCardSlot sets it (caller value); TaskObjF__FormatCard nonzero-tests it; TaskObjF__OpenAndReadMemcardFile/TaskObjF__ProbeCardFreeSpace forward it as BuildMemcardPath's arg1 */
    s32 cardHandle;             /* +0x010, TaskObjF__SetCardSlot: cardSlot << 4; TaskObjF__CardInfoStatus/TaskObjF__CardLoadStatus: a resource handle passed to _card_info/_card_load/_card_clear */
    s32 events[4];        /* +0x014..+0x020, TaskObjF__OpenEvents: 4 OpenTh-style thread handles, one per D_80086E78[] entry */
    u8 pad24[0x060 - 0x024];
    Res3bb8cE *res02;      /* +0x060, tag 2 */
    Res3bb8cE *res05;      /* +0x064, tag 5 */
    Res3bb8cE *unk68;      /* +0x068, zeroed only -- no setter in this unit */
    u8 pad6C[0x078 - 0x06C];
    Res3bb8cE *res10;      /* +0x078, tag 0x10 */
    Res3bb8cE *res20;      /* +0x07C, tag 0x20 */
};

/* Helpers this unit calls into, defined in class_3bb8c_f.c (extern for a
 * function OUTSIDE this unit). */
/* BuildMemcardPath(dest, selector, suffix): 3-parameter, and both of this
 * unit's call sites pass all three. TaskObjF__OpenAndReadMemcardFile emits no $a2 set-up
 * because its own 3rd parameter arrives in $a2 and is forwarded unchanged
 * (round 75; this was an `arity-ok` K&R declaration until then, on the
 * reading that TaskObjF__OpenAndReadMemcardFile made a 2-argument call). */
extern void *BuildMemcardPath(void *dest, s32 selector, void *suffix);
extern void TaskObjF__EnableEvents(void *self);
extern void *TaskObjF__DisableEvents(void *self);
extern void TaskObjF__TestEvents(void *self);
extern void TaskObjF__ForEachEvent(void *self, void (*fn)(void), s32 arg2);
extern s32 TaskObjF__WaitForReadyEvent(void *self);

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

void TaskObjF__ClearResourceSlots(Node3bb8cE *self)
{
    self->res02 = NULL;
    self->res05 = NULL;
    self->unk68 = NULL;
    self->res10 = NULL;
    self->res20 = NULL;
}

void TaskObjF__Finalize(Node3bb8cE *self)
{
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}

void TaskObjF__AddChild(Node3bb8cE *self, Res3bb8cE *res)
{
    s32 tag;

    if (res == NULL) {
        return;
    }
    Get_vtable_BasicClass()->addChild((BasicClass *)self, (BasicClass *)res);
    tag = res->methods->header;
    if ((tag & 0xF) == 2) {
        self->res02 = res;
        return;
    }
    if ((tag & 0xF) == 5) {
        self->res05 = res;
        return;
    }
    if ((tag & 0xFF) == 0x10) {
        self->res10 = res;
        return;
    }
    if ((tag & 0xFF) == 0x20) {
        self->res20 = res;
    }
}

void TaskObjF__RemoveChild(Node3bb8cE *self, Res3bb8cE *res)
{
    s32 tag;

    if (res == NULL) {
        return;
    }
    tag = res->methods->header;
    if ((tag & 0xF) == 2) {
        self->res02 = NULL;
    } else if ((tag & 0xF) == 5) {
        self->res05 = NULL;
    } else if ((tag & 0xFF) == 0x10) {
        self->res10 = NULL;
    } else if ((tag & 0xFF) == 0x20) {
        self->res20 = NULL;
    }
    Get_vtable_BasicClass()->removeChild((BasicClass *)self, (BasicClass *)res);
}

void TaskObjF__RemoveAllChildren(Node3bb8cE *self)
{
    self->res02 = NULL;
    self->res05 = NULL;
    self->unk68 = NULL;
    self->res10 = NULL;
    self->res20 = NULL;
    Get_vtable_BasicClass()->removeAllChildren((BasicClass *)self);
}

void TaskObjF__SetCardSlot(Node3bb8cE *self, s32 val)
{
    self->cardSlot = val;
    self->cardHandle = val << 4;
}

extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);

s32 TaskObjF__OpenEvents(Node3bb8cE *self)
{
    s32 i;
    Node3bb8cE *cur;

    EnterCriticalSection();
    i = 0;
    cur = self;
    do {
        cur->events[0] = OpenEvent(0xF4000001, D_80086E78[i], 0x2000, 0);
        i++;
        cur = (Node3bb8cE *)((u8 *)cur + 4);
    } while (i < 4);
    ExitCriticalSection();
    TaskObjF__EnableEvents(self);
    return 1;
}

extern void CloseEvent(void);

s32 TaskObjF__CloseEvents(Node3bb8cE *self)
{
    TaskObjF__DisableEvents(self);
    TaskObjF__ForEachEvent(self, CloseEvent, 1);
    return 1;
}

extern s32 TaskObjF__CardInfoAndLoadStatus(Node3bb8cE *self, s32 *p1, s32 *p2, s32 *p3);

s32 TaskObjF__CheckCardStatus(Node3bb8cE *self, s32 *p1, s32 *p2, s32 *p3)
{
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

extern s32 TaskObjF__CardInfoStatus(Node3bb8cE *self, s32 *p1, s32 *p2);
extern s32 TaskObjF__CardLoadStatus(Node3bb8cE *self, s32 *p1, s32 *p2);

s32 TaskObjF__CardInfoAndLoadStatus(Node3bb8cE *self, s32 *p1, s32 *p2, s32 *p3)
{
    if (TaskObjF__CardInfoStatus(self, p1, p2) != 0) {
        TaskObjF__CardLoadStatus(self, p1, p3);
    }
}

extern s32 _card_info(s32 arg0);
extern s32 _card_clear(s32 arg0);

s32 TaskObjF__CardInfoStatus(Node3bb8cE *self, s32 *p1, s32 *p2)
{
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

s32 TaskObjF__CardLoadStatus(Node3bb8cE *self, s32 *p1, s32 *p2)
{
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

s32 TaskObjF__FormatCard(Node3bb8cE *self)
{
    s32 retries;
    s32 result;
    DeviceName866E8 *path;

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
extern s32 TaskObjF__OpenAndReadMemcardFile(Node3bb8cE *self, u8 *destBuf, u8 *suffix);

s32 TaskObjF__ProbeMemcardFile(Node3bb8cE *self, u8 *destBuf, u8 *suffix)
{
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

s32 TaskObjF__OpenAndReadMemcardFile(Node3bb8cE *self, u8 *destBuf, u8 *suffix)
{
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
        strcpy((char *)destBuf, (char *)buf + 4);
        BMemPMgrFree(buf);
    }
    close(handle);
    return 1;
}

char *TaskObjF__FindUnusedMemcardName(Node3bb8cE *self, char *buf, char *middle, char **entries)
{
    while (*entries != NULL) {
        strcpy(buf, middle);
        strcat(buf, *entries);
        if (self->methods->slot54(self, 0, buf) == 0) {
            return buf;
        }
        entries++;
    }
    return NULL;
}

s32 TaskObjF__CollectExistingMemcardFiles(Node3bb8cE *self, s32 *values, char **outArr, char *middle, char **entries)
{
    s32 count;
    char buf[0x20];

    count = 0;
    while (*entries != NULL) {
        strcpy(buf, middle);
        strcat(buf, *entries);
        if (self->methods->slot54(self, *values, buf) != 0) {
            count++;
            *outArr = *entries;
            values++;
            outArr++;
        }
        entries++;
    }
    return count;
}

extern s32 TaskObjF__ProbeCardFreeSpace(Node3bb8cE *self, u8 id, s32 sizeArg);

s32 TaskObjF__CheckCardSpace(Node3bb8cE *self, u8 id, s32 sizeArg)
{
    s32 retries;
    s32 result;

    retries = 10;
    do {
        result = TaskObjF__ProbeCardFreeSpace(self, id, sizeArg);
    } while (result == 0 && retries-- != 0);
    return result;
}

extern s32 delete(void *arg0);

s32 TaskObjF__ProbeCardFreeSpace(Node3bb8cE *self, u8 id, s32 sizeArg)
{
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
    delete(pathBuf);
    return 1;
}
