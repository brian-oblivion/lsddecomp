# Class876FC__ReleaseSpritesB -- MATCHED (9/9 words)

> Renamed from `LinkOwnerObj__ReleaseLinksB` on 2026-09-26 (tools/rename.py). Address 0x80056f28.

> Renamed from `func_80056F28` on 2026-09-18 (tools/rename.py). Address 0x80056f28.

Unit: `class_3bb8c_o` (round 17). Byte-identical body to `Class876FC__ReleaseSprites`
(see that report) -- releases the same 5-element `arr84` array.

## Final source

```c
void Class876FC__ReleaseSpritesB(LinkOwnerObj *this) {
    ReleaseBasicClassArray((void **)this->arr84, 5);
}
```

## Derivation

Same disassembly shape as `Class876FC__ReleaseSprites`: `addiu $a0,$a0,0x84` /
`jal ReleaseBasicClassArray` / `ori $a1,$zero,5`. Two separate ROM functions with
identical bodies is unremarkable for this project (an "add-links"/
"remove-links" pair that both happen to reduce to a full release in this
particular class, or simply two call sites the original source shared via
one small helper that GCC didn't inline differently). See `Class876FC__ReleaseSprites`'s
report for the type derivation (`LinkOwnerObj`, `ReleaseBasicClassArray`'s generic
`void **` signature).

### Proposed learning

None -- see `Class876FC__ReleaseSprites`.

## Naming

**`Class876FC__ReleaseSpritesB` -- tier A.** Byte-identical body to
`Class876FC__ReleaseSprites` (see that report), but a genuinely different
ROM function, reached from a different dispatch state
(`class_3bb8c_s.c:Class876FC__ReleaseByKind`'s `case 3` vs. `ReleaseLinks`'s `case 2`).
No evidence distinguishes what the two states mean, so the name only marks
this as the second, otherwise-identical, release entry point (suffix `B`)
rather than asserting a state-specific purpose that isn't established.

## Track 4 (2026-09-26, round 88, charlie)

Renamed from the `LinkOwnerObj__` family to `Class876FC__` with the class's
unification (`include/Class876FC.h`). Evidence: the only caller is
Class876FC's own per-kind dispatch in `class_3bb8c_s.c`
(`Class876FC__InitByKind` kind 3, `Class876FC__UpdateByKind` kind 3,
`Class876FC__ReleaseByKind` kinds 2/3), each passing its own `self`; the
five-element array at +0x084 ("links") is `Class876FC::sprites`, filled by
`Class876FC__SpawnSprites` with `New_VariantSprite` objects. The old
`LinkOwnerObj`/`LinkElemObj` views were Class876FC and VariantSprite under
another name; RandomizeSprites' `slot48` is VariantSprite's inherited
`updateScale` and its `angle` (+0x084) is `sprite.rotate` (Sprite, +0x064 +
0x020, 4096 per degree -- the `(rand() % 360) << 12` it stores).

View replaced the same day: the `LinkOwnerObj`/`LinkElemObj` views in class_3bb8c_o.c are deleted and the unit includes include/Class876FC.h (`this` is `Class876FC *self`; `links` is `sprites`, `slot48` is `updateScale`, `angle` is `sprite.rotate`). Image byte-identical.
