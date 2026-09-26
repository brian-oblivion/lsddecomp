# Viewport__DrawNode -- MATCHED (round 81, alpha)

> Renamed from `Unk18Obj__DrawNode` on 2026-09-25 (tools/rename.py). Address 0x80012064.

> Renamed from `func_80012064` on 2026-09-25 (tools/rename.py). Address 0x80012064.

Unit `src/code_2864.c` (the unit's only function, 449 words, 0x80012064..0x80012768).
Byte-exact, whole-image SHA1 green (`./build-and-verify.sh`: OK), `tools/check-nonmatching.sh` green.

## What it is

Slot +0x0A0 of Unk18Obj's method table (`tools/classtable.py gViewportMethods`; the same entry is
inherited by `gNodeGuardedViewportMethods`). `self` is the Unk18Obj view/renderer, `node` a scene node.
It draws `node` into `self->ot[self->buf]` (the GsOT pointers at +0x78, index at +0x74):

1. Early out: class-id low byte 0x24 with the GsDOBJ2 attribute's sign bit (GsDOFF) set.
2. If the node's GsCOORDINATE2 `flg` is 0: `RotMatrix(&param->rotate, &coord)`, then scale
   each row of `coord.m` by `param->scale` (unsigned, `>> 12`), and remember to dirty children.
3. Walk `node->children` with `GetNextBasicClass`, and for every child whose class id low nibble
   is 4 and whose +0x00C parent is `node`: zero its coord2 `flg` if dirty, then recurse.
4. Dispatch on the class-id low byte: 0x54 `GsSortBg(node+0x44, ot, (1<<otLen)-1)`;
   0x64 box fill (optionally percent-of-half-screen positioned) through `GsSortBoxFill`;
   anything but 0x44 a model (`GsGetLws`, `GsSetLightMatrix`, `GsSetLsMatrix`, then
   `SortTmdObject(&obj, ot, 14 - otLen, scratchpad 0x1F800000)` if it has a TMD);
   0x144 a screen-space sprite; other 0x44 a world-space sprite (`GsGetLs`, depth range check,
   optional `ApplyMatrixToLVArray` offset when the parent has a parent, perspective divide by
   `self->projH`, clamp to +-0x200, `GsSortSprite` with a depth-derived priority).

All types (`DrawNode`, `DrawView`, the Gs shapes) are local to `src/code_2864.c`. No header was
edited; `include/code_2cc8c.h` / `include/class_3bb8c.h` untouched as instructed.

## Path to the match (build scores)

| step | score | change |
| --- | --- | --- |
| 1 | 19/449, 24 words short | straightforward transcription |
| 2 | 150/449 | `MATRIX *ls = &lsBuf; *lw = &lwBuf;` explicit pointers (retail holds them in s5/s7) |
| 3 | 226/449, 2 words short | `VECTOR pos` moved into the world-sprite block (frame order); `DrawNode *b = node` / `*n = node` copies |
| 4 | 427/449, exact length | screen-sprite x/y as ONE ternary store each |
| 5 | 429/449 | `pos.vz > self->nearZ` (operand order sets load order) |
| 6 | 449/449 | unused `SVECTOR scr;` after `pos` in the same block (8 frame bytes) |

Six builds; no permuter search was spent.

## Levers (all measured here)

- **Callee-saved register holding `&stackStruct` from the prologue** = an explicit pointer local
  (`MATRIX *ls = &lsBuf;`). Reads of the struct that retail makes as `N(sp)` rather than
  `N(s5)` are spelled through the buffer directly (`lsBuf.t[0]`); the one `0x1C(s5)` read is
  `ls->t[2]`.
- **Frame order**: address-taken scalars of the outer block (`child`, `cursor`) sat at 0x50/0x54
  BELOW `pos` at 0x58; declaring the aggregate in the inner block that uses it moves it above them.
- **Unused frame bytes** at the top of the locals area (frame 0x98 vs 0x90, every save slot +8)
  = an unused 8-byte aggregate (`SVECTOR`) declared after `pos` in the same block. Consistent
  with the existing learning "an unused stack frame is reserved by an unused local ARRAY".
- **`move v0,zero; sh v0,N(a2)` in one arm and `j L; sh v0,N(a2)` in the other** = ONE store
  of a ternary `x = c ? e : 0;`; the if/else spelling stores `$zero` directly and is 2 words short.
- **`move a3,s2` / `move a1,s2` / `move a0,s2` just before a branch body** = a separate local
  copy of the pointer (`DrawNode *b = node;`); cse rewrites uses inside the extended basic block
  back to `s2`, so only the uses after the join keep the copy's register.
- **`nor v0,zero,x; addiu v0,v0,1`** (not `negu`) = the source wrote `~x + 1`.
- **`addiu v1,v1,-1; sltu` against 0xFFFE** = the range test `t < 1 || t > 0xFFFF`.
- `(u8)methods->header == 0x24` compiles to `lbu` of the header word; `(header & 0xFF)` of an
  already-loaded word stays `andi`.

## Naming

Renamed `func_80012064` -> `Unk18Obj__DrawNode` (round 81, charlie), tier B: confirmed as slot
+0x0A0 of Unk18Obj's method table (`tools/classtable.py gViewportMethods`, inherited by
`gNodeGuardedViewportMethods`), behaviour (draws one scene node and its 0x4-nibble children into the
current OT) read from the body. `Unk18Obj` itself is still a placeholder class name -- the
prefix is inherited, not re-derived here -- so the method name is tier B rather than A.

### Proposed learning

A dead 8-byte aggregate in the INNER block reproduces unused frame bytes that sit ABOVE an
inner-block aggregate; placement of the filler follows the same block rule as the live
aggregate (outer-block address-taken scalars first, then inner-block aggregates in
declaration order).

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__DrawNode`. Slot +0x0A0 `drawNode`. `self` is now `Viewport *`: DrawView's names were carried into the header at their offsets (width/height -> screenSize.width/height, otLen -> otLength, buf -> otIndex; ot, projH, nearZ, zDiv unchanged). The node keeps code_2864's local DrawNode view, so the method is not prototyped in Viewport.h. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.
