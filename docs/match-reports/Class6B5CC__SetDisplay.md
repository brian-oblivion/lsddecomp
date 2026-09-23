# Class6B5CC__SetDisplay

> Renamed from `func_8001D344` on 2026-09-23 (tools/rename.py). Address 0x8001d344.

**Unit:** code_d294 · **Size:** 12 words · **Status:** MATCHED (12/12 words)

## What it does

`Class6B5CC` vtable slot `+0x060`. Sets bit 31 (a 1-bit field) of
`self->unk10` to `(a1 == 0)`, and returns whether the field's PREVIOUS
value was 0 (i.e. it returns the logical negation of the old bit).

See `include/code_d294.h` for `GetSetBitField`, the generic packed-bitfield
accessor all five sibling functions in this file (`Class6B5CC__SetDisplay`,
`Class6B5CC__SetSemiTrans`, `Class6B5CC__SetSemiTransRate`, `Class6B5CC__SetLighting`, `Class6B5CC__SetLightMode`) wrap.
It lives in the next, still-uncarved slice (`asm/code_d294_b.s`) and was
read directly off its own disassembly rather than decompiled here.

## The C

```c
s32 Class6B5CC__SetDisplay(Class6B5CCObj *self, s32 a1) {
    return GetSetBitField(&self->unk10, 0x1F, 1, a1 == 0) == 0;
}
```

`a1 == 0` compiles to `sltiu $r,$r,1` (the documented "sltiu $r,$r,1 is
x==0" lowering, DECOMPILATION_LEARNINGS.md), matching retail's
`sltiu $a3,$a1,0x1` exactly. Same idiom for the trailing
`== 0` on the return value (`sltiu $v0,$v0,0x1`).

## Provenance

round 11 (2026-09-03), runner charlie, unit code_d294 (fresh carve, first attempt).
Matched on the first build; the five bitfield-setter siblings and the
struct fields around them (`self->unk10` as a packed flags word,
`GetSetBitField`'s signature) were derived together as one group before any
of the five were individually verified — see `New_Class6B5CC.md` and
`Class6B5CC__Class6B5CC.md` for the ctor/dtor work that established `Class6B5CCObj`
first.

## Proposed learning

`GetSetBitField` (bit-packed word accessor: given `word, shift, width,
value`, clears `width` bits at `shift`, ORs in `value << shift`, returns
the previous bitfield value shifted back to bit 0) recurs as a project-wide
idiom for this game's flag words — worth grepping for the same
call-shape (`addiu $a0,$a0,N; li $a1,shift; li $a2,width; ...; jal`)
elsewhere before re-deriving it from scratch.

## Naming

Round 71 (alpha). `func_8001D344` -> `Class6B5CC__SetDisplay`, **tier A**. Table slot +0x060. GetSetBitField(&attribute, 31, 1, on == 0): bit 31 of GsDOBJ2.attribute is GsDOFF (LIBGS.H), so on=0 hides the object; returns the previous display state (old DOFF == 0). DreamSys calls it with 0; class_3bb8c_s calls the slot setDisplay.
