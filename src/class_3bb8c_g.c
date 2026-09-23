#include "common.h"
#include "class_3bb8c.h"

void Class86E00_3bb8c_g__SetState(Class86E00_3bb8c_g *self, s32 arg1)
{
    Class86E00Methods_3bb8c_g *methods = self->methods;
    s32 ret;
    s32 i;

    if (self->unk28 == arg1) {
        arg1 = 0x17;
    }

    methods->slot30(self, arg1);
    methods->slot84(self);
    methods->slot80(self, arg1);

    self->unk5C = 0;
    switch (arg1) {
    case 0x13:
        arg1 = methods->slot50(self) ? 0x11 : 8;
        methods->slot7C(self, arg1);
        break;
    case 0x14:
        if (*(u8 *)self->unk40 == 0) {
            methods->slot58(self, self->unk40, self->unk30, self->unk34);
        }
        ret = methods->slot68(self, self->unk40, self->unk44, self->unk4C,
                              self->unk50, self->unk54, self->unk58);
        arg1 = ret ? 0x16 : 0xC;
        methods->slot7C(self, arg1);
        break;
    case 0x15:
        ret = methods->slot64(self, self->unk40, self->unk54, self->unk58);
        arg1 = ret ? 0x16 : 0x10;
        methods->slot7C(self, arg1);
        break;
    case 0x11:
        methods->slot9C(self);
        break;
    case 0x12:
        methods->slotA8(self);
        break;
    }

    if ((u32)(arg1 - 0x16) < 2) {
        if (self->unk24 == 1 && self->unk38 != NULL) {
            func_80017CFC(self->unk3C);
            for (i = 0; i < self->unk2C; i++) {
                func_80017CFC(((void **)self->unk38)[i]);
            }
            func_80017CFC(self->unk38);
            self->unk38 = NULL;
        }
        self->unk28 = 0;
        self->unk24 = 0;
    } else {
        self->unk28 = arg1;
    }
}

/* Not this round's function (lives outside this unit's slice) -- a
 * resource loader taking a path, returning a handle. Already typed at
 * several other call sites in the project (`class_3bb8c_i.c`,
 * `class_3bb8c_j.c`, `class_39e08.h`, `code_2cc8c.h`), each with its own
 * local view per this project's established convention. */
extern ChildObj86ED0 *func_8003B39C(char *path);
/* Not this round's function -- consumes the short-lived handle above and
 * produces the object stored into `self->unk70`. */
extern ChildObj86ED0 *func_80041C9C(ChildObj86ED0 *arg0, void *arg1, s32 arg2);

/* 0x11 (17) entries, indexed by `arg1` (range-checked `< 0x11` below);
 * mostly `char *` string pointers into rodata, a few raw literal words at
 * indices never reached from this call site. `asm/data/76DC8.data.s`. */
extern char *D_80086E80[];
extern const char D_8008AAB4[]; /* "CARD\\" */
extern const char D_8008AABC[]; /* ".TIM" */
/* 3-word opaque block, `func_80041C9C`'s arg1, address-only here. */
extern s32 D_80086EC4;
/* opaque block, the fresh `unk70`'s own `slot4C` arg2, address-only here. */
extern s32 D_8008AA94;

void Class86E00_3bb8c_g__LoadCardIcon(Class86E00_3bb8c_g *self, s32 arg1)
{
    char path[0x20];
    char *buf;
    char *name;
    ChildObj86ED0 *handle;
    Class86E00Unk70Obj_3bb8c_g *newVal;

    if (arg1 >= 0x11) {
        return;
    }
    if (self->unk68 == 0) {
        return;
    }
    if (self->unk70 != NULL) {
        return;
    }

    buf = path;
    name = D_80086E80[arg1];
    buf[0] = '\0';
    strcat(buf, D_8008AAB4);
    strcat(buf, name);
    strcat(buf, D_8008AABC);

    handle = func_8003B39C(buf);
    handle->methods->slot78(handle);
    newVal = func_80041C9C(handle, (void *)&D_80086EC4, 0);
    self->unk70 = newVal;
    handle->methods->release(handle);
    newVal->methods->slot4C(newVal, self->unk68, (void *)&D_8008AA94);
}

void Class86E00_3bb8c_g__TickCardIcon(Class86E00_3bb8c_g *self)
{
    if (self->unk70 != NULL) {
        self->unk70 = self->unk70->methods->slot4(self->unk70);
    }
}

