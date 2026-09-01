#ifndef CLASS_DREAMSYS
#define CLASS_DREAMSYS

#include "common.h"
/* For StageChunk / GetMoodFromStageChunk, used by DreamSys__LogChunkMood
   (round 2026-08-30-d). */
#include "StageGrid.h"

/* .sbss values. The 7B4B4 sbss segment that actually holds these is still
   plain `data` (un-flipped to dot-form), so it already provides these
   symbols; declaring them `extern` here lets this header be #included
   without a multiple-definition link error. Whoever flips that segment to
   `.data, DreamSys` should drop `extern` here in the same commit. */
extern s8 (*gpNavChallengesComplete)[30];
extern s32 *gpDinamicLinkPenalty;
extern s32 D_8008ACBC;
extern s32 D_8008ACC0;
extern s32 D_8008ACC4;
extern s32 D_8008ACC8;

/* Delta/threshold table pairs consumed by func_80059814 (D_80087E50 /
   D_80087E5C, indexed by DreamSys::unk_0x88) and func_800598E8 (D_80087E68 /
   D_80087E74, indexed by DreamSys::unk_0x90). Index 0 is unused/zero in both
   pairs; indices 1 and 2 are the negative/positive delta and its matching
   threshold. Still raw `nonmatching` data (round 2026-08-30). */
extern s32 D_80087E50[3];
extern s32 D_80087E5C[3];
extern s32 D_80087E68[3];
extern s32 D_80087E74[3];

/* {value, flag} pair array; func_800598E8 always writes index 0's value and
   passes &D_80087E84[-1] (== &D_80087E80, a distinct label immediately
   before it) to func_8001CEB4. Still raw `nonmatching` data. */
typedef struct D_80087E84Entry {
	s16 value;
	s16 flag;
} D_80087E84Entry;
extern D_80087E84Entry D_80087E84[8];

/* Address-of only (never loaded through) by DreamSys__func_588ec, forwarded
   as func_8001CEB4's arg2. Still raw `nonmatching` data; element type/count
   unconfirmed (round 2026-08-30-b). */
extern u8 D_80087E08[];

/* 12-byte-stride table, address-of only (never loaded through) by
   func_8005A050, indexed by DreamSys::unk_0xA4 and forwarded as
   func_8001CEB4's arg2. Splat resolves this to the SAME symbol name as
   &D_80087E84[-1] (see that field's comment above), but the two call sites
   use incompatible strides (4 vs 12 bytes) -- likely two unrelated globals
   that just happen to sit at adjacent addresses, not one shared array.
   Element layout unconfirmed; still raw `nonmatching` data
   (round 2026-08-30-b). */
typedef struct D_80087E80Entry {
	s32 unk0;
	s32 unk4;
	s32 unk8;
} D_80087E80Entry;
extern D_80087E80Entry D_80087E80[];

typedef struct CinematicCall{
	s16 bank;
	s16 entry;
} CinematicCall;

typedef struct {
	MoodGraphPoint lastMood;
	/* 2 bytes unused */
	struct sumAxis{
		s32 dynamic;
		s32 upper;
	} sumMoods;
	s32 amountMoods;
} MoodGraphContributor;

typedef struct PlayerSpawnPoint {
	struct MapChunk {
		u8 col;
		u8 row;
	} chunk;
	struct MapTile {
		u8 col;
		u8 row;
	} tile;
	struct RelativePos {
		s16 x;
		s16 y;
		s16 z;
	} position;
} PlayerSpawnPoint;

typedef struct {
	s32 stageID;
	PlayerSpawnPoint position;
	struct Angle{
		s16 angle;
		s16 one;
	} pitch; /* Does not do what you think it does */
	struct Angle heading;
	struct Angle roll; /* Ditto */
	s16 timeLimit;
	s32 unknown_value_0x1c;
	s32 day;
} FlashbackEntry;

/* Struct pointed to by DreamSys::unk_0x5C. +0x14 / +0x20 are a pair of
   two-word (x,y) points per func_8005942C (still INCLUDE_ASM elsewhere in
   this unit; not confirmed by this round). +0x24 (the second point's y) is
   confirmed: func_80059814 (round 2026-08-30) nudges it. */
typedef struct DreamSysUnk5C {
	s8 unknown_values_0x0[0x24];
	s32 unk_0x24;
} DreamSysUnk5C;

/* Object pointed to by DreamSys::unk_0x58, used ONLY by func_80059E3C (this
   round): loaded, dereferenced for its own vtable pointer at offset 0, and
   called through slot +0x84. Everything else about this class -- including
   whether it is the SAME class as DreamSys::unk_0x4C below -- is unknown.
   Elsewhere in this unit unk_0x58 is set/read as a plain s32
   (func_8005937C, func_8005A134's call into func_8002CC84), which is
   consistent with it being a pointer value just not typed that way there.
   slot0x84 takes TWO arguments, not one -- head-adjudicated 2026-08-30-c:
   the guard value (DreamSys::unk_0xBC) loaded into $a1 by func_80059E3C is
   never overwritten before the jalr, so it is passed through, not just
   branched on. See func_80059E3C.md. */
typedef struct DreamSysUnk58Vtable {
	u8 pad00[0x84];
	void (*slot0x84)(void *self, s32 flag);
} DreamSysUnk58Vtable;
typedef struct DreamSysUnk58 {
	DreamSysUnk58Vtable *vt;
} DreamSysUnk58;

