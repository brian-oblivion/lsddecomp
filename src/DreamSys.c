#include "common.h"
#include "DreamSys.h"

/* Forward declarations for two of this unit's OWN functions, both called
   around line 450 but not defined until ~200 lines later, in ROM order.
   Without these, C89 implicitly declares them as `int ()` at the call site
   and cpp emits "implicit declaration of function". The implicit type
   happens to agree with the real one here, so nothing miscompiled -- but an
   implicit declaration also disables argument checking, which is precisely
   what caught func_8005D714's over-narrow `s8` parameters in include/Entity.h
   this round. A declaration is not a definition, so this does NOT affect the
   strict ROM-address ordering of the definitions below. */
s32 TestForStaticLink(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage);
s32 Test4TunnelLinks(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage);

DreamSys *New_DreamSys(void *arg0, s32 arg1, s32 arg2)
{
	DreamSys *this;

	this = func_80017B34(sizeof(DreamSys));
	if (this != NULL) {
		Get_vtable_DreamSys()->Constructor(this, arg0, arg1, arg2);
		return this;
	}
	return NULL;
}

DreamSys *DreamSys__DreamSys(DreamSys *this, void *arg1, s32 arg2, s32 arg3)
{
	void *val;

	func_80057C84()->ctor(this);
	this->vt = Get_vtable_DreamSys();
	this->unk_0x58 = arg2;
	this->unk_0x5C = (DreamSysUnk5C *)arg3;
	this->unk_0x64 = 0;
	this->unk_0x60 = arg1;
	val = ((DreamSysCtorArgObj *)arg1)->methods->slot0x80(arg1, 0);
	this->vt->slot10(this, val);
	this->vt->GetSetDreamTimeLimit(this, -1);
	this->unk_0x70 = 1;
	this->unk_0x6c = 0;
	this->unk_0x878 = 1;
	this->vt->InitNewGame(this);
	return this->vt->func_800588EC(this);
}

void DreamSys__func_588ec(DreamSys *this)
{
	this->vt->func_8001D344(this, 0);
	this->vt->func_8001CEB4(this, 1, D_80087E08);
	this->callback_0x80 = NULL;
	this->callback_0x98 = NULL;
	*(s32 *)this->unk_0xCC = 0;
	this->unk_0x908 = 0;
	this->unk_0x90C = 0;
	this->unk_0x910 = 0;
	this->unk_0x78 = 0;
	this->unk_0x924 = 0;
}

void DreamSys__func_58968(DreamSys *this, DreamSysFunc58968ArgObj *arg1)
{
	s32 local[4];

	arg1->methods->slot0xE4(arg1, local, this, &this->linkCoordinates);
	func_80057C84()->slot4C(this, arg1, local);
	this->vt->slot10(this, arg1);
	if (this->unknwon_int_0x44 == 0xE) {
		FlashbackEntry *entry = &this->storedFlasbacks[this->currentFlashbackIndex];
		this->vt->func_8001CEB4(this, 1, &entry->rotation);
		this->vt->GetSetDreamTimeLimit(this, entry->timeLimit + 4);
		this->currentFlashbackIndex++;
	}
	if (this->unk_0x6c != 0 && this->unk_0x888 != 0) {
		this->vt->func_8001CEB4(this, 1, (void *)this->unk_0x888);
	}
}

void func_80058A94(DreamSys *this)
{
	this->unk_0x4C->methods->slot0xF0(this->unk_0x4C);
	this->vt->func_80057130(this, this->unk_0x4C);
	func_80057C84()->slot0x50(this);
}

void func_80058B08(DreamSys *this, s32 arg1)
{
	s32 v;

	func_80057C84()->slot0x88(this, arg1);
	if (arg1 == -2)
		goto handle_neg2;
	if (arg1 != -1)
		return;

	v = this->unk_0x28->unk_0x36 & 0x7F;
	this->unk_0xB8 = v;
	if (v >= 0x18)
		this->unk_0xB8 = 0;

	if (this->unknwon_int_0x44 == 15 && this->unk_0xB8 == 0)
		this->unk_0xB8 = 2;

	if (this->currentStage != 9)
		return;
	goto shared_tail;

handle_neg2:
	if (this->unk_0x4C->methods->slot0x11C(this->unk_0x4C, (u8 *)this->unk_0x14 + 0x18)->unk_0x4->unk_0x2C != 2)
		goto neg2_mismatch;

shared_tail:
	this->vt->func_8005A7A0(this, this->unk_0x4C->methods->slot0x10C(this->unk_0x4C, 0, 0));
	return;

neg2_mismatch:
	this->vt->func_8005B990(this);
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80058C58);

