#include "common.h"
#include "code_2cc8c.h"

s32 func_8003CD48(Obj86B60 *self)
{
    s32 c = 0x80 - (self->unk1C * self->unk84);
    u8 buf[3];

    buf[0] = c;
    buf[1] = c;
    buf[2] = c;
    self->methods->slotE4(self, buf);
    self->unk78->methods->slotB8(self->unk78, 1, buf);
    return (u8)c >= 0x81;
}

void func_8003CDE0(Obj86B60 *self, const char *a1, Unk74Obj *a2)
{
    if (a1 != NULL) {
        if (self->unk70 != NULL) {
            self->unk74->methods->slot4(self->unk74);
        }
        self->unk74 = func_8003B39C(a1);
        self->unk74->methods->slot78(self->unk74);
        self->unk74->methods->slot5C(self->unk74);
    } else {
        self->unk74 = a2;
    }
    self->unk70 = a1;
}

void func_8003CE98(Obj86B60 *self, Unk4CObj *a1)
{
    char **list;
    s32 count;
    s32 size;
    Unk64Elem **arr;
    Unk74Obj *handle;
    s32 i;

    self->unk4C = a1;
    if (a1 == NULL) {
        return;
    }

    list = a1->unk1C;
    count = 0;
    while (*list++ != NULL) {
        count++;
    }
    size = count * 4;
    arr = func_80017B34(size);
    self->unk54 = arr;
    self->unk5C = func_80017B34(size);
    self->unk60 = func_80017B34(size);
    self->unk64 = func_80017B34(size);
    self->unk50 = count;

    if (a1->unk0 != NULL) {
        handle = func_8003B39C(a1->unk0);
        handle->methods->slot78(handle);
        handle->methods->slot5C(handle);
    } else {
        handle = a1->unk4;
    }

    list = a1->unk1C;
    i = 0;
    if (*list != NULL) {
        do {
            void *extra = a1->unk24[i];
            s32 len = strlen(*list);

            *arr = func_800408CC(handle, len, *list);
            arr++;
            if (extra != NULL) {
                self->unk58 = i;
                self->methods->slotF8(self, extra, handle);
            }
            list++;
            i++;
        } while (*list != NULL);
    }

    self->unk68 = func_800404D0(D_8008A8E8, D_8008A8F0, 0);
    a1->unk4 = handle;
}

void func_8003D050(Obj86B60 *self)
{
    Unk64Elem **arr;
    s32 i;

    if (self->unk4C == NULL) {
        return;
    }
    if (self->unk4C->unk0 != NULL) {
        Unk74Obj *o = self->unk4C->unk4;
        o->methods->slot4(o);
    }
    self->unk68->methods->slot4(self->unk68);
    arr = self->unk54;
    for (i = 0; i < self->unk50; arr++) {
        Unk64Elem *elem;

        if (self->unk4C->unk24[i] != NULL) {
            self->unk58 = i;
            self->methods->slotFC(self);
        }
        elem = *arr;
        elem->methods->slot4(elem);
        i++;
    }
    func_80017CFC(self->unk64);
    func_80017CFC(self->unk60);
    func_80017CFC(self->unk5C);
    func_80017CFC(self->unk54);
}

void func_8003D194(Obj86B60 *self, void *a1)
{
    Unk64Elem **arr;
    u8 *ptr;
    s32 i;

    if (self->unk4C == NULL) {
        return;
    }
    arr = self->unk54;
    ptr = self->unk4C->unk20;
    for (i = 0; i < self->unk50; i++, arr++, ptr += 8) {
        if (self->unk4C->unk18[i] == NULL) {
            Unk64Elem *elem = *arr;

            elem->methods->slot4C(elem, a1, ptr);
            if (self->unk4C->unk24[i] != NULL) {
                self->unk58 = i;
                self->methods->slot100(self, a1, 0);
            }
        } else {
            Unk64Elem *elem = *arr;

            elem->methods->slot50(elem);
        }
    }
}

void func_8003D2CC(Obj86B60 *self, void *a1)
{
    s32 origIdx;
    Unk64Elem **arr;
    s32 i;

    if (self->unk4C == NULL) {
        return;
    }
    arr = self->unk54;
    origIdx = self->unk58;
    for (i = 0; i < self->unk50;) {
        Unk64Elem *elem = *arr;

        arr++;
        elem->methods->slotB8(elem, a1);
        if (self->unk4C->unk24[i] != NULL) {
            self->unk58 = i;
            self->methods->slot104(self, a1);
        }
        i++;
        __asm__("");
    }
    self->unk58 = origIdx;
}