/* Object pointed to by DreamSys::unk_0x4C, used ONLY by func_80058A94 (this
   round): same "vtable pointer at offset 0" shape as DreamSysUnk58 above,
   but slot +0xF0 instead. Unidentified class; unknown if related to
   DreamSysUnk58. */
typedef struct DreamSysUnk4CMethods {
	u8 pad00[0xF0];
	void (*slot0xF0)(void *self);
} DreamSysUnk4CMethods;
typedef struct DreamSysUnk4CObj {
	DreamSysUnk4CMethods *methods;
} DreamSysUnk4CObj;

/* Shared intermediate base class table (D_800878D4 -- see
   docs/research/class-framework.md and code_55dd4.h's D800878D4Methods,
   which types the same table for Class65650, a sibling of DreamSys under
   this same base). Declared locally here rather than pulled in from
   code_55dd4.h to avoid a cross-unit include; only slot +0x050 is needed by
   this unit (func_80058A94, this round). */
typedef struct DreamSysBaseMethods {
	u8 pad00[0x50];
	/* Deliberately `struct DreamSys *`, not `DreamSys *` -- this precedes
	   the real `typedef struct DreamSys {...}` below, so GCC 2.6.3 warns
	   "declared inside parameter list ... probably not what you want" and
	   scopes a distinct tag here. A forward `typedef struct DreamSys
	   DreamSys;` to avoid the warning does NOT work: this compiler treats
	   the later real typedef as a "redefinition of DreamSys" error instead.
	   The warning is cosmetic -- both tags are pointer-compatible at the
	   ABI level, and the call site casts implicitly with no codegen
	   difference (round 2026-08-30-b). */
	void (*slot0x50)(struct DreamSys *self);
	u8 pad54[0x9C - 0x54];
	/* Called by func_80058E8C as (this, arg1, arg2) -- round 2026-08-30-d. */
	void (*slot0x9C)(struct DreamSys *self, void *arg1, s32 arg2);
} DreamSysBaseMethods;
extern DreamSysBaseMethods *func_80057C84(void);

/* Opaque view of whatever object DreamSys__ProcessChunkChange's `entity`
   parameter points to -- almost certainly an `Entity*` (include/Entity.h),
   but that unit's own `EntityMethods` doesn't type this slot (+0x10C) and
   extending it is out of this unit's scope. Declared minimally, locally,
   for this one call site only (round 2026-08-30-d). */
typedef struct DreamSysEntityMethods {
	u8 pad00[0x10C];
	PlayerSpawnPoint *(*slot0x10C)(void *self, s32 arg1, s32 arg2);
} DreamSysEntityMethods;
typedef struct DreamSysEntityObj {
	DreamSysEntityMethods *methods;
} DreamSysEntityObj;

/* Struct pointed to by func_8005A1F4's arg1 -- forwarded (never called) as
   func_8002CC34's 5th argument via func_800596E8's arg1==2 case, so its
   real caller/owner lives outside this unit. Only the offsets
   func_8005A1F4 itself touches are named; +0x8..+0x1C and +0x24..+0x30
   are unconfirmed gaps (round 2026-08-30-d). */
typedef struct Func8005A1F4Arg {
	s32 mode;             /* +0x0, compared against literal 1 */
	s32 value;              /* +0x4, divided by 20 */
	s8 unknown_values_0x8[0x14];
	s32 field_0x1C;
	s32 field_0x20;
	s8 unknown_values_0x24[0xC];
	s32 field_0x30;
	s32 field_0x34;
} Func8005A1F4Arg;

/* Full-word (x,y,z) vector, distinct from `struct RelativePos` (s16 triplet
   -- the on-disk/network form). func_8005AF64 builds one of these on the
   stack as a-b with y forced to 0; func_8005A0B0 passes the static
   D_80087EA4 instance of one. Both feed vtable slot +0xBC
   (func_800573A8, round 2026-08-30-d). */
typedef struct DreamSysVec3 {
	s32 x, y, z;
} DreamSysVec3;
extern DreamSysVec3 D_80087EA4;

/* Argument shape for func_8005950C: two "keyframe" points, each with a
   value (+0x4) and a position/time (+0x8); offset +0x0 unconfirmed
   (unread by this function). Called by still-INCLUDE_ASM func_8005942C as
   func_8005950C(&this->unk_0x5C->unknown_values_0x0[0x14], arg2, arg3) --
   the first argument is one of DreamSysUnk5C's two documented "point"
   fields (round 2026-08-30-d). */
typedef struct DreamSysInterpPoint {
	s8 unknown_values_0x0[4];
	s32 value;
	s32 position;
} DreamSysInterpPoint;

/* 3x3 lookup table indexed by [dynamicClass][upperClass], each axis
   classified into {0,1,2} by CalcDreamColor first (round 2026-08-30-d). */
extern s8 D_80087E14[9];

/* BasicClass-family allocator; see code_171e0.h / code_55dd4.h / Entity.h /
   class_16334.h for the other units that also declare it locally. */
extern void *func_80017B34(s32 size);

/* Also declared in Entity.h for a different (Entity) struct's fields; here
   called by func_8005A134 as (this->unk_0x58, this->unk_0xCC)
   (round 2026-08-30-b). */
extern void func_8002CC84(s32 arg0, void *arg1);

/* Also declared in Entity.h. Called by func_8005A0B0 as
   (this->unk_0x58, this->unk_0xCC) -- same argument shape as
   func_8002CC84 above (round 2026-08-30-d). */
extern void func_8002CD08(s32 arg0, void *arg1);

