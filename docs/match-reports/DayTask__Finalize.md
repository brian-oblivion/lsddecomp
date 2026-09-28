# DayTask__Finalize — MATCHED (74/74 words)

> Renamed from `Class865C8__Finalize` on 2026-09-26 (tools/rename.py). Address 0x80049830.

> Renamed from `Obj865C8__Dtor` on 2026-09-26 (tools/rename.py). Address 0x80049830.

> Renamed from `func_80049830` on 2026-09-23 (tools/rename.py). Address 0x80049830.

`DayTaskMethods` slot +0x00C (the dtor).

## Disassembly shape

```
addiu $sp, $sp, -0x20
sw    $s1, 0x14($sp)
addu  $s1, $a0, $zero        ; s1 = self
sw    $ra, 0x18($sp)
sw    $s0, 0x10($sp)
lw    $v0, 0x0($s1)          ; self->methods
lw    $a1, 0x38($s1)         ; self->unk38
lw    $v0, 0x14($v0)         ; methods->slot14
lw    $s0, 0xC($s1)          ; s0 = self->unk0C   (hoisted early, unrelated to the call below)
jalr  $v0
 nop                          ; self->methods->slot14(self, self->unk38)
lw    $a0, 0xC($s0)          ; a0 = self->unk0C->unkC
nop
lw    $v0, 0x0($a0)          ; a0->methods
nop
lw    $v0, 0x4($v0)          ; methods->slot4
jalr  $v0
 nop                          ; a0->methods->slot4(a0)
lw    $a0, 0x8($s0)          ; a0 = self->unk0C->unk8
sw    $v0, 0xC($s0)          ; self->unk0C->unkC = result
lw    $v0, 0x0($a0)
nop
lw    $v0, 0x4($v0)
jalr  $v0
 nop                          ; a0->methods->slot4(a0)
lw    $a0, 0x10($s0)         ; a0 = self->unk0C->unk10
sw    $v0, 0x8($s0)          ; self->unk0C->unk8 = result
lw    $v0, 0x0($a0)
nop
lw    $v0, 0x4($v0)
jalr  $v0
 nop                          ; a0->methods->slot4(a0)
sw    $v0, 0x10($s0)         ; self->unk0C->unk10 = result
lw    $a0, 0x40($s1)         ; self->unk40
...(same shape)...            ; self->unk40->methods->slot4(self->unk40), result discarded
lw    $a0, 0x48($s1)         ; self->unk48
...(same shape)...            ; self->unk48->methods->slot4(self->unk48), result discarded
lw    $a0, 0x44($s1)         ; self->unk44
...(same shape)...            ; self->unk44->methods->slot4(self->unk44), result discarded
jal   ReleaseDreamAuxModels
 nop
jal   GetTimedTaskMethods
 nop
lw    $v0, 0xC($v0)          ; gTimedTaskMethods's own +0x00C
jalr  $v0
 addu $a0, $s1, $zero        ; GetTimedTaskMethods()->dtor(self)
...
jr $ra
```

## Final C

```c
void DayTask__Finalize(Obj865C8 *self) {
    Obj0C *o = self->unk0C;
    SubObjG *g;

    self->methods->slot14(self, self->unk38);
    g = o->unkC;
    o->unkC = g->methods->slot4(g);
    g = o->unk8;
    o->unk8 = g->methods->slot4(g);
    g = o->unk10;
    o->unk10 = g->methods->slot4(g);
    self->unk40->methods->slot4(self->unk40);
    self->unk48->methods->slot4(self->unk48);
    self->unk44->methods->slot4(self->unk44);
    ReleaseDreamAuxModels();
    GetTimedTaskMethods()->dtor(self);
}
```

## Residue and how it closed (2 attempts)

**Attempt 1** wrote the three "step and store back" lines as
`o->unkC = o->unkC->methods->slot4(o->unkC);` directly (reading `o->unkC`
three times in one expression: once as the call's `self` argument, once to
fetch `->methods`, once as the store target). Two problems, both from the
same root cause:

1. `o = self->unk0C;` was declared/assigned in a SEPARATE statement placed
   AFTER the first call (`self->methods->slot14(...)`) rather than before
   it. Retail loads `self->unk0C` into a register in the delay-adjacent
   slot BEFORE the `slot14` call even happens (same "load early since the
   register is free and needed soon" scheduling `DayTask__OnInit` showed
   earlier this round). My statement order pushed the load after the call,
   and the compiled instruction landed one slot later than retail's,
   shifting everything after it by one word (nop/removed word 8 residue).
