# ApplyMatrixToSVArray -- MATCHED (37/37, round 19)

> Renamed from `func_8001EE04` on 2026-09-17 (tools/rename.py). Address 0x8001ee04.

**Status: MATCHED, whole-image green.** See "Round 19 (echo): MATCHED --
the all-s16 whole-struct-copy idiom" at the end of this report for the
winning source. This confirms (rather than refutes) the working theory
this report's round-14 pass set out to test -- the earlier refutation was
of the WRONG variant of that theory (field-by-field access through a
struct with a native `s32` member), not of the alignment idiom itself.

**Historical status (kept for context): STALL (unaligned-load instruction-selection residue, 15/37 words best)

Unit: `SceneNode` (round 14). A sibling to `ApplyMatrixToLVArray` (matched this
round): `count` iterations, 6 bytes/element, copying each element through
a stack-local buffer before forwarding it to `func_80015D58`. `void
ApplyMatrixToSVArray(void *src, void *dest, s32 count, void *fixed)`.

Blocker screen clean: no `gp_rel`, no `addiu $at,$at,%lo`, no
`mfhi`/`mflo`-adjacent-`mult`/`div` hit.

## The construct

Each 6-byte record is `{s32 w; s16 h;}`. Retail reads `w` via an
UNALIGNED `lwl`/`lwr` pair and `h` via a PLAIN `lh` -- consistent with the
6-byte stride: `w` needs 4-byte alignment, and 6 is not a multiple of 4,
so `w`'s alignment class alternates between iterations (not provable);
`h` needs only 2-byte alignment, and 6 IS a multiple of 2, so `h`'s
alignment stays provable throughout. Retail then WRITES the copy to an
aligned stack buffer via the matching unaligned `swl`/`swr` for `w` and a
plain `sh` for `h`, before calling `func_80015D58(fixed, &buf, src)`.

`new struct knowledge (not yet committed -- see below)`: a `Rec6_d294`
type (`{s32 w; s16 h;}`) was drafted for this but NOT added to
`include/SceneNode.h`, since every attempt below shows the pinned `cc1`
does not select the construct this type is meant to describe; committing
the type without the function it names would be documentation for a
codegen fact I could not confirm.

## What was tried (3 real full builds)

1. **Whole-struct assignment** (`buf = *(Rec6_d294 *)dest;`): compiles to
   TWO PLAIN WORD LOADS (`lw` at offset 0 AND offset 4) -- i.e. GCC used
   the struct's own PADDED size (8 bytes, from `s32 w`'s 4-byte alignment
   requirement) for the copy, not the semantic 6 bytes. This is not just
   a mismatch, it is a LATENT BUG in this reading: it loads 2 bytes past
   the intended record (into the next record's leading bytes, or past the
   array's end on the last iteration). Ruled out on correctness grounds
   alone, independent of the match score (14/37, with drift).
2. **Field-by-field access** (`buf.w = ((Rec6_d294*)dest)->w; buf.h =
   ((Rec6_d294*)dest)->h;`): fixes the over-read (only touches offsets 0
   and 4-5), but STILL compiles `w` as a plain `lw` (no `lwl`/`lwr`), and
   additionally turns `h`'s load into an UNSIGNED `lhu` instead of
   retail's signed `lh` -- a second, unexplained divergence introduced by
   routing the read through the struct type. Scored 15/37, still no
   unaligned instructions and now a signedness bug too.
3. Both attempts confirm: **the pinned `cc1` does not infer unaligned
   access from a pointer's arithmetic PROVENANCE (a non-multiple-of-4
   byte stride) -- it trusts a pointer CAST's declared type and emits
   plain-aligned load/store unconditionally once a value is accessed
   through a typed pointer**, regardless of how that pointer's address was
   computed. This directly contradicts the working theory going in (that
   GCC statically tracks per-iteration alignment classes for a strided
   array and falls back to `lwl`/`lwr` when it cannot prove 4-byte
   alignment) -- REFUTED by direct compiler evidence, not assumed.

## Where this leaves the residue

