# DayTask__DayTask — MATCHED (107/107 words)

> Renamed from `Class865C8__Class865C8` on 2026-09-26 (tools/rename.py). Address 0x80049684.

> Renamed from `Obj865C8__Obj865C8` on 2026-09-26 (tools/rename.py). Address 0x80049684.

> Renamed from `func_80049684` on 2026-09-23 (tools/rename.py). Address 0x80049684.

`DayTaskMethods` slot +0x008 (the ctor). The largest function in this
unit's round, and the one the head's queue flagged as most likely to leave a
residue. It didn't — full match on the second attempt (one instruction: a
signed/unsigned comparison mismatch).

## Disassembly shape

```
addu  $a0, $zero, $zero
sw    $ra, 0x34($sp)
jal   GetSoundEffectDir
 sw   $s0, 0x20($sp)
jal   GetTimedTaskMethods
 addu $s0, $v0, $zero          ; s0 = GetSoundEffectDir(0)'s return
addu  $a0, $s1, $zero
addu  $a1, $s0, $zero
lw    $v0, 0x8($v0)            ; gTimedTaskMethods's own ctor slot
jalr  $v0
 addu $a2, $zero, $zero        ; GetTimedTaskMethods()->ctor(self, s0, NULL)
jal   GetDayTaskMethods
 nop
jal   InitDreamAux
 sw   $v0, 0x0($s1)            ; self->methods = GetDayTaskMethods() (== &gDayTaskMethods)
lui   $a0, %hi(sEtcTimPath)
addiu $a0, $a0, %lo(sEtcTimPath)
jal   New_TimImage
 nop
sw    $v0, 0x44($s1)           ; self->unk44 = New_TimImage("ETC\ETC.TIM")
lw    $v1, 0x0($v0)
nop
lw    $v1, 0x78($v1)
jalr  $v1
 addu $a0, $v0, $zero          ; self->unk44->methods->slot78(self->unk44), discarded
lw    $a0, 0x44($s1)
nop
lw    $v0, 0x0($a0)
nop
lw    $v0, 0x5C($v0)
jalr  $v0
 nop                            ; self->unk44->methods->slot5C(self->unk44), discarded
addiu $a0, $sp, 0x10            ; &req
lui   $v0, %hi(sDreamerTmdPath)
addiu $v0, $v0, %lo(sDreamerTmdPath)
sw    $zero, 0x10($sp)          ; req.type = 0
jal   New_LinkResource
 sw   $v0, 0x14($sp)             ; req.path = "ETC\DREAMER.TMD"
addu  $a0, $zero, $zero
jal   PickSoundBank
 sw   $v0, 0x48($s1)              ; self->unk48 = New_LinkResource(&req)
addu  $a0, $v0, $zero
addu  $a1, $zero, $zero
jal   New_WBgm
 ori  $a2, $zero, 0x1               ; New_WBgm(PickSoundBank(0), 0, 1)
ori   $a0, $zero, 0x1
jal   RegisterRecordTableFiles
 sw   $v0, 0x40($s1)                 ; self->unk40 = New_WBgm(...)
sltiu $a0, $s4, 0x1                   ; a0 = (unsigned)(arg3 < 1)   -- RegisterRecordTableFiles(1)'s own return discarded
ori   $a1, $zero, 0x1
jal   SetActiveDataSourceDriverMode
 ori  $a2, $zero, 0x1                   ; SetActiveDataSourceDriverMode(arg3<1u, 1, 1), discarded
jal   New_NodeGuardedViewport
 sw   $s3, 0xC($s1)                       ; self->unk0C = arg1
jal   New_FrameClock
 sw   $v0, 0x10($s3)                       ; arg1->unk10 = New_NodeGuardedViewport()
addu  $a0, $zero, $zero
ori   $a1, $zero, 0x1
jal   New_StageMap
 sw   $v0, 0x8($s3)                          ; arg1->unk8 = New_FrameClock()
sw    $v0, 0xC($s3)                           ; arg1->unkC = New_StageMap(0, 1)
lw    $v0, 0x0($s1)
addu  $a0, $s1, $zero
sw    $s2, 0x38($s1)                            ; self->unk38 = arg2
lw    $v0, 0x10($v0)
jalr  $v0
 addu $a1, $s2, $zero                              ; self->methods->slot10(self, arg2)
lw    $v0, 0x0($s2)
lw    $a1, 0x34($s1)                                ; self->subB
lw    $v0, 0x10C($v0)
jalr  $v0
 addu $a0, $s2, $zero                                 ; arg2->methods->slot10C(arg2, self->subB)
lw    $v0, 0x0($s2)
lw    $a1, 0x44($s1)                                    ; self->unk44
lw    $v0, 0x114($v0)
jalr  $v0
 addu $a0, $s2, $zero                                    ; arg2->methods->slot114(arg2, self->unk44)
lw    $v0, 0x0($s1)
nop
lw    $v0, 0x40($v0)
jalr  $v0
 addu $a0, $s1, $zero                                     ; self->methods->resetUnk3C(self)
...
jr $ra
```

