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

/* pitch/heading/roll grouped into one 12-byte nested struct (round
   2026-09-02, DreamSys__AddFlashback): that function block-copies all
   three from its `angles` argument in ONE retail load-all-then-store-all
   sequence (six unaligned lwl/lwr loads, all before any of the six
   unaligned swl/swr stores) -- the same "whole-struct assignment, not a
   per-word copy" idiom already documented for other block moves in this
   project. A per-field or looped copy would not reproduce that
   instruction ordering. */
typedef struct FlashbackRotation {
	struct Angle{
		s16 angle;
		s16 one;
	} pitch; /* Does not do what you think it does */
	struct Angle heading;
	struct Angle roll; /* Ditto */
} FlashbackRotation;

typedef struct {
	s32 stageID;
	PlayerSpawnPoint position;
	FlashbackRotation rotation;
	s16 timeLimit;
	/* Was `s32`; retyped (round 2026-09-02): DreamSys__AddFlashback writes
	   it with a bare `sh` (halfword store) from an `s32` argument, which
	   only makes sense if the field itself is 2 bytes -- an `s32` field
	   fed by an `s32` argument would store all 4 bytes (`sw`), not 2. The
	   remaining 2 bytes before `day` are ordinary C struct alignment
	   padding, not a separate field. */
	s16 unknown_value_0x1c;
	s32 day;
} FlashbackEntry;

/* Opaque 0x28-byte block, block-copied wholesale by func_8005B904 (read) /
   func_8005B990 (write) -- see DreamSysUnk14::unk_0x44 below. Declared as a
   word array (not a byte array) so a whole-struct assignment reproduces
   retail's 4-word-per-iteration block-move codegen, per the confirmed idiom
   in DECOMPILATION_LEARNINGS.md ("A whole-struct assignment... for a block
   copy"); internal field layout is unconfirmed (round 2026-09-02). */
typedef struct DreamSysUnk14Tail {
	s32 raw[0x28 / 4];
} DreamSysUnk14Tail;

/* Struct pointed to by DreamSys::unk_0x14. Confirmed fields, all from
   func_8005B904/func_8005B990 (round 2026-09-02): +0x0 is a word cleared to
   0 by func_8005B990 (restore) after the rest of the struct is overwritten;
   +0x38 is the pre-existing 3-word vector (func_8001E600/func_8005942C,
   still INCLUDE_ASM); +0x44 is a pointer to a DreamSysUnk14Tail, itself
   block-copied (not just followed) by the same two functions. The whole
   0x50-byte struct (this field included, raw) is block-copied to/from
   DreamSys::unk14Snapshot -- see that field's comment. Bytes 0x4..0x38 and
   0x48..0x50 are unconfirmed padding. */
typedef struct DreamSysUnk14 {
	s32 unk_0x0;
	s32 unknown_values_0x4[0x34 / 4];
	s32 vec[3];
	DreamSysUnk14Tail *unk_0x44;
	s32 unknown_values_0x48[0x8 / 4];
} DreamSysUnk14;

/* Struct pointed to by DreamSys::unk_0x5C. +0x14 / +0x20 are a pair of
   two-word (x,y) points per func_8005942C (still INCLUDE_ASM elsewhere in
   this unit; not confirmed by this round). +0x24 (the second point's y) is
   confirmed: func_80059814 (round 2026-08-30) nudges it. +0x18 (the first
   point's y) is also now confirmed: func_80059BE0 (round 2026-09-02) nudges
   it by the SAME delta as +0x24, in the same statement pair. */
typedef struct DreamSysUnk5C {
	s8 unknown_values_0x0[0x18];
	s32 unk_0x18;
	s8 unknown_values_0x1C[8];
	s32 unk_0x24;
} DreamSysUnk5C;

