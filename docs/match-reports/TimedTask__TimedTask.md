# TimedTask__TimedTask — MATCHED (35/35 words)

> Renamed from `Class86668__Class86668` on 2026-09-26 (tools/rename.py). Address 0x8004a19c.

> Renamed from `func_8004A19C` on 2026-09-23 (tools/rename.py). Address 0x8004a19c.

`TimedTaskMethods` slot +0x008: `gTimedTaskMethods`'s own constructor, occupying
the same slot `New_TimedTask` (the `New_X` allocator for this class) calls
as `GetTimedTaskMethods()->ctor(self, arg1, arg2)`.

## Disassembly shape

```
addiu $sp, $sp, -0x20
sw    $s0, 0x10($sp)
addu  $s0, $a0, $zero        ; s0 = self
sw    $s1, 0x14($sp)
addu  $s1, $a1, $zero        ; s1 = arg1
sw    $s2, 0x18($sp)
sw    $ra, 0x1C($sp)
jal   GetIntermediateBaseMethods
 addu $s2, $a2, $zero        ; s2 = arg2
lw    $v0, 0x8($v0)          ; base ctor slot
nop
jalr  $v0
 addu $a0, $s0, $zero        ; base ctor(self) -- only self set up, no other args
jal   GetTimedTaskMethods
 nop
beqz  $s1, .L8004A1F0
 sw   $v0, 0x0($s0)          ; self->methods = vtable  (delay slot -- unconditional)
jal   New_VabStreamObj
 addu $a0, $s1, $zero        ; arg1 != 0: New_VabStreamObj(arg1)
j     .L8004A1F4
 sw   $v0, 0x34($s0)         ; self->subB = result
.L8004A1F0:
sw    $s2, 0x34($s0)         ; else: self->subB = arg2
.L8004A1F4:
lw    $v0, 0x0($s0)          ; self->methods (reload)
sw    $s1, 0x30($s0)         ; self->unk30 = arg1
lw    $v0, 0x40($v0)         ; methods->resetUnk3C slot
jalr  $v0
 addu $a0, $s0, $zero
...
jr $ra
```

## Final C

```c
void TimedTask__TimedTask(Obj865C8 *self, s32 arg1, SubObjB *arg2) {
    GetIntermediateBaseMethods()->ctor(self);
    self->methods = (DayTaskMethods *)GetTimedTaskMethods();
    if (arg1 != 0) {
        self->subB = New_VabStreamObj(arg1);
    } else {
        self->subB = arg2;
    }
    self->unk30 = arg1;
    self->methods->resetUnk3C(self);
}
```

