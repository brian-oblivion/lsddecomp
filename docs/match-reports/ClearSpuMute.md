# ClearSpuMute

> Renamed from `func_80032A7C` on 2026-09-23 (tools/rename.py). Address 0x80032a7c.

**Unit:** code_179d8_c · **Size:** 8 instructions · **Status:** MATCHED (8/8 words)

## What it does

A zero-argument tail-call wrapper around `func_80038FB0` (lives in a
different, already-carved segment: `the 0x272C8..0x2C054 Psy-Q block (now linked from lib/ where a disc owns it, formerly asm/psyq_SpuSetMute.s)`). That
function guards on an SPU-mute-ish flag (`D_8006E184`), and when set,
clears it, resets several sound globals, and calls two SPU voice-key
helpers (`func_8003902C`/`func_8003903C`, both trivial `jr`-based
trampolines). It takes no arguments (called with a `nop` delay slot
everywhere it's invoked) and never sets `$v0`, so it is `void(void)`.

## The C

```c
extern void func_80038FB0(void);

void ClearSpuMute(void)
{
    func_80038FB0();
}
```

## Provenance

round 16 (2026-09-04), runner delta, unit code_179d8_c (fresh carve).
Matched first attempt.

## Naming

Round 69 (delta). `ClearSpuMute` (was `func_80032A7C`): a zero-argument
tail-call wrapper to `func_80038FB0`, an uncarved function this report
already documents as guarding on an SPU-mute flag and, when set, clearing
it and resetting several sound globals plus two SPU voice-key helpers.
Tier B -- the name reflects that callee's documented behaviour (its own
future name is not this unit's to assign, per FINISHING-PLAN track 3's
field/function ownership rule extended by analogy: `func_80038FB0` lives
outside this unit's file). Unrelated to the `SeqTimer*` cluster the rest of
this unit is about; it only shares this address range.
