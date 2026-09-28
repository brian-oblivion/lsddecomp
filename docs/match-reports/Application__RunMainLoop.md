# Application__RunMainLoop

> Renamed from `Class6E4F0__RunMainLoop` on 2026-09-26 (tools/rename.py). Address 0x8003b110.

> Renamed from `func_8003B110` on 2026-09-25 (tools/rename.py). Address 0x8003b110.

**Round 81 (delta)** · **Unit:** Application · **Size:** 63 words · **Status:** MATCHED (63/63 words, whole-image SHA1 green, second build)

## What it does

Slot `+0x04C` of gApplicationMethods (inherited by GameApplication; `main` dispatches
it). If `initialized`, calls `+0x050` once, then runs forever: `+0x054`,
then polls `+0x058` for a status and dispatches: 1 -> `+0x05C` and poll
again, 2 -> `+0x064` if `+0x060` says so, 0 -> back to `+0x054`. Slots
+0x050..+0x064 exist only in the subclass table (GameApplication's
LoadIntroLogoSequence, StartWeeklyStreamTask, PollGraphRoomStatus,
NoOpSlot5C, PollStatusObj, StartStreamTaskWithInit). It never returns
while initialized: the game's main loop.

```c
void Application__RunMainLoop(Application *self) {
    s32 status;

    if (self->initialized) {
        self->methods->slot50(self);
        for (;;) {
            self->methods->slot54(self);
            for (;;) {
                status = self->methods->slot58(self);
                if (status == 1) {
                    self->methods->slot5C(self);
                    continue;
                }
                if (status == 2) {
                    if (self->methods->slot60(self)) {
                        self->methods->slot64(self);
                    }
                }
                if (status == 0) {
                    break;
                }
            }
        }
    }
}
```

## Derivation (one build, 62/63)

First try was the natural `do { ... if (1) A; else if (2) {...} } while
(status != 0);`. 62/63: the only diff was the `j` after the slot5C
`lw`, retail `j .L8003B15C` (the `nop; jalr` of the slot54 call, falling
into the slot58 poll) against built `j` to the slot64 call's `nop; jalr`
(falling into the `status != 0` test). Both are GCC cross-jumping the
`jalr v0; move a0,s0` tail onto an identical one; which tail it picks
follows where the source path rejoins. Retail's path after slot5C goes
straight to the poll without testing status, i.e. a `continue` in a
`for (;;)` whose exit is an explicit `if (status == 0) break;` at the
bottom. Byte-exact on that spelling.

### Proposed learning

**Cross-jump target = where the C rejoins.** When the only diff is a `j`
target that lands on a different `nop; jalr v0; move a0,sN` tail, the two
candidates are both legal cross-jumps of an identical call tail; pick the
C whose control flow after the jumping branch reaches the SAME next
statement retail's target falls into. `continue` in `for (;;)` (goes to
the loop top, no test) vs `do {} while (x)` (goes to the test) is the
usual switch.

## Naming

**Round 81 (delta), track 3.** Renamed `func_8003B110` -> `Application__RunMainLoop`.
**Tier B**: "runs forever while initialized, dispatching the subclass's own
state-machine slots" is evident from the body alone; "main loop" is
corroborated by `include/GameApplication.h`'s own note that this is the slot
"first dispatched by main" (`src/main.c`), but that is one caller, not two
agreeing ones, so it stays B rather than A.

## Track 4

**2026-09-25, round 84 (echo).** The class is declared once, in
`include/Application.h`. The six slots this function calls, +0x050..+0x064,
are NULL words in gApplicationMethods's own data (the table is 0x68 bytes, past the 19
slots classtable.py prints); they are named for the subclass's occupants
(`loadIntroLogoSequence`, `startWeeklyStreamTask`, `pollGraphRoomStatus`,
`slot5C` for the no-op, `pollStatusObj`, `startStreamTaskWithInit`). +0x060
stays `s32`: this body tests its return, and the occupant,
GameApplication__RunDayTask, returns `s32`. Bytes unchanged.

## Track 7 (round 101, charlie)

The status literals are `enum ApplicationLoopStatus` (include/Application.h):
`APPLICATION_LOOP_OPENING` (0, the `break` back to +0x054),
`APPLICATION_LOOP_SLOT5C` (1, +0x05C then `continue`) and
`APPLICATION_LOOP_DAY` (2, +0x060 and, on nonzero, +0x064). Evidence: this
body's own dispatch; the names follow the hooks', which are named for
GameApplication's occupants, and agree with GameApplication.h's
`GameApplicationLoopStatus` (0 = OPENING, 2 = DAY; its RunTitleMenu never
returns 1). Tier B, like the hook names. Byte-identical. The Final C above is
the pre-track-7 text.

## Track 10 (2026-09-28, round 104, alpha)

Application slot +0x05C is `onRepeatMenu` (was `slot5C`) and its status `APPLICATION_LOOP_REPEAT_MENU` (was `APPLICATION_LOOP_SLOT5C`): runMainLoop calls it when runTitleMenu returns 1 and then runs runTitleMenu again. Tier B: the mechanics; GameApplication never returns 1, so what the hook was for is not in this game.
