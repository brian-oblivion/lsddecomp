# DayTask__OnObjMNotify — MATCHED (byte-exact, whole-image `build exit=0`)

> Renamed from `Class865C8__OnObjMNotify` on 2026-09-26 (tools/rename.py). Address 0x80049eb4.

> Renamed from `Obj865C8__OnTag2Notify` on 2026-09-26 (tools/rename.py). Address 0x80049eb4.

> Renamed from `func_80049EB4` on 2026-09-23 (tools/rename.py). Address 0x80049eb4.

Unit: `DayTaskStageMap` · Size: 107 words (0x1AC bytes) · Round 23 (2026-09-07),
head. **Third of the five `REOPENED -- ASSIGNABLE` functions closed this round**
(with `TaskCore__OnPadEvent` and `TaskCore__SetState`).

## History

Filed as blocked-not-attempted on the `addiu_at` construct in its
`jtbl_8001140C` dispatch. Round 21 resolved `addiu_at`; the dispatch reproduces
untouched and nothing here is toolchain-blocked.

## The match

```c
void DayTask__OnObjMNotify(Obj865C8 *self, s32 arg1, s32 arg2) {
    struct SubObjDPos pos;
    s32 result;

    switch (arg2) {
    case 4:
        self->unk4C->methods->slot48(self->unk4C);
        self->unk4C->methods->slot4(self->unk4C);
        result = self->unk38->methods->slot1B8(self->unk38, 0);
        if (result == 0) {
            pos = self->unk38->methods->slot1BC(self->unk38);
            self->unk28 = pos.unk2 < 0 ? 1 : 2;
        } else {
            self->unk28 = 3;
        }
        self->methods->onEventArg(self, 3);
        break;
    case 5:
    case 6:
    case 7:
    case 8:
    case 0xA:
        self->unk3C = 3;
        break;
    case 0xC:
    case 0xD:
        self->unk4C->methods->slot48(self->unk4C);
        self->unk4C->methods->slot4(self->unk4C);
        self->unk38->methods->slot1B8(self->unk38, arg2 != 0xC ? 2 : 1);
        self->unk28 = 3;
        self->methods->onEventArg(self, 3);
        break;
    }
}
```

`arg1` is genuinely unused — same shape as the already-matched sibling
`DayTask__AdvancePhase`, which this function closely parallels. Codes 9 and 0xB map to
the epilogue (no-ops). The two `onEventArg(self, 3)` calls cross-jump into one
site, which falls out of the shape and needed no encouragement.

## The finding worth having: a struct RETURNED BY VALUE reads as a shifted call

The call at `0x80049F78` looks wrong on first read — it passes the *object* as
argument two:

```
lw    $a1, 0x38($s0)      # $a1 = self->unk38   <- the object, in a1
lw    $v0, 0x0($a1)
lw    $v0, 0x1BC($v0)
jalr  $v0
 addiu $a0, $sp, 0x10     # $a0 = &<a local>    <- a stack slot, in a0
```

**That is not an argument-order anomaly, it is GCC 2.6.3's struct return
convention.** A function returning a struct receives a hidden pointer to the
caller's destination as an INVISIBLE FIRST argument, so every real argument
shifts right by one. The source is an ordinary assignment:

```c
pos = self->unk38->methods->slot1BC(self->unk38);
```

Two corroborating details, both needed before writing it that way:

- **The frame proves the local is 8 bytes.** Frame `0x28`: `0x00`-`0x0F` is the
  four-word argument spill area, `$s0`/`$s1`/`$ra` sit at `0x18`/`0x1C`/`0x20`,
  so the local area is exactly `0x10`-`0x17`. A 4-byte local would have let
  `$s0` sit at `0x14`.
- **Only the `s16` at +2 is ever read** (`lh $v0, 0x12($sp)`), and its sign
  selects between two `unk28` codes. So the struct's full shape is NOT
  established, which is why `struct SubObjDPos` is declared in
  `src/DayTaskStageMap.c` and not in `include/DayTaskStageMap.h` — a sibling unit could
  not reuse it unchanged. The header records the return type by name and says
  where the definition lives.

## Struct knowledge added to `include/DayTaskStageMap.h`

- **`SubObjDMethods::slot1BC`** carved out of `pad1BC[0x1E0 - 0x1BC]`. Split is
  additive and preserves the 0x24 total (4 + `pad1C0` of 0x20).