/* Object pointed to by DreamSys::unk_0x58, used by func_80059E3C (slot
   +0x84) and, this round, ExecuteLink (slot +0x80): loaded, dereferenced
   for its own vtable pointer at offset 0, and called through. Everything
   else about this class -- including whether it is the SAME class as
   DreamSys::unk_0x4C below -- is unknown. Elsewhere in this unit unk_0x58
   is set/read as a plain s32 (func_8005937C, func_8005A134's call into
   func_8002CC84), which is consistent with it being a pointer value just
   not typed that way there. slot0x84 takes TWO arguments, not one --
   head-adjudicated 2026-08-30-c: the guard value (DreamSys::unk_0xBC)
   loaded into $a1 by func_80059E3C is never overwritten before the jalr, so
   it is passed through, not just branched on. See func_80059E3C.md.
   slot0x80 (ExecuteLink, round 2026-09-02) takes three arguments, all
   literal constants at that call site (0x90, 0x6E, 0x6E) -- nothing here
   suggests what they mean. */
typedef struct DreamSysUnk58Vtable {
	u8 pad00[0x80];
	void (*slot0x80)(void *self, s32 a1, s32 a2, s32 a3);
	void (*slot0x84)(void *self, s32 flag);
} DreamSysUnk58Vtable;
typedef struct DreamSysUnk58 {
	DreamSysUnk58Vtable *vt;
} DreamSysUnk58;

/* Object pointed to by DreamSys__DreamSys's `arg1` constructor parameter --
   same "vtable pointer at offset 0" shape as the other opaque classes in
   this unit. Stored verbatim into DreamSys::unk_0x60 and also used
   immediately: `arg1->methods->slot0x80(arg1, 0)`'s return value (a
   companion pointer, forwarded as `void *`) is passed to DreamSys's own
   `vt->slot10` right after -- the same "buddy-link" shape code_55dd4.h
   documents for Class65650's `slot10`/`slot14` pair (round 2026-09-02).
   Nothing else identifies this class. */
typedef struct DreamSysCtorArgMethods {
	u8 pad00[0x80];
	void *(*slot0x80)(void *self, s32 arg1);
} DreamSysCtorArgMethods;
typedef struct DreamSysCtorArgObj {
	DreamSysCtorArgMethods *methods;
} DreamSysCtorArgObj;

/* Object pointed to by DreamSys__func_58968's `arg1` parameter -- same
   "vtable pointer at offset 0" shape as this unit's other opaque classes.
   Unidentified; may or may not be the same class as DreamSysCtorArgObj
   above (both are eventually forwarded to vt->slot10, but this one is
   passed through directly rather than via a derived value) -- kept as a
   separate local view per this unit's "multiple independent views of one
   table" convention until proven otherwise (round 2026-09-02). */
typedef struct DreamSysFunc58968ArgMethods {
	u8 pad00[0xE4];
	/* Called by DreamSys__func_58968 as (arg1, &local, this,
	   &this->linkCoordinates); return value discarded. */
	void (*slot0xE4)(void *self, void *arg1, struct DreamSys *arg2, PlayerSpawnPoint *arg3);
} DreamSysFunc58968ArgMethods;
typedef struct DreamSysFunc58968ArgObj {
	DreamSysFunc58968ArgMethods *methods;
} DreamSysFunc58968ArgObj;

/* Inner struct chased by func_80058B08's `arg1 == -2` path: the return of
   DreamSysUnk4CMethods::slot0x11C (a DreamSysUnk11CResult below) has a
   pointer at +0x4 to one of THESE, and only +0x2C (a `s16`, compared
   against the literal 2) is read (round 2026-09-02). */
typedef struct DreamSysUnk11CInner {
	s8 unknown_values_0x0[0x2C];
	s16 unk_0x2C;
} DreamSysUnk11CInner;

/* Return type of DreamSysUnk4CMethods::slot0x11C. Only +0x4 (a pointer to
   DreamSysUnk11CInner above) is read, by func_80058B08 (round 2026-09-02). */
typedef struct DreamSysUnk11CResult {
	s8 unknown_values_0x0[4];
	DreamSysUnk11CInner *unk_0x4;
} DreamSysUnk11CResult;

/* Object pointed to by DreamSys::unk_0x4C, used by func_80058A94 (slot
   +0xF0) and func_8005A82C (slot +0xE8, this round): same "vtable pointer
   at offset 0" shape as DreamSysUnk58 above. Unidentified class; unknown if
   related to DreamSysUnk58. */
