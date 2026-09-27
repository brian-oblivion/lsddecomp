# ObjM__InitStyleAndWorld -- MATCHED (137/137 words)

> Renamed from `func_80052F10` on 2026-09-24 (tools/rename.py). Address 0x80052f10.

Unit `src/class_3bb8c_l.c`. Round 26, runner delta.

## What it is

An `Obj87034_3bb8c_l` initialization/setup routine (its own table slot is
`+0x04C` per `tools/classtable.py 0x80087034`). Fires a chain of dispatches
through `self->unk18` (a `DreamSysObj_3bb8c_l*`), `self->unk54` (another
`Obj87034_3bb8c_l*`), `self->unk3C` (`DreamSysObj_3bb8c_l*`) and
`self->unk14` (`Obj14_3bb8c_l*`), sets up several state fields, and ends by
notifying `self->unk3C` and marking `self->unk20 = 5` (a phase tag this
unit's other functions also write with 4/5/6 -- see
`docs/match-reports/ObjM__OnDreamSysNotify.md` and the header's own comment on it).

```c
extern s32 PickVariant(void *arg0, s32 arg1);
extern s32 PickDailyVariant(void *arg0, s32 arg1, s32 arg2);
extern s32 New_TimBlockSrc(s32 arg0);
extern void func_8001EF60(s32 arg0);
extern s32 RegisterStyleConfig(void *arg0, s32 arg1, s32 *arg2, s32 arg3, s32 arg4);

extern s32 D_8008715C;
extern s32 D_80087168;
extern s32 gStagePendingExtras[];
extern s32 D_80087150;

void ObjM__InitStyleAndWorld(Obj87034_3bb8c_l *self, s32 arg1, StyleConfig *arg2, s32 arg3) {
    DreamSysObj_3bb8c_l *unk18 = self->unk18;
    s32 ret1;
    s32 flag;

    unk18->methods->slot74(unk18);
    self->unk60 = 1;
    ret1 = PickVariant(self->unk38, 0);
    self->unk54->methods->slot5C(self->unk54, ret1);

    ret1 = self->unk3C->methods->slot1A0(self->unk3C, 0);
    ret1 = PickDailyVariant(self->unk38, 0, ret1);
    self->unk58 = (Obj87034_3bb8c_l *) New_TimBlockSrc(ret1);

    unk18->methods->slot70(unk18, self->unk3C, &D_8008715C, &D_80087168, 0);

    self->unk78 = unk18;
    ret1 = self->unk3C->methods->slot1A0(self->unk3C, 0);
    self->unk50 = (StyleConfig *) RegisterStyleConfig(self->unk14, self->unk38, &self->unk6C, ret1, 0);
    if (arg2 != 0) {
        self->unk50 = arg2;
    }

    self->unk4C = arg3;
    if (self->unk38 != 0) {
        s32 unk38val;
        s32 three;

        unk38val = (s32) *(void * volatile *) &self->unk38;
        self->unk40 = 0x10;
        three = 3;
        __asm__("");
        flag = (unk38val == 5);
        if (unk38val == 6) {
            flag = 1;
        }
        self->unk44 = three;
        if (unk38val == three) {
            flag = 1;
        }
        self->unk14->methods->slot134(self->unk14, 0);
    } else {
        self->unk40 = 0x10;
        self->unk44 = 2;
        flag = 1;
        self->unk14->methods->slot134(self->unk14, &D_80087150);
    }

    self->unk48 = arg1;
    if (arg1 == 0) {
        self->unk48 = 0xA000;
    }
    func_8001EF60(flag);

    self->unk3C->methods->slotEC(self->unk3C, gStagePendingExtras[(s32) self->unk38]);
    self->unk20 = 5;
}
```

Cross-unit helpers (`PickVariant`, `PickDailyVariant`, `New_TimBlockSrc`,
`func_8001EF60`, `RegisterStyleConfig`) have no established prototypes anywhere
else in the project (all still `INCLUDE_ASM` in their own units), so they
are declared locally per CLAUDE.md's rule. `D_8008715C`/`D_80087168` are
referenced only by address (never loaded), so their real type is unknown;
`gStagePendingExtras` is a plain word array indexed by `self->unk38`;
`D_80087150` is likewise referenced only by address.

## Three levers, in the order that closed the gap (138 -> 137 words)

The naive transcription (each field access written as its own fresh C
expression, matching how many DIFFERENT registers self->unk38 occupies
across the function) compiled to 138 words -- one too many, then dropped
to 135 (two too few) once an unconditional `self->unk4C = arg3;` was
pulled out in front of the `if`. Three fixes, each addressing a genuinely
different residue, closed it:

1. **`self->unk4C = arg3;` had to be a single UNCONDITIONAL statement
   before the `if`, not duplicated inside both arms.** Retail's delay slot
   for the outer `beqz $v0, .L80053090` is `sw $s3, 0x4C($s1)` -- it
   executes on EITHER path, because a delay slot always runs regardless of
   whether the branch is taken. Writing it once, before the `if`, put it
   in exactly that position; duplicating it inside each arm (the more
   "obviously correct"-looking C, matching that only the `unk38 != 0` arm
   conceptually needs it) put it four words later than retail, and cost a
   whole word by itself.
2. **Retail reloads `self->unk38` a second time immediately inside the
   `if`, even though the OUTER test just read the identical field with
   nothing writing it in between -- ordinary CSE would (and, in the first
   few attempts, did) treat the second read as redundant and reuse the
   outer test's register.** This is the SAME class of residue documented
   in `SpuVmAlloc`'s report this round (cross-branch redundant-recompute
   elision) and it resisted the same first attempts that failed there: a
   `goto`-rewrite of the `if`, a bare `__asm__("")` at the top of the
   branch, and casting only the OUTER test. What worked here (and is worth
   recording as a NEW lever for that residue class, since `SpuVmAlloc`
   never found one): reading the INNER occurrence through a
   `(void * volatile *)` cast on the field's ADDRESS --
   `*(void * volatile *) &self->unk38` -- rather than through `self->unk38`
   directly. This is the same idiom `code_179d8_m.c` documents for
   `D_8008EA26` (a volatile-qualified POINTER TYPE at the read site changes
   the load, independent of the pointee's own declared volatility), used
   in the OPPOSITE direction: there it was applied to fold a load that
   would otherwise stay separate; here it FORCES a reload that would
   otherwise be folded away. Same mechanism, reached from the other side.
3. **A bare `__asm__("")` scheduling barrier, to pin `three = 3`'s `li`
   ahead of a later store the scheduler otherwise moved it past.** Pure
   instruction-order difference confirmed by removing it: doing so does
   not change which register holds `three`'s value, only where the `li`
   lands relative to `self->unk40`'s store -- the permitted use per
   CLAUDE.md's test. Source-level statement reordering (tried first, at no
   cost) had NO effect on this scheduling choice, same as observed
   elsewhere this round; only the barrier moved it.
   Also needed: swapping `func_8001EF60(flag);` to AFTER (not before) the
   `self->unk48 = arg1; if (arg1 == 0) { ... }` block -- another pure
   order fix, this one resolved by plain statement reordering (no barrier
   needed).

