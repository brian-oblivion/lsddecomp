#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "class_3bb8c.h"
#include "TaskObjF.h"
#include "TimImage.h"

/*
 * class_3bb8c_f: `TaskObjF` methods (include/TaskObjF.h, track 4 round 89),
 * slots +0x064..+0x078 and the +0x038 onNotify override, with the unit's
 * helpers:
 *
 * - The memory-card file API (BuildMemcardPath,
 *   TaskObjF__ReadMemcardFile/TryReadMemcardFile,
 *   TaskObjF__WriteMemcardSaveFile/TryWriteMemcardSaveFile): opens BIOS
 *   `bu00:`/`bu10:` paths directly to read save data and to write a save
 *   file whose header is structurally exact to the standard PS1 memory-
 *   card save format (magic, icon-frame count, block count, title, icon
 *   palette, up to three icon frames).
 * - The event helpers over `events[4]` (EnableEvents/DisableEvents/
 *   TestEvents/ForEachEvent/WaitForReadyEvent).
 * - The two operations the parent starts, TaskObjF__BeginLoad and
 *   TaskObjF__BeginSave, with their card check (TaskObjF__Validate) and the
 *   16-entry title buffer pool (Alloc/FreeUnused/FreeBuffers); TaskObjF__Init
 *   and Deinit; TaskObjF__OnNotify, which routes a child's notification by
 *   the child's class id.
 */

/* Forward declarations: these are defined later in this file (strict
 * ROM-address order), but earlier functions call them. The class's own
 * methods are prototyped in include/TaskObjF.h (track 4, round 89); these
 * two are the unit's plain helpers.
 *
 * `BuildMemcardPath`'s entry here fixes a real Gate-0 warning (round 60):
 * `TaskObjF__TryReadMemcardFile` (line ~52) calls it before its line-226 definition, and
 * without a prototype in scope cc1 implicitly declares it as returning
 * `int`, then complains at the real definition ("type mismatch with
 * previous implicit declaration", "was previously implicitly declared to
 * return `int'"). This is a same-file forward-declaration gap, not a
 * cross-unit signature disagreement -- both the call site and the
 * definition are in this .c, so the fix is simply adding the prototype
 * here like its neighbours. Confirmed byte-identical after the fix
 * (`TaskObjF__TryReadMemcardFile` is already MATCHED and stays MATCHED): a pointer
 * return value lives in `$v0` either way, so the implicit-int reading
 * never produced different code, only a diagnostic. */
s32 WaitForReadyEvent(s32 *arr, s32 count);
char *BuildMemcardPath(McDevicePath *dest, s32 selector, char *suffix);

/* PSX BIOS file trampolines, linked from Sony's own objects since round 34
 * (libapi/a50,a52,a51,a54,a69 -- one 0x10-byte object per stub). These used
 * to live as `func_8005xxxx` prototypes in include/class_3bb8c.h; they are
 * LOCAL here on purpose, because a shared header eleven units include is the
 * wrong place for names this generic, and because class_3bb8c_e.c's view of
 * `open` takes a `void *` where this unit's takes a `char *`. Two local
 * views are legitimate; one shared declaration would not be.
 * These are C89 identifiers under -fno-builtin, nothing else claims them. */
extern s32 open(char *path, s32 mode);             /* B(0x32) */
extern s32 read(s32 handle, void *buf, s32 size);  /* B(0x34) */
extern s32 lseek(s32 handle, s32 pos, s32 whence); /* B(0x33) */
extern s32 close(s32 handle);                      /* B(0x36) */
extern s32 delete (void *path);                    /* B(0x45) */

s32 TaskObjF__ReadMemcardFile(TaskObjF *self, char *suffix, void *outBuf, s32 outSize) {
    s32 count;
    s32 result;

    count = 10;
    do {
        result = TaskObjF__TryReadMemcardFile(self, suffix, outBuf, outSize);
        if (result != 0) {
            break;
        }
    } while (count-- != 0);
    return result;
}

s32 TaskObjF__TryReadMemcardFile(TaskObjF *self, char *suffix, void *outBuf, s32 outSize) {
    char pathBuf[0x20];
    char *path;
    s32 handle;
    void *hdr;
    s32 seekPos;
    u8 raw;

    path = BuildMemcardPath((McDevicePath *)pathBuf, self->cardSlot, suffix);
    handle = open(path, 1);
    if (handle == -1) {
        return 0;
    }
    hdr = BMemPMgrAlloc(0x80);
    read(handle, hdr, 0x80);
    raw = ((u8 *)hdr)[2];
    seekPos = (raw << 7) - 0x780;
    BMemPMgrFree(hdr);
    lseek(handle, seekPos, 0);
    read(handle, outBuf, outSize);
    close(handle);
    return 1;
}

