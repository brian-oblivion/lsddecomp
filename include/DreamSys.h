#ifndef CLASS_DREAMSYS
#define CLASS_DREAMSYS

/* DreamSys is the game's core per-playthrough subsystem: one instance
 * (constructed by New_DreamSys, called from src/code_1677c.c) owns the day
 * counter, the current dream's stage/spawn selection, the mood graph (two
 * MoodGraphContributor accumulators averaged into a DreamColors value),
 * flashback recording/playback, and the "link" state machine that decides
 * when the player transitions between stages -- static wall links, dynamic/
 * instance links, tunnels, staircases, instant teleporters, and a handful of
 * timed/special-case transitions, each tried in turn from a handler tried by
 * DreamSys__TimerTick's per-tick update. Every method takes an explicit
 * `this` through the hand-rolled class framework (CLAUDE.md; NOT C++): the
 * table is `DREAMSYS_METHODS` (struct vtable_DreamSys, resolved with
 * tools/classtable.py). Several of DreamSys's OWN methods are implemented in
 * sibling units that also include this header -- src/class_3bb8c_p.c (the
 * grid-based nearby-link search and the local-offset/"apply or find nearby"
 * family), class_3bb8c_t.c, class_3bb8c_r.c and class_3bb8c_o.c -- because
 * the class's code spans more than one disassembly segment; this header is
 * their shared, only correct view of `struct DreamSys` and its vtable.
 * Naming pass, round 65 (42 functions, 6 fields) and round 66 (29 more
 * functions, 31 fields, 33 globals, and the rotation types below). Round 65's
 * note here said the "0x6C/0x70.../0xBC" state machine had no name confident
 * enough to clear the bar; round 66 found one, and the thing that unlocked it
 * was NOT reading those functions harder. It was two cross-unit
 * identifications that turn opaque call sites into evidence:
 *   - Class6B5CC__UpdateRotation, the inherited slot +0x044, is MATCHED in src/code_d294.c
 *     and is the ROTATION SETTER. Every `void *` constant this unit hands it
 *     is therefore three {numerator, denominator} degree ratios, and every one
 *     of them decodes to a round angle. That converts "opaque generic
 *     pointer" into "+-45 degrees of yaw" at six call sites.
 *   - DreamSys::soundObj is a VabStreamObj* (src/code_179d8_e.c): its vtable
 *     +0x84 and +0x9C ARE VabStreamObj__StopVoice and
 *     VabStreamObj__SetPitchOffset, at the same offsets with the same
 *     signatures, and the value +0x80 returns is exactly what +0x84 is later
 *     handed. That turns the +0xB8/+0xBC pair into a voice selector and a
 *     voice index.
 * The general form: a placeholder-named callee in ANOTHER unit can be the
 * only evidence your own unit's names need, and `grep` for it costs nothing.
 * Per-name evidence and tiers are in docs/match-reports/<func>.md. */

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
extern s32 gLinkSrcStage;
extern s32 gLinkTriggerIndex;
extern s32 gLinkDstStage;
extern s32 gLinkSpawnIndex;

/* Delta/threshold table pairs consumed by DreamSys__StepLookOffset (LOOK_OFFSET_STEPS /
   LOOK_OFFSET_LIMITS, indexed by DreamSys::lookOffsetCommand) and DreamSys__StepLookYaw (LOOK_YAW_STEPS /
   LOOK_YAW_LIMITS, indexed by DreamSys::lookYawCommand). Index 0 is unused/zero in both
   pairs; indices 1 and 2 are the negative/positive delta and its matching
   threshold. Still raw `nonmatching` data (round 2026-08-30). */
extern s32 LOOK_OFFSET_STEPS[3];
extern s32 LOOK_OFFSET_LIMITS[3];
extern s32 LOOK_YAW_STEPS[3];
extern s32 LOOK_YAW_LIMITS[3];

/* Consumed by DreamSys__ApplyMoveCommand (round 2026-09-06), both indexed by that
   function's own `arg1` (a mood/day-type selector, range implied by the
   table sizes below): `MOVE_COMMAND_SIGNS[arg1] * MOVE_MODE_SPEEDS[this->moveMode]` forms
   a signed delta, then `MOVE_COMMAND_DISPATCH[arg1]` is called with it. Index 0 is
   unused/null in MOVE_COMMAND_DISPATCH (arg1 == 0 returns before reaching any of
   these, per that function's own guard) -- consistent with MOVE_COMMAND_SIGNS[0]
   being 0 too. MOVE_MODE_SPEEDS is indexed separately by DreamSys::moveMode (its
   own "Current" value, see that field), not by arg1. */
extern s32 MOVE_MODE_SPEEDS[5];
extern s8 MOVE_COMMAND_SIGNS[8];
/* Declared further down (after the real `DreamSys` typedef exists) as
   `extern void (*MOVE_COMMAND_DISPATCH[5])(DreamSys *this, s32 val, void *extra);` --
   same element type as DreamSys__DispatchOffsetSlotC4/DreamSys__DispatchOffsetSlot0 below, which this table
   holds pointers to. */

/* A single {numerator, denominator} degree ratio. This is not a guess about
   the LAYOUT any more (round 66): Class6B5CC__UpdateRotation -- vtable slot +0x044, the
   inherited rotation setter, MATCHED in src/code_d294.c -- reads exactly
   three of these from its `data` argument, one per axis, converts each with
   RatioToFixed12 and divides by 360, then either STORES them into the
   object's rotation vector (flag != 0) or ADDS them modulo a full turn
   (flag == 0). Every constant this unit hands that slot is three of these,
   and every one of them decodes to a plausible angle: see
   ROTATION_YAW_180 / _PLUS45 / _MINUS45 and CARDINAL_ROTATIONS below. */
typedef struct RotationRatio {
	s16 numerator;
	s16 denominator;
} RotationRatio;

/* The x/y/z triple Class6B5CC__UpdateRotation actually consumes. */
typedef struct RotationRatios {
	RotationRatio x;
	RotationRatio y;
	RotationRatio z;
} RotationRatios;

/* One 12-byte-stride RotationRatios array that splat had to split across two
   labels, because DreamSys__StepLookYaw references its SECOND word (entry
   0's yaw numerator, which it overwrites with its own per-tick delta) while
   DreamSys__ApplyPendingTurn address-takes whole entries. The round-2026-08-30-b
   note here read the 4-byte and 12-byte views as "likely two unrelated
   globals"; they are one table, and the data says so -- entry 0 is
   (0 deg, 45 deg, 0 deg) with the 45 being exactly the +-0x2D
   DreamSys__StepLookYaw writes, entry 1 is (0, -6, 0) and entry 2 is
   (0, +6, 0), which are DreamSys::turnCommand's two values 1 and 2. */
extern RotationRatio TURN_ROTATION_YAW[]; /* == &TURN_ROTATIONS[0].y */
extern RotationRatios TURN_ROTATIONS[];

/* (0 deg, 180 deg, 0 deg). Address-of only, forwarded as Class6B5CC__UpdateRotation's
   arg2 with flag 1 (absolute) by DreamSys__ResetSessionState. */
extern RotationRatios ROTATION_YAW_180;

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

/* The `chunk`+`tile` half of a PlayerSpawnPoint (4 bytes), reinterpreted as
   one struct so a single whole-struct assignment reproduces retail's
   unaligned 4-byte `lwl`/`lwr` + `swl`/`swr` copy -- DreamSys__TryStaircaseLink (round
   2026-09-06) copies a `PlayerSpawnPoint *currentPos` piecewise into
   DreamSys::staircaseGridPos (this type) and DreamSys::staircaseOrigin (a plain
   `struct RelativePos`, the `position` half) rather than as one 10-byte
   copy, matching retail's own two separate unaligned-copy instruction
   groups. */
typedef struct PlayerSpawnGridPos {
	struct MapChunk chunk;
	struct MapTile tile;
} PlayerSpawnGridPos;

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

/* Opaque 0x28-byte block, block-copied wholesale by DreamSys__SaveLinkSnapshot (read) /
   DreamSys__RestoreLinkSnapshot (write) -- see DreamSysUnk14::unk_0x44 below. Declared as a
   word array (not a byte array) so a whole-struct assignment reproduces
   retail's 4-word-per-iteration block-move codegen, per the confirmed idiom
   in DECOMPILATION_LEARNINGS.md ("A whole-struct assignment... for a block
   copy"); internal field layout is unconfirmed (round 2026-09-02). */
typedef struct DreamSysUnk14Tail {
	s32 raw[0x28 / 4];
} DreamSysUnk14Tail;

/* Struct pointed to by DreamSys::unk_0x14. Confirmed fields, all from
   DreamSys__SaveLinkSnapshot/DreamSys__RestoreLinkSnapshot (round 2026-09-02): +0x0 is a word cleared to
   0 by DreamSys__RestoreLinkSnapshot (restore) after the rest of the struct is overwritten;
   +0x38 is the pre-existing 3-word vector (Class6B5CC__LocalOffsetToWorldPos/DreamSys__ProjectPointAtDistance,
   both MATCHED); +0x44 is a pointer to a DreamSysUnk14Tail, itself
   block-copied (not just followed) by the same two functions. The whole
   0x50-byte struct (this field included, raw) is block-copied to/from
   DreamSys::unk14Snapshot -- see that field's comment. Bytes 0x4..0x38 and
   0x48..0x50 are unconfirmed padding. */
