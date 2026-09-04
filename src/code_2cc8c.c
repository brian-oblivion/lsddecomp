/* code_2cc8c -- first 20-function slice of the 0x2CC8C block (153 functions
 * total; the remainder is the code_2cc8c_b asm segment).
 *
 * Carved in round 10 on the belief that this block had the LOWEST
 * toolchain-blocker density of any uncarved segment. That belief was
 * WRONG for two of the 20: `func_8003C48C` and `func_8003C63C` both hit
 * the `addiu $at, $at, %lo` screening grep on their jump-table dispatch
 * (`%lo(jtbl_*)`), and a `jtbl_*` symbol there is NOT a safe exception --
 * measured through the pinned pipeline, maspsx folds a jump-table address
 * resolution exactly like an indexed data-global one, with no way to tell
 * them apart below cc1 (which emits the same generic pseudo-op for both:
 * `lw $2,$L13($2)` against `lbu $2,D_x($4)`). Both are genuinely
 * `addiu_at`-blocked; see docs/research/addiu-at-blocker.md and each
 * function's own docs/match-reports/*.md for the reproducer detail. Of the
 * OTHER 18, none hit either open blocker.
 *
 * This comment originally asserted the opposite -- that the two were
 * ordinary switch dispatchers and the whole slice was clear. That was the
 * head's error, reasoned rather than measured, and it was retracted the
 * same round. Do not re-derive it; the note under "Open toolchain
 * blockers" in CLAUDE.md exists to stop exactly this.
 *
 * Shape: this is class-framework code. Objects carry their method table at
 * offset 0 (`lw $v1, 0x0($a0)` then `lw $v0, 0xNN($v1)` then `jalr`), so
 * resolve slots with tools/classtable.py rather than by counting. The
 * class is `Obj86B60` (include/code_2cc8c.h), named after its base method
 * table D_80086B60 (78 slots; a derived override table also exists at
 * D_80087AAC, 73 slots -- see the header's own comment). The first two
 * functions are switch dispatchers over a small event/message code -- both
 * blocked, see above.
 */

#include "common.h"
#include "code_2cc8c.h"

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003C48C);

void func_8003C51C(Obj86B60 *self, s32 a1, s32 a2)
{
    Obj86B60Methods *methods;

    methods = self->methods;
    func_8003E5C8()->slot5C(self, a1, a2);
    if (self->unk3C != 0) {
        u32 bound;

        bound = self->unk1C;
        if ((u32)self->unk40 < bound) {
            methods->slot60(self, 6);
        }
    }
    switch (self->unk20) {
    case 2:
        methods->slot60(self, 4);
        break;
    case 4:
        methods->slotAC(self);
        break;
    case 7:
        methods->slotC0(self);
        break;
    case 8:
        methods->slot60(self, 3);
        break;
    }
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003C63C);

void func_8003C794(Obj86B60 *self, s32 a1)
{
    self->unk40 = a1;
    if (a1 >= 0) {
        self->unk40 = a1 * 20;
    }
}

void func_8003C7B4(Obj86B60 *self, s32 a1)
{
    Unk48Obj *child;

    child = self->unk48;
    if (child != NULL) {
        child->methods->slot80(child, a1, 0x60, 0x60);
    }
}

void func_8003C7F4(Obj86B60 *self, s32 a1)
{
    if (self->unk4C != NULL) {
        self->methods->slot70(self, 0x10);
        self->methods->slot60(self, 0xA);
    }
}

void func_8003C858(Obj86B60 *self, s32 a1)
{
    s32 reason;

    if (self->unk4C != NULL) {
        self->methods->slot70(self, 0x10);
        reason = 0xF;
        if (self->unk3C == 1) {
            reason = 0xB;
        }
        self->methods->slot60(self, reason);
    }
}

void func_8003C8D0(Obj86B60 *self, s32 a1)
{
    if (self->unk4C != NULL && self->unk3C != 1) {
        self->methods->slot70(self, 0x10);
        self->methods->slot60(self, 0x11);
    }
}

void func_8003C944(Obj86B60 *self, s32 a1)
{
    void (*handler)(Obj86B60 *self);

    if (self->unk4C == NULL) {
        return;
    }
    if (self->unk3C == 1) {
        handler = self->methods->slotEC;
    } else if (self->unk3C == 2) {
        handler = self->methods->slot118;
    } else {
        return;
    }
    handler(self);
}

void func_8003C9B0(Obj86B60 *self, s32 a1)
{
    void (*handler)(Obj86B60 *self);

    if (self->unk4C == NULL) {
        return;
    }
    if (self->unk3C == 1) {
        handler = self->methods->slotE8;
    } else if (self->unk3C == 2) {
        handler = self->methods->slot114;
    } else {
        return;
    }
    handler(self);
}

void func_8003CA1C(Obj86B60 *self)
{
    Unk4CObj *target;
    s32 idx;

    target = self->unk4C;
    idx = self->unk58;
    if (target->unk24[idx] != NULL) {
        self->methods->slot108(self);
    } else if (idx == target->unkC) {
        self->methods->slot94(self);
    }
}

void func_8003CA94(Obj86B60 *self)
{
    if (self->unk9C != NULL) {
        self->unk9C(self->unkA0);
    }
    self->methods->slot60(self, 7);
}

void func_8003CAEC(Obj86B60 *self, void (*a1)(void *ctx), void *a2)
{
    self->unk9C = a1;
    self->unkA0 = a2;
}

void func_8003CAF8(Obj86B60 *self, s32 a1)
{
    Obj86B60Methods *methods;

    methods = self->methods;
    switch (a1) {
    case 0:
        self->unk88 = NULL;
        break;
    case 1:
        self->unk88 = methods->slotB0;
        break;
    }
}

void func_8003CB30(Obj86B60 *self, s32 a1)
{
    Obj86B60Methods *methods;

    methods = self->methods;
    switch (a1) {
    case 0:
        self->unk8C = NULL;
        break;
    case 1:
        self->unk8C = methods->slotC4;
        break;
    }
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c", func_8003CB68);

void func_8003CBB8(Obj86B60 *self, s32 a1)
{
    self->unk84 = a1;
}

s32 func_8003CBC0(Obj86B60 *self)
{
    s32 result;

    result = 1;
    if (self->unk88 != NULL) {
        result = self->unk88(self);
    }
    if (result != 0) {
        self->methods->slot60(self, 5);
    }
    return result;
}

s32 func_8003CC2C(Obj86B60 *self)
{
    s32 prod;
    u8 buffer[3];

    prod = self->unk1C * self->unk84;
    buffer[0] = prod + self->unk90[0];
    buffer[1] = prod + self->unk90[1];
    buffer[2] = prod + self->unk90[2];
    self->methods->slotE4(self, buffer);
    self->unk78->methods->slotB8(self->unk78, 1, buffer);
    return (u8)prod >= 0x81;
}

s32 func_8003CCDC(Obj86B60 *self)
{
    s32 result;

    result = 1;
    if (self->unk8C != NULL) {
        result = self->unk8C(self);
        if (result == 0) {
            goto epilogue;
        }
    }
    self->methods->slot60(self, 8);
epilogue:
    return result;
}
