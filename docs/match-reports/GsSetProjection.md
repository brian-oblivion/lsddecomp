# GsSetProjection — MATCHED

> Renamed from `Unk18Obj__SetGeomScreen` on 2026-09-25 (tools/rename.py). Address 0x8003f28c.

> Renamed from `func_8003F28C` on 2026-09-23 (tools/rename.py). Address 0x8003f28c.

Unit: `code_2cc8c_d`. Round 14, runner delta. 8/8 words, full match.

## Signature

```c
void GsSetProjection(Unk18Obj *self);
```

Not a `gViewportMethods` vtable slot — called directly by symbol.

## What it does

A thin wrapper forwarding `self` unexamined to a PsyQ library call, return
ignored.

```c
void GsSetProjection(Unk18Obj *self) {
    func_80024B90(self);
}
```

## Header changes

`include/code_2cc8c.h`: new extern `func_80024B90(Unk18Obj *self)` (PsyQ
library, `asm/psyq_GsLinkObject4.s`, not decompiled in this project).

## Naming

`GsSetProjection` -- tier A. Trivial one-line forwarding wrapper to Sony's `SetGeomScreen(self)`; mechanics are the entire function.

## Track 2 screen, round 79

`plan.py` lists this as unnamed for track 2: a game-worded name can shadow
Sony's own function, so every game-style name on an SDK-shaped body is
checked. `config/sdk-in-game.txt` carries a LEAD here (`libgs/gs_106:
GsSetProjection`, shape 1.00), but it does not hold up as identification.
`tools/sdkname.py GsSetProjection` reports the 8-word body
AMBIGUOUS: 18 different Sony functions across unrelated libraries (libgs,
libc2, libcd, libetc, libpress, libsn, libsnd) share this exact
relocation-masked fingerprint, because "call one function with one argument,
return" is a common trivial shape. `libgs/gs_106` (`GsSetProjection`) is
never placed as an object anywhere in this executable, so it supplies no
position evidence either. The function's real neighbours are game code
immediately before (`GetRootNode`, this class's own getter, at
0x8003F25C) and Sony's `libgs/gs_131` (`GsSetRefView2`) immediately after
with zero gap (0x8003F2AC == 0x8003F28C + 8 words) -- neither side is
`gs_106`. This is an ordinary, already-matched (round 14) member of the
`Unk18Obj` setter/getter family, forwarding `self` to the established Psy-Q
call `func_80024B90` (`asm/psyq_GsLinkObject4.s`). No evidence kind reaches
the track 2 bar for `GsSetProjection`; rejected with a `// not SDK:` comment
above the symbols-file entry. Name and body unchanged.

## Head note, round 79: identified as Sony's GsSetProjection (libgs/gs_106)

Bravo's track 2 pass rejected the `GsSetProjection` lead as `not SDK`; the head
overturned it at merge. The rejection argued that gs_106 is never placed, so it
cannot supply position evidence. But position evidence comes from the
NEIGHBOURS, not from the candidate object: this function is the last word
before the placed libgs run (zero gap to gs_131's `GsSetRefView2` at
0x8003F2AC), and of `sdkname.py`'s 18 exact ties `GsSetProjection` is the only
libgs one. The header prototype agrees: LIBGS.H's `GsSetProjection(long h)`.
The one caller, `Viewport__Update`, passed `self->unk40` through a cast to
`Unk18Obj *`, and hands the same field to `SetFogNear(a, h)` as `h` a few
lines later, so the argument is the projection distance and the method typing
was the misread. No class table holds 0x8003F28C, unlike every other
`Unk18Obj__*` function around it. Bravo's comment also cited `func_80024B90`
as the callee; the body calls libgte's `SetGeomScreen`.

Renamed with `tools/rename.py`, retyped to LIBGS.H's shape (`long h`), and its
prototype moved out of `include/code_2cc8c.h` into `src/code_2cc8c_d.c`,
following round 78's `GsSetNearClip` precedent. Byte-identical.
