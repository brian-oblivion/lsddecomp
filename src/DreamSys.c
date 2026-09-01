#include "common.h"
#include "DreamSys.h"

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

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__DreamSys);

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

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__func_58968);

void func_80058A94(DreamSys *this)
{
	this->unk_0x4C->methods->slot0xF0(this->unk_0x4C);
	this->vt->func_80057130(this, this->unk_0x4C);
	func_80057C84()->slot0x50(this);
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80058B08);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80058C58);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__TimerTick);

void func_80058E8C(DreamSys *this, void *arg1, s32 arg2)
{
	func_80057C84()->slot0x9C(this, arg1, arg2);
	if ((*(s32 *)(*(void **)arg1) & 0xFFF) == 0x114) {
		this->vt->ProcessChunkChange(this, arg1, arg2);
	}
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80058F18);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__WallLink);

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

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_800591B4);

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
INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005942C);

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

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A0B0);

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

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A1F4);

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

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__StartDay);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__EndDay);

CinematicCall DreamSys__GetCinematic(DreamSys *this)
{
	return this->nextCinematic;
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__InitSpawnLoc);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__DynamicLink);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__StaticWallLink);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__LoadNextFlashback);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A700);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A7A0);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A82C);

INCLUDE_ASM("asm/nonmatchings/DreamSys", ExecuteLink);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A9CC);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005AB2C);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005AC24);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005AD68);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005AE40);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005AF64);

s32 func_8005AFD0(DreamSys *this)
{
	return this->currentStage;
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__ProcessChunkChange);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__InstanceEffectsOnJournal);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__GetPreviousDayMood);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__InitMoodContibutors);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__LogChunkMood);

void DreamSys__LogInstanceMood(DreamSys *this, MoodGraphPoint *source)
{
	this->vt->LogMood(this, &this->entityMoods, source);
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__UpdateDreamChart);

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

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__LogMood);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__GetMoodAverage);

INCLUDE_ASM("asm/nonmatchings/DreamSys", CalcMoodAxis);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__CalcUnlockScore);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__AddFlashback);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__FlashbackSaving);

void DreamSys__ResetFlashbackList(DreamSys *this)
{
	this->amountFlashbacksAvailable = 0;
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005B904);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005B990);

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

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005BB14);

INCLUDE_ASM("asm/nonmatchings/DreamSys", GetRandomSpawnFromStage);

s32 TestForStaticLink(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage)
{
	return GetStaticSpawn(target, currentPos, stage, LEN_STAGE_PERMALINK_TRIGGERS,
	                       STAGE_PERMALINK_TRIGGERS, STAGE_PERMALINK_SPAWNS, 1);
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", Test4TunnelLinks);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005BD3C);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005BE28);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005BE90);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005BF48);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005BF68);

INCLUDE_ASM("asm/nonmatchings/DreamSys", Test4InstantTeleporters);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005BFC4);

INCLUDE_ASM("asm/nonmatchings/DreamSys", Test4StaircaseNodes);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005C02C);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005C118);

INCLUDE_ASM("asm/nonmatchings/DreamSys", GetStaticSpawn);

INCLUDE_ASM("asm/nonmatchings/DreamSys", GenerateInitialSpawn);

INCLUDE_ASM("asm/nonmatchings/DreamSys", IsDaySpecial);