s32 TaskObjF__WriteMemcardSaveFile(TaskObjF *self, char *fileName, char *title, char a3,
                                   struct TimImage *icon, void *data, s32 size) {
    s32 count;
    s32 result;

    count = 10;
    CopyMemcardIconTemplate((s32)title, (s32)fileName);
    do {
        result = TaskObjF__TryWriteMemcardSaveFile(self, fileName, title, a3 & 0xFF, icon, data, size);
        if (result != 0) {
            break;
        }
    } while (count-- != 0);
    if (result == 0) {
        CopyMemcardIconTemplate((s32)title, 0);
    }
    return result;
}

/* TaskObjF__TryWriteMemcardSaveFile -- MATCHED round 75 (see its report).
 * `a3` is a `u8` parameter: the caller's promoted word lives in one
 * register for the `sb` of a3+0x10 and GCC's QImode copy in another for
 * the zero-extended `(a3 << 7)` size, which is retail's `move $s7,$s4`
 * and its late `andi 0xFF`. */
/* TaskObjF__TryWriteMemcardSaveFile's own local types -- none shared
 * elsewhere in this unit.
 *
 * NAMING (round 60): the submitted 0x200-byte buffer is structurally
 * exact to the well-documented PS1 memory-card save FILE HEADER format
 * -- 'S'/'C' magic, an icon-frame-count byte, a block-count byte, a
 * 0x5C title field, a 16-colour icon palette (2 x 8-colour halves,
 * matching this function's own back-to-back-pair copy shape), and up to
 * three 0x80-byte (16x16 4bpp) icon animation frames, for exactly
 * 4+0x5C+0x20+3*0x80 = 0x200 bytes. Named on that structural match, not
 * on any string or symbol table -- Tier B. */

/* Half of the 16-colour icon CLUT (8 x s16 = 0x10 bytes) -- all s16
 * members (alignment 2, no s32) so a whole-struct copy compiles to the
 * unaligned lwl/lwr + swl/swr idiom already documented (Descriptor10 in
 * class_3bb8c.h, Block24 in class_3bb8c_r.c). Two of these sit back to
 * back (0x14..0x33) in the source object and (0x60..0x7F) in the request
 * buffer -- copied as an array of 2, not a loop (matches retail: fully
 * unrolled, no branch, no runtime alignment check). */
typedef struct IconPaletteHalf {
    s16 color[8];
} IconPaletteHalf;

/* One 16x16 4bpp icon animation frame -- a raw, opaque 0x80-byte span
 * (alignment 1, a plain byte array), so a whole-struct copy compiles to
 * the RUNTIME-alignment-checked lw/sw-vs-lwl/lwr dual path retail shows
 * for these three chunks (the same idiom src/class_3bb8c_r.c's Block24
 * documents: "a byte array... compiles the copy as a generic
 * runtime-alignment-checked memcpy loop instead"). Three of these are
 * copied in sequence. */
typedef struct IconFrame {
    u8 raw[0x80];
} IconFrame;

/* The icon TimImage's file buffer (its FileResource `buffer`, +0x010) --
 * only the two palette halves (+0x14/+0x24) and the three icon frames
 * (+0x40/+0xC0/+0x140) are ever read by this function; nothing here
 * establishes the leading 0x14 bytes or the 0xC-byte gap at +0x34 (a TIM's
 * 8-byte header and 12-byte CLUT block header would put the CLUT at +0x14). */
typedef struct McIconSource {
    u8 pad0[0x14];
    IconPaletteHalf palette[2]; /* +0x14 */
    u8 pad34[0x40 - 0x34];
    IconFrame frame0; /* +0x40 */
    IconFrame frame1; /* +0xC0 */
    IconFrame frame2; /* +0x140 */
} McIconSource;

/* The 0x200-byte memory-card save FILE HEADER this function builds and
 * submits -- see the NAMING note above. magic0/magic1 are the literal
 * bytes 'S'/'C'; iconFrameFlag/blockCount are computed size/mode bytes;
 * title is a strcpy target (source: this function's own `title`
 * parameter, TaskObjF::title; typed `char *` in track 4, round 89, where it
 * had been an `s32` named `handle`, byte-identical). */
typedef struct McSaveHeader {
    u8 magic0;
    u8 magic1;
    u8 iconFrameFlag;
    u8 blockCount;
    char title[0x5C];
    IconPaletteHalf palette[2];
    IconFrame frame0;
    IconFrame frame1;
    IconFrame frame2;
} McSaveHeader;

extern const char D_80011530[]; /* rodata string "File not create in WriteFile\n" */
extern s32 write(s32 handle, void *buf,
                 s32 size); /* CD/streaming read-request submit; own local view, not yet declared elsewhere in this project */
extern void printf(const char *fmt); /* own local view: this call site passes only the format string, no variadic args (code_8220.h's 3-arg view is a DIFFERENT call site's shape) */

