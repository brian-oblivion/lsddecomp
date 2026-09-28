# Viewport__SetTwist -- MATCHED 37/37 words

> Renamed from `Unk18Obj__SetRatio12` on 2026-09-25 (tools/rename.py). Address 0x8003ec2c.

> Renamed from `func_8003EC2C` on 2026-09-23 (tools/rename.py). Address 0x8003ec2c.

Unit `Task`, carved round 13. Reopened round 42 as
`nop_mflo_mfhi`-blocked (the blocker is RESOLVED, see CLAUDE.md); the stub
above was never actually attempted until now.

## Round 44 (echo)

Guarded 20.12 fixed-point division: if `self->unk10 != NULL`, computes the
same split-division idiom as `SceneNode`'s `RatioToFixed12` on a caller-
supplied `{s16 whole; s16 frac;}` pair and stores the result to a
NEW field, `Unk18Obj::unk2C` (previously undiscovered padding, +0x02C --
exactly the 4 bytes between `unk20` (`Vec3_2cc8c`, ending at +0x02C) and the
already-known `unk30`).

```c
void Viewport__SetTwist(Unk18Obj *self, s16 *pair) {
    s32 q1, r1, q2;

    if (self->unk10 != NULL) {
        q1 = pair[0] / pair[1];
        r1 = pair[0] % pair[1];
        q2 = (r1 << 12) / pair[1];
        self->unk2C = (q1 << 12) + q2;
    }
}
```

Read the input pair as raw `s16 *` rather than reusing `scene_node.h`'s
`Ratio16` -- this unit has its own local view of the shape and does
not include that header; a shared struct across units is a shared-header
hazard per CLAUDE.md's "one exception" note, and there is no reuse benefit
here since nothing in this unit dereferences the pair as anything but two
adjacent halfwords.

## Header change

`include/Task.h`: `Unk18Obj`'s `pad2C[4]` retyped to `s32 unk2C`
(same offset, same size -- no other field moves). Verified safe: this is
the FIRST function in the project to touch `+0x02C` of `Unk18Obj`, so there
is no sibling already-matched function reading/writing that offset to
regress. Confirmed via the full oracle after the edit, not assumed.

Matched on the first build: **37/37 words**, whole-image SHA1 clean (`build
exit=0`). Zero attempts beyond this one.

## Proposed learning

Second confirmation this round (after `RatioToFixed12`, `SceneNode`) that
the split-division idiom for 20.12 fixed-point (`q,r = a/b, a%b; return (q
<< 12) + ((r << 12) / b);`) is a recognizable retail shape wherever a
`nop_mflo_mfhi`-flagged function's disassembly shows two adjacent
`div`-then-`mflo`/`mfhi`-then-`div` blocks. Two independent units, two
independent occupants of unrelated class hierarchies, same idiom -- this
reads like a shared runtime helper pattern the original codebase used
often, not a coincidence.

## Naming

`Unk18Obj__SetRatio12` -- tier B. Computes a 20.12 fixed-point value from a caller-supplied `{s16 whole; s16 frac;}` pair via the same split-division idiom as `SceneNode`'s `RatioToFixed12` (divide for quotient+remainder, divide the shifted remainder again for the fraction), guarded by `self->unk10`, stores to `unk2C`. Named after the identified idiom (matches an existing, already-named sibling function's own algorithm), not after any established in-game meaning for the ratio.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__SetRatio12`. Renamed for the GsRVIEW2 member it writes: `refView.rz`, the twist, as a 20.12 value from a `Ratio16` (the same split division as RatioToFixed12, which reads the same type). Slot +0x080 `setTwist`. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.


## Track 7 (round 95, alpha, polish pass)

Locals `q1`/`r1`/`q2` -> `whole`/`rem`/`frac`; `<< 12` -> `* ONE` (libgte.h's 4096, 20.12 fixed point), byte-identical. The body is the same num/den -> 20.12 computation as `RatioToFixed12`, inlined.
