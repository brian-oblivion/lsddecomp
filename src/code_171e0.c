#include "common.h"
#include "code_171e0.h"

void *GetClass6D3C8Methods(void) {
    return D_8006D3C8;
}

void *DestroyChained(UnkFlagsObj_171e0 *this) {
    this->unk20 = 0;
    this->methods->dtor(this);
    Get_vtable_BasicClass()->dtor(this);
    func_80017CFC(this);
    return NULL;
}

void Class6D430__Class6D430(UnkFlagsObj_171e0 *this) {
    Get_vtable_BasicClass()->ctor(this);
    this->methods = (UnkFlagsObjMethods_171e0 *) GetClass6D430Methods();
    this->unk0C = 0;
    this->unk10 = NULL;
    this->unk14 = 0;
    this->unk20 = 0;
    this->unk22 = 0;
    this->unknown_value_0x24 = 0;
    this->unk28 = 0;
    this->unk2A = 0;
}

void *Class6D430__Destroy(UnkFlagsObj_171e0 *this) {
    this->methods->slot48(this);
    return this->methods->slot5C(this);
}

void Class6D430__AllocBuffer(UnkFlagsObj_171e0 *this, s32 arg1) {
    s32 savedUnk0C;
    s32 size;
    void *newRes;

    if (this->unk10 != NULL) {
        return;
    }
    savedUnk0C = this->unk0C;
    this->unk0C = 0;
    this->methods->slot44(this, arg1, 1, 0);
    size = this->methods->slot4C(this, 0, 2);
    newRes = func_80017B34(size);
    if (newRes != NULL) {
        this->methods->slot4C(this, 0, 0);
        this->methods->slot54(this, newRes, size);
        this->methods->slot48(this);
        this->unk10 = newRes;
        this->unk14 = size;
        this->unk0C = savedUnk0C;
    } else {
        func_80017CFC(NULL);
        this->methods->slot48(this);
    }
}

void Class6D430__FreeBuffer(UnkFlagsObj_171e0 *this) {
    if (this->unk10 == NULL) {
        return;
    }
    if (this->unk14 == 0) {
        return;
    }
    if (this->unk20 != 0) {
        return;
    }
    func_80017CFC(this->unk10);
    this->unk10 = NULL;
}

void NoOp(void) {
}

void Class6D430__SetFlag(UnkFlagsObj_171e0 *this) {
    this->unknown_value_0x24 |= 1;
}

void *GetClass6D430Methods(void) {
    return D_8006D430;
}

extern s32 gActiveDataSource;
extern void *func_8002C438(void);
extern void *GetClass6D4E8Methods(void);

void *GetActiveDataSourceMethods(void) {
    if (gActiveDataSource == 0x23) {
        return func_8002C438();
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

void Class6D430__CopyFields(UnkFlagsObj_171e0 *dst, UnkFlagsObj_171e0 *src) {
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
    if (gActiveDataSource == 0x13) {
        LockCd();
    }
}

extern s32 UnlockCd(void);

void UnlockActiveDataSource(void) {
    if (gActiveDataSource == 0x13) {
        UnlockCd();
    }
}

extern s32 IsCdBusy(void);

s32 IsActiveDataSourceBusy(void) {
    if (gActiveDataSource == 0x13) {
        return IsCdBusy();
    }
    return 0;
}

extern s32 IsCdIdle(void);

s32 IsActiveDataSourceIdle(void) {
    if (gActiveDataSource == 0x13) {
        return IsCdIdle();
    }
    return 1;
}

extern s32 GetCdOperation(void);

s32 GetActiveDataSourceOperation(void) {
    if (gActiveDataSource == 0x13) {
        return GetCdOperation();
    }
    return 0;
}

extern s32 GetCdState(void);

s32 GetActiveDataSourceState(void) {
    if (gActiveDataSource == 0x13) {
        return GetCdState();
    }
    return 0;
}

typedef s32 (*Func80026F34Fn)(s32, s32, s32);
extern s32 func_8002C468(s32 arg0, s32 arg1, s32 arg2);
extern s32 SetCdDriverMode(s32 arg0, s32 arg1, s32 arg2);

void SetActiveDataSourceDriverMode(s32 arg0, s32 arg1, s32 arg2) {
    Func80026F34Fn fn;

    fn = func_8002C468;
    if (gActiveDataSource == 0x13) {
        fn = SetCdDriverMode;
    }
    do {
    } while (fn(arg0, arg1, arg2) == 0);
}

extern s32 GetCdDriverMode(void);
extern s32 func_8002C448(void);

s32 GetActiveDataSourceDriverMode(void) {
    if (gActiveDataSource == 0x13) {
        return GetCdDriverMode();
    } else {
        return func_8002C448();
    }
}

extern s32 func_80028B6C(void);
extern s32 func_8002C478(void);

s32 GetActiveDataSourceUseVSyncCallback(void) {
    if (gActiveDataSource == 0x13) {
        return func_80028B6C();
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

    if (gActiveDataSource == 0x13) {
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
