# Pad__NoOpSlot4C

> Renamed from `Pad__func_80025E14` on 2026-09-28 (tools/rename.py). Address 0x80025e14.

**Unit:** `src/Pad.c` (naming pass, round 77, `runner/echo`)
**Status:** MATCHED (2/2 words, full build verified byte-exact)
**Vtable slot:** `gPadMethods+0x4C` (`PadMethods.func4C`)

## What it does

An empty function body (`void Pad__NoOpSlot4C(void) { }`) compiling to
retail's `jr $ra; nop` at this slot. Trivially matched -- splat's own
extraction already produced the correct bytes for an empty C function, no
derivation was needed.

## Naming

**Tier A** (round 101, track 7; was tier C `Pad__func_800xxxxx`). An empty
leaf's mechanics are its purpose, which the naming rules make tier A by
definition, and `Class__NoOpSlotNN` is the project's established form for an
empty method-table occupant (`Actor__NoOpSlotD8`, `StreamTask__NoOpSlot88`,
`Application__NoOpSlot48`, ...). The slot number is `gPadMethods+0x4C`,
the field it occupies (`PadMethods.slot4C`, which stays a slot name: there
is still no caller to name it for). No caller anywhere in `src/` dispatches
through it; its sibling `Pad__NoOpSlot54` is the same shape.

### Proposed field names

None -- the occupying field is owned entirely by this unit's header and
carries no cross-unit accessor.