void Class86E00_3bb8c_g__OnNotify(Class86E00_3bb8c_g *self, s32 arg1, s32 arg2)
{
    if (self->unk28 != 0) {
        if (arg2 == 0x19) {
            self->methods->slot90(self);
        } else if (arg2 == 0x17) {
            self->methods->slot94(self);
        }
    }
}

void Class86E00_3bb8c_g__SetChildFlag8(Class86E00_3bb8c_g *self, s32 arg1)
{
    if (self->unk6C != NULL) {
        self->unk6C->methods->slot80(self->unk6C, arg1, 0x7F, 0x7F);
    }
}

void Class86E00_3bb8c_g__AdvanceState(Class86E00_3bb8c_g *self)
{
    Class86E00Methods_3bb8c_g *methods = self->methods;

    switch (self->unk28) {
    case 2:
    case 4:
    case 0xA:
    case 0xE:
        methods->slot8C(self, 0);
        if (self->unk28 == 0xE) {
            strcpy((char *)self->unk40, self->unk30);
            strcat((char *)self->unk40, ((char **)self->unk3C)[(s32)self->unk80]);
            strcpy((char *)self->unk44, ((char **)self->unk38)[(s32)self->unk80]);
        }
        if (self->unk24 == 2) {
            methods->slot78(self, self->unk40, self->unk44, self->unk48,
                             self->unk4C, self->unk50, self->unk54, self->unk58);
        } else if (self->unk24 == 1) {
            methods->slot74(self, self->unk40, self->unk44, self->unk54, self->unk58);
        }
        break;
    case 6:
        methods->slot8C(self, 0);
        methods->slot7C(self, 7);
        break;
    case 3:
    case 5:
    case 8:
    case 9:
    case 0xC:
    case 0xD:
    case 0x10:
        methods->slot8C(self, 0x10);
        methods->slot7C(self, 0x17);
        break;
    }
}

void Class86E00_3bb8c_g__ForceIdleFromState(Class86E00_3bb8c_g *self)
{
    switch (self->unk28) {
    case 4:
    case 6:
    case 0xA:
    case 0xE:
        self->methods->slot8C(self, 0x10);
        self->methods->slot7C(self, 0x17);
        break;
    default:
        break;
    }
}

void Class86E00_3bb8c_g__TickStateDelay(Class86E00_3bb8c_g *self)
{
    s32 old;
    s32 newVal;

    if (self->unk28 == 7) {
        old = self->unk5C;
        newVal = old + 1;
        self->unk5C = newVal;
        if (old < 6) {
            return;
        }
        self->methods->slot7C(self, 0x13);
    } else if (self->unk28 == 0xB) {
        old = self->unk5C;
        newVal = old + 1;
        self->unk5C = newVal;
        if (old < 6) {
            return;
        }
        self->methods->slot7C(self, 0x14);
    } else if (self->unk28 == 0xF) {
        old = self->unk5C;
        newVal = old + 1;
        self->unk5C = newVal;
        if (old < 6) {
            return;
        }
        self->methods->slot7C(self, 0x15);
    }
}

void Class86E00_3bb8c_g__AttachChildA(Class86E00_3bb8c_g *self)
{
    if (self->unk68 != 0 && self->unk60 != 0) {
        if (self->unk78 == NULL) {
            self->unk78 = func_80050BA8((self->unk48 << 1) + self->unk44, 1);
            self->unk74 = 1;
        }
        self->methods->slot10(self, self->unk78);
        self->unk78->methods->slot44(self->unk78, self->unk68);
        self->unk78->methods->slot4C(self->unk78, self->unk60, self->unk64, self->unk6C);
    }
}

void Class86E00_3bb8c_g__DetachChildA(Class86E00_3bb8c_g *self)
{
    if (self->unk68 != 0 && self->unk60 != 0 && self->unk78 != NULL) {
        self->unk78->methods->slot50(self->unk78);
        self->unk78->methods->slot48(self->unk78);
        if (self->unk74 != 0) {
            self->unk78->methods->release(self->unk78);
            self->unk78 = NULL;
        }
    }
}

void Class86E00_3bb8c_g__OnCommand(Class86E00_3bb8c_g *self, void *arg1, s32 arg2)
{
    switch (arg2) {
    case 2:
        self->methods->slotA0(self);
        self->methods->slot78(self, self->unk40, self->unk44, self->unk48,
                               self->unk4C, self->unk50, self->unk54, self->unk58);
        break;
    case 3:
        self->methods->slotA0(self);
        self->methods->slot7C(self, 0x17);
        break;
    }
}

