# Viewport__OnNotify — MATCHED

> Renamed from `Unk18Obj__OnNotify` on 2026-09-25 (tools/rename.py). Address 0x8003e8b8.

> Renamed from `func_8003E8B8` on 2026-09-23 (tools/rename.py). Address 0x8003e8b8.

Unit: `code_2cc8c_d`. Round 14, runner delta. 44/44 words, full match.

## Signature

```c
void Viewport__OnNotify(Unk18Obj *self, GenericObj *arg1, s32 arg2);
```

`Unk18ObjMethods`'s own `+0x038` slot occupant.

## What it does

Forwards to the inherited BasicClass `slot38`, then dispatches `self`'s
OWN `slot94` or `slot98` depending on `arg1`'s dynamic class tag (5 or 1
respectively, read off its header nibble — same idiom already established
by round 13's `Viewport__AddChild`).

```c
void Viewport__OnNotify(Unk18Obj *self, GenericObj *arg1, s32 arg2) {
    s32 tag;

    Get_vtable_BasicClass()->slot38(self, arg1, arg2);

    tag = arg1->methods->header & 0xF;
    if (tag == 5) {
        self->methods->slot94(self, arg1, arg2);
    } else if (tag == 1) {
        self->methods->slot98(self, arg1, arg2);
    }
}
```

## Self-caught bug: a forgotten pad broke an already-matched round-13 function

Adding `slot94`/`slot98` to `Unk18ObjMethods` (splitting the old
`pad094[0x0A8-0x094]` span) left a forgotten gap: `slot94` (`+0x094`) +
`slot98` (`+0x098`) only account for 8 of the 20 bytes that pad used to
cover, so `slotA8` silently landed at struct offset `+0x09C` instead of its
real `+0x0A8`. Compiled clean; the failure showed up only as a whole-image
SHA1 mismatch with `build exit=2` and no diff in `Viewport__OnNotify` itself.
Localized with the corrected recipe from `CLAUDE.md`'s fifth-way-a-score-
lies-that-wasn't entry (`cmp -l`, 1-based, then `lsdde.map`): the single
differing byte fell inside `Viewport__Finalize` (round 13, alpha — dispatches
through this same `slotA8`). Added the missing `u8 pad09C[0x0A8-0x09C];`
and reconfirmed the whole image is byte-exact before trusting either
function's score. This is the SAME class of bug CLAUDE.md now documents
(a field insertion, not a retype) — recorded here as a second live
instance of exactly the failure mode the doc warns about, not a new
finding.

## Header changes

`include/code_2cc8c.h`: `Unk18ObjMethods` gains `slot94`/`slot98` (both
`void (*)(Unk18Obj*, GenericObj*, s32)`, occupants `Viewport__OnNotifyTag5`/
`Viewport__OnNotifyTag1`, still queued as of this report) plus the corrective
`pad09C` gap described above.

## Naming

`Unk18Obj__OnNotify` -- tier A. Body is a supercall to `Get_vtable_BasicClass()->onNotify` followed by dispatch on the sender's dynamic-class tag nibble (5 -> slot94, 1 -> slot98) -- the exact override shape already established and named for `BasicClass__OnNotify`/`Class6B5CC__OnNotify` (`include/code_8220.h`, `src/code_d294.c`). Matching an adopted, cross-class convention rather than a fresh guess.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__OnNotify`. The +0x038 onNotify override: after the base, a class-5 sender (D_8006EF50) goes to +0x094 onNotifyTag5 and a class-1 sender (DrawSystem) to +0x098 onNotifyTag1. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.