void DreamSys__TimerTick(DreamSys *this, s32 arg1, s32 arg2)
{
	s32 old;

	if (arg2 != 2)
		return;

	old = this->dreamTimer;
	this->dreamTimer = old + 1;
	if ((u32)old < (u32)this->dreamTimeLimit)
		goto tick_only;

	if (this->isFlashbackSession) {
		if (this->unknwon_int_0x44 != 0 || this->vt->LoadNextFlashback(this, 0)) {
			__asm__("");
			this->dreamTimer = 0;
			return;
		}
	} else {
		this->vt->FlashbackSaving(this, 0, 0x10);
	}
	this->vt->slot30(this, 0xA);
	this->dreamTimer = 0;
	return;

tick_only:
	this->vt->func_80059394(this);
	this->vt->func_800593D8(this);
}

void func_80058E8C(DreamSys *this, void *arg1, s32 arg2)
{
	func_80057C84()->slot0x9C(this, arg1, arg2);
	if ((*(s32 *)(*(void **)arg1) & 0xFFF) == 0x114) {
		this->vt->ProcessChunkChange(this, arg1, arg2);
	}
}

void func_80058F18(DreamSys *this, void *arg1, s32 arg2)
{
	func_80057C84()->slot0xDC(this, arg1, arg2);
	if ((*(s32 *)(*(void **)arg1) & 0xFFFFF) == 0x1F234) {
		this->vt->InstanceEffectsOnJournal(this, arg1, arg2);
	}
}

void DreamSys__WallLink(DreamSys *this, void* unk_class_86aa0, int arg2)
{
	func_80057C84()->slot0xE0(this, unk_class_86aa0, arg2);
	if (arg2 != 4)
		return;
	if (this->unknwon_int_0x44 != 0)
		return;
	this->linkCoordinates = *this->unk_0x4C->methods->slot0xD4(this->unk_0x4C, unk_class_86aa0);
	if (!this->vt->StaticWallLink(this, &this->linkCoordinates) && this->unk_0x124 != 0) {
		this->vt->DynamicLink(this);
	}
	this->vt->func_8005B990(this);
	this->vt->func_800590E0(this);
}

void func_800590E0(void) {
}

s32 func_800590E8(DreamSys *this, DreamColors *out, s32 value)
{
	s32 old;

	old = this->isFlashbackSession;
	if (value < 0) {
		*out = CalcDreamColor(&this->moodPreviousDays[this->currentDay]);
	} else {
		this->isFlashbackSession = value;
	}
	return old;
}

void func_80059148(DreamSys *this, s32 value)
{
	this->unk_0x6c = value;
	if (value != 0) {
		this->vt->func_8005A168(this, 1);
		if (this->unk_0x884 != 0)
			this->vt->func_8001CEB4(this, 1, (void *)this->unk_0x884);
	}
}

void func_800591B4(DreamSys *this, s32 arg1, s32 arg2)
{
	struct {
		s8 unknown_values_0x0[8];
		s16 field_0x8;
		s16 field_0xA;
	} local;

	this->vt->LogChunkMood(this, &this->linkCoordinates);
	this->vt->func_8005966C(this, 1);
	this->vt->func_800596E8(this, 1);
	this->vt->func_8005A168(this, arg1);

	this->unk_0xBC = -1;
	this->unk_0xB4 = 0;
	this->unk_0xB8 = 0;
	this->unk_0xA0 = 0;
	this->unk_0xA4 = 0;
	this->unk_0x88 = 0;
	this->unk_0x90 = 0;
	this->unk_0x8C = 0;
	this->unk_0x94 = 0;
	this->vt->func_8005A1B0(this, 0, 1, 1, 1);

	this->vt->func_8005A1EC(this, arg2);

	this->nextCinematic.entry = -1;
	this->unk_0x70 = 0;
	this->unknwon_int_0x44 = 0;
	this->unk_0x74 = 0;
	this->unk_0x908 = 0;
	this->unk_0x90C = 0;
	this->unk_0x910 = 0;
	this->unk_0x78 = 0;
	func_8001E6F8(this, &local);

	local.field_0x8 = 0;
	local.field_0xA = 1;
	this->vt->func_8001CEB4(this, 1, &local);
}

