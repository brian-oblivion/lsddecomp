> Renamed from `func_80056F28` on 2026-09-18 (tools/rename.py). Address 0x80056f28.

# LinkOwnerObj__ReleaseLinksB -- MATCHED (9/9 words)

Unit: `class_3bb8c_o` (round 17). Byte-identical body to `LinkOwnerObj__ReleaseLinks`
(see that report) -- releases the same 5-element `arr84` array.

## Final source

```c
void LinkOwnerObj__ReleaseLinksB(LinkOwnerObj *this) {
    ReleaseBasicClassArray((void **)this->arr84, 5);
}
```

## Derivation

Same disassembly shape as `LinkOwnerObj__ReleaseLinks`: `addiu $a0,$a0,0x84` /
`jal ReleaseBasicClassArray` / `ori $a1,$zero,5`. Two separate ROM functions with
identical bodies is unremarkable for this project (an "add-links"/
"remove-links" pair that both happen to reduce to a full release in this
particular class, or simply two call sites the original source shared via
one small helper that GCC didn't inline differently). See `LinkOwnerObj__ReleaseLinks`'s
report for the type derivation (`LinkOwnerObj`, `ReleaseBasicClassArray`'s generic
`void **` signature).

### Proposed learning

None -- see `LinkOwnerObj__ReleaseLinks`.
