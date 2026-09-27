# RotateAndOffsetHullList -- MATCHED (147/147 words), round 82

> Renamed from `func_8001F66C` on 2026-09-25 (tools/rename.py). Address 0x8001f66c.

Round 82, runner charlie (matching slot, second pass on the unit). Unit `src/code_fa50.c`. Fresh ground, no prior attempt.

- **What:** walks a counted list of box-corner sets (`HullList_fa50`: `s32 n` then `n` x 48-byte `BoxCorners`, eight `{s16 x,y,z}` as two faces of four; `TmdModel__GetHull` writes a list of one). For each: if `turn`, copy the 48 bytes to a stack temp and permute the corners back in (a quarter turn of the box), then add `d` to `.x` of face `back`; otherwise add `d` to `.z` of face `back`. Caller: `class_3bb8c_o.c` (`RotateAndOffsetHullList(&buf, isSeven, nonneg, adjusted)`).
- **Result:** byte-exact; 147/147 words, whole-image SHA1 green. Build 5.
- **Builds / levers, measured:**
  1. flat `Vec3 v[8]`, face loops as `v[k]` / `v[k + 4]`: 106/147, 1 word long per `+4` loop (`li a1,0x18; addu v0,t1,a1` instead of retail's `move v1,t1` + `0x18(v1)`).
  2. **`Vec3 f[2][4]` (two faces) and `c->f[1][k]`**: 121/147, right length; only the counter/pointer registers swapped in all four face loops (retail counter `a0`, pointer `v1`).
  3. `k` declared first: no change (121).
  4. hand-stepped `Vec3 *p` pointer per loop: 96/147, size changed.
  5. **a separate `s32 k;` declared inside each of the four branches**: 147/147.
- **Types:** the 48-byte struct copy `tmp = *c` gives retail's lwl/lwr-or-lw 16-byte-chunk copy loop with the alignment test. `Hull_fa50` (`TmdModel__GetHull`) is left as `{ s32 type; Vec3 v[8]; }`; it is the one-element case of `HullList_fa50`.

## Source

```c
typedef struct Vec3_fa50 { s16 x, y, z; } Vec3_fa50;
typedef struct BoxCorners { Vec3_fa50 f[2][4]; } BoxCorners;
typedef struct HullList_fa50 { s32 n; BoxCorners c[1]; } HullList_fa50;

void RotateAndOffsetHullList(HullList_fa50 *h, s32 turn, s32 back, s32 d) {
    BoxCorners tmp;
    BoxCorners *c;
    s32 i;

    for (i = 0; i < h->n; i++) {
        c = &h->c[i];
        if (turn != 0) {
            tmp = *c;
            c->f[0][3] = tmp.f[0][0];
            c->f[0][2] = tmp.f[0][1];
            c->f[1][2] = tmp.f[0][2];
            c->f[1][3] = tmp.f[0][3];
            c->f[0][0] = tmp.f[1][0];
            c->f[0][1] = tmp.f[1][1];
            c->f[1][1] = tmp.f[1][2];
            c->f[1][0] = tmp.f[1][3];
            if (back == 0) {
                s32 k;
                for (k = 0; k < 4; k++) {
                    c->f[0][k].x += d;
                }
            } else {
                s32 k;
                for (k = 0; k < 4; k++) {
                    c->f[1][k].x += d;
                }
            }
        } else {
            if (back == 0) {
                s32 k;
                for (k = 0; k < 4; k++) {
                    c->f[0][k].z += d;
                }
            } else {
                s32 k;
                for (k = 0; k < 4; k++) {
                    c->f[1][k].z += d;
                }
            }
        }
    }
}
```

### Proposed learning

Several sibling counted loops whose counter and strength-reduced pointer come out register-swapped (counter where retail has the pointer, at every loop) close by declaring the counter INSIDE each branch (`{ s32 k; for (k = 0; ...) }`) instead of once at function top: each block-local is a fresh pseudo numbered after the loop's giv base, which flips the allocation order. Declaration reordering at function top did not move it.

## Naming

`RotateAndOffsetHullList` -- KEPT (not renamed this round). Tier C: mechanics fully
known (rotates a counted list of box-corner sets a quarter turn and offsets
one face), but its only caller is `src/class_3bb8c_o.c`, a live types-runner
unit this round; renaming would rewrite that unit's extern declaration and
call site out from under it.

## Proposed name

`RotateAndOffsetHullList` -- tier B (mechanics: turn + per-axis offset of a
`HullList_fa50`; the game-level purpose of the turn is not established).
Posted to the broadcast for the head to apply once `class_3bb8c_o.c` is not
live.

## Track 7, re-send (2026-09-26, round 94, bravo)

Unit-local fields renamed (every accessor is in this function):
`HullList_fa50::n` -> `count` and `::c` -> `boxes` (the same shape as
TmdHull's `count`), `BoxCorners::f` -> `face` (two faces of four corners:
TmdModel__GetHull's v[0..3] min-z face and v[4..7] max-z face). Parameter
`d` -> `delta` (added to x or z of one face). `turn` and `back` kept: they
already say what they select. A `MATCHING:` line marks the per-branch `k`
(build 5 above). The types themselves are track 6's. Byte-identical.

## Naming (track 6, round 95)

2026-09-27, delta: `HullList_fa50` (`s32 count; BoxCorners boxes[1];`) is
deleted and the parameter is `include/TmdModel.h`'s `TmdHull`. Same layout
(0x34 bytes: the count, then 48 bytes of corners) and the same object: the
one caller, `Actor__NotifyMove` (class_3bb8c_o.c), fills its buffer through
`getModelHull` (TmdModel__GetHull writes a `TmdHull`) and hands the same
buffer on as `(TmdHull *)&buf`; TmdHull's own comment already calls it a
counted list of boxes' corners. Box `i` is `&((corners type *)h->v)[i]`, the
`v[8]` array viewed as groups of eight: byte-identical to `&h->boxes[i]`
(the address is `h + 4 + i * 48` either way). The two-faces-of-four corner
type stays, because the rotation permutes faces (build 2 above: the flat
`v[k + 4]` form costs a word per loop).
