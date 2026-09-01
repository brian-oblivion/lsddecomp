#include "common.h"
#include "class_39e08.h"

/* New_ClassA: allocate a 0x50-byte instance and construct it. */
ClassA *func_80049608(void *arg1, void *arg2, s32 arg3) {
    ClassA *self;

    self = func_80017B34(0x50);
    if (self == NULL) {
        goto fail;
    }
    func_8004A060()->ctor(self, arg1, arg2, arg3);
    return self;
fail:
    return NULL;
}

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049684);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049830);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049958);

/* ClassA::resetState -- +0x040 slot. */
void func_80049A14(ClassA *self) {
    self->state = 0;
}

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049A1C);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049AC0);

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049B54);

/* ClassA::slot50 -- dispatches the two calls on the unk18 sub-object. */
void func_80049C50(ClassA *self) {
    Dispatch18 *obj;

    obj = self->unk18;
    obj->methods->slot90(obj);
    obj->methods->slot74(obj);
}

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049CA8);

/* STALL (32/33 words) -- see docs/match-reports/func_80049E20.md. Retail
 * spills the never-read-again second parameter to 0x10($sp); no reshaping
 * tried reproduces that store. */
void func_80049E20(ClassA *self, s32 arg1)
{
	Dispatch4C *d;

	d = func_80052B70(self->unk34, self->unk40, self->unk44, self->unk48, arg1);
	self->unk4C = d;
	self->methods->slot10(self, d);
	self->unk4C->methods->slot44(self->unk4C, self->unk0C, self->unk38);
	self->state = 2;
}

void func_80049EA4(void) {
}

void func_80049EAC(void) {
}

/* BLOCKED: addiu_at via jump table jtbl_8001140C, see
 * docs/research/addiu-at-blocker.md and docs/match-reports/func_80049EB4.md.
 * Pre-screened; not attempted. */
INCLUDE_ASM("asm/nonmatchings/class_39e08", func_80049EB4);

/* Get_vtable_ClassA -- a direct address load, not a call. */
ClassAMethods *func_8004A060(void) {
    return &D_800865C8;
}

/* BLOCKED: gp_rel, see docs/research/gp-relative-blocker.md and
 * docs/match-reports/func_8004A070.md. Pre-screened; not attempted. */
INCLUDE_ASM("asm/nonmatchings/class_39e08", func_8004A070);

/* New_ClassC: allocate a 0x38-byte instance of a third, unrelated class and
 * construct it through BasicClass's own ctor slot. */
void *func_8004A130(void *arg1, void *arg2) {
    void *self;

    self = func_80017B34(0x38);
    if (self == NULL) {
        goto fail;
    }
    func_8004A4B8()->ctor(self, arg1, arg2);
    return self;
fail:
    return NULL;
}

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_8004A19C);

/* ClassB::dtor -- notifies unk34 if present, then defers to ClassA's own
 * dtor (reached through func_8003E5C8's table, not this object's own
 * methods pointer -- calling through self->methods here would recurse into
 * this same override). */
void *func_8004A228(ClassA *self) {
    if (self->unk30 != 0) {
        self->unk34->methods->slot4(self->unk34);
    }
    return func_8003E5C8()->dtor(self);
}

/* ClassB::resetState (+0x040) -- resets the radius field through the
 * (shared) virtual setter with -1, which the setter's own sign check leaves
 * unscaled. */
void func_8004A294(ClassA *self) {
    self->methods->setRadius(self, -1);
}

/* ClassB::slot44 (+0x044) -- zeroes the output field, lets ClassA's base
 * implementation (reached through func_8003E5C8, not this object's own
 * methods pointer) populate it, then returns whatever it left behind. */
s32 func_8004A2C4(ClassA *self, void *arg1, void *arg2) {
    self->unk28 = 0;
    func_8003E5C8()->slot44(self, arg1, arg2);
    return self->unk28;
}

/* ClassB::slot48 (+0x048) -- a pure passthrough to ClassA's base
 * implementation of the same slot. */
s32 func_8004A324(ClassA *self) {
    return func_8003E5C8()->slot48(self);
}

void func_8004A35C(void) {
}

INCLUDE_ASM("asm/nonmatchings/class_39e08", func_8004A364);

/* ClassA::onEvent (+0x060), shared with ClassB -- forwards to ClassA's own
 * base implementation of this same slot (reached through func_8003E5C8),
 * then on event 4 marks the output flag and notifies slot7C. */
void func_8004A3EC(ClassA *self, s32 event) {
    func_8003E5C8()->onEvent(self, event);
    if (event == 4) {
        self->unk28 = 1;
        self->methods->slot7C(self);
    }
}

/* ClassA::setRadius (+0x06C), shared with ClassB -- stores the value
 * verbatim, then rescales it by *20 unless it was negative. */
void func_8004A458(ClassA *self, s32 val) {
    self->radius = val;
    if (val >= 0) {
        self->radius = val * 20;
    }
}