typedef struct DreamSysUnk14 {
	s32 unk_0x0;
	s32 unknown_values_0x4[(0x18 - 0x4) / 4];
	/* Compared against -0x1F3 (-499) by DreamSys__ApplyMoveCommand (round 2026-09-06):
	   must be >= that, together with unk_0x1C's own bound below, to run a
	   WallLink call -- only reached while DreamSys::currentStage is 0. */
	s32 unk_0x18;
	/* Compared against -0x7D0 (-2000) by DreamSys__ApplyMoveCommand (round 2026-09-06):
	   must be < that alongside unk_0x18 above. */
	s32 unk_0x1C;
	s32 unknown_values_0x20[(0x38 - 0x20) / 4];
	s32 vec[3];
	DreamSysUnk14Tail *unk_0x44;
	s32 unknown_values_0x48[0x8 / 4];
} DreamSysUnk14;

/* Struct pointed to by DreamSys::heightCurve. +0x14 / +0x20 are a pair of
   two-word (x,y) points per DreamSys__ProjectPointAtDistance (MATCHED, elsewhere in
   this unit; not independently re-confirmed this round). +0x24 (the second point's y) is
   confirmed: DreamSys__StepLookOffset (round 2026-08-30) nudges it. +0x18 (the first
   point's y) is also now confirmed: DreamSys__AdvanceMoveCycle (round 2026-09-02) nudges
   it by the SAME delta as +0x24, in the same statement pair. */
typedef struct DreamSysUnk5C {
	s8 unknown_values_0x0[0x18];
	s32 startValue;
	s8 unknown_values_0x1C[8];
	s32 endValue;
} DreamSysUnk5C;

/* Object pointed to by DreamSys::soundObj, used by DreamSys__StopVoice (slot
   +0x84) and, this round, ExecuteLink (slot +0x80): loaded, dereferenced
   for its own vtable pointer at offset 0, and called through. Everything
   else about this class -- including whether it is the SAME class as
   DreamSys::unk_0x4C below -- is unknown. Elsewhere in this unit soundObj
   is set/read as a plain s32 (DreamSys__SetSoundObj, DreamSys__StopDrift's call into
   FlushSoundCueSet), which is consistent with it being a pointer value just
   not typed that way there. slot0x84 takes TWO arguments, not one --
   head-adjudicated 2026-08-30-c: the guard value (DreamSys::voiceIndex)
   loaded into $a1 by DreamSys__StopVoice is never overwritten before the jalr, so
   it is passed through, not just branched on. See DreamSys__StopVoice.md.
   slot0x80 (ExecuteLink, round 2026-09-02) takes three arguments, all
   literal constants at that call site (0x90, 0x6E, 0x6E) -- nothing here
   suggests what they mean.

   DreamSys__StartVoice (round 2026-09-06) adds two more confirmed facts: slot0x80
   DOES return a value -- it stores the result into DreamSys::voiceIndex on one
   call path -- so its return type widens from `void` to `s32` here; this is
   safe for every existing call site (ExecuteLink, DreamSys__DreamSys's own
   slot0x80 use on the DIFFERENT DreamSysCtorArgObj vtable below) because none
   of them ever read $v0 after the call, so a discarded s32 return compiles
   identically to a void one. DreamSys__StartVoice also reaches a new slot at +0x9C,
   one argument, called three times with a byte-table value and small
   literal constants (1, 2); nothing here suggests what it does either. */
