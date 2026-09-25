# Class6E4F0__RunMainLoop

> Renamed from `func_8003B110` on 2026-09-25 (tools/rename.py). Address 0x8003b110.

**Round 81 (delta)** · **Unit:** code_2b78c · **Size:** 63 words · **Status:** MATCHED (63/63 words, whole-image SHA1 green, second build)

## What it does

Slot `+0x04C` of D_8006E4F0 (inherited by Class6D3C8; `main` dispatches
it). If `initialized`, calls `+0x050` once, then runs forever: `+0x054`,
then polls `+0x058` for a status and dispatches: 1 -> `+0x05C` and poll
again, 2 -> `+0x064` if `+0x060` says so, 0 -> back to `+0x054`. Slots
+0x050..+0x064 exist only in the subclass table (Class6D3C8's
LoadIntroLogoSequence, StartWeeklyStreamTask, PollGraphRoomStatus,
NoOpSlot5C, PollStatusObj, StartStreamTaskWithInit). It never returns
while initialized: the game's main loop.

```c
void Class6E4F0__RunMainLoop(Class6E4F0 *self) {
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

**Round 81 (delta), track 3.** Renamed `func_8003B110` -> `Class6E4F0__RunMainLoop`.
**Tier B**: "runs forever while initialized, dispatching the subclass's own
state-machine slots" is evident from the body alone; "main loop" is
corroborated by `include/Class6D3C8.h`'s own note that this is the slot
"first dispatched by main" (`src/main.c`), but that is one caller, not two
agreeing ones, so it stays B rather than A.
