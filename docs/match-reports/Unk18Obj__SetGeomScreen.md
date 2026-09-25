# Unk18Obj__SetGeomScreen — MATCHED

> Renamed from `func_8003F28C` on 2026-09-23 (tools/rename.py). Address 0x8003f28c.

Unit: `code_2cc8c_d`. Round 14, runner delta. 8/8 words, full match.

## Signature

```c
void Unk18Obj__SetGeomScreen(Unk18Obj *self);
```

Not a `D_8006E8E4` vtable slot — called directly by symbol.

## What it does

A thin wrapper forwarding `self` unexamined to a PsyQ library call, return
ignored.

```c
void Unk18Obj__SetGeomScreen(Unk18Obj *self) {
    func_80024B90(self);
}
```

## Header changes

`include/code_2cc8c.h`: new extern `func_80024B90(Unk18Obj *self)` (PsyQ
library, `asm/psyq_GsLinkObject4.s`, not decompiled in this project).

## Naming

`Unk18Obj__SetGeomScreen` -- tier A. Trivial one-line forwarding wrapper to Sony's `SetGeomScreen(self)`; mechanics are the entire function.

## Track 2 screen, round 79

`plan.py` lists this as unnamed for track 2: a game-worded name can shadow
Sony's own function, so every game-style name on an SDK-shaped body is
checked. `config/sdk-in-game.txt` carries a LEAD here (`libgs/gs_106:
GsSetProjection`, shape 1.00), but it does not hold up as identification.
`tools/sdkname.py Unk18Obj__SetGeomScreen` reports the 8-word body
AMBIGUOUS: 18 different Sony functions across unrelated libraries (libgs,
libc2, libcd, libetc, libpress, libsn, libsnd) share this exact
relocation-masked fingerprint, because "call one function with one argument,
return" is a common trivial shape. `libgs/gs_106` (`GsSetProjection`) is
never placed as an object anywhere in this executable, so it supplies no
position evidence either. The function's real neighbours are game code
immediately before (`Unk18Obj__GetTail`, this class's own getter, at
0x8003F25C) and Sony's `libgs/gs_131` (`GsSetRefView2`) immediately after
with zero gap (0x8003F2AC == 0x8003F28C + 8 words) -- neither side is
`gs_106`. This is an ordinary, already-matched (round 14) member of the
`Unk18Obj` setter/getter family, forwarding `self` to the established Psy-Q
call `func_80024B90` (`asm/psyq_GsLinkObject4.s`). No evidence kind reaches
the track 2 bar for `GsSetProjection`; rejected with a `// not SDK:` comment
above the symbols-file entry. Name and body unchanged.