typedef struct DreamSysUnk58Vtable {
	u8 pad00[0x80];
	s32 (*slot0x80)(void *self, s32 a1, s32 a2, s32 a3);
	void (*slot0x84)(void *self, s32 flag);
	u8 pad88[0x14];
	void (*slot0x9C)(void *self, s32 a1);
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

/* Object pointed to by DreamSys__SpawnAtLink's `arg1` parameter -- same
   "vtable pointer at offset 0" shape as this unit's other opaque classes.
   Unidentified; may or may not be the same class as DreamSysCtorArgObj
   above (both are eventually forwarded to vt->slot10, but this one is
   passed through directly rather than via a derived value) -- kept as a
   separate local view per this unit's "multiple independent views of one
   table" convention until proven otherwise (round 2026-09-02). */
typedef struct DreamSysSpawnArgMethods {
	u8 pad00[0xE4];
	/* Called by DreamSys__SpawnAtLink as (arg1, &local, this,
	   &this->linkCoordinates); return value discarded. */
	void (*slot0xE4)(void *self, void *arg1, struct DreamSys *arg2, PlayerSpawnPoint *arg3);
} DreamSysSpawnArgMethods;
typedef struct DreamSysSpawnArgObj {
	DreamSysSpawnArgMethods *methods;
} DreamSysSpawnArgObj;

/* Inner struct chased by DreamSys__NotifyLinkAttempt's `arg1 == -2` path: the return of
   DreamSysUnk4CMethods::slot0x11C (a DreamSysUnk11CResult below) has a
   pointer at +0x4 to one of THESE, and only +0x2C (a `s16`, compared
   against the literal 2) is read (round 2026-09-02). */
typedef struct DreamSysUnk11CInner {
	s8 unknown_values_0x0[0x2C];
	s16 unk_0x2C;
} DreamSysUnk11CInner;

/* Return type of DreamSysUnk4CMethods::slot0x11C. Only +0x4 (a pointer to
   DreamSysUnk11CInner above) is read, by DreamSys__NotifyLinkAttempt (round 2026-09-02). */
typedef struct DreamSysUnk11CResult {
	s8 unknown_values_0x0[4];
	DreamSysUnk11CInner *unk_0x4;
} DreamSysUnk11CResult;

/* Object pointed to by DreamSys::unk_0x4C, used by DreamSys__UnlinkLinkMgr (slot
   +0xF0) and DreamSys__TryInstantTeleportLink (slot +0xE8, this round): same "vtable pointer
   at offset 0" shape as DreamSysUnk58 above. Unidentified class; unknown if
   related to DreamSysUnk58. */
typedef struct DreamSysUnk4CMethods {
	u8 pad00[0xD4];
	/* Called by DreamSys__WallLink as (this->unk_0x4C, arg1), return value
	   whole-struct-assigned into this->linkCoordinates (a PlayerSpawnPoint)
	   (round 2026-09-02). */
	PlayerSpawnPoint *(*slot0xD4)(void *self, void *arg1);
	u8 pad_0xD8[0xE8 - 0xD8];
	/* Called by DreamSys__TryInstantTeleportLink as (this->unk_0x4C, &local, &this->
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
	   DreamSys__NotifyLinkAttempt's shared tail block, whose return value is forwarded
	   straight into vtable slot +0x1D4 (DreamSys__TryStageTimerLink)'s `currentPos`
	   argument (round 2026-09-02) -- same signature, different caller. */
	PlayerSpawnPoint *(*slot0x10C)(void *self, s32 arg1, s32 arg2);
	/* Called by this unit's own DreamSys__FindNearbyLink as (this->unk_0x4C, out,
	   this->unk_0x14 + 0x18) -- an output buffer (`out`, later read by
	   DreamSys__BuildLinkQueries as its own `arg3`) and the same "self->unk_0x14 + 0x18"
	   raw byte-offset position pointer as slot0x11C's own call site above
	   (round 2026-09-04). Named `queryLinkAtPos`: it fills a `LinkQueryBuf`
	   (`class_3bb8c_p.c`'s own type) from a world position, returning 0 on
	   success -- accessed only from `class_3bb8c_p.c` (round 57 naming
	   pass). */
	s32 (*queryLinkAtPos)(void *self, void *out, void *pos);
	u8 pad_0x114[0x118 - 0x114];
	/* DreamSys__BuildLinkQueries (class_3bb8c_p.c) dispatches to it twice,
	   both times as (this->unk_0x4C, pos) with pos an adjacent grid index
	   (s3 +/- 1), and stores the result into a `GridArrElem *` array slot.
	   Round 75 corrected the arity: retail sets only a0/a1 at the second
	   call, and the a2/a3 values visible at the first are the caller's own
	   leftovers; a 4-argument type kept two extra callee-saved registers
	   live (DreamSys__BuildLinkQueries's report). Named `getGridArrElemAt`
	   in the round 57 naming pass; accessed only from class_3bb8c_p.c. */
	void *(*getGridArrElemAt)(void *self, s32 pos);
	/* Called by DreamSys__NotifyLinkAttempt's `arg1 == -2` path as (this->unk_0x4C,
	   (u8 *)this->unk_0x14 + 0x18); the result's `unk_0x4` is chased and its
	   `unk_0x2C` compared against the literal 2 (round 2026-09-02). */
	DreamSysUnk11CResult *(*slot0x11C)(void *self, void *arg1);
} DreamSysUnk4CMethods;

/* Object pointed to by DreamSysUnk4CObj::unk_0x68 -- only the two fields
 * this unit's own DreamSys__BuildLinkQueries reads are named (round 2026-09-04). */
typedef struct DreamSysUnk4C68Obj {
	u8 pad00[0x2];
	s16 unk_0x2;
	s32 unk_0x4;
} DreamSysUnk4C68Obj;

typedef struct DreamSysUnk4CObj {
	DreamSysUnk4CMethods *methods;
	u8 pad04[0x68 - 0x4];
	/* Read by this unit's own DreamSys__BuildLinkQueries (round 2026-09-04). */
	DreamSysUnk4C68Obj *unk_0x68;
} DreamSysUnk4CObj;

/* Shared intermediate base class table (D_800878D4 -- see
   docs/research/class-framework.md and code_55dd4.h's D800878D4Methods,
   which types the same table for Class65650, a sibling of DreamSys under
   this same base). Declared locally here rather than pulled in from
   code_55dd4.h to avoid a cross-unit include; only slot +0x050 is needed by
   this unit (DreamSys__UnlinkLinkMgr, this round). */
typedef struct DreamSysBaseMethods {
	u8 pad00[0x8];
	/* Shared with Class65650's own inherited "ctor" slot at the same offset
	   in the SAME base table (code_55dd4.h's D800878D4Methods, which already
	   names and resolves this exact slot as `BaseObjO__BaseObjO`, taking/
	   returning `Class65650 *self`). Called by DreamSys__DreamSys as
	   (this), its return value discarded (round 2026-09-02) -- consistent
	   with the base ctor returning `self` for chaining, unneeded here since
	   the caller already has `this`. */
	struct DreamSys *(*ctor)(struct DreamSys *self);
	u8 pad0C[0x4C - 0xC];
	/* Shared with Class65650's own inherited "slot4C" at the same offset in
	   the SAME base table (code_55dd4.h's D800878D4Methods: "called by
	   Class65650__AttachToParent as slot4C(self, arg3, arg5)"). Called by
	   DreamSys__SpawnAtLink as (this, arg1, &local) (round 2026-09-02). */
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
	/* Called by DreamSys__NotifyLinkAttempt (this unit's own +0x088 slot) as
	   (this, arg1), return value discarded (round 2026-09-02). */
	void (*slot0x88)(struct DreamSys *self, s32 arg1);
	u8 pad8C[0x9C - 0x8C];
	/* Called by DreamSys__DispatchChunkChange as (this, arg1, arg2) -- round 2026-08-30-d. */
	void (*slot0x9C)(struct DreamSys *self, void *arg1, s32 arg2);
	u8 padA0[0xDC - 0xA0];
	/* Called unconditionally by DreamSys__DispatchInstanceEffect (this unit's own +0xDC slot)
	   as (this, arg1, arg2) -- same argument shape as slot0x9C above
	   (round 2026-09-02). Resolves to DreamSys__DispatchLinkCommandAndTryAttach in D_800878D4, out of
	   this unit's scope. */
	void (*slot0xDC)(struct DreamSys *self, void *arg1, s32 arg2);
	/* Called by DreamSys__WallLink as (this, arg1, arg2) -- same argument
	   shape as slot0x9C/slot0xDC above (round 2026-09-02). */
	void (*slot0xE0)(struct DreamSys *self, void *arg1, s32 arg2);
} DreamSysBaseMethods;
extern DreamSysBaseMethods *DreamSys__GetBaseMethods(void);

/* Opaque view of whatever object DreamSys__ProcessChunkChange's `entity`
   parameter points to -- almost certainly an `Entity*` (include/Entity.h),
   but that unit's own `EntityMethods` doesn't type these slots and
   extending it is out of this unit's scope. Declared minimally, locally,
   for this unit's own call sites only (round 2026-08-30-d;
   +0x38/+0x14C/+0x150/+0x154/+0x158 added round 2026-09-06 by
   DreamSys__InstanceEffectsOnJournal). +0x38 takes the DreamSys instance
   as its own second argument, same shape as DreamSysBaseMethods::slot0x50
   above; +0x14C returns a pointer forwarded straight into
   LogInstanceMood, so `MoodGraphPoint *`; +0x150/+0x154 return plain s32
   (added to/negated into DreamSys fields); +0x158's return is stored with
   a bare `sh`, consistent with either `s16` or `s32` at this call shape,
   kept `s32` for uniformity with its self-only siblings. */
typedef struct DreamSysEntityMethods {
	u8 pad00[0x38];
	/* Three arguments, not two (round 75, DreamSys__InstanceEffectsOnJournal):
	   the caller forwards its own `effect` in $a2 untouched, which is why its
	   switch index lives in $v1. Only caller: that function. */
	void (*slot0x38)(void *self, struct DreamSys *arg1, s32 effect);
	u8 pad3C[0x10C - 0x3C];
	PlayerSpawnPoint *(*slot0x10C)(void *self, s32 arg1, s32 arg2);
	u8 pad110[0x14C - 0x110];
	MoodGraphPoint *(*slot0x14C)(void *self);
	s32 (*slot0x150)(void *self);
	s32 (*slot0x154)(void *self);
	s32 (*slot0x158)(void *self);
} DreamSysEntityMethods;
typedef struct DreamSysEntityObj {
	DreamSysEntityMethods *methods;
} DreamSysEntityObj;

/* Struct pointed to by DreamSys__SoundCueCallback's arg1 -- forwarded (never called) as
   InitSoundCueSet's 5th argument via DreamSys__SelectCallback98's arg1==2 case, so its
   real caller/owner lives outside this unit. Only the offsets
   DreamSys__SoundCueCallback itself touches are named; +0x8..+0x1C and +0x24..+0x30
   are unconfirmed gaps (round 2026-08-30-d). */
typedef struct SoundCueCallbackArg {
	s32 mode;             /* +0x0, compared against literal 1 */
	s32 value;              /* +0x4, divided by 20 */
	s8 unknown_values_0x8[0x14];
	s32 field_0x1C;
	s32 field_0x20;
	s8 unknown_values_0x24[0xC];
	s32 field_0x30;
	s32 field_0x34;
} SoundCueCallbackArg;

/* Full-word (x,y,z) vector, distinct from `struct RelativePos` (s16 triplet
   -- the on-disk/network form). DreamSys__ApplyRelativeOffset builds one of these on the
   stack as a-b with y forced to 0; DreamSys__TickDrift passes the static
   DRIFT_STEP instance of one. Both feed vtable slot +0xBC
   (BaseObjO__AddVec14, round 2026-08-30-d). */
typedef struct DreamSysVec3 {
	s32 x, y, z;
} DreamSysVec3;
extern DreamSysVec3 DRIFT_STEP;

/* gProjectOffsetZ is the LAST word of an unnamed 3-word (DreamSysVec3-shaped)
   global scratch vector; the other two words are NOT independently named
   -- splat's dlabel boundary put them inside `VOICE_PITCH_BY_SELECT`'s dlabel as
   unlabeled tail bytes (asm/data/783DC.data.s), because nothing took their
   address directly until DreamSys__ProjectPointAtDistance (round 19). Do not rename/resegment
   this round (config/ out of scope); reach the vector's start with pointer
   arithmetic off this symbol instead: `(DreamSysVec3 *)((s32 *)&gProjectOffsetZ
   - 2)`.

   Two independent pieces of evidence pin this down, not a guess:
   - `DreamSys__NotifyLinkAttempt` (this unit, already matched) clamps
     `this->voiceSelect = (this->unk_0x28->unk_0x36 & 0x7F); if (voiceSelect >=
     0x18) voiceSelect = 0;` -- i.e. `voiceSelect` is bounded to [0, 0x18). Both
     `VOICE_BY_SELECT` and `VOICE_PITCH_BY_SELECT` (each already-named 24+-byte byte
     tables) are indexed by this SAME bounded value in `DreamSys__StartVoice`
     (`VOICE_PITCH_BY_SELECT[voiceSelect]`), so `VOICE_PITCH_BY_SELECT`'s real, ever-read extent is
     exactly 24 bytes (`0x80087EC8`-`0x80087EDF`) -- the 8 trailing zero
     bytes splat lumped into its dlabel (`0x80087EE0`-`0x80087EE7`) are
     never reached by that indexed access and belong to something else.
   - `Class6B5CC__LocalOffsetToWorldPos` (code_d294_c, already matched) forwards its own `src`
     parameter to `ApplyMatrixToLVArray(dst, src, 1, buf)`, and `ApplyMatrixToLVArray`'s
     own doc comment (include/code_d294.h) confirms it treats both
     pointers as 0xC-byte (3-word) elements. `DreamSys__ProjectPointAtDistance` passes
     `(s32 *)&gProjectOffsetZ - 2` as that exact `src` argument, which only
     type-checks sensibly as a 3-word vector's start -- matching the 8
     "spare" bytes above exactly (2 words = 8 bytes immediately before
     `gProjectOffsetZ`, which is the vector's 3rd word). */
extern s32 gProjectOffsetZ;

/* A `struct RelativePos` constant, passed as DreamSys__ApplyRelativeOffset's `a` argument
   by DreamSys__TickStaircaseCase2 (round 2026-09-02). */
extern struct RelativePos STAIRCASE_OFFSET_2;

/* Another `struct RelativePos` constant, passed as DreamSys__ApplyRelativeOffset's `a`
   argument by DreamSys__TickStaircaseCase0 -- same call shape as STAIRCASE_OFFSET_2 above, just a
   different constant (round 2026-09-02). */
extern struct RelativePos STAIRCASE_OFFSET_0;

/* Another `struct RelativePos` constant, passed as DreamSys__ApplyRelativeOffset's `a`
   argument by DreamSys__TickStaircaseCase1 -- same call shape as STAIRCASE_OFFSET_0/STAIRCASE_OFFSET_2
   above, just a different constant (round 2026-09-02). */
extern struct RelativePos STAIRCASE_OFFSET_1;

/* Another `struct RelativePos` constant, passed as DreamSys__ApplyRelativeOffset's `a`
   argument by DreamSys__TickStaircaseCase3 -- same call shape as STAIRCASE_OFFSET_2/STAIRCASE_OFFSET_0
   above, just a different constant (round 2026-09-02). */
extern struct RelativePos STAIRCASE_OFFSET_3;

/* (0 deg, +45 deg, 0 deg), forwarded as vtable slot +0x044's (Class6B5CC__UpdateRotation)
   arg2 with flag 0 (relative) by DreamSys__TickStaircaseCase0 and
   DreamSys__TickStaircaseCase2. Typed RotationRatios round 66: its three
   {numerator, denominator} words are {0,1} {0x2D,1} {0,1}, byte-identical in
   form to ROTATION_YAW_180 and to every CARDINAL_ROTATIONS entry. */
extern RotationRatios ROTATION_YAW_PLUS45;

/* (0 deg, -45 deg, 0 deg) -- the mirror of ROTATION_YAW_PLUS45 above
   ({0,1} {0xFFD3,1} {0,1}), used the same way by
   DreamSys__TickStaircaseCase1 and DreamSys__TickStaircaseCase3. */
extern RotationRatios ROTATION_YAW_MINUS45;

/* Argument shape for InterpolateKeyframeValue: two "keyframe" points, each with a
   value (+0x4) and a position/time (+0x8); offset +0x0 unconfirmed
   (unread by this function). Called by still-INCLUDE_ASM DreamSys__ProjectPointAtDistance as
   InterpolateKeyframeValue(&this->heightCurve->unknown_values_0x0[0x14], arg2, arg3) --
   the first argument is one of DreamSysUnk5C's two documented "point"
   fields (round 2026-08-30-d). */
typedef struct DreamSysInterpPoint {
	s8 unknown_values_0x0[4];
	s32 value;
	s32 position;
} DreamSysInterpPoint;

/* Struct pointed to by DreamSys::unk_0x28. Only +0x36 is touched (a `u16`,
   masked to 0x7F by DreamSys__NotifyLinkAttempt, round 2026-09-02); everything else is
   unconfirmed. */
typedef struct DreamSysUnk28Target {
	s8 unknown_values_0x0[0x36];
	u16 unk_0x36;
} DreamSysUnk28Target;

/* 3x3 lookup table indexed by [dynamicClass][upperClass], each axis
   classified into {0,1,2} by CalcDreamColor first (round 2026-08-30-d). */
extern s8 DREAM_COLOR_TABLE[9];

/* Byte tables indexed by DreamSys::voiceSelect (already bounded to [0,0x18) at
   the write site -- see that field's own comment). DreamSys__StartVoice
   (round 2026-09-06) reads both: VOICE_BY_SELECT[voiceSelect] (values 0..0x1E) feeds
   DreamSysUnk58Vtable::slot0x80's `a1` argument, left-shifted by 4;
   VOICE_PITCH_BY_SELECT[voiceSelect] (values include -2..2, hence `s8` not `u8`) feeds
   slot0x9C's `a1` argument directly. VOICE_PITCH_BY_SELECT's real extent is exactly
   these 24 bytes -- the trailing zero bytes splat lumped into its dlabel
   belong to the gProjectOffsetZ vector documented above, not to this table. */
extern const s8 VOICE_BY_SELECT[0x18];
extern const s8 VOICE_PITCH_BY_SELECT[0x18];

/* BasicClass-family allocator; see code_171e0.h / code_55dd4.h / Entity.h /
   class_16334.h for the other units that also declare it locally. */
extern void *BMemPMgrAlloc(s32 size);

/* Also declared in Entity.h for a different (Entity) struct's fields; here
   called by DreamSys__StopDrift as (this->soundObj, this->soundCueSet)
   (round 2026-08-30-b). */
extern void FlushSoundCueSet(s32 arg0, void *arg1);

/* Also declared in Entity.h. Called by DreamSys__TickDrift as
   (this->soundObj, this->soundCueSet) -- same argument shape as
   FlushSoundCueSet above (round 2026-08-30-d). */
extern void ServiceSoundCueSet(s32 arg0, void *arg1);

typedef struct DreamSys {
	struct vtable_DreamSys *vt;
	s8 unknown_values_0x4[8];

	/* Gate observed by Class6B5CC__LocalOffsetToWorldPos and DreamSys__ProjectPointAtDistance: when nonzero,
	   unk_0x14 is treated as valid and its "+0x38" vector is used;
	   otherwise a zero vector is used instead. Meaning of the pointed-to
	   struct is unidentified. */
	s32 unk_0xC;
	s8 unknown_values_0x10[4];

	/* Pointer to DreamSysUnk14 (see that type for confirmed fields); a
	   3-word vector lives at +0x38 of what this points to (read by
	   Class6B5CC__LocalOffsetToWorldPos / DreamSys__ProjectPointAtDistance, guarded by unk_0xC above). The whole
	   struct is saved/restored to/from unk14Snapshot below by
	   DreamSys__SaveLinkSnapshot/DreamSys__RestoreLinkSnapshot (round 2026-09-02). */
	DreamSysUnk14 *unk_0x14;
	s8 unknown_values_0x18[12];

	s32 dreamTimer;
	/* Read as a pointer by DreamSys__NotifyLinkAttempt (round 2026-09-02): loaded, then
	   dereferenced at +0x36 for a `u16` (masked to 0x7F and stashed into
	   voiceSelect below). Target type otherwise unconfirmed. */
	DreamSysUnk28Target *linkTarget;
	s8 unknown_values_0x2C[24];

	s32 pendingLinkType;
	/* Written by DreamSys__ApplyOffsetSlotAndNotify (this unit's own helper, invoked via its own
	   +0x0C8/+0x0CC slots DreamSys__ApplyOffsetSlot0/DreamSys__ApplyOffsetSlot1) and by DreamSys__SetLastOffsetValue
	   (this unit's own +0x0E4 slot), both as a plain `sh` store of a `s16`
	   value (round 2026-09-04). Named `lastOffsetValue`: it mirrors whichever
	   local-offset component was most recently written, but survives the
	   transient scratch buffer's own reset back to 0 right after
	   DreamSys__ApplyOffsetSlotAndNotify applies it -- accessed only from
	   `class_3bb8c_p.c` (round 57 naming pass). */
	s16 lastOffsetValue;
	s8 unknown_values_0x4A[2];
	/* Pointer to an unidentified object (own vtable at offset 0, slot
	   +0xF0 called with itself as the sole argument). Used by
	   DreamSys__UnlinkLinkMgr (round 2026-08-30-b); see DreamSysUnk4CObj above. */
	DreamSysUnk4CObj *linkMgr;
	s8 unknown_values_0x50[4];
	/* Written by this unit's own DreamSys__SetPendingExtra (its own +0x0EC slot), a
	   plain `sw` store of its `extra` argument (round 2026-09-04). Named
	   `pendingExtra`: no reader is confirmed yet, and it is accessed only
	   from `class_3bb8c_p.c` (round 57 naming pass). */
	void *pendingExtra;

	/* Set by DreamSys__SetSoundObj(this, value); no other observed use. */
	s32 soundObj;
	/* Set by DreamSys__SetHeightCurve(this, value); read as a pointer by
	   DreamSys__ProjectPointAtDistance (this->heightCurve + 0x14 and + 0x20 are passed to
	   InterpolateKeyframeValue), so it points to a pair of two-word (x,y) points. */
	DreamSysUnk5C *heightCurve;
	/* Set unconditionally to the constructor's `arg1` by DreamSys__DreamSys
	   (round 2026-09-02) -- see DreamSysCtorArgObj. No other observed use in
	   this unit's queued functions. */
	void *unk_0x60;
	/* Set by DreamSys__func_5938c(this, value); no other observed use. */
	s32 unk_0x64;

	bool isFlashbackSession;
	/* Read by DreamSys__TickMove; compared against 0 / 1, else-branch otherwise.
	   Meaning unidentified beyond that (round 2026-08-30). */
	s32 moveOverride;

	/* Gate flag: DreamSys__BlockMovement sets it to 1; DreamSys__GetLinkCommandFlag reads it back;
	   DreamSys__UpdateTickState skips its whole body while this is nonzero. */
	s32 movementBlocked;
	/* Cleared to 0, then set to (dreamTimer % tickPeriod == 0) by
	   DreamSys__UpdateTickState. */
	s32 linkCommandFlag;
	/* Cleared to 0 by DreamSys__func_59598; no other observed use. */
	s32 unk_0x78;
	/* Cleared to 0 by DreamSys__func_59590; no other observed use. */
	s32 unk_0x7C;
	/* Set by DreamSys__SelectCallback80(this, arg1): NULL when arg1==0, otherwise one of
	   three vtable-slot function pointers selected by arg1 (1/2/3). Called
	   with (this) by DreamSys__RunTickCallbacks, if non-NULL. */
	void (*callback_0x80)(struct DreamSys *this);
	/* Set unconditionally to arg1 by DreamSys__SelectCallback80(this, arg1); no other
	   observed use (round 2026-08-30). */
	s32 callback80Mode;
	/* Index into the (LOOK_OFFSET_STEPS, LOOK_OFFSET_LIMITS) delta/threshold table pair,
	   consumed and reset to 0 by DreamSys__StepLookOffset (round 2026-08-30). */
	s32 lookOffsetCommand;
	/* Running accumulator nudged by lookOffsetCommand's table entry, or decayed by
	   600/call towards 0 when lookOffsetCommand is 0; also propagated into
	   heightCurve->endValue. Set by DreamSys__StepLookOffset (round 2026-08-30). */
	s32 lookOffset;
	/* Index into the (LOOK_YAW_STEPS, LOOK_YAW_LIMITS) delta/threshold table pair,
	   consumed and reset to 0 by DreamSys__StepLookYaw (round 2026-08-30). */
	s32 lookYawCommand;
	/* Running delta accumulator paired with lookYawCommand; see DreamSys__StepLookYaw
	   (round 2026-08-30). */
	s32 lookYaw;
	/* Set by DreamSys__SelectCallback80(this, arg1) exactly like callback_0x80, but from
	   a *different* trio of vtable slots. Called with (this) by
	   DreamSys__RunTickCallbacks, if non-NULL. */
	void (*callback_0x98)(struct DreamSys *this);
	/* "Mode" field read/written by DreamSys__SelectCallback98(this, arg1): when ==2 on
	   entry, this->vt->DreamSys__StopDrift(this, 0) fires first; then it is set
	   unconditionally to arg1 (round 2026-08-30). */
	s32 callback98Mode;
	/* (this->moveCommand ^ 1) < 1u, i.e. (moveCommand == 1), written by
	   DreamSys__StepLookYaw; also toggled/incremented by DreamSys__FlipMoveCommand and forced
	   to 1 by DreamSys__TickMoveForced (round 2026-08-30). */
	s32 moveCommand;
	/* Index into the 12-byte-stride TURN_ROTATIONS table; consumed and reset
	   to 0 by DreamSys__ApplyPendingTurn (round 2026-08-30-b). */
	s32 turnCommand;
	/* (moveCommand == 1) as computed by DreamSys__StepLookYaw; unconditionally cleared
	   to 0 by DreamSys__FlipMoveCommand on every call (round 2026-08-30). */
	s32 moveCommandLatch;
	/* "Current" value; DreamSys__RestorePreviousMoveMode overwrites this with previousMoveMode.
	   DreamSys__GetSetMoveMode's bounds-checked setter (vtable +0x180) writes both
	   this and previousMoveMode together; DreamSys__ChangeMoveMode copies the OLD value of
	   this into previousMoveMode before overwriting it, when the new value
	   differs (round 2026-08-30-b). */
	s32 moveMode;
	/* "Previous"/paired value; see moveMode (round 2026-08-30-b). */
	s32 previousMoveMode;
	/* Attempt/beat counter incremented (and bounded to [0,4)) by
	   DreamSys__AdvanceMoveCycle on every call while moveCommand is nonzero; reset to 0 once
	   moveCommand goes back to 0. Compared against 3 there to pick a +-50
	   nudge applied to heightCurve's y fields, and against 4 (together with
	   moveMode) to force moveCommand back to 0 (round 2026-09-02). */
	s32 moveCycleTick;
	/* Derived from `unknown_values_0x28[0x36]` masked to 0x7F, or forced to
	   0 (if >= 0x18) or 2 (if `pendingLinkType == 15` and this is still 0)
	   by DreamSys__NotifyLinkAttempt's `arg1 == -1` path (round 2026-09-02). Also an index:
	   DreamSys__StartVoice (round 2026-09-06) does nothing when this is 0, else
	   uses it to index VOICE_BY_SELECT/VOICE_PITCH_BY_SELECT (see those externs), compares
	   it against 0x16 (22) to decide whether to keep or discard
	   voiceIndex's new value, and against 0xB (11) to gate two extra vtable
	   calls. */
	s32 voiceSelect;
	/* Gate flag: DreamSys__StopVoice runs its body (a call through
	   soundObj->vt->slot0x84, then resets this to -1) only while this is
	   >= 0 (round 2026-08-30-b). */
	s32 voiceIndex;
	s8 unknown_values_0xC0[4];
	/* Set to 1 by DreamSys__SelectCallback98's arg1==2 case, alongside cueServiceActive and
	   callback_0x98 (round 2026-08-30). */
	s32 driftActive;
	/* Set to 1 by DreamSys__SelectCallback98's arg1==2 case, alongside driftActive
	   (round 2026-08-30). */
	s32 cueServiceActive;
	/* Struct initialized in-place by InitSoundCueSet (still INCLUDE_ASM, in
	   the uncarved code_179d8) via DreamSys__SelectCallback98's arg1==2 case; internal
	   layout unknown beyond that entry point (round 2026-08-30). */
	s8 soundCueSet[0x54];

	/* Divisor for DreamSys__UpdateTickState's (dreamTimer % tickPeriod) check. */
	s32 tickPeriod;
	/* Result of DreamSys__UpdateTickState's (dreamTimer % tickPeriod == 0) check. */
	s32 tickBoundary;
	/* tickBoundary/0x128/0x12C/0x130 are also bounds-checked-set as a group of
	   four by DreamSys__SetGateFlags (vtable +0x18C): each is overwritten with the
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
	s32 saveMagic;
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

	s32 newGamePending;
	s32 currentFlashbackIndex;
	/* Set (whole word) by DreamSys__TryStageTimerLink to GetStageLinkAngle()'s return value,
	   right before an ExecuteLink (round 2026-09-02). */
	s32 stageLinkAngle;
	/* Gate flag read by DreamSys__SetMoveOverride (round 2026-08-30-b): when nonzero
	   (reusing the SAME loaded value, not a fresh 0/1 test), forwarded as
	   Class6B5CC__UpdateRotation's arg2 -- cast from s32 to void*, not dereferenced. */
	s32 enterRotation;
	/* Zeroed (whole word) by DreamSys__TryStageTimerLink alongside enterRotation
	   (round 2026-09-02). */
	s32 exitRotation;

	s32 storedDay;

	/* The struct previously ended here (0x890), but New_DreamSys allocates
	   sizeof(DreamSys) via a literal `ori $a0, $zero, 0x928` -- 0x98 bytes
	   more than any field so far discovered accounts for. Extended to the
	   allocator's real size (round 2026-08-30-b); the four words
	   DreamSys__ResetSessionState clears are named, the rest of the tail is still
	   unclaimed.

	   The first 0x78 bytes of that tail are a save/restore scratch buffer
	   for *unk_0x14: DreamSys__SaveLinkSnapshot copies *unk_0x14 (0x50 bytes) then
	   *unk_0x14->unk_0x44 (0x28 bytes, DreamSysUnk14Tail) into these two
	   fields; DreamSys__RestoreLinkSnapshot copies them back and then clears
	   unk_0x14->unk_0x0 to 0 (round 2026-09-02). */
	DreamSysUnk14 unk14Snapshot;
	DreamSysUnk14Tail unk14TailSnapshot;
	s32 staircaseActive;
	/* Compared with an UNSIGNED `< 1` (sltiu) by DreamSys__ApplyMoveCommand (round
	   2026-09-06) -- typed `u32` rather than `s32` to reproduce that,
	   confirmed safe since its only two writers (round 2026-08-30) both
	   set it to the literal 0. */
	u32 staircaseMoveGate;
	/* Function pointer, called as `staircaseTickFn(this)` and its `s32` result
	   used as a truth value (DreamSys__TryStaircaseLink, round 2026-09-06); set from
	   `STAIRCASE_TICK_FNS[GetLastSpawnExtra()]` (both MATCHED) or NULLed --
	   0 is a valid state, tested with a plain `!= 0`/`== 0` before ever
	   being called through. */
	s32 (*staircaseTickFn)(struct DreamSys *this);
	/* A retry/attempt counter (round 2026-09-02, DreamSys__TickStaircaseCase2): read as a
	   whole word, compared against several literal bands, and incremented
	   by 1 at that function's normal exit. */
	s32 staircaseFrame;
	/* See PlayerSpawnGridPos's own comment -- the `chunk`+`tile` half of a
	   PlayerSpawnPoint whole-struct-copied here by DreamSys__TryStaircaseLink. */
	PlayerSpawnGridPos staircaseGridPos;
	/* A `struct RelativePos`, address-taken and passed to DreamSys__ApplyRelativeOffset as
	   its `b` argument (round 2026-09-02, DreamSys__TickStaircaseCase2) -- carved out of
	   what was raw padding in the same 0x10-byte block as staircaseFrame above. */
	struct RelativePos staircaseOrigin;
	s8 unknown_values_0x922[2];
	s32 unk_0x924;
} DreamSys;

/* Dispatch table indexed by DreamSys__ApplyMoveCommand's `arg1`; see that table's own
   comment near MOVE_MODE_SPEEDS/MOVE_COMMAND_SIGNS above. Same element signature as
   DreamSys__DispatchOffsetSlotC4/DreamSys__DispatchOffsetSlot0 below. */
extern void (*MOVE_COMMAND_DISPATCH[5])(DreamSys *this, s32 val, void *extra);

/* 4-entry table of `s32 (DreamSys *this)` functions (DreamSys__TickStaircaseCase0,
   DreamSys__TickStaircaseCase1, DreamSys__TickStaircaseCase2, DreamSys__TickStaircaseCase3, all already matched with
   exactly that signature), indexed by GetLastSpawnExtra()'s return value and
   stashed into DreamSys::staircaseTickFn by DreamSys__TryStaircaseLink (round 2026-09-06). */
extern s32 (*STAIRCASE_TICK_FNS[4])(DreamSys *this);

/* Called by DreamSys__TryStaircaseLink with NO explicit argument setup (the disassembly's
   call site leaves `$a0` holding an unrelated leftover value from the
   preceding statement, same "empty delay slot, no a0-a3 setup" shape as
   GetStageLinkAngle above); return value used as STAIRCASE_TICK_FNS's index. MATCHED
   round 43 (2026-09-15) -- both the gp-relative and addiu_at blockers it was
   filed under are resolved (see docs/research/gp-relative-blocker.md and
   docs/research/addiu-at-blocker.md), and the one-line body
   `STAIRCASE_SPAWNS[gLinkDstStage][gLinkSpawnIndex].extra` matched on the first rebuild
   (docs/match-reports/GetLastSpawnExtra.md). Still declared here to type
   DreamSys__TryStaircaseLink's call site, which remains INCLUDE_ASM in this unit. */
extern s32 GetLastSpawnExtra(void);

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
	   `slot10`, resolved there as `BaseObjO__LinkCompanion`, the "link" companion of
	   `slot14`/`BaseObjO__UnlinkCompanion` immediately below -- this unit already names
	   THAT slot `BaseObjO__UnlinkCompanion` and notes the same companion relationship).
	   Called by DreamSys__DreamSys as (this, arg1->methods->slot0x80(arg1,
	   0)) -- the constructor's own "buddy-link" step (round 2026-09-02). */
	void (*slot10)(DreamSys *this, void *arg);
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x014); shared
	   with Class65650's inherited slot14 (code_55dd4.h: "'unlink' companion
	   of slot10"). Called by DreamSys__UnlinkLinkMgr as (this, this->unk_0x4C)
	   (round 2026-08-30-b). Still INCLUDE_ASM; address 0x80057130 is
	   outside this unit/runner's range. */
	void (*BaseObjO__UnlinkCompanion)(DreamSys *this, DreamSysUnk4CObj *arg1);
	u32 unknown_functions_0x18[6];
	/* +0x030, BasicClass__NotifyParents -- shared base-class slot, same one
	   `class_3ac78.h`/`Class6D3C8.h` name (see their comments); called by
	   ExecuteLink as (this, unk1) with its return discarded
	   (round 2026-09-02). */
	void (*slot30)(DreamSys *this, s32 arg1);
	u32 unknown_functions_0x34[3];
	/* Called by DreamSys__DreamSys as the constructor's LAST step, as
	   (this); its return value is never overwritten before the function's
	   own epilogue, so it becomes DreamSys__DreamSys's own return value
	   unchanged (round 2026-09-02) -- typed `DreamSys *` to match. */
	DreamSys *(*DreamSys__ResetSessionState)(DreamSys *this);
	/* Called by DreamSys__StepLookYaw as (this, 0, &TURN_ROTATION_YAW[-1]); return value,
	   if any, unused (round 2026-08-30). */
	void (*Class6B5CC__UpdateRotation)(DreamSys *this, s32 arg1, void *arg2);
	u32 unknown_functions_0x48[1];
	/* This function's OWN slot; called this round (round 2026-09-02). */
	void (*DreamSys__SpawnAtLink)(DreamSys *this, DreamSysSpawnArgObj *arg1);
	u32 unknown_functions_0x50[4];
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x060). Called by
	   DreamSys__ResetSessionState as (this, 0) (round 2026-08-30-b). Still
	   INCLUDE_ASM; address 0x8001D344 is outside this unit/runner's
	   range. */
	void (*Class6B5CC__SetDisplay)(DreamSys *this, s32 arg1);
	u32 unknown_functions_0x64[9];
	/* This function's OWN slot (+0x088, resolved via
	   tools/classtable.py DREAMSYS_METHODS). Dispatches on `arg1`
	   (-2 / -1 / anything else) after an unconditional call through the
	   shared base table's own +0x088 slot (round 2026-09-02). */
	void (*DreamSys__NotifyLinkAttempt)(DreamSys *this, s32 arg1);
	u32 unknown_functions_0x8c[3];
	void *TimerTick;
	/* This function's OWN slot; resolved via tools/classtable.py
	   (round 2026-08-30-d). */
	void (*DreamSys__DispatchChunkChange)(DreamSys *this, void *arg1, s32 arg2);
	/* Called by this unit's own DreamSys__DispatchLinkCommandAndTryAttach as (this, arg1, count) when
	   `5 <= count < 9` -- dispatched through THIS object's own vtable
	   (unlike DreamSys__DispatchLinkCommandAndTryAttach's other, unconditional call, which goes
	   through the shared base table via GetClass6B5CCMethods() instead).
	   Resolves to Class6B5CC__TryAttachNearby, not overridden at the DreamSys level
	   (round 2026-09-04). */
	void (*tryAttachNearby)(DreamSys *this, void *arg1, s32 count);
	u32 unknown_functions_0xa4[5];
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x0B8). Called by
	   DreamSys__TryInstantTeleportLink right after unk_0x4C->methods->slot0xE8, as (this,
	   &local) using that same output buffer (round 2026-09-02). Address
	   0x80057384 is outside this unit/runner's range; still INCLUDE_ASM. */
	void (*BaseObjO__SetVec14)(DreamSys *this, void *arg1);
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x0BC). Called by
	   DreamSys__ApplyRelativeOffset and DreamSys__TickDrift with a DreamSysVec3* second argument
	   (round 2026-08-30-d). */
	void (*BaseObjO__AddVec14)(DreamSys *this, DreamSysVec3 *arg1);
	/* Called by DreamSys__ApplyOffsetSlotAndNotify (this unit's own helper, invoked by its own
	   +0x0C8/+0x0CC slots) as (this, &D_8008ABA4) -- resolves to
	   BaseObjO__ApplyRotatedVec14, out of this unit/runner's range (round 2026-09-04). */
	void (*BaseObjO__ApplyRotatedVec14)(DreamSys *this, void *arg1);
	/* Read (not called) by this unit's own +0x0D0 slot (DreamSys__DispatchOffsetSlotC4) and
	   forwarded as a raw callback value to DreamSys__ApplyOffsetOrFindNearby -- resolves to
	   BaseObjO__func_5748c, out of this unit/runner's range (round 2026-09-04). */
	void (*BaseObjO__func_5748c)(DreamSys *this, s32 val, void *extra);
	/* This function's OWN slot; forwards (val, extra) to
	   DreamSys__ApplyOffsetSlotAndNotify(this, &D_8008ABA4[0], val, extra, 7)
	   (round 2026-09-04). */
	void (*DreamSys__ApplyOffsetSlot0)(DreamSys *this, s32 val, void *extra);
	/* This function's OWN slot; forwards (val, extra) to
	   DreamSys__ApplyOffsetSlotAndNotify(this, &D_8008ABA4[1], val, extra, 8)
	   (round 2026-09-04). */
	void (*DreamSys__ApplyOffsetSlot1)(DreamSys *this, s32 val, void *extra);
	/* This function's OWN slot; reads its NEIGHBOUR slot +0x0C4
	   (BaseObjO__func_5748c) as a raw callback value and forwards it, with its
	   own two arguments, to DreamSys__ApplyOffsetOrFindNearby (round 2026-09-04). */
	void (*DreamSys__DispatchOffsetSlotC4)(DreamSys *this, s32 val, void *extra);
	/* This function's OWN slot; reads its NEIGHBOUR slot +0x0C8
	   (DreamSys__ApplyOffsetSlot0, self-referential) as a raw callback value and
	   forwards it, with its own two arguments, to DreamSys__ApplyOffsetOrFindNearby
	   (round 2026-09-04). */
	void (*DreamSys__DispatchOffsetSlot0)(DreamSys *this, s32 val, void *extra);
	/* This function's OWN slot; empty stub `{ }` (round 2026-09-04). */
	void (*DreamSys__NoOpSlotD8)(DreamSys *this);
	u32 unknown_functions_0xdc[1];
	/* This function's OWN slot; resolved via tools/classtable.py
	   (round 2026-09-02). */
	void (*LinkWall)(DreamSys *this, void *arg1, s32 arg2);
	/* This function's OWN slot; a single `sh a1, 0x48(a0)` store
	   (`this->lastOffsetValue = val`) (round 2026-09-04). */
	void (*DreamSys__SetLastOffsetValue)(DreamSys *this, s16 val);
	/* This unit's own no-op stub (`DreamSys__NoOpSlotE8Default`, `{ }`). Called by
	   DreamSys__WallLink as (this) -- the callee ignores its argument
	   (round 2026-09-02). */
	void (*DreamSys__NoOpSlotE8Default)(DreamSys *this);
	/* This function's OWN slot; a single `sw a1, 0x54(a0)` store
	   (`this->pendingExtra = extra`) (round 2026-09-04). */
	void (*DreamSys__SetPendingExtra)(DreamSys *this, void *extra);
	u32 unknown_functions_0xf0[2];
	/* This function's OWN slot (+0x0F8, resolved via
	   tools/classtable.py DREAMSYS_METHODS). A straight-line initializer:
	   calls LogChunkMood/DreamSys__SelectCallback80/DreamSys__SelectCallback98/DreamSys__GetSetMoveMode/
	   DreamSys__SetGateFlags/DreamSys__SetTickPeriod in sequence, then zeroes a large block of
	   per-dream state, ending with a Class6B5CC__GetRotationDegrees/Class6B5CC__UpdateRotation pair over a
	   small local buffer (round 2026-09-02). */
	void (*DreamSys__ResetLinkState)(DreamSys *this, s32 arg1, s32 arg2);
	void (*DreamSys__BlockMovement)(DreamSys *this);
	s32 (*DreamSys__GetLinkCommandFlag)(DreamSys *this);
	s32 (*GetSetDreamTimeLimit)(DreamSys *this, s32 time);
	s32 (*DreamSys__GetDreamTimerScaled)(DreamSys *this);
	void (*DreamSys__SetSoundObj)(DreamSys *this, s32 value);
	void (*DreamSys__SetHeightCurve)(DreamSys *this, void *value);
	void (*DreamSys__func_5938c)(DreamSys *this, s32 value);
	void (*DreamSys__UpdateTickState)(DreamSys *this);
	void (*DreamSys__RunTickCallbacks)(DreamSys *this);
	s32 (*DreamSys__ProjectPointAtDistance)(DreamSys *this, void *out, s32 dist, s32 *reference, s32 tolerance);
	void (*DreamSys__func_59590)(DreamSys *this);
	void (*DreamSys__func_59598)(DreamSys *this);
	s32 (*DreamSys__NoOpSlot12C)(DreamSys *this);
	void (*DreamSys__ClearTickCallbacks)(DreamSys *this, bool arg1);
	/* Chains DreamSys__SelectCallback98(this, arg1) then DreamSys__SelectCallback80(this, arg2)
	   (round 2026-08-30). */
	void (*DreamSys__SetTickCallbacks)(DreamSys *this, s32 arg1, s32 arg2);
	/* Called by DreamSys__ClearTickCallbacks(this, TRUE) as this->vt->DreamSys__SelectCallback80(this, 0). */
	void (*DreamSys__SelectCallback80)(DreamSys *this, s32 arg1);
	/* Called unconditionally by DreamSys__ClearTickCallbacks as this->vt->DreamSys__SelectCallback98(this, 0). */
	void (*DreamSys__SelectCallback98)(DreamSys *this, s32 arg1);
	/* Calls DreamSys__StepLookOffset(this) then DreamSys__StepLookYaw(this) (round 2026-08-30). */
	void (*DreamSys__StepLook)(DreamSys *this);
	void (*DreamSys__StepLookOffset)(DreamSys *this);
	void (*DreamSys__StepLookYaw)(DreamSys *this);
	/* No-op stub (`{ }`); one of DreamSys__SelectCallback80's callback_0x80 choices. */
	void (*DreamSys__NoOpSlot14C)(DreamSys *this);
	/* No-op stub (`{ }`); one of DreamSys__SelectCallback80's callback_0x80 choices. */
	void (*DreamSys__NoOpSlot150)(DreamSys *this);
	/* Dispatches on unk_0x6C to DreamSys__ApplyPendingTurn+DreamSys__TickMoveFree, DreamSys__TickMoveHeld,
	   or DreamSys__TickMoveForced, and returns whichever's result (round 2026-08-30). */
	s32 (*DreamSys__TickMove)(DreamSys *this);
	/* Returns movementBlocked unchanged if nonzero; otherwise calls DreamSys__AdvanceMoveCycle
	   and DreamSys__ApplyMoveCommand in sequence and returns the latter's result
	   (round 2026-08-30). */
	s32 (*DreamSys__TickMoveFree)(DreamSys *this);
	/* Forces moveCommand to 1; then either calls DreamSys__AdvanceMoveCycle(this, 0) and
	   returns its result, or chains DreamSys__AdvanceMoveCycle(this, 1) into
	   DreamSys__ApplyMoveCommand and returns THAT result (round 2026-08-30). */
	s32 (*DreamSys__TickMoveForced)(DreamSys *this);
	/* Sets moveCommand to 1 and returns 1 (round 2026-08-30). */
	s32 (*DreamSys__TickMoveHeld)(DreamSys *this);
	/* Referenced by DreamSys__TickMoveFree/DreamSys__TickMoveForced; return value is threaded
	   into DreamSys__ApplyMoveCommand. MATCHED, round 32 (a permuter-found register-
	   forcing lever closed the round-2026-09-02 register-identity/delay-
	   slot-filler residue -- see docs/match-reports/DreamSys__AdvanceMoveCycle.md):
	   bumps moveCycleTick while moveCommand is nonzero,
	   conditionally calls DreamSys__StartVoice (slot +0x168) or DreamSys__StopVoice
	   (slot +0x16C), nudges heightCurve's y fields, and always returns the
	   ORIGINAL moveCommand value read on entry (0 if it was already 0). */
	s32 (*DreamSys__AdvanceMoveCycle)(DreamSys *this, s32 arg1);
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x168). Called by
	   DreamSys__AdvanceMoveCycle as (this), return value discarded -- MATCHED, round 37
	   (the addiu_at blocker this was once filed under is resolved; see
	   docs/match-reports/DreamSys__StartVoice.md). */
	void (*DreamSys__StartVoice)(DreamSys *this);
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x16C). Called by
	   DreamSys__AdvanceMoveCycle as (this) on the path where the +0x168 slot was NOT
	   taken (round 2026-09-02). MATCHED. */
	void (*DreamSys__StopVoice)(DreamSys *this);
	/* Referenced by DreamSys__TickMoveFree/DreamSys__TickMoveForced; MATCHED (110/110 words). */
	s32 (*DreamSys__ApplyMoveCommand)(DreamSys *this, s32 arg1);
	/* Referenced by DreamSys__TickMove; MATCHED (24/24 words). Called with
	   (this) only, return value discarded. */
	void (*DreamSys__ApplyPendingTurn)(DreamSys *this);
	/* Referenced by DreamSys__SelectCallback98's arg1==2 case; stored into
	   callback_0x98, never called directly by this runner's functions. */
	void (*DreamSys__TickDrift)(DreamSys *this);
	/* Referenced by DreamSys__SelectCallback98's entry guard (this->callback98Mode==2); called
	   as (this, 0). */
	void (*DreamSys__StopDrift)(DreamSys *this, s32 arg1);
	/* +0x180..+0x190: resolved via tools/classtable.py DREAMSYS_METHODS,
	   all five matched this round (2026-08-30-b). DreamSys__GetSetMoveMode is also
	   called directly (not through the vtable) by DreamSys__SetMoveOverride, as
	   (this, 1). */
	s32 (*DreamSys__GetSetMoveMode)(DreamSys *this, s32 value);
	void (*DreamSys__ChangeMoveMode)(DreamSys *this, s32 value);
	void (*DreamSys__RestorePreviousMoveMode)(DreamSys *this);
	void (*DreamSys__SetGateFlags)(DreamSys *this, s32 a, s32 b, s32 c, s32 d);
	void (*DreamSys__SetTickPeriod)(DreamSys *this, s32 value);
	/* Referenced by DreamSys__SelectCallback98's arg1==2 case: its raw address (never
	   called there) is forwarded as InitSoundCueSet's 5th argument. arg0 is
	   unused in the body; kept generic rather than typed DreamSys* since
	   nothing here confirms it (round 2026-08-30-d). */
	void (*DreamSys__SoundCueCallback)(void *arg0, SoundCueCallbackArg *arg1);
	void (*InitNewGame)(DreamSys *this);
	void (*GetSetScreenShake)(DreamSys *this, bool *value);
	/* Called by Class6D3C8's slot58 (func_80026410, src/code_1677c.c) as
	   this->vt->DreamSys__GetCurrentDayAndYear(this, 0), compared against 1. Real name and
	   full semantics unknown outside that one call site. arg1 is an OUTPUT
	   pointer (this->currentYear is written through it when non-NULL), not
	   a plain s32 -- retyped round 2026-08-30-c; the one external call site
	   passes literal 0, compatible with either. */
	s32 (*DreamSys__GetCurrentDayAndYear)(DreamSys *this, s32 *arg1);
	s32 (*AdvanceDay)(DreamSys *this);
	/* Zeroes newGamePending unconditionally (round 2026-08-30-c). */
	void (*DreamSys__ClearNewGameFlag)(DreamSys *this);
	/* Getter for newGamePending (round 2026-08-30-c). */
	s32 (*DreamSys__GetNewGameFlag)(DreamSys *this);
	/* Optionally writes a literal 0x700 through arg1 (if non-NULL), always
	   returns &this->saveMagic (round 2026-08-30-c). */
	s32 *(*DreamSys__GetSaveBlock)(DreamSys *this, s32 *arg1);
	s32 (*StartDay)(DreamSys *this);
	s32 (*EndDay)(DreamSys *this, s32 arg1);
	CinematicCall (*GetCinematic)(DreamSys *this);
	void (*InitSpawnLoc)(DreamSys *this);
	void (*DynamicLink)(DreamSys *this);
	bool (*StaticWallLink)(DreamSys *this, PlayerSpawnPoint *currentPos);
	bool (*LoadNextFlashback)(DreamSys *this, bool unknown);
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x1D0) --
	   DreamSys__TryTunnelLink is already matched (`bool (DreamSys *this,
	   PlayerSpawnPoint *currentPos)`, see its definition in DreamSys.c).
	   Called by DreamSys__ApplyMoveCommand (round 2026-09-06) as the third of three
	   "link test" tries, same argument shape as DreamSys__TryStageTimerLink/DreamSys__TryInstantTeleportLink/
	   DreamSys__TryStaircaseLink below. */
	bool (*DreamSys__TryTunnelLink)(DreamSys *this, PlayerSpawnPoint *currentPos);
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x1D4), this
	   round. Tests this->pendingLinkType, then a static-link-with-timer
	   test (Test4StageTransition) against this->linkCoordinates/currentStage/
	   dreamTimer, then ExecuteLinks with literal type 0x10 on success --
	   see DreamSys__TryStageTimerLink.md. */
	bool (*DreamSys__TryStageTimerLink)(DreamSys *this, PlayerSpawnPoint *currentPos);
	/* Resolved via tools/classtable.py DREAMSYS_METHODS (+0x1D8/+0x1DC).
	   Both still INCLUDE_ASM. DreamSys__ApplyMoveCommand (round 2026-09-06) calls
	   DreamSys__TryStaircaseLink (+0x1DC) first, then DreamSys__TryInstantTeleportLink (+0x1D8), then
	   DreamSys__TryTunnelLink above -- same "link test" signature as those, confirmed
	   by this call site alone (neither function's own body has been read
	   yet). */
	bool (*DreamSys__TryInstantTeleportLink)(DreamSys *this, PlayerSpawnPoint *currentPos);
	bool (*DreamSys__TryStaircaseLink)(DreamSys *this, PlayerSpawnPoint *currentPos);
	/* Getter for currentStage (round 2026-08-30-c). */
	s32 (*DreamSys__GetCurrentStage)(DreamSys *this);
	void (*ProcessChunkChange)(DreamSys *this, void *entity, s32 effect);
	/* Renamed from the previous placeholder `InstanceEffectsOnPlayer` --
	   this slot's real symbol (config/symbols.slps01556.lsdde.txt) is
	   `DreamSys__InstanceEffectsOnJournal` (see the forward declaration
	   below and src/DreamSys.c), confirmed via tools/classtable.py
	   DREAMSYS_METHODS (+0x1E8) while resolving DreamSys__DispatchInstanceEffect's call
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
	   *this->unk_0x14 (see DreamSysUnk14). DreamSys__SaveLinkSnapshot saves *unk_0x14
	   and *unk_0x14->unk_0x44 into this->unk14Snapshot/unk14TailSnapshot;
	   DreamSys__RestoreLinkSnapshot restores them and clears unk_0x14->unk_0x0 to 0. Both
	   take only `this` -- the previous note's "driven by a length read
	   from this->unk_0x14" undersold it: unk_0x14 is a POINTER, dereferenced
	   for its own bytes, not read as a length. */
	void (*DreamSys__SaveLinkSnapshot)(DreamSys *this);
	void (*DreamSys__RestoreLinkSnapshot)(DreamSys *this);
	/* This field is named `func_228`, not `DreamSys__func_5ba20`, even though it
	   IS DreamSys__func_5ba20's slot (resolved via tools/classtable.py this
	   round) -- src/code_1677c.c (a different unit, out of this runner's
	   scope) already references it by this name
	   (`self->dreamSys->vt->func_228(...)`), and renaming the field would
	   require an out-of-scope edit there. Do not "fix" this name without
	   updating that call site in the same commit.
	   This slot was previously thought to sit PAST a documented struct end
	   at 0x21c; that was also wrong -- it directly follows
	   ResetFlashbackList/DreamSys__SaveLinkSnapshot/DreamSys__RestoreLinkSnapshot above, no gap.
	   Original call-site note preserved: called once, from Class6D3C8's
	   constructor (func_80025FDC in src/code_1677c.c), as
	   this->vt->func_228(this, arg->unk14) right after DreamSys is
	   allocated by New_DreamSys -- the call site's own signature (single
	   s32 arg, return value discarded) matches DreamSys__func_5ba20's own
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
	/* u8, not s8 (round 2026-09-08, GenerateInitialSpawn): retail reads it
	   with `lbu` -- it indexes SPAWN_POS_ADJUST, so must zero-extend. */
	u8 adjustment;
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
/* Retyped u8 (round 2026-09-08, GenerateInitialSpawn): retail reads it with
   `lbu`, and the surrounding loop guard (`count != 0` implying `count > 0`,
   a single `beqz`) only holds if it can't be negative -- a signed `s8` here
   forces GCC to add a second `blez` check that retail does not have. */
