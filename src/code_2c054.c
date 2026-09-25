#include "common.h"
#include "code_2c054.h"

StreamTaskObj *New_StreamTaskObj(s32 a1, s32 a2, s32 a3, s32 a4)
{
    StreamTaskObj *self;

    self = BMemPMgrAlloc(0xDC);
    if (self != NULL) {
        Get_vtable_StreamTaskObj()->slot08(self, a1, a2, a3, a4);
        return self;
    }
    return NULL;
}

void StreamTaskObj__StreamTaskObj(StreamTaskObj *self, s32 a1, s32 a2, s32 a3, StreamTaskInitData *a4) {
    Get_vtable_TaskCore()->slot08(self, a1, a2, a3);
    self->methods = Get_vtable_StreamTaskObj();
    if (a4 != NULL) {
        self->unkA8 = *a4;
    } else {
        self->unkA8 = *GetDefaultStreamTaskInitData();
    }
    self->unkB4 = New_MoviePlayer(GetDefaultStreamTaskInitData(), 0, 0);
    self->unkB8 = 0;
    self->methods->slot40(self);
}

void StreamTaskObj__Destroy(StreamTaskObj *self) {
    self->unkB4->methods->slot04(self->unkB4);
    Get_vtable_TaskCore()->slot0C(self);
}

void StreamTaskObj__Reset(StreamTaskObj *self) {
    self->unkC8 = -1;
    self->unkC4 = 0;
    self->unkCC = 1;
    self->unkD0 = 0;
    self->unkD4 = 1;
}

void StreamTaskObj__Configure(StreamTaskObj *self, s32 a1, s32 arg2, s32 typeLookup, s32 flag) {
    self->unkB8 = arg2;
    self->unkBC = typeLookup;
    self->unkC0 = flag;
    Get_vtable_TaskCore()->slot44(self, a1, 0);
}

void StreamTaskObj__func_8003BAB4(StreamTaskObj *self) {
    Get_vtable_TaskCore()->slot4C(self);
    self->unkA4 = 0;
    self->unkB4->methods->slot6C(self->unkB4, self->unkC0);
    if (self->unkB4->methods->slot40(self->unkB4, self->unkB8, self->unkBC, self->unkC4, self->unkC8) != 0) {
        self->methods->slot6C(self, 0);
    }
}