void func_80059310(DreamSys *this)
{
	this->unk_0x70 = 1;
}
s32 func_8005931C(DreamSys *this)
{
	return this->unk_0x74;
}
s32 DreamSys__GetSetDreamTimeLimit(DreamSys *this, s32 value)
{
	s32 result;

	if (value >= 0)
		value = value * 15;
	result = this->dreamTimeLimit;
	this->dreamTimeLimit = value;
	if (result >= 0)
		result = (u32)result / 15;
	return result;
}
s32 func_80059360(DreamSys *this)
{
	return (u32)this->dreamTimer / 15;
}
void func_8005937C(DreamSys *this, s32 value)
{
	this->unk_0x58 = value;
}
void func_80059384(DreamSys *this, void *value)
{
	this->unk_0x5C = value;
}
void func_8005938C(DreamSys *this, s32 value)
{
	this->unk_0x64 = value;
}
void func_80059394(DreamSys *this)
{
	if (this->unk_0x70 == 0) {
		this->unk_0x74 = 0;
		this->unk_0x124 = ((u32)this->dreamTimer % (u32)this->unk_0x120) == 0;
	}
}
void func_800593D8(DreamSys *this)
{
	if (this->callback_0x80 != NULL)
		this->callback_0x80(this);
	if (this->callback_0x98 != NULL)
		this->callback_0x98(this);
}
/* Local prototypes, own local view (func_8001E600 is a different unit's
 * already-matched function taking an unrelated class as arg0; func_8005950C
 * is this unit's own next-in-queue function, forward-declared per
 * CLAUDE.md's convention for calling into a not-yet-preceding definition).
 * arg4 on func_8001E600 is unused by its own body but IS set (to 0) by this
 * call site's own disassembly, so it is declared here to reproduce that. */
extern void func_8001E600(void *self, s32 *dst, s32 *src, s32 arg4);
extern s32 func_8005950C(void *a, void *b, s32 day);
extern s32 func_8001EF14(s32 *a, s32 range, s32 *b);

s32 func_8005942C(DreamSys *this, s32 *out, s32 day, s32 *reference, s32 tolerance)
{
	s32 local[3];
	s32 ret;
	s32 *vec;
	s32 *p;

	p = &D_80087EE8;
	*p = day;
	func_8001E600(this, local, p - 2, 0);

	ret = func_8005950C((void *)((u8 *)this->unk_0x5C + 0x14),
	                     (void *)((u8 *)this->unk_0x5C + 0x20), day);

	vec = this->unk_0xC != 0 ? (s32 *)((u8 *)this->unk_0x14 + 0x38) : 0;
	local[1] = ret + vec[1];

	if (out != NULL) {
		*(DreamSysVec3 *)out = *(DreamSysVec3 *)local;
	}

	if (reference != NULL)
		return func_8001EF14(local, tolerance, reference);
	return 0;
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005950C);

void func_80059590(DreamSys *this)
{
	this->unk_0x7C = 0;
}
void func_80059598(DreamSys *this)
{
	this->unk_0x78 = 0;
}
s32 func_800595A0(DreamSys *this)
{
	return 0;
}
void func_800595A8(DreamSys *this, bool arg1)
{
	this->vt->func_800596E8(this, 0);
	if (arg1)
		this->vt->func_8005966C(this, 0);
}
void func_80059610(DreamSys *this, s32 arg1, s32 arg2)
{
	this->vt->func_800596E8(this, arg1);
	this->vt->func_8005966C(this, arg2);
}

void func_8005966C(DreamSys *this, s32 arg1)
{
	struct vtable_DreamSys *vt = this->vt;

	this->unk_0x84 = arg1;
	switch (arg1) {
	case 0:
		this->callback_0x80 = NULL;
		break;
	case 1:
		this->callback_0x80 = vt->func_800597C0;
		break;
	case 2:
		this->callback_0x80 = vt->func_80059A48;
		break;
	case 3:
		this->callback_0x80 = vt->func_80059A50;
		break;
	}
}

extern void func_8002CC34(s32 arg0, void *arg1, s32 arg2, DreamSys *arg3, void *arg4);

void func_800596E8(DreamSys *this, s32 arg1)
{
	struct vtable_DreamSys *vt = this->vt;

	if (this->unk_0x9C == 2)
		vt->func_8005A134(this, 0);
	this->unk_0x9C = arg1;
	switch (arg1) {
	case 0:
		this->callback_0x98 = NULL;
		break;
	case 1:
		this->callback_0x98 = (void (*)(DreamSys *))vt->func_80059A58;
		break;
	case 2:
		this->callback_0x98 = vt->func_8005A0B0;
		this->unk_0xC4 = 1;
		this->unk_0xC8 = 1;
		func_8002CC34(this->unk_0x58, this->unk_0xCC, 1, this, this->vt->func_8005A1F4);
		break;
	}
}

void func_800597C0(DreamSys *this)
{
	this->vt->func_80059814(this);
	this->vt->func_800598E8(this);
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80059814);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_800598E8);

void func_80059A1C(DreamSys *this)
{
	this->unk_0xA8 = 0;
	if (this->unk_0xA0 != 0) {
		if (this->unk_0xA0 & 1)
			this->unk_0xA0 = this->unk_0xA0 + 1;
		else
			this->unk_0xA0 = this->unk_0xA0 - 1;
	}
}

void func_80059A48(void) {
}

void func_80059A50(void) {
}

s32 func_80059A58(DreamSys *this)
{
	if (this->unk_0x6c == 0) {
		this->vt->func_8005A050(this);
		return this->vt->func_80059AEC(this);
	} else if (this->unk_0x6c != 2) {
		return this->vt->func_80059B50(this);
	} else {
		return this->vt->func_80059BD4(this);
	}
}

