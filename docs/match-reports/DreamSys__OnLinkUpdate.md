# DreamSys__OnLinkUpdate -- MATCHED (2/2 words)

> Renamed from `DreamSys__NoOpSlotE8Default` on 2026-09-29 (tools/rename.py). Address 0x800590e0.

Unit: `src/world/dream_sys.c`. Class: `DreamSys`.

## What it does

`jr $ra; nop` -- an empty function body (`{ }`), splat-matched trivially. Never a STALL; simply never had its own report before this naming pass (CLAUDE.md: "some bodies are just jr $ra; nop and splat generated them itself").

## Naming

- **Tier A.** Empty `{}` body, no arguments read. Default value of vtable slot +0x0E8; DreamSys__WallLink calls it unconditionally on `this` and ignores the argument. A different address at the SAME conceptual slot, used by sibling subclass instance tables, is already named `Actor__OnLinkUpdate` (src/world/dream_scene.c, round 57) -- this function is DreamSys's own DEFAULT occupant of that slot, so it needed a distinct name.