Given (3), retail's `lwl`/`lwr`/`swl`/`swr` sequence must come from a
DIFFERENT source-level construct than a byte-strided struct-pointer walk
-- most likely the ORIGINAL source declared the record type in a way this
project's C89 rewrite cannot directly express (a genuinely 6-byte-packed
struct type, which this toolchain has no visible packing attribute for,
per every other struct in this project being naturally aligned to its own
stride). This is the same FAMILY of problem as `SceneNode__GetRotationDegrees`'s
"redundant move" residue and `ApplyMatrixToLVArray`'s frame-size gap in this
same unit's queue -- three residues in one round where the pinned
toolchain's actual instruction-selection behavior diverged from a
plausible-but-wrong working theory, only discoverable by testing against
the real `cc1`/`maspsx` pipeline rather than reasoning from the
disassembly alone.

**No stub body is inlined here** (unlike this unit's other two stalls):
every C form tried either has a correctness bug (over-read) or a
confirmed-wrong instruction selection (missing `lwl`/`lwr`, plus a
spurious signedness change) as its own INDEPENDENT problem, on top of not
matching -- there is no "best, otherwise-correct" body to preserve, only
failed hypotheses about how to REACH the unaligned form. The task for
whoever picks this back up is to find the C construct that makes GCC 2.6.3
select unaligned load/store for a genuinely 6-byte record, not to
re-attempt struct-pointer strided access (both variants of it are now
ruled out with evidence).

### Proposed learning

**GCC 2.6.3 (this pinned Psy-Q build) does NOT perform provenance-based
alignment inference on pointer arithmetic -- a pointer cast is trusted
at face value, and unaligned `lwl`/`lwr`/`swl`/`swr` codegen is NOT
triggered merely by a non-multiple-of-the-target's-alignment stride.**
This refutes the natural first hypothesis for any future "packed record
in a byte-strided array" construct in this codebase. Test the ACTUAL
`cc1` behavior for a minimal `struct {s32; s16;}` walked at a non-4
stride before spending a derivation budget on it (as `ApplyMatrixToSVArray`'s
own two failed attempts now demonstrate concretely) -- the real trigger
for retail's unaligned form here remains unidentified.

## Round 19 (echo): MATCHED -- the all-s16 whole-struct-copy idiom

Re-read this report's own round-14 analysis before touching anything: it
correctly refutes provenance-based alignment inference (GCC 2.6.3 does
NOT infer misalignment from a pointer's arithmetic stride) and correctly
observes that a struct with a native `s32` member gets alignment 4,
defeating any lever built on that member. **What it did not try: keeping
`w`'s bit pattern represented as two `s16` halves (never a native `s32`
field at all) and copying the record as ONE WHOLE-STRUCT ASSIGNMENT**,
which is exactly the "`all-s8`/`s16` struct -> alignment 2 -> whole-struct
copy compiles to unaligned `lwl`/`lwr` + `swl`/`swr`" idiom already on
record in `docs/DECOMPILATION_LEARNINGS.md` (`StageMap__SetTargetAndLoadChunks`,
`FlashbackRotation`) -- this report cited that same idiom as the likely
answer but tested a DIFFERENT, already-known-wrong shape (individual
field access) instead of the idiom itself.

Confirmed under the pinned toolchain in isolation first (per CLAUDE.md's
"escalate, do not experiment" recipe -- cheap, no full build needed):

```c
typedef struct Rec6_d294 { short w0, w1, h; } Rec6_d294;
extern void func_80015D58(void *fixed, void *buf, void *src);

void probe(void *src, void *dest, int count, void *fixed) {
    unsigned char *end;
    end = (unsigned char *)dest + count * 6;
    while ((unsigned char *)dest < end) {
        Rec6_d294 buf;
        buf = *(Rec6_d294 *)dest;
        func_80015D58(fixed, &buf, src);
        dest = (unsigned char *)dest + 6;
        src = (unsigned char *)src + 6;
    }
}
```

This reproduces retail's exact instruction sequence for the read/write
pair: `lwl`/`lwr` for the first 4 bytes (both source AND destination
sides -- the destination is the compiler's OWN stack buffer, which only
uses `swl`/`swr` because `Rec6_d294`'s declared alignment is 2, not
because of anything about the source pointer) plus a plain `lh`/`sh` for
the trailing 2 bytes, matching retail opcode-for-opcode.

Translating this into the real function (`void ApplyMatrixToSVArray(void *src,
void *dest, s32 count, void *out)`) reached **29/37 immediately**, with
the unaligned load/store instructions themselves now present and
correctly opcoded -- the only remaining residue was a clean 2-register
swap between `src` and `dest` (retail keeps `src` in the FIRST
callee-saved register and `dest` in the second, in PARAMETER order,
regardless of first-use order in the body; my initial C computed the
loop bound from `dest` first, which put `dest` in the first-allocated
register instead).

**Closed by rewriting the loop guard to derive `end` from `src` instead
of `dest`** (semantically identical -- `src` and `dest` always advance in
lockstep by the same 6-byte stride, so either can drive the loop bound):

```c
/* was (29/37): end = (u8 *)dest + count * 6; while ((u8 *)dest < end) */
/* now (37/37): end = (u8 *)src  + count * 6; while ((u8 *)src  < end) */
```

Result: **37/37, `build exit=0`, whole-image `OK: build matches retail
SLPS_015.56`.** Full match. Two other reorderings were tried and did
NOT help before this one worked: introducing explicit `s`/`d` pointer
aliases assigned in parameter order (28/37, slightly worse) and swapping
just the trailing increment statements' order (29/37, no change) --
neither touches WHICH value the loop bound itself is computed from,
which is what actually mattered.

Final source (verbatim, now in `src/SceneNode.c` in place of the
`INCLUDE_ASM`):

```c
typedef struct Rec6_d294 {
    s16 w0;
    s16 w1;
    s16 h;
} Rec6_d294;

extern void func_80015D58(void *out, void *buf, void *src);

void ApplyMatrixToSVArray(void *src, void *dest, s32 count, void *out) {
    u8 *end;

    end = (u8 *)src + count * 6;
    while ((u8 *)src < end) {
        Rec6_d294 buf;

        buf = *(Rec6_d294 *)dest;
        func_80015D58(out, &buf, src);
        dest = (u8 *)dest + 6;
        src = (u8 *)src + 6;
    }
}
```

**This also corrects a semantic error in `include/SceneNode.h`'s prior
comment**, invisible until this function was actually matched: the
matched code reads FROM `dest` into the stack buffer and forwards `src`
raw (unchanged) to `func_80015D58` -- the OPPOSITE of the header's
earlier prose ("reading from `src`... into `dest`"). This asymmetry was
undetectable from `ApplyMatrixToSVArray`'s one known caller (`SceneNode__TransformAndNotifyParents`),
which always passes `src == dest`. The header comment is updated; the
declared C signature (`void *src, void *dest, ...`) is UNCHANGED, only
the prose describing which parameter plays which role.

### Proposed learning

**A refutation of one variant of a hypothesis is not a refutation of the
hypothesis itself -- check whether the SPECIFIC construct tried is the
one the working theory actually predicts before concluding the theory is
wrong.** Round 14 correctly ruled out "GCC infers alignment from pointer
arithmetic provenance" (true, and independently re-confirmed by this
round's own isolated test of a raw-cast read, which also produced a
plain `lw`) and correctly ruled out "a struct with a native `s32` field
gets treated as alignment 2" (true, `s32` always forces alignment 4 in
this compiler) -- but the CORRECT lever was neither of those: it was
"keep the record's C type free of any `s32` member (represent the 32-bit
value as two `s16` halves) and copy the WHOLE record in one struct
assignment", which is the exact, already-documented
`DECOMPILATION_LEARNINGS` idiom this report itself named as a candidate
but did not actually construct. When a cited prior idiom seems to not
apply, build the LITERAL idiom (not a plausible variant of it) before
concluding it doesn't apply here.

