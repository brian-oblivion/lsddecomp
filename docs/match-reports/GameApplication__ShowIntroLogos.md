# GameApplication__ShowIntroLogos

> Renamed from `GameApplication__LoadIntroLogoSequence` on 2026-09-27 (tools/rename.py). Address 0x80026170.

> Renamed from `Class6D3C8__LoadIntroLogoSequence` on 2026-09-26 (tools/rename.py). Address 0x80026170.

> Renamed from `func_80026170` on 2026-09-24 (tools/rename.py). Address 0x80026170.

**Unit:** GameApplicationFileResource · **Size:** 57 instructions (0xE4 bytes) · **Status:** MATCHED (57/57 words, whole-image SHA1 green)

## What it does

`GameApplicationMethods` slot `+0x050`. Gated entirely by `self->arg->unk0C != 0`
(the ctor argument's `+0x0C` field, opaque until this function): registers a
"loader" task for `"ETC\ASMKLOGO.TIM"` (`GameApplication__ShowImage`), then builds a
separate "stream" task, initializes it with a filename
(`"ETC\ASMK.STR"`) and a type/format code looked up from a table, starts it,
then registers a second loader task for `"ETC\OSDLOGO.TIM"`.

## Derivation

```
lw   $v0, 0x20($s2)          ; s2 = self; v0 = self->arg
lw   $v0, 0xC($v0)            ; v0 = arg->unk0C
beqz $v0, .L80026238            ; whole body gated on this
 ...
jal  SetActiveDataSourceDriverMode(0, 0, 0)
jal  GameApplication__ShowImage(self, sLogoPathAsmk)     ; "ETC\ASMKLOGO.TIM"
jal  New_StreamTask(0, 0, 0, 0)            ; -> s1 = task (New_X shape, 0xDC bytes)
addiu $a0, $sp, 0x18
jal  GetAsmkMovie                          ; writes 0x31 to local, returns &sAsmkMoviePath
 (delay: s1 = v0, i.e. the PRECEDING call's return = task)
lw   $a0, 0x18($sp)                          ; reload the type code (0x31) GetAsmkMovie just wrote
jal  GetMovieFrameCount(a0=0x31)                    ; halfword lookup in sMovieFrameCounts
 (delay: s0 = v0, i.e. the PRECEDING call's return = streamName, &sAsmkMoviePath)
move $a0, $s1                                    ; a0 = task
move $a2, $s0                                     ; a2 = streamName
lw   $a3, 0x0($s1)                                 ; a3 = task->methods (scratch, to fetch the slot)
li   $v1, 1
sw   $v1, 0x10($sp)                                  ; 5th argument, stack-spilled
lw   $a1, 0x1C($s2)                                    ; a1 = self->unk1C
lw   $v1, 0x44($a3)                                     ; v1 = task->methods->slot44
jalr $v1
 (delay: a3 = v0)                                        ; a3 OVERWRITTEN with GetMovieFrameCount's
                                                            ; return value -- the REAL 4th argument
lw   $v0, 0x0($s1)                                          ; reload task->methods
lw   $v0, 0x4($v0)                                           ; slot4
jalr $v0
 (delay: a0 = s1 = task)
jal  GameApplication__ShowImage(self, sLogoPathOsd)                          ; "ETC\OSDLOGO.TIM"
```

```c
void GameApplication__ShowIntroLogos(GameApplication *self) {
    const char *streamName;
    s32 typeCode;
    s32 typeLookup;
    StreamTask *task;

    if (self->arg->unk0C != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        GameApplication__ShowImage(self, sLogoPathAsmk);
        task = New_StreamTask(0, 0, 0, 0);
        streamName = GetAsmkMovie(&typeCode);
        typeLookup = GetMovieFrameCount(typeCode);
        task->methods->slot44(task, self->unk1C, streamName, typeLookup, 1);
        task->methods->slot4(task);
        GameApplication__ShowImage(self, sLogoPathOsd);
    }
}
```

## Two things that were not obvious from a first read of the disassembly

**1. The delay slot after `jalr` is not a scratch reload — it is the real
4th argument, and it overwrites what looks like the argument in the
preceding instructions.** `lw $a3, 0x0($s1)` (task->methods) is used to
*compute the call target* (`v1 = task->methods->slot44`), but by the time
the `jalr` actually transfers control, its own delay slot (`move $a3, $v0`)
has already replaced `$a3` with `GetMovieFrameCount`'s return value. A naive read
sees "`a3` = task's own vtable pointer, passed as an argument" and that
reading is wrong — the vtable pointer was only ever a scratch value for the
indirect call computation, immediately clobbered before the callee sees it.
The first C attempt (discarding `GetMovieFrameCount`'s return and passing
`task->methods` as the 4th argument) built and even scored 44/57 — plausible
enough to be mistaken for a near-miss — before the diff against retail's
actual delay-slot value (`move a3,v0` where `v0` is the *call's* return, not
a memory load) exposed the real data flow. Always check what the delay slot
*is* a move of, not just that a move exists there.

**2. Register (s0 vs s1) assignment for two callee-saved temporaries tracked
declaration order, not statement/liveness order.** `task` (from
`New_StreamTask`) and `streamName` (from `GetAsmkMovie`) are both live
across further calls, `task` for longer (reloaded twice more). The first
attempt declared `task` before `streamName` and code assigned `task` first
(chronologically first live); GCC put `task` in `s0` and `streamName` in
`s1` — backwards from retail (`s1`=task, `s0`=streamName). Simply reordering
the **declarations** (not the statements) — `streamName`/`typeCode` first,
`task` last — flipped the allocation to match, with no other change. Same
`__asm__("")`-relevant caveat as always: this only ever moved WHICH
callee-saved register held which value, never introduced a
`register asm("$N")` binding, so it's within the rules.

## Proposed learning

When two callee-saved locals' physical register assignment (not their
control flow) is the only residue, try reordering their **C declarations**
before reaching for anything else — this GCC's simple linear allocator
appears to hand out `$s0`, `$s1`, ... in declaration order rather than
first-use order, at least for this shape (mirrors, but is distinct from,
`Pad__DispatchEvents`'s head-broadcast finding that prologue *store* order is
unreachable from C at all — this is about which variable gets which
register, a different axis).