typedef struct DreamSys {
	struct vtable_DreamSys *vt;
	s8 unknown_values_0x4[8];

	/* Gate observed by func_8001E600 and func_8005942C: when nonzero,
	   unk_0x14 is treated as valid and its "+0x38" vector is used;
	   otherwise a zero vector is used instead. Meaning of the pointed-to
	   struct is unidentified. */
	s32 unk_0xC;
	s8 unknown_values_0x10[4];

	/* Pointer to an unidentified struct; a 3-word vector lives at +0x38
	   of what this points to (read by func_8001E600 / func_8005942C,
	   guarded by unk_0xC above). */
	void *unk_0x14;
	s8 unknown_values_0x18[12];

	s32 dreamTimer;
	s8 unknown_values_0x28[28];

	s32 unknwon_int_0x44;
	s8 unknown_values_0x48[4];
	/* Pointer to an unidentified object (own vtable at offset 0, slot
	   +0xF0 called with itself as the sole argument). Used by
	   func_80058A94 (round 2026-08-30-b); see DreamSysUnk4CObj above. */
	DreamSysUnk4CObj *unk_0x4C;
	s8 unknown_values_0x50[8];

	/* Set by func_8005937C(this, value); no other observed use. */
	s32 unk_0x58;
	/* Set by func_80059384(this, value); read as a pointer by
	   func_8005942C (this->unk_0x5C + 0x14 and + 0x20 are passed to
	   func_8005950C), so it points to a pair of two-word (x,y) points. */
	DreamSysUnk5C *unk_0x5C;
	s8 unknown_values_0x60[4];
	/* Set by func_8005938C(this, value); no other observed use. */
	s32 unk_0x64;

	bool isFlashbackSession;
	/* Read by func_80059A58; compared against 0 / 1, else-branch otherwise.
	   Meaning unidentified beyond that (round 2026-08-30). */
	s32 unk_0x6c;

	/* Gate flag: func_80059310 sets it to 1; func_8005931C reads it back;
	   func_80059394 skips its whole body while this is nonzero. */
	s32 unk_0x70;
	/* Cleared to 0, then set to (dreamTimer % unk_0x120 == 0) by
	   func_80059394. */
	s32 unk_0x74;
	/* Cleared to 0 by func_80059598; no other observed use. */
	s32 unk_0x78;
	/* Cleared to 0 by func_80059590; no other observed use. */
	s32 unk_0x7C;
	/* Set by func_8005966C(this, arg1): NULL when arg1==0, otherwise one of
	   three vtable-slot function pointers selected by arg1 (1/2/3). Called
	   with (this) by func_800593D8, if non-NULL. */
	void (*callback_0x80)(struct DreamSys *this);
	/* Set unconditionally to arg1 by func_8005966C(this, arg1); no other
	   observed use (round 2026-08-30). */
	s32 unk_0x84;
	/* Index into the (D_80087E50, D_80087E5C) delta/threshold table pair,
	   consumed and reset to 0 by func_80059814 (round 2026-08-30). */
	s32 unk_0x88;
	/* Running accumulator nudged by unk_0x88's table entry, or decayed by
	   600/call towards 0 when unk_0x88 is 0; also propagated into
	   unk_0x5C->unk_0x24. Set by func_80059814 (round 2026-08-30). */
	s32 unk_0x8C;
	/* Index into the (D_80087E68, D_80087E74) delta/threshold table pair,
	   consumed and reset to 0 by func_800598E8 (round 2026-08-30). */
	s32 unk_0x90;
	/* Running delta accumulator paired with unk_0x90; see func_800598E8
	   (round 2026-08-30). */
	s32 unk_0x94;
	/* Set by func_8005966C(this, arg1) exactly like callback_0x80, but from
	   a *different* trio of vtable slots. Called with (this) by
	   func_800593D8, if non-NULL. */
	void (*callback_0x98)(struct DreamSys *this);
	/* "Mode" field read/written by func_800596E8(this, arg1): when ==2 on
	   entry, this->vt->func_8005A134(this, 0) fires first; then it is set
	   unconditionally to arg1 (round 2026-08-30). */
	s32 unk_0x9C;
	/* (this->unk_0xA0 ^ 1) < 1u, i.e. (unk_0xA0 == 1), written by
	   func_800598E8; also toggled/incremented by func_80059A1C and forced
	   to 1 by func_80059B50 (round 2026-08-30). */
	s32 unk_0xA0;
	/* Index into the 12-byte-stride D_80087E80 table; consumed and reset
	   to 0 by func_8005A050 (round 2026-08-30-b). */
	s32 unk_0xA4;
	/* (unk_0xA0 == 1) as computed by func_800598E8; unconditionally cleared
	   to 0 by func_80059A1C on every call (round 2026-08-30). */
	s32 unk_0xA8;
	/* "Current" value; func_8005A1A4 overwrites this with unk_0xB0.
	   func_8005A168's bounds-checked setter (vtable +0x180) writes both
	   this and unk_0xB0 together; func_8005A184 copies the OLD value of
	   this into unk_0xB0 before overwriting it, when the new value
	   differs (round 2026-08-30-b). */
	s32 unk_0xAC;
	/* "Previous"/paired value; see unk_0xAC (round 2026-08-30-b). */
	s32 unk_0xB0;
	s8 unknown_values_0xB4[8];
	/* Gate flag: func_80059E3C runs its body (a call through
	   unk_0x58->vt->slot0x84, then resets this to -1) only while this is
	   >= 0 (round 2026-08-30-b). */
	s32 unk_0xBC;
	s8 unknown_values_0xC0[4];
	/* Set to 1 by func_800596E8's arg1==2 case, alongside unk_0xC8 and
	   callback_0x98 (round 2026-08-30). */
	s32 unk_0xC4;
	/* Set to 1 by func_800596E8's arg1==2 case, alongside unk_0xC4
	   (round 2026-08-30). */
	s32 unk_0xC8;
	/* Struct initialized in-place by func_8002CC34 (still INCLUDE_ASM, in
	   the uncarved code_179d8) via func_800596E8's arg1==2 case; internal
	   layout unknown beyond that entry point (round 2026-08-30). */
	s8 unk_0xCC[0x54];

	/* Divisor for func_80059394's (dreamTimer % unk_0x120) check. */
	s32 unk_0x120;
	/* Result of func_80059394's (dreamTimer % unk_0x120 == 0) check. */
	s32 unk_0x124;
	/* unk_0x124/0x128/0x12C/0x130 are also bounds-checked-set as a group of
	   four by func_8005A1B0 (vtable +0x18C): each is overwritten with the
	   corresponding argument only when that argument is >= 0
	   (round 2026-08-30-b). */
	s32 unk_0x128;
	s32 unk_0x12C;
	s32 unk_0x130;

	s32 dreamTimeLimit;
	s8 unknown_values_0x138[12];

	MoodGraphContributor areaMoods;
	MoodGraphContributor entityMoods;
	s32 currentStage;
	CinematicCall nextCinematic;
	PlayerSpawnPoint linkCoordinates;
	/* 2 bytes unused */
	s32 unknown_sdata_0x178;
	s32 currentYear;
	s32 currentDay;
	s32 totalFlasbackUnlockScore;
	s32 navigationFlasbackUnlockScore;
	s32 instanceFlasbackUnlockScore;
	MoodGraphPoint moodPreviousDays[365];
	/* 2 bytes unused */
	s32 amountFlashbacksAvailable;
	FlashbackEntry storedFlasbacks[10];

	s8 unknown_values_0x5d8[8];

	s8 navChallengesArray[30];
	/* 2 bytes unused */
	s32 amountDynamicLinksDone;
	s8 unknown_values_0x604[116];

	bool screenShakeOn;
	s32 unknown_word_0x67c;
	s32 unknown_word_0x680;
	s8 unknown_values_0x684[500];

	s32 unk_0x878;
	s32 currentFlashbackIndex;
	/* Set by func_8005A7A0 from func_8005BF48's return, which is itself
	   either 0 or `&D_8008ABF0` -- pointer-shaped, not a plain word
	   (round 2026-09-01-e). */
	void *unk_0x880;
	/* Gate flag read by func_80059148 (round 2026-08-30-b): when nonzero
	   (reusing the SAME loaded value, not a fresh 0/1 test), forwarded as
	   func_8001CEB4's arg2 -- cast from s32 to void*, not dereferenced. */
	s32 unk_0x884;
	/* Cleared to 0 by func_8005A7A0 alongside unk_0x880/unk_0x884; no other
	   observed use (round 2026-09-01-e). */
	s32 unk_0x888;

	s32 storedDay;

	/* The struct previously ended here (0x890), but New_DreamSys allocates
	   sizeof(DreamSys) via a literal `ori $a0, $zero, 0x928` -- 0x98 bytes
	   more than any field so far discovered accounts for. Extended to the
	   allocator's real size (round 2026-08-30-b); the four words
	   DreamSys__func_588ec clears are named, the rest of the tail is still
	   unclaimed. */
	s8 unknown_values_0x890[0x78];
	s32 unk_0x908;
	s32 unk_0x90C;
	s32 unk_0x910;
	s8 unknown_values_0x914[0x10];
	s32 unk_0x924;
} DreamSys;

