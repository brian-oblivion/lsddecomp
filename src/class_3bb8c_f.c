#include "common.h"
#include "class_3bb8c.h"

/*
 * class_3bb8c_f: `TaskObjF`, a `BasicClass` subclass. Two mostly
 * independent halves share the object:
 *
 * - A direct memory-card file API (BuildMemcardPath,
 *   TaskObjF__ReadMemcardFile/TryReadMemcardFile,
 *   TaskObjF__WriteMemcardSaveFile/TryWriteMemcardSaveFile): opens BIOS
 *   `bu00:`/`bu10:` paths directly to read save data and to write a save
 *   file whose header is structurally exact to the standard PS1 memory-
 *   card save format (magic, icon-frame count, block count, title, icon
 *   palette, up to three icon frames).
 * - A generic async-task skeleton (TaskObjF__Init/Deinit,
 *   TaskObjF__AllocBuffers/FreeBuffers/FreeUnusedBuffers,
 *   TaskObjF__Validate, TaskObjF__Notify, TaskObjF__func_8004F638/
 *   func_8004F8A4) managing a 16-entry buffer pool and dispatching the
 *   actual work through vtable slots a subclass outside this unit
 *   implements -- what those two entry points DO is not established
 *   here (see their own reports' Tier-C naming).
 * - `TaskObjF::events[4]` is corroborated cross-unit: class_3bb8c_e.c's
 *   func_8004E5E4 populates the identical +0x014 offset via OpenEvent()
 *   and hands the same object to this unit's own TaskObjF__EnableEvents.
 */

/* Forward declarations: these are defined later in this file (strict
 * ROM-address order), but earlier functions call them.
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
s32 TaskObjF__Validate(TaskObjF *self);
void TaskObjF__AllocBuffers(TaskObjF *self);
void TaskObjF__FreeUnusedBuffers(TaskObjF *self);
void TaskObjF__FreeBuffers(TaskObjF *self);
s32 TaskObjF__ForEachEvent(TaskObjF *self, s32 (*callback)(s32), s32 flag);
s32 WaitForReadyEvent(s32 *arr, s32 count);
s32 TaskObjF__TryReadMemcardFile(TaskObjF *self, char *suffix, void *outBuf, s32 outSize);
s32 TaskObjF__TryWriteMemcardSaveFile(TaskObjF *self, s32 a1, s32 handle, s32 a3, s32 arg5, s32 arg6, s32 arg7);
char *BuildMemcardPath(DeviceName866E8 *dest, s32 selector, char *suffix);

/* PSX BIOS file trampolines, linked from Sony's own objects since round 34
 * (libapi/a50,a52,a51,a54,a69 -- one 0x10-byte object per stub). These used
 * to live as `func_8005xxxx` prototypes in include/class_3bb8c.h; they are
 * LOCAL here on purpose, because a shared header eleven units include is the
 * wrong place for names this generic, and because class_3bb8c_e.c's view of
 * `open` takes a `void *` where this unit's takes a `char *`. Two local
 * views are legitimate; one shared declaration would not be.
 * These are C89 identifiers under -fno-builtin, nothing else claims them. */
extern s32 open(char *path, s32 mode);            /* B(0x32) */
extern s32 read(s32 handle, void *buf, s32 size); /* B(0x34) */
extern s32 lseek(s32 handle, s32 pos, s32 whence);/* B(0x33) */
extern s32 close(s32 handle);                     /* B(0x36) */
extern s32 delete(void *path);                    /* B(0x45) */

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

    path = BuildMemcardPath((DeviceName866E8 *)pathBuf, self->cardSlot, suffix);
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

s32 TaskObjF__WriteMemcardSaveFile(TaskObjF *self, s32 a1, s32 handle, char a3, s32 arg5, s32 arg6, s32 arg7) {
    s32 count;
    s32 result;

    count = 10;
    CopyMemcardIconTemplate(handle, a1);
    do {
        result = TaskObjF__TryWriteMemcardSaveFile(self, a1, handle, a3 & 0xFF, arg5, arg6, arg7);
        if (result != 0) {
            break;
        }
    } while (count-- != 0);
    if (result == 0) {
        CopyMemcardIconTemplate(handle, 0);
    }
    return result;
}

