#include "common.h"
#include "code_2c054.h"

StreamTaskObj *func_8003B854(s32 a1, s32 a2, s32 a3, s32 a4)
{
    StreamTaskObj *self;

    self = func_80017B34(0xDC);
    if (self != NULL) {
        func_8003BE84()->slot08(self, a1, a2, a3, a4);
        return self;
    }
    return NULL;
}

void func_8003B8E4(StreamTaskObj *self, s32 a1, s32 a2, s32 a3, StreamTaskInitData *a4) {
    func_8003DFBC()->slot08(self, a1, a2, a3);
    self->methods = func_8003BE84();
    if (a4 != NULL) {
        self->unkA8 = *a4;
    } else {
        self->unkA8 = *func_8003DFCC();
    }
    self->unkB4 = func_80045438(func_8003DFCC(), 0, 0);
    self->unkB8 = 0;
    self->methods->slot40(self);
}

void func_8003B9DC(StreamTaskObj *self) {
    self->unkB4->methods->slot04(self->unkB4);
    func_8003DFBC()->slot0C(self);
}

void func_8003BA38(StreamTaskObj *self) {
    self->unkC8 = -1;
    self->unkC4 = 0;
    self->unkCC = 1;
    self->unkD0 = 0;
    self->unkD4 = 1;
}

void func_8003BA58(StreamTaskObj *self, s32 a1, s32 arg2, s32 typeLookup, s32 flag) {
    self->unkB8 = arg2;
    self->unkBC = typeLookup;
    self->unkC0 = flag;
    func_8003DFBC()->slot44(self, a1, 0);
}

void func_8003BAB4(StreamTaskObj *self) {
    func_8003DFBC()->slot4C(self);
    self->unkA4 = 0;
    self->unkB4->methods->slot6C(self->unkB4, self->unkC0);
    if (self->unkB4->methods->slot40(self->unkB4, self->unkB8, self->unkBC, self->unkC4, self->unkC8) != 0) {
        self->methods->slot6C(self, 0);
    }
}

void func_8003BB5C(StreamTaskObj *self, s32 a1, s32 a2) {
    func_8003DFBC()->slot5C(self, a1, a2);
    if (self->unkA4 != 0) {
        return;
    }
    self->unkA4 = self->unkB4->methods->slot48(self->unkB4);
    if (self->unkA4 == 0) {
        return;
    }
    if (self->unkD8 != 0) {
        return;
    }
    self->methods->slot60(self, 7);
}

void func_8003BC14(StreamTaskObj *self, s32 a1) {
    func_8003DFBC()->slot60(self, a1);
    switch (a1) {
    case 5:
        self->unkD8 = 0;
        break;
    case 7:
        self->unkD8 = 1;
        break;
    case 8:
        if (self->unkD4 == 0) {
            self->unkB4->methods->slot4C(self->unkB4);
        }
        break;
    case 0x12:
        self->methods->slot94(self);
        break;
    }
}

void func_8003BCF4(StreamTaskObj *self, s32 a1) {
    self->unk40 = a1;
    if (a1 >= 0) {
        self->unk40 = a1 * 15;
    }
}

void func_8003BD10(StreamTaskObj *self) {
    func_8003DFBC()->slot78(self);
    if (self->unkCC != 0) {
        self->unk38 = 2;
        self->methods->slot60(self, 0x12);
    }
}

void func_8003BD74(StreamTaskObj *self) {
    func_8003DFBC()->slot80(self);
}

void func_8003BDAC(StreamTaskObj *self) {
    func_8003DFBC()->slot84(self);
}

void func_8003BDE4(void) {
}

void func_8003BDEC(void) {
}

void func_8003BDF4(StreamTaskObj *self) {
    if (self->unkD4 != 0) {
        self->unkB4->methods->slot4C(self->unkB4);
    } else {
        self->methods->slot60(self, 7);
    }
}

void func_8003BE5C(StreamTaskObj *self, s32 a1) {
    self->unkC4 = a1;
}

void func_8003BE64(StreamTaskObj *self, s32 a1) {
    self->unkC8 = a1;
}

void func_8003BE6C(StreamTaskObj *self, s32 a1) {
    self->unkCC = a1;
}

void func_8003BE74(StreamTaskObj *self, s32 a1) {
    self->unkD0 = a1;
}

