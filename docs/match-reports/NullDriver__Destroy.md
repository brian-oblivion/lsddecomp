# NullDriver__Destroy

> Renamed from `VabDriver__Destroy` on 2026-09-28 (tools/rename.py). Address 0x8002c3c8.

**Unit (by ROM address):** vab_sound · **Semantic owner:** `gNullDriverMethods`
(vab_sound.c) · **Size:** 1 instruction (`jr $ra; nop`, 0x8 bytes) ·
**Status: MATCHED**, whole-image SHA1 green. Splat matched this itself (empty
body); no derivation was spent.

## Role

The `+0x00C` (dtor) slot of `gNullDriverMethods`. Empty, mirroring
`NullDriver__NullDriver` immediately preceding it (same unit, same round):
this backend needs no extra generic teardown beyond FileResource's own base
dtor.

```c
void NullDriver__Destroy(void) {
}
```

Confirmed as this exact slot by `python3 tools/classtable.py 0x8006D9BC`
(`+0x00C  0x8002C3C8`, prior to this rename).

## Naming

Renamed `func_8002C3C8 -> NullDriver__Destroy`, tier A, same evidence and
same cross-unit-ownership note as `NullDriver__NullDriver`'s report
(vab_sound.c's own comments already identified this slot by its old
placeholder name; matches the `FileResource__Finalize` naming precedent at the
same `+0x00C` slot position in the base class and in `gPlacementGridMethods`'s own
`PlacementGrid__Finalize`, this unit, this round).
