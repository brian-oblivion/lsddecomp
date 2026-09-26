# VabDriver__Destroy

**Unit (by ROM address):** code_179d8_d · **Semantic owner:** `gVabDriverMethods`
(code_179d8_e.c) · **Size:** 1 instruction (`jr $ra; nop`, 0x8 bytes) ·
**Status: MATCHED**, whole-image SHA1 green. Splat matched this itself (empty
body); no derivation was spent.

## Role

The `+0x00C` (dtor) slot of `gVabDriverMethods`. Empty, mirroring
`VabDriver__VabDriver` immediately preceding it (same unit, same round):
this backend needs no extra generic teardown beyond FileResource's own base
dtor.

```c
void VabDriver__Destroy(void) {
}
```

Confirmed as this exact slot by `python3 tools/classtable.py 0x8006D9BC`
(`+0x00C  0x8002C3C8`, prior to this rename).

## Naming

Renamed `func_8002C3C8 -> VabDriver__Destroy`, tier A, same evidence and
same cross-unit-ownership note as `VabDriver__VabDriver`'s report
(code_179d8_e.c's own comments already identified this slot by its old
placeholder name; matches the `FileResource__Finalize` naming precedent at the
same `+0x00C` slot position in the base class and in `D_8006D940`'s own
`PlacementGrid__Finalize`, this unit, this round).
