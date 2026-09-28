# New_StyleEffect -- MATCHED (40/40 words)

> Renamed from `New_Class876FC` on 2026-09-26 (tools/rename.py). Address 0x80056320.

> Renamed from `func_80056320` on 2026-09-23 (tools/rename.py). Address 0x80056320.

Unit: `class_3bb8c_k` (round 17 continuation). `New_X` allocator (0x98
bytes) for the `gStyleEffectMethods` class, dispatching through
`GetStyleEffectMethods()->ctor` -- a CROSS-UNIT call into the already-matched
`class_3bb8c_o.c` (previous pass, same round) rather than calling
`StyleEffect__StyleEffect` (this unit's own ctor) by name.

## Final source

```c
void *New_StyleEffect(void *arg0, void *arg1, void *arg2, void *arg3) {
    StyleEffect *self = BMemPMgrAlloc(0x98);

    if (self != NULL) {
        if (GetStyleEffectMethods()->ctor(self, arg0, arg1, arg2, arg3) != NULL) {
            return self;
        }
        BMemPMgrFree(self);
        return NULL;
    }
    return NULL;
}
```

## Derivation

The `New_X` allocator sub-shape #3 from `DECOMPILATION_LEARNINGS.md`
("tests BOTH the allocation and the constructor's return, freeing on
constructor failure -- plain `if`/`return`"), same shape as
`class_3bb8c_o.c`'s own `New_Actor` from the previous pass. The
constructor is reached THROUGH THE VTABLE (`GetStyleEffectMethods()->ctor(...)`,
where `GetStyleEffectMethods` is `class_3bb8c_o.c`'s already-matched getter for
`&gStyleEffectMethods`) rather than by a direct `jal` to `StyleEffect__StyleEffect` -- both
resolve to the same function at runtime, but the disassembly's own
`jal GetStyleEffectMethods` / `lw v0,8(v0)` / `jalr v0` sequence requires the
vtable form, not a direct call. `StyleEffectMethods::ctor` (declared in this
unit, see `StyleEffect__StyleEffect`'s report) is 5-argument (`self` + 4 forwarded
parameters), matching this allocator's own 4 incoming parameters plus
`self`.

### Proposed learning

None -- confirms the `New_X` sub-shape #3 pattern established last pass,
this time reached through a vtable dispatch rather than a bare `jal`.

## Naming

**Tier A.** `New_X` allocator convention (`New_Actor`, `New_SceneNode`, `New_StageMap`, `New_BoxFill`, `New_VabStreamObj`), matching this unit's own established allocator shape and `StyleEffect` (see below).

## Track 4 (2026-09-26, round 88, charlie)

Retyped with the class's unification (include/StyleEffect.h): returns `StyleEffect *` and takes `(s32 kind, StyleEffectParams *params, SceneNode *parent, LongVec3 *pos)` -- kind 0..3 from StyleFillEffectKind0..3's constants, params the gStyleSpawnOffsetX block the reset slot copies, parent gStyleGrid (attachToParent's parent), pos the caller's position (AddVec3's input). Image byte-identical.

## Track 7 (2026-09-27, round 96, charlie)

- `BMemPMgrAlloc(0x98)` is `BMemPMgrAlloc(sizeof(StyleEffect))`: the struct
  is 0x98 bytes (include/StyleEffect.h), byte-exact. Zero bytes.
