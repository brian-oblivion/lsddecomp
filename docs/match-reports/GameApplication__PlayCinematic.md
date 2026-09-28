# GameApplication__PlayCinematic

> Renamed from `GameApplication__StartCinematicStream` on 2026-09-27 (tools/rename.py). Address 0x8002677c.

> Renamed from `Class6D3C8__StartCinematicStream` on 2026-09-26 (tools/rename.py). Address 0x8002677c.

> Renamed from `func_8002677C` on 2026-09-24 (tools/rename.py). Address 0x8002677c.

**Unit:** game_shell · **Size:** 97 instructions (0x184 bytes) · **Status:** MATCHED (97/97 words, whole-image SHA1 green)

## What it does

Called by `GameApplication__RunDayTask` when its `StatusObj` slot44 result is `2` (per
that function's still-stalled derivation). Reads `DreamSys`'s current
cinematic slot (`GetCinematic`), packs its two 16-bit fields and resolves
them to a channel index (`GetSpecialDayOrEventRecord`, which also returns a second,
unrelated `s32` used later as `groupId`). If the channel is unresolved
(`-1`), starts a `LoaderTask` on a fixed "no cinematic" path. Otherwise, if
`self->arg->unk08` gates it, starts a `StreamTask` on the resolved channel.
Either branch finishes by starting whichever task it built; if the channel
resolved but the gate was off, nothing else happens.

## Final C

```c
void GameApplication__PlayCinematic(GameApplication *self) {
    CinematicCall cc;
    struct {
        s32 chan;
        u32 unk04;
        u32 unk08;
    } chanBuf;
    s32 groupId;
    s32 lookup;
    LoaderTask *task;

    cc = self->dreamSys->vt->GetCinematic(self->dreamSys);
    groupId = GetSpecialDayOrEventRecord(&chanBuf.chan, (u16) cc.bank | ((u32) (u16) cc.entry << 16));
    SetActiveDataSourceDriverMode(0, 0, 0);

    if (chanBuf.chan != -1) {
        if (self->arg->unk08 != 0) {
            StreamTask *streamTask = New_StreamTask(0, 0, 0, 0);

            streamTask->methods->slot12C(streamTask, 0);
            lookup = GetMovieFrameCount(chanBuf.chan);
            streamTask->methods->slot44(streamTask, self->unk1C, groupId, lookup, 1);
            streamTask->methods->slot4(streamTask);
        }
    } else {
        task = New_TaskCore(0, 0, 0);
        task->methods->slot6C(task, 10);
        task->methods->slotD4(task, groupId, 0);
        task->methods->slot44(task, self->unk1C, 0);
        task->methods->slot4(task);
    }
}
```

## Four levers, in the order that closed them

1. **A struct returned by value with a hidden pointer (`CinematicCall
   GetCinematic(DreamSys*)`) is the ABI, not a manual convention.** Retail's
   `addiu $a0, $sp, 0x18` right after the vtable-slot fetch is the compiler
   inserting the hidden return-pointer argument in front of the real ones
   (`this` shifts to `$a1`) -- ordinary `CinematicCall cc = ...->GetCinematic(...)`
   reproduces it with no manual pointer plumbing needed.

2. **Branch polarity/placement had to match the source's own guard, not just
   "some if/else."** First attempt wrote `if (chan == -1) { LoaderTask } else
   if (unk08) { StreamTask }`, which put the LoaderTask body first in the
   instruction stream (source order == emission order here, unlike the
   goto/return cases in other functions in this unit) -- 23/97, with the two
   task-construction blocks visibly swapped in the diff. Retail's actual
   `beq chan,-1,ELSE` skips forward past an in-line `if (unk08)` block to an
   out-of-line `else`, i.e. the CONDITION had to be inverted at the C level
   (`if (chan != -1) { if (unk08) {...} } else { LoaderTask }`) to match
   which block sits inline vs. jumped-to. This alone was worth 53 words.

3. **Two structurally-identical 8-byte-shortfall locals, both from the
   "partially-used out-param" idiom already seen in `GameApplication__GameApplication` and
   `GameApplication__PlaySpecialDayMovies`.** `CinematicCall cc` (a real return value, needs no
   padding of its own) sits fine as a standalone local, but combining it
   into ONE struct with the separate `chan` out-param broke the
   struct-return codegen entirely (GCC materialized `GetCinematic`'s hidden
   pointer into a fresh temporary and then did a manual unaligned
   `lwl/lwr`+`swl/swr` copy into the combined struct's `cc` member, instead
   of passing `&cc` directly as the hidden pointer) -- worth reverting.
   Keeping `cc` standalone and padding *only* `chan` (into a 3-word struct,
   `chan` first) closed the frame-size gap cleanly: 85 -> 95/97.

4. **A value captured in an unrelated call's delay slot, again.** The last
   residue (2 words) was an instruction-order swap around
   `SetActiveDataSourceDriverMode(0,0,0)`'s call setup. Reading it closely: retail's
   `move s2,v0` sits in `SetActiveDataSourceDriverMode`'s OWN call's delay slot, using
   whatever `v0` held immediately before -- which is `GetSpecialDayOrEventRecord`'s
   return, not `SetActiveDataSourceDriverMode`'s. My first attempt had it backwards
   (`groupId` assigned from `SetActiveDataSourceDriverMode`'s return, called after
   discarding `GetSpecialDayOrEventRecord`'s) -- swapping which call's return feeds
   `groupId`, and discarding `SetActiveDataSourceDriverMode`'s (matching its established
   "called for side effect, return unused" role in `GameApplication__ShowIntroLogos` and
   `GameApplication__PlayOpeningMovie`), closed the last two words.

## New struct/header knowledge

`include/GameApplication.h`: `SetActiveDataSourceDriverMode`'s extern retyped from `void` to
`s32` (it does return a meaningful value -- whatever its internal dispatch
loop last produced -- confirmed here even though this call site, like
`GameApplication__ShowIntroLogos`/`GameApplication__PlayOpeningMovie`, discards it). Added `GetSpecialDayOrEventRecord`
(psyq_memset.s: resolves a packed `{bank,entry}` `CinematicCall` to a
channel index via an out-param, **and** returns a second, separate `s32`
kept by this function -- easy to miss since most callers of "write to
*out" helpers in this unit only use the out-param, not the return).

## Proposed learning

When a "write to `*out`, also returns a value" helper (this unit has
several: `GetAsmkMovie`, `PickOpeningMovie`, `GetSpecialDayMovieSpan`, now
`GetSpecialDayOrEventRecord`) is followed immediately by ANOTHER call whose own return
is discarded, check which call's return actually lands in the next
persistent register via the delay-slot-capture idiom before assuming it's
the SECOND (most recently called) function's return -- it can just as
easily be the FIRST call's return, sitting unclobbered in `$v0` until the
second call's own delay slot opportunistically claims it.

## HEAD BROADCAST cross-check (this round's two levers, plus #3's branch-target check)

- **goto/return early-exit lever:** not directly applicable (no
  differing-return-value early exit; this is a void function), but the
  underlying idea -- match which block sits inline vs. out-of-line to the
  guard's actual branch polarity -- is exactly lever 2 above, just at the
  `if`/`else` block-placement level rather than `goto`/`return`.
- **hand-hoisted loop invariant lever:** not applicable -- no loop.
- **Branch-target check (broadcast #3):** performed at each step; the 23/97
  and 76/97 attempts both had CORRECT branch targets once the polarity was
  fixed (lever 2), confirming the remaining residues at each stage were
  register/stack-layout content, not CFG mistakes.

## Naming history (before round 100)
**`GameApplication__PlayCinematic` -- tier B.** Mechanics: reads the owned
`DreamSys`'s current cinematic slot (`vt->GetCinematic`, an already-named
vtable accessor), resolves it to a channel index; if resolution fails (-1),
starts a `LoaderTask` on a fixed "no cinematic" fallback path; otherwise, if
gated by `arg->unk08`, starts a `StreamTask` on the resolved channel. Named
for the dispatch mechanic ("start [a task streaming] the cinematic"); the
`LoaderTask` fallback arm makes "stream" not literally universal (the
no-cinematic path loads, it doesn't stream), but the function's dominant,
gated behavior and its trigger (`GetCinematic`) are both clearly cinematic-
related, which is stronger grounding than the generic "start task" shape
shared by this unit's other four StreamTask launchers.

## Track 4 (2026-09-25, round 84, alpha)

The task these functions build with New_TaskCore is a plain TaskCore (include/TaskCore.h, track 4 round 84); GameApplication.h's LoaderTask view is gone and the calls use TaskCore's slot names (setCallback, setFrameBound, setSubHandle, init, release). The old `start` slot at +0x004 is BasicClass's release, and StreamTask's own +0x004 is typed `void *(*release)` too: with one void and one value-returning, StartCinematicStream's two branches stopped cross-jumping into one call (+6 instructions). Byte-identical.

## Track 7 polish (round 100, echo)

### Naming

**`GameApplication__PlayCinematic` -- tier B** (renamed from `GameApplication__StartCinematicStream` with tools/rename.py). Evidence: resolves DreamSys's getCinematic pair with GetSpecialDayOrEventRecord: a movie (movie id != -1) is streamed, gated by config->playStreams, with no skip on confirm; a special day's TIM image (movie id -1) is shown by a TaskCore for 10 seconds (setFrameBound(10), TASKCORE_FRAMES_PER_SECOND). Tier B: "cinematic" is DreamSys's slot name, and what the pair selects in the game is not established (GetSpecialDayOrEventRecord.md).

Body changes, all byte-identical: chanBuf {s32 chan; u32 unk04, unk08} -> idBuf {s32 movieId; u8 pad04[8]} (MATCHING: 12 bytes, the read word first); groupId/lookup -> path/frameCount; GetSpecialDayOrEventRecord's extern returns const char * (the record, used as a path by both branches). The old reading ("channel index", a "no cinematic" fallback path, SetActiveDataSourceDriverMode's return "kept") is in the history section below: the value kept across SetActiveDataSourceDriverMode is GetSpecialDayOrEventRecord's return, and SetActiveDataSourceDriverMode returns void (game_shell.c).

### History: code_1677c.c comments before the round-100 polish

Moved here from the source, verbatim (names as they stood then, where the tools had not already rewritten them).

```c
/* Reads DreamSys's current cinematic slot, resolves it to a channel index
 * (GetSpecialDayOrEventRecord); if that fails (-1), starts a LoaderTask on the fixed
 * "no cinematic" path; otherwise, if self->config->playStreams gates it, starts a
 * StreamTask on the resolved channel. Either branch finishes by starting
 * whichever task it built; if neither branch runs, nothing happens. */

extern s32 SetActiveDataSourceDriverMode(s32 a0, s32 a1, s32 a2); /* code_171e0, still INCLUDE_ASM there; returns
                                                       the last value its internal dispatch loop got --
                                                       GameApplication__ShowIntroLogos/GameApplication__PlayOpeningMovie discard it, but
                                                       GameApplication__PlayCinematic keeps it */
extern s32 GetSpecialDayOrEventRecord(s32 *out, s32 packedBankEntry); /* psyq_memset.s: resolves a packed
    {bank; entry} CinematicCall (low 16 bits = bank, high 16 = entry) to a channel index written
    to *out (-1 if unresolved); the packing must zero-extend both halves before combining
    (retail loads them with lhu, not lh) since the result is bitwise-composed, not a value read
    back as a signed 32-bit number. Also returns its own (separate) s32 value, kept by
    GameApplication__PlayCinematic. */
```

## Track 10 (2026-09-28, round 104, alpha)

`StreamTask::streamName`, StreamTask__Init's parameter and StreamTaskInitFn's are `const char *` (were `s32`): every caller passes a path (GetAsmkMovie's string or a FilePathRecord), so the five `(s32)` casts in game_shell.c are gone. Byte-identical.

## Track 10 (2026-09-28, round 104, echo)

GetSpecialDayOrEventRecord is now prototyped in include/game_files.h with the definition's `(s32 *movieIdOut, CinematicCall pick)`, and PlayCinematic passes `cc` directly instead of packing bank | entry << 16 by hand. cc1 passes the 4-byte struct in $a1 with the same lhu/sll/or the hand packing wrote, so the only change was the frame: the 12-byte `idBuf` struct (movieId plus 8 bytes of pad) had been sized to match the frame the packing produced, and with the struct passed directly a plain `s32 movieId` gives retail's 0x40 frame. Byte-identical (97/97, whole image green).

game_files.h's `FilePathRecord` (an opaque `u8 data[0x1C]`) merged into cd_driver.h's `CdFileEntry` ({name[0x14], CdlLOC pos, u32 size}, 0x1C): sRecordTable goes GetRecordTable -> RegisterRecordTableFiles -> RegisterFileTableEntries -> SetFileTable, so its records are the CD driver's file-table entries. game_files.h includes cd_driver.h; sRecordTable is declared `CdFileEntry[]` and GetRecordTable returns `CdFileEntry *`, dropping the casts at its callers, and a record used as a path is spelled `record->name` instead of a `(char *)`/`(const char *)` cast. Byte-identical.
