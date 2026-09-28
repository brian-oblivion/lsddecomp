# Actor__Reset -- MATCHED (4/4 words)

> Renamed from `BaseObjO__InitDefaults` on 2026-09-25 (tools/rename.py). Address 0x800571e8.

> Renamed from `func_800571E8` on 2026-09-18 (tools/rename.py). Address 0x800571e8.

Unit: `class_3bb8c_k` (round 17). A two-field setter: `self->unk48 = 0x12C`
(halfword), `self->unk54 = 0` (word).

## Final source

```c
void Actor__Reset(BaseObjO *self) {
    self->unk48 = 0x12C;
    self->unk54 = 0;
}
```

## Derivation

`sh $v0,0x48($a0)` (`$v0` pre-loaded with the literal `0x12C`) then
`sw $zero,0x54($a0)` in the branch delay slot of `jr $ra`. Two independent
field writes, no control flow -- matched first try. `unk48`'s width
(`s16`) is confirmed by `Actor__NotifyMove`'s own `lh` read of the same field
(sign-extending load).

### Proposed learning

None -- straightforward setter.

## Naming

**`Actor__Reset` -- tier B.** Mechanics fully described: sets
`self->unk48 = 0x12C` (300) and `self->unk54 = 0`, unconditionally, no
control flow. Named for the mechanic (initializing two fields to fixed
default values) without asserting what those fields represent in the
game -- `unk48`/`unk54` are left unrenamed. `Actor__NotifyMove` reads
both fields together (adding/subtracting `unk54` from `unk48` depending on
sign) which is suggestive of a paired value/adjustment relationship, but
not strong enough evidence for a purpose-asserting name like "heading".

## Track 4 (2026-09-25, round 82, delta)

Renamed from `BaseObjO__InitDefaults`. Override of +0x040 (SceneNode's reset, which the ctor calls last), named for its slot: lastOffsetValue = 300, pendingExtra = 0. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_k.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 99, alpha)

0x12C is written 300 (a distance, decimal). No name: nothing else shares the value, and the comment says what it is for (the distance NotifyMove stretches the hull by until a move or setLastOffsetValue sets one).