/* STALL snapshot -- see docs/match-reports/TaskObjF__TryWriteMemcardSaveFile.md. Best
 * reached: 188/240 words, 0x3BC/0x3C0 (1 word / 4 bytes short, zero
 * address drift beyond that). Genuinely fresh derivation this round --
 * full struct layout (McIconSource/McSaveHeader/etc, all new) derived
 * from scratch and confirmed correct in control flow, arithmetic
 * (including two signed/unsigned shift fixes -- retail uses `srl`, a
 * naive signed `>>` on `s32 arg7` compiles to `sra`), and struct-copy
 * shape (the aligned-vs-unaligned runtime-checked copy idiom, matching
 * class_3bb8c_r.c's Block24 precedent). The residue is confirmed as
 * the SAME 9-register-saturation class already documented for this
 * function's own caller, TaskObjF__WriteMemcardSaveFile -- a single delay-slot
 * scheduling choice (which of two independent register-materializing
 * moves fills a branch's delay slot) that did not respond to any
 * position/type/declaration-order variant tried. Preserved here per
 * convention -- not live C. */
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

/* arg5->iconSource's pointee -- only the two palette halves (+0x14/+0x24)
 * and the three icon frames (+0x40/+0xC0/+0x140) are ever read by this
 * function; nothing establishes the leading 0x14 bytes or the 0xC-byte
 * gap at +0x34. */
typedef struct McIconSource {
    u8 pad0[0x14];
    IconPaletteHalf palette[2];  /* +0x14 */
    u8 pad34[0x40 - 0x34];
    IconFrame frame0;              /* +0x40 */
    IconFrame frame1;                /* +0xC0 */
    IconFrame frame2;                  /* +0x140 */
} McIconSource;

/* arg5's own type -- only +0x10 (a McIconSource*) is ever read. */
typedef struct McIconSourceRef {
    u8 pad0[0x10];
    McIconSource *iconSource;
} McIconSourceRef;

/* The 0x200-byte memory-card save FILE HEADER this function builds and
 * submits -- see the NAMING note above. magic0/magic1 are the literal
 * bytes 'S'/'C'; iconFrameFlag/blockCount are computed size/mode bytes;
 * title is a strcpy target (source: this function's own `handle`
 * parameter, which despite its established `s32` type across this file
 * is used here as a raw C string -- kept `s32` at the parameter per this
 * project's per-site-cast convention, since retyping it risks the
 * ALREADY-MATCHED TaskObjF__WriteMemcardSaveFile's own signature). */
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

extern const char D_80011530[];  /* rodata string "File not create in WriteFile\n" */
extern s32 write(s32 handle, void *buf, s32 size);  /* CD/streaming read-request submit; own local view, not yet declared elsewhere in this project */
extern void printf(const char *fmt);  /* own local view: this call site passes only the format string, no variadic args (code_8220.h's 3-arg view is a DIFFERENT call site's shape) */

#if 0
/* ROUND 37 (delta): re-verified 239/240 (1 word short, 0x3BC/0x3C0), then
 * re-measured raw word-match at 191/240 (previously recorded as 188/240 --
 * the 3-word difference is not a regression, just the first re-measurement
 * since round 34 relinked the BIOS trampolines this body calls through;
 * `asm-differ` confirms the residue is IDENTICAL in kind and location to
 * the one this report already documents: the whole callee-saved register
 * set permuted relative to retail's own, plus the single missing
 * `sw $s0,0x30($sp)` / delay-slot `move $s7,$s4` at file 0x3F7A4/0x3F7F0).
 * Seeded a permuter search (never run before this round) -- see the report
 * for iteration count and result. Restored here, not left live -- see the
 * report. */