extern u8 LEN_STAGE_SPAWNPOINTS[];

extern StageSpawn* STAGE_PERMALINK_SPAWNS[];
extern StaticLinkTrigger* STAGE_PERMALINK_TRIGGERS[];
extern s8 LEN_STAGE_PERMALINK_TRIGGERS[];

extern s16 SPECIAL_DAYS[];

/* The fixed "special day" mood, returned by IsDaySpecial on a match
   (round 2026-09-02); only ever address-taken there, never dereferenced by
   this unit's queued functions. */
extern MoodGraphPoint SPECIAL_DAY_MOOD;

/* Also declared in Entity.h for the same libc-style function. */
extern s32 rand(void);

extern s8 SPECIAL_COLORS[];

/* Shared by TestForStaticLink/Test4TunnelLinks/Test4StaircaseNodes/
   Test4InstantTeleporters, each of which forwards its own three args
   straight through and appends a fixed trailing quadruple (length table,
   trigger table, spawn table, literal 1). Defined later in this unit's own
   ROM order; this is a forward declaration for the earlier call sites
   above, not a cross-unit prototype. MATCHED (the gp-relative/addiu_at
   blockers this was once filed under are resolved, see CLAUDE.md); return
   type is confirmed s32 by every call site's `bltz` check, not just a guess
   -- CLAUDE.md's tail-call-wrapper warning no longer applies once a
   function is its own real C body, only while it is still INCLUDE_ASM
   (round 2026-08-30-c note superseded). */
