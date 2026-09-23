# IsStyleCueNear -- MATCHED (31/31 words), class_3bb8c_n

> Renamed from `func_80055874` on 2026-09-23 (tools/rename.py). Address 0x80055874.

Round 46 (second sitting, alpha). Byte-exact, whole-image SHA1 verified.

## Signature

```c
s32 IsStyleCueNear(ObjN14 *arg0, void *arg1);
```

Called from `StopStyleCueIfNear` in this unit (already matched, forward-declared
this signature). `arg0` is the same `ObjN14 *` local-view type this unit
carries for `FlushStyleCue`/`StopStyleCueIfNear`; this function is the first to
touch its padding region, so the local struct grows three named fields.

## Struct: `ObjN14` grows three fields

The previous sitting's `ObjN14` only named `unk0` (+0x000, `ObjN14Sub *`) and
`unk14` (+0x014), with everything between as opaque `pad4[0x10]`. This
function reads/writes +0x004, +0x00C and +0x010 directly, so the pad is split:

```c
struct ObjN14 {
    ObjN14Sub *unk0; /* +0x000 */
    s32 unk4;        /* +0x004 */
    u8 pad8[0x4];
    s32 unkC;        /* +0x00C */
    s32 unk10;       /* +0x010 */
    s32 unk14;       /* +0x014 */
};
```

`+0x008` is still unaccounted for (never touched by any function in this
unit so far) and stays as `pad8`. `ObjN14` is a unit-local view (not in
`include/class_3bb8c.h`), so this edit cannot affect any other unit --
confirmed with `grep -rl ObjN14 src/ include/` before editing.

## Body

```c
extern s32 D_80087474[];

s32 IsStyleCueNear(ObjN14 *arg0, void *arg1) {
    s32 dx, dy, dist;
    s8 idx;

    if (arg1 == 0) {
        return 0;
    }
    dx = arg0->unk4 - *(s32 *) arg1;
    if (dx < 0) {
        dx = ~dx + 1;
    }
    dy = arg0->unkC - *(s32 *) ((u8 *) arg1 + 0x8);
    if (dy >= 0) {
        dist = dx + dy;
    } else {
        dist = dx - dy;
    }
    arg0->unk10 = dist;
    idx = arg0->unk0->unk6;
    if (dist < D_80087474[-idx]) {
        dist = 1;
        return dist;
    }
    return 0;
}
```

`arg1` is read only at offsets +0x000 and +0x008 (both `s32`), most likely a
position/vector pointer whose middle (Y?) field is skipped -- consistent with
a 2D (X/Z) proximity check. `D_80087474` is a 15-entry `s32[]` in
`asm/data/76DC8.data.s` (dlabel, offsets 0x77C74-0x77CAC) indexed *backward*
from `idx` (`D_80087474[-idx]`), which is an `ObjN14Sub::unk6` (`s8`,
presumably a small negative "type" tag, 0..-14).

## Two levers found closing this one -- both needed, in this order

**1. Negation idiom affects delay-slot filling.** `if (dx < 0) dx = -dx;`
compiles (GCC 2.6.3, `-O2`, this pipeline) to a single `negu`/`subu`
instruction placed AFTER the branch (delay slot filled with `nop`), because a
plain `subu $reg,$zero,$reg` would corrupt the "already non-negative" path if
it executed unconditionally in the delay slot. Retail instead has
`nor $v0,$zero,$a2` (unconditional, safe -- writes to a scratch register, not
the tested one) in the delay slot, then `addiu $a2,$v0,1` only on the
not-taken path. Writing the negation as literal two's-complement --
`dx = ~dx + 1;` instead of `dx = -dx;` -- reproduces this exactly: GCC keeps
the `~` as a `nor` and the `+1` as a separate `addiu`, and the delay-slot
filler can now hoist the (safe, scratch-register) `nor` into the branch delay
slot. Verified in isolation through the pinned pipeline
(`tools/gcc263/cpp | cc1 | maspsx | as`) before touching `src/`.

**2. GCC 2.6.3's own jump optimizer collapses `if (cond) return 1; return 0;`
into a bare `slt`+`return`, when the compared value has no other use after
the branch.** Retail's tail is NOT collapsed -- it keeps a redundant
`slt` / `bnez` / `li v0,1` (delay slot) / `move v0,zero` (fallthrough) /
`jr ra`, i.e. it re-materializes the boolean from the branch instead of
reusing the `slt` result directly. Every natural rephrasing I tried
(`return cond;`, `if/return` either polarity, `if/else` assigning a fresh
result variable, `if (cond) return 1; else return 0;`, different return
types s8/u8/s16/s32, swapped comparison operands) collapsed identically to
the SHORT form -- confirmed both with a same-context reproducer through the
pinned pipeline and in-tree. **What defeats the collapse: assigning back into
the ALREADY-LIVE variable that fed the comparison, then returning that
variable** -- `dist = 1; return dist;` instead of `return 1;`. Since `dist`
still has a live use (it was just compared and stored to `arg0->unk10`
earlier), GCC's jump optimizer does not fold the store-flag pattern away.
Found by a bounded permuter search (60 iterations, `--stop-on-zero
--best-only`, all three permuter checks implicitly satisfied since the
scaffold IS the reproduction target and the found body was verified byte-
exact in the real in-tree build immediately after); reproduced and confirmed
in isolation via the pinned pipeline before landing it.

### Proposed learning

**A `slt`-fed boolean return that stays as a real `bnez`+materialize-1/`
materialize-0 pair (instead of collapsing to a bare `slt`+`return`) is a sign
the retail source reassigned the ALREADY-LIVE compared variable to the
literal before returning it**, e.g. `x = 1; return x;` rather than
`return 1;`. GCC 2.6.3's own jump/store-flag optimizer folds the latter
unconditionally at `-O2` in this pipeline (confirmed across every operator
polarity, return width and clause shape tried) but leaves the former alone
because the destination is not a dead/fresh pseudo. Worth checking on any
same-length `slt`-then-immediately-`jr` residue elsewhere in the queue where
retail's own bytes show the "redundant" branch-and-materialize shape instead.

## Attempts

Two direct pipeline reproducers (negation idiom, boolean-collapse family) plus
one bounded permuter search (60 iterations, found score 0 at iteration 60).
Well under the 30-*build* cap (this was 1 real `build-and-verify.sh` run
after the pinned-pipeline reproducers converged).

## Naming

**`IsStyleCueNear`, tier B.**

Computes a Manhattan-style X/Z distance from `arg0`'s own position
(`posX`/`posZ`) to `arg1`, stores it into `arg0->lastDist`, and returns 1 if
under the `D_80087474[-idx]` threshold (`idx` from the claimed entry's
`countSign`) else 0. Not tier A despite the boolean-getter shape: it has a
real side effect (`lastDist` write) beyond the return value, so it is not a
"pure leaf" by track 3's tier-A test. MATCHED, 31/31.
