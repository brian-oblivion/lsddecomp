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

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__LoadNextFlashback);

/* Forward decl: defined later in this unit (Test4TunnelLinks, still in
   ROM-address order), used here before that point in the file. */
extern s32 Test4TunnelLinks(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage);
/* Still INCLUDE_ASM, outside this runner's range; typed from this call
   site's own register setup (round 2026-09-01-e). The 0x10-byte stack
   buffer `func_8001E6F8` fills and `func_8005BD3C` reads back is of
   unconfirmed element type -- kept as a raw word buffer rather than
   guessing a struct neither function's OWN body confirms. */
extern void func_8001E6F8(DreamSys *this, void *out);
extern s32 func_8005BD3C(s32 *arg0, s32 *arg1, void *arg2);

bool func_8005A700(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	s32 buf[4];
	s32 result;

	if (this->unknwon_int_0x44 != 0)
		return false;
	result = Test4TunnelLinks(&this->linkCoordinates, currentPos, this->currentStage);
	if (result < 0)
		return false;
	func_8001E6F8(this, buf);
	if (func_8005BD3C(&this->unk_0x888, &this->unk_0x884, buf) == 0)
		return false;
	if (this->unk_0xA8 == 0)
		return false;
	ExecuteLink(this, result, 0xF, 0);
	return true;
}

/* Still INCLUDE_ASM, outside this runner's range; typed from this and
   func_8005A700's call sites (round 2026-09-01-e). func_8005BE90's own
   return is a spawn/link index; func_8005BF48 ignores all four of its
   arguments and returns either NULL or `&D_8008ABF0` (see below). */
extern s32 func_8005BE90(PlayerSpawnPoint *target, s32 mode, PlayerSpawnPoint *currentPos, s32 dreamTimer);
/* Takes NO arguments -- confirmed from ITS OWN body (still visible in this
   unit's asm), which never reads $a0-$a3 at all, only a %gp_rel global.
   The call site's own registers ($a0=this, $a1=result, $a2=0x10, $a3=0) are
   argument setup for the FOLLOWING call (ExecuteLink), not this one -- the
   `jal func_8005BF48` itself has a bare `nop` delay slot. Declaring this
   with a false 4-argument signature made GCC emit real arg-setup code
   before this call instead of after it, growing the function by two words
   and drifting everything downstream (round 2026-09-01-e). */
extern void *func_8005BF48(void);

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

INCLUDE_ASM("asm/nonmatchings/DreamSys", ExecuteLink);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005A9CC);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005AB2C);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005AC24);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005AD68);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005AE40);

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

INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__InitMoodContibutors);

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

s32 Test4TunnelLinks(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage)
{
	return GetStaticSpawn(target, currentPos, stage, D_800889F0,
	                       D_80088980, D_80088820, 1);
}

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005BD3C);

INCLUDE_ASM("asm/nonmatchings/DreamSys", func_8005BE28);

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
