#include "common.h"
#include "class_3bb8c.h"

/* Forward declarations: these are defined later in this file (strict
 * ROM-address order), but func_8004F638/func_8004F8A4 (defined earlier)
 * call them. */
s32 func_8004F9D8(TaskObjF *self);
void func_8004F704(TaskObjF *self);
void func_8004F784(TaskObjF *self);
void func_8004F810(TaskObjF *self);
s32 func_8004F40C(TaskObjF *self, s32 (*callback)(s32), s32 flag);
s32 func_8004F4C8(s32 *arr, s32 count);
s32 func_8004EDC0(TaskObjF *self, char *suffix, void *outBuf, s32 outSize);
s32 func_8004EF6C(TaskObjF *self, s32 a1, s32 handle, s32 a3, s32 arg5, s32 arg6, s32 arg7);

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

s32 func_8004ED40(TaskObjF *self, char *suffix, void *outBuf, s32 outSize) {
    s32 count;
    s32 result;

    count = 10;
    do {
        result = func_8004EDC0(self, suffix, outBuf, outSize);
        if (result != 0) {
            break;
        }
    } while (count-- != 0);
    return result;
}

s32 func_8004EDC0(TaskObjF *self, char *suffix, void *outBuf, s32 outSize) {
    char pathBuf[0x20];
    char *path;
    s32 handle;
    void *hdr;
    s32 seekPos;
    u8 raw;

    path = func_8004F32C((DeviceName866E8 *)pathBuf, self->unk0C, suffix);
    handle = open(path, 1);
    if (handle == -1) {
        return 0;
    }
    hdr = func_80017B34(0x80);
    read(handle, hdr, 0x80);
    raw = ((u8 *)hdr)[2];
    seekPos = (raw << 7) - 0x780;
    func_80017CFC(hdr);
    lseek(handle, seekPos, 0);
    read(handle, outBuf, outSize);
    close(handle);
    return 1;
}

s32 func_8004EEA0(TaskObjF *self, s32 a1, s32 handle, char a3, s32 arg5, s32 arg6, s32 arg7) {
    s32 count;
    s32 result;

    count = 10;
    func_800507F8(handle, a1);
    do {
        result = func_8004EF6C(self, a1, handle, a3 & 0xFF, arg5, arg6, arg7);
        if (result != 0) {
            break;
        }
    } while (count-- != 0);
    if (result == 0) {
        func_800507F8(handle, 0);
    }
    return result;
}

/* STALL snapshot -- see docs/match-reports/func_8004EF6C.md. Best
 * reached: 188/240 words, 0x3BC/0x3C0 (1 word / 4 bytes short, zero
 * address drift beyond that). Genuinely fresh derivation this round --
 * full struct layout (StreamSrcObj/StreamReq/etc, all new) derived from
 * scratch and confirmed correct in control flow, arithmetic (including
 * two signed/unsigned shift fixes -- retail uses `srl`, a naive signed
 * `>>` on `s32 arg7` compiles to `sra`), and struct-copy shape (the
 * aligned-vs-unaligned runtime-checked copy idiom, matching
 * class_3bb8c_r.c's Block24 precedent). The residue is confirmed as
 * the SAME 9-register-saturation class already documented for this
 * function's own caller, func_8004EEA0 -- a single delay-slot
 * scheduling choice (which of two independent register-materializing
 * moves fills a branch's delay slot) that did not respond to any
 * position/type/declaration-order variant tried. Preserved here per
 * convention -- not live C. */
#if 0
/* func_8004EF6C's own local types -- none shared elsewhere in this unit. */

/* A small opaque sub-record, read/written as a whole -- all s16 members
 * (alignment 2, no s32) so a whole-struct copy compiles to the unaligned
 * lwl/lwr + swl/swr idiom already documented (Descriptor10 in
 * class_3bb8c.h, Block24 in class_3bb8c_r.c). Two of these sit back to
 * back (0x14..0x33) in the source object and (0x60..0x7F) in the request
 * buffer -- copied as an array of 2, not a loop (matches retail: fully
 * unrolled, no branch, no runtime alignment check). */
typedef struct StreamSmallSub {
    s16 f0, f2, f4, f6, f8, fA, fC, fE;
} StreamSmallSub;

/* A raw, opaque 0x80-byte span -- alignment 1 (a plain byte array), so a
 * whole-struct copy compiles to the RUNTIME-alignment-checked
 * lw/sw-vs-lwl/lwr dual path retail shows for these three chunks (the
 * same idiom src/class_3bb8c_r.c's Block24 documents: "a byte array...
 * compiles the copy as a generic runtime-alignment-checked memcpy loop
 * instead"). Three of these are copied in sequence. */
typedef struct StreamRawBlock {
    u8 raw[0x80];
} StreamRawBlock;