void StreamTaskObj__func_8003BB5C(StreamTaskObj *self, s32 a1, s32 a2) {
    Get_vtable_TaskCore()->slot5C(self, a1, a2);
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

void StreamTaskObj__func_8003BC14(StreamTaskObj *self, s32 a1) {
    Get_vtable_TaskCore()->slot60(self, a1);
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

void StreamTaskObj__SetUnk40(StreamTaskObj *self, s32 a1) {
    self->unk40 = a1;
    if (a1 >= 0) {
        self->unk40 = a1 * 15;
    }
}

void StreamTaskObj__func_8003BD10(StreamTaskObj *self) {
    Get_vtable_TaskCore()->slot78(self);
    if (self->unkCC != 0) {
        self->unk38 = 2;
        self->methods->slot60(self, 0x12);
    }
}

void StreamTaskObj__func_8003BD74(StreamTaskObj *self) {
    Get_vtable_TaskCore()->slot80(self);
}

void StreamTaskObj__func_8003BDAC(StreamTaskObj *self) {
    Get_vtable_TaskCore()->slot84(self);
}

void StreamTaskObj__NoOpSlot88(void) {
}

void StreamTaskObj__NoOpSlot8C(void) {
}

void StreamTaskObj__func_8003BDF4(StreamTaskObj *self) {
    if (self->unkD4 != 0) {
        self->unkB4->methods->slot4C(self->unkB4);
    } else {
        self->methods->slot60(self, 7);
    }
}

void StreamTaskObj__SetUnkC4(StreamTaskObj *self, s32 a1) {
    self->unkC4 = a1;
}

void StreamTaskObj__SetUnkC8(StreamTaskObj *self, s32 a1) {
    self->unkC8 = a1;
}

void StreamTaskObj__SetUnkCC(StreamTaskObj *self, s32 a1) {
    self->unkCC = a1;
}

void StreamTaskObj__SetUnkD0(StreamTaskObj *self, s32 a1) {
    self->unkD0 = a1;
}

void StreamTaskObj__SetUnkD4(StreamTaskObj *self, s32 a1) {
    self->unkD4 = a1;
}

StreamTaskObjMethods *Get_vtable_StreamTaskObj(void) {
    return &gStreamTaskObjMethods;
}

/* The TaskCore allocator: 0xA4 bytes, constructed through the base class's
 * own slot +0x008. The cast is because TaskCoreMethods::slot08 is typed for
 * StreamTaskObj (its usual caller) rather than for the base object. */
TaskCoreObj *New_TaskCore(s32 a1, s32 a2, s32 a3)
{
    TaskCoreObj *self;

    self = BMemPMgrAlloc(0xA4);
    if (self != NULL) {
        Get_vtable_TaskCore()->slot08((StreamTaskObj *)self, a1, a2, a3);
        return self;
    }
    return NULL;
}

void TaskCore__TaskCore(StreamTaskObj *self, s32 a1, s32 a2, StreamTaskUnkB4Obj *a3) {
    StreamTaskUnkB4Obj *tmp;
    TaskCoreMethods *core;

    Get_vtable_IntermediateBase()->ctor((IntermediateBase *)self);
    core = Get_vtable_TaskCore();
    self->methods = (StreamTaskObjMethods *)core;
    core->slotD8(self, a1);
    if (a2 != 0) {
        self->unk48 = New_VabStreamObj(a2);
    } else {
        self->unk48 = a3;
    }
    self->unk44 = a2;
    self->methods->slotD4(self, 0, 0);
    tmp = New_TileAtlas(0);
    self->unk80 = tmp;
    tmp = New_TileMap(0, tmp);
    self->unk7C = tmp;
    self->unk78 = New_BgLayer(tmp, 1);
    self->methods->slot40(self);
}

void TaskCore__Finalize(StreamTaskObj *self) {
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
    Get_vtable_IntermediateBase()->finalize((IntermediateBase *)self);
}

void TaskCore__Reset(StreamTaskObj *self) {
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

s32 TaskCore__Init(StreamTaskObj *self, s32 a1, s32 a2) {
    Get_vtable_IntermediateBase()->init((IntermediateBase *)self, (IntermediateBaseInitArgs *)a1, a2);
    return self->unk38;
}

void TaskCore__OnInit(StreamTaskObj *self) {
    StreamTaskUnk18Obj *unk18;
    StreamTaskUnk18Methods *core;

    unk18 = self->unk18;
    core = unk18->methods;
    self->methods->slotE0(self, self->unk14);
    self->unk78->methods->slot4C(self->unk78, self->unk14, 0);
    if (self->unk88 != 0) {
        self->methods->slotE4(self, &self->unk90);
        self->unk78->methods->slotB8(self->unk78, 1, &self->unk90);
    }
    if (self->unk74 == 0) {
        self->unkC->unk0->methods->slot78(self->unkC->unk0, &self->unk90, gDefaultStreamTaskInitData);
    }
    self->unkC->unk0->methods->slot78(self->unkC->unk0, &self->unk90, 0);
    core->slot48(unk18, self->unk28);
    core->slot4C(unk18, self->unk2C);
    core->slot50(unk18, self->unk30);
    core->slot70(unk18, self->unk14, D_8006E86C, D_8006E86C, 0);
    core->slot8C(unk18);
    self->unk38 = 0;
}

void TaskCore__OnDeinit(StreamTaskObj *self) {
    StreamTaskUnk18Obj *obj = self->unk18;
    obj->methods->slot90(obj);
    obj->methods->slot74(obj);
    self->unk78->methods->slot50(self->unk78);
    if (self->unk34 != 0) {
        self->unkC->unk0->methods->slot78(self->unkC->unk0, &self->unk93, 0);
    }
}