## Struct edits (all additive; see `ObjM__OnDreamSysNotify.md` for the sibling
edits made in the same session)

- `Obj87034Methods_3bb8c_l`: added `slot5C` (`void (*)(Obj87034_3bb8c_l
  *self, s32 arg1)`), splitting `pad4C[0x074-0x04C]`.
- `DreamSysMethods_3bb8c_l`: added `slot70` (5-argument, the 5th passed on
  the stack -- `void (*)(void *self, void *arg1, void *arg2, void *arg3,
  s32 arg4)`, replacing a pad that was EXACTLY one slot wide already),
  `slotEC` (`s32 (*)(void *self, s32 arg1)`, splitting `padD8[0x0F0-0x0D8]`),
  and `slot1A0` (`s32 (*)(void *self, s32 arg1)`, splitting
  `pad10C[0x200-0x10C]`). `slotF0` and `slot200` (both already used by
  matched code) confirmed to stay at their original offsets after the
  splits.
- `Obj14Methods_3bb8c_l`: appended `slot134` (`void (*)(void *self, void
  *arg1)`) as trailing padding + one slot past the previous end of the
  struct (0x110 -> 0x138) -- append-only, the safest kind of edit since
  nothing indexes this struct by `sizeof`.
- `Obj87034_3bb8c_l` (the instance struct): added `unk48`/`unk4C` (splits
  `pad48[0x050-0x048]` exactly, no leftover padding needed) and
  `unk6C`/`unk78` (splits `pad6C[0x080-0x06C]` into `unk6C` + `pad70[8]` +
  `unk78` + `pad7C[4]`).