extern s32 GetStaticSpawn(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage,
                           s8 *triggerLens, StaticLinkTrigger **triggers, StageSpawn **spawns, s32 flag);

/* Called by DreamSys__TryStageTimerLink as (&this->linkCoordinates, this->currentStage,
   currentPos, this->dreamTimer); result compared with `bltz` exactly like
   TestForStaticLink's call site, so s32 (round 2026-09-02). MATCHED, defined
   later in this unit's own ROM order -- this is a forward declaration, not a
   cross-unit prototype (the gp-relative blocker this was once filed under is
   resolved; see docs/match-reports/Test4StageTransition.md). */
extern s32 Test4StageTransition(PlayerSpawnPoint *target, s32 stage, PlayerSpawnPoint *currentPos, s32 timer);

/* Called by DreamSys__TryStageTimerLink with NO arguments (the disassembly's call site has
   an empty delay slot and no a0-a3 setup); its return value is stored whole
   into this->stageLinkAngle, hence s32 (round 2026-09-02). MATCHED, defined later
   in this unit's own ROM order -- forward declaration only (gp-relative
   blocker resolved; see docs/match-reports/GetStageLinkAngle.md). */
extern s32 GetStageLinkAngle(void);

/* Called by DreamSys__TryTunnelLink as (this, &local) where `local` is a 0x10-byte
   stack buffer also forwarded to DreamSys__CheckTunnelHeading below; return value is
   discarded at this call site (round 2026-09-02). NOT in this unit at all --
   its body disassembles into asm/code_d294.s, an uncarved segment -- so this
   prototype only types this one call site, per the "calling into a function
   that is still INCLUDE_ASM elsewhere is fine" convention
   (DECOMPILATION_LEARNINGS.md). A discarded return is not evidence of
   `void` (same doc); kept `void` here only because nothing at this call
   site constrains it further. */