s32 func_80059AEC(DreamSys *this)
{
	if (this->unk_0x70 != 0)
		return this->unk_0x70;
	return this->vt->func_80059E98(this, this->vt->func_80059BE0(this, 1));
}

s32 func_80059B50(DreamSys *this)
{
	this->unk_0xA0 = 1;
	if (this->unk_0x70 != 0)
		return this->vt->func_80059BE0(this, 0);
	return this->vt->func_80059E98(this, this->vt->func_80059BE0(this, 1));
}

s32 func_80059BD4(DreamSys *this)
{
	return this->unk_0xA0 = 1;
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80059BE0);

#if 0
/* Best-reached body, 55/72 words -- see docs/match-reports/func_80059D1C.md
   for the residue analysis. Restored to INCLUDE_ASM below per project rule
   (no score short of byte-exact stays in src/). */
void func_80059D1C(DreamSys *this)
{
	DreamSysUnk58 *obj;
	s32 idx;
	DreamSysUnk58Vtable *vt;
	s32 heading;

	obj = (DreamSysUnk58 *)this->unk_0x58;
	vt = obj->vt;
	idx = this->unk_0xB8;
	if (idx == 0) {
		return;
	}

	heading = D_80087EB0[idx];
	heading <<= 4;
	vt->slot0x9C(obj, D_80087EC8[idx]);
	this->unk_0xBC = vt->slot0x80(obj, heading, 0x6E, 0x6E);
	if (this->unk_0xB8 != 0x16) {
		this->unk_0xBC = -1;
	}

	if (this->unk_0xB8 == 0xB) {
		vt->slot0x9C(obj, 1);
		vt->slot0x80(obj, heading, 0x6E, 0x6E);
		vt->slot0x9C(obj, 2);
		vt->slot0x80(obj, 0x90, 0x6E, 0x6E);
	}
}
#endif

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80059D1C);

void func_80059E3C(DreamSys *this)
{
	DreamSysUnk58 *obj;

	if (this->unk_0xBC >= 0) {
		obj = (DreamSysUnk58 *)this->unk_0x58;
		obj->vt->slot0x84(obj, this->unk_0xBC);
		this->unk_0xBC = -1;
	}
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80059E98);

void func_8005A050(DreamSys *this)
{
	s32 idx;

	idx = this->unk_0xA4;
	if (idx != 0) {
		this->vt->func_8001CEB4(this, 0, &D_80087E80[idx]);
		this->unk_0xA4 = 0;
	}
}

void func_8005A0B0(DreamSys *this)
{
	if (this->unk_0xC4 != 0) {
		this->vt->func_800573A8(this, &D_80087EA4);
		this->unk_0x5C->unk_0x24 -= 0x258;
	}
	if (this->unk_0xC8 != 0)
		func_8002CD08(this->unk_0x58, this->unk_0xCC);
}

void func_8005A134(DreamSys *this, s32 arg1)
{
	this->unk_0xC4 = 0;
	this->unk_0xC8 = arg1;
	if (arg1 != 0)
		func_8002CC84(this->unk_0x58, this->unk_0xCC);
}

s32 func_8005A168(DreamSys *this, s32 value)
{
	s32 old;

	old = this->unk_0xAC;
	if (value >= 0) {
		this->unk_0xAC = value;
		this->unk_0xB0 = value;
	}
	return old;
}

void func_8005A184(DreamSys *this, s32 value)
{
	s32 old;

	old = this->unk_0xAC;
	if (old != value) {
		this->unk_0xB0 = old;
		this->unk_0xAC = value;
	}
}

void func_8005A1A4(DreamSys *this)
{
	this->unk_0xAC = this->unk_0xB0;
}

void func_8005A1B0(DreamSys *this, s32 a, s32 b, s32 c, s32 d)
{
	if (a >= 0)
		this->unk_0x124 = a;
	if (b >= 0)
		this->unk_0x128 = b;
	if (c >= 0)
		this->unk_0x12C = c;
	if (d >= 0)
		this->unk_0x130 = d;
}

void func_8005A1EC(DreamSys *this, s32 value)
{
	this->unk_0x120 = value;
}

void func_8005A1F4(void *arg0, Func8005A1F4Arg *arg1)
{
	s32 isDivisible;

	if (arg1->mode == 1) {
		isDivisible = (arg1->value % 20) == 0;
		if (isDivisible) {
			arg1->field_0x1C = 9;
			arg1->field_0x20 = -1;
		} else {
			arg1->field_0x30 = 9;
			arg1->field_0x34 = -1;
		}
	}
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__InitNewGame);