struct vtable_DreamSys{
	u32 unknown_int;
	void *extfunc_17eb0;
	/* Called by New_DreamSys as (allocation, arg0, arg1, arg2) -- see
	   New_DreamSys, round 2026-08-30-b. Return value is discarded there
	   (New_DreamSys returns the allocation regardless). Still INCLUDE_ASM
	   (DreamSys__DreamSys); types here are New_DreamSys's own forwarded
	   parameter types, not independently confirmed by this round. */
	DreamSys *(*Constructor)(DreamSys *this, void *arg1, s32 arg2, s32 arg3);
	u32 unknown_functions_0xc[2];
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x014); shared
	   with Class65650's inherited slot14 (code_55dd4.h: "'unlink' companion
	   of slot10"). Called by func_80058A94 as (this, this->unk_0x4C)
	   (round 2026-08-30-b). Still INCLUDE_ASM; address 0x80057130 is
	   outside this unit/runner's range. */
	void (*func_80057130)(DreamSys *this, DreamSysUnk4CObj *arg1);
	u32 unknown_functions_0x18[10];
	void *func_800588EC;
	/* Called by func_800598E8 as (this, 0, &D_80087E84[-1]); return value,
	   if any, unused (round 2026-08-30). */
	void (*func_8001CEB4)(DreamSys *this, s32 arg1, void *arg2);
	u32 unknown_functions_0x48[1];
	void *func_58968;
	u32 unknown_functions_0x50[4];
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x060). Called by
	   DreamSys__func_588ec as (this, 0) (round 2026-08-30-b). Still
	   INCLUDE_ASM; address 0x8001D344 is outside this unit/runner's
	   range. */
	void (*func_8001D344)(DreamSys *this, s32 arg1);
	u32 unknown_functions_0x64[13];
	void *TimerTick;
	/* This function's OWN slot; resolved via tools/classtable.py
	   (round 2026-08-30-d). */
	void (*func_80058E8C)(DreamSys *this, void *arg1, s32 arg2);
	u32 unknown_functions_0xa0[7];
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x0BC). Called by
	   func_8005AF64 and func_8005A0B0 with a DreamSysVec3* second argument
	   (round 2026-08-30-d). */
	void (*func_800573A8)(DreamSys *this, DreamSysVec3 *arg1);
	u32 unknown_functions_0xc0[8];
	void *LinkWall;
	u32 unknown_functions_0xe4[6];
	void (*func_80059310)(DreamSys *this);
	s32 (*func_8005931C)(DreamSys *this);
	s32 (*GetSetDreamTimeLimit)(DreamSys *this, s32 time);
	s32 (*func_80059360)(DreamSys *this);
	void (*func_8005937C)(DreamSys *this, s32 value);
	void (*func_80059384)(DreamSys *this, void *value);
	void (*func_8005938C)(DreamSys *this, s32 value);
	void (*func_80059394)(DreamSys *this);
	void (*func_800593D8)(DreamSys *this);
	s32 (*func_8005942C)(DreamSys *this, void *out, s32 day, s32 *arg3);
	void (*func_80059590)(DreamSys *this);
	void (*func_80059598)(DreamSys *this);
	s32 (*func_800595A0)(DreamSys *this);
	void (*func_800595A8)(DreamSys *this, bool arg1);
	/* Chains func_800596E8(this, arg1) then func_8005966C(this, arg2)
	   (round 2026-08-30). */
	void (*func_80059610)(DreamSys *this, s32 arg1, s32 arg2);
	/* Called by func_800595A8(this, TRUE) as this->vt->func_8005966C(this, 0). */
	void (*func_8005966C)(DreamSys *this, s32 arg1);
	/* Called unconditionally by func_800595A8 as this->vt->func_800596E8(this, 0). */
	void (*func_800596E8)(DreamSys *this, s32 arg1);
	/* Calls func_80059814(this) then func_800598E8(this) (round 2026-08-30). */
	void (*func_800597C0)(DreamSys *this);
	void (*func_80059814)(DreamSys *this);
	void (*func_800598E8)(DreamSys *this);
	/* No-op stub (`{ }`); one of func_8005966C's callback_0x80 choices. */
	void (*func_80059A48)(DreamSys *this);
	/* No-op stub (`{ }`); one of func_8005966C's callback_0x80 choices. */
	void (*func_80059A50)(DreamSys *this);
	/* Dispatches on unk_0x6C to func_8005A050+func_80059AEC, func_80059BD4,
	   or func_80059B50, and returns whichever's result (round 2026-08-30). */
	s32 (*func_80059A58)(DreamSys *this);
	/* Returns unk_0x70 unchanged if nonzero; otherwise calls func_80059BE0
	   and func_80059E98 in sequence and returns the latter's result
	   (round 2026-08-30). */
	s32 (*func_80059AEC)(DreamSys *this);
	/* Forces unk_0xA0 to 1; then either calls func_80059BE0(this, 0) and
	   returns its result, or chains func_80059BE0(this, 1) into
	   func_80059E98 and returns THAT result (round 2026-08-30). */
	s32 (*func_80059B50)(DreamSys *this);
	/* Sets unk_0xA0 to 1 and returns 1 (round 2026-08-30). */
	s32 (*func_80059BD4)(DreamSys *this);
	/* Referenced by func_80059AEC/func_80059B50; still INCLUDE_ASM outside
	   this runner's range. Return value is threaded into func_80059E98. */
	s32 (*func_80059BE0)(DreamSys *this, s32 arg1);
	u32 unknown_functions_0x168[1];
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x16C). Not
	   called through the vtable by any function in this round; named
	   because its slot is now known (round 2026-08-30-b). */
	void (*func_80059E3C)(DreamSys *this);
	/* Referenced by func_80059AEC/func_80059B50; still INCLUDE_ASM. */
	s32 (*func_80059E98)(DreamSys *this, s32 arg1);
	/* Referenced by func_80059A58; still INCLUDE_ASM. Called with (this)
	   only, return value discarded. */
	void (*func_8005A050)(DreamSys *this);
	/* Referenced by func_800596E8's arg1==2 case; stored into
	   callback_0x98, never called directly by this runner's functions. */
	void (*func_8005A0B0)(DreamSys *this);
	/* Referenced by func_800596E8's entry guard (this->unk_0x9C==2); called
	   as (this, 0). */
	void (*func_8005A134)(DreamSys *this, s32 arg1);
	/* +0x180..+0x190: resolved via tools/classtable.py DREAMSYS_METHODS,
	   all five matched this round (2026-08-30-b). func_8005A168 is also
	   called directly (not through the vtable) by func_80059148, as
	   (this, 1). */
	s32 (*func_8005A168)(DreamSys *this, s32 value);
	void (*func_8005A184)(DreamSys *this, s32 value);
	void (*func_8005A1A4)(DreamSys *this);
	void (*func_8005A1B0)(DreamSys *this, s32 a, s32 b, s32 c, s32 d);
	void (*func_8005A1EC)(DreamSys *this, s32 value);
	/* Referenced by func_800596E8's arg1==2 case: its raw address (never
	   called there) is forwarded as func_8002CC34's 5th argument. arg0 is
	   unused in the body; kept generic rather than typed DreamSys* since
	   nothing here confirms it (round 2026-08-30-d). */
	void (*func_8005A1F4)(void *arg0, Func8005A1F4Arg *arg1);
	void (*InitNewGame)(DreamSys *this);
	void (*GetSetScreenShake)(DreamSys *this, bool *value);
	/* Called by Class6D3C8's slot58 (func_80026410, src/code_1677c.c) as
	   this->vt->func_8005A2E4(this, 0), compared against 1. Real name and
	   full semantics unknown outside that one call site. arg1 is an OUTPUT
	   pointer (this->currentYear is written through it when non-NULL), not
	   a plain s32 -- retyped round 2026-08-30-c; the one external call site
	   passes literal 0, compatible with either. */
	s32 (*func_8005A2E4)(DreamSys *this, s32 *arg1);
	s32 (*AdvanceDay)(DreamSys *this);
	/* Zeroes unk_0x878 unconditionally (round 2026-08-30-c). */
	void (*func_8005A33C)(DreamSys *this);
	/* Getter for unk_0x878 (round 2026-08-30-c). */
	s32 (*func_8005A344)(DreamSys *this);
	/* Optionally writes a literal 0x700 through arg1 (if non-NULL), always
	   returns &this->unknown_sdata_0x178 (round 2026-08-30-c). */
	s32 *(*func_8005A350)(DreamSys *this, s32 *arg1);
	s32 (*StartDay)(DreamSys *this);
	s32 (*EndDay)(DreamSys *this, s32 arg1);
	CinematicCall (*GetCinematic)(DreamSys *this);
	void (*InitSpawnLoc)(DreamSys *this);
	void (*DynamicLink)(DreamSys *this);
	bool (*StaticWallLink)(DreamSys *this, PlayerSpawnPoint *currentPos);
	bool (*LoadNextFlashback)(DreamSys *this, bool unknown);
	u32 unknown_functions_0x1d0[4];
	/* Getter for currentStage (round 2026-08-30-c). */
	s32 (*func_8005AFD0)(DreamSys *this);
	void (*ProcessChunkChange)(DreamSys *this, void *entity, s32 effect);
	void (*InstanceEffectsOnPlayer)(DreamSys *this, void *entity, int effect);
	void (*GetPreviousDayMood)(DreamSys *this, MoodGraphPoint *target, bool unknown);
	void (*InitMoodContibutors)(DreamSys *this, MoodGraphPoint *special);
	void (*LogChunkMood)(DreamSys *this, PlayerSpawnPoint *currentPos);
	void (*LogInstanceMood)(DreamSys *this,MoodGraphPoint *source);
	void (*UpdateDreamChart)(DreamSys *this, MoodGraphPoint *ret);
	s32 (*GetDreamColor)(DreamSys *this);
	void (*ClearMoodGraph)(DreamSys *this, MoodGraphContributor *contributor);
	void (*LogMood)(DreamSys* this, MoodGraphContributor* layer, MoodGraphPoint* mood);
	void (*GetMoodAverage)(DreamSys *this, MoodGraphContributor *layer, MoodGraphPoint *ret);
	void (*CalcUnlockScore)(DreamSys *this);
	void *GameManager__AddFlashback;
	void *GameManager__FlashbackSaving;
	void (*ResetFlashbackList)(DreamSys *this);
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x220/+0x224);
	   both do block-copies of storedFlasbacks-adjacent memory (this+0x890,
	   this+0x8E0 -- inside this round's newly-extended struct tail) driven
	   by a length read from this->unk_0x14. Out of scope this round;
	   named only, not typed beyond `void *` (round 2026-08-30-c). */
	void *func_8005B904;
	void *func_8005B990;
	/* This field is named `func_228`, not `func_8005BA20`, even though it
	   IS func_8005BA20's slot (resolved via tools/classtable.py this
	   round) -- src/code_1677c.c (a different unit, out of this runner's
	   scope) already references it by this name
	   (`self->dreamSys->vt->func_228(...)`), and renaming the field would
	   require an out-of-scope edit there. Do not "fix" this name without
	   updating that call site in the same commit.
	   This slot was previously thought to sit PAST a documented struct end
	   at 0x21c; that was also wrong -- it directly follows
	   ResetFlashbackList/func_8005B904/func_8005B990 above, no gap.
	   Original call-site note preserved: called once, from Class6D3C8's
	   constructor (func_80025FDC in src/code_1677c.c), as
	   this->vt->func_228(this, arg->unk14) right after DreamSys is
	   allocated by New_DreamSys -- the call site's own signature (single
	   s32 arg, return value discarded) matches func_8005BA20's own
	   (this, s32 value) -> s32 get/set exactly, hence the retype from
	   `void (*)(DreamSys*, s32)` to `s32 (*)(DreamSys*, s32)` (a discarded
	   non-void return in a bare statement is legal C either way, so this
	   retype does not require touching code_1677c.c). */
	s32 (*func_228)(DreamSys *this, s32 arg1);
};