typedef struct DreamSysUnk4CMethods {
	u8 pad00[0xD4];
	/* Called by DreamSys__WallLink as (this->unk_0x4C, arg1), return value
	   whole-struct-assigned into this->linkCoordinates (a PlayerSpawnPoint)
	   (round 2026-09-02). */
	PlayerSpawnPoint *(*slot0xD4)(void *self, void *arg1);
	u8 pad_0xD8[0xE8 - 0xD8];
	/* Called by func_8005A82C as (this->unk_0x4C, &local, &this->
	   linkCoordinates) -- `local` is an output buffer also consumed by
	   vt->slot0xB8 right after (round 2026-09-02). */
	void (*slot0xE8)(void *self, void *arg1, PlayerSpawnPoint *arg2);
	u8 pad_0xEC[0xF0 - 0xEC];
	void (*slot0xF0)(void *self);
	u8 pad_0xF4[0x10C - 0xF4];
	/* Called by DreamSys__FlashbackSaving as (this->unk_0x4C, 0, 0), return
	   value forwarded straight into DreamSys::AddFlashback's `pos` argument.
	   Same slot OFFSET and signature as DreamSysEntityMethods::slot0x10C
	   below (used for the unrelated `entity` parameter in
	   DreamSys__ProcessChunkChange) -- plausibly the same underlying class,
	   but kept as a separate local view per this unit's convention for
	   "multiple local views of the same table" (round 2026-09-02). */
	/* Called with (this->unk_0x4C, 0, 0) by DreamSys__FlashbackSaving (see
	   above); ALSO called with the identical (0, 0) argument pair by
	   func_80058B08's shared tail block, whose return value is forwarded
	   straight into vtable slot +0x1D4 (func_8005A7A0)'s `currentPos`
	   argument (round 2026-09-02) -- same signature, different caller. */
	PlayerSpawnPoint *(*slot0x10C)(void *self, s32 arg1, s32 arg2);
	u8 pad_0x110[0x11C - 0x110];
	/* Called by func_80058B08's `arg1 == -2` path as (this->unk_0x4C,
	   (u8 *)this->unk_0x14 + 0x18); the result's `unk_0x4` is chased and its
	   `unk_0x2C` compared against the literal 2 (round 2026-09-02). */
	DreamSysUnk11CResult *(*slot0x11C)(void *self, void *arg1);
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
	u8 pad00[0x8];
	/* Shared with Class65650's own inherited "ctor" slot at the same offset
	   in the SAME base table (code_55dd4.h's D800878D4Methods, which already
	   names and resolves this exact slot as `func_80057044`, taking/
	   returning `Class65650 *self`). Called by DreamSys__DreamSys as
	   (this), its return value discarded (round 2026-09-02) -- consistent
	   with the base ctor returning `self` for chaining, unneeded here since
	   the caller already has `this`. */
	struct DreamSys *(*ctor)(struct DreamSys *self);
	u8 pad0C[0x4C - 0xC];
	/* Shared with Class65650's own inherited "slot4C" at the same offset in
	   the SAME base table (code_55dd4.h's D800878D4Methods: "called by
	   func_80065918 as slot4C(self, arg3, arg5)"). Called by
	   DreamSys__func_58968 as (this, arg1, &local) (round 2026-09-02). */
	void (*slot4C)(struct DreamSys *self, void *arg1, void *arg2);
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
	u8 pad54[0x88 - 0x54];
	/* Called by func_80058B08 (this unit's own +0x088 slot) as
	   (this, arg1), return value discarded (round 2026-09-02). */
	void (*slot0x88)(struct DreamSys *self, s32 arg1);
	u8 pad8C[0x9C - 0x8C];
	/* Called by func_80058E8C as (this, arg1, arg2) -- round 2026-08-30-d. */
	void (*slot0x9C)(struct DreamSys *self, void *arg1, s32 arg2);
	u8 padA0[0xDC - 0xA0];
	/* Called unconditionally by func_80058F18 (this unit's own +0xDC slot)
	   as (this, arg1, arg2) -- same argument shape as slot0x9C above
	   (round 2026-09-02). Resolves to func_80057B90 in D_800878D4, out of
	   this unit's scope. */
	void (*slot0xDC)(struct DreamSys *self, void *arg1, s32 arg2);
	/* Called by DreamSys__WallLink as (this, arg1, arg2) -- same argument
	   shape as slot0x9C/slot0xDC above (round 2026-09-02). */
	void (*slot0xE0)(struct DreamSys *self, void *arg1, s32 arg2);
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

/* A `struct RelativePos` constant, passed as func_8005AF64's `a` argument
   by func_8005AD68 (round 2026-09-02). */
extern struct RelativePos D_8008ABD0;

/* Another `struct RelativePos` constant, passed as func_8005AF64's `a`
   argument by func_8005AB2C -- same call shape as D_8008ABD0 above, just a
   different constant (round 2026-09-02). */
extern struct RelativePos D_8008ABC0;

/* Another `struct RelativePos` constant, passed as func_8005AF64's `a`
   argument by func_8005AC24 -- same call shape as D_8008ABC0/D_8008ABD0
   above, just a different constant (round 2026-09-02). */
extern struct RelativePos D_8008ABC8;

/* Another `struct RelativePos` constant, passed as func_8005AF64's `a`
   argument by func_8005AE40 -- same call shape as D_8008ABD0/D_8008ABC0
   above, just a different constant (round 2026-09-02). */
extern struct RelativePos D_8008ABD8;

/* Address-of only (never dereferenced by this unit's queued functions),
   forwarded as vtable slot +0x044's (func_8001CEB4) arg2 by func_8005AD68
   (round 2026-09-02) -- same "opaque generic pointer" shape as that slot's
   other known call site (D_80087E84[-1]). */
extern u8 D_80087EFC[];

/* Same "opaque generic pointer" shape as D_80087EFC above, forwarded as
   vtable slot +0x044's (func_8001CEB4) arg2 by func_8005AE40
   (round 2026-09-02) -- a different constant/address, same call shape. */
extern u8 D_80087F08[];

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

/* Struct pointed to by DreamSys::unk_0x28. Only +0x36 is touched (a `u16`,
   masked to 0x7F by func_80058B08, round 2026-09-02); everything else is
   unconfirmed. */
typedef struct DreamSysUnk28Target {
	s8 unknown_values_0x0[0x36];
	u16 unk_0x36;
} DreamSysUnk28Target;

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

	/* Pointer to DreamSysUnk14 (see that type for confirmed fields); a
	   3-word vector lives at +0x38 of what this points to (read by
	   func_8001E600 / func_8005942C, guarded by unk_0xC above). The whole
	   struct is saved/restored to/from unk14Snapshot below by
	   func_8005B904/func_8005B990 (round 2026-09-02). */
	DreamSysUnk14 *unk_0x14;
	s8 unknown_values_0x18[12];

	s32 dreamTimer;
	/* Read as a pointer by func_80058B08 (round 2026-09-02): loaded, then
	   dereferenced at +0x36 for a `u16` (masked to 0x7F and stashed into
	   unk_0xB8 below). Target type otherwise unconfirmed. */
	DreamSysUnk28Target *unk_0x28;
	s8 unknown_values_0x2C[24];

	s32 unknwon_int_0x44;
	/* Written by func_80057534 (this unit's own helper, invoked via its own
	   +0x0C8/+0x0CC slots func_800574C4/func_800574FC) and by func_80057C6C
	   (this unit's own +0x0E4 slot), both as a plain `sh` store of a `s16`
	   value (round 2026-09-04). */
	s16 field_0x48;
	s8 unknown_values_0x4A[2];
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
	/* Set unconditionally to the constructor's `arg1` by DreamSys__DreamSys
	   (round 2026-09-02) -- see DreamSysCtorArgObj. No other observed use in
	   this unit's queued functions. */
	void *unk_0x60;
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
	/* Attempt/beat counter incremented (and bounded to [0,4)) by
	   func_80059BE0 on every call while unk_0xA0 is nonzero; reset to 0 once
	   unk_0xA0 goes back to 0. Compared against 3 there to pick a +-50
	   nudge applied to unk_0x5C's y fields, and against 4 (together with
	   unk_0xAC) to force unk_0xA0 back to 0 (round 2026-09-02). */
	s32 unk_0xB4;
	/* Derived from `unknown_values_0x28[0x36]` masked to 0x7F, or forced to
	   0 (if >= 0x18) or 2 (if `unknwon_int_0x44 == 15` and this is still 0)
	   by func_80058B08's `arg1 == -1` path (round 2026-09-02). */
	s32 unk_0xB8;
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
	/* Set (whole word) by func_8005A7A0 to func_8005BF48()'s return value,
	   right before an ExecuteLink (round 2026-09-02). */
	s32 unk_0x880;
	/* Gate flag read by func_80059148 (round 2026-08-30-b): when nonzero
	   (reusing the SAME loaded value, not a fresh 0/1 test), forwarded as
	   func_8001CEB4's arg2 -- cast from s32 to void*, not dereferenced. */
	s32 unk_0x884;
	/* Zeroed (whole word) by func_8005A7A0 alongside unk_0x884
	   (round 2026-09-02). */
	s32 unk_0x888;

	s32 storedDay;

	/* The struct previously ended here (0x890), but New_DreamSys allocates
	   sizeof(DreamSys) via a literal `ori $a0, $zero, 0x928` -- 0x98 bytes
	   more than any field so far discovered accounts for. Extended to the
	   allocator's real size (round 2026-08-30-b); the four words
	   DreamSys__func_588ec clears are named, the rest of the tail is still
	   unclaimed.

	   The first 0x78 bytes of that tail are a save/restore scratch buffer
	   for *unk_0x14: func_8005B904 copies *unk_0x14 (0x50 bytes) then
	   *unk_0x14->unk_0x44 (0x28 bytes, DreamSysUnk14Tail) into these two
	   fields; func_8005B990 copies them back and then clears
	   unk_0x14->unk_0x0 to 0 (round 2026-09-02). */
	DreamSysUnk14 unk14Snapshot;
	DreamSysUnk14Tail unk14TailSnapshot;
	s32 unk_0x908;
	s32 unk_0x90C;
	s32 unk_0x910;
	/* A retry/attempt counter (round 2026-09-02, func_8005AD68): read as a
	   whole word, compared against several literal bands, and incremented
	   by 1 at that function's normal exit. */
	s32 unk_0x914;
	s8 unknown_values_0x918[4];
	/* A `struct RelativePos`, address-taken and passed to func_8005AF64 as
	   its `b` argument (round 2026-09-02, func_8005AD68) -- carved out of
	   what was raw padding in the same 0x10-byte block as unk_0x914 above. */
	struct RelativePos unk_0x91C;
	s8 unknown_values_0x922[2];
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
	u32 unknown_functions_0xc[1];
	/* Shared with Class65650's own vtable at the same offset (code_55dd4.h:
	   `slot10`, resolved there as `func_800570B4`, the "link" companion of
	   `slot14`/`func_80057130` immediately below -- this unit already names
	   THAT slot `func_80057130` and notes the same companion relationship).
	   Called by DreamSys__DreamSys as (this, arg1->methods->slot0x80(arg1,
	   0)) -- the constructor's own "buddy-link" step (round 2026-09-02). */
	void (*slot10)(DreamSys *this, void *arg);
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x014); shared
	   with Class65650's inherited slot14 (code_55dd4.h: "'unlink' companion
	   of slot10"). Called by func_80058A94 as (this, this->unk_0x4C)
	   (round 2026-08-30-b). Still INCLUDE_ASM; address 0x80057130 is
	   outside this unit/runner's range. */
	void (*func_80057130)(DreamSys *this, DreamSysUnk4CObj *arg1);
	u32 unknown_functions_0x18[6];
	/* +0x030, BasicClass__func_182cc -- shared base-class slot, same one
	   `class_3ac78.h`/`Class6D3C8.h` name (see their comments); called by
	   ExecuteLink as (this, unk1) with its return discarded
	   (round 2026-09-02). */
	void (*slot30)(DreamSys *this, s32 arg1);
	u32 unknown_functions_0x34[3];
	/* Called by DreamSys__DreamSys as the constructor's LAST step, as
	   (this); its return value is never overwritten before the function's
	   own epilogue, so it becomes DreamSys__DreamSys's own return value
	   unchanged (round 2026-09-02) -- typed `DreamSys *` to match. */
	DreamSys *(*func_800588EC)(DreamSys *this);
	/* Called by func_800598E8 as (this, 0, &D_80087E84[-1]); return value,
	   if any, unused (round 2026-08-30). */
	void (*func_8001CEB4)(DreamSys *this, s32 arg1, void *arg2);
	u32 unknown_functions_0x48[1];
	/* This function's OWN slot; called this round (round 2026-09-02). */
	void (*func_58968)(DreamSys *this, DreamSysFunc58968ArgObj *arg1);
	u32 unknown_functions_0x50[4];
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x060). Called by
	   DreamSys__func_588ec as (this, 0) (round 2026-08-30-b). Still
	   INCLUDE_ASM; address 0x8001D344 is outside this unit/runner's
	   range. */
	void (*func_8001D344)(DreamSys *this, s32 arg1);
	u32 unknown_functions_0x64[9];
	/* This function's OWN slot (+0x088, resolved via
	   tools/classtable.py DREAMSYS_METHODS). Dispatches on `arg1`
	   (-2 / -1 / anything else) after an unconditional call through the
	   shared base table's own +0x088 slot (round 2026-09-02). */
	void (*func_80058B08)(DreamSys *this, s32 arg1);
	u32 unknown_functions_0x8c[3];
	void *TimerTick;
	/* This function's OWN slot; resolved via tools/classtable.py
	   (round 2026-08-30-d). */
	void (*func_80058E8C)(DreamSys *this, void *arg1, s32 arg2);
	u32 unknown_functions_0xa0[6];
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x0B8). Called by
	   func_8005A82C right after unk_0x4C->methods->slot0xE8, as (this,
	   &local) using that same output buffer (round 2026-09-02). Address
	   0x80057384 is outside this unit/runner's range; still INCLUDE_ASM. */
	void (*func_80057384)(DreamSys *this, void *arg1);
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x0BC). Called by
	   func_8005AF64 and func_8005A0B0 with a DreamSysVec3* second argument
	   (round 2026-08-30-d). */
	void (*func_800573A8)(DreamSys *this, DreamSysVec3 *arg1);
	/* Called by func_80057534 (this unit's own helper, invoked by its own
	   +0x0C8/+0x0CC slots) as (this, &D_8008ABA4) -- resolves to
	   func_80057444, out of this unit/runner's range (round 2026-09-04). */
	void (*func_80057444)(DreamSys *this, void *arg1);
	/* Read (not called) by this unit's own +0x0D0 slot (func_800575B0) and
	   forwarded as a raw callback value to func_80057618 -- resolves to
	   func_8005748C, out of this unit/runner's range (round 2026-09-04). */
	void (*func_8005748C)(DreamSys *this, s32 val, void *extra);
	/* This function's OWN slot; forwards (val, extra) to
	   func_80057534(this, &D_8008ABA4[0], val, extra, 7)
	   (round 2026-09-04). */
	void (*func_800574C4)(DreamSys *this, s32 val, void *extra);
	/* This function's OWN slot; forwards (val, extra) to
	   func_80057534(this, &D_8008ABA4[1], val, extra, 8)
	   (round 2026-09-04). */
	void (*func_800574FC)(DreamSys *this, s32 val, void *extra);
	/* This function's OWN slot; reads its NEIGHBOUR slot +0x0C4
	   (func_8005748C) as a raw callback value and forwards it, with its
	   own two arguments, to func_80057618 (round 2026-09-04). */
	void (*func_800575B0)(DreamSys *this, void *arg1, void *arg2);
	/* This function's OWN slot; reads its NEIGHBOUR slot +0x0C8
	   (func_800574C4, self-referential) as a raw callback value and
	   forwards it, with its own two arguments, to func_80057618
	   (round 2026-09-04). */
	void (*func_800575E0)(DreamSys *this, void *arg1, void *arg2);
	/* This function's OWN slot; empty stub `{ }` (round 2026-09-04). */
	void (*func_80057610)(DreamSys *this);
	u32 unknown_functions_0xdc[1];
	/* This function's OWN slot; resolved via tools/classtable.py
	   (round 2026-09-02). */
	void (*LinkWall)(DreamSys *this, void *arg1, s32 arg2);
	/* This function's OWN slot; a single `sh a1, 0x48(a0)` store
	   (`this->field_0x48 = val`) (round 2026-09-04). */
	void (*func_80057C6C)(DreamSys *this, s16 val);
	/* This unit's own no-op stub (`func_800590E0`, `{ }`). Called by
	   DreamSys__WallLink as (this) -- the callee ignores its argument
	   (round 2026-09-02). */
	void (*func_800590E0)(DreamSys *this);
	/* This function's OWN slot; a single `sw a1, 0x54(a0)` store
	   (`this->unk_0x54 = extra`) (round 2026-09-04). */
	void (*func_80057C7C)(DreamSys *this, void *extra);
	u32 unknown_functions_0xf0[2];
	/* This function's OWN slot (+0x0F8, resolved via
	   tools/classtable.py DREAMSYS_METHODS). A straight-line initializer:
	   calls LogChunkMood/func_8005966C/func_800596E8/func_8005A168/
	   func_8005A1B0/func_8005A1EC in sequence, then zeroes a large block of
	   per-dream state, ending with a func_8001E6F8/func_8001CEB4 pair over a
	   small local buffer (round 2026-09-02). */
	void (*func_800591B4)(DreamSys *this, s32 arg1, s32 arg2);
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
	/* Referenced by func_80059AEC/func_80059B50; return value is threaded
	   into func_80059E98. Still INCLUDE_ASM -- STALLED at 45/79 words round
	   2026-09-02 on a register-identity/delay-slot-filler residue, not a
	   value/structure error: bumps unk_0xB4 while unk_0xA0 is nonzero,
	   conditionally calls func_80059D1C (slot +0x168) or func_80059E3C
	   (slot +0x16C), nudges unk_0x5C's y fields, and always returns the
	   ORIGINAL unk_0xA0 value read on entry (0 if it was already 0). See
	   docs/match-reports/func_80059BE0.md for the best-reached body and
	   what was tried. */
	s32 (*func_80059BE0)(DreamSys *this, s32 arg1);
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x168). Called by
	   func_80059BE0 as (this), return value discarded -- still INCLUDE_ASM,
	   blocked (addiu_at indexed-global load; see
	   docs/match-reports/func_80059D1C.md). This prototype only types that
	   call site (round 2026-09-02). */
	void (*func_80059D1C)(DreamSys *this);
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x16C). Called by
	   func_80059BE0 as (this) on the path where the +0x168 slot was NOT
	   taken (round 2026-09-02). */
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
	u32 unknown_functions_0x1d0[1];
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x1D4), this
	   round. Tests this->unknwon_int_0x44, then a static-link-with-timer
	   test (func_8005BE90) against this->linkCoordinates/currentStage/
	   dreamTimer, then ExecuteLinks with literal type 0x10 on success --
	   see func_8005A7A0.md. */
	bool (*func_8005A7A0)(DreamSys *this, PlayerSpawnPoint *currentPos);
	u32 unknown_functions_0x1d8[2];
	/* Getter for currentStage (round 2026-08-30-c). */
	s32 (*func_8005AFD0)(DreamSys *this);
	void (*ProcessChunkChange)(DreamSys *this, void *entity, s32 effect);
	/* Renamed from the previous placeholder `InstanceEffectsOnPlayer` --
	   this slot's real symbol (config/symbols.slps01556.lsdde.txt) is
	   `DreamSys__InstanceEffectsOnJournal` (see the forward declaration
	   below and src/DreamSys.c), confirmed via tools/classtable.py
	   DREAMSYS_METHODS (+0x1E8) while resolving func_80058F18's call
	   through this slot (round 2026-09-02). No call site referenced the
	   old name, so this is a plain correction, not a rename requiring an
	   out-of-scope edit elsewhere. */
	void (*InstanceEffectsOnJournal)(DreamSys *this, void *entity, s32 effect);
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
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x214): called by
	   DreamSys__FlashbackSaving (round 2026-09-02) as (this,
	   this->currentStage, pos, &local, arg1, arg2, this->currentDay) --
	   exactly DreamSys__AddFlashback's own parameter shape, 3 in registers
	   and 4 more forwarded on the stack past a3 (o32 ABI). Retyped from the
	   previous untyped `void *` placeholder; no other caller referenced the
	   old field name (grep across src/include turned up none), so this is a
	   plain correction. */
	void (*AddFlashback)(DreamSys *this, s32 stage, PlayerSpawnPoint *pos, s32 *angles, s32 unknown, s32 time, s32 day);
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x218): this is
	   DreamSys__FlashbackSaving's OWN slot (round 2026-09-02). Not called
	   through the vtable by any function in this unit; retyped for
	   documentation only, matching the function's real signature. */
	void (*FlashbackSaving)(DreamSys *this, s32 arg1, s32 arg2);
	void (*ResetFlashbackList)(DreamSys *this);
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x220/+0x224).
	   Typed and matched this round (2026-09-02): a save/restore pair for
	   *this->unk_0x14 (see DreamSysUnk14). func_8005B904 saves *unk_0x14
	   and *unk_0x14->unk_0x44 into this->unk14Snapshot/unk14TailSnapshot;
	   func_8005B990 restores them and clears unk_0x14->unk_0x0 to 0. Both
	   take only `this` -- the previous note's "driven by a length read
	   from this->unk_0x14" undersold it: unk_0x14 is a POINTER, dereferenced
	   for its own bytes, not read as a length. */
	void (*func_8005B904)(DreamSys *this);
	void (*func_8005B990)(DreamSys *this);
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

