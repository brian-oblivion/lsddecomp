# Actor__NotifyMove -- MATCHED (74/74 words)

> Renamed from `BaseObjO__func_571f8` on 2026-09-25 (tools/rename.py). Address 0x800571f8.

> Renamed from `func_800571F8` on 2026-09-18 (tools/rename.py). Address 0x800571f8.

Unit: `dream_scene` (round 17). `BaseObjOMethods::slot88` (via the fixed
`GetSceneNodeMethods()` table) followed by a guarded body that only runs for
`arg1` in `[5, 9)`: an "is armed" check on `self->unk20`, an own-`slot8C`
call, a conditional angle-adjustment helper call, an own-`slot90` call, and
a tag-gated `slotE8` notification through `self->unk28`. The most
structurally involved function in this unit's queue -- 74 words, several
independent control-flow decisions, and one residue that took real
iteration to close.

## Final source

```c
void Actor__NotifyMove(BaseObjO *self, s32 arg1) {
    GetSceneNodeMethods()->slot88(self, arg1);
    /* Written as two nested guards, not a combined `arg1 >= 5 && arg1 < 9`
     * range test -- the combined form optimizes into a single unsigned
     * `(arg1-5) < 4` comparison, which is not what retail does (two
     * separate `slti`s). */
    if (arg1 < 9) {
        if (arg1 >= 5) {
            Buf38O buf;

            if (self->unk20 != NULL && TmdModel__GetBoundsCount(self->unk20)) {
                self->methods->slot8C(self, &buf);
                if (arg1 != 5) {
                    s16 h = self->unk48;
                    s32 isSeven = (arg1 == 7);
                    s32 nonneg = (h >= 0);
                    s32 adjusted;

                    /* `goto`, not `if/else`, to match retail's actual
                     * branch shape. */
                    if (h < 0) {
                        goto negative;
                    }
                    adjusted = h + self->unk54;
                    goto joinAdjust;
                negative:
                    adjusted = h - self->unk54;
                joinAdjust:
                    RotateAndOffsetHullList(&buf, isSeven, nonneg, adjusted);
                }
                self->methods->slot90(self, &buf, arg1);
                if (self->unk28 != NULL) {
                    if (self->unk28->methods->tag == 0x34) {
                        self->unk28->methods->slotE8(self->unk28);
                    }
                }
            }
        }
    }
}
```

## Derivation, in the order the residues were found

1. **The scratch buffer is 0x38 bytes, not 0xC.** The naive first cut used
   a `Vec3O buf` (12 bytes) for the local retail addresses at `$sp+0x10`,
   which undersized the frame (`addiu $sp,$sp,-0x30` instead of retail's
   `-0x58`) and shifted every function after this one in the whole
   translation unit (148KB of outside-range drift on the first build).
   Measured retail's true buffer size from its OWN frame layout: saved
   registers start at `$sp+0x48`, the buffer starts at `$sp+0x10`, so it's
   `0x38` bytes. Declared a dedicated opaque `Buf38O { u8 raw[0x38]; }` and
   retyped `BaseObjOMethods::slot8C`/`slot90` and `RotateAndOffsetHullList`'s first
   parameter to it. This single fix took the function from a near-total
   mismatch to 69/74.
2. **The range guard needs TWO nested `if`s, not one combined
   `arg1 >= 5 && arg1 < 9`.** GCC 2.6.3 optimizes the combined boolean
   expression into a single unsigned `(arg1-5) < 4` comparison (`addiu
   $v0,$s1,-5` / `sltiu $v0,$v0,4`), which retail does not do -- retail
   has two separate `slti`s (`arg1<9` then `arg1<5`, inverted into the
   guard). Nesting the ifs (matching CLAUDE.md's "an inverted guard clause,
   not the `else` of a big `if`" family of levers, applied here to
   suppress an unwanted OPTIMIZATION rather than to reshape a big-if/else)
   restored the two-`slti` shape.
3. **The angle-adjustment fork needed `goto`, not `if`/`else`.** Writing
   `if (h < 0) { adjusted = h - unk54; } else { adjusted = h + unk54; }`
   compiles to the DEFAULT shape (test the `if` condition, branch PAST the
   `then`-block to reach `else` on failure) -- `bgez`, testing `h >= 0`
   first, with the "then" (negative) block inline and the "else" (add)
   block as the branch target. Retail does the OPPOSITE: `bltz`, testing
   `h < 0` first as an early-exit-style guard, with the ADD case inline
   (fall-through) and the SUBTRACT case as the branch target followed by a
   `j` back to the shared continuation. This is exactly the documented
   "when a branch seems to vanish... write the literal jump graph with
   `goto`" family, applied to an ordinary two-armed arithmetic fork rather
   than a vanishing comparison. Rewriting as an explicit
   `if (h<0) goto negative; ...add...; goto join; negative: ...subtract...;
   join:` reproduced retail's branch polarity exactly.