typedef enum DreamColors{
	DREAM_COLOR_BLACK, DREAM_COLOR_BLUE,
	DREAM_COLOR_GREEN, DREAM_COLOR_CYAN,
	DREAM_COLOR_RED, DREAM_COLOR_PINK,
	DREAM_COLOR_YELLOW, DREAM_COLOR_WHITE,
}DreamColors;

typedef struct StageSpawn{
	struct MapChunk chunk;
	struct MapTile tile;
	s8 adjustment;
	s8 extra;
}StageSpawn;

typedef struct StaticLinkTrigger{
	struct MapChunk chunk;
	union TriggerTile{
		struct MapTile axis;
		s16 value;
	} tile;
	s8 stage;
	s8 spawnpointIndex;
}StaticLinkTrigger;

/* Jumptable holding all of DreamSys "virtual" methods */
extern struct vtable_DreamSys DREAMSYS_METHODS;

extern s16 STAGE_TIME_LIMITS[];

extern struct RelativePos SPAWN_POS_ADJUST[];

extern StageSpawn* STAGE_SPAWNPOINTS[];
extern s8 LEN_STAGE_SPAWNPOINTS[];

extern StageSpawn* STAGE_PERMALINK_SPAWNS[];
extern StaticLinkTrigger* STAGE_PERMALINK_TRIGGERS[];
extern s8 LEN_STAGE_PERMALINK_TRIGGERS[];

