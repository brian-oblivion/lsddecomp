# SetTeleportsEnabled

**Unit:** code_4cd08 · **Size:** 15 words · **Status:** MATCHED (15/15 words)

## What it does

Enables or disables teleports depending on the trigger-type value passed in:
enabled only for trigger types `0xB` and `0x3`, disabled for everything else.
Forwards the resulting boolean to `func_8005BF68` (defined in `DreamSys`,
still `INCLUDE_ASM` at the time of this report — only its call-site signature
mattered here).

## Derivation

```
addiu $sp, $sp, -0x18
sw    $ra, 0x10($sp)
ori   $v0, $zero, 0xB
beq   $a0, $v0, .L8005C6F8
 addu $v1, $zero, $zero        ; v1 = 0 (delay slot, always runs)
ori   $v0, $zero, 0x3
bne   $a0, $v0, .L8005C6FC
 nop
.L8005C6F8:
ori   $v1, $zero, 0x1          ; v1 = 1
.L8005C6FC:
jal   func_8005BF68
 addu $a0, $v1, $zero
...
```

`$v1` is unconditionally zeroed in the first branch's delay slot, then set to 1
only when `$a0 == 0xB` (falls straight into the `ori $v1, 1`) or when the
second comparison fails to skip past it (`$a0 == 0x3`). That is exactly the
short-circuit evaluation order of `a0 == 0xB || a0 == 3`, checked in that
order, so the C is written the same way rather than as two separate `if`s.

```c
extern void func_8005BF68(bool value);

void SetTeleportsEnabled(s32 triggerType)
{
    func_8005BF68(triggerType == 0xB || triggerType == 3);
}
```

`func_8005BF68`'s own definition lives in `DreamSys` and was untouched by this
change — only an `extern` prototype was added here so the call site type-checks.
Its parameter is a plain 0/1 value coming out of the `||` here, hence `bool`;
nothing at this call site says anything about its return type, and the return
value is discarded, so it is declared `void` (weak evidence — revisit if
`DreamSys`'s own work on `func_8005BF68` finds otherwise).

## Proposed learning

Calling a function that is still `INCLUDE_ASM` in a DIFFERENT unit is fine —
add a local `extern` prototype at the call site derived from how it's used
(argument count/type from the registers loaded before the `jal`, return type
from whether the caller consumes `$v0`). The prototype does not need to be
authoritative; it only needs to make the call site's own bytes match, since
the callee's own body is untouched either way.

## Naming

**SetTeleportsEnabled** — tier A, pre-existing (not renamed this round;
reviewed as part of the track 3 pass). A pure leaf: forwards
`triggerType == 0xB || triggerType == 3` to `func_8005BF68`. The mechanics
ARE the name. Note: `func_8005BF68` (called here and by
`EnableTeleportsForKind`) is defined in `DreamSys`, not in this unit --
MATCHED there round 43 (this report's "still `INCLUDE_ASM` at the time of
this report" note is accurate for when it was written but is now stale;
left as historical record rather than rewritten). It is DreamSys's function
to name, not this unit's, so it keeps `func_` here.
