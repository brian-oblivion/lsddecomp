# DreamSys__StopVoice — MATCHED (round 2026-08-30-c)

> Renamed from `func_80059E3C` on 2026-09-22 (tools/rename.py). Address 0x80059e3c.

**Unit:** DreamSys · **Size:** 23 instructions · **Status:** MATCHED (23/23 words)

> **RESOLVED, round 2026-08-30-c, head adjudication overturning the STALL
> below.** The classification as "register identity" was wrong. Tracing
> forward from the `lw $a1, 0xBC($s0)` at `0x80059E4C` to the `jalr $v0` at
> `0x80059E74`: nothing overwrites `$a1` in between (the only other setup is
> `$a0` from `this->unk_0x58`), so `$a1` is not just a branch condition --
> it is LIVE at the call and is being PASSED as the slot's second argument.
> `slot0x84` takes `(void *self, s32 flag)`, not `(void *self)`. All four
> reshapes below kept the one-argument call, so none of them could have
> closed this -- the parameter list itself was wrong, not the register
> allocation. See "Proposed learning" for the general test this establishes.

## What it does

Vtable slot `+0x16C` (resolved via `tools/classtable.py DREAMSYS_METHODS`,
named in the header even though the function itself stalled -- the slot
identity is not in question, only the body's exact codegen). Gate flag
`unk_0xBC`: while `>= 0`, calls through `this->unk_0x58`'s own vtable at
slot `+0x84` (with itself as the sole argument), then resets `unk_0xBC` to
-1.

## The matching C

```c
void DreamSys__StopVoice(DreamSys *this)
{
	DreamSysUnk58 *obj;

	if (this->unk_0xBC >= 0) {
		obj = (DreamSysUnk58 *)this->unk_0x58;
		obj->vt->slot0x84(obj, this->unk_0xBC);
		this->unk_0xBC = -1;
	}
}
```

```c
typedef struct DreamSysUnk58Vtable {
	u8 pad00[0x84];
	void (*slot0x84)(void *self, s32 flag);
} DreamSysUnk58Vtable;
typedef struct DreamSysUnk58 {
	DreamSysUnk58Vtable *vt;
} DreamSysUnk58;
```

The only change from every prior attempt: `slot0x84(obj)` became
`slot0x84(obj, this->unk_0xBC)`. Nothing else about the body moved.

## History: what looked like a register-identity residue (kept for the record)

Originally misdiagnosed as register identity. Two-word diff (both differing
by exactly one byte -- the register field), offset
`0x80059E4C`-`0x80059E54`:

```
retail: lw $a1, 0xBC($s0)      built: lw $v0, 0xBC($s0)
retail: bltz $a1, .L80059E84   built: bltz $v0, .L80059E84
```

Retail loads `this->unk_0xBC` directly into `$a1` and branches on it there;
this body's natural codegen loads it into `$v0` instead. Everything else --
including the branch TARGET, the call sequence, and the final store --
matches exactly (21/23 words; the other 2 are these two).

Reshapes tried, all reproducing the IDENTICAL two-word diff with no
movement:

1. Direct `if (this->unk_0xBC >= 0) { ... }` (shown above, minus the local).
2. Load into an explicit `s32 flag = this->unk_0xBC;` local before the `if`,
   declared either before or after the `DreamSysUnk58 *obj` local
   (declaration-order swap, per the "prologue callee-save store order isn't
   reachable from C" precedent -- tried both orders, no change).
3. Fully inlined, no local at all:
   `((DreamSysUnk58 *)this->unk_0x58)->vt->slot0x84((DreamSysUnk58
   *)this->unk_0x58);` with the cast expression written twice.
4. A bare `__asm__("")` scheduling barrier as the function's first
   statement -- per CLAUDE.md rule 6, this only changes instruction ORDER,
   never register identity, and confirmed exactly that here: no change
   whatsoever to the diff.

All four reshapes above kept `slot0x84` as a one-argument call and only
varied HOW the guard value was loaded/named -- none of them could have
closed the diff, because the diff's actual cause (a missing second argument)
was untouched by any of them. Round 2026-08-30-b filed this as a stall on
the (reasonable-looking, but wrong) theory that four failed reshapes of a
register-identity-shaped residue meant it was unreachable from C. It wasn't
unreachable -- the reshapes were just all testing the wrong hypothesis.

### Proposed learning (supersedes the round 2026-08-30-b version)

**Before ever classifying a residue as register identity, trace forward
from the load to the next `jal`/`jalr` and check whether the register
survives unclobbered.** A struct field loaded into an ARGUMENT register
(`$a0`-`$a3`) that is not overwritten before the next call is an argument TO
that call -- even when its only visible source-level use looks like a
branch condition. `bltz $a1, ...` reads `$a1` for the branch, but does not
consume or clobber it; if nothing between the load and the following
`jalr` touches `$a1` again, it is live at the call and the call's parameter
list is wrong, not the register allocation. This is a strictly cheaper check
than reshaping: read forward from the load ONE time, before spending any of
the 30-attempt budget on declaration-order swaps, temp variables, or
scheduling barriers, all of which are structurally incapable of fixing a
missing parameter regardless of how many are tried. Confirmed twice now in
this unit (this function; see `New_DreamSys.md` for a case that looked
similar but genuinely wasn't -- the residue register there, `$v0`, is a
return-value register, not an argument register, and was never live into a
following call).

## Provenance

round 2026-08-30-b, runner ALPHA, address range `0x80058774`-`0x8005A1EC`
(misdiagnosed as a stall, reshapes 1-4). Resolved round 2026-08-30-c, same
runner, address range widened to the whole unit (head-adjudicated fix).

## Naming

`DreamSys__StopVoice` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_80059E3C`.

`if (voiceIndex >= 0) { soundObj->vt->slot0x84(soundObj,
voiceIndex); voiceIndex = -1; }` -- a pure leaf, so the SHAPE is tier A by the leaf
rule. It is tier B because the word "Voice" comes from another unit, not this body:
slot +0x84 on `soundObj`'s vtable is `VabStreamObjMethods::slot84` ==
`VabStreamObj__StopVoice` (src/code_179d8_e.c, matched), at the same offset with
the same signature, and `FlushSoundCueSet` in that unit guards its own call to it
with the identical `index >= 0` test.

## Track 4 (2026-09-26, round 87, VabStreamObj)

`include/DreamSys.h`'s `DreamSysUnk58`/`DreamSysUnk58Vtable` view is deleted.
`DreamSys::soundObj` is cast to `VabStreamObj *` (`include/VabStreamObj.h`),
and the slots are called by the class's names: `slot0x80` -> `playTone`
(`VabStreamObj__PlayTone`: index = program << 4 | tone, then vol and
endVol; it returns the voice), `slot0x84` -> `stopVoice`, and `slot0x9C` ->
`setPitchOffset` (its argument is an octave: pitchOffset = octave * 12 - 24).
The view had typed stopVoice `void`, but its occupant returns s32 (always
-1). The whole image stays byte-identical with the s32 slot, because the one
call site discards the value. `soundObj` itself stays `s32`: it is
DreamSys's field.
