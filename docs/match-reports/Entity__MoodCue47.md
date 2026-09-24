# Entity__MoodCue47 -- MATCHED (80/80 words)

> Renamed from `func_8006090C` on 2026-09-24 (tools/rename.py). Address 0x8006090c.

Unit: `Entity_d` (second pass, round 2026-09-03). State-machine-style
dispatch on `this->unk44`, no "out" parameter (confirmed from its own body,
same as `Entity__func_80060710`/`Entity__MoodCue56` earlier in this unit).
`void Entity__MoodCue47(Entity *this)`.

## Final source

```c
void Entity__MoodCue47(Entity *this) {
    if (this->unkF4 == 0) {
        return;
    }
    if (this->unk44 == 0) {
        this->unk44 = 0xC;
        this->unkFC = 0;
        return;
    }
    if (this->unk44 == 0xC) {
        if (this->unkFC < 0x1E) {
            if (this->unk94->methods->slot100(this->unk94) != 0) {
                this->unk94->methods->slot130(this->unk94, 0);
                this->unkFC = 0;
                this->unk44 = 0xB;
            }
        } else {
            this->methods->slot30(this, 0xB);
            this->unk44 = 0xA;
        }
    } else if (this->unk44 == 0xB) {
        if (this->unkFC == 0x64) {
            this->methods->slot30(this, 0xC);
        } else {
            this->unk94->methods->slotCC(this->unk94, -0x64, 0);
        }
    }
}
```

## Derivation notes

Matched first attempt, no iteration needed. Two early guard clauses (`if
(!cond) return;`) followed by a two-level dispatch on `this->unk44`'s
current small-integer state (`0xC` vs `0xB`), each level with its own
nested condition -- reads as an ordinary state machine tick function, one
state transition per call.

New vtable slot discovered: `Unk94Methods::slotCC` (`void (*)(Unk94Obj
*self, s32 arg1, s32 arg2)`), called as `slotCC(this->unk94, -0x64, 0)`.
Return value unused at this, its only known call site, so `void` is a safe
default per the project's usual caveat. Added to `include/Entity.h`,
carved out of the `pad000[0x100]` gap at the FRONT of `Unk94Methods` (the
struct previously started its first named slot at `+0x100`; this pushes a
named slot as low as `+0xCC`, with `slot100` unchanged at `+0x100`). This
also reuses the already-typed `Unk94Methods::slot100` (`s32 (*)(Unk94Obj
*self)`) and `slot130` (`void (*)(Unk94Obj*, s32)`) with no changes.

### Proposed learning

None -- clean state-machine dispatch, no residue, no register-allocation
surprises. Notable only for being the third distinct slot discovered on
`Unk94Methods` in this unit (after `slot1A0` from `Entity__MoodCue46`),
confirming `Unk94Obj` carries a substantial vtable of its own worth
resolving incrementally as more Entity_d functions touch it.

## Naming

`Entity__MoodCue47` -- tier B (round 76, runner delta, FINISHING-PLAN track 3). Same
convention as `Entity__MoodCue00` (round 71): the function's address is the
handler word of `gEntityMoodHandlerTable` (`asm/data/79528.data.s`, base
0x80089EB0, 0x10-byte stride) at row 47, read directly from
`disk/SLPS_015.56` (not inferred from address proximity -- see
`src/Entity_d.c`'s unit header comment, which flags that row order does NOT
track code address once row 115 is reached). Mechanics established
(mood-tick sound-cue-set callback, per `Entity__StartSoundCue`/
`Entity_b.c`'s own header comment); which dream object owns the row is not.
