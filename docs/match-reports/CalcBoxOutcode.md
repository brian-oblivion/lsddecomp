> Renamed from `func_8001ECFC` on 2026-09-17 (tools/rename.py). Address 0x8001ecfc.

# CalcBoxOutcode -- MATCHED (44/44 words)

Unit: `code_d294_c` (round 14). A standalone leaf, not yet reached by any
caller in this round's queue -- a Cohen-Sutherland-style "outcode"
computation: tests a point's x/y/z against a box's min/max per axis and
returns a 6-bit flag word. `s32 CalcBoxOutcode(s16 *box, s16 *point)`.

## Final source

```c
s32 CalcBoxOutcode(s16 *box, s16 *point) {
    s32 flags;

    flags = 0;
    if (box[3] < point[0]) {
        flags = 8;
    } else if (point[0] < box[0]) {
        flags = 4;
    }
    if (box[4] < point[1]) {
        flags |= 2;
    } else if (point[1] < box[1]) {
        flags |= 1;
    }
    if (box[5] < point[2]) {
        flags |= 0x20;
    } else if (point[2] < box[2]) {
        flags |= 0x10;
    }
    return flags;
}
```

## Derivation notes

- `box` is read as six `s16`s: `box[0..2]` (min x/y/z) and `box[3..5]`
  (max x/y/z, at byte offsets 6/8/0xA); `point` as three `s16`s (x/y/z at
  byte offsets 0/2/4). No struct type declared for either -- nothing
  ties this leaf to any already-known struct, and inventing one from a
  single unconnected function would be a guess rather than a derivation.
- Each axis is an independent `if`/`else if`: "past the max" sets its
  higher bit outright (`flags = N`, since `flags` is provably `0` at that
  point on the FIRST axis, or ORs on the 2nd/3rd), "before the min" sets
  its lower bit; a point within bounds on that axis sets neither. The
  first axis's FIRST branch reads as a plain assignment rather than an OR
  only because `flags` is still `0` when it fires (verified against the
  disassembly: retail's own `v1 = 0` in the branch's delay slot is
  immediately overwritten by `v1 = 8`, not OR'd) -- axes 2 and 3
  correctly use `|=` since `flags` may already be nonzero by then.
- First-try match, no residue.

No new struct or vtable-slot knowledge; this function stands alone in the
current derivation.

## Naming (round 50, charlie -- FINISHING-PLAN track 3)

- **`func_8001ECFC` -> `CalcBoxOutcode`. Tier A.** Free function, complete
  semantics visible in the body: three independent axis tests against a
  bounding box's low and high corners, each contributing one of two bits
  (x -> 8 past max / 4 before min, y -> 2 / 1, z -> 0x20 / 0x10), returned
  as a 6-bit word. "Outcode" is the standard name for exactly this
  Cohen-Sutherland region code, and the original report already identified
  the shape.
- Corroborated by its caller: `ClipSegmentToBox` (code_d294_b) computes it for
  two points and masks each result with `0xFF`, which is the classic
  outcode segment-vs-box trivial-accept/reject test.
- Parameters already carry the derived types `BoundsBox_d294 *` /
  `Vec3S16_d294 *` from an earlier round; unchanged.
