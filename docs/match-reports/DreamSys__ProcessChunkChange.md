# DreamSys__ProcessChunkChange

**Unit:** DreamSys · **Size:** 24 instructions · **Status:** MATCHED (24/24 words)

## What it does

Vtable `+0x1E4`. If `effect == 5`, calls a method on `entity` (vtable slot
`+0x10C`, `(entity, 0, 0)`) that returns a `PlayerSpawnPoint *`, and passes
it straight into `this->vt->LogChunkMood`.

## The C

```c
void DreamSys__ProcessChunkChange(DreamSys *this, void *entity, s32 effect)
{
	PlayerSpawnPoint *pos;

	if (effect == 5) {
		pos = ((DreamSysEntityObj *)entity)->methods->slot0x10C(entity, 0, 0);
		this->vt->LogChunkMood(this, pos);
	}
}
```

```c
typedef struct DreamSysEntityMethods {
	u8 pad00[0x10C];
	PlayerSpawnPoint *(*slot0x10C)(void *self, s32 arg1, s32 arg2);
} DreamSysEntityMethods;
typedef struct DreamSysEntityObj {
	DreamSysEntityMethods *methods;
} DreamSysEntityObj;
```

## `entity` is almost certainly `Entity*`, but typed opaquely instead

`entity`'s shape (vtable pointer at offset 0, `void*` signature elsewhere
in this header for the same parameter role -- see
`InstanceEffectsOnPlayer`) matches `include/entity.h`'s `Entity` type
closely. That header's own `EntityMethods` struct doesn't type slot
`+0x10C` yet (falls inside an unnamed `pad64[0x114-0x64]` gap), and
extending it is out of this unit's scope (`entity.h` belongs to a different
unit). Declared a minimal, LOCAL, DreamSys-only view of the same shape
instead of touching `entity.h` -- same pattern as `DreamSysBaseMethods` for
the shared class-framework base table in an earlier round.

Retyped the vtable field `ProcessChunkChange` from `void *` to this
function's own real signature, since this function IS that slot.

## Provenance

round 2026-08-30-d, runner ALPHA, unit DreamSys (whole-unit, third pass).