void Class86E00_3bb8c_g__AttachChildB(Class86E00_3bb8c_g *self)
{
    if (self->unk68 != 0 && self->unk60 != 0) {
        if (self->unk7C == NULL) {
            self->unk7C = func_80051A5C(self->unk38, 1);
            self->unk74 = 1;
        }
        self->methods->slot10(self, self->unk7C);
        self->unk7C->methods->slot44(self->unk7C, self->unk68);
        self->unk7C->methods->slot4C(self->unk7C, self->unk60, self->unk64, self->unk6C);
    }
}

void Class86E00_3bb8c_g__DetachChildB(Class86E00_3bb8c_g *self)
{
    if (self->unk68 != 0 && self->unk60 != 0 && self->unk7C != NULL) {
        self->unk7C->methods->slot50(self->unk7C);
        self->unk7C->methods->slot48(self->unk7C);
        if (self->unk74 != 0) {
            self->unk7C->methods->release(self->unk7C);
            self->unk7C = NULL;
        }
    }
}

void Class86E00_3bb8c_g__OnItemSelected(Class86E00_3bb8c_g *self, GenericSlot9CObj_3bb8c_g *arg1, s32 arg2)
{
    switch (arg2) {
    case 2:
        self->unk80 = arg1->methods->slot9C(arg1);
        self->methods->slotAC(self);
        self->methods->slot7C(self, 0xE);
        break;
    case 3:
        self->methods->slotAC(self);
        self->methods->slot7C(self, 0x17);
        break;
    }
}

GenericCtorTable_3bb8c_d *GetClass86E00Methods(void)
{
    return &D_80086DC4;
}

/* Sony's, from libc2 (round 45's own local view -- this unit's first use). */
extern s32 atoi(char *s);

/* VALUE-of `%gp_rel`, round 45's own local view -- a fixed rodata template
 * (ROM image still-uncarved, `asm/data/1C34.rodata.s` region) this
 * function copies raw byte ranges out of; also read by
 * `class_3bb8c_d.c`'s own (differently-typed) local view. */
extern u8 *D_8008AAC4;

/* Struct-copy helper types for round 45's Class86E00_3bb8c_g__CopyMemcardIconTemplate, all deliberately
 * all-`s8` (alignment 1) per this round's FormatNumberIntoBuffer lever: retail
 * copies these ranges as one unaligned `lwl`/`lwr` word chunk per 4 bytes,
 * with any non-multiple-of-4 remainder as INDIVIDUAL byte loads/stores,
 * never merged into a halfword -- alignment 2 would let GCC trust a
 * halfword move retail does not have. */
typedef struct {
    s8 raw[6];
} Buf6_3bb8c_g;
typedef struct {
    s8 raw[12];
} Buf12_3bb8c_g;
typedef struct {
    s8 a, b;
} Pair2_3bb8c_g;

/* Signature is `include/class_3bb8c.h`'s ALREADY-shared
 * `extern s32 Class86E00_3bb8c_g__CopyMemcardIconTemplate(s32 arg0, s32 arg1);` (class_3bb8c_m's own
 * caller, TaskObjF__WriteMemcardSaveFile), matched exactly -- this unit's own definition
 * must agree with that declaration since both are visible in this
 * translation unit. Cast to `u8 *` internally; retail's own register
 * content at exit (`$v0` left holding a pointer into the `D_8008AAC4`
 * template in every path) confirms the real return type is a pointer,
 * loosely read as `s32` by the caller that never dereferences it. */
s32 Class86E00_3bb8c_g__CopyMemcardIconTemplate(s32 arg0, s32 arg1)
{
    u8 *self = (u8 *)arg0;
    u8 *src = (u8 *)arg1;
    s32 t0;
    s32 idx;
    u8 *p;

    if (src != NULL) {
        t0 = ((u32)(src[0xE] - 0x38) < 2) ? 0xE : 0xD;

        *(Pair2_3bb8c_g *)(self + 0x18) = *(Pair2_3bb8c_g *)(D_8008AAC4 + 0x1E);
        *(Buf12_3bb8c_g *)(self + 0x6) = *(Buf12_3bb8c_g *)(D_8008AAC4 + 0x1E);

        idx = atoi((char *)(src + t0)) - 1;
        p = D_8008AAC4 + idx * 2;
        *(Pair2_3bb8c_g *)(self + 0x8) = *(Pair2_3bb8c_g *)p;
        return (s32)p;
    } else {
        u8 *q = D_8008AAC4;

        *(Buf6_3bb8c_g *)(self + 0x6) = *(Buf6_3bb8c_g *)(q + 0x1E);
        return (s32)q;
    }
}
