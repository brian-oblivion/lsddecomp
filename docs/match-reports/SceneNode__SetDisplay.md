# SceneNode__SetDisplay

> Renamed from `Class6B5CC__SetDisplay` on 2026-09-26 (tools/rename.py). Address 0x8001d344.

> Renamed from `func_8001D344` on 2026-09-23 (tools/rename.py). Address 0x8001d344.

**Unit:** SceneNode · **Size:** 12 words · **Status:** MATCHED (12/12 words)

## What it does

`SceneNode` vtable slot `+0x060`. Sets bit 31 (a 1-bit field) of
`self->unk10` to `(a1 == 0)`, and returns whether the field's PREVIOUS
value was 0 (i.e. it returns the logical negation of the old bit).

See `include/scene_node.h` for `GetSetBitField`, the generic packed-bitfield
accessor all five sibling functions in this file (`SceneNode__SetDisplay`,
`SceneNode__SetSemiTrans`, `SceneNode__SetSemiTransRate`, `SceneNode__SetLighting`, `SceneNode__SetLightMode`) wrap.
It lives in the next, still-uncarved slice (`asm/SceneNode.s`) and was
read directly off its own disassembly rather than decompiled here.

## The C

```c
s32 SceneNode__SetDisplay(SceneNodeObj *self, s32 a1) {
    return GetSetBitField(&self->unk10, 0x1F, 1, a1 == 0) == 0;
}
```

`a1 == 0` compiles to `sltiu $r,$r,1` (the documented "sltiu $r,$r,1 is
x==0" lowering, DECOMPILATION_LEARNINGS.md), matching retail's
`sltiu $a3,$a1,0x1` exactly. Same idiom for the trailing
`== 0` on the return value (`sltiu $v0,$v0,0x1`).

## Provenance

round 11 (2026-09-03), runner charlie, unit SceneNode (fresh carve, first attempt).
Matched on the first build; the five bitfield-setter siblings and the
struct fields around them (`self->unk10` as a packed flags word,
`GetSetBitField`'s signature) were derived together as one group before any
of the five were individually verified — see `New_SceneNode.md` and
`SceneNode__SceneNode.md` for the ctor/dtor work that established `SceneNodeObj`
first.

## Proposed learning

`GetSetBitField` (bit-packed word accessor: given `word, shift, width,
value`, clears `width` bits at `shift`, ORs in `value << shift`, returns
the previous bitfield value shifted back to bit 0) recurs as a project-wide
idiom for this game's flag words — worth grepping for the same
call-shape (`addiu $a0,$a0,N; li $a1,shift; li $a2,width; ...; jal`)
elsewhere before re-deriving it from scratch.

## Naming

Round 71 (alpha). `func_8001D344` -> `SceneNode__SetDisplay`, **tier A**. Table slot +0x060. GetSetBitField(&attribute, 31, 1, on == 0): bit 31 of GsDOBJ2.attribute is GsDOFF (LIBGS.H), so on=0 hides the object; returns the previous display state (old DOFF == 0). DreamSys calls it with 0; dream_scene calls the slot setDisplay.

## Proposed field names

For the head to apply by type scope. Each one fails to compile in another unit when renamed in the definition, so this unit did not apply it.

- `SceneNodeObj.unk10` -> `attribute` (tier A): GsDOBJ2.attribute. SceneNode__LinkModel passes `&self->unk10` to GsLinkObject4 as the GsDOBJ2, and the five setters here write it at GsDOFF/GsALON/GsA*/GsLOFF/light-mode bit positions. Accessors: SceneNode, code_d294_b, code_d294_c.

## Round 101 (delta): track 7

Step 3 (locals and parameters): `a1` -> `on` (written inverted into GsDOFF, so nonzero means displayed). Byte-identical.

Step 5 (comments): Function comment added (the double inversion over GsDOFF).