void DreamSys__GetSetScreenShake(DreamSys *this, bool *value)
{
	bool old;

	old = this->screenShakeOn;
	this->screenShakeOn = *value;
	*value = old;
}

s32 func_8005A2E4(DreamSys *this, s32 *arg1)
{
	if (arg1 != NULL)
		*arg1 = this->currentYear;
	return this->currentDay + 1;
}

s32 DreamSys__AdvanceDay(DreamSys *this)
{
	this->currentDay++;
	if (this->currentDay >= 0x16D) {
		this->currentDay = 0;
		this->currentYear++;
	}
	return this->currentDay;
}

void func_8005A33C(DreamSys *this)
{
	this->unk_0x878 = 0;
}

s32 func_8005A344(DreamSys *this)
{
	return this->unk_0x878;
}

s32 *func_8005A350(DreamSys *this, s32 *arg1)
{
	if (arg1 != NULL)
		*arg1 = 0x700;
	return &this->unknown_sdata_0x178;
}

s32 DreamSys__StartDay(DreamSys *this)
{
	s32 oldDay;
	MoodGraphPoint *special;

	oldDay = this->currentDay;
	this->currentFlashbackIndex = 0;
	this->dreamTimer = 0;
	this->storedDay = oldDay;
	if (this->isFlashbackSession) {
		this->vt->LoadNextFlashback(this, 1);
	} else {
		special = IsDaySpecial(&this->nextCinematic, this->currentDay + 1);
		this->vt->InitMoodContibutors(this, special);
		if (special != NULL) {
			return -1;
		}
		this->vt->InitSpawnLoc(this);
	}
	return this->currentStage;
}

s32 DreamSys__EndDay(DreamSys *this, s32 arg1)
{
	this->currentDay = this->storedDay;
	if (!this->isFlashbackSession && arg1 == 0) {
		this->vt->CalcUnlockScore(this);
		this->vt->UpdateDreamChart(this, &this->moodPreviousDays[this->currentDay]);
		this->vt->AdvanceDay(this);
	} else if (arg1 == 2) {
		this->vt->InitNewGame(this);
		this->unk_0x878 = 1;
	}
	return this->isFlashbackSession;
}

CinematicCall DreamSys__GetCinematic(DreamSys *this)
{
	return this->nextCinematic;
}

void DreamSys__InitSpawnLoc(DreamSys *this)
{
	MoodGraphPoint mood;
	s32 timeLimit;

	this->vt->GetPreviousDayMood(this, &mood, 1);
	this->currentStage = GenerateInitialSpawn(&this->linkCoordinates, &timeLimit, &mood, this->currentDay);
	timeLimit = this->vt->GetSetDreamTimeLimit(this, timeLimit);
	this->unknwon_int_0x44 = 0xB;
}

void DreamSys__DynamicLink(DreamSys *this)
{
	s32 stage;

	if (this->unknwon_int_0x44 == 0) {
		stage = GetRandomSpawnFromStage(&this->linkCoordinates, this->currentStage, this->dreamTimer);
		ExecuteLink(this, stage, 0xC, 1);
	}
}

bool DreamSys__StaticWallLink(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	s32 result;

	if (this->unknwon_int_0x44 != 0)
		return false;
	result = TestForStaticLink(&this->linkCoordinates, currentPos, this->currentStage);
	if (result < 0)
		return false;
	ExecuteLink(this, result, 0xD, 1);
	return true;
}

bool DreamSys__LoadNextFlashback(DreamSys *this, bool unknown)
{
	s32 idx;
	FlashbackEntry *entry;

	idx = this->currentFlashbackIndex;
	if (idx >= this->amountFlashbacksAvailable) {
		goto fail;
	}
	this->unknwon_int_0x44 = 0xE;
	entry = &this->storedFlasbacks[idx];
	if (!unknown) {
		this->vt->slot30(this, 0xE);
	}
	this->currentDay = entry->day;
	this->currentStage = entry->stageID;
	this->linkCoordinates = entry->position;
	return true;
fail:
	return false;
}

bool func_8005A700(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	s32 result;
	s32 local[4];

	if (this->unknwon_int_0x44 != 0)
		return false;
	result = Test4TunnelLinks(&this->linkCoordinates, currentPos, this->currentStage);
	if (result < 0)
		return false;
	func_8001E6F8(this, local);
	if (!func_8005BD3C(&this->unk_0x888, &this->unk_0x884, local))
		return false;
	if (this->unk_0xA8 == 0)
		return false;
	ExecuteLink(this, result, 0xF, 0);
	return true;
}

bool func_8005A7A0(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	s32 result;

	if (this->unknwon_int_0x44 != 0)
		return false;
	result = func_8005BE90(&this->linkCoordinates, this->currentStage, currentPos, this->dreamTimer);
	if (result < 0)
		return false;
	this->unk_0x880 = func_8005BF48();
	this->unk_0x884 = 0;
	this->unk_0x888 = 0;
	ExecuteLink(this, result, 0x10, 0);
	return true;
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A82C);

