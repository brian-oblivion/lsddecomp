# SceneNode__LocalOffsetToWorldPos -- MATCHED (62/62 words)

> Renamed from `Class6B5CC__LocalOffsetToWorldPos` on 2026-09-26 (tools/rename.py). Address 0x8001e600.

> Renamed from `func_8001E600` on 2026-09-17 (tools/rename.py). Address 0x8001e600.

Unit: `SceneNode` (round 14). Fills a 0x20-byte stack buffer via
`slot84` (same shape as `SceneNode__RotateLocalVector`), forwards it into
`ApplyMatrixToLVArray` (this unit, still `INCLUDE_ASM` -- calling it is fine,
its retail bytes still link and run correctly), then adds a
conditionally-NULL 3-word table into `dst` one axis at a time.
`void SceneNode__LocalOffsetToWorldPos(SceneNodeObj *self, s32 *dst, s32 *src)`.

## Final source

```c
void SceneNode__LocalOffsetToWorldPos(SceneNodeObj *self, s32 *dst, s32 *src) {
    u8 buf[0x20];
    s32 *table;

    self->methods->slot84(self, buf, 0);
    ApplyMatrixToLVArray(dst, src, 1, buf);

    table = self->unkC != 0 ? self->unk14->unk38 : 0;
    dst[0] = dst[0] + table[0];

    table = self->unkC != 0 ? self->unk14->unk38 : 0;
    dst[1] = dst[1] + table[1];

    table = self->unkC != 0 ? self->unk14->unk38 : 0;
    dst[2] = dst[2] + table[2];
}
```

## New struct knowledge (`include/SceneNode.h`, additive)

`SceneNodeSub14` gains `unk38` (`s32[3]`), split out of the previously
opaque `unk24[0x044-0x024]` byte span (now `unk24[0x038-0x024]` +
`unk38[3]`, ending exactly at `+0x044` where `unk44` already began -- no
size change). No existing field was retyped; `self->unkC`
(`UnkOwner_d294 *`, already typed) and `self->unk14`
(`SceneNodeSub14 *`, already typed) are both used here exactly as
already declared -- this function's `self->unkC != 0`/`self->unk14->...`
accesses are plain uses of existing fields, not new ones.

## Derivation notes

- The `self->unkC != 0 ? self->unk14->unk38 : 0` ternary, computing
  `table` fresh before EACH of the three additions (not once, reused
  three times), matches retail's disassembly exactly: the `self->unkC`
  NULL check and the `self->unk14->unk38` address computation are
  genuinely redone three separate times in the retail bytes, not shared
  across a loop -- the same "manually unrolled 3x repetition" shape as
  `SceneNode__GetRotationDegrees` earlier in this unit's queue.
- **When `self->unkC == 0`, `table` is a literal NULL, and `table[i]` is
  still unconditionally dereferenced** -- reproducing retail's own
  apparent behavior exactly rather than guessing a safer reading. Nothing
  in this call site suggests retail treats this as reachable in practice
  (a defensive branch that's never actually hit with `unkC == 0`, or the
  low address happens to be validly mapped and zero-filled on this
  target); matching the bytes does not require resolving that question.
- `slot84`'s output buffer is 0x20 bytes here too (same measurement
  technique as `SceneNode__RotateLocalVector`'s report: sized from the caller's own
  frame, not from what's read back), confirming that finding generalizes
  across both call sites rather than being a one-off.
- First-try match once the struct field was added; no residue.

## Naming (round 50, charlie -- FINISHING-PLAN track 3)

- **`func_8001E600` -> `SceneNode__LocalOffsetToWorldPos`. Tier B.** Two
  measured halves: (1) it rotates `src` by the object's own orientation
  (`slot84` with the flag-0, un-negated branch of `SceneNode__GetRotMatrix`), and
  (2) it adds `self->unk14->unk38`, which is `GsCOORDINATE2.workm.t` -- the
  COMPOSED world matrix's translation (`workm` at +0x24, `t` at +0x14 into
  MATRIX, = +0x38; see the PSY-Q IDENTIFICATION note in
  include/SceneNode.h). `SceneNode__RaycastVertical` is the function that maintains that
  field, by summing `coord.t` down the owner chain.
- **Why B, not A:** "WorldPos" rests on the `workm.t` identification, which
  is solid; "Local" rests on the rotation being the object's own only, which
  is exactly true for this object but does not compose an ancestor's
  rotation. A hierarchy with a rotated parent would make the output not
  literally world-space.
