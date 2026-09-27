# GetSetHitHeightGate -- MATCHED 4/4 words

> Renamed from `func_8001EF60` on 2026-09-27 (tools/rename.py). Address 0x8001ef60.

Unit `code_d294_c`, carved round 13. Reopened round 42 as `gp_rel`-blocked
(the blocker is RESOLVED, see CLAUDE.md); the stub above was never actually
attempted until now.

## Round 44 (echo)

Trivial atomic-swap global: reads `gHitHeightGate`, stores the new value, returns
the old one.

```c
extern s32 gHitHeightGate;

s32 GetSetHitHeightGate(s32 value) {
    s32 old;

    old = gHitHeightGate;
    gHitHeightGate = value;
    return old;
}
```

`gHitHeightGate` is a plain `.sdata` word (`asm/data/7B018.sdata.s`), already
present in `config/gp-symbols.txt`, so the `%gp_rel(gHitHeightGate)($gp)` access
came out for free once the extern was declared with an ordinary `s32` type --
no cast, no struct, no retype needed.

Matched on the first build: **4/4 words**, whole-image SHA1 clean (`build
exit=0`). Zero attempts beyond this one.

## Proposed learning

Nothing new -- this is exactly the "reopened gp_rel stub, first build closes
it" pattern round 43 already documented at scale. Filing it mainly to keep
`tools/progress.py`'s STALL/FRESH bookkeeping accurate and to confirm the
pattern held for a 3rd, unrelated unit.

## Naming (round 50, charlie -- FINISHING-PLAN track 3)

- **KEPT as `GetSetHitHeightGate`. Tier C.** And `gHitHeightGate` keeps its
  placeholder too. What IS known:
  - Mechanically it is a get-and-set of the global `gHitHeightGate`: read the
    old value, store the new one, return the old. That is the shape this
    project spells `GetSet...` (`DreamSys__GetSetScreenShake`,
    `DreamSys__GetSetDreamTimeLimit`), so the FORM of the name is settled
    and only the noun is missing.
  - **The noun is missing because both call sites decline to supply it.**
    The only known reader is `SceneNode__ClassifyAgainstPlanes` (code_d294_b, still a
    documented stall), where `gHitHeightGate == 0 || outWord >= 0x201` gates
    whether a `TmdModel__RaycastFaces` result is accepted -- and `TmdModel__RaycastFaces` is
    unidentified Psy-Q, so what is being accepted is unknown. The only
    known writer is `class_3bb8c_l.c`'s `ObjM__InitStyleAndWorld`, which passes a
    flag it computes as "this stage/mode value is 3, 5 or 6" -- a flag
    whose own meaning that unit does not establish either.
  - Naming it would mean choosing between "a precision/threshold mode", "a
    strict-hit mode" and "a debug gate" on no evidence. Per track 3, a
    wrong tier-A name is worse than `func_`.
  - `gHitHeightGate` is a plain `.sdata` word (`asm/data/7B018.sdata.s`),
    already in `config/gp-symbols.txt`; renaming it is a one-command job the
    moment either call site is understood.

## Naming (round 98, echo -- FINISHING-PLAN track 7)

- **`func_8001EF60` -> `GetSetHitHeightGate`, `D_8008A838` -> `gHitHeightGate`. Tier B.** Round 50 kept both as placeholders because `TmdModel__RaycastFaces` was then unidentified, so what the gate accepted was unknown. It is now matched and documented (src/TmdModel.c): its 4th argument receives `hit.y - box.min.y`, the hit point's height above the face box's minimum y. So in `SceneNode__ClassifyAgainstPlanes` the global, when non-zero, makes the segment pass accept a hit only when that height is `>= 0x201`, the test the corner-edge pass applies unconditionally. That is the mechanism the name states; why stages 0, 3, 5 and 6 want it (the only writer, `ObjM__InitStyleAndWorld`) is not established, hence tier B. The `GetSet...` form is the project's for read-old-store-new-return-old (`GetSetBitField`, `DreamSys__GetSetScreenShake`).

## Round 98 (echo): track 7, moved from src/code_d294_c.c

The source comment was rewritten as documentation; the one it replaced, verbatim (field names as they were then):

```c
/* TIER C -- deliberately still `func_`. Mechanically this is a get-and-set
 * of the global `gHitHeightGate` (read old, store new, return old), the shape
 * the project spells `GetSet...` elsewhere. What the global MEANS is not
 * established, so there is no noun to put in the name: its only known
 * reader is SceneNode__ClassifyAgainstPlanes (code_d294_b), where `gHitHeightGate == 0 || outWord
 * >= 0x201` gates accepting a hit, and its only known writer is
 * class_3bb8c_l.c's ObjM__InitStyleAndWorld, which passes a flag derived from a
 * stage/mode value of 3, 5 or 6. Two call sites, neither naming the thing.
 * gHitHeightGate keeps its placeholder name for the same reason. */
```
