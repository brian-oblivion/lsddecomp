#include "common.h"
#include "code_2cc8c.h"

/* ROUND 34: THIS UNIT LOST ITS FIRST EIGHT FUNCTIONS -- six of them to Sony,
 * two to files of their own -- and now begins at 0x305B0 / New_Class6E99C.
 * The segment it used to be is split three ways:
 *
 *   [c code_2cc8c_e0]  GsSetNearClip          game code, own file
 *   [o libgs/gs_123]   Gssub_make_matrix      was func_8003FB1C, matched C
 *   [c code_2cc8c_e1]  GsSetWorkBase          game code, own file
 *   [o libgs/gs_111]   GsDrawOt               was func_8003FBF4, matched C
 *   [o libgs/gs_113]   GsClearOt              was func_8003FC18, matched C
 *   [o libgs/gs_108]   GsSetLightMode         was func_8003FC70, matched C
 *   [o libgte/fgo_00]  TransposeMatrix        was func_8003FCFC, a 20w stall
 *   [o libgte/fog_01]  SetFogNear             was func_8003FD4C, matched C
 *   [c code_2cc8c_e]   New_Class6E99C onward   <- this file
 *
 * THIS FILE KEEPS THE NAME deliberately: it holds the unit's remaining
 * INCLUDE_ASM stubs and its class, so every
 * `INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", ...)` path below and every
 * match report naming this unit stays valid. Only the two one-function heads
 * needed new names.
 *
 * NEITHER RODATA SLOT IS OURS ANY MORE. jtbl_80011108 (0x1908) went with
 * Gssub_make_matrix and D_80011194 (0x1994, "not supported light mode %d\n")
 * went with GsSetLightMode -- both are their own object's `.rdata` section
 * now. This unit needs no `.rodata` attach at all; if a future carve of it
 * hits Gate 2's `undefined reference to '.LXXXXXXXX'`, that is a NEW jump
 * table, not these.
 *
 * The six Sony bodies are gone from this file, not lost -- five matched C
 * bodies and one INCLUDE_ASM stub. Each one's match report is kept and
 * retitled CONVERTED, and carries its derivation verbatim.
 */

/*
 * WHAT THIS UNIT IS (round 61, track 3; revised rounds 85 and 87, track 4).
 * Its 17 functions are the bottom two links of `Class6B5CC -> BoxFill ->
 * Class6E99C`: first Class6E99C's (D_8006E99C, 0x164, `New_Class6E99C` to
 * `GetClass6E99CMethods`, include/Class6E99C.h), then BoxFill's allocator,
 * ctor and Reset (0x64, include/BoxFill.h, a GsBOXF screen rectangle; the
 * rest of its methods open code_2cc8c_f).
 *
 * Class6E99C fades the box's colour: configure picks the channels (a
 * 4/2/1 = r/g/b mask) and a tick count, StartFadeDown/StartFadeUp set the
 * start colour and the step's sign, Update steps the selected channels once
 * per call until Stop, and PushPosition/PopPosition save and restore the
 * box's size and position (tier B; include/Class6E99C.h's banner has the
 * evidence). See each function's own `## Naming` section.
 */

Class6E99C *New_Class6E99C(void *size, s32 channels, s32 pri) {
    Class6E99C *self;

    self = BMemPMgrAlloc(0xA0);
    if (self != NULL) {
        GetClass6E99CMethods()->ctor(self, size, channels, pri);
        return self;
    }
    return NULL;
}

void Class6E99C__Class6E99C(Class6E99C *self, void *size, s32 channels, s32 pri) {
    BoxFillMethods *base;
    void *color;

    base = GetBoxFillMethods();
    if (channels != 0) {
        color = &D_8006EA90[channels * 3];
    } else {
        color = D_8006EAA8;
    }
    base->ctor((BoxFill *)self, size, color, pri);
    self->methods = GetClass6E99CMethods();
    ((Class6E99CResetFn)self->methods->reset)(self, channels);
}

void Class6E99C__Reset(Class6E99C *self, s32 channels) {
    self->defaultChannels = channels;
    self->state = 0;
    self->step = 0xA;
    self->channels = 0;
    self->unk7C = 0;
    self->methods->setDisplay(self, 0);
    self->methods->setSemiTrans(self, 0);
    self->altMode = 0;
}

void Class6E99C__Update(Class6E99C *self, void *sender, s32 event) {
    s32 old;

    if (event != 2) {
        return;
    }
    old = self->ticksLeft;
    self->ticksLeft = old - 1;
    if (old > 0) {
        if (self->unk7C == 9) {
            return;
        }
        if (self->channels & 4) {
            self->color[0] += (u8)self->step;
        }
        if (self->channels & 2) {
            self->color[1] += (u8)self->step;
        }
        if (self->channels & 1) {
            self->color[2] += (u8)self->step;
        }
    } else {
        self->methods->stop(self, sender);
    }
}

void Class6E99C__SetStep(Class6E99C *self, s32 step) {
    self->step = step;
}

/* Both StartFade functions forward their own three arguments to configure
 * untouched (no argument register is set before that jalr), and spelling
 * the forward is load-bearing: a `(self)`-only call compiles to the same
 * instructions in a different order (29/35; round 73). */