4. **The last 2 words: `(u32)(~h) >> 31` vs. `(h >= 0)` for the same
   value.** After steps 1-3 the function was 72/74 -- both remaining
   differing words were the SAME two instructions (`nor`/`srl`) with a
   DIFFERENT destination/source register (`$a2` in retail vs. `$v0` in the
   build, with an extra `move` implied downstream). This is exactly what
   CLAUDE.md calls a register-identity residue -- but per the "if
   reshaping does not move it, it is a stall" test, it WAS reachable:
   roughly a dozen reshapes of the `nonneg` computation (moving the
   declaration, inlining it at the call site, adding a scheduling barrier,
   reordering relative to `isSeven`) either left it at 72/74 unchanged or
   made things much worse (blowing the frame size back up), UNTIL simply
   writing the semantically-equivalent boolean `s32 nonneg = (h >= 0);` in
   place of the explicit bit-trick `(u32)(~h) >> 31` closed it to 74/74.
   GCC 2.6.3 apparently compiles the plain `>=` comparison for a value it
   already knows the sign-extension width of (a `s16` promoted to `s32`)
   into the identical `nor`/`srl` MACHINE CODE retail has, but allocates
   the destination register differently than when the same bit pattern is
   spelled out by hand as `~x >> 31`. This is the OPPOSITE of CLAUDE.md's
   "do not transcribe a lowering back into C" caution in the usual
   direction (the transcribed form was actually WORSE here, not just
   unnecessary) -- the semantic spelling won on both counts.

### Proposed learning

- **A "sign flag via `nor`+`srl`" register-identity residue can resolve by
  writing the semantic boolean (`x >= 0`) instead of the literal bit-trick
  (`~x >> 31`), even though both compile to the identical `nor`/`srl`
  instruction PAIR.** Only the register allocation differs between the two
  spellings, and the semantic one matched retail here. Cheap to try before
  filing a register-identity stall on this specific shape (unsigned-shifted
  bitwise-not of a small signed value, used as a boolean flag).
- **A combined boolean range test (`a <= x && x < b`) can compile to a
  single unsigned bounds check that retail's own source did NOT use**, even
  when the two-branch `slti`/`slti` shape is otherwise a completely
  ordinary, unremarkable range guard. Nesting the two conditions as
  separate `if`s (rather than combining with `&&`) suppresses the
  optimization. Distinct from (though related in spirit to) the
  already-documented inverted-guard-clause family -- this one is about
  suppressing an unwanted STRENGTH REDUCTION, not about `if`/`else` layout.

## Naming

**`Actor__NotifyMove` -- tier C.** The most structurally involved
function in this unit (74 words) and the existing report already
describes its mechanics exhaustively (a range-gated dispatch through
`slot8C`/`slot90`/`slotE8`, an "armed" check via `TmdModel__GetBoundsCount`, and a
sign-based adjustment of `unk48` by `unk54`) -- but nothing establishes
what the `arg1` range `[5,9)` selects between, what "armed" means in the
game, or what `unk20`/`unk28`/`unk48`/`unk54` actually represent. Kept the
tier-C `Class__func_xxxxx` form rather than inventing a purpose for the
most consequential unknown in the unit.

## Track 4 (2026-09-25, round 82, delta)

Renamed from `BaseObjO__func_571f8`. Override of +0x088 (notifyIfUnk20Active). Not named for the slot because the body does more: it chains the base, then for events 5..8 with an active model reads the model data (readUnk20Data, +0x08C), for events other than 5 adjusts it by lastOffsetValue +- pendingExtra (RotateAndOffsetHullList), hands it to transformAndNotifyParents (+0x090), and calls slotE8 on an Actor linkTarget. Events 6, 7 and 8 are exactly the ones Actor__MoveLocalZ/X/Y pass (through Actor__MoveAlongLocalAxis); tier B. The fields read here: unk20 = SceneNode.model, unk28 = linkTarget, unk48 = lastOffsetValue, unk54 = pendingExtra (s32: this function's addu/subu settle the merge CONFLICT with DreamSys's void *). The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/world/dream_scene.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 99, alpha)

Name unchanged (tier B). Constants: the event tests use enum ActorMoveEvent (include/actor.h; 5 ACTOR_EVENT_UNSWEPT, 6..8 ACTOR_EVENT_MOVED_Z/X/Y, the events Actor__MoveAlongLocalAxis sends), the class-byte test ACTOR_CLASS_ID, read as `(u8)linkTarget->methods->header` (the same lbu) instead of `*(u8 *)methods`. Locals: h/isSeven/nonneg/adjusted -> offset/alongX/forward/delta, which are RotateAndOffsetHullList's delta/turn/back; labels negative/joinAdjust -> backward/offsetHull. The two derivation comments (nested guards, goto) are one `MATCHING:` line each now; the derivation is items 2 and 3 above.

**The buffer is a plain TmdHull, measured.** The unit carried a local `Buf38O { u8 raw[0x38]; }` with this comment, moved here as history:

> Actor__NotifyMove's model-data buffer, filled by readUnk20Data and handed to RotateAndOffsetHullList and transformAndNotifyParents. Retail's frame needs it to be 0x38 bytes (sp+0x10 .. sp+0x47, the saved registers from sp+0x48); a smaller buffer shifts everything after the function. Its layout is not established here.

The frame does need 0x38 bytes there, but a `TmdHull` (0x34: count plus eight TmdVec3 corners) gets the same slot, because cc1 rounds the local to 8: with `TmdHull hull;` the function is 74/74 and the whole image byte-identical. So the type and both casts (`(TmdHull *)&buf`) are gone.
