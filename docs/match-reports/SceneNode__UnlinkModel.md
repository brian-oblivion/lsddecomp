# SceneNode__UnlinkModel -- MATCHED (3/3 words)

> Renamed from `Class6B5CC__UnlinkModel` on 2026-09-26 (tools/rename.py). Address 0x8001e7b0.

> Renamed from `func_8001E7B0` on 2026-09-17 (tools/rename.py). Address 0x8001e7b0.

Unit: `code_d294` (round 14, first slice-3 carve). Called by
`SceneNode__RemoveAllChildren` (`code_d294.c`) as its forward target for
`SceneNodeMethods::slot18`. `void SceneNode__UnlinkModel(SceneNodeObj *self)`.

## Final source

```c
void SceneNode__UnlinkModel(SceneNodeObj *self) {
    self->unk18 = 0;
    self->unk20 = 0;
}
```

## Derivation notes

Already MEASURED and documented verbatim in `include/code_d294.h`'s own
standing comment before this carve existed ("SceneNode__UnlinkModel's whole body is
`self->unk18 = 0; self->unk20 = 0;`") -- this round only had to move the
body from documentation into `src/code_d294.c` and confirm it still
matches now that the function has its own real address. No residue.
Comment in the header updated (round 14) to say "now carved" instead of
"still uncarved" since the function moved out of `asm/` into this unit;
no type or prototype changed.

No new struct or vtable-slot knowledge.

## Naming (round 50, charlie -- FINISHING-PLAN track 3)

- **`func_8001E7B0` -> `SceneNode__UnlinkModel`. Tier B.** It zeroes
  exactly the two fields `SceneNode__LinkModel` sets, and only those: the
  GsDOBJ2's `tmd` pointer at +0x18 and the source object at +0x20. The name
  is the inverse of its pair BY CONSTRUCTION, not because any Sony call
  pins it -- there is no GS call here at all, which is why this is B where
  `SceneNode__LinkModel` is A.
- Its one dispatch path supports the pairing: `SceneNode__RemoveAllChildren` (code_d294.c)
  forwards `SceneNodeMethods::slot18` straight into it, while
  `SceneNode__AddChild` -- the +0x010 slot -- is what forwards into
  `SceneNode__LinkModel`.

## Round 98 (echo): track 7, moved from src/code_d294.c

The source comment was rewritten as documentation; the one it replaced, verbatim (field names as they were then):

```c
/* Clears exactly the two fields SceneNode__LinkModel sets: the GsDOBJ2's
 * tmd pointer (+0x18) and the source object (+0x20). No GS call -- the
 * pairing with LinkModel is by construction, not by a Sony API. */
```