extern s16 SPECIAL_DAYS[];

extern s8 SPECIAL_COLORS[];

/* Shared by TestForStaticLink/Test4TunnelLinks/Test4StaircaseNodes/
   Test4InstantTeleporters, each of which forwards its own three args
   straight through and appends a fixed trailing quadruple (length table,
   trigger table, spawn table, literal 1). Still INCLUDE_ASM; return type is
   a guess (s32, compared with `bltz` at DreamSys__StaticWallLink's call
   site) -- CLAUDE.md's tail-call-wrapper warning applies: byte match alone
   proves nothing about it (round 2026-08-30-c). */
extern s32 GetStaticSpawn(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage,
                           s8 *triggerLens, StaticLinkTrigger **triggers, StageSpawn **spawns, s32 flag);

/* Table triple for Test4TunnelLinks (round 2026-08-30-d), same roles as the
   STAGE_PERMALINK_* triple above but for tunnel links specifically. */
extern s8 D_800889F0[];
extern StaticLinkTrigger* D_80088980[];
extern StageSpawn* D_80088820[];

/* Table triple for Test4StaircaseNodes (round 2026-08-30-d). */
extern s8 D_80088CBC[];
extern StaticLinkTrigger* D_80088C4C[];
extern StageSpawn* D_80088BA4[];

