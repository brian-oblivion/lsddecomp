# UpdateFlashbackLock

> Renamed from `CheckSaveScoreFlag` on 2026-09-27 (tools/rename.py). Address 0x8004d678.

> Renamed from `CheckObj866E8CountFlag` on 2026-09-26 (tools/rename.py). Address 0x8004d678.

> Renamed from `func_8004D678` on 2026-09-22 (tools/rename.py). Address 0x8004d678.

**Unit:** TitleMenuTaskObjF · **Size:** 13 words · **Status:** MATCHED (13/13)

## What it does

Not a `NodeGuardedViewport`/`GridCell` method -- called directly (`jal`) from the
still-uncarved `TitleMenu__RefreshMenu` in `asm/TitleMenuTaskObjF.s`. Given a caller-side
context struct and a result struct, reaches through the context to an
`Obj866E8` instance, checks one of its fields against a large constant
(9999999) and a second field against zero, and writes a 0/1 flag into the
result.

## The C

```c
void UpdateFlashbackLock(Ctx678_3bb8c_c *ctx, Result678_3bb8c_c *out)
{
    Obj866E8 *target = ctx->target;
    s32 flag = 1;

    if (target->unkC > 9999999) {
        flag = (target->unk2F4 == 0);
    }
    out->block[1] = flag;
}
```

## Two Obj866E8 struct extensions (both additive)

- **`unkC` (s32, +0x00C).** Falls inside the existing `pad04[0x54-0x04]`
  range -- split into `pad04[0x0C-0x04]` / `unkC` / `pad10[0x54-0x10]`,
  every existing byte still covered, nothing renumbered.
- **`unk2F4` (s32, +0x2F4).** Past the struct's previously-documented tail
  (`unk1E4` at +0x1E4) -- appended as `pad1E8[0x2F4-0x1E8]` + `unk2F4`,
  with every existing field/pad left untouched. This is the tail extension
  the round's brief anticipated; flagging here per that brief's request in
  case runner alpha's own DayTaskStageMap work touches the same tail.

## New types: Ctx678_3bb8c_c / Result678_3bb8c_c

`UpdateFlashbackLock`'s first argument is NOT `Obj866E8` itself -- it is a
larger, unrelated caller-side struct (only visible from its one caller,
`TitleMenu__RefreshMenu`, which reads its own fields at +0x4C/+0x58/+0xA4/+0xB0
around the call) whose own +0x0BC field is a pointer to the `Obj866E8`
this function actually operates on. This is deliberately NOT the same as
`Obj866E8`'s own +0x0BC (already documented as the embedded `Descriptor10`
value `unkBC`) -- the coincidence in offset is exactly why the round brief
called this out as a name to watch for. The second argument holds, at
+0x018, a pointer to a small block whose word at +0x004 is the flag this
function computes; modeled as `s32 *block` with the write expressed as
`block[1] = flag`.

## The residue: a default value in a delay slot, not a scheduling choice

First attempt (`if (...) flag = A; else flag = B; out->... = flag;`)
reached 10/13 -- three words wrong, all the SAME shape: retail's `ori`,
`sltiu` and final `sw` all target `$a2`, the built version targeted `$v1`
throughout. Textbook "same instructions, one register swapped, whole
tail of the function" residue.

Reading the retail assembly's delay slots directly resolved it per
DECOMPILATION_LEARNINGS' "how to read a one-instruction residue": the
`beqz` branch's delay slot is `ori $a2, $zero, 0x1` -- i.e. retail
pre-loads the flag's DEFAULT value (1) into a fixed register BEFORE
deciding whether to branch, and only the taken-branch case overwrites it.
That is a `s32 flag = 1;` declared with its default value up front,
conditionally overwritten inside the `if`, with NO `else` -- not an
`if/else` assigning both arms explicitly. Switching from the two-armed
form to "declare with default, override in `if`, no `else`" matched
immediately, all three words.

## Proposed learning

When a residue is confined to a **default value with no `else`** (branch
skips straight to a use of the "as-is" value), write it as one variable
initialized at declaration and conditionally overwritten -- not as an
`if { a = X; } else { a = default; }` pair. The two forms are
value-equivalent but the register the compiler settles on for the
whole-function-tail lifetime of the variable can differ between them; this
matches the project's existing `New_GameApplication`/`strcat`
"how a value is mentioned, not how it's computed" finding, but for
*initialization* shape rather than *return* shape.

## Extern arity (round 59)

**Verdict: arity-ok idiom.** `src/TitleMenuTaskObjF.c`'s 3-parameter declaration
stays.

**Callee evidence** (`0x8004D678`, and the matched definition in
`src/TitleMenuTaskObjF.c`): the body reads `$a0` and `$a1`, and *writes* `$a2`
before ever reading it:

```
8004d678:  lw    a0,188(a0)       <- $a0 read
8004d68c:  beqz  v0,8004d6a0
8004d690:  li    a2,0x1           <- $a2 written, not read
8004d69c:  sltiu a2,v0,1
8004d6a0:  lw    v0,24(a1)        <- $a1 read
```

Two real arguments, exactly as the definition
(`void UpdateFlashbackLock(Ctx678_3bb8c_c *ctx, Result678_3bb8c_c *out)`) says.
`$a2` is a local flag, not an argument.

