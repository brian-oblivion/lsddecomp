# Actor__OnClass86AA0LinkCommand -- MATCHED (22/22)

> Renamed from `DreamSys__DispatchLinkCommand` on 2026-09-25 (tools/rename.py). Address 0x80057c14.

> Renamed from `func_80057C14` on 2026-09-19 (tools/rename.py). Address 0x80057c14.

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys`, own vtable slot `+0x0E0`
in the shared base table `gActorMethods` (`include/DreamSys.h`'s
`DreamSysBaseMethods::slot0xE0` already carried a comment naming this
exact function as its resolution). Not overridden at the top `DreamSys`
level, which instead has its own distinct `+0xE0` implementation
(`DreamSys__WallLink`, `LinkWall` in `include/DreamSys.h`).

## Signature

```c
void Actor__OnClass86AA0LinkCommand(DreamSys *self, void *arg1, s32 count);
```

## Body

```c
void Actor__OnClass86AA0LinkCommand(DreamSys *self, void *arg1, s32 count) {
    GetClass6B5CCMethods()->dispatchLinkCommand(self, arg1, count);
}
```

Sibling of `Actor__OnActorLinkCommand` (see that report for `GetClass6B5CCMethods()` and this
unit's local `Class6B5CCBaseTable` view): same single unconditional call
through the shared base table's `+0x09C` slot, but no second conditional
dispatch.

## Naming

**`Actor__OnClass86AA0LinkCommand` -- tier A.** A pure single-call forward
to `Class6B5CCBaseTable::dispatchLinkCommand` with no other logic --
mechanics ARE the purpose, matching the "pure leaf" carve-out. Sibling of
`Actor__OnActorLinkCommand` (this unit) without its
conditional second dispatch.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py Actor__OnClass86AA0LinkCommand   # 22/22
```

## Track 4 (2026-09-25, round 82, delta)

Renamed from `DreamSys__DispatchLinkCommand`. Occupant of +0x0E0, which Actor__DispatchLinkCommand calls for a 0x24 (Class86AA0) sender; DreamSys overrides it as DreamSys__WallLink, Entity as Entity__NotifyReset. Body: chain Class6B5CC's dispatchLinkCommand. The old name also collided with the +0x09C slot's. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a Class6B5CC subclass and the base of Class65650/Entity, DreamSys and Class876FC. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_p.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