s32 TaskObjF__TryWriteMemcardSaveFile(TaskObjF *self, s32 a1, s32 handle, s32 a3, s32 arg5, s32 arg6, s32 arg7) {
    char pathBuf[0x20];
    char *path;
    s32 fileHandle;
    s32 openMode;
    s32 flagCopy;
    s32 payload;
    McIconSource *src;
    McSaveHeader *req;

    payload = arg6;
    path = BuildMemcardPath((DeviceName866E8 *)pathBuf, self->cardSlot, (char *)a1);
    delete(path);
    openMode = ((((u32)arg7 + 0x21FF) >> 13) << 16) | 0x200;
    fileHandle = open(path, openMode);
    flagCopy = a3;
    if (fileHandle == -1) {
        printf(D_80011530);
        return 0;
    }
    close(fileHandle);
    fileHandle = open(path, 2);
    if (fileHandle == -1) {
        return 0;
    }
    src = ((McIconSourceRef *)arg5)->iconSource;
    req = (McSaveHeader *)BMemPMgrAlloc(0x200);
    req->magic0 = 'S';
    req->magic1 = 'C';
    req->iconFrameFlag = a3 + 0x10;
    req->blockCount = ((u32)arg7 + 0x1FFF) >> 13;
    strcpy(req->title, (char *)handle);
    req->palette[0] = src->palette[0];
    req->palette[1] = src->palette[1];
    req->frame0 = src->frame0;
    req->frame1 = src->frame1;
    req->frame2 = src->frame2;
    write(fileHandle, req, (((flagCopy & 0xFF) << 7)) + 0x80);
    BMemPMgrFree(req);
    write(fileHandle, (void *)payload, (((u32)arg7 + 0x7F) >> 7) << 7);
    close(fileHandle);
    return 1;
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", TaskObjF__TryWriteMemcardSaveFile);


char *BuildMemcardPath(DeviceName866E8 *dest, s32 selector, char *suffix) {
    DeviceName866E8 *src;

    if (selector) {
        src = &D_8008AA9C;
    } else {
        src = &D_8008AAA4;
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
                return D_80086E78[i];
            }
        }
    }
}

void TaskObjF__Init(TaskObjF *self, s32 a1, s32 a2, s32 a3, s32 a5, s32 a6, s32 a7) {
    self->unk30 = a1;
    self->unk34 = a2;
    self->bufArray = 0;
    self->unk68 = a6;
    self->unk6C = a7;
    self->methods->addChild(self, (void *)a3);
    self->methods->addChild(self, (void *)a5);
    self->unk70 = 0;
    self->statusCode = 0;
    self->opMode = 0;
}

void TaskObjF__Deinit(TaskObjF *self) {
    self->unk6C = 0;
    self->unk68 = 0;
    self->methods->removeChild(self, (void *)self->unk60);
    self->methods->removeChild(self, (void *)self->unk64);
}

void TaskObjF__func_8004F638(TaskObjF *self, s32 a1, s32 a2, s32 a3, s32 a4) {
    s32 result;
    s32 code;

    self->unk40 = a1;
    self->unk44 = a2;
    self->unk54 = a3;
    self->opMode = 1;
    self->unk58 = a4;
    if (TaskObjF__Validate(self)) {
        TaskObjF__FreeBuffers(self);
        TaskObjF__AllocBuffers(self);
        result = self->methods->slot5C(self, self->bufArray, self->scratchBuf, self->unk30, self->unk34);
        self->bufCount = result;
        if (result != 0) {
            TaskObjF__FreeUnusedBuffers(self);
            if (self->statusCode == 0xE) {
                code = 0xF;
            } else {
                code = 0x12;
            }
        } else {
            code = 0xD;
            self->bufCount = 0xF;
        }
        self->methods->slot7C(self, code);
    }
}

void TaskObjF__AllocBuffers(TaskObjF *self) {
    s32 i;

    if (self->bufArray == 0) {
        self->bufArray = BMemPMgrAlloc(0x40);
        for (i = 0; i < 15; i++) {
            self->bufArray[i] = BMemPMgrAlloc(0x41);
        }
        self->scratchBuf = BMemPMgrAlloc(0x40);
    }
}