- **`SubObjDMethods::slot1B8` RETYPED `void` -> `s32`.** `DayTask__OnObjMNotify`
  branches on the return value directly off the `jalr` (`bnez $v0`), which is
  positive evidence the slot is non-void.

  **This is the round-7 shared-vtable-slot hazard and it was checked as such,
  not assumed.** The unit's other caller, `DayTask__AdvancePhase`, is already matched
  and DISCARDS the return — exactly the configuration where retyping
  `void` -> `s32` stopped GCC tail-merging two identical discarded calls and
  cost 4 words. Here it came back clean: whole-image SHA1 green, and
  `DayTask__AdvancePhase` (94/94) and `DayTask__StartObjM` (33/33) re-verified individually
  after the retype. The check is per-slot and cannot be reasoned by analogy —
  round 7 had two symmetric slots that needed opposite answers.

## Residue history: two ternaries, both branch polarity, 6 words then 3

The body was structurally exact on the first attempt (58127/58133 in range,
correct length). Both remaining residues were the same thing at two sites, and
neither is a scheduling question:

| attempt | residue | fix |
| --- | --- | --- |
| 1 | `bne` where retail has `beq`, plus the two `ori` immediates swapped | `arg2 == 0xC ? 1 : 2` -> `arg2 != 0xC ? 2 : 1` |
| 2 | the two `ori` immediates swapped at the other site | `pos.unk2 >= 0 ? 2 : 1` -> `pos.unk2 < 0 ? 1 : 2` |

### Proposed learning

**A method call whose FIRST argument is a stack address and whose SECOND is the
object is a struct-returning call, not a strange calling convention.** GCC 2.6.3
returns every struct through a hidden first-argument pointer, so the object
lands in `$a1` and each real argument shifts one register right. Recognise it
by the pair: an `addiu $aN, $sp, <local offset>` in the `jalr`'s delay slot
plus the object one register later. Then size the destination from the FRAME —
the gap between the argument spill area and the first saved register is the
local area, and it gives the struct's size directly. Declaring the slot
`void (*)(Dest *, Obj *)` instead would match the bytes at the call site and be
wrong about the type, which then propagates to every other caller.

**Second: a two-armed ternary's branch polarity is set by writing the condition
whose NEGATION the target branches on.** Retail branching `beq` / `bgez` means
the source condition was `!=` / `< 0` respectively, with the ternary's arms in
the corresponding order — GCC tests the negation and falls through to the first
arm. This cost two attempts here at two sites in one function, and it is
independent per site: fixing one says nothing about the other. Round 23's
runner charlie hit the same class in `Entity__NotifyLinkStage` and reported it needing the
fix applied twice at different nesting levels, so this is now three instances in
one round. **The cheap tell is that only the branch mnemonic and the two literal
immediates differ, with everything else exact** — that pattern is always
polarity and never scheduling.

## Naming

`DayTask__OnObjMNotify` -- tier B. Occupies +0x084, dispatched by `DayTask__OnNotify`'s other tag branch (0x2F230). A `switch` over small integer codes (4, 5-8/0xA, 0xC/0xD) that queries/reconfigures `subD` and sets `eventCode`/`state`; the dispatch shape is clear, the meaning of the tag and its sub-codes is not.

## Track 4 (2026-09-26, round 88)

Obj865C8::unk38 is the game's DreamSys (GameApplicationFileResource passes
GameApplication::dreamSys to New_DayTask), so the SubObjD view is gone and its
slots are DreamSys's: +0x1B8 endDay, +0x1BC getCinematic. The by-value
return this body tests is therefore a CinematicCall {bank, entry}; the
local 8-byte SubObjDPos is replaced by it and `pos.unk2` is `pos.entry`.
Byte-identical (107/107): the "8 bytes" was read off the stack frame, and
the 4-byte struct compiles to the same frame.

## Track 4 (2026-09-26, round 88, DayTask)

The class (table D_800865C8, id 0x1F230, TimedTask's subclass) is unified as DayTask in include/DayTask.h; the Obj865C8/DayTaskMethods views in DayTaskStageMap.h are gone. Renamed from Obj865C8__OnTag2Notify: own slot +0x084, which DayTask__OnNotify calls for a sender whose id & 0xFFFFF is 0x2F230 (gObjMMethods, the ObjM StartObjM builds). Events 4/0xC/0xD end the day through the DreamSys and set TimedTask's `result` then setState(3); 5..8 and 0xA set phase 3.

## Naming (track 7, round 99, charlie)

The cases are ObjM.h's `enum ObjMState` (its banner already names this
function as the reader of each), the results DayTask.h's new `enum
DayTaskResult`, `setState(3)` IntermediateBase's `INTERMEDIATEBASE_STATE_STOP`.
Local `pos` -> `cinematic` (getCinematic's CinematicCall).