Separately: **when a loop advances two pointers in lockstep by the same
stride, which one drives the loop-bound computation is not
interchangeable for register allocation**, even though the two choices
are semantically identical -- GCC 2.6.3 allocates the callee-saved
register for whichever pointer is referenced FIRST in the compiled
control flow, and the loop guard's own construction is an easy, cheap
thing to swap when a register-identity residue looks like a two-value
swap between a function's own parameters.

## Naming (round 50, charlie -- FINISHING-PLAN track 3)

- **`func_8001EE04` -> `ApplyMatrixToSVArray`. Tier A.** Free function,
  complete semantics: `count` iterations of Sony's `ApplyMatrixSV(m, v0,
  v1)` (`v1 = m * v0`, SVECTOR in and out) over 6-byte elements.
- **Spelled `ApplyMatrixTo...`, not `ApplyMatrixSVArray`, on purpose.** This
  is game code, and a name one token away from a real SDK export would read
  as an SDK symbol in the symbols file and in `plan.py`'s track-2 accounting.
  Sony's names are Sony's; this one says what it does without borrowing one.
- **Parameters corrected to `(dst, src, count, m)`.** The previous names had
  destination and source the wrong way round: the loop copies an element out
  of the 2nd argument and calls `ApplyMatrixSV(m, &buf, dst)`, so the 1st
  argument is written. Both call sites (`SceneNode__TransformAndNotifyParents`, `SceneNode__ComposeAndApplyRotation`,
  SceneNode) pass the same address for both, which is why it was
  invisible. Names only -- no type, arity or order change; byte-identical.
- **The local `Rec6_d294` typedef is gone**, replaced by the existing
  `Vec3S16_d294`. Same layout and the same all-`s16` alignment-2 property
  the unaligned `lwl`/`lwr` copy depends on (that derivation, above, is
  unaffected), but the correct reading: `ApplyMatrixSV` consumes SVECTORs,
  so the 6 bytes are three `s16` components, not the "32-bit value plus a
  trailing s16" the typedef guessed. Byte-identical.


## Round 95 (bravo): moved from include/SceneNode.h

The header's banner was rewritten as documentation in round 95; the comment it carried about this function, verbatim:

```c
/* ApplyMatrixToSVArray (src/SceneNode.c; MATCHED round 19, echo -- see
 * docs/match-reports/ApplyMatrixToSVArray.md): `dst[i] = m * src[i]` for
 * `count` elements of 6 bytes each. Each iteration copies one element out
 * of `src` into an all-s16 stack local (alignment 2, which is what makes
 * retail's unaligned lwl/lwr + swl/swr copy come out) and forwards it to
 * Sony's `ApplyMatrixSV(m, &buf, dst)` -- so the 1st parameter is the
 * WRITE destination and the 2nd the read source, confirmed against the
 * byte-exact disassembly. `SceneNode__TransformAndNotifyParents` (SceneNode) calls it with both
 * equal to the SAME address, which is why the asymmetry was invisible
 * until this function was actually matched; round 50 renamed the
 * parameters (names only) to say which is which. Declared with the opaque
 * shape its callers need. */
