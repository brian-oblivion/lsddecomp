# SetTargetOffset -- MATCHED (14/14 words), round 82

> Renamed from `func_80020510` on 2026-09-25 (tools/rename.py). Address 0x80020510.

Round 82, runner charlie (matching slot). Unit `src/code_fa50.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table; called from class_3bb8c_o.c as `SetTargetOffset(obj, &D_8008AB94)` (`tools/classtable.py gTmdModelMethods`).
- **What:** `t = self->unk10->unk10; v = xy[0] / 16; t->unk6 = v; t->unk6 = v + xy[1] * 64;`: a double store to one s16 field, the second one reusing the first value without reloading it.
- **Result:** byte-exact; 14/14 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views (`TmdModel`, `ModelData_fa50`, `Quad_fa50`, `TmdObject_fa50`, `Outer_fa50`/`Inner_fa50`/`Target_fa50`) and prototypes live in the unit; no shared header was touched.
- **Lever:** `sh v; ...; addu v, v, x; sh v` to the SAME field with no `lh` in between comes from a named `s32` local stored twice. `t->unk6 += ...` would reload the truncated s16.

## Source

```c
void SetTargetOffset(Outer_fa50 *self, s16 *xy) {
    Target_fa50 *t = self->unk10->unk10;
    s32 v;

    v = xy[0] / 16;
    t->unk6 = v;
    t->unk6 = v + xy[1] * 64;
}
```

## Naming

`SetTargetOffset` -- KEPT (not renamed this round). Tier C: mechanics known
(`t->unk6 = xy[0]/16; t->unk6 = ... + xy[1]*64;`, a double store overwriting
the field), but its only caller is `src/class_3bb8c_o.c`, a live types-runner
unit this round; the class owning the `Outer_fa50`/`Inner_fa50`/`Target_fa50`
chain is itself unconfirmed (see `AccumulateTargetOffset.md`, its sibling).

## Proposed name

`SetTargetOffset` -- tier B, discriminating it from its sibling
`AccumulateTargetOffset` (renamed this round, no collision): this one SETS
the field from a fresh value, the sibling ACCUMULATES onto the existing
one. Posted to the broadcast for the head to apply once `class_3bb8c_o.c`
is not live and the owning class is known.

## Track 7 (2026-09-26, round 94, bravo)

Named the fields (see `AccumulateTargetOffset.md`'s matching entry for the
full rationale; both functions share the same three structs and both are
the only readers/writers, all inside this unit): `Target_fa50::unk6` ->
`offset`, `Inner_fa50::unk10` -> `target`, `Outer_fa50::unk10` -> `inner`.
`class_3bb8c_o.c`'s call site (`SetTargetOffset(obj, &D_8008AB94)`) passes
opaque pointers and never names these fields itself, so the field rename
does not touch it. Compiler-verified accessor list, build and
check-nonmatching.sh green.
