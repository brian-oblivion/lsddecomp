# Application__Application

> Renamed from `Class6E4F0__Class6E4F0` on 2026-09-26 (tools/rename.py). Address 0x8003af8c.

> Renamed from `func_8003AF8C` on 2026-09-25 (tools/rename.py). Address 0x8003af8c.

**Round 81 (delta)** · **Unit:** Application · **Size:** 38 words · **Status:** MATCHED (38/38 words, whole-image SHA1 green, first build)

## What it does

The gApplicationMethods constructor (slot `+0x008`). GameApplication__GameApplication
(GameApplicationFileResource) calls it through `GetApplicationMethods()->ctor(self, arg->unk00)`.

1. base ctor through BasicClass's table;
2. installs its own table (GetApplicationMethods);
3. one-time `CdInit()`, guarded by the sdata flag sCdInitDone (gp_rel);
4. clears `initialized` (+0x18) and calls `SetActiveDataSource(source)`;
5. calls its own `+0x040` slot with the {320, 240} default (sDefaultScreenDims).

```c
void Application__Application(Application *self, s32 source) {
    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetApplicationMethods();
    if (sCdInitDone == 0) {
        CdInit();
        sCdInitDone = 1;
    }
    self->initialized = 0;
    SetActiveDataSource(source);
    self->methods->setDims(self, &sDefaultScreenDims, 0);
}
```

Nothing needed tuning: the `sw $zero, 0x18($s0)` lands in
SetActiveDataSource's delay slot and the table store in the `bnez` slot
by the scheduler. `sCdInitDone` is gp-relative through
`--gp-symbols` (resolved blocker; ordinary extern). `CdInit`/`SsInit`/
`GsInit3D` declared locally from the Psy-Q prototypes.

## Naming

**Round 81 (delta), track 3.** Renamed `func_8003AF8C` -> `Application__Application`
(constructor convention, `Class__Class`). **Tier A**: it is the +0x008 ctor
slot (`classtable.py 0x8006E4F0 --vs 0x8006B58C`), confirmed by
`GameApplication__GameApplication` (GameApplicationFileResource) calling it through
`GetApplicationMethods()->ctor(self, arg->unk00)` as the base-constructor step
before installing its own vtable -- the base-ctor-through-slot+8 shape from
docs/research/class-framework.md. The body is substantive ctor work (base
ctor, install own table, one-time CdInit, clear `initialized`,
SetActiveDataSource, default screen dims), not a guess about purpose.

Globals `sCdInitDone` (tier A: guards the one-time `CdInit()` call, mechanics
is the purpose) and `sDefaultScreenDims` (tier B: a `{0x140, 0xF0}` = {320, 240}
ScreenDims constant, the "default" claim is evident from being the ctor's
own default argument to `setDims`) renamed via `tools/rename.py`: referenced
only from this unit (`grep -rn` over `src/`), so in this unit's ownership
per the field/global rule.

`func_8003B20C` (the table getter) is **NOT renamed this round**: proposed
`GetApplicationMethods`, but `tools/rename.py` cannot apply it -- see that
function's own report for the blocker and the broadcast post.

## Track 4

**2026-09-25, round 84 (echo).** The class is declared once, in
`include/Application.h`. The parameter is now `dataSource`: it goes straight
to `SetActiveDataSource`, and the one caller passes `sGameApplicationConfig`'s
first word, 0x13 (the CD driver's class id, gCdDriverMethods). The table getter is
`GetApplicationMethods` (renamed from `func_8003B20C`). Bytes unchanged.

## History (moved from the unit banner, round 92)

code_2b78c's banner, before track 6 rewrote it as documentation (names as
they stand after the round-92 renames):

```text
/*
 * code_2b78c -- GAME code carved from psyq_2b78c on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x2B78C..0x2BA1C (vram 0x8003AF8C..0x8003B21C). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: methods of gApplicationMethods and
 * gGameApplicationMethods, calling SetActiveDataSource; the yaml had called this gap "the
 * game's own libsnd build".
 *
 * Round 81 (delta): all seven functions matched. They are the whole of one
 * class, gApplicationMethods (local view Application below, class id 0x60): its ctor
 * (`Application__Application`), empty finalize override, four own slots
 * (`SetScreenDims`, `InitSystems`, a no-op, and `RunMainLoop` -- the
 * subclass GameApplication's per-frame dispatcher, first run from `src/main.c`)
 * and the table getter.
 *
 * Round 81 (delta), track 3 naming pass: all seven functions and both
 * unit-local globals (`gCdInitDone`, `gDefaultScreenDims`) named -- tiers
 * and evidence in each function's own match report's `## Naming` section.
 * One exception: `func_8003B20C` (the table getter, proposed
 * `GetApplicationMethods`) was NOT renamed -- `tools/rename.py` cannot apply
 * it because this address already carried an explicit, now-stale, track-2
 * "unidentified" line in the symbols file and the tool's placeholder-name
 * address resolution never finds it to replace; see
 * docs/match-reports/GetApplicationMethods.md and the round-81 broadcast for the
 * head to apply by hand. Round 84 (echo, track 4): applied with rename.py,
 * which now replaces the existing symbols line.
 *
 * Round 84 (echo, track 4): the class is declared once, in
 * include/Application.h; this unit's local view of it is gone.
 */
```

## Track 6 (round 92, charlie)

Class `Class6E4F0` renamed `Application` (`tools/renametype.py`), table
`D_8006E4F0` renamed `gApplicationMethods` (`tools/rename.py`). Tier A:
the class's methods are the program's bring-up and outer loop and nothing
else. The ctor runs CdInit (once per boot, sCdInitDone) and picks the data
source; initSystems registers main()'s DrawSystem, opens the display
(GsInitGraph through DrawSystem's initGraph), then SsInit and GsInit3D, and
allocates the argument block every task receives; runMainLoop never returns
and calls six hooks this class leaves NULL. main() builds exactly one object
of its only subclass, GameApplication, and calls initSystems then runMainLoop on
it. The two callers (main and GameApplication's ctor) agree.

`Class6E4F0Aux` deleted: it is `IntermediateBaseInitArgs` (same 0x14 bytes;
GameApplication's methods cast `self->aux` to that type at every task init;
+0x000 the DrawSystem, which IntermediateBase's onNotify routes as a
gDrawSystemMethods sender; +0x004 the Pad; three NULLs that
IntermediateBase__Init fills with its own FrameClock, LightRig, Viewport).
InitSystems's five stores are the compiler's accessor list.

Facts the header banner carried as derivation, kept here:

- The table is 0x68 bytes, not the 19 slots `classtable.py` prints: six NULL
  words at +0x050..+0x064 follow +0x04C in the data, and runMainLoop calls
  all six through `self->methods`. Named for GameApplication's occupants, as
  FileResource's interface slots are named for the CD driver's.
- +0x044's fourth parameter is the caller's: GameApplication__InitSystems passes
  0 (`move a3,zero` in the jalr's delay slot); the occupant never reads $a3.
- Object size 0x20: no allocator, but GameApplication's ctor stores its argument
  at +0x020, which bounds it.
- ScreenDims and InitSystems's drawSystem argument became DrawSystem's in
  round 87 (include/DrawSystem.h).

## History (moved from src/Application.c)

The file's banner carried its edge evidence:

> Its edges are Sony's objects on both sides (libsnd/sspause before,
> libgs/gs_104 after), so the file is the whole region; tools/tuboundary.py
> (round 101) finds no rodata inside it that joins or splits anything, and
> its seven functions are one class's.