bool ExecuteLink(DreamSys *system, s32 stage, s32 unk1, s32 unk2)
{
	DreamSysUnk58 *obj;

	system->unknwon_int_0x44 = unk1;
	system->vt->slot30(system, unk1);
	if (system->unknwon_int_0x44 == 0) {
		return false;
	}
	system->currentStage = stage;
	if (system->isFlashbackSession) {
		system->dreamTimer = 0;
	}
	if (unk2 != 0) {
		obj = (DreamSysUnk58 *)system->unk_0x58;
		obj->vt->slot0x80(obj, 0x90, 0x6E, 0x6E);
	}
	return true;
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A9CC);

s32 func_8005AB2C(DreamSys *this)
{
	if (this->unk_0x914 == 0) {
		func_8005AF64(this, &D_8008ABC0, &this->unk_0x91C);
	}
	if (this->unk_0xAC != 4) {
		if (this->unk_0x914 >= 0x85)
			return 1;
		if ((u32)(this->unk_0x914 - 0x2B) < 0xF || (u32)(this->unk_0x914 - 0x4B) < 0xF) {
			this->unk_0xA4 = 2;
		}
	} else {
		if (this->unk_0x914 >= 0x13)
			return 1;
		if ((u32)(this->unk_0x914 - 8) < 2 || (u32)(this->unk_0x914 - 0xD) < 2) {
			this->vt->func_8001CEB4(this, 0, &D_80087EFC);
		}
	}
	this->unk_0xA0 = 1;
	this->unk_0x914++;
	return 0;
}

s32 func_8005AC24(DreamSys *this)
{
	s32 flag;

	if (this->unk_0x914 == 0) {
		func_8005AF64(this, &D_8008ABC8, &this->unk_0x91C);
	}
	if (this->unk_0xAC != 4) {
		if (this->unk_0x914 >= 0x95)
			return 1;
		if ((u32)(this->unk_0x914 - 0x16) < 0xF || (u32)(this->unk_0x914 - 0x39) < 0x10 || (u32)(this->unk_0x914 - 0x6E) < 0xF) {
			this->unk_0xA4 = 1;
		}
		flag = (u32)(this->unk_0x914 - 0x39) < 0x35;
	} else {
		if (this->unk_0x914 >= 0x19)
			return 1;
		if ((u32)(this->unk_0x914 - 6) < 2 || (u32)(this->unk_0x914 - 0xB) < 2 || (u32)(this->unk_0x914 - 0x14) < 2) {
			this->vt->func_8001CEB4(this, 0, &D_80087F08);
		}
		flag = (u32)(this->unk_0x914 - 3) < 0xE;
	}
	if (flag) {
		this->unk_0x88 = 2;
	}
	this->unk_0xA0 = 1;
	this->unk_0x914++;
	return 0;
}

s32 func_8005AD68(DreamSys *this)
{
	if (this->unk_0x914 == 0) {
		func_8005AF64(this, &D_8008ABD0, &this->unk_0x91C);
	}
	if (this->unk_0xAC != 4) {
		if (this->unk_0x914 < 0x65) {
			if ((u32)(this->unk_0x914 - 0x2B) < 0xF) {
				this->unk_0xA4 = 2;
			}
		} else {
			return 1;
		}
	} else {
		if (this->unk_0x914 < 15) {
			if ((u32)(this->unk_0x914 - 8) < 2) {
				this->vt->func_8001CEB4(this, 0, &D_80087EFC);
			}
		} else {
			return 1;
		}
	}
	this->unk_0xA0 = 1;
	this->unk_0x914++;
	return 0;
}

s32 func_8005AE40(DreamSys *this)
{
	s32 flag;

	if (this->unk_0x914 == 0) {
		func_8005AF64(this, &D_8008ABD8, &this->unk_0x91C);
	}
	if (this->unk_0xAC != 4) {
		if (this->unk_0x914 >= 0x71)
			return 1;
		if ((u32)(this->unk_0x914 - 0x1E) < 0xF || (u32)(this->unk_0x914 - 0x52) < 0xF) {
			this->unk_0xA4 = 1;
		}
		flag = (u32)(this->unk_0x914 - 0x1E) < 0x34;
	} else {
		if (this->unk_0x914 >= 0x13)
			return 1;
		if ((u32)(this->unk_0x914 - 6) < 2 || (u32)(this->unk_0x914 - 0xF) < 2) {
			this->vt->func_8001CEB4(this, 0, &D_80087F08);
		}
		flag = (u32)this->unk_0x914 < 9;
	}
	if (flag) {
		this->unk_0x88 = 2;
	}
	this->unk_0xA0 = 1;
	this->unk_0x914++;
	return 0;
}