extern void Class6B5CC__GetRotationDegrees(DreamSys *this, void *arg1);

/* Called by DreamSys__TryTunnelLink as (&this->exitRotation, &this->enterRotation, &local) --
   same `local` buffer Class6B5CC__GetRotationDegrees fills above; result used as a truth
   value (`beqz`), so s32 (round 2026-09-02). MATCHED, defined later in
   this unit's own ROM order -- forward declaration only (the gp-relative
   and addiu_at blockers this was once filed under are both resolved; see
   docs/match-reports/DreamSys__CheckTunnelHeading.md). */
extern s32 DreamSys__CheckTunnelHeading(s32 *arg0, s32 *arg1, void *arg2);

/* Called by DreamSys__TryStaircaseLink (round 2026-09-06) as (&this->linkCoordinates,
   currentPos, this->currentStage) -- same forwarding shape as
   Test4TunnelLinks/TestForStaticLink above. Defined later in this unit's own
   ROM order (`src/DreamSys.c`); this is a forward declaration for that
   earlier call site, not a cross-unit prototype. */
extern s32 Test4StaircaseNodes(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 arg2);

/* Called by DreamSys__TryStaircaseLink as (&this->exitRotation, &this->enterRotation, &local) --
   identical call shape to DreamSys__CheckTunnelHeading above (same `local` buffer, same two
   `this` fields), so the same signature. MATCHED, defined later in this
   unit's own ROM order -- forward declaration only (the gp-relative and
   addiu_at blockers this was once filed under are both resolved; see
   docs/match-reports/DreamSys__CheckStaircaseHeading.md). */
