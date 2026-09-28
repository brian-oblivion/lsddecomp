# StreamTask__StreamTask

> Renamed from `StreamTaskObj__StreamTaskObj` on 2026-09-26 (tools/rename.py). Address 0x8003b8e4.

> Renamed from `func_8003B8E4` on 2026-09-23 (tools/rename.py). Address 0x8003b8e4.

**Unit:** task · **Size:** 62 words · **Status:** MATCHED (62/62)

## Summary

This is `StreamTaskObj`'s constructor — occupies `gStreamTaskMethods`'s own slot
`+0x008` (per `classtable.py gStreamTaskMethods`, confirming the earlier header
comment that named this function as the ctor reached through
`GetStreamTaskMethods()`'s slot).

```c
void StreamTask__StreamTask(StreamTaskObj *self, s32 a1, s32 a2, s32 a3, StreamTaskInitData *a4) {
    GetTaskCoreMethods()->slot08(self, a1, a2, a3);
    self->methods = GetStreamTaskMethods();
    if (a4 != NULL) {
        self->unkA8 = *a4;
    } else {
        self->unkA8 = *GetDefaultMovieFrame();
    }
    self->unkB4 = New_MoviePlayer(GetDefaultMovieFrame(), 0, 0);
    self->unkB8 = 0;
    self->methods->slot40(self);
}
```

## New structure discovered

- `GetTaskCoreMethods()->slot08(...)`: new `TaskCoreMethods` slot `+0x008`.
  `classtable.py gTaskCoreMethods` shows it occupied by `TaskCore__TaskCore` — **this
  unit's own next queued function**, confirming the 4-argument
  `(self, a1, a2, a3)` signature ahead of writing that function.
- `self->methods->slot40(self)`: new `StreamTaskObjMethods` slot `+0x040`,
  occupied by this unit's own already-matched `StreamTask__Reset` (per
  `classtable.py gStreamTaskMethods`) — confirms single-argument arity.
- `StreamTaskInitData` (new type): a plain 3-word struct. Both the
  function's optional 5th (stack) argument `a4` and `GetDefaultMovieFrame()`'s
  return value are this shape — retail copies whichever one applies
  wholesale into `self->unkA8` (see residue below).
- `self->unkA8` (`+0x0A8`, `StreamTaskInitData`, embedded by value, 0xC
  bytes): supersedes an earlier plan (never committed) to model it as three
  separate `s32` fields `unkA8`/`unkAC`/`unkB0` — see residue.
- New externs: `GetDefaultMovieFrame(void)` (returns `StreamTaskInitData *`, a
  "default init data" singleton accessor, called twice — once for the `a4 ==
  NULL` fallback copy, once again fresh as `New_MoviePlayer`'s first argument;
  each call's result is consumed immediately, so no local caching needed
  between them, unlike the `self->field`-across-`jalr` case) and
  `New_MoviePlayer(StreamTaskInitData *, s32, s32)` (returns
  `StreamTaskUnkB4Obj *`, allocates/inits `self->unkB4`; not in this unit).

## The one real residue and its fix

First attempt modeled `unkA8`/`unkAC`/`unkB0` as three separate `s32` fields
and wrote three separate assignments per branch
(`self->unkA8 = a4->unk0; self->unkAC = a4->unk4; self->unkB0 = a4->unk8;`).
That compiled and linked, but scored 23/62 with a **huge "differs outside
range" byte count** (186158 bytes) — the two-way `if`/`else` branch was one
word short in each arm, shifting everything after it (and, since this is an
early function in the unit, most of the rest of the executable).

`asm-differ` showed the real shape directly: retail loads **all three**
source words first (`v0`/`v1`/`a0`), *then* stores all three
(`sw v0,0xa8`/`sw v1,0xac`/`sw a0,0xb0`) — the already-documented
"whole-struct assignment, not indexed access" idiom
(`docs/DECOMPILATION_LEARNINGS.md`, `Pad__LoadButtonTable`), just at 3 words instead
of a larger block. Three separate `field = field` statements instead
interleave load/store/load/store, one word longer per arm than the batched
form. Modeling `self->unkA8` as an embedded `StreamTaskInitData` and writing
plain struct assignment (`self->unkA8 = *a4;`) reproduced the batched
load-all-then-store-all form exactly.