void func_8005AF64(DreamSys *this, struct RelativePos *a, struct RelativePos *b)
{
	DreamSysVec3 diff;

	diff.x = a->x - b->x;
	diff.y = a->y - b->y;
	diff.z = a->z - b->z;
	diff.y = 0;
	this->vt->func_800573A8(this, &diff);
}

s32 func_8005AFD0(DreamSys *this)
{
	return this->currentStage;
}

void DreamSys__ProcessChunkChange(DreamSys *this, void *entity, s32 effect)
{
	PlayerSpawnPoint *pos;

	if (effect == 5) {
		pos = ((DreamSysEntityObj *)entity)->methods->slot0x10C(entity, 0, 0);
		this->vt->LogChunkMood(this, pos);
	}
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__InstanceEffectsOnJournal);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__GetPreviousDayMood);

void DreamSys__InitMoodContibutors(DreamSys *this, MoodGraphPoint *special)
{
	this->vt->ClearMoodGraph(this, &this->areaMoods);
	this->vt->ClearMoodGraph(this, &this->entityMoods);
	if (special != NULL) {
		this->vt->LogMood(this, &this->areaMoods, special);
		this->vt->LogMood(this, &this->entityMoods, special);
	}
}

void DreamSys__LogChunkMood(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	MoodGraphPoint *mood;

	mood = GetMoodFromStageChunk(this->currentStage, (StageChunk *)currentPos);
	this->vt->LogMood(this, &this->areaMoods, mood);
}

void DreamSys__LogInstanceMood(DreamSys *this, MoodGraphPoint *source)
{
	this->vt->LogMood(this, &this->entityMoods, source);
}

void DreamSys__UpdateDreamChart(DreamSys *this, MoodGraphPoint *ret)
{
	MoodGraphPoint areaAvg;
	MoodGraphPoint entityAvg;

	this->vt->GetMoodAverage(this, &this->areaMoods, &areaAvg);
	this->vt->GetMoodAverage(this, &this->entityMoods, &entityAvg);
	if (this->entityMoods.amountMoods == 0) {
		entityAvg.value = areaAvg.value;
	}
	ret->axis.dynamic = (areaAvg.axis.dynamic + entityAvg.axis.dynamic) / 2;
	ret->axis.upper = (areaAvg.axis.upper + entityAvg.axis.upper) / 2;
}