/* This function might be called when the player hits a wall?
It tries to do an static link first, then a dynamic one */
void DreamSys__WallLink(DreamSys *this, void* unk_class_86aa0, int arg2);

/* @brief Sets the overall time limit for the dream and returns its previous value. */
/* @param value The new time limit, in seconds. Negative values are stored as-is. */
/* @return The previous time limit, in seconds, or -1. */
s32 DreamSys__GetSetDreamTimeLimit(DreamSys *this, s32 value);

/* @brief (Re)initializes playthrough-relevant data, like day number, flashabcks, etc. */
void DreamSys__InitNewGame(DreamSys *this);

/* @brief Sets whether the camera should shake when the player walks. */
/* @param value If True, the screen will shake when the player walks. */
/* @return To &value, the previous value of ScreenShakeOn */
void DreamSys__GetSetScreenShake(DreamSys *this, bool *value);

/* @brief Moves the currentDay counter foward one day, looping over on new years. */
/* @return Integer between 0 and 364, of the new currentDay value. */
s32 DreamSys__AdvanceDay(DreamSys *this);

/* @brief Checks what kind of dream comes next, and executes the appropiate start-of-dream actions. */
/* @return ID of the Stage to spawn on. Or -1 if the dream is Special (i.e. non-interactive). */
s32 DreamSys__StartDay(DreamSys *this);

/* @brief Executes various end-of-dream actions. */
s32 DreamSys__EndDay(DreamSys *this, s32 arg1);

/* @brief Gets the indicies of the Cinematic to be played next, if any. */
/* @return CinematicCall with the currently stored indicies. An Entry value of -1 means no cinematic. */
CinematicCall DreamSys__GetCinematic(DreamSys *this);

/* @brief Sets the next spawnpoint to be the intial spawn appropiate for the last day's graph. */
void DreamSys__InitSpawnLoc(DreamSys *this);

/* @brief Handles either dynamic or instance links, based on the value of DreamSys.currentStage */
void DreamSys__DynamicLink(DreamSys *this);

/* @brief Test whether a given position in the current stage is a static wall link */
/* @param currentPos The player's current position on the stage */
/* @return True if a valid link was found, False otherwise */
bool DreamSys__StaticWallLink(DreamSys *this, PlayerSpawnPoint *currentPos);

/* @brief Loads the next flashback on a flashback session */
/* @return False if it is the end of the flashback session, True otherwise */
bool DreamSys__LoadNextFlashback(DreamSys *this, bool unknown);

/* Called during some links, but no idea what it actually does */
bool ExecuteLink(DreamSys *system, s32 stage, s32 unk1, s32 unk2);

/* DreamSys__ProcessChunkChange(DreamSys *this,); */

/* @brief Processes the instance actions that directly affect this class, like instance linking and flashback logging. */
/* @param entity Pointer to the instance? */
/* @param effect Index of the effect to handle. */
void DreamSys__InstanceEffectsOnJournal(DreamSys *this, void *entity, s32 effect);

void DreamSys__GetPreviousDayMood(DreamSys *this, MoodGraphPoint *target, bool unknown);

/* @brief (Re)initializes both Mood Contributors in preparation for the start of the day. */
/* @param special If not NULL, both graphs will be initialized with this point logged in. */
void DreamSys__InitMoodContibutors(DreamSys *this, MoodGraphPoint *special);

/* @brief Logs the mood effect of the chunk at the given position. */
/* @param currentPos The player's current position on the stage */
void DreamSys__LogChunkMood(DreamSys *this, PlayerSpawnPoint *currentPos);