Every split's arithmetic was verified by hand before editing, and the
build was re-run after EACH struct edit (not batched) to confirm the
whole-image SHA1 stayed green before writing any of this function's own
body -- per CLAUDE.md's warning that a struct edit is non-local and can
silently break an unrelated already-matched function.

## Proposed learning

**The `(void * volatile *)` / `*(u8 *)&sym` family of idioms is
bidirectional.** `code_179d8_m.c`'s documented use forces a COMPACT fold
(collapsing what would otherwise be two loads into one, by defeating a
volatile object's forced reload through a non-volatile-qualified pointer
type at the read site). This function needed the mirror image: forcing a
SECOND load of an otherwise-ordinary (non-volatile) field, by qualifying
the pointer type at the read site as volatile instead. Worth stating
explicitly in `DECOMPILATION_LEARNINGS.md` next to the existing entry:
the pointer TYPE at the point of dereference, not the pointee's own
declared type, is the lever in both directions, and `SpuVmAlloc`'s
still-open cross-branch-CSE residue this round is a candidate to revisit
with this specific tool now that it has one confirmed win.

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_80052F10` | `ObjM__InitStyleAndWorld` | B | see below |

**Evidence.** vtable slot +0x04C. The larger of the two setup routines: calls `RegisterStyleConfig`, wires the target/world fields (`self->unk78`, `self->unk6C`, `self->unk48/unk4C/unk40/unk44`), dispatches `self->target`'s own `slot134`, and ends by marking `self->phase = 5`.


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the class_3bb8c_k/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and class_39e08.h's Obj4C/SubObjB/EventArg are gone. Byte-identical. The onInit override: `world` was IntermediateBase::viewport (NodeGuardedViewport: detachViewChild, attachViewChild), `unk54` the bgm (WBgm setSeq), `pendingOther` the New_TimBlockSrc object (`timBlockSrc`), `unk38` the stage, `unk48` gridSpan (the StageMap's setGridSpan), `unk14` the StageMap (setBounds); `&ctorSound` is RegisterStyleConfig's third argument.

## asm sites

Round 89 (runner delta, track 5 `asm-sites`): the bare `__asm__("")` after
`three = 3;` is **justified**; its existing site comment already says what it
forces, and a re-measurement confirms it. Deleting it alone turned the image
red (161210 bytes: the function came out one word shorter and everything after
drifted), `funcdiff` 74/136, and asm-differ shows `li a0,0x3` moved from above
`sw v0,0x40(s1)` (the `self->unk40 = 0x10` store) to below the `stage` compare,
where it displaces `move a1,zero` and pulls `sw a0,0x44(s1)` along with it.
No source change.

## Round 94 (track 6, charlie)

`Unk50Struct_3bb8c_l` is `StyleConfig` (ObjM::styleConfig's pointee,
include/class_3bb8c.h), its fields named from their readers: `unk0`/`unk4`
-> `lightDirs`/`lightColors` (the StageMap's setChildParams), `unk8` ->
`ambientColor` (setAmbientColor), `unkC` -> `clearColor` (the viewport's
setClearColor), `unk14` -> `colorMode` (1: the far colour is clearColor; 2:
the TimBlockSrc fades to clearColor), `unk18` -> `farColor` (setFarColor
otherwise), `unk1C` -> `fogNear` (setFogNear). Tier B for `colorMode`, whose
two tested values are read by different methods for different choices.
Zero bytes changed.
