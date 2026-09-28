# SubVec3S16 -- MATCHED (16/16 words)

> Renamed from `func_8001EA8C` on 2026-09-17 (tools/rename.py). Address 0x8001ea8c.

Unit: `SceneNode` (round 14). A standalone leaf, not yet reached by any
caller in this round's queue -- three-element vector subtraction between
two `s16` arrays, widening the result into an `s32` array.
`void SubVec3S16(s32 *dest, s16 *b, s16 *a)`.

## Final source

```c
void SubVec3S16(s32 *dest, s16 *b, s16 *a) {
    dest[0] = a[0] - b[0];
    dest[1] = a[1] - b[1];
    dest[2] = a[2] - b[2];
}
```

## Derivation notes

No struct type fits either input array -- both are read as bare 3-element
`s16` vectors (`lh` at offsets 0/2/4), and the result is written as a bare
3-element `s32` vector (`sw` at offsets 0/4/8). Declared with raw pointer
parameters rather than inventing a named type, since nothing here
constrains it further; a future caller reaching this function may narrow
the parameter types if it passes a named struct's address.

First-try match, no residue. Parameter naming follows the register order
(`a0`=`dest`, `a1`=`b`, subtracted; `a2`=`a`, subtracted from) rather than
guessing semantic names, since nothing here indicates what `a`/`b`
represent (possibly a target-minus-current delta for the same kind of
16-bit angle triple `SceneNodeSub44::unk10/12/14` holds, given the
matching element count and halfword source width, but that is a guess,
not evidence -- left unstated).

No new struct or vtable-slot knowledge.

## Naming (round 50, charlie -- FINISHING-PLAN track 3)

- **`func_8001EA8C` -> `SubVec3S16`. Tier B.** Free function (no receiver),
  so `VerbNoun`. The operation is complete and visible -- three-component
  subtraction of `s16` inputs widened into an `s32` output -- but WHAT the
  two vectors are is not established, which is what keeps it at B: its only
  known caller is `SceneNode__RaycastVertical`, still `INCLUDE_ASM`, where they are the
  two outputs of a `TmdModel__RaycastFaces` call that is itself unidentified Psy-Q.
- **Parameters renamed `(dest, b, a)` -> `(dest, from, to)`** and the body
  rewritten to `dest[i] = to[i] - from[i]`, same expression, so that the
  operand order is readable at the call site: the subtrahend is the 2nd
  argument and the minuend the 3rd. Byte-identical.

## Round 98 (echo): track 7, moved from src/SceneNode.c

The old comment's `buf18` and `buf28` are RaycastVertical's `origin` and `hit`, so SubVec3S16 there computes hit - origin, both in the node's own frame.

The source comment was rewritten as documentation; the one it replaced, verbatim (field names as they were then):

```c
/* dest = to - from, over three components, widening s16 inputs to s32.
 * Parameter ORDER is the subtrahend first: the value subtracted is the 2nd
 * argument, the value subtracted FROM is the 3rd. What the two vectors
 * represent is not established -- SceneNode__RaycastVertical is the only known caller,
 * and it passes `buf18` (slotA4's rotated delta) as `from` and `buf28`
 * (TmdModel__RaycastFaces's own output) as `to`. */
```
