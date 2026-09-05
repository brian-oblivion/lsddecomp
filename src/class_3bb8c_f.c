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
    handle = func_80050938(path, 1);
    if (handle == -1) {
        return 0;
    }
    hdr = func_80017B34(0x80);
    func_80050928(handle, hdr, 0x80);
    raw = ((u8 *)hdr)[2];
    seekPos = (raw << 7) - 0x780;
    func_80017CFC(hdr);
    func_800508E8(handle, seekPos, 0);
    func_80050928(handle, outBuf, outSize);
    func_800508F8(handle);
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

s32 func_8004F394(TaskObjF *self) {
    return func_8004F40C(self, func_80038F6C, 1);
}

s32 func_8004F3BC(TaskObjF *self) {
    return func_8004F40C(self, func_8003903C, 1);
}

s32 func_8004F3E4(TaskObjF *self) {
    return func_8004F40C(self, func_800390F4, 0);
}

s32 func_8004F40C(TaskObjF *self, s32 (*callback)(s32), s32 flag) {
    s32 i;
    s32 result;

    if (flag) {
        func_80024CE0();
    }
    for (i = 0; i < 4; i++) {
        result = callback(self->field14[i]);
        if (result == 0) {
            break;
        }
    }
    if (flag) {
        func_80024CF0();
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
            if (func_800390F4(arr[i]) != 0) {
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