## HEAD BROADCAST cross-check (this round's two levers)

- **goto/return early-exit lever:** not applicable — this function has no
  early-return-with-different-value shape; its single guard wraps the
  entire body and falls through to a shared epilogue either way.
- **hand-hoisted loop invariant lever:** not applicable — no loop in this
  function.

## Naming history (before round 100)
**`GameApplication__ShowIntroLogos` -- tier B.** Mechanics established
from the body and its string constants: gated by `arg->unk0C`, registers a
loader task for `"ETC\ASMKLOGO.TIM"`, then a stream task for whatever
`GetAsmkMovie` resolves (`"ETC\ASMK.STR"` per the header comment), then a
second loader task for `"ETC\OSDLOGO.TIM"` -- three named boot-time assets
loaded in sequence. "Intro logo sequence" describes what the function DOES
(loads these three specific assets, gated, in this order); it does not
assert why the game shows them (splash-screen framing is a reasonable
inference from the filenames but not confirmed by any code in this unit).

## Track 4 (2026-09-25, round 84, alpha)

GameApplication.h's StreamTask view names +0x004 `release` (BasicClass's, `void *`), was `start` (track 4 round 84; see GameApplication__PlayCinematic for the bytes that settled the return type). Byte-identical.

## Track 7 polish (round 100, echo)

### Naming

**`GameApplication__ShowIntroLogos` -- tier A** (renamed from `GameApplication__LoadIntroLogoSequence` with tools/rename.py). Evidence: gated by config->showIntroLogos, the body shows ETC\ASMKLOGO.TIM (ShowImage), streams ETC\ASMK.STR (GetAsmkMovie, MOVIE_ASMK) and shows ETC\OSDLOGO.TIM; nothing is loaded for later use, so "Load" and "Sequence" said less than "Show".

Body changes, all byte-identical: locals streamName/typeCode/typeLookup -> moviePath/movieId/frameCount.

### History: code_1677c.c comments before the round-100 polish

Moved here from the source, verbatim (names as they stood then, where the tools had not already rewritten them).

```c
/* Optional stream-load block, gated by self->config->showIntroLogos: registers a
 * "loader" task for "ETC\ASMKLOGO.TIM" (GameApplication__StartLoaderTask), then a separate
 * "stream" task for whatever type code GetAsmkMovie hands back
 * ("ETC\ASMK.STR"), then a second loader task for "ETC\OSDLOGO.TIM". */

extern const char *GetAsmkMovie(s32 *typeCodeOut); /* psyq_memset.s: writes 0x31 to *typeCodeOut if non-NULL, always returns &sAsmkMoviePath */
extern s32 GetMovieFrameCount(s32 index); /* psyq_memset.s: signed-halfword lookup into gMovieFrameCounts[index] */
```

## Track 10 (2026-09-28, round 104, alpha)

`StreamTask::streamName`, StreamTask__Init's parameter and StreamTaskInitFn's are `const char *` (were `s32`): every caller passes a path (GetAsmkMovie's string or a FilePathRecord), so the five `(s32)` casts in GameApplicationFileResource.c are gone. Byte-identical.
