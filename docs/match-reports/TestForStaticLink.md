# TestForStaticLink

**Unit:** DreamSys · **Size:** 17 instructions · **Status:** MATCHED (17/17 words)

## What it does

A pure forwarding wrapper: passes its own three arguments straight through
to `GetStaticSpawn` (still `INCLUDE_ASM`) and appends a fixed trailing
quadruple identifying which permalink table set to search
(`LEN_STAGE_PERMALINK_TRIGGERS`, `STAGE_PERMALINK_TRIGGERS`,
`STAGE_PERMALINK_SPAWNS`, literal `1`).

## The C

```c
s32 TestForStaticLink(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage)
{
	return GetStaticSpawn(target, currentPos, stage, LEN_STAGE_PERMALINK_TRIGGERS,
	                       STAGE_PERMALINK_TRIGGERS, STAGE_PERMALINK_SPAWNS, 1);
}
```

```c
extern s32 GetStaticSpawn(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage,
                           s8 *triggerLens, StaticLinkTrigger **triggers, StageSpawn **spawns, s32 flag);
```

## How the 3-argument signature and the 7-argument callee were derived

The raw disassembly sets up only `$a3` and three stack slots
(`0x10`/`0x14`/`0x18`) before `jal GetStaticSpawn` -- `$a0`-`$a2` are never
touched, meaning TestForStaticLink's OWN three parameters flow straight
through unchanged. Confirmed the parameter TYPES by reading the one caller
(`DreamSys__StaticWallLink`, still `INCLUDE_ASM`, but its own disassembly
is retail bytes regardless): it calls
`TestForStaticLink(&this->linkCoordinates, currentPos, this->currentStage)`,
where `currentPos` is `DreamSys__StaticWallLink`'s OWN second parameter
(`PlayerSpawnPoint *`), forwarded without ever being reloaded -- explaining
why `TestForStaticLink`'s call site sets only `$a0`/`$a2` and leaves `$a1`
untouched (it's already sitting there from the caller's own entry).
`GetStaticSpawn`'s return value flows straight back out (nothing
overwrites `$v0` between the `jal` and `jr $ra`), so `TestForStaticLink`
tail-forwards its return type too.

**Return type is a guess, not evidence** -- flagged per CLAUDE.md's
tail-call-wrapper warning: the byte match proves the CALL is right, not
that `s32` is `GetStaticSpawn`'s real return type. Chose `s32` because the
one caller (`DreamSys__StaticWallLink`) tests the result with `bltz`
(negative = failure), consistent with the `s32`-stage-or-negative
convention already documented elsewhere in this header
(`GetRandomSpawnFromStage`).

`Test4TunnelLinks`, `Test4StaircaseNodes`, and `Test4InstantTeleporters`
(none in this round's scope) share the identical shape against different
table triples -- `GetStaticSpawn`'s declared signature should cover all
four once someone gets to them.

## Provenance

round 2026-08-30-c, runner ALPHA, unit DreamSys (whole-unit second pass).

## Comment moved from src/DreamSys.c (round 92, track 7)

Replaced in the source by a comment that says what the code does; kept here as written.

```c
/* Forward declarations for two of this unit's OWN functions, both called
   around line 450 but not defined until ~200 lines later, in ROM order.
   Without these, C89 implicitly declares them as `int ()` at the call site
   and cpp emits "implicit declaration of function". The implicit type
   happens to agree with the real one here, so nothing miscompiled -- but an
   implicit declaration also disables argument checking, which is precisely
   what caught Entity__IsNearTarget's over-narrow `s8` parameters in include/Entity.h
   this round. A declaration is not a definition, so this does NOT affect the
   strict ROM-address ordering of the definitions below. */
```
