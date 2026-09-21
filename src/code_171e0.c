/*
 * code_171e0 -- Class6D430's own module, plus an active-data-source
 * dispatch layer built on top of it.
 *
 * Class6D430 (Class6D430/Class6D430Methods) is a small
 * BasicClass subclass holding one lazily-(re)allocated buffer (buffer/bufferSize,
 * managed by Class6D430__AllocBuffer/FreeBuffer) plus a flags word.
 * D_8006D4E8 (the CD-ROM read driver, code_179d8_q.c) shares its own
 * +0x004/+0x05C/+0x060/+0x064 slots with it verbatim (DestroyChained/
 * Class6D430__FreeBuffer/NoOp/Class6D430__SetFlag), so this is a real shared
 * base for at least that sibling, and plausibly for gVabDriverMethods too (the
 * SPU/VAB streamer, code_179d8_e.c).
 *
 * Most of this unit's remaining functions dispatch between those same two
 * sibling classes by `gActiveDataSource` (DATASOURCE_CD/DATASOURCE_SPU,
 * their own header words): Lock/UnlockActiveDataSource,
 * IsActiveDataSourceBusy/Idle, GetActiveDataSourceOperation/State/
 * DriverMode/Methods/UseVSyncCallback, SetActiveDataSourceDriverMode,
 * RegisterFileTableEntries and the stalled SetActiveDataSource all forward
 * to the CD driver's own functions when it is active, and to an SPU/VAB-
 * side fallback otherwise.
 *
 * GetClass6D3C8Methods, SetVec3 and BuildFileName are unrelated utilities
 * that happen to live in this segment; func_800270AC/func_800270B8 (a
 * getter/setter pair for D_8008A854) are left unnamed -- see their reports.
 */
#include "common.h"
#include "code_171e0.h"

/* gActiveDataSource's two observed values are the header words of the two
 * sibling classes it selects between: D_8006D4E8 (the CD-ROM read driver,
 * code_179d8_q.c) and gVabDriverMethods (the SPU/VAB streamer, code_179d8_e.c). */
#define DATASOURCE_CD  0x13
#define DATASOURCE_SPU 0x23

void *GetClass6D3C8Methods(void) {
    return D_8006D3C8;
}

void *DestroyChained(Class6D430 *this) {
    this->freeGuard = 0;
    this->methods->dtor(this);
    Get_vtable_BasicClass()->dtor(this);
    func_80017CFC(this);
    return NULL;
}

void Class6D430__Class6D430(Class6D430 *this) {
    Get_vtable_BasicClass()->ctor(this);
    this->methods = (Class6D430Methods *) GetClass6D430Methods();
    this->pendingGeneration = 0;
    this->buffer = NULL;
    this->bufferSize = 0;
    this->freeGuard = 0;
    this->unk22 = 0;
    this->flags = 0;
    this->unk28 = 0;
    this->unk2A = 0;
}

void *Class6D430__Destroy(Class6D430 *this) {
    this->methods->onBufferChanged(this);
    return this->methods->freeBuffer(this);
}

void Class6D430__AllocBuffer(Class6D430 *this, s32 arg1) {
    s32 savedPendingGeneration;
    s32 size;
    void *newRes;

    if (this->buffer != NULL) {
        return;
    }
    savedPendingGeneration = this->pendingGeneration;
    this->pendingGeneration = 0;
    this->methods->configureBuffer(this, arg1, 1, 0);
    size = this->methods->bufferControl(this, 0, 2);
    newRes = func_80017B34(size);
    if (newRes != NULL) {
        this->methods->bufferControl(this, 0, 0);
        this->methods->installBuffer(this, newRes, size);
        this->methods->onBufferChanged(this);
        this->buffer = newRes;
        this->bufferSize = size;
        this->pendingGeneration = savedPendingGeneration;
    } else {
        func_80017CFC(NULL);
        this->methods->onBufferChanged(this);
    }
}

void Class6D430__FreeBuffer(Class6D430 *this) {
    if (this->buffer == NULL) {
        return;
    }
    if (this->bufferSize == 0) {
        return;
    }
    if (this->freeGuard != 0) {
        return;
    }
    func_80017CFC(this->buffer);
    this->buffer = NULL;
}

void NoOp(void) {
}

void Class6D430__SetFlag(Class6D430 *this) {
    this->flags |= 1;
}

void *GetClass6D430Methods(void) {
    return D_8006D430;
}

extern s32 gActiveDataSource;
extern void *GetVabDriverMethods(void);
extern void *GetClass6D4E8Methods(void);

void *GetActiveDataSourceMethods(void) {
    if (gActiveDataSource == DATASOURCE_SPU) {
        return GetVabDriverMethods();
    } else {
        return GetClass6D4E8Methods();
    }
}

Vec3_171e0 *SetVec3(Vec3_171e0 *this, s32 x, s32 y, s32 z) {
    this->x = x;
    this->y = y;
    this->z = z;
    return this;
}

INCLUDE_ASM("asm/nonmatchings/code_171e0", SetActiveDataSource);

void Class6D430__CopyFields(Class6D430 *dst, Class6D430 *src) {
    dst->unk40 = src->unk40;
    dst->unk44 = src->unk44;
    dst->unk48 = src->unk48;
    dst->unk4C = src->unk4C;
    dst->unk50 = src->unk50;
    dst->unk54 = src->unk54;
    dst->unk58 = src->unk58;
    dst->unk68 = src->unk68;
    dst->unk6C = src->unk6C;
    dst->unk70 = src->unk70;
    dst->unk74 = src->unk74;
}