void TaskObjF__FreeUnusedBuffers(TaskObjF *self) {
    s32 i;

    for (i = self->bufCount; i < 15; i++) {
        self->bufArray[i] = BMemPMgrFree(self->bufArray[i]);
    }
    self->bufArray[i] = 0;
}

void TaskObjF__FreeBuffers(TaskObjF *self) {
    s32 i;

    if (self->bufArray != 0) {
        BMemPMgrFree(self->scratchBuf);
        for (i = 0; i < self->bufCount; i++) {
            BMemPMgrFree(self->bufArray[i]);
        }
        BMemPMgrFree(self->bufArray);
        self->bufArray = 0;
    }
}

#ifdef NON_MATCHING
/* NON_MATCHING: 63/77 words, 0x140/0x134 (3 words / 12 bytes too long).
 * Residue: early-exit tail re-materialization -- retail reuses
 * `TaskObjF__Validate`'s own false(0) return value directly as the
 * function's return with zero extra instructions, but this build always
 * re-materializes an explicit `v0=0` plus a skip-jump around it, whether
 * the C returns a literal `0` or a captured variable holding the same
 * value (docs/match-reports/TaskObjF__func_8004F8A4.md). Hand-derived:
 * caching `self->methods` into a local `TaskObjFMethods *m` right where
 * retail does (in the `slot60`-returned-nonzero tail, before its own
 * 2-way `statusCode` check) reproduces retail's whole dispatch structure
 * byte-for-byte up through the final shared `jalr`; only the early-exit
 * tail remains unmatched. */
s32 TaskObjF__func_8004F8A4(TaskObjF *self, s32 a1, s32 a2, s32 a3, u8 a5, s32 a6, s32 a7, s32 a8) {
    s32 code;
    s32 (*dispatch)(TaskObjF *, s32);

    self->unk40 = a1;
    self->unk44 = a2;
    self->unk48 = a3;
    self->opMode = 2;
    self->unk4C = a5;
    self->unk50 = a6;
    self->unk54 = a7;
    self->unk58 = a8;
    if (TaskObjF__Validate(self)) {
        if (self->methods->slot54(self, 0, a1) == 0) {
            goto slot60_path;
        }
        code = 0xA;
        if (self->statusCode == code) {
            code = 0x11;
        } else if (self->statusCode == 0x11) {
            code = 0xB;
        }

    top_dispatch:
        dispatch = self->methods->slot7C;
        goto call_it;

    slot60_path:
        if (!self->methods->slot60(self, a5, a8)) {
            code = 9;
            dispatch = self->methods->slot7C;
            goto call_it;
        }
        {
            TaskObjFMethods *m = self->methods;
            code = 0x11;
            if (self->statusCode == code) {
                code = 0xB;
            }
            dispatch = m->slot7C;
        }

    call_it:
        return dispatch(self, code);
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", TaskObjF__func_8004F8A4);
#endif


s32 TaskObjF__Validate(TaskObjF *self) {
    s32 buf10;
    s32 buf14;
    s32 buf18;
    s32 slot4CRet;
    s32 code;

    self->methods->slot44(self);
    slot4CRet = self->methods->slot4C(self, &buf10, &buf14, &buf18);
    self->methods->slot48(self);

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
    self->methods->slot7C(self, code);
    return 0;
}

void TaskObjF__Notify(TaskObjF *self, void *arg1, s32 arg2) {
    TaskObjFMethods *methods;
    BasicMethods866E8F *bm;
    s32 tag;
    s32 mask;

    methods = self->methods;
    bm = Get_vtable_BasicClass();
    bm->slot38(self, arg1, arg2);

    tag = **(s32 **)arg1;
    mask = tag & 0xF;
    if (mask == 2) {
        methods->slot88(self, arg1, arg2);
    } else if (mask == 5) {
        methods->slot98(self, arg1, arg2);
    } else {
        mask = tag & 0xFF;
        if (mask == 0x10) {
            methods->slotA4(self, arg1, arg2);
        } else if (mask == 0x20) {
            methods->slotB0(self, arg1, arg2);
        }
    }
}