/* The fixed "special day" mood, returned by IsDaySpecial on a match
   (round 2026-09-02); only ever address-taken there, never dereferenced by
   this unit's queued functions. */
extern MoodGraphPoint D_8008ABF4;

/* Also declared in Entity.h for the same libc-style function. */
extern s32 rand(void);

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

/* Called by func_8005A7A0 as (&this->linkCoordinates, this->currentStage,
   currentPos, this->dreamTimer); result compared with `bltz` exactly like
   TestForStaticLink's call site, so s32 (round 2026-09-02). Blocked by the
   gp-relative addressing blocker -- see docs/match-reports/func_8005BE90.md
   -- so it stays INCLUDE_ASM; this prototype only types the call site. */
extern s32 func_8005BE90(PlayerSpawnPoint *target, s32 stage, PlayerSpawnPoint *currentPos, s32 timer);

/* Called by func_8005A7A0 with NO arguments (the disassembly's call site has
   an empty delay slot and no a0-a3 setup); its return value is stored whole
   into this->unk_0x880, hence s32 (round 2026-09-02). Blocked by the
   gp-relative addressing blocker -- see docs/match-reports/func_8005BF48.md
   -- so it stays INCLUDE_ASM; this prototype only types the call site. */
extern s32 func_8005BF48(void);

