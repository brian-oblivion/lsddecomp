# func_8003EC2C -- MATCHED 37/37 words

Unit `code_2cc8c_d`, carved round 13. Reopened round 42 as
`nop_mflo_mfhi`-blocked (the blocker is RESOLVED, see CLAUDE.md); the stub
above was never actually attempted until now.

## Round 44 (echo)

Guarded 20.12 fixed-point division: if `self->unk10 != NULL`, computes the
same split-division idiom as `code_d294_c`'s `RatioToFixed12` on a caller-
supplied `{s16 whole; s16 frac;}` pair and stores the result to a
NEW field, `Unk18Obj::unk2C` (previously undiscovered padding, +0x02C --
exactly the 4 bytes between `unk20` (`Vec3_2cc8c`, ending at +0x02C) and the
already-known `unk30`).

```c
void func_8003EC2C(Unk18Obj *self, s16 *pair) {
    s32 q1, r1, q2;

    if (self->unk10 != NULL) {
        q1 = pair[0] / pair[1];
        r1 = pair[0] % pair[1];
        q2 = (r1 << 12) / pair[1];
        self->unk2C = (q1 << 12) + q2;
    }
}
```

Read the input pair as raw `s16 *` rather than reusing `code_d294.h`'s
`WholeFrac_d294` -- this unit has its own local view of the shape and does
not include that header; a shared struct across units is a shared-header
hazard per CLAUDE.md's "one exception" note, and there is no reuse benefit
here since nothing in this unit dereferences the pair as anything but two
adjacent halfwords.

## Header change

`include/code_2cc8c.h`: `Unk18Obj`'s `pad2C[4]` retyped to `s32 unk2C`
(same offset, same size -- no other field moves). Verified safe: this is
the FIRST function in the project to touch `+0x02C` of `Unk18Obj`, so there
is no sibling already-matched function reading/writing that offset to
regress. Confirmed via the full oracle after the edit, not assumed.

Matched on the first build: **37/37 words**, whole-image SHA1 clean (`build
exit=0`). Zero attempts beyond this one.

## Proposed learning

Second confirmation this round (after `RatioToFixed12`, `code_d294_c`) that
the split-division idiom for 20.12 fixed-point (`q,r = a/b, a%b; return (q
<< 12) + ((r << 12) / b);`) is a recognizable retail shape wherever a
`nop_mflo_mfhi`-flagged function's disassembly shows two adjacent
`div`-then-`mflo`/`mfhi`-then-`div` blocks. Two independent units, two
independent occupants of unrelated class hierarchies, same idiom -- this
reads like a shared runtime helper pattern the original codebase used
often, not a coincidence.
