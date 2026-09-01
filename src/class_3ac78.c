#include "common.h"
#include "class_3ac78.h"

void func_8004A478(Class3AC78Sub34 *self, s32 arg1)
{
	Class86668 *child = self->child;

	if (child != NULL) {
		child->methods->slot80(child, arg1, 0x7F, 0x7F);
	}
}

Class86668Methods *func_8004A4B8(void)
{
	return &D_80086668;
}

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004A4C8);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004A534);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004A7C0);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004A984);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004AA10);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004AA6C);

void func_8004AB24(Class866E8 *self)
{
	if (self->unk70 != NULL) {
		self->methods->slotF4(self);
		self->methods->slot13C(self);
	}
}

void func_8004AB88(Class866E8 *self, AnyObj *other)
{
	if (other->methods->headerLowByte == 0x34) {
		self->methods->slotD0(self, other);
	}
}

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004ABD0);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004ACF8);

void func_8004ADC4(Class866E8 *self, s32 arg1, s32 arg2)
{
	self->unk60 = arg1;
	self->unk64 = arg2;
}

void func_8004ADD0(Class866E8 *self, void *arg1)
{
	self->unkE8 = arg1;
}

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004ADD8);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004AEA4);

void func_8004AFE0(Class866E8 *self, u8 *arg1, s32 arg2)
{
	u8 b3;

	self->unk7C = (s8)arg1[2] - 1;
	b3 = arg1[3];
	self->unk80 = arg2;
	self->unk84 = arg2;
	self->unk7E = (s8)b3 - 1;
	func_8004C93C(self);
}

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004B030);

INCLUDE_ASM("asm/nonmatchings/class_3ac78", func_8004B100);

void func_8004B2D4(Class866E8 *self)
{
	if (self != NULL && (self->unk36 & 0x80) != 0) {
		self->methods->slot38(self);
	}
}

void *func_8004B31C(Class866E8 *self)
{
	return &self->pad1C0;
}

void func_8004B324(void) {
}

void func_8004B32C(Class866E8 *self, s32 arg1)
{
	self->unk74 = arg1;
	self->unk7A = (s16)(arg1 >> 11);
	self->unk78 = (s16)(arg1 >> 12);
}

void func_8004B344(Class866E8 *self, s32 arg1)
{
	self->methods->slot40(self);
	self->unk68 = arg1;
}