void Class6E99C__StartFadeDown(Class6E99C *self, BasicClass *source, s32 channels, s32 arg3) {
    s32 idx;

    if (self->state != 0) {
        return;
    }
    idx = self->methods->configure(self, source, channels, arg3);
    self->methods->setColor(self, 1, &D_8006EA90[idx * 3]);
    self->state = 1;
    self->step = -self->step;
}

void Class6E99C__StartFadeUp(Class6E99C *self, BasicClass *source, s32 channels, s32 arg3) {
    if (self->state != 0) {
        return;
    }
    channels = self->methods->configure(self, source, channels, arg3);
    if (self->altMode != 0) {
        self->ticksLeft--;
    } else {
        self->methods->setColor(self, 1, &D_8006EAA8[channels * 3]);
    }
    self->state = 2;
}

s32 Class6E99C__Configure(Class6E99C *self, BasicClass *source, s32 channels, s32 arg3) {
    Class6E99CMethods *methods;
    s32 rate;
    s32 q1, q2;

    methods = self->methods;
    if (channels < 0) {
        channels = self->defaultChannels;
    } else {
        self->defaultChannels = channels;
    }
    rate = 1;
    if (channels != 0) {
        self->channels = channels;
    } else {
        rate = 2;
        self->channels = 0xF;
    }
    self->channels = channels;
    if (channels == 0) {
        self->channels = 0xF;
    }
    q1 = 0x100 / self->step;
    self->unk7C = arg3;
    self->ticksLeft = q1;
    if (self->altMode != 0) {
        q2 = q1 / self->divisor;
        self->ticksLeft = q1 - (s16)q2;
    }
    q2 = self->mask;
    q2 = q2 / self->ticksLeft;
    self->unk84 = q2;
    methods->addChild(self, source);
    methods->setSemiTrans(self, 1);
    methods->setSemiTransRate(self, rate);
    methods->setDisplay(self, 1);
    return channels;
}

void Class6E99C__Stop(Class6E99C *self, BasicClass *source) {
    Class6E99CMethods *methods;
    s32 event;

    methods = self->methods;
    if (self->state == 0) {
        return;
    }
    if (self->state == 1) {
        event = 5;
        if (self->altMode == 0) {
            methods->setDisplay(self, 0);
            methods->setSemiTrans(self, 0);
        }
    } else {
        event = 6;
        if (self->altMode != 0) {
            if (self->channels == 0xF) {
                methods->setColor(self, 1, D_8006EAA8);
            }
            methods->setSemiTrans(self, 0);
        }
    }
    methods->removeChild(self, source);
    if (self->step < 0) {
        self->step = -self->step;
    }
    self->state = 0;
    methods->notifyParents(self, event);
}

void *Class6E99C__GetColor(Class6E99C *self) {
    if (self->channels == 0xF) {
        return D_8006EAA8;
    }
    return &D_8006EA90[self->channels * 3];
}

/* Both s32 pairs are copied as whole structs. GCC 2.6.3's MIPS
 * `movstrsi_internal` clobbers $v0/$v1/$a0/$a1, so `self` and `size`, live
 * across the first copy, cannot stay in their incoming registers: that is
 * retail's entry `move $a3,$a0` / delay-slot `move $t0,$a1` (round 73). */
void Class6E99C__PushPosition(Class6E99C *self, SkipShort2 *size, Pair32E99C *pos) {
    if (self->parent != 0) {
        self->savedW = self->boxW;
        self->savedH = self->boxH;
        *(Pair32E99C *)&self->savedPosX = *(Pair32E99C *)&self->posX;
        self->boxW = size->x;
        self->boxH = size->y;
        *(Pair32E99C *)&self->posX = *pos;
    }
}

void Class6E99C__PopPosition(Class6E99C *self) {
    s32 t0, t1;

    t0 = self->savedPosX;
    t1 = self->savedPosY;
    self->posX = t0;
    self->posY = t1;
    __asm__("" ::: "memory");
    self->boxW = self->savedW;
    self->boxH = self->savedH;
}

void Class6E99C__SetDivisorMode(Class6E99C *self, s32 altMode, s32 divisor) {
    self->altMode = altMode;
    self->divisor = divisor;
}

Class6E99CMethods *GetClass6E99CMethods(void) {
    return &D_8006E99C;
}

BoxFill *New_BoxFill(void *size, void *color, s32 pri) {
    BoxFill *self;

    self = BMemPMgrAlloc(0x6C);
    if (self != NULL) {
        GetBoxFillMethods()->ctor(self, size, color, pri);
        return self;
    }
    return NULL;
}

void BoxFill__BoxFill(BoxFill *self, SkipShort2 *size, void *color, s32 pri) {
    GetClass6B5CCMethods()->ctor((Class6B5CC *)self);
    self->methods = GetBoxFillMethods();
    ((BoxFillResetFn)self->methods->reset)(self, size, color, pri);
}

void BoxFill__Reset(BoxFill *self, SkipShort2 *size, void *color, s32 pri) {
    BoxFillMethods *methods;

    self->pri = pri;
    self->relative = 1;
    self->unk4C = 0;
    self->boxAttribute = 0;
    self->boxX = 0;
    self->boxY = 0;
    self->boxW = size->x;
    self->boxH = size->y;
    methods = self->methods;
    if (color == NULL) {
        color = D_8008A924;
    }
    methods->setColor(self, 1, color);
    self->methods->setMask(self, 0xD);
}