/* Called by func_8005A700 as (this, &local) where `local` is a 0x10-byte
   stack buffer also forwarded to func_8005BD3C below; return value is
   discarded at this call site (round 2026-09-02). NOT in this unit at all --
   its body disassembles into asm/code_d294.s, an uncarved segment -- so this
   prototype only types this one call site, per the "calling into a function
   that is still INCLUDE_ASM elsewhere is fine" convention
   (DECOMPILATION_LEARNINGS.md). A discarded return is not evidence of
   `void` (same doc); kept `void` here only because nothing at this call
   site constrains it further. */
extern void func_8001E6F8(DreamSys *this, void *arg1);

/* Called by func_8005A700 as (&this->unk_0x888, &this->unk_0x884, &local) --
   same `local` buffer func_8001E6F8 fills above; result used as a truth
   value (`beqz`), so s32 (round 2026-09-02). Blocked by both the
   gp-relative and addiu_at blockers -- see
   docs/match-reports/func_8005BD3C.md -- so it stays INCLUDE_ASM; this
   prototype only types the call site. */
extern s32 func_8005BD3C(s32 *arg0, s32 *arg1, void *arg2);

/* Same (target, currentPos, stage) forwarding shape as Test4TunnelLinks
   above (see that function's own comment) -- called by func_8005A82C as
   (&this->linkCoordinates, currentPos, this->currentStage), result compared
   with `bltz` (round 2026-09-02). Blocked by the gp-relative addressing
   blocker -- see docs/match-reports/Test4InstantTeleporters.md -- so it
   stays INCLUDE_ASM; this prototype only types the call site. */
extern s32 Test4InstantTeleporters(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage);

/* Called by func_8005A82C with NO arguments, same shape as func_8005BF48
   above; return value is forwarded straight into ExecuteLink's stage-type
   argument, hence s32 (round 2026-09-02). Blocked by the gp-relative
   addressing blocker -- see docs/match-reports/func_8005BFC4.md -- so it
   stays INCLUDE_ASM; this prototype only types the call site. */
extern s32 func_8005BFC4(void);

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