## Final C

```c
void DayTask__DayTask(Obj865C8 *self, Obj0C *arg1, SubObjD *arg2, s32 arg3) {
    LoadRequest req;
    s32 tmp;

    GetTimedTaskMethods()->ctor(self, GetSoundEffectDir(0), 0);
    self->methods = GetDayTaskMethods();
    InitDreamAux();
    self->unk44 = New_TimImage(sEtcTimPath);
    self->unk44->methods->slot78(self->unk44);
    self->unk44->methods->slot5C(self->unk44);
    req.type = 0;
    req.path = sDreamerTmdPath;
    self->unk48 = New_LinkResource(&req);
    tmp = PickSoundBank(0);
    self->unk40 = New_WBgm(tmp, 0, 1);
    RegisterRecordTableFiles(1);
    SetActiveDataSourceDriverMode((u32)arg3 < 1, 1, 1);
    self->unk0C = arg1;
    arg1->unk10 = New_NodeGuardedViewport();
    arg1->unk8 = New_FrameClock();
    arg1->unkC = (SubObjG *)New_StageMap(0, 1);
    self->unk38 = arg2;
    self->methods->slot10(self, (Obj4C *)arg2);
    arg2->methods->slot10C(arg2, self->subB);
    arg2->methods->slot114(arg2, self->unk44);
    self->methods->resetUnk3C(self);
}
```

## The one residue and how it closed (2 attempts)

Attempt 1 wrote the comparison as `arg3 < 1` on the plain `s32` parameter,
which compiles to `slti` (signed). Retail uses `sltiu` (unsigned) — same
opcode-family residue already seen this round on `TimedTask__CheckTimeout`. Cast the
comparison's operand: `(u32)arg3 < 1`. 106/107 -> 107/107, no other change
needed anywhere in this 107-word function.

## Shape notes

