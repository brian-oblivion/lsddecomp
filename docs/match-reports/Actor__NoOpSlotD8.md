# Actor__NoOpSlotD8 -- MATCHED (2/2)

> Renamed from `DreamSys__NoOpSlotD8` on 2026-09-25 (tools/rename.py). Address 0x80057610.

Unit: `src/world/dream_scene.c`. Class: `DreamSys`, own vtable slot `+0x0D8`
(resolved via `tools/classtable.py gDreamSysMethods`, which lists this
exact function -- `0x80057610` -- as the default table's `+0x0D8` value).
No match report existed before this round: this is a splat-matched,
length-exact two-word leaf (`jr $ra; nop`), one of the "four two-word
leaves, two splat-matched itself" the unit's original carve banner
mentions -- untouched, never a `STALL`, just never documented.

## Body

```c
void Actor__NoOpSlotD8(void) {
}
```

Retail's own `+0x0D8` implementation is an empty function body. The C
prototype takes no parameters even though the vtable slot's call site
passes `this`: the callee's body never references any argument, so (per
this project's established "per-call-site signature" precedent, see
`AcceptGridElem`'s report) the definition needs none either.

## Naming

**`Actor__NoOpSlotD8` -- tier A.** A pure no-op leaf: mechanics ARE
the purpose (nothing happens). Named after the established project
convention for exactly this shape -- compare `TextRow__NoOpSlotD0`
(`src/ui/screen_widgets.c`) and `NoOpIgnoreArgs` (`src/world/dream_scene.c`), both
`Class__NoOpSlotOFFSET`/`NoOpXxx` for an empty vtable-slot implementation
of otherwise-unknown purpose. `Actor__NoOpSlotE8` (this unit, below)
is the sibling case: a DIFFERENT slot with the SAME shape.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py Actor__NoOpSlotD8   # 2/2
```

## Track 4 (2026-09-25, round 82, delta)

Renamed from `DreamSys__NoOpSlotD8`. Occupant of +0x0D8 in gActorMethods, empty; the prefix follows the table it sits in. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/world/dream_scene.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