## Third-learning check (per head's request)

**Not needed here** in the "value read then re-read after a `jalr`" sense —
`self->methods` IS written mid-function (`self->methods = GetStreamTaskMethods();`)
and read again at the very end after two more calls
(`GetDefaultMovieFrame`/`New_MoviePlayer`) for `self->methods->slot40(self)`, but that
final read is a **plain, single, natural field dereference** with no earlier
same-expression read to conflict with — nothing needed caching because
nothing was read twice. The lever from `TaskCore__OnDeinit`/`TaskCore__Reset` is
specifically about *reusing an already-loaded value* across a call; here the
value is simply read once, fresh, at its point of use, which needs no local.
The residue this function actually had was the sibling lesson from the same
"multi-word struct" family (batched vs. per-field codegen), not the
"self->field across jalr" one.

## Proposed learning

**A struct-shaped group of sibling fields (here 3 consecutive words) that is
ever copied wholesale from one source to another should be modeled as ONE
embedded struct field, not N separate scalar fields** — even before writing
the first function that touches it, if the call site's own load/store
pattern (batch-then-batch vs. interleaved) is visible in the disassembly.
Three separate C assignments cost an extra word versus one struct
assignment; the register-batching pattern is the tell, distinguishable from
an ordinary "3 fields happen to be adjacent" case by whether retail loads
all sources before storing any of them.

## Naming

**StreamTask__StreamTask** -- tier A. Occupies `gStreamTaskMethods`'s
own ctor slot `+0x008` (`classtable.py`), matching the established
`Class__Class` constructor convention already used elsewhere in this
codebase (`IntermediateBase__IntermediateBase`, `StageMap__StageMap`).

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `GetTaskCoreMethods()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/StreamTask.h): the `Obj` suffix is dropped (track 4 step 2; include/game_application.h already viewed the class as `StreamTask`). The ctor at +0x008.

## Track 4 (2026-09-26, round 89)

The player is a MoviePlayer (`include/MoviePlayer.h`); task.h's StreamTaskUnkB4Obj view is gone and task.c's `PLAYER()` casts `player` (still `BasicClass *` in StreamTask.h) to `MoviePlayer *`. The allocation is `New_MoviePlayer((DrawRect *)GetDefaultMovieFrame(), 0, 0)`: the ctor's frame rectangle (a pointer cast, no code). Byte-identical.


## Track 6 (2026-09-27, round 99, runner bravo)

`StreamTaskInitData` was a local spelling of draw_system.h's `DrawRect` (the same three words, and `sDefaultMovieFrame`, the fallback, was already `extern DrawRect` in both units that name it); it is deleted. `initData` (+0x0A8), the fifth parameter and `GetDefaultMovieFrame()` are `DrawRect`, so the allocation is `New_MoviePlayer(GetDefaultMovieFrame(), 0, 0)` with no cast. The struct copy is unchanged (both types are 12 bytes, 4-aligned). Byte-identical; `typeviews.py --warnings` 0 new. The getter and the data were renamed this round from `GetDefaultStreamTaskInitData`/`gDefaultStreamTaskInitData` (see GetDefaultMovieFrame.md).

## History (moved from include/StreamTask.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
 * The name is the table's stem with its `Obj` dropped (track 4 step 2);
 * include/GameApplication.h's view already called it StreamTask. What it does,
 * measured: it owns
```

```c
 * +0x044's contradiction, settled by track 4 step 6 (round 85's TodActor
 * +0x04C rule): StreamTask__Init takes
```

```c
 * (no caller reads it), as the round-84 `configure` view had it.
```

## Track 10 (2026-09-28, round 104, alpha)

`StreamTask::streamName`, StreamTask__Init's parameter and StreamTaskInitFn's are `const char *` (were `s32`): every caller passes a path (GetAsmkMovie's string or a FilePathRecord), so the five `(s32)` casts in game_shell.c are gone. Byte-identical.
