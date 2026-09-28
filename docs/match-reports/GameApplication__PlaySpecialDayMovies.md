# GameApplication__PlaySpecialDayMovies

> Renamed from `GameApplication__StartGraphRoomStreamTask` on 2026-09-27 (tools/rename.py). Address 0x8002658c.

> Renamed from `Class6D3C8__StartGraphRoomStreamTask` on 2026-09-26 (tools/rename.py). Address 0x8002658c.

> Renamed from `func_8002658C` on 2026-09-24 (tools/rename.py). Address 0x8002658c.

**Unit:** game_shell · **Size:** 65 instructions (0x104 bytes) · **Status:** MATCHED (65/65 words, whole-image SHA1 green)

## What it does

Called by `GameApplication__RunTitleMenu` (matched earlier) when its first `PollTask`
reports `2`. Gated by `self->arg->unk08 != 0` (the same gate
`GameApplication__PlayOpeningMovie` uses). Builds a `StreamTask`, derives a count via
`GetSpecialDayMovieSpan`, initializes the task with that count divided by 15 and a
fixed sub-slot, then a 5-argument `slot44` call (`a3 = -1`, unlike this
unit's other `slot44` call sites which pass a computed lookup), then starts
it.

## Final C

```c
void GameApplication__PlaySpecialDayMovies(GameApplication *self) {
    StreamTask *task;
    struct {
        u32 unk00;
        u32 unk04;
        u32 count;
    } buf;
    s32 extra;

    if (self->arg->unk08 != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        task = New_StreamTask(0, 0, 0, 0);
        extra = GetSpecialDayMovieSpan(&buf.count, 0, 10);
        task->methods->slot6C(task, buf.count / 15);
        task->methods->slot12C(task, 0);
        task->methods->slot44(task, self->unk1C, extra, -1, 1);
        task->methods->slot4(task);
    }
}
```

## Two residues, both familiar shapes from earlier functions in this unit

**1. Divisor: the magic constant proved the divisor, not the other way
round.** First attempt divided by 9 (the surrounding call passed a literal
`10`, and `9`/`10` looked like plausible siblings), producing a *different*
magic-multiply constant (`0x38E38E39`, shift 1) than retail's
(`0x88888889`, shift 3). Rather than guess again, solved for the divisor
`N` retail's constants imply: `0x88888889 * 15 == 2^35 + 7` — the standard
unsigned-division-by-15 magic pair. Divide by `9` was simply wrong; `/ 15`
reproduced retail's exact instruction sequence including the shift amount.
This is the same "reconstructing the sequence by hand is wrong, the operand
GCC actually divided by is a hard fact recoverable from the constant" idiom
CLAUDE.md already documents for `%`/`/`, just applied in the direction of
*discovering* the divisor rather than confirming a known one.

**2. Stack-local struct size AND field order both mattered, not just size.**
Same 8-byte-shortfall shape as `GameApplication__GameApplication`'s `LoadModelRequest`: the
`out`-parameter local only needed 4 bytes for what this call site reads
back, but retail reserves 12. Padding it to 3 words fixed the frame size
(`-0x38` matched) but left the field's *address* 8 bytes short
(`sp+0x18` instead of retail's `sp+0x20`) — because the single field this
call site actually touches was declared *first* in a padded struct, placing
it at the LOW end of the block. Retail's compiler put the touched word at
the block's *high* end, which only reproduces when the padding fields are
declared BEFORE the real one, not after. **A partially-used local's total
size fixes the frame; which end of it is padding additionally fixes the
field's own address.**

## New struct/header knowledge

`include/game_application.h`: added `StreamTaskMethods.slot6C` and `.slot12C` (new
slots, both `(void *self, s32 a1)`), retyped `slot44`'s 3rd parameter from
`const char *path` to plain `s32 arg2` -- confirmed generic by this call
site passing a computed count where `GameApplication__ShowIntroLogos` passed a string
pointer and `GameApplication__PlayOpeningMovie` passed another plain count; the field is a
raw 32-bit value whose interpretation is call-site-specific, not
uniformly a string. Declared `GetSpecialDayMovieSpan` (day/count helper,
`psyq_memset.s`, same "write to *out, return a separate value" shape as
`GetAsmkMovie`/`PickOpeningMovie`).

## Proposed learning

When a magic-multiply constant doesn't match a guessed divisor, don't
iterate divisors by trial and error against the *shape* of the
instructions -- solve `constant * N == 2^(32+shift) + small_remainder` for
the retail constant and shift directly; it names the exact divisor in one
step. And when a partially-used stack local's size fix moves the frame size
right but leaves the field's own address off by the padding amount, try
moving the padding fields to the OTHER side of the struct before assuming a
second local is involved.

## HEAD BROADCAST cross-check (this round's two levers, plus #3's branch-target check)

- **goto/return early-exit lever:** not applicable -- single guard wraps the
  whole body, no differing-return-value early exit (function is void).
- **hand-hoisted loop invariant lever:** not applicable -- no loop.
- **Branch-target check (broadcast #3):** the only branch in this function
  is the single top-level gate (`beqz v0,16e74`), and its target matched
  retail from the very first attempt; every residue here was instruction
  content (magic constant, stack offset), never a branch target, so there
  was never a CFG question to resolve.

## Naming history (before round 100)
**`GameApplication__PlaySpecialDayMovies` -- tier B.** Mechanics: gated by
`arg->unk08`, builds a `StreamTask`, derives a count via `GetSpecialDayMovieSpan`,
initializes the task from `count/15` and a fixed sub-slot, dispatches a
5-argument `configure` (`a3 = -1`, unlike every other StreamTask launcher in
this unit), then starts it. Named for its one and only caller:
`GameApplication__RunTitleMenu` invokes it exactly when its first
`GraphRoom`-named PollTask (`New_GraphRoom`) reports status "2" -- the
same "GraphRoom" vocabulary as that report's naming, not a guess (evidence:
the call-site gate, not the function body alone, which by itself doesn't
mention GraphRoom).

## Track 4 (2026-09-25, round 84, alpha)

game_application.h's StreamTask view names +0x004 `release` (BasicClass's, `void *`), was `start` (track 4 round 84; see GameApplication__PlayCinematic for the bytes that settled the return type). Byte-identical.

## Track 7 polish (round 100, echo)

### Naming

**`GameApplication__PlaySpecialDayMovies` -- tier B** (renamed from `GameApplication__StartGraphRoomStreamTask` with tools/rename.py). Evidence: streams GetSpecialDayMovieSpan(&frameTotal, 0, 10)'s record, FILM\SPDAY01A.STR (special day 0's first movie), with player frame count -1 and a frame bound of frameTotal, the total frames of the first ten special days' movies (setFrameBound(frameTotal / STREAMTASK_FRAMES_PER_SECOND), StreamTask's bound being bound * 15 frames). Its caller is RunTitleMenu when the GraphRoom scored. Tier B: whether the player really runs through all twenty movies in one stream, and what "scored" means in the game, are not established.

