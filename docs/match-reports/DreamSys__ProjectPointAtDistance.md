# DreamSys__ProjectPointAtDistance — MATCH (56/56 words, whole-image SHA1 confirmed)

> Renamed from `func_8005942C` on 2026-09-22 (tools/rename.py). Address 0x8005942c.

**Unit:** DreamSys · **Size:** 56 instructions · **Status:** byte-exact.

Closed round 19 (second pass, runner delta) from a mid-attempt snapshot that
did not even compile. History below in arrival order; read this section
first.

## Final body (landed in `src/DreamSys.c`)

```c
extern void SceneNode__LocalOffsetToWorldPos(void *self, s32 *dst, s32 *src, s32 arg4);
extern s32 InterpolateKeyframeValue(void *a, void *b, s32 day);
extern s32 IsVec3WithinRange(s32 *a, s32 range, s32 *b);

s32 DreamSys__ProjectPointAtDistance(DreamSys *this, s32 *out, s32 day, s32 *reference, s32 tolerance)
{
	s32 local[3];
	s32 ret;
	s32 *vec;
	s32 *p;

	p = &gProjectOffsetZ;
	*p = day;
	SceneNode__LocalOffsetToWorldPos(this, local, p - 2, 0);

	ret = InterpolateKeyframeValue((void *)((u8 *)this->unk_0x5C + 0x14),
	                     (void *)((u8 *)this->unk_0x5C + 0x20), day);

	vec = this->unk_0xC != 0 ? (s32 *)((u8 *)this->unk_0x14 + 0x38) : 0;
	local[1] = ret + vec[1];

	if (out != NULL) {
		*(DreamSysVec3 *)out = *(DreamSysVec3 *)local;
	}

	if (reference != NULL)
		return IsVec3WithinRange(local, tolerance, reference);
	return 0;
}
```

Real signature: `(DreamSys *this, s32 *out, s32 day, s32 *reference, s32
tolerance)` -- 5 parameters, the 5th (`tolerance`) passed on the stack
(`lw a1, 0x48(sp)` reads it from the caller's incoming-argument area,
confirmed against the O32 ABI's arithmetic: frame is `-0x38`, +0x10 for the
reserved a0-a3 spill slots = 0x48). The project's own vtable slot
(`vtable_DreamSys::DreamSys__ProjectPointAtDistance`) was declared with only 4 params before
this round; corrected in `include/DreamSys.h` (function-pointer prototype
only, no struct-size change -- verified safe since `gDreamSysMethods`
itself is not yet a C data definition anywhere, so nothing could break from
the signature edit).

## New global named: the unnamed 3-word vector at 0x80087EE0

`gProjectOffsetZ` is the LAST word of an unnamed 3-word (`DreamSysVec3`-shaped)
global scratch vector. The other two words were NOT independently named --
splat's dlabel boundary put them inside `VOICE_PITCH_BY_SELECT`'s dlabel as unlabeled
tail bytes (`asm/data/783DC.data.s`), because nothing took their address
directly until this function. Declared in `include/DreamSys.h` as
`extern s32 gProjectOffsetZ;` (unchanged type -- it really is a lone word by
itself); the vector's start is reached with pointer arithmetic off it,
`(s32 *)&gProjectOffsetZ - 2`, since renaming/resegmenting `VOICE_PITCH_BY_SELECT`'s dlabel
is a `config/` change and out of scope this round.

**This is not a guess -- two independent pieces of evidence pin it down:**

1. `DreamSys__NotifyLinkAttempt` (this unit, already matched) clamps `this->unk_0xB8 =
   (this->unk_0x28->unk_0x36 & 0x7F); if (unk_0xB8 >= 0x18) unk_0xB8 = 0;`
   -- i.e. `unk_0xB8` is bounded to `[0, 0x18)`. Both `VOICE_BY_SELECT` and
   `VOICE_PITCH_BY_SELECT` (each already-named 24+-byte byte tables) are indexed by
   this SAME bounded value in `DreamSys__StartVoice` (`VOICE_PITCH_BY_SELECT[unk_0xB8]`), so
   `VOICE_PITCH_BY_SELECT`'s real, ever-read extent is exactly 24 bytes
   (`0x80087EC8`-`0x80087EDF`) -- the 8 trailing zero bytes splat lumped
   into its dlabel (`0x80087EE0`-`0x80087EE7`) are never reached by that
   indexed access and belong to something else.
2. `SceneNode__LocalOffsetToWorldPos` (SceneNode, already matched) forwards its own `src`
   parameter to `ApplyMatrixToLVArray(dst, src, 1, buf)`, and `ApplyMatrixToLVArray`'s
   own doc comment (`include/SceneNode.h`) confirms it treats both pointers
   as 0xC-byte (3-word) elements. `DreamSys__ProjectPointAtDistance` passes
   `(s32 *)&gProjectOffsetZ - 2` as that exact `src` argument, which only
   type-checks sensibly as a 3-word vector's start -- matching the 8
   "spare" bytes above exactly (2 words = 8 bytes immediately before
   `gProjectOffsetZ`, which is the vector's 3rd word).

## Two real bugs found and fixed in the inherited sketch

The mid-attempt snapshot's CONTROL FLOW was already correct (verified
instruction-by-instruction against the raw disassembly before touching
anything) -- both defects were in HOW two already-identified pieces of
logic were expressed in C, not in what the function does.

