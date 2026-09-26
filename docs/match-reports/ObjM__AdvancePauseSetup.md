# ObjM__AdvancePauseSetup

> Renamed from `func_800542D0` on 2026-09-23 (tools/rename.py). Address 0x800542d0.

**Unit:** class_3bb8c_m · **Size:** 75 instructions · **Status:** MATCHED (75/75 words)

## Context

Establishes `ObjM::unk74` (`void *`, forwarded opaquely to
`New_TextRow`'s `ctx` argument) and `ObjM::unk7C` (`FieldM7C *`, first
WRITTEN here from `New_TextRow`'s return, then dispatched in
`ObjM__TeardownPauseOverlay`). Also the first function in this unit to call the
already-matched-elsewhere `New_TextRow` (`src/code_2cc8c_f.c`,
established there with return type `Unk64Elem *` -- this unit keeps its
own independent local return type `FieldM7C *` for the same external
symbol, which is fine: each translation unit's own typing of a shared
external is independent and does not touch the other unit's bytes).

## What this function does

A small state machine gated on `self->unk80`:

```c
void ObjM__AdvancePauseSetup(ObjM *self) {
    s32 state = self->unk80;
    if (state == 0) {
        self->unk7C = New_TextRow(self->unk74, 5, &D_8008AB44[0]);
        self->unk7C->methods->slot4C(self->unk7C, self->unk14, &D_8008AB38);
        self->unk7C->methods->slotB8(self->unk7C, &D_8008AB40);
        self->unk80 = state + 1;
        return;
    }
    self->unk80 = state + 1;
    if (state != 4) {
        return;
    }
    self->unk18->methods->slotB4(self->unk18, 0);
    self->unk10->methods->slot4C(self->unk10);
    self->unk54->methods->slot4C(self->unk54);
    self->unk34->methods->slot88(self->unk34);
}
```

`D_8008AB44` is the string `"Pause"` (`asm/data/7B008.sdata.s`); the two
other literal-address arguments (`D_8008AB38`, `D_8008AB40`) are small
opaque blocks, address-only in this unit.

## The residue: the function is `void`, not `s32` -- found by the permuter after 18 failed manual reshapes

Every earlier draft typed this `s32`, on the theory that retail's final
instruction sequence (`li v0,1` / `j` / `sw v0,0x80(s0)` in the
zero-state branch, and a bare `li v0,4` reached by fallthrough in the
non-4 branch) meant the literal was both stored AND returned -- an
instance of the project's documented `return self->field = N;` idiom.
That theory was wrong, and finding out took real effort:

**18 isolated reproducers through the pinned pipeline (`tools/gcc263/cpp
| cc1 | maspsx | as`), none of which reproduced retail.** Every `s32`
phrasing tried --

- `self->unk80 = 1; return 1;` (two statements)
- `return self->unk80 = 1;` (the documented idiom)
- an explicit temp: `s32 v = 1; self->unk80 = v; return v;`
- `self->unk80 = state + 1;` so the zero-branch's literal is the SAME
  expression as the other branch's (constant-folds to 1 when
  `state == 0`), instead of a bare `1`
- `goto` to a shared tail label instead of a direct `return`
- routing the OTHER branch's return through the identical
  `return self->field = N;` idiom too, for symmetry
- a single local `result` set in each arm with one shared
  `return result;` at the end

-- produced the SAME extra instruction every time: `li v0,1` (matching
retail) immediately followed by a SECOND, otherwise-redundant `li v1,1`,
with the store then reading `v1` instead of `v0`. Isolating further
(`/tmp/repro1.c` .. `/tmp/repro18.c` in that session, not preserved in
the repo) narrowed the trigger to "a function with **two or more return
paths**, where one path both STORES and RETURNS the same literal
immediately after an INDIRECT call (`jalr`)" -- present regardless of
whether the second path is a trivial early return, whether a local
`state` variable is read at the top, and regardless of how many
statements separate the store from the call. A bare `return 1;` with NO
struct store, in the identical two-path shape, does NOT trigger it. This
is a genuine GCC 2.6.3 register-allocation quirk tied to the RETURN VALUE
register being live across a branch merge, not something reachable by
reshaping the store/return statement.

**The permuter found the way out in 486 iterations (~2 minutes,
`-j 6 --stop-on-zero`): declare the function `void`.** With no
caller-visible return register, GCC no longer needs to keep `$v0`
consistent across the three exits, and the ordinary "compute a constant,
store it" codegen naturally lands in `v0` with no merge-driven eviction
-- reproducing retail exactly. The `return X;` statements in a `void`
function are legal (GCC 2.6.3 accepts and simply discards the value,
matching the two `return;`-with-no-value forms used above); the
permuter's own winning source kept `return self->unk80 = state + 1;`
verbatim, which also scores 0 -- this report's idiomatic version splits
it into `self->unk80 = state + 1; return;` for clarity, verified to
score identically.

Nothing in this unit calls `ObjM__AdvancePauseSetup` (still uncalled within
`class_3bb8c_m`), so there is no caller evidence either way about the
return type -- the toolchain fact above (only `void` reproduces the
byte sequence) IS the evidence.

## Proposed learning

**A function with 2+ distinct return points, where one point both STORES
a literal into a struct field AND returns that same literal immediately
after an indirect (`jalr`) call, cannot be typed `s32` no matter how the
store/return is phrased -- try `void` before spending more than two or
three reshaping attempts.** GCC 2.6.3 reserves `$v0` for the
merge-consistent return value across all exits and evicts the store's
operand to a second register (`$v1`), producing one genuinely
unavoidable extra instruction in `s32` form. Eighteen reshapes (documented
above) all failed identically; the permuter found the fix in under 500
iterations once pointed at it. This generalizes the existing "discarded
return value is never evidence of void" caution in the opposite
direction: here a *used*-looking return value, RIGHT NEXT TO a store of
the identical constant, is exactly the shape that turns out to be void.
The tell in the disassembly: a literal computed once (`li $v0,K`) and
reused directly by the very next instruction's delay slot for BOTH the
store and the implicit return, with NO redundant second `li` anywhere
near it -- if your own C reproduces that instruction but adds a spurious
sibling `li` in a second register, the function is void, not the
constant needing yet another phrasing.

## Provenance

round 15b (2026-09-04), runner echo, second pass on `class_3bb8c_m`.
Permuter transcript and the 18 reproducers referenced above were run
in this session; the reproducers themselves were scratch files under
`/tmp`, not preserved.

## Naming

**ObjM__AdvancePauseSetup** -- tier B. A 5-step counter (`self->unk80`, 0..4) driving a state machine: on step 0, builds an object literally named "Pause" (`New_TextRow(self->unk74, 5, &D_8008AB44[0])`, `D_8008AB44` == "Pause", asm/data/7B008.sdata.s); on the final step (4), notifies several sibling components. The literal string is strong, concrete evidence for the "pause overlay" reading, but the class's exact game role stays tier B.


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the class_3bb8c_k/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and class_39e08.h's Obj4C/SubObjB/EventArg are gone. Byte-identical. Holders now typed: pauseText (+0x07C, TextRow: attachToParent, setColor), etcTim (+0x074, New_TextRow's font), viewport (Class869D8 setDrawEnabled), unk10 (FrameClock pause), bgm (WBgm pause), sound (VabStreamObj mute).