extern s32 DreamSys__CheckStaircaseHeading(s32 *arg0, s32 *arg1, void *arg2);

/* Same (target, currentPos, stage) forwarding shape as Test4TunnelLinks
   above (see that function's own comment) -- called by DreamSys__TryInstantTeleportLink as
   (&this->linkCoordinates, currentPos, this->currentStage), result compared
   with `bltz` (round 2026-09-02). MATCHED, defined later in this unit's own
   ROM order -- forward declaration only (gp-relative blocker resolved; see
   docs/match-reports/Test4InstantTeleporters.md). */
extern s32 Test4InstantTeleporters(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage);

/* Called by DreamSys__TryInstantTeleportLink with NO arguments, same shape as GetStageLinkAngle
   above; return value is forwarded straight into ExecuteLink's stage-type
   argument, hence s32 (round 2026-09-02). MATCHED, defined later in this
   unit's own ROM order -- forward declaration only (gp-relative blocker
   resolved; see docs/match-reports/func_8005BFC4.md). */
extern s32 func_8005BFC4(void);

/* Table triple for Test4TunnelLinks (round 2026-08-30-d), same roles as the
   STAGE_PERMALINK_* triple above but for tunnel links specifically. */
extern s8 LEN_TUNNEL_TRIGGERS[];
extern StaticLinkTrigger* TUNNEL_TRIGGERS[];
extern StageSpawn* TUNNEL_SPAWNS[];

/* Table triple for Test4StaircaseNodes (round 2026-08-30-d). */
extern s8 LEN_STAIRCASE_TRIGGERS[];
extern StaticLinkTrigger* STAIRCASE_TRIGGERS[];
extern StageSpawn* STAIRCASE_SPAWNS[];

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