/* arg5->unk10's pointee -- only the two small subs (+0x14/+0x24) and the
 * three raw 0x80-byte spans (+0x40/+0xC0/+0x140) are ever read by this
 * function; nothing establishes the leading 0x14 bytes or the 0xC-byte
 * gap at +0x34. */
typedef struct StreamSrcObj {
    u8 pad0[0x14];
    StreamSmallSub arr[2];      /* +0x14 */
    u8 pad34[0x40 - 0x34];
    StreamRawBlock blkA;          /* +0x40 */
    StreamRawBlock blkB;            /* +0xC0 */
    StreamRawBlock blkC;              /* +0x140 */
} StreamSrcObj;

/* arg5's own type -- only +0x10 (a StreamSrcObj*) is ever read. */
typedef struct StreamArg5Obj {
    u8 pad0[0x10];
    StreamSrcObj *unk10;
} StreamArg5Obj;

/* The 0x200-byte request buffer this function builds and submits.
 * tag0/tag1 are the literal bytes 'S'/'C'; b2/b3 are computed size/mode
 * bytes; name is a strcpy target (source: this function's own `handle`
 * parameter, which despite its established `s32` type across this file
 * is used here as a raw C string -- kept `s32` at the parameter per this
 * project's per-site-cast convention, since retyping it risks the
 * ALREADY-MATCHED func_8004EEA0's own signature). */
typedef struct StreamReq {
    u8 tag0;
    u8 tag1;
    u8 b2;
    u8 b3;
    char name[0x5C];
    StreamSmallSub arr[2];
    StreamRawBlock blkA;
    StreamRawBlock blkB;
    StreamRawBlock blkC;
} StreamReq;

extern const char D_80011530[];  /* rodata string "File not create in WriteFile\n" */
extern s32 write(s32 handle, void *buf, s32 size);  /* CD/streaming read-request submit; own local view, not yet declared elsewhere in this project */
extern void printf(const char *fmt);  /* own local view: this call site passes only the format string, no variadic args (code_8220.h's 3-arg view is a DIFFERENT call site's shape) */

s32 func_8004EF6C(TaskObjF *self, s32 a1, s32 handle, s32 a3, s32 arg5, s32 arg6, s32 arg7) {
    char pathBuf[0x20];
    char *path;
    s32 fileHandle;
    s32 openMode;
    s32 flagCopy;
    s32 payload;
    StreamSrcObj *src;
    StreamReq *req;

    payload = arg6;
    path = func_8004F32C((DeviceName866E8 *)pathBuf, self->unk0C, (char *)a1);
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
    src = ((StreamArg5Obj *)arg5)->unk10;
    req = (StreamReq *)func_80017B34(0x200);
    req->tag0 = 'S';
    req->tag1 = 'C';
    req->b2 = a3 + 0x10;
    req->b3 = ((u32)arg7 + 0x1FFF) >> 13;
    strcpy(req->name, (char *)handle);
    req->arr[0] = src->arr[0];
    req->arr[1] = src->arr[1];
    req->blkA = src->blkA;
    req->blkB = src->blkB;
    req->blkC = src->blkC;
    write(fileHandle, req, (((flagCopy & 0xFF) << 7)) + 0x80);
    func_80017CFC(req);
    write(fileHandle, (void *)payload, (((u32)arg7 + 0x7F) >> 7) << 7);
    close(fileHandle);
    return 1;
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004EF6C);