2. Reading `o->unkC` three times in one expression made cc1 reload it from
   memory redundantly (2 extra words: a spurious `lw v0,4(v0)` /`nop` pair)
   instead of reusing one register the way retail's hand-scheduled asm
   does.

**Fix:** declare-and-assign `o` in the SAME position as retail's early load
(right after `self`, before the first call), and route each "step" through
an explicit local (`g = o->unkC; o->unkC = g->methods->slot4(g);`) so cc1
only ever reads the struct field once per step. 74/74 on the rebuild.

## New/corrected struct knowledge (`include/dream_day.h`)

- `DayTaskMethods::dtor` (+0x00C) and `::slot14` (+0x014) typed (were
  untyped placeholders / grouped `void *` padding).
- `TimedTaskMethods::dtor` added at +0x00C, typed from this function's own
  `GetTimedTaskMethods()->dtor(self)` call — occupied by `TimedTask__Finalize`
  (already matched), the sibling class's own dtor override.
- New opaque type `SubObjG`/`SubObjGMethods` — a self-consuming "step"
  object: `slot4` takes and returns the same type. Confirmed at SIX
  independent call sites in this one function.
- **Correction to earlier-this-round typings** (see below): `Obj0C::unk8`,
  `Obj0C::unk10`, and `Obj865C8::unk40`/`unk44`/`unk48` were all typed `s32`
  by `DayTask__Init`/`DayTask__StartObjM` (earlier this session), whose own call
  sites only ever forward these fields as opaque register values through a
  vtable call that never dereferences them — consistent with either a
  scalar or a pointer at the time. This function dereferences all five
  directly (`->methods->slot4`), settling it: they are `SubObjG *`. Added
  `Obj0C::unkC` (brand new field, same type, same pattern). Both older call
  sites (`DayTask__Init`'s two `unk8`/`unk10` forwards,
  `DayTask__StartObjM`'s three `unk40`/`unk44`/`unk48` forwards) got explicit
  `(s32)` casts added at their existing call sites — same register value
  either way, confirmed by rebuilding all nine of this unit's matched
  functions together (all still full matches).

## Match reports updated (not replaced) for this correction

- `docs/match-reports/DayTask__Init.md` — `Obj0C::unk8`/`unk10` were
  documented there as "plain scalar register-passthrough"; this function
  proves they are pointers. Report NOT rewritten (still an accurate
  description of THAT function's own call sites); adding a short forward
  pointer to this report instead, since CLAUDE.md's per-function report
  policy makes each report a record of what THAT function established, not
  a place to retroactively rewrite once a later function adds evidence.
- `docs/match-reports/DayTask__StartObjM.md` — same note for
  `unk40`/`unk44`/`unk48`.

(Both updates below, appended as a dated addendum rather than editing the
original derivation, so the original reasoning stays intact and inspectable.)

## Attempts

2 (see residue above).

### Proposed learning — direct answer to the head's question

**Still no instance of "field name describes layout, not which function
runs once `self->methods` is reassigned"** in this function — the dtor's
`self->methods->slot14` call happens BEFORE `GetTimedTaskMethods()->dtor(self)`,
and nothing here reassigns `self->methods` at all (unlike `TimedTask__TimedTask`'s
ctor, which is the one confirmed instance so far, from the earlier round).
Three functions in, reporting negative again as requested.

**A different, recurring lever worth generalizing explicitly:** when a
struct field is read three times in one C expression that both consumes and
overwrites it (`x = x->method(x)`), route it through an explicit local
first (`t = x; x = t->method(t);`). This is not new in kind — CLAUDE.md
already documents "let GCC hoist its own loop invariants" and distinguishes
hand-hoisted locals from letting the compiler do it — but this is a
straight-line (non-loop) instance of the same family: cc1 does not
common-subexpression-eliminate repeated STRUCT FIELD reads within a single
statement as aggressively as retail's source apparently avoided needing to,
and the fix is the same discipline (name the value once, don't re-derive it
from memory each time it's used).

## Naming

`DayTask__Finalize` -- tier A. Releases every owned sub-object (`unk0C`'s own three `SubObjG` fields, `unk40`/`unk44`/`unk48`) via their `slot4` release method, then forwards to the base dtor: a pure teardown leaf, mechanics are its purpose.

## Track 4 (2026-09-26, round 88, DayTask)

The class (table D_800865C8, id 0x1F230, TimedTask's subclass) is unified as DayTask in include/DayTask.h; the Obj865C8/DayTaskMethods views in dream_day.h are gone. Renamed from Obj865C8__Dtor: the +0x00C finalize override (it ends in TimedTask's finalize). SubObjG's slot4 is BasicClass's release, so the six `x = x->methods->slot4(x)` calls are `release`; slot14 is removeChild.