**Why the extern must keep the third parameter.** `TitleMenu__RefreshMenu`'s call site
loads it, and retail emits that load:

```
8004de70:  lw    a1,76(s0)
8004de74:  lw    a2,164(s0)       <- the dead 3rd argument, in retail
8004de78:  jal   8004d678 <UpdateFlashbackLock>
```

`self->unkA4` is a memory load, not a value already in `$a2`, so this is
unambiguous: the instruction exists only because the source passes a third
argument. Reducing the declaration to the definition's two parameters would be
a `too many arguments` error, and dropping the argument from the call site would
delete `lw a2,164(s0)` and break `TitleMenu__RefreshMenu`.

**Declaration sites changed:** none (arity unchanged). `/* arity-ok: ... */`
added to `src/TitleMenuTaskObjF.c:219`. Oracle green.

## Naming

**UpdateFlashbackLock** -- tier B. Free function (not a vtable method --
called directly by `jal` from the still-uncarved `TitleMenu__RefreshMenu`), so named
`VerbNoun`. Mechanics are fully evident: reaches an `Obj866E8` through a
caller-side context struct, compares one field (`unkC`) against a large
literal (`9999999`), and writes a computed 0/1 flag into a result block.
The literal reads as a sentinel (a game-common "treat as unlimited/uncapped"
threshold) but that is a purpose GUESS the rule forbids naming on, so the
name stays mechanical: "check a count against its threshold, produce a
flag" -- not "IsUncapped"/"IsAvailable"/etc. `Ctx678_3bb8c_c`/
`Result678_3bb8c_c` (the two parameter types) are left as-is; they already
name exactly what evidence supports (a caller-side context wrapper and a
result block), and CLAUDE.md's four-ways-a-score-lies precedent (delta,
round 13) is the standing reason not to touch a shared/ambiguous struct
without stronger cause.

## Track 4 (2026-09-26, round 89)

Renamed `CheckObj866E8CountFlag` -> `UpdateFlashbackLock` with tools/rename.py
while unifying StageMap (include/StageMap.h). The old name claimed the
object it reads is a StageMap; it is not. Its one caller,
`TitleMenu__RefreshMenu`, passes its own `self` as `ctx`, so
`ctx->+0x0BC` is `TitleMenu::saveBlock` (include/TitleMenu.h: the DreamSys's
`getSaveBlock` result, `&saveMagic`), and a StageMap is only 0x1E8 bytes
while this reads +0x2F4. From `saveMagic`, +0x00C is DreamSys's
`totalFlasbackUnlockScore` (include/DreamSys.h: saveMagic, currentYear,
currentDay, totalFlasbackUnlockScore), which is the word compared against
9999999. Image byte-identical.

## Round 94 (track 6, charlie): its three views were TitleMenu, TaskCoreTarget and the save block

- `Ctx678_3bb8c_c` (a pointer at +0x0BC) is `TitleMenu`: the one caller,
  TitleMenu__RefreshMenu, passes its own self, and +0x0BC is
  TitleMenu::saveBlock. Deleted; the parameter is `TitleMenu *self`.
- `Result678_3bb8c_c` (a pointer at +0x018) is `TaskCoreTarget`: the caller
  passes `self->target`, and +0x018 is `registrationSlots`, whose NULL
  entries are the slots the cursor stops on. Deleted; the parameter is
  `TaskCoreTarget *target` and the store is
  `target->registrationSlots[1] = (void *)locked` (slot 1 is FLASHBACK).
- `SaveBlock678_3bb8c_c` is renamed `DreamSaveBlock` (include/class_3bb8c.h):
  getSaveBlock returns `&saveMagic`, DreamSys +0x178, so +0x00C is DreamSys
  +0x184 `totalFlasbackUnlockScore` and +0x2F4 is DreamSys +0x46C
  `amountFlashbacksAvailable` (offsets measured with cc1 on
  `offsetof(DreamSys, ...)`). The fields take DreamSys's (inherited,
  tier B) names, typo included, so one grep finds both.

The body reads: FLASHBACK stays locked unless the total unlock score is past
9999999 and at least one flashback is stored. The TitleMenu banner already
said so. Zero bytes changed.

Proposed (not applied, a symbol rename outside this types job):
`CheckSaveScoreFlag` -> `UpdateFlashbackLock` (tier B: the mechanics are
certain, the two DreamSys field names are inherited).

## Naming (round 100, track 7)

**UpdateFlashbackLock** (was `CheckSaveScoreFlag`), tier B, as round 94
proposed: the body sets TitleMenu's FLASHBACK entry
(`registrationSlots[TITLEMENU_FLASHBACK]`) locked unless the save block's
`totalFlasbackUnlockScore` is past 9999999 and `amountFlashbacksAvailable`
is nonzero; its one caller, TitleMenu__RefreshMenu, runs it on every
refresh. Tier B because both DreamSys field names are inherited hypotheses.
The old name described a flag, not what the flag is.

Constants: `FLASHBACK_UNLOCK_SCORE` (9999999, unit-local `#define`; no
other code has the literal) and `TITLEMENU_FLASHBACK` (include/TitleMenu.h's
enum, already defined) for the slot index 1.