char *func_8004F32C(DeviceName866E8 *dest, s32 selector, char *suffix) {
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
 * makes them usable as func_8004F40C's `s32 (*)(s32)` callback. */
extern s32 EnableEvent(s32 event);
extern s32 DisableEvent(s32 event);
extern s32 TestEvent(s32 event);

s32 func_8004F394(TaskObjF *self) {
    return func_8004F40C(self, EnableEvent, 1);
}

s32 func_8004F3BC(TaskObjF *self) {
    return func_8004F40C(self, DisableEvent, 1);
}

s32 func_8004F3E4(TaskObjF *self) {
    return func_8004F40C(self, TestEvent, 0);
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

s32 func_8004F40C(TaskObjF *self, s32 (*callback)(s32), s32 flag) {
    s32 i;
    s32 result;

    if (flag) {
        EnterCriticalSection();
    }
    for (i = 0; i < 4; i++) {
        result = callback(self->field14[i]);
        if (result == 0) {
            break;
        }
    }
    if (flag) {
        ExitCriticalSection();
    }
    return result;
}

s32 func_8004F4A4(TaskObjF *self) {
    return func_8004F4C8(self->field14, 4);
}

s32 func_8004F4C8(s32 *arr, s32 count) {
    s32 i;

    for (;;) {
        for (i = 0; i < count; i++) {
            if (TestEvent(arr[i]) != 0) {
                return D_80086E78[i];
            }
        }
    }
}

void func_8004F55C(TaskObjF *self, s32 a1, s32 a2, s32 a3, s32 a5, s32 a6, s32 a7) {
    self->unk30 = a1;
    self->unk34 = a2;
    self->unk38 = 0;
    self->unk68 = a6;
    self->unk6C = a7;
    self->methods->addChild(self, (void *)a3);
    self->methods->addChild(self, (void *)a5);
    self->unk70 = 0;
    self->unk28 = 0;
    self->unk24 = 0;
}

void func_8004F5DC(TaskObjF *self) {
    self->unk6C = 0;
    self->unk68 = 0;
    self->methods->removeChild(self, (void *)self->unk60);
    self->methods->removeChild(self, (void *)self->unk64);
}

void func_8004F638(TaskObjF *self, s32 a1, s32 a2, s32 a3, s32 a4) {
    s32 result;
    s32 code;

    self->unk40 = a1;
    self->unk44 = a2;
    self->unk54 = a3;
    self->unk24 = 1;
    self->unk58 = a4;
    if (func_8004F9D8(self)) {
        func_8004F810(self);
        func_8004F704(self);
        result = self->methods->slot5C(self, self->unk38, self->unk3C, self->unk30, self->unk34);
        self->unk2C = result;
        if (result != 0) {
            func_8004F784(self);
            if (self->unk28 == 0xE) {
                code = 0xF;
            } else {
                code = 0x12;
            }
        } else {
            code = 0xD;
            self->unk2C = 0xF;
        }
        self->methods->slot7C(self, code);
    }
}

void func_8004F704(TaskObjF *self) {
    s32 i;

    if (self->unk38 == 0) {
        self->unk38 = func_80017B34(0x40);
        for (i = 0; i < 15; i++) {
            self->unk38[i] = func_80017B34(0x41);
        }
        self->unk3C = func_80017B34(0x40);
    }
}

void func_8004F784(TaskObjF *self) {
    s32 i;

    for (i = self->unk2C; i < 15; i++) {
        self->unk38[i] = func_80017CFC(self->unk38[i]);
    }
    self->unk38[i] = 0;
}

void func_8004F810(TaskObjF *self) {
    s32 i;

    if (self->unk38 != 0) {
        func_80017CFC(self->unk3C);
        for (i = 0; i < self->unk2C; i++) {
            func_80017CFC(self->unk38[i]);
        }
        func_80017CFC(self->unk38);
        self->unk38 = 0;
    }
}

#if 0
/* STALL snapshot -- see docs/match-reports/func_8004F8A4.md. Best reached
 * this round (delta): 63/77 words, 0x140/0x134 (3 words / 12 bytes TOO
 * LONG, address drift beyond that -- opposite direction from the previous
 * 36/77-at-1-word-short best). Caching `self->methods` into a local
 * `TaskObjFMethods *m` right where retail does (in the `slot60`-returned-
 * nonzero tail, before its own 2-way `unk28` check) took this from 36/77
 * to 63/77 -- reproduces retail's whole body byte-for-byte up through the
 * final shared `jalr`. The residue is now isolated entirely to the
 * function's OWN early-exit tail (see report): retail reuses
 * `func_8004F9D8`'s own false(0) return value directly as the function's
 * return with zero extra instructions, but this build always
 * re-materializes an explicit `v0=0` plus a skip-jump around it,
 * regardless of whether the C returns a literal `0` or a captured
 * variable holding the same value. Preserved here per convention -- not
 * live C. */
s32 func_8004F8A4(TaskObjF *self, s32 a1, s32 a2, s32 a3, u8 a5, s32 a6, s32 a7, s32 a8) {
    s32 code;
    s32 (*dispatch)(TaskObjF *, s32);

    self->unk40 = a1;
    self->unk44 = a2;
    self->unk48 = a3;
    self->unk24 = 2;
    self->unk4C = a5;
    self->unk50 = a6;
    self->unk54 = a7;
    self->unk58 = a8;
    if (func_8004F9D8(self)) {
        if (self->methods->slot54(self, 0, a1) == 0) {
            goto slot60_path;
        }
        code = 0xA;
        if (self->unk28 == code) {
            code = 0x11;
        } else if (self->unk28 == 0x11) {
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
            if (self->unk28 == code) {
                code = 0xB;
            }
            dispatch = m->slot7C;
        }

    call_it:
        return dispatch(self, code);
    }
    return 0;
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_f", func_8004F8A4);


s32 func_8004F9D8(TaskObjF *self) {
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
    } else if (self->unk24 == 1) {
        code = 5;
    } else {
        code = 6;
    }

dispatch:
    self->methods->slot7C(self, code);
    return 0;
}

void func_8004FB04(TaskObjF *self, void *arg1, s32 arg2) {
    TaskObjFMethods *methods;
    BasicMethods866E8F *bm;
    s32 tag;
    s32 mask;

    methods = self->methods;
    bm = func_80018390();
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