DreamColors DreamSys__GetDreamColor(DreamSys *this)
{
	MoodGraphPoint local;

	this->vt->UpdateDreamChart(this, &local);
	return CalcDreamColor(&local);
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", CalcDreamColor);

void DreamSys__ClearMoodGraph(DreamSys *this, MoodGraphContributor *contributor)
{
	contributor->lastMood.value = 0;
	contributor->sumMoods.upper = 0;
	contributor->sumMoods.dynamic = 0;
	contributor->amountMoods = 0;
}

void DreamSys__LogMood(DreamSys *this, MoodGraphContributor *layer, MoodGraphPoint *mood)
{
	layer->lastMood.value = mood->value;
	layer->sumMoods.dynamic = mood->axis.dynamic + layer->sumMoods.dynamic;
	layer->sumMoods.upper = mood->axis.upper + layer->sumMoods.upper;
	layer->amountMoods = layer->amountMoods + 1;
}

void DreamSys__GetMoodAverage(DreamSys *this, MoodGraphContributor *layer, MoodGraphPoint *ret)
{
	if (layer->amountMoods != 0) {
		ret->axis.dynamic = CalcMoodAxis(layer->lastMood.axis.dynamic, layer->sumMoods.dynamic, layer->amountMoods);
		ret->axis.upper = CalcMoodAxis(layer->lastMood.axis.upper, layer->sumMoods.upper, layer->amountMoods);
	} else {
		ret->value = layer->lastMood.value;
	}
}

s32 CalcMoodAxis(s32 lank, s32 sum, s32 amount)
{
	s32 result;

	result = sum / amount;
	result += lank / 3;
	if (result >= 10)
		result = -9;
	else if (result < -9)
		result = 9;
	return result;
}

void DreamSys__CalcUnlockScore(DreamSys *this)
{
	this->navigationFlasbackUnlockScore = CalcNavigationScore();
	if (this->instanceFlasbackUnlockScore < 0) {
		this->instanceFlasbackUnlockScore = 0;
	} else if (this->instanceFlasbackUnlockScore > 50000000) {
		this->instanceFlasbackUnlockScore = 50000000;
	}
	this->totalFlasbackUnlockScore = this->navigationFlasbackUnlockScore + this->instanceFlasbackUnlockScore;
}

void DreamSys__AddFlashback(DreamSys *this, s32 stage, PlayerSpawnPoint *pos, s32 *angles, s32 unknown, s32 time, s32 day)
{
	FlashbackEntry *entry;

	entry = this->storedFlasbacks;
	if (this->amountFlashbacksAvailable < 10) {
		entry += this->amountFlashbacksAvailable++;
	} else {
		entry += (u32)this->dreamTimer % 9;
	}
	entry->stageID = stage;
	entry->position = *pos;
	entry->rotation = *(FlashbackRotation *)angles;
	entry->unknown_value_0x1c = unknown;
	entry->timeLimit = time;
	entry->day = day;
}

void DreamSys__FlashbackSaving(DreamSys *this, s32 arg1, s32 arg2)
{
	PlayerSpawnPoint *pos;
	s32 local[4];

	if (this->unk_0x4C != NULL && rand() % 3 == 0) {
		pos = this->unk_0x4C->methods->slot0x10C(this->unk_0x4C, 0, 0);
		func_8001E6F8(this, local);
		this->vt->AddFlashback(this, this->currentStage, pos, local, arg1, arg2, this->currentDay);
	}
}

void DreamSys__ResetFlashbackList(DreamSys *this)
{
	this->amountFlashbacksAvailable = 0;
}

void func_8005B904(DreamSys *this)
{
	DreamSysUnk14 *p = this->unk_0x14;

	this->unk14Snapshot = *p;
	this->unk14TailSnapshot = *p->unk_0x44;
}

void func_8005B990(DreamSys *this)
{
	DreamSysUnk14 *p = this->unk_0x14;

	*p = this->unk14Snapshot;
	*p->unk_0x44 = this->unk14TailSnapshot;
	p->unk_0x0 = 0;
}

s32 func_8005BA20(DreamSys *this, s32 value)
{
	s32 old;

	if (value >= 0) {
		old = this->unk_0x924;
		this->unk_0x924 = value;
	} else {
		old = this->unk_0x924;
	}
	return old;
}

struct vtable_DreamSys *Get_vtable_DreamSys(void)
{
	return &DREAMSYS_METHODS;
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", InitNavChallengesArray);

INCLUDE_ASM("asm/nonmatchings/DreamSys", CalcNavigationScore);

s32 func_8005BB14(s32 stage)
{
	return STAGE_TIME_LIMITS[stage];
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", GetRandomSpawnFromStage);

s32 TestForStaticLink(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage)
{
	return GetStaticSpawn(target, currentPos, stage, LEN_STAGE_PERMALINK_TRIGGERS,
	                       STAGE_PERMALINK_TRIGGERS, STAGE_PERMALINK_SPAWNS, 1);
}

s32 Test4TunnelLinks(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage)
{
	return GetStaticSpawn(target, currentPos, stage, D_800889F0,
	                       D_80088980, D_80088820, 1);
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005BD3C);

/* Unit-local reading of the second parameter: the caller (func_8005BD3C)
   passes down a `s32 local[4]` buffer that func_8001E6F8 (code_d294_c) fills
   with a 3-entry WholeFrac_d294 table; the byte offset +4 read here lands on
   that table's `out[1].whole` (a degrees value, per func_8001E6F8's own
   report). This function reads it unsigned (`lhu`), independent of
   WholeFrac_d294's own `s16 whole` -- a second, disjoint view of the same
   bytes, so it is kept local rather than folded into that shared struct. */
typedef struct DirectionCheckArg {
	s8 unk0[4];
	u16 heading;
} DirectionCheckArg;

/* 4-entry cardinal-direction table (12-byte stride); only the first u16 of
   each entry (the angle: 0/90/180/270) is read anywhere in this unit's
   queue. Kept local for the same reason as DirectionCheckArg above. */
typedef struct DirectionTableEntry {
	u16 angle;
	u16 unk2;
	u16 unk4;
	u16 unk6;
	u16 unk8;
	u16 unkA;
} DirectionTableEntry;

extern DirectionTableEntry D_8008875C[];

s32 func_8005BE28(DirectionCheckArg *a0, u8 a1)
{
	s16 diff;

	diff = a0->heading - D_8008875C[a1].angle;
	if (diff >= 181) {
		diff -= 360;
	} else if (diff < -180) {
		diff += 360;
	}
	return (u16)(diff + 44) < 89;
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005BE90);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005BF48);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005BF68);

INCLUDE_ASM("asm/nonmatchings/DreamSys", Test4InstantTeleporters);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005BFC4);

s32 Test4StaircaseNodes(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 arg2)
{
	if (arg2 == 0)
		return GetStaticSpawn(target, currentPos, 0, D_80088CBC,
		                       D_80088C4C, D_80088BA4, 0);
	return -1;
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005C02C);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005C118);

INCLUDE_ASM("asm/nonmatchings/DreamSys", GetStaticSpawn);

INCLUDE_ASM("asm/nonmatchings/DreamSys", GenerateInitialSpawn);

INCLUDE_ASM("asm/nonmatchings/DreamSys", IsDaySpecial);
