#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "class_3bb8c.h"
#include "TaskObjF.h"
#include "TimImage.h"
#include <sys/file.h>

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
s32 WaitForReadyEvent(s32 *events, s32 count);
char *BuildMemcardPath(McDevicePath *dest, s32 cardSlot, char *suffix);

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
 * bytes 'S'/'C'; iconDisplayFlag/blockCount are computed size/mode bytes;
 * title is a strcpy target (source: this function's own `title`
 * parameter, TaskObjF::title; typed `char *` in track 4, round 89, where it
 * had been an `s32` named `handle`, byte-identical). */
typedef struct McSaveHeader {
    u8 magic0;
    u8 magic1;
    u8 iconDisplayFlag;
    u8 blockCount;
    char title[0x5C];
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
    CopyMemcardIconTemplate((s32)title, (s32)fileName);
    do {
        result = TaskObjF__TryWriteMemcardSaveFile(self, fileName, title, iconFrames & 0xFF, icon,
                                                   data, size);
        if (result != 0) {
            break;
        }
    } while (retries-- != 0);
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
extern const char sFileNotCreatedMsg[]; /* rodata string "File not create in WriteFile\n" */
extern s32 write(s32 handle, void *buf,
                 s32 size); /* CD/streaming read-request submit; own local view, not yet declared elsewhere in this project */
extern void printf(const char *fmt); /* own local view: this call site passes only the format string, no variadic args (code_8220.h's 3-arg view is a DIFFERENT call site's shape) */

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
        self->titles = 0;
    }
}

/* MATCHED round 75 (docs/match-reports/TaskObjF__BeginSave.md): the
 * function returns nothing -- the early exit falls straight into the
 * epilogue with Validate's own $v0 -- and each of the three leaves makes its
 * own setState call, which GCC cross-jumps down to one shared `jalr`. */
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
        if (self->methods->probeMemcardFile(self, 0, fileName) != 0) {
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

s32 TaskObjF__Validate(TaskObjF *self) {
    s32 error;
    s32 cardChanged;
    s32 formatted;
    s32 responded;
    s32 state;

    self->methods->openEvents(self);
    responded = self->methods->checkCardStatus(self, &error, &cardChanged, &formatted);
    self->methods->closeEvents(self);

    if (responded != 0) {
        if (cardChanged == 0 && formatted != 0) {
            return 1;
        }
    }

    if (responded == 0) {
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