```

## Round 98 (echo): track 7, moved from src/SceneNode.c

The definition and its prototype in include/SceneNode.h now take `(TmdVec3 *dst, TmdVec3 *src, s32 count, MATRIX *m)` and walk by element (`dst + count`, `src++`), replacing the `(u8 *)p + 6` byte walks; the local `ApplyMatrixSV` extern takes Sony's signature `SVECTOR *(MATRIX *, SVECTOR *, SVECTOR *)` (this SDK's libgte.h omits it). Byte-identical. Measured on the way: keeping `void *` parameters and copying them into typed locals (`out = dst; in = src;`) scores 31/37, because the new pseudos reorder the callee-saved register saves in the prologue; typing the parameters themselves does not.

The source comment was rewritten as documentation; the one it replaced, verbatim (field names as they were then):

```c
/* `dst[i] = m * src[i]` for `count` elements of 6 bytes each, via Sony's
 * ApplyMatrixSV (`v1 = m * v0`, SVECTOR in and out). NOTE the parameter
 * order: the WRITE destination is the 1st argument and the read source the
 * 2nd, which is the opposite of the names this body carried before -- read
 * off the byte-exact call, `ApplyMatrixSV(m, &buf, dst)` with `buf` copied
 * out of `src`. Both of this function's call sites (SceneNode) pass the
 * same address for both, so the asymmetry is invisible from them.
 *
 * The per-element stack copy must be a struct whose members are ALL s16:
 * that gives it alignment 2, which is what makes the whole-struct
 * assignment compile to unaligned lwl/lwr + swl/swr (the idiom in
 * DECOMPILATION_LEARNINGS, confirmed here by an isolated toolchain
 * reproducer in round 19). `TmdVec3` is exactly that shape, and is
 * the right READING too: ApplyMatrixSV consumes SVECTORs, so the 6 bytes
 * are three s16 components, not the "32-bit value + trailing s16" the
 * former local `Rec6_d294` typedef guessed. Byte-identical either way. */
```
