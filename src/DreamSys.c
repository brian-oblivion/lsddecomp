#include "common.h"
#include "DreamSys.h"

INCLUDE_ASM("asm/nonmatchings/DreamSys", New_DreamSys);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__DreamSys);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__func_588ec);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__func_58968);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80058A94);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80058B08);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80058C58);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__TimerTick);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80058E8C);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80058F18);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__WallLink);

void func_800590E0(void) {
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_800590E8);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80059148);

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
INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80059610);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005966C);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_800596E8);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_800597C0);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80059814);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_800598E8);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80059A1C);

void func_80059A48(void) {
}

void func_80059A50(void) {
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80059A58);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80059AEC);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80059B50);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80059BD4);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80059BE0);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80059D1C);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80059E3C);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_80059E98);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A050);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A0B0);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A134);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A168);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A184);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A1A4);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A1B0);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A1EC);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A1F4);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__InitNewGame);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__GetSetScreenShake);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A2E4);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__AdvanceDay);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A33C);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A344);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A350);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__StartDay);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__EndDay);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__GetCinematic);

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

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005AFD0);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__ProcessChunkChange);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__InstanceEffectsOnJournal);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__GetPreviousDayMood);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__InitMoodContibutors);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__LogChunkMood);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__LogInstanceMood);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__UpdateDreamChart);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__GetDreamColor);

INCLUDE_ASM("asm/nonmatchings/DreamSys", CalcDreamColor);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__ClearMoodGraph);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__LogMood);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__GetMoodAverage);

INCLUDE_ASM("asm/nonmatchings/DreamSys", CalcMoodAxis);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__CalcUnlockScore);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__AddFlashback);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__FlashbackSaving);

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__ResetFlashbackList);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005B904);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005B990);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005BA20);

INCLUDE_ASM("asm/nonmatchings/DreamSys", Get_vtable_DreamSys);

INCLUDE_ASM("asm/nonmatchings/DreamSys", InitNavChallengesArray);

INCLUDE_ASM("asm/nonmatchings/DreamSys", CalcNavigationScore);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005BB14);

INCLUDE_ASM("asm/nonmatchings/DreamSys", GetRandomSpawnFromStage);

INCLUDE_ASM("asm/nonmatchings/DreamSys", TestForStaticLink);

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
