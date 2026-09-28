# SceneNode__UpdateRotation -- MATCHED, round 45 (2026-09-15)

> Renamed from `Class6B5CC__UpdateRotation` on 2026-09-26 (tools/rename.py). Address 0x8001ceb4.

> Renamed from `func_8001CEB4` on 2026-09-23 (tools/rename.py). Address 0x8001ceb4.

**Unit:** `SceneNode` · **Size:** 85 words · **Status:** MATCHED, 85/85 exact,
whole-image SHA1 green.

Filed since round 13/14 as blocked by `nop_mflo_mfhi` (a sibling of the
`addiu_at` blocker). That flag was RESOLVED in round 42 by
`maspsx --no-nop-mflo-mfhi`, pinned in the Makefile. Round 42's own reopen
note on this file recorded that the PRESERVED body, rebuilt with the flag on,
was NOT a match (one word longer than retail, plus a register-identity
difference) -- that finding held: the body needed one more source-level fix
beyond the flag, described below.

## Body

```c
void SceneNode__UpdateRotation(SceneNodeObj *self, s32 flag, void *data) {
    s32 vals[3];
    SceneNodeSub44 *dst;
    s16 *field;

    vals[0] = RatioToFixed12(data);
    vals[1] = RatioToFixed12((u8 *)data + 4);
    vals[2] = RatioToFixed12((u8 *)data + 8);
    vals[0] /= 360;
    vals[1] /= 360;
    vals[2] /= 360;
    dst = self->unk14->unk44;
    field = &dst->vec.x;
    if (flag) {
        dst->vec.x = vals[0];
        dst->vec.y = vals[1];
        dst->vec.z = vals[2];
    } else {
        s32 i;
        s16 *cur;

        for (i = 0; i < 3; i++) {
            cur = field;
            field++;
            *cur = (*cur + vals[i]) % 4096;
        }
    }
    self->unk14->unk0 = 0;
}
```

## What changed from the round-13/round-42 preserved body

The round-13 derivation (three `RatioToFixed12` reads divided by 360, an
`if(flag)` overwrite vs. a wrap-accumulate `else`, unconditional
`field = &dst->vec.x` before the branch) was already correct in full --
confirmed by rebuilding it as-is first this round: **71/85 words match, zero
drift outside the function**, once the toolchain flag was on. The remaining
14-word mismatch was a single loop-shape choice, not a new derivation:

- The original preserved body wrote the accumulate loop as
  `field[i] = (field[i] + vals[i]) % 4096;` (array indexing off a fixed
  base). That produces the right VALUES but retail's own loop is a
  **pointer-walk with a one-iteration-ahead advance**: each pass copies the
  carried pointer into a scratch register, uses the scratch register for
  the load AND the store, and advances the carried pointer immediately
  after the load (before the arithmetic) -- i.e. `cur = field; field++;
  *cur = ...;`, not `field[i] = ...; field++`. Array indexing keeps `field`
  itself as the base and recomputes an offset each time, which allocates
  differently and needed an extra `move` to get the initial pointer into
  the loop's working register -- the "one word longer" residue round 42's
  rebuild note flagged.
- Rewriting the loop with an explicit `cur` scratch pointer (copy-then-
  advance-then-dereference, matching retail's instruction order exactly)
  reproduced retail's register allocation (`field` in `$a3` as the carried
  pointer, `cur` in `$a0` as the per-iteration scratch) with zero remaining
  diff. No other change was needed -- struct layout, division idiom, and
  overall control flow were already right.

## Derivation notes (carried over, still accurate)

- Three `RatioToFixed12` reads (`data+0`/`+4`/`+8`), each divided by 360
  (magic `0xB60B60B7`, shift 8) -- the three divisions are scheduled
  back-to-back by the compiler (each `mult` issued before the previous
  one's `mfhi` is consumed), which is why the raw disassembly interleaves
  them even though the C is three simple, independent statements in
  source order.
- `flag != 0`: overwrite `dst->vec.x/y/z` directly with the divided values
  (truncating store to `s16`, implicit via the destination's field type).
- `flag == 0`: accumulate each divided delta into the existing field value
  and wrap modulo 4096 (a full-turn wrap for a PSX 4096-per-circle angle
  unit) -- ordinary C `%`, verified byte-for-byte against the pinned `cc1`
  in the original round-13 derivation.
- `field = &dst->vec.x` is computed unconditionally before the `if`, not
  inside the `else` alone -- retail materializes it in the `beqz` branch's
  own delay slot on both paths, even though only the `else` path reads it
  back.

### Proposed learning

**A loop that reads-modifies-writes through a pointer and also advances
that pointer can need the advance modeled as an EXPLICIT extra step
(`cur = p; p++; *cur = ...`), not array indexing (`p[i] = ...`) and not a
combined post-increment dereference (`*p++ = ...`) -- even when all three
are value-equivalent C.** This function is the second confirmed instance of
the "resolved toolchain blocker, but the preserved body still needed a
loop-shape fix, not just a rebuild" pattern this round (see also the
round-42 reopen notes generally) -- worth checking on any other queued
function whose STALL predates round 42 and involves a pointer-walking loop
over a small fixed count.

## Naming

Round 71 (alpha). `func_8001CEB4` -> `SceneNode__UpdateRotation`, **tier A**. Table slot +0x044 (round 70 named the slot `updateRotation`). Converts a ratio triple of degrees (RatioToFixed12, /360) into 4096-per-turn units and either assigns (flag != 0) or accumulates mod 4096 into GsCOORD2PARAM.rotate, then clears coord2 flg. Callers agree: DreamSys turns (ROTATION_YAW_PLUS45 etc., accumulate) and sets headings (assign); ObjMStyleActor calls the slot updateRotation.

## Proposed field names

For the head to apply by type scope. Each one fails to compile in another unit when renamed in the definition, so this unit did not apply it.

- `SceneNodeSub44.vec` -> `rotate` (tier A): GsCOORD2PARAM.rotate (SVECTOR at +0x10), which UpdateRotation writes in 4096-per-turn units. Accessors: SceneNode, code_d294_b, code_d294_c.

## Round 97 (alpha): Sony's SVECTOR

GsCOORD2PARAM.rotate is Sony's SVECTOR now (S16Quad_d294 deleted from include/SceneNode.h), so the rotation accessors read `rotate.vx`/`.vy`/`.vz` for the old `.x`/`.y`/`.z`. Byte-identical.

## Round 101 (delta): track 7

Step 2: the three raw offsets `(u8 *)data + 4` / `+ 8` read the table as what it is, Ratio16[3] (include/SceneNode.h), through a local `Ratio16 *ratios = data`: `&ratios[0]`, `&ratios[1]`, `&ratios[2]`. The parameter stays `void *` because the prototype and the slot type in include/SceneNode.h say so (not this unit's to change; proposed). Byte-identical.

Step 3 (locals and parameters): `flag` -> `set`, `data` -> `table` (the prototype's names), `vals` -> `angles`, `dst` -> `param` (GsCOORD2PARAM), `field` -> `next` (the carried pointer the loop advances ahead of `cur`). Byte-identical.

Step 4 (constants): `360` -> `DEGREES_PER_TURN` (unit-local: a degree count in 20.12 over 360 is the angle in 4096ths of a turn), `% 4096` -> `% ONE` (libgte's ONE, one full turn in GsCOORD2PARAM.rotate's unit). Byte-identical.

Step 5 (comments): Function comment added, and two `MATCHING:` lines, both measured this pass: dividing inside each call's statement (`RatioToFixed12(...) / DEGREES_PER_TURN`) breaks the build, as the report above records for the loop shape and the pre-branch `next`.