/* @brief Logs the given mood point as an instance mood. */
/* @param source Pointer to the mood to get logged. */
void DreamSys__LogInstanceMood(DreamSys *this,MoodGraphPoint *source);

/* @brief Calculates the current Overall Mood of the dream based on data from the contributors. */
/* @param ret Pointer where the final graph point will be written to. */
void DreamSys__UpdateDreamChart(DreamSys *this, MoodGraphPoint *ret);

/* @brief Gets the color value associated with the current dream mood */
/* @return Enum value of the current mood's color */
DreamColors DreamSys__GetDreamColor(DreamSys *this);

/* @brief Calculates the DreamColor for a given mood */
/* @param mood The graph point to get a color from */
/* @return Enum value of the calculated color */
DreamColors CalcDreamColor(MoodGraphPoint *mood);

/* @brief Resets all the values stored in a contributor back to zero. */
/* @param contributor MoodGraphContributor to be cleared. */
void DreamSys__ClearMoodGraph(DreamSys *this, MoodGraphContributor *contributor);

/* @brief "Logs" a given Mood Effect on the given Contributor. */
/* @param layer The contributor that will recieve the mood. */
/* @param mood The mood contribution to be logged. */
void DreamSys__LogMood(DreamSys* this, MoodGraphContributor* layer, MoodGraphPoint* mood);

/* @brief Calculates the average point of a given Contributor. */
/* @param layer The MoodGraphContributor to be calculated. */
/* @param ret Pointer where this contributor's average point will be written to. */
/* @return To &ret, MoodPoint between (-9,-9) and (9,9). */
void DreamSys__GetMoodAverage(DreamSys* this, MoodGraphContributor* layer, MoodGraphPoint* ret);

/* @brief Turns the values of a given mood contributor axis into an useable average */
/* @param lank The mood contribution that happened last, which recieves a boost in the code */
/* @param sum The cumulative value from all mood contributions */
/* @param amount The amount of mood contributions adquired */
/* @return Normalized integer between -9 and 9 */
s32 CalcMoodAxis(s32 lank, s32 sum, s32 amount);

/* @brief Recalculates the total progress towards unlocking the flashback feature */
void DreamSys__CalcUnlockScore(DreamSys *this);

/* @brief Saves a "Flashback Spawnpoint" into the player's flashback session. */
/* @param stage Stage index of the flashback */
/* @param pos Coordinates of the player in the stage */
/* @param angles Array of angles, used to make the player face the correct way */
/* @param unknown */
/* @param time Time limit of the flashback */
/* @param day Day number of the flashback */
void DreamSys__AddFlashback(DreamSys *this, s32 stage, PlayerSpawnPoint* pos, s32 *angles, s32 unknown, s32 time, s32 day);

/* @brief Called by the Grey Man to "erase" your flashback log */
void DreamSys__ResetFlashbackList(DreamSys *this);

/* @brief Gets the jumptable of "Virtual methods" assigned to the DreamSys class. */
/* @return Pointer to vtable_DreamSys */
struct vtable_DreamSys *Get_vtable_DreamSys(void);

/* @brief Allocates and constructs a DreamSys instance.
 * Still INCLUDE_ASM in src/DreamSys.c; declared here so other units'
 * matched C (e.g. func_80025FDC in src/code_1677c.c) can call it -- see
 * "Calling into a function that is still INCLUDE_ASM in another unit is
 * fine" in docs/DECOMPILATION_LEARNINGS.md. */
DreamSys *New_DreamSys(void *arg0, s32 arg1, s32 arg2);



/* @brief Initializes the values that will be used by CalcNavigationScore. */
/* @param arrayMem Pointer to the array of challenges completed */
/* @param linkCounter Pointer to an integer counting up the dynamic/instance links */
void InitNavChallengesArray(s8 (*arrayMem)[30], s32 *linkCounter);

/* @brief Calculates a score based on amount of Navigation Challenges achieved */
/* @return Integer between 0 and 50,000,000 */
s32 CalcNavigationScore(void);

/* @brief Gets the stage, spawn point, and time limit of a given point in the graph. */
/* @param target Pointer where the spawnpoint found will be written */
/* @param timeLimit Pointer where the time limit will be written to */
/* @param mood The mood that will be used for the calculation */
/* @param day Unused? */
/* @return The stage index of the initial spawn. */
s32 GenerateInitialSpawn(PlayerSpawnPoint *target, s32 *timeLimit, MoodGraphPoint *mood, s32 day);

/* @brief Obtains a random Spawnpoint on, or away from, a given stage. */
/* @param target Pointer where the new spawn will be written to */
/* @param stg The current stage (or target stage, if negative) */
/* @return Stage the spawn belongs to */
s32 GetRandomSpawnFromStage(PlayerSpawnPoint *target, s32 stg, s32 unused);
/* This function has two modes of operation, depending on the signage of stg.
   If stg is positive or zero, it behaves as a fully dynamic link *away* from a given stage.
   If stg is negative, it behaves as a "semi-static" link *on* a given stage. (This is the kind of link normally used by instances)
   Regardless of mode, this spawn will count towards the "Dynamic link penalty" of the flashback unlock score.*/

/* @brief Checks whether a given day is Special, and loads a random cinematic if it is. */
/* @param cinematic The CinematicCall that will be written to if a match is found. */
/* @param day The day number to check against (1-indexed). */
/* @return The pointer to this dream's graph contribution, or NULL if the dream is *not* Special. */
MoodGraphPoint *IsDaySpecial(CinematicCall *cinematic, int day);

#endif