void func_8003D3B0(Obj86B60 *self)
{
    s32 i;

    if (self->unk4C == NULL) {
        return;
    }
    i = self->unk58;
    i++;
    for (;;) {
        if (i >= self->unk50) {
            i = 0;
        }
        if (i == self->unk58) {
            break;
        }
        if (self->unk4C->unk18[i++] != NULL) {
            continue;
        }
        i--;
        break;
    }
    self->methods->slotF0(self, i, 1);
}

void func_8003D444(Obj86B60 *self)
{
    s32 i;

    if (self->unk4C == NULL) {
        return;
    }
    i = self->unk58;
    i--;
    for (;;) {
        if (i < 0) {
            i = self->unk50 - 1;
        }
        if (i == self->unk58) {
            break;
        }
        if (self->unk4C->unk18[i--] != NULL) {
            continue;
        }
        i++;
        break;
    }
    self->methods->slotF0(self, i, 1);
}

void func_8003D4DC(Obj86B60 *self, s32 a1, void *a2)
{
    s32 idx;
    Unk64Elem *elemB;
    Unk64Elem *elemA;

    if (self->unk4C == NULL) {
        return;
    }
    idx = self->unk58;
    elemB = self->unk54[idx];
    elemA = self->unk54[a1];
    if (idx >= 0) {
        elemB->methods->slotB8(elemB, self->unk4C->unk10);
    }
    elemA->methods->slotB8(elemA, (u8 *)self->unk4C + 0x13);
    self->unk58 = a1;
    if (a2 != NULL) {
        self->methods->slot70(self, 0);
    }
    self->methods->slot60(self, 9);
}

s32 func_8003D5C0(Obj86B60 *self)
{
    return self->unk58;
}

void func_8003D5CC(Obj86B60 *self, SrcDesc *a1, void *a2)
{
    char **list;
    s32 idx;
    s32 count;
    Unk64Elem **buf;

    list = a1->unk18;
    idx = self->unk58;
    count = 0;
    while (*list++ != NULL) {
        count++;
    }
    buf = func_80017B34(count * 4);
    self->unk64[idx] = (void *)buf;
    self->unk60[idx] = a1->unk4;
    self->unk5C[idx] = count;

    list = a1->unk18;
    if (*list != NULL) {
        do {
            s32 len = strlen(*list);

            *buf = func_800408CC(a2, len, *list);
            list++;
            buf++;
        } while (*list != NULL);
    }
}

void func_8003D6D4(Obj86B60 *self)
{
    func_800183DC(self->unk64[self->unk58], self->unk5C[self->unk58]);
    func_80017CFC(self->unk64[self->unk58]);
}

/* STALL -- see docs/match-reports/func_8003D73C.md. Round 49 (delta):
 * a bare __asm__("") barrier between a NAMED `delta = counter * 10;` and
 * the reload-and-subtract fixed the long-standing sll/lw scheduling swap
 * outright (retail's own mult-before-reload order now reproduced
 * exactly) -- length now EXACT (145/145 total, no drift), raw 136/145.
 * The remaining 9 words are the SAME target->unk14 register-identity
 * residue this unit shares with func_8003DAD4 (2 words) plus a
 * register-role swap that appears tied to it (6 words) plus one
 * early-materialization delay-slot filler (`addiu $a2,sp,0x10` vs a
 * `nop`, the same class already carried permuter-exhausted elsewhere in
 * this project). Restored to INCLUDE_ASM per project rule. */