s32 TaskObjF__TryWriteMemcardSaveFile(TaskObjF *self, char *fileName, char *title, u8 a3,
                                      struct TimImage *icon, void *data, s32 size) {
    char pathBuf[0x20];
    char *path;
    s32 fileHandle;
    s32 openMode;
    McIconSource *src;
    McSaveHeader *req;

    path = BuildMemcardPath((McDevicePath *)pathBuf, self->cardSlot, fileName);
    delete (path);
    openMode = ((((u32)size + 0x21FF) >> 13) << 16) | 0x200;
    fileHandle = open(path, openMode);
    if (fileHandle == -1) {
        printf(D_80011530);
        return 0;
    }
    close(fileHandle);
    fileHandle = open(path, 2);
    if (fileHandle == -1) {
        return 0;
    }
    src = (McIconSource *)icon->buffer;
    req = (McSaveHeader *)BMemPMgrAlloc(0x200);
    req->magic0 = 'S';
    req->magic1 = 'C';
    req->iconFrameFlag = a3 + 0x10;
    req->blockCount = ((u32)size + 0x1FFF) >> 13;
    strcpy(req->title, title);
    req->palette[0] = src->palette[0];
    req->palette[1] = src->palette[1];
    req->frame0 = src->frame0;
    req->frame1 = src->frame1;
    req->frame2 = src->frame2;
    write(fileHandle, req, (a3 << 7) + 0x80);
    BMemPMgrFree(req);
    write(fileHandle, data, (((u32)size + 0x7F) >> 7) << 7);
    close(fileHandle);
    return 1;
}

char *BuildMemcardPath(McDevicePath *dest, s32 selector, char *suffix) {
    McDevicePath *src;

    if (selector) {
        src = &gMcDevicePath1;
    } else {
        src = &gMcDevicePath0;
    }
    *dest = *src;
    strcat((char *)dest, suffix);
    return (char *)dest;
}

/* Psy-Q kernel event queue, linked from libapi/a12, libapi/a13 and
 * libapi/a11. Local view: these are Sony's, declared in the one unit that
 * calls them rather than in include/class_3bb8c.h, which 21 units include.
 * Each takes an event descriptor and returns a status word, which is what
 * makes them usable as TaskObjF__ForEachEvent's `s32 (*)(s32)` callback. */
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

/* Psy-Q's kernel critical-section pair (libapi/a36, libapi/a37, linked from
 * Sony's own SDK objects), called with no arguments around this unit's scan
 * loop when its `flag` argument is set. These belong to another translation
 * unit, so they are declared LOCAL here rather than in class_3bb8c.h, which
 * twenty units include (CLAUDE.md's header-contention rule). Sony's
 * EnterCriticalSection returns int; no call site here reads it, so the local
 * view stays `void` -- per-call-site typing, the convention this block of
 * units already uses. */
extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);

s32 TaskObjF__ForEachEvent(TaskObjF *self, s32 (*callback)(s32), s32 flag) {
    s32 i;
    s32 result;

    if (flag) {
        EnterCriticalSection();
    }
    for (i = 0; i < 4; i++) {
        result = callback(self->events[i]);
        if (result == 0) {
            break;
        }
    }
    if (flag) {
        ExitCriticalSection();
    }
    return result;
}

s32 TaskObjF__WaitForReadyEvent(TaskObjF *self) {
    return WaitForReadyEvent(self->events, 4);
}

s32 WaitForReadyEvent(s32 *arr, s32 count) {
    s32 i;

    for (;;) {
        for (i = 0; i < count; i++) {
            if (TestEvent(arr[i]) != 0) {
                return gCardEventSpecs[i];
            }
        }
    }
}

void TaskObjF__Init(TaskObjF *self, char *namePrefix, char **nameSuffixes, BasicClass *inputSource,
                    BasicClass *tickSource, struct SceneNode *spriteParent, struct VabStreamObj *sound) {
    self->namePrefix = namePrefix;
    self->nameSuffixes = nameSuffixes;
    self->titles = 0;
    self->spriteParent = spriteParent;
    self->sound = sound;
    self->methods->addChild(self, inputSource);
    self->methods->addChild(self, tickSource);
    self->cardIcon = 0;
    self->state = 0;
    self->opMode = 0;
}

void TaskObjF__Deinit(TaskObjF *self) {
    self->sound = 0;
    self->spriteParent = 0;
    self->methods->removeChild(self, self->inputSource);
    self->methods->removeChild(self, self->tickSource);
}