Body changes, all byte-identical: buf {u32 unk00, unk04, count} -> {u8 pad00[8]; s32 frameTotal} (MATCHING: 12 bytes, the read word last); `extra` -> moviePath; the divide keeps its unsigned cast ((u32)frameTotal: retail's 0x88888889 multiply is unsigned); 15 -> STREAMTASK_FRAMES_PER_SECOND.

### History: code_1677c.c comments before the round-100 polish

Moved here from the source, verbatim (names as they stood then, where the tools had not already rewritten them).

```c
/* Called by GameApplication__PollGraphRoomStatus when its first PollTask reports "2". Gated by
 * self->config->playStreams (same gate as GameApplication__StartWeeklyStreamTask). Builds a StreamTask,
 * derives a count via GetSpecialDayMovieSpan, sets the task's frame bound
 * to that count / 15 and clears skipOnConfirm, then runs its init (stream
 * group -1, unlike the other call sites) and releases it. */

extern s32 GetSpecialDayMovieSpan(s32 *out, s32 a1, s32 a2); /* psyq_memset.s: writes a derived count to *out, returns a separate derived value */
```

## Track 10 (2026-09-28, round 104, alpha)

`StreamTask::streamName`, StreamTask__Init's parameter and StreamTaskInitFn's are `const char *` (were `s32`): every caller passes a path (GetAsmkMovie's string or a FilePathRecord), so the five `(s32)` casts in game_shell.c are gone. Byte-identical.

## Track 10 (2026-09-28, round 104, echo)

game_files.h's `FilePathRecord` (an opaque `u8 data[0x1C]`) merged into cd_driver.h's `CdFileEntry` ({name[0x14], CdlLOC pos, u32 size}, 0x1C): sRecordTable goes GetRecordTable -> RegisterRecordTableFiles -> RegisterFileTableEntries -> SetFileTable, so its records are the CD driver's file-table entries. game_files.h includes cd_driver.h; sRecordTable is declared `CdFileEntry[]` and GetRecordTable returns `CdFileEntry *`, dropping the casts at its callers, and a record used as a path is spelled `record->name` instead of a `(char *)`/`(const char *)` cast. Byte-identical.