- **Corroborated by both callers**, which is what makes the direction
  (offset in, position out) more than a reading: `DreamSys.c`'s
  `DreamSys__ProjectPointAtDistance` feeds it a 3-word offset and treats the result as a map
  position; `DreamAux.c`'s `PlaceDreamAuxEntityByPlayer` does the same for an aux slot.

## Extern arity (round 59)

**Verdict: arity-ok idiom.** Both 4-parameter declarations stay.

**Callee evidence** (`0x8001E600`): entry is `move s1,a0` / `move s2,a1` /
`move s0,a2` and `$a3` is never read — three real arguments, exactly what the
definition in `src/graphics/SceneNode.c` says.

**Why both externs must keep the 4th parameter.** Every known call site sets
`$a3` to zero, and that instruction is in retail:

```
80059460:  move  a3,zero                                  <- DreamSys__ProjectPointAtDistance (DreamSys.c)
80059468:  jal   8001e600 <SceneNode__LocalOffsetToWorldPos>

8005cf78:  jal   8001e600 <SceneNode__LocalOffsetToWorldPos>
8005cf7c:  move  a3,zero                                  <- PlaceDreamAuxEntityByPlayer (DreamAux.c)
```

Reducing either declaration to the definition's three parameters makes the
call a `too many arguments` compile error, and dropping the literal `0` from
the call site deletes the `move a3,zero` and breaks both matches. The callee
ignores the value; the caller still has to place it.

**Declaration sites changed:** none (arity unchanged). `/* arity-ok: ... */`
added to `src/DreamSys.c:364` and `src/DreamAux.c:443`. Oracle green.

## Track 4 (2026-09-25, round 81, charlie)

The two 4-argument `arity-ok` externs (`src/DreamSys.c`, `src/DreamAux.c`)
are gone. SceneNode's one header, `include/SceneNode.h`, declares the
method, and a 3-parameter prototype there would make both callers a compile
error. Both callers set `$a3 = 0` (0x80059460, 0x8005CF7C), so the
definition now takes that word as an unused 4th parameter, `s32 unused`. The
body never reads `$a3`, and the whole image is byte-identical with the
parameter added. The callers upcast (`(SceneNode *)this`,
`(SceneNode *)gDreamAuxWorld`).

## Round 97 (alpha): Sony's GsCOORDINATE2

SceneNodeSub14 is deleted: SceneNode.coord2 is Sony's GsCOORDINATE2 (flg; MATRIX coord, whose t is the offset from the parent; MATRIX workm, whose t is the world position; param, super, sub -- 0x50 bytes, offset for offset). Accessors here follow the compiler's list: tx/ty/tz -> coord.t[0]/[1]/[2], unk38 -> workm.t; a local that holds coord.t or workm.t is `long *` (MATRIX.t is long[3]; s32 is int); any cast to GsCOORDINATE2 * is gone. Byte-identical.

## Round 98 (echo): track 7, moved from src/graphics/SceneNode.c

The `u8 buf[0x20]` buffer is now `MATRIX rot`, and `table` is `worldPos`: byte-identical. The per-axis parent test was re-measured: hoisting it to one `worldPos` computation scores 17/48 with 294257 bytes drifted, so the source comment keeps a one-line `MATCHING:` note. The old comment's `self->unkC` is `self->parent` (SceneNode.h).

The source comment was rewritten as documentation; the one it replaced, verbatim (field names as they were then):

```c
/* Turns an offset given in the object's own local frame into a world
 * position: rotate `src` by the object's orientation (slot84, un-negated
 * angles, same as SceneNode__RotateLocalVector above) and add the object's
 * accumulated world translation. That translation is `coord2->workm.t`, which
 * is GsCOORDINATE2.workm.t -- the composed world matrix's own translation
 * (+0x24 workm, +0x14 into MATRIX = +0x38). SceneNode__RaycastVertical is the function
 * that maintains it, by summing coord.t down the owner chain.
 *
 * The `self->unkC != 0 ? ... : 0` ternary is recomputed before EACH of the
 * three additions rather than hoisted: retail genuinely redoes the NULL
 * test and the address computation three times. When `unkC` is NULL the
 * resulting NULL is still dereferenced, exactly as retail does. */
```
