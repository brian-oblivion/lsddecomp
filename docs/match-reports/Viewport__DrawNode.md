# Viewport__DrawNode -- MATCHED (round 81, alpha)

> Renamed from `Unk18Obj__DrawNode` on 2026-09-25 (tools/rename.py). Address 0x80012064.

> Renamed from `func_80012064` on 2026-09-25 (tools/rename.py). Address 0x80012064.

Unit `src/graphics/viewport_draw.c` (the unit's only function, 449 words, 0x80012064..0x80012768).
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

All types (`DrawNode`, `DrawView`, the Gs shapes) are local to `src/graphics/viewport_draw.c`. No header was
edited; `include/task.h` / `include/class_3bb8c.h` untouched as instructed.

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

Renamed from `Unk18Obj__DrawNode`. Slot +0x0A0 `drawNode`. `self` is now `Viewport *`: DrawView's names were carried into the header at their offsets (width/height -> screenSize.width/height, otLen -> otLength, buf -> otIndex; ot, projH, nearZ, zDiv unchanged). The node keeps viewport_draw's local DrawNode view, so the method is not prototyped in viewport.h. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.

## Unit history (moved from the code_2864.c banner, track 6 round 94)

- Carved 2026-09-25 (FINISHING-PLAN revision 18) as GAME code out of the segment then named
  `psyq_2864`, 0x2864..0x2F68: `tools/gameinsdk.py` measured it as game (a method-table entry
  beside game methods, calls into GetNextBasicClass and ApplyMatrixToLVArray, no Sony fingerprint).
- Matched round 81 (alpha). `self`'s local view DrawView was merged into include/Viewport.h in
  round 85.
- Round 94 (echo, track 6): the local `*_2864` copies of MATRIX, VECTOR, SVECTOR, GsCOORD2PARAM,
  GsCOORDINATE2, GsDOBJ2, GsSPRITE and GsBOXF were deleted in favour of Sony's `<libgte.h>` /
  `<libgs.h>`, with Sony's prototypes for RotMatrix and the Gs* calls; the tag-0x54 arm's
  `u8 bg[0x28]` became Sony's GsBG. Byte-exact unchanged. Two measured divergences from Sony's
  declarations survive in the source: the scale loop reads GsCOORD2PARAM.scale (Sony: `VECTOR`, signed
  long) through a `u32 *`, because retail shifts the products with `srl`; and the ordering-table
  arguments are cast to `GsOT *` because include/Viewport.h still carries its own ViewportOt view.

- Round 95 (delta, track 6): the local `DrawNode` view of the node (a BasicClass with a GsDOBJ2 at
  +0x010 and a union of the three tag-specific tails) was deleted. `node` is a `SceneNode *`, the
  type Viewport.h's drawNode slot already gave it, and each draw arm downcasts to the class its
  id names: 0x54 `BgLayer` (`bgAttribute` is the GsBG's first word), 0x64 `BoxFill` (`pri`,
  `relative`, `posX`/`posY`, `boxX`/`boxY`, GsBOXF at `boxAttribute`), 0x144 `ScreenSprite`
  (`screenPos.x`/`.y`, the old `ratioX`/`ratioY`), other 0x44 `Sprite` (`sprite`, a SpriteGs).
  Every field the local view named sits at the same offset in those headers, so no header was
  edited. The GsDOBJ2 is `(GsDOBJ2 *)&node->attribute` at the SortTmdObject call, as
  SceneNode__LinkModel spells it for GsLinkObject4; coord2 is cast to `GsCOORDINATE2 *` where
  its matrix or param is read (SceneNode.h's SceneNodeSub14 is still the parked view). Byte-exact
  on the first build.

Round 96 (alpha, track 6). include/Viewport.h's local `ViewportOt` (a
0x14-byte view of the GsOT header: length, org, pad) is deleted: `ot[2]` is
Sony's `GsOT *`, `otTags[2]` Sony's `GsOT_TAG *` (each header's `org`) and
`workBase[2]` Sony's `PACKET *` (GsSetWorkBase's argument), and every unit
including Viewport.h takes Sony's headers after common.h. The `(GsOT *)`
casts at GsClearOt, GsSortClear, GsDrawOt and drawNode's five sort calls and
Update's `(PACKET *)` cast are gone. Byte-identical (whole image green).

## Round 97 (alpha): Sony's GsCOORDINATE2

SceneNodeSub14 is deleted: SceneNode.coord2 is Sony's GsCOORDINATE2 (flg; MATRIX coord, whose t is the offset from the parent; MATRIX workm, whose t is the world position; param, super, sub -- 0x50 bytes, offset for offset). Accessors here follow the compiler's list: tx/ty/tz -> coord.t[0]/[1]/[2], unk38 -> workm.t; a local that holds coord.t or workm.t is `long *` (MATRIX.t is long[3]; s32 is int); any cast to GsCOORDINATE2 * is gone. Byte-identical.

## Round 99 (delta): track 7, moved from src/graphics/viewport_draw.c

Locals renamed for their roles (zero bytes): `c` -> `coord2`, `m` -> `elem`,
`sc` -> `scale`, `tag` -> `classId`, `b` -> `box`, the ScreenSprite `n` ->
`screenSprite` and `sp` -> `gsSprite`, `size` -> `screen`, the Sprite `n` ->
`worldSprite`. `scr` keeps its name (it has no role; typeviews' baselined
`unused variable` warning names it).

Constants (each byte-identical, whole image green): `0x24` ->
`GRIDCELL_CLASS_ID`; `& 0xF) == 4` -> `& CLASS_ID_ROOT_MASK) ==
SCENENODE_CLASS_ID`; `0x54`/`0x64`/`0x44`/`0x144` -> the new
`BGLAYER_CLASS_ID`/`BOXFILL_CLASS_ID`/`SPRITE_CLASS_ID`/`SCREENSPRITE_CLASS_ID`
(values from `plan.py classes`: gBgLayerMethods 0x54, gBoxFillMethods 0x64,
gSpriteMethods 0x44, gScreenSpriteMethods 0x144); `>> 12` -> `FIX12_SHIFT`
(GsCOORD2PARAM.scale is 20.12, ONE = unit); `(void *)0x1F800000` -> libetc's
`getScratchAddr(0)` (the scratchpad base, as Sony's samples pass
GsSortObject4); `14` -> unit-local `OTZ_BITS` (SortTmdObject indexes
`otBase[otz >> otShift]`, tmd_renderer.c, so `14 - otLength` spans a 14-bit
OTZ over 1 << otLength tags); `0x200` -> unit-local `SPRITE_POS_LIMIT 512`.
Left literal: the class-id nibble masks `0xFF`/`0xFFF` (no name in
basic_class.h beyond CLASS_ID_ROOT_MASK; proposed), the percent scale `100`
and `10000` (= 100 * 100, the ScreenSprite spelling of `half * pos / 100`),
and `0xFFFF`, the GTE's 16-bit screen-z bound, explained where it is used.

The extern `ApplyMatrixToLVArray(void *, void *, s32, void *)` stays in this
unit: viewport_draw does NOT include include/scene_node.h (round 98's note that it
did came from a grep matching this unit's comment `(include/scene_node.h)`),
so typing scene_node.h's prototype cannot collide here.

The unit banner, verbatim, as it was before this pass:

```c
/*
 * viewport_draw -- Viewport__DrawNode, the scene-graph walk that draws one node
 * and its drawable children into the Viewport's current ordering table
 * (vram 0x80012064..0x80012768).
 *
 * It is slot +0x0A0 (drawNode) of gViewportMethods, inherited unchanged by
 * gNodeGuardedViewportMethods (include/viewport.h); `self` is the Viewport
 * and `node` a SceneNode (include/scene_node.h). The class-id low byte picks
 * the draw path, and each path reads the node as the subclass that id names:
 * 0x54 a BgLayer (its GsBG at +0x044 to GsSortBg), 0x64 a BoxFill (its GsBOXF
 * placed in percent of the half-screen while `relative` is set), 0x144 a
 * ScreenSprite (its GsSPRITE placed from `screenPos`), any other 0x44 a
 * Sprite projected from its GsCOORDINATE2's world position, and anything
 * else the node's own GsDOBJ2 (+0x010) sorted by SortTmdObject
 * (tmd_renderer.c), the game's replacement for GsSortObject4. A GridCell
 * (0x24) whose GsDOFF bit is set is skipped outright.
 *
 * Before drawing, a node whose coord2 is dirty (flg == 0) rebuilds its matrix
 * from GsCOORD2PARAM's rotate and scale and marks its children dirty;
 * children (class-id low nibble 4, parent == node) are drawn first, so the
 * order in the OT is children before parent.
 *
 * Types are Sony's (libgte.h, libgs.h) and the classes' own. SceneNode's
 * coord2 is Sony's GsCOORDINATE2; the subclasses spell their
 * GsBG/GsBOXF/GsSPRITE field by field: hence the casts to Sony's types at
 * the libgs calls. The OT is Viewport's own GsOT.
 */
```

The function comment it replaced, verbatim (the source-shape notes now sit
as one `/* MATCHING: */` line at each construct; the measurements behind
them are "Levers" above):

```c
/*
 * Draw `node` into self's current ordering table, after first drawing every
 * child whose class-id low nibble is 4 and whose parent is `node`.
 *
 * Source-shape notes (all measured; see the match report):
 *  - `ls`/`lw` are explicit pointers to the two stack matrices: retail keeps
 *    &lsBuf/&lwBuf live in s5/s7 from the prologue on.
 *  - `pos` and the unused `scr` live in the world-space-sprite block: that
 *    puts them ABOVE child/cursor in the frame, and `scr` is the 8 bytes of
 *    frame retail reserves and never touches.
 *  - `b`/`n` are separate copies of `node` (retail's move a3/a1/a0,s2);
 *    the ratio ternaries are one store each (the second copy of the store
 *    is the delay-slot filler's); `~v + 1` is retail's nor/addiu negate.
 */
```

The extern's comment was `/* code_d294_c.c (include/scene_node.h) */`, and `scr`'s was `/* never used; reserves retail's 8 unused frame bytes */`.

## History (moved from src/ViewportDraw.c, comments pass)

The file's banner carried its edge evidence and the reason for its name:

> Edges: the file is exactly this one function, between two linked Sony
> objects (_obj/malloc before it, libapi/c159 after it), so the binary puts
> both edges there and there is no neighbouring game unit to merge with. The
> rest of the Viewport class is in other files, well away from this one.
> This file is named for what it holds, the Viewport's scene-graph draw, and
> leaves the Viewport stem to the class header, include/Viewport.h.