**Bug 1: the shared-pointer computation was split into two separate
expressions, so GCC computed `&gProjectOffsetZ`'s address TWICE instead of
once.** The snapshot wrote `gProjectOffsetZ = day;` and, later, `&gProjectOffsetZ -
2` as a fresh address-of expression at the call site. Retail computes the
address ONCE (into a single register) and reuses it for both the plain
store and the pointer-arithmetic argument -- GCC 2.6.3 does not CSE this
across two syntactically distinct `&gProjectOffsetZ` occurrences. Fix: bind it
to a named pointer once, use it for both:
```c
p = &gProjectOffsetZ;
*p = day;
SceneNode__LocalOffsetToWorldPos(this, local, p - 2, 0);
```
This alone was worth several words and, more importantly, is what makes
`gProjectOffsetZ`'s own later uses read as a genuine defect rather than random
noise -- a nice instance of the round's own standing theme (an unverified
intermediate claim reads as something else entirely once actually
recompiled).

**Bug 2: a conditional `+=` was used where retail evaluates the ternary and
the addition UNCONDITIONALLY.** The snapshot wrote:
```c
if (this->unk_0xC)
    v0 += *(s32 *)((u8 *)this->unk_0x14 + 0x3C);
```
Retail's actual codegen computes `vec = (this->unk_0xC ? this->unk_0x14 +
0x38 : NULL)` and then does `v0 = ret + vec[1]` on EVERY path, including a
literal null-pointer-plus-4 dereference when `unk_0xC == 0` -- the ADD
itself is never skipped, only the POINTER varies. This exact idiom (a
ternary yielding a null pointer, then indexed unconditionally) is not
invented for this function: it is already proven byte-exact in
`SceneNode__LocalOffsetToWorldPos`'s own matched body (`SceneNode.c`): `table = self->unkC
!= 0 ? self->unk14->unk38 : 0; dst[0] = dst[0] + table[0];`. Recognising
the SAME pattern here (rather than writing the more "obviously safe"
guarded form) is what closed this bug:
```c
vec = this->unk_0xC != 0 ? (s32 *)((u8 *)this->unk_0x14 + 0x38) : 0;
local[1] = ret + vec[1];
```

**Bug 3 (found last, the one that actually closed the function): a
3-word struct copy written as three indexed assignments compiles to 9
instructions (load/nop/store per field, same register reused), not
retail's 6 (three loads into three different registers, then three
stores).** This is the project's own documented "whole-struct assignment,
not an indexed loop, for a block copy" idiom
(`docs/DECOMPILATION_LEARNINGS.md`), applied to an OUT-PARAMETER copy
rather than a loop body -- a new instance of an already-known lever, not a
new lever. `out` is `s32 *` and `local` is `s32[3]`; both are exactly the
shape of the already-existing `DreamSysVec3` struct, so:
```c
*(DreamSysVec3 *)out = *(DreamSysVec3 *)local;
```
closed the remaining 3-word overshoot (`objdump` word count 59 -> 56,
matching retail exactly) and, since the whole-image drift this caused had
been shifting even the LATER, unrelated `gProjectOffsetZ` global's own resolved
address (data placed after this code in the single contiguous PS-X image)
and `InterpolateKeyframeValue`'s call target, fixing this one bug cleared every other
remaining diff in the same build.

## Method note: the "20/56 in-range, 140723 bytes outside" number was read correctly, and it was still not diagnostic of the eventual fix

The prior round's outside-range figure correctly signalled "this is not a
near-miss, do not treat it as one" -- and it was right to signal that. But
it did not, and could not, point at which of the three bugs above mattered
most: all three were found by re-deriving the control flow from the raw
disassembly line by line and cross-checking each already-proven idiom
against a SIBLING function's own matched body (`SceneNode__LocalOffsetToWorldPos`), not by
reading the drift number harder. The number's job is "stop treating this
as close"; it is not a diagnostic for what to fix. That distinction is
worth keeping separate from this round's other finding (that an unverified
"clean"/"pure reordering" claim can itself be wrong) -- this report is an
example of the SAME underlying discipline (verify by rebuilding, don't
trust the inherited state) closing a function outright rather than merely
correcting a stall's classification.

### Proposed learning

**A "materialize a value once, use it twice" bug is a real, distinct
subclass of the register-identity/drift family, and it is invisible until
the function is actually rebuilt.** Two occurrences of the same
address-of expression, written as two separate statements, do not
automatically share one computed address under GCC 2.6.3 -- binding it to
a named pointer once and reusing that pointer is the fix, and it is cheap
to try whenever a global (or any addressable lvalue) is both written and
then used again via pointer arithmetic in the same function.

## History (pre-fix, preserved for context; superseded above)

### Status, precisely (as filed)

This was **not a considered plateau**. No author ever applied a stop rule
to it. runner/echo was in the middle of this function when an
account-wide session limit killed the process; the head recovered the
text per docs/PARALLEL-RUNS.md 4c. It carried none of the "this is as far
as reshaping got" implication a normal preserved body does.

**As echo left it, it did not compile**: `gProjectOffsetZ` was undeclared.
echo's last recorded thought was that it compiled fine and it was about
to read a funcdiff score -- it would have got a false number off the
previous build.

With the three missing declarations added by the head purely to obtain a
measurement, it reached **19/56 words in range, and 140723 bytes differ
OUTSIDE the range**.

### Preserved body (mid-attempt, did not compile as-is)

```c
#if 0
extern s32 gProjectOffsetZ;
extern s32 SceneNode__LocalOffsetToWorldPos();
extern s32 InterpolateKeyframeValue();
extern s32 IsVec3WithinRange();