Note: at the point of the last call, `self->methods` has already been
reassigned to `GetTimedTaskMethods()`'s table (`gTimedTaskMethods`), so the runtime
target of `self->methods->resetUnk3C(self)` is `gTimedTaskMethods`'s own +0x040
override (`TimedTask__CancelTimeout`), not `DayTask__ResetPhase`. The FIELD name
(`resetUnk3C`, chosen from `gDayTaskMethods`'s occupant of that slot) still
describes the right SIGNATURE for the shared struct layout; only the actual
function invoked at runtime differs by which vtable `self->methods` points
at. No behavioral ambiguity, just a naming note for the next reader.

## New/changed struct knowledge (`include/dream_day.h`)

- `IntermediateBaseMethods::ctor` added at +0x008: `void *(*ctor)(void
  *self)` — called with only `self` set up, matching the base-ctor shape
  elsewhere in the project (e.g. `BasicClassMethods::ctor` in
  `pad.h`).
- `TimedTaskMethods::ctor` retyped from the placeholder `void *(*ctor)(void
  *self, void *arg1, void *arg2)` to the real signature `void (*ctor)(Obj865C8
  *self, s32 arg1, SubObjB *arg2)`. Void: this function's OWN body never
  materializes a return value in `$v0` before its final `jr $ra` — the
  callee-side reading, independent of `New_TimedTask`'s caller-side
  discarding of the same call (which by itself would NOT be enough evidence,
  per CLAUDE.md's "a discarded return is never evidence of void").
- `TimedTaskMethods` instances documented as sharing `Obj865C8`'s own
  layout: this constructor writes `unk30`/`subB` at exactly the offsets
  `Obj865C8`'s other (gDayTaskMethods-side) functions already use, so no second
  parallel struct was introduced.
- New extern `SubObjB *New_VabStreamObj(s32 arg1)` (uncarved unit
  `code_179d8`): an allocator (0x64 bytes) whose one call site here stores
  the result straight into `Obj865C8::subB`.

## Attempts

1 (matched on first attempt).

### Proposed learning

When a class's constructor reassigns `self->methods` partway through its own
body (as every constructor in this class framework does, right after the
base ctor call), later calls through `self->methods` in the SAME function
resolve to the class's OWN overrides, not whatever slot name was chosen when
that struct offset was first typed from a different (base) table's occupant.
The struct field name is a description of the SLOT LAYOUT, shared correctly
across sibling classes; it is not a promise about which function runs at any
particular call site once a subclass's vtable is installed.

## Naming

`TimedTask__TimedTask` -- tier A. The sibling class's own ctor (+0x008 of `gTimedTaskMethods`), matching the `Class__Class` convention; called by `New_TimedTask`.

## Track 4

2026-09-25, round 84 (bravo): class unified in `include/timed_task.h`. Not renamed. Signature `(TimedTask *self, char *soundBankPath, BasicClass *sound)`; `unk30`/`subB` are `soundBankPath`/`sound`, and the final call is IntermediateBase's `resetCounters` slot (was `resetState`), which this class fills with TimedTask__CancelTimeout. Its first call is IntermediateBase's ctor, and both subclass ctors (DayTask__DayTask, ObjM__ObjM) call this one first, so the id tree's parent links hold. Image byte-identical.

## Track 4 (2026-09-26, round 87, VabStreamObj)

`include/dream_day.h`'s local `extern BasicClass *New_VabStreamObj(char *)`
is deleted. `src/world/dream_day.c` now includes `include/VabStreamObj.h`, where
the allocator returns `VabStreamObj *`, and casts the result to
`BasicClass *` for `TimedTask::sound`. The whole image stays
byte-identical.

## Track 6 (2026-09-26, round 92, charlie): the class is TimedTask

`python3 tools/renametype.py Class86668 TimedTask` renamed the class, its
table (`gClass86668Methods` -> `gTimedTaskMethods`), its getter, allocator,
ctor and every `Class86668__` method; image byte-identical. Tier B.

Evidence. What this class adds to IntermediateBase is, from its own
methods: `timeoutFrames` (SetTimeout stores n * 20, a negative n as is),
CheckTimeout (the update override: base update, then setState(4) once
frameCounter passes timeoutFrames unsigned), SetState (for 4: result = 1,
onState4), Init (zero `result`, base init, return `result`), and a `sound`
object that PlaySound plays tones on. The timeout ending the job with a
result is the one behaviour no sibling (TaskCore) or parent defines in this
form, so it names the class. Tier B, not A: the only setTimeout call in the
game is CancelTimeout's `setTimeout(-1)`, so the timeout is never armed and
its purpose in play is not observed; the sound half is shared with TaskCore
and does not distinguish it. Rejected: a "dream"/"day" name (both
subclasses run the dream day, but that is what the subclasses do, not this
class).

Moved here from the header banner (derivation, not documentation):
DayTask passes `GetSoundEffectDir()` as soundBankPath; ObjM passes a null
path and DayTask's own sound, so only DayTask's copy releases it.
PlaySound is TaskCore__PlaySound's shape with 0x7F, 0x7F. ObjM calls the
same object's +0x088/+0x08C, VabStreamObj's Mute/Unmute.
`tools/classtable.py` stops at the last non-NULL slot, which is why it shows
the 0x80-byte table as ending at +0x070. GameApplication__RunDayTask switches
on init's return through DayTask; 2 and 3 are DayTask's own codes.

Proposed, not applied (accessors outside this job's units):
- slot +0x074 `slot74` -> `togglePause`: NULL here, its one occupant is
  ObjM__TogglePause and its one caller ObjM__DispatchPadEvent's 0x21 case
  (`src/world/dream_scene.c`).
- field +0x034 `sound` `BasicClass *` -> `struct VabStreamObj *`, with the
  ctor's and New_TimedTask's `sound` parameter: every object that reaches it
  is a New_VabStreamObj, and four units cast it back. Needs objm.h's ctor
  and New_ObjM parameters retyped with it (ObjM passes its own `BasicClass *`
  sound), or the build gains pointer-type warnings where it has none now.