- This ctor delegates to the SIBLING class's own ctor (`GetTimedTaskMethods()->
  ctor`, i.e. `TimedTask__TimedTask`) for shared base construction FIRST — same
  "override calls base impl via the other table" pattern already documented
  for the dtor (`DayTask__Finalize`) and several slot forwarders this round —
  then immediately re-asserts `self->methods = GetDayTaskMethods()` (`&
  gDayTaskMethods`, this class's REAL vtable), overwriting what the delegated
  ctor had just set to `&gTimedTaskMethods`. Two `self->methods` writes in one
  function, back to back, both legitimate.
- `arg1` (this function's own 2nd parameter, `Obj0C *`) is both stored
  wholesale into `self->unk0C` AND has its OWN sub-fields (`unk8`, `unkC`,
  `unk10`) populated in this same function — the constructor is
  simultaneously wiring up `self` and finishing construction of an object
  `self` doesn't own outright (passed in already allocated by the caller).
- `arg2` (`SubObjD *`) is stored into `self->unk38` (its established type
  from `DayTask__Deinit`/`DayTask__Init` this round) AND separately passed to
  `self->methods->slot10`, whose established signature (from
  `DayTask__StartObjM`, an EARLIER round) expects `Obj4C *` — a different type
  entirely. Both usages are of the SAME raw pointer value (just a
  register-forwarded call argument on the `slot10` side, never
  dereferenced there), so a `(Obj4C *)` cast at that one call site
  reconciles the two established types without disturbing either.
- Six independent "New_X"-shaped allocator calls (`New_TimImage`,
  `New_LinkResource`, `New_WBgm`, `New_NodeGuardedViewport`, `New_FrameClock`,
  `New_StageMap`) populate six different fields (`self->unk40/44/48`,
  `arg1->unk8/unkC/unk10`) that this unit's OWN dtor (`DayTask__Finalize`,
  earlier this round) already established as a uniform `SubObjG` family via
  `->methods->slot4`. These are almost certainly six DIFFERENT real
  classes under the hood (`New_StageMap` is independently and fully typed
  in `DayTaskStageMap.h` as returning `StageMap *`, a much richer type with
  its own documented `ctor`/`slot38`/`slot40`/`slot80`/`slotD0`) — `SubObjG`
  is this unit's own minimal, deliberately-unified LOCAL view (only the
  slots this unit's own functions actually reach: `slot4`, and now
  `slot5C`/`slot78` added here), not a claim that all six are the same
  class. Casts at the `New_StageMap` call site reconcile the two
  independent local views, per this project's established
  multiple-local-views convention.
- Two calls (`New_NodeGuardedViewport`, `New_FrameClock`) and one more
  (`InitDreamAux`) are invoked with NO argument-loading instructions
  immediately before their `jal` at all — confirmed genuinely zero-argument
  by reading each callee's OWN prologue (`New_NodeGuardedViewport`/`New_FrameClock`
  are `New_X` allocators whose own ctor dispatch passes only the freshly
  allocated `self`, no forwarded arguments; `InitDreamAux` is
  independently established elsewhere as `void InitDreamAux(void)`,
  itself a documented STALL in `dream_aux` unrelated to this unit).
  Reading the CALLEE's prologue was the right move here, same lesson
  `DayTask__StartObjM` used earlier this round for the opposite question (an
  argument that looked unused turning out to be real).

## New struct/extern knowledge (`include/DayTaskStageMap.h`)

- `SubObjGMethods` extended with `slot5C` and `slot78` (both `void
  (*)(SubObjG *self)`, return discarded at both call sites here).
- `SubObjDMethods` extended with `slot10C` (`void (*)(SubObjD *self,
  SubObjB *arg1)`) and `slot114` (`void (*)(SubObjD *self, SubObjG
  *arg1)`).
- New local type `LoadRequest` (`{ s32 type; const char *path; s32 unk08;
  s32 unk0C; }`, 0x10 bytes) — this unit's own local view of the SAME
  request-block shape `GameApplication.h` already established as
  `LoadModelRequest` at a different unit's ctor (`GameApplication__GameApplication`); reused
  here rather than cross-including that header, per the per-unit-view
  convention. Declared 0x10 bytes (4 fields) even though only the first two
  are written, per that report's own hard-won lesson ("local struct SIZE
  matters, not shape").
- New rodata externs `sEtcTimPath` ("ETC\ETC.TIM") and `sDreamerTmdPath`
  ("ETC\DREAMER.TMD"), both `const char[]`.
- Nine new function externs, several deliberately re-declared locally with
  a DIFFERENT (but ABI-compatible) type than an existing declaration
  elsewhere in the project (`New_LinkResource`, `RegisterRecordTableFiles`,
  `SetActiveDataSourceDriverMode`, `New_StageMap`) — all four already have an extern
  somewhere else (`GameApplication.h` or `GameApplicationFileResource.c`); this unit keeps its
  own local view rather than cross-including, per established policy. The
  other five (`GetSoundEffectDir`, `InitDreamAux`, `New_TimImage`,
  `PickSoundBank`, `New_WBgm`, `New_NodeGuardedViewport`, `New_FrameClock`) are
  new to the project entirely (the first is a genuine one-off; the rest
  come from uncarved Psy-Q segments or `DayTaskStageMap`).

## Attempts

2 (see residue above).

### Proposed learning — direct answer to the head's question

**Still no instance of "field name describes layout, not which function
runs once `self->methods` is reassigned"** in the narrow sense the head's
question asks about (a LATER call through `self->methods` resolving to a
DIFFERENT function BECAUSE of an earlier reassignment within the same
function) — the two `self->methods` writes here don't interact with each
other that way; nothing calls through `self->methods` AGAIN until well
after both writes have settled to the FINAL value (`&gDayTaskMethods`,
`GetDayTaskMethods()`'s return), so every actual dispatch through
`self->methods` in this function sees the same, final table. Six functions
into this round, reporting negative consistently. The one confirmed
instance remains `TimedTask__TimedTask`'s ctor from the earlier round (where a
call through `self->methods` genuinely DOES happen between the
reassignment and the read, resolving to the overridden implementation).

**A sharper statement of when the lever WOULD apply, worth carrying
forward:** the risk is real only when a dispatch through `self->methods`
sits BETWEEN two writes to that field within the same function body — not
merely because the field gets written twice. This function is the second
data point ruling out the naive "any double-write is suspect" reading.

### Session summary note

This closes out the round: all 5 of this session's assigned functions
(`DayTask__OnNotify`, `DayTask__OnInit`, `DayTask__Finalize`, `DayTask__AdvancePhase`,
`DayTask__DayTask`) matched, none stalled — including the two the head
expected residues to survive on. `DayTask__AdvancePhase`'s residue (a GCC
switch-case-balancing quirk) took 9 attempts; every other function in this
session matched within 1-2.

## Naming

`DayTask__DayTask` -- tier A. Named by the project's `Class__Class` ctor convention. Its body constructs the base, installs `gDayTaskMethods`, and loads two named resources verbatim ("ETC\ETC.TIM", "ETC\DREAMER.TMD") -- evidence from the body alone that this is the class's constructor.

## Track 4 (2026-09-26, round 88)

TimImage is unified (`include/TimImage.h`); `include/DayTaskStageMap.h`'s local
`extern SubObjG *New_TimImage(const char *)` is deleted. `unk44` is a
TimImage but keeps Obj865C8's own `SubObjG *` type (Obj865C8's view is that
class's job): the result is cast to `SubObjG *`, and the two calls cast
`unk44` back to `TimImage *` -- +0x078 (FileResource's `void *slot78`,
occupant TimImage__Upload) through `TimImageUploadFn`, +0x05C is
`freeBuffer`. Image byte-identical.

## Track 4 (2026-09-26, round 88, delta: FrameClock)

`New_D8006EF50` is now `New_FrameClock` (include/FrameClock.h), returning `FrameClock *`; the store into `Obj0C::unk8` (`SubObjG *`, field unchanged) upcasts it, and DayTaskStageMap.h's own `SubObjG *` extern of it is gone. Byte-identical.

## Track 4 (2026-09-26, round 88, DayTask)

The class (table D_800865C8, id 0x1F230, TimedTask's subclass) is unified as DayTask in include/DayTask.h; the Obj865C8/DayTaskMethods views in DayTaskStageMap.h are gone. Renamed from Obj865C8__Obj865C8. Accessors now use the parent's names: unk0C -> initArgs, Obj0C::unk8/unkC/unk10 -> IntermediateBaseInitArgs unk8/unkC/viewport, subB -> sound, slot10 -> addChild, resetState -> resetCounters. Own fields named from this body: dreamSys (+0x038, arg2), etcTim (+0x044, New_TimImage("ETC\ETC.TIM"), TimImage *), dreamerTmd (+0x048, New_LinkResource("ETC\DREAMER.TMD")), bgm (+0x040, New_WBgm, WBgm *; the WBgm runner's proposal in New_WBgm.md). Byte-identical.

## Track 6 (2026-09-26, round 94, alpha): the class name

`Class865C8` (the table's address, D_800865C8, kept through tracks 4 and 5:
the header was unified in round 88 and said "a day's loop but not enough to
name it") is now `DayTask`, with `python3 tools/renametype.py Class865C8
DayTask` (the family: DayTaskMethods, DayTaskInitFn, DAYTASK_FIELDS/SLOTS,
gDayTaskMethods, New_DayTask, GetDayTaskMethods and the twelve methods).
Tier B. Evidence, all from this class's own bodies and its one caller:

- every exit brackets one DreamSys `startDay`/`endDay` pair: AdvancePhase's
  phase 1 calls startDay and starts an ObjM on the stage it returns (or, on
  a refusal, endDay(0) and ends); every path of OnObjMNotify that ends the
  task calls endDay first;
- between the two it runs ObjM children, replacing one with a fresh ObjM on
  `getCurrentStage` (phase 3) on ObjM's events 5..8 and 0xA;
- `result` (init's return) encodes how the day ended, and its one caller
  (GameApplication__RunDayTask, reached from Application__RunMainLoop on
  GraphRoom status 2) acts on it: 2 plays the cinematic, 3 sets
  skipGraphRoomPoll.

It is tier B, not A, because the "day" reading rests on DreamSys's
startDay/endDay, which are FirecatFG's names (tier-B hypotheses by rule).
The `Task` suffix follows the project's other IntermediateBase jobs run to
a result (StreamTask) and its parent, TimedTask.

The unit banner (include/DayTaskStageMap.h) no longer carries "track 4, round
88/89"; that history is this section and the Track 4 sections above.

## Track 6 (round 96, delta): the request local is ResourceSourceRequest

include/DayTaskStageMap.h `LoadRequest`, `{ s32 type; const char *path; s32 unk08; s32 unk0C; }`, is the same
0x10-byte record as DayTaskStageMap.c's and the third caller's: the body writes
`type = 0` (ResourceSource's NULL `buffer`: no buffer to adopt) and `path`
(its `name`: the file to request) and passes it to New_LinkResource. It
retired onto include/FileResource.h's `ResourceSourceRequest` (a
`ResourceSource src` then 8 bytes of padding; tier A: the fields are the
ctor's own descriptor, read by LinkResource__LinkResource as `src->buffer`
and `src->name`). The body now writes `req.src.buffer = NULL` and
`req.src.name` and passes `&req.src` with no cast; byte-exact. The 0x10
size is kept (8 changes the frame, measured on StageMap__PopulateSlotCells).

## Track 6 (round 97, alpha): the request local is ResourceRequest

`ResourceSourceRequest` is deleted. The local is now `ResourceRequest req;`
(include/FileResource.h, 0x0C) with `mode` left unset; the body is
unchanged (`req.src...` writes, `&req.src` to New_LinkResource). A plain
`ResourceSource` (8 bytes) was measured to shrink this function's frame by
8 and move every callee-save slot, and an unused pad local is dropped by
cc1, so ResourceRequest is the smallest existing type that keeps the frame.
Byte-exact. Table and details: ResourceRequest__Set.md, round 97 second job.

## History moved from comments (track 7, round 99, charlie)

The unit banner used to live in `include/class_39e08.h`; it now sits at the
top of `src/class_39e08.c`, and the header's own comment says only what the
header declares. Text the header carried until round 99, kept here:

- Banner: "Unit class_39e08: the methods of two classes, in ROM order.
  DayTask (gDayTaskMethods, 0x1F230), New_DayTask through GetDayTaskMethods
  ... TimedTask (gTimedTaskMethods, 0x230), its parent, New_TimedTask through
  TimedTask__SetTimeout ... NoOpSlot58, CheckTimeout, SetState and SetTimeout
  are TimedTask's own methods that DayTask inherits unchanged. plus
  func_8004A070 [now RegisterRecordTableFiles], called once from DayTask's
  ctor and once from code_1677c: it manages a pair of file-scope globals and
  loops on RegisterFileTableEntries; nothing pins down what it registers.
  What stays here are the call-site views of objects this unit reaches
  without a unified class to type them ... ObjM (DayTask::objM) is
  include/ObjM.h."
- SubObjE: "Opaque view of whatever object DayTask__OnInit reaches through
  IntermediateBaseInitArgs::unk0" (that field is `drawSystem` now, and
  the slot is the DrawSystem's getDims: DayTask__OnInit.md).
- BMemPMgrAlloc: "see code_171e0.h / code_55dd4.h / Entity.h /
  class_16334.h for the other units that also declare it locally."
- InitDreamAux: "Matched in code_4cd08.c (still called `InitDreamAux` there,
  STALLED at 40/56 -- see docs/match-reports/InitDreamAux.md)". Stale: that
  report records the match at 56/56 in round 24.
- GetSoundEffectDir: "MATCHED, src/code_39094.c (`char
  *GetSoundEffectDir(void)`)". The extern now returns `char *` like the
  definition, which dropped the ctor's `(char *)` cast; the dead `0`
  argument stays, as its `arity-ok` note says.
- sEtcTimPath/sDreamerTmdPath (then D_800113EC/D_800113F8): "Filenames right
  next to each other in the same rodata blob (asm/data/1A90.rodata.s)". The
  blob is `asm/data/1B84.rodata.s` today.
- RegisterRecordTableFiles: "Also declared in code_1677c.c as `extern s32
  func_8004A070(s32 a0)`." SetActiveDataSourceDriverMode: "Also declared in
  src/code_1677c.c with this exact signature. Return value discarded at this
  call site." "New_StageMap is declared in include/StageMap.h."

## Naming (track 7, round 99, charlie)

- Parameter `arg3` -> `syncDriver` (tier B). Its only use is
  `SetActiveDataSourceDriverMode(syncDriver == 0, 1, 1)`, whose first
  parameter is SetCdDriverMode's `async`: a nonzero value selects a
  synchronous driver mode. The caller passes
  `GameApplication::config->unk04`, whose meaning is not established.
  `(u32)syncDriver < 1` was m2c's spelling of `== 0`; the plain form is
  byte-identical (verified).
- Local `tmp` -> `vabPath`: PickSoundBank's word is a VAB path, and it is
  New_WBgm's `vabPath`. Kept `s32`, as PickSoundBank returns it.
- `D_800113EC` -> `sEtcTimPath` ("ETC\\ETC.TIM") and `D_800113F8` ->
  `sDreamerTmdPath` ("ETC\\DREAMER.TMD") (tier A: the rodata strings
  themselves, in `asm/data/1B84.rodata.s`, loaded into `etcTim` and
  `dreamerTmd`). Unit-static: no other code reads them.
- DreamSys +0x114 `slot114` -> `setEtcTim`, the name DreamSys__SetEtcTim.md
  proposed; this unit holds the slot's only accessor.

## History (moved from include/DayTaskStageMap.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
/* arity-ok: the definition takes no parameter and reads no argument register, but this dead argument IS byte-load-bearing -- retail emits `move a0,zero` at 0x800496A8 ahead of the jal at 0x800496B0 */
```