void func_8003BE7C(StreamTaskObj *self, s32 a1) {
    self->unkD4 = a1;
}

StreamTaskObjMethods *func_8003BE84(void) {
    return &D_8006E5F8;
}

/* The TaskCore allocator: 0xA4 bytes, constructed through the base class's
 * own slot +0x008. The cast is because TaskCoreMethods::slot08 is typed for
 * StreamTaskObj (its usual caller) rather than for the base object. */
TaskCoreObj *func_8003BE94(s32 a1, s32 a2, s32 a3)
{
    TaskCoreObj *self;

    self = func_80017B34(0xA4);
    if (self != NULL) {
        func_8003DFBC()->slot08((StreamTaskObj *)self, a1, a2, a3);
        return self;
    }
    return NULL;
}

void func_8003BF10(StreamTaskObj *self, s32 a1, s32 a2, StreamTaskUnkB4Obj *a3) {
    StreamTaskUnkB4Obj *tmp;
    TaskCoreMethods *core;

    func_8003E5C8()->slot08(self);
    core = func_8003DFBC();
    self->methods = (StreamTaskObjMethods *)core;
    core->slotD8(self, a1);
    if (a2 != 0) {
        self->unk48 = New_VabStreamObj(a2);
    } else {
        self->unk48 = a3;
    }
    self->unk44 = a2;
    self->methods->slotD4(self, 0, 0);
    tmp = func_80044F30(0);
    self->unk80 = tmp;
    tmp = func_80044CD4(0, tmp);
    self->unk7C = tmp;
    self->unk78 = func_800441B4(tmp, 1);
    self->methods->slot40(self);
}

void func_8003C008(StreamTaskObj *self) {
    self->unk78->methods->slot04(self->unk78);
    self->unk7C->methods->slot04(self->unk7C);
    self->unk80->methods->slot04(self->unk80);
    if (self->unk44 != 0) {
        self->unk48->methods->slot04(self->unk48);
    }
    if (self->unk70 != 0) {
        self->unk74->methods->slot04(self->unk74);
    }
    self->methods->slotDC(self);
    func_8003E5C8()->slot0C(self);
}

void func_8003C11C(StreamTaskObj *self) {
    StreamTaskObjMethods *methods = self->methods;
    methods->slot6C(self, -1);
    methods->slotA4(self, &D_8006E860[0], &D_8006E860[3], &D_8006E860[6]);
    methods->slot9C(self, 1);
    methods->slotA0(self, 1);
    self->unk84 = 9;
    self->unk28 = 3;
    self->unk2C = 0x12C;
    self->unk30 = 0x40;
    self->unk9C = 0;
    self->unkA0 = 0;
    self->unk34 = 1;
    self->unk3C = 0;
}

s32 func_8003C1DC(StreamTaskObj *self, s32 a1, s32 a2) {
    func_8003E5C8()->slot44(self, a1, a2);
    return self->unk38;
}

void func_8003C238(StreamTaskObj *self) {
    TaskCoreObj *unk18;
    TaskCoreObjMethods *core;

    unk18 = self->unk18;
    core = unk18->methods;
    self->methods->slotE0(self, self->unk14);
    self->unk78->methods->slot4C(self->unk78, self->unk14, 0);
    if (self->unk88 != 0) {
        self->methods->slotE4(self, &self->unk90);
        self->unk78->methods->slotB8(self->unk78, 1, &self->unk90);
    }
    if (self->unk74 == 0) {
        self->unkC->unk0->methods->slot78(self->unkC->unk0, &self->unk90, D_8006E854);
    }
    self->unkC->unk0->methods->slot78(self->unkC->unk0, &self->unk90, 0);
    core->slot48(unk18, self->unk28);
    core->slot4C(unk18, self->unk2C);
    core->slot50(unk18, self->unk30);
    core->slot70(unk18, self->unk14, D_8006E86C, D_8006E86C, 0);
    core->slot8C(unk18);
    self->unk38 = 0;
}

void func_8003C3D0(StreamTaskObj *self) {
    TaskCoreObj *obj = self->unk18;
    obj->methods->slot90(obj);
    obj->methods->slot74(obj);
    self->unk78->methods->slot50(self->unk78);
    if (self->unk34 != 0) {
        self->unkC->unk0->methods->slot78(self->unkC->unk0, &self->unk93, 0);
    }
}
