# DreamSys__NoOpSlot14C -- MATCHED (2/2 words)

Unit: `src/world/dream_sys.c`. Class: `DreamSys`.

## What it does

`jr $ra; nop` -- an empty function body (`{ }`), splat-matched trivially. Never a STALL; simply never had its own report before this naming pass (CLAUDE.md: "some bodies are just jr $ra; nop and splat generated them itself").

## Naming

- **Tier A.** Empty `{}` body. One of DreamSys__SelectLookCallback's three non-NULL menu choices (mode 2), vtable slot +0x14C.