s32 DreamSys__ProjectPointAtDistance(DreamSys *this, s32 *out, s32 day, s32 *reference, s32 tolerance)
{
	s32 local[3];
	s32 v0;

	gProjectOffsetZ = day;
	SceneNode__LocalOffsetToWorldPos(this, local, &gProjectOffsetZ - 2, 0);

	v0 = InterpolateKeyframeValue((void *)((u8 *)this->unk_0x5C + 0x14),
	                    (void *)((u8 *)this->unk_0x5C + 0x20), day);

	if (this->unk_0xC)
		v0 += *(s32 *)((u8 *)this->unk_0x14 + 0x3C);

	local[1] = v0;

	if (out != NULL) {
		out[0] = local[0];
		out[1] = local[1];
		out[2] = local[2];
	}

	if (reference != NULL)
		return IsVec3WithinRange(local, tolerance, reference);
	return 0;
}
#endif
```

This body's CONTROL FLOW turned out to be entirely correct -- every branch,
call, and field offset matched the raw disassembly on re-derivation. Its
three defects (documented above) were all about EXPRESSION SHAPE, not
logic.

## Provenance

Round 17, runner echo (killed mid-attempt, recovered by the head per
PARALLEL-RUNS 4c). Round 19 second pass, runner delta: named the global,
fixed three expression-shape bugs, MATCHED 56/56.

## Naming

`DreamSys__ProjectPointAtDistance` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_8005942C`.

Writes `dist` into the z word of the three-word global scratch
offset vector, converts that LOCAL offset to a world position through
`SceneNode__LocalOffsetToWorldPos` (SceneNode, matched), replaces the result's Y
with `InterpolateKeyframeValue(heightCurve+0x14, heightCurve+0x20, dist)` plus the
object's own world-base Y, optionally copies the point out, and optionally returns
whether it lies within `tolerance` of `reference`.
The same evidence corrects the third parameter's name: it was `day`, an artefact of
m2c having no caller to type against. It is the abscissa of a curve whose keyframes
carry `position` fields, and the z component of a local offset -- a distance.
Tier B: the computation is certain, what the projected point is FOR is not (it has
no carved caller; the vtable slot is +0x120).

## Comment moved from src/DreamSys.c (round 92, track 7)

Replaced in the source by a comment that says what the code does; kept here as written.

```c
/* Local prototypes, own local view (SceneNode__LocalOffsetToWorldPos is a different unit's
 * already-matched function taking an unrelated class as arg0; InterpolateKeyframeValue
 * is this unit's own next-in-queue function, forward-declared per
 * CLAUDE.md's convention for calling into a not-yet-preceding definition).
 * SceneNode__LocalOffsetToWorldPos's unused 4th parameter IS set (to 0) by
 * this call site's own disassembly; include/SceneNode.h declares it. */
```

```c
/* `dist` was called `day` until round 66, which was a transcription of the
   caller-less m2c signature and is wrong: it is written into the z word of
   the global scratch vector that SceneNode__LocalOffsetToWorldPos converts
   from a LOCAL OFFSET to a world position, and it is also the abscissa
   InterpolateKeyframeValue evaluates the viewport's two refView points at -- whose
   own `position` fields are what it is compared against. It is a distance
   along the local axis, not a day index. */
```

## Round 97 (alpha): Sony's GsCOORDINATE2

SceneNodeSub14 is deleted: SceneNode.coord2 is Sony's GsCOORDINATE2 (flg; MATRIX coord, whose t is the offset from the parent; MATRIX workm, whose t is the world position; param, super, sub -- 0x50 bytes, offset for offset). Accessors here follow the compiler's list: tx/ty/tz -> coord.t[0]/[1]/[2], unk38 -> workm.t; a local that holds coord.t or workm.t is `long *` (MATRIX.t is long[3]; s32 is int); any cast to GsCOORDINATE2 * is gone. Byte-identical.