extern s32 gActiveDataSource;
extern s32 LockCd(void);

void LockActiveDataSource(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        LockCd();
    }
}

extern s32 UnlockCd(void);

void UnlockActiveDataSource(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        UnlockCd();
    }
}

extern s32 IsCdBusy(void);

s32 IsActiveDataSourceBusy(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        return IsCdBusy();
    }
    return 0;
}

extern s32 IsCdIdle(void);

s32 IsActiveDataSourceIdle(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        return IsCdIdle();
    }
    return 1;
}

extern s32 GetCdOperation(void);

s32 GetActiveDataSourceOperation(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        return GetCdOperation();
    }
    return 0;
}

extern s32 GetCdState(void);

s32 GetActiveDataSourceState(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        return GetCdState();
    }
    return 0;
}

typedef s32 (*Func80026F34Fn)(s32, s32, s32);
/* round 58 (alpha, externcheck.py): SetVabDriverMode's real definition
 * (code_179d8_e.c) takes 2 args; SetCdDriverMode's (code_179d8_q.c)
 * genuinely takes 3. Both are only ever REFERENCED here, never called
 * directly -- `fn` dispatches through the shared 3-arg Func80026F34Fn
 * pointer type SetCdDriverMode needs, with SetVabDriverMode's own body
 * simply not reading the 3rd word. Declaring SetVabDriverMode's own
 * arity honestly (2, matching its definition) costs nothing byte-wise --
 * a function-pointer VALUE assignment emits no argument-count-dependent
 * code, just an address load -- and produces only a benign "incompatible
 * pointer type" warning at the `fn = SetVabDriverMode;` line below. */
extern s32 SetVabDriverMode(s32 a, s32 b);
extern s32 SetCdDriverMode(s32 arg0, s32 arg1, s32 arg2);

void SetActiveDataSourceDriverMode(s32 arg0, s32 arg1, s32 arg2) {
    Func80026F34Fn fn;

    fn = SetVabDriverMode;
    if (gActiveDataSource == DATASOURCE_CD) {
        fn = SetCdDriverMode;
    }
    do {
    } while (fn(arg0, arg1, arg2) == 0);
}

extern s32 GetCdDriverMode(void); /* arity-ok: the definition takes (s32 *outMode2) and the body reads $a0 (`beqz a0` at 0x80027EF8), but GetActiveDataSourceDriverMode's tail call sets nothing -- retail's jal at 0x80026FD0 has a nop delay slot */
extern s32 GetVabDriverMode(void); /* arity-ok: same as above -- the definition takes (s32 *arg0) and the body reads $a0 (`beqz a0` at 0x8002C448), but the call at 0x80026FC0 has a nop delay slot and sets nothing */

s32 GetActiveDataSourceDriverMode(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        return GetCdDriverMode();
    } else {
        return GetVabDriverMode();
    }
}

extern s32 GetCdUseVSyncCallback(void);
extern s32 func_8002C478(void);

s32 GetActiveDataSourceUseVSyncCallback(void) {
    if (gActiveDataSource == DATASOURCE_CD) {
        return GetCdUseVSyncCallback();
    } else {
        return func_8002C478();
    }
}

extern s32 D_8008A850;
extern void SetFileTable(void *arg0);
extern s32 GetFileTableCount(void);
extern void SetFileTableCount(s32 arg0);
extern s32 ResolveFileEntries(void *arg0, s32 arg1);

s32 RegisterFileTableEntries(void *arg0, s32 arg1) {
    s32 idx;

    if (gActiveDataSource == DATASOURCE_CD) {
        D_8008A850 = 1;
        SetFileTable(arg0);
        idx = GetFileTableCount();
        SetFileTableCount(idx + arg1);
        return ResolveFileEntries((u8 *) arg0 + idx * 0x1C, arg1);
    }
    return 1;
}

extern void *D_8008A854;

void func_800270AC(void *value)
{
	D_8008A854 = value;
}

void *func_800270B8(void)
{
	return D_8008A854;
}

char *BuildFileName(char *dest, char *arg1, char *arg2, char *arg3) {
    dest[0] = '\0';
    if (arg2 != NULL) {
        strcat(dest, arg2);
    }
    strcat(dest, arg1);
    strcat(dest, arg3);
    return dest;
}

/* ROUND 34: `strcat` (0x80027130, this unit's last function, 42 words) LEFT
 * THIS FILE. It is Sony's -- `libc2/strcat.o`, Psy-Q 3.3, 0xA8 of text
 * covering exactly it -- and the unit's segment now ends at 0x17930 with an
 * `o` entry after it. It had been matched as C since round 8, and the head
 * had noticed at the time that it reads as library code rather than game
 * code ("carries a guard textbook strcat has no reason to"); it was right,
 * and the reclassification is the correction CLAUDE.md asks for, not a
 * regression.
 *
 * The C body and the two load-bearing source shapes it turned on (the
 * post-increment scan, worth 25 words; `return dest` rather than
 * `return NULL` on the NULL-dest path, worth one) are preserved in full in
 * docs/match-reports/strcat.md. Nothing is lost by deleting them here.
 *
 * Callers in this unit (BuildFileName, just above) keep calling `strcat`
 * under that name -- the declaration in include/code_171e0.h still serves,
 * and now resolves to the linked object. */