void TaskObjF__BeginLoad(TaskObjF *self, char *fileName, char *title, void *data, s32 size) {
    s32 result;
    s32 code;

    self->fileName = fileName;
    self->title = title;
    self->data = data;
    self->opMode = 1;
    self->dataSize = size;
    if (TaskObjF__Validate(self)) {
        TaskObjF__FreeBuffers(self);
        TaskObjF__AllocBuffers(self);
        result = self->methods->collectExistingMemcardFiles(self, self->titles, self->foundSuffixes,
                                                            self->namePrefix, self->nameSuffixes);
        self->bufCount = result;
        if (result != 0) {
            TaskObjF__FreeUnusedBuffers(self);
            if (self->state == 0xE) {
                code = 0xF;
            } else {
                code = 0x12;
            }
        } else {
            code = 0xD;
            self->bufCount = 0xF;
        }
        self->methods->setState(self, code);
    }
}

void TaskObjF__AllocBuffers(TaskObjF *self) {
    s32 i;

    if (self->titles == 0) {
        self->titles = BMemPMgrAlloc(0x40);
        for (i = 0; i < 15; i++) {
            self->titles[i] = BMemPMgrAlloc(0x41);
        }
        self->foundSuffixes = BMemPMgrAlloc(0x40);
    }
}

void TaskObjF__FreeUnusedBuffers(TaskObjF *self) {
    s32 i;

    for (i = self->bufCount; i < 15; i++) {
        self->titles[i] = BMemPMgrFree(self->titles[i]);
    }
    self->titles[i] = 0;
}

void TaskObjF__FreeBuffers(TaskObjF *self) {
    s32 i;

    if (self->titles != 0) {
        BMemPMgrFree(self->foundSuffixes);
        for (i = 0; i < self->bufCount; i++) {
            BMemPMgrFree(self->titles[i]);
        }
        BMemPMgrFree(self->titles);
        self->titles = 0;
    }
}

/* MATCHED round 75 (docs/match-reports/TaskObjF__BeginSave.md): the
 * function returns nothing -- the early exit falls straight into the
 * epilogue with Validate's own $v0 -- and each of the three leaves makes its
 * own setState call, which GCC cross-jumps down to one shared `jalr`. */
void TaskObjF__BeginSave(TaskObjF *self, char *fileName, char *title, s32 titleEditPos,
                         u8 iconFrames, struct TimImage *icon, void *data, s32 size) {
    s32 code;
    TaskObjFMethods *m;

    self->fileName = fileName;
    self->title = title;
    self->titleEditPos = titleEditPos;
    self->opMode = 2;
    self->iconFrames = iconFrames;
    self->iconImage = icon;
    self->data = data;
    self->dataSize = size;
    if (TaskObjF__Validate(self)) {
        if (self->methods->probeMemcardFile(self, 0, fileName) != 0) {
            code = 0xA;
            if (self->state == code) {
                code = 0x11;
            } else if (self->state == 0x11) {
                code = 0xB;
            }
            self->methods->setState(self, code);
        } else if (!self->methods->checkCardSpace(self, iconFrames, size)) {
            self->methods->setState(self, 9);
        } else {
            m = self->methods;
            code = 0x11;
            if (self->state == code) {
                code = 0xB;
            }
            m->setState(self, code);
        }
    }
}

s32 TaskObjF__Validate(TaskObjF *self) {
    s32 buf10;
    s32 buf14;
    s32 buf18;
    s32 slot4CRet;
    s32 code;

    self->methods->openEvents(self);
    slot4CRet = self->methods->checkCardStatus(self, &buf10, &buf14, &buf18);
    self->methods->closeEvents(self);

    if (slot4CRet != 0) {
        if (buf14 == 0 && buf18 != 0) {
            return 1;
        }
    }

    if (slot4CRet == 0) {
        code = 2;
    } else if (buf10 != 0) {
        code = 3;
    } else if (buf14 != 0) {
        code = 4;
    } else if (buf18 != 0) {
        goto dispatch;
    } else if (self->opMode == 1) {
        code = 5;
    } else {
        code = 6;
    }

dispatch:
    self->methods->setState(self, code);
    return 0;
}

void TaskObjF__OnNotify(TaskObjF *self, void *arg1, s32 arg2) {
    TaskObjFMethods *methods;
    BasicClassMethods *bm;
    s32 tag;
    s32 mask;

    methods = self->methods;
    bm = Get_vtable_BasicClass();
    bm->onNotify((BasicClass *)self, arg1, arg2);

    tag = **(s32 **)arg1;
    mask = tag & 0xF;
    if (mask == 2) {
        methods->onInputEvent(self, arg1, arg2);
    } else if (mask == 5) {
        methods->tickStateDelay(self, arg1, arg2);
    } else {
        mask = tag & 0xFF;
        if (mask == 0x10) {
            methods->onTextEntryResult(self, arg1, arg2);
        } else if (mask == 0x20) {
            methods->onItemListResult(self, arg1, arg2);
        }
    }
}