#if 0
void func_8003D73C(Obj86B60 *self, void *a1, s32 a2)
{
    s32 idx;
    Unk64Elem **arr;
    s32 count;
    s32 counter;
    s32 local[2];

    idx = self->unk58;
    arr = (Unk64Elem **)self->unk64[idx];
    {
        Unk24Elem *target = (Unk24Elem *)self->unk4C->unk24[idx];

        count = self->unk5C[idx];
        counter = target->unk4;
    }

    {
        s32 i;

        for (i = 0; i < count; i++) {
            (*arr)->methods->slot50(*arr);
            arr++;
        }
    }

    {
        Unk24Elem *target = (Unk24Elem *)self->unk4C->unk24[idx];
        s32 t0 = target->unk10;
        s32 t1 = target->unk14;

        local[0] = t0;
        local[1] = t1;
    }
    __asm__("" ::: "memory");
    {
        s32 delta = counter * 10;
        __asm__("");
        local[1] -= delta;
    }

    if (a2 != 0) {
        s32 local2[2];

        self->unk68->methods->slot4C(self->unk68, self->unk14);
        local2[0] = 0x28;
        local2[1] = count * 12;
        self->unk68->methods->slotC0(self->unk68, local2);
    } else {
        self->unk68->methods->slot50(self->unk68);
    }

    arr = (Unk64Elem **)self->unk64[idx];
    {
        s32 i;

        for (i = 0; i < count; i++) {
            (*arr)->methods->slot4C(*arr, a1, local);
            (*arr)->methods->slot60(*arr, a2);
            local[1] += 10;
            arr++;
        }
    }

    arr = (Unk64Elem **)self->unk64[idx];
    {
        Unk64Elem *elem = arr[counter];

        elem->methods->slot60(elem, 1);
    }
}
#endif

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003D73C);

void func_8003D980(Obj86B60 *self, void *a1)
{
    s32 idx = self->unk58;
    Unk64Elem **arr = (Unk64Elem **)self->unk64[idx];
    s32 count = self->unk5C[idx];
    s32 i;

    for (i = 0; i < count; i++) {
        Unk64Elem *elem = *arr;
        arr++;
        elem->methods->slotB8(elem, a1);
    }
}

void func_8003DA10(Obj86B60 *self)
{
    s32 idx;
    Unk64Elem *elem;
    u8 *buf;

    if (self->unk3C != 1) {
        return;
    }
    idx = self->unk58;
    self->methods->slot100(self, self->unk14, 1);
    elem = ((Unk64Elem **)self->unk64[idx])[self->unk60[idx]];
    buf = (u8 *)self->unk4C->unk24[idx] + 8;
    elem->methods->slotB8(elem, buf);
    self->unk3C = 2;
    self->methods->slot60(self, 14);
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_b", func_8003DAD4);

void func_8003DCAC(Obj86B60 *self)
{
    s32 idx;
    s32 counter;
    Unk64Elem **arr;
    Unk64Elem *elem1;
    Unk64Elem *elem2;
    s32 newVal;

    if (self->unk3C != 2) {
        return;
    }
    idx = self->unk58;
    counter = self->unk60[idx];
    self->methods->slot100(self, self->unk14, 0);
    arr = (Unk64Elem **)self->unk64[idx];
    elem1 = arr[counter];
    elem1->methods->slotB8(elem1, self->unk4C->unk10);
    newVal = ((s32 *)self->unk4C->unk24[idx])[1];
    self->unk60[idx] = newVal;
    elem2 = arr[newVal];
    elem2->methods->slot60(elem2, 1);
    self->unk3C = 1;
    self->methods->slot60(self, 17);
}

void func_8003DDC8(Obj86B60 *self)
{
    s32 idx = self->unk58;
    s32 v = self->unk60[idx];

    v++;
    if (v >= self->unk5C[idx]) {
        v = 0;
    }
    self->methods->slot11C(self, v, 1);
}

void func_8003DE30(Obj86B60 *self)
{
    s32 idx = self->unk58;
    s32 v = self->unk60[idx];

    v--;
    if (v < 0) {
        v = self->unk5C[idx] - 1;
    }
    self->methods->slot11C(self, v, 1);
}

void func_8003DE9C(Obj86B60 *self, s32 a1, void *a2)
{
    s32 idx;
    s32 counter;
    Unk64Elem **arr;
    Unk64Elem *elem1;
    Unk64Elem *elem2;
    u8 *buf;

    idx = self->unk58;
    counter = self->unk60[idx];
    arr = (Unk64Elem **)self->unk64[idx];
    elem1 = arr[counter];
    elem2 = arr[a1];
    elem1->methods->slotB8(elem1, self->unk4C->unk10);
    buf = (u8 *)self->unk4C->unk24[idx] + 8;
    elem2->methods->slotB8(elem2, buf);
    self->unk60[idx] = a1;
    if (a2 != NULL) {
        self->methods->slot70(self, 0);
    }
    self->methods->slot60(self, 9);
}
