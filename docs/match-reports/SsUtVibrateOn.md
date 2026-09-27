# SsUtVibrateOn -- MATCHED (2/2 words, splat-generated)

> Renamed from `func_8002E2F8` on 2026-09-27 (tools/rename.py). Address 0x8002e2f8.

Unit: `src/code_179d8_l.c`. Address `0x8002E2F8`.

Trivially matched: the empty C body (`void SsUtVibrateOn(void) {}`) compiles
to retail's own `jr $ra; nop` -- splat emitted this body itself when the
unit was carved (round 24, 2026-09-08). Not decompilation work.

## Naming

**Unnameable at this pass -- kept `func_`, tier C.** No caller anywhere in
`src/` (`grep -rn SsUtVibrateOn src/ include/` outside this unit's own
carve-note comment finds nothing) and no reference from any `asm/data/*.s`
rodata table at this address (checked both numerically and for a symbolic
`.word` pointing here). An empty two-word stub with zero known callers and
zero table attachment carries no evidence at all for what it was meant to
do -- a game-style name here would be a pure guess, which HARD naming rule
("name what the code does, never what you guess it is for") and the tier
rubric ("a wrong tier-A name is worse than `func_`") both rule out. Possibly
a debug/unused vtable-filler entry; not established.
