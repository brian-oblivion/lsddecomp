# SceneNode__SceneNode

> Renamed from `Class6B5CC__Class6B5CC` on 2026-09-26 (tools/rename.py). Address 0x8001caf4.

> Renamed from `func_8001CAF4` on 2026-09-23 (tools/rename.py). Address 0x8001caf4.

**Unit:** code_d294 · **Size:** 44 words · **Status:** MATCHED (44/44 words)

## What it does

`SceneNode`'s own constructor — vtable slot `+0x008` of `gSceneNodeMethods`
(confirmed directly: `tools/classtable.py gSceneNodeMethods` names `SceneNode__SceneNode`
as the occupant of that slot). Takes an already-allocated `self` (the
allocation itself is `New_SceneNode`, a separate `New_X` wrapper, not this
function). Allocates two sub-blocks (`self->unk14`, 0x50 bytes, then
`self->unk14->unk44`, 0x28 bytes), frees the first and bails out if the
second allocation fails, otherwise calls the BasicClass base constructor
(`Get_vtable_BasicClass()->ctor(self)`), overwrites `self->methods` with this
class's own vtable (`GetSceneNodeMethods()`, i.e. `&gSceneNodeMethods`), zeroes several
freshly-added fields, and finally calls its own virtual init hook
(`self->methods->slot40`, `SceneNode__Reset` — still queued) before returning
`self` unconditionally.

## The C

```c
void *SceneNode__SceneNode(SceneNodeObj *self) {
    void *blockB;

    self->unk14 = BMemPMgrAlloc(0x50);
    if (self->unk14 == NULL) {
        return NULL;
    }
    blockB = BMemPMgrAlloc(0x28);
    self->unk14->unk44 = blockB;
    if (blockB == NULL) {
        BMemPMgrFree(self->unk14);
        return NULL;
    }
    Get_vtable_BasicClass()->ctor(self);
    self->methods = GetSceneNodeMethods();
    self->unk20 = 0;
    self->unk18 = 0;
    self->unkC = NULL;
    self->unk14->unk48 = 0;
    self->methods->slot40(self);
    return self;
}
```

## Two source-shape lessons, both confirmed by measured attempts

- **Do not cache a re-read field in a local across a call sequence.** A
  first attempt cached `self->unk14` once into a local `sub` and reused it
  for both `sub->unk44 = ...` and, much later (after 3 intervening calls),
  `sub->unk48 = 0`. That forced the compiler to promote the cache to a
  callee-saved register (`$s1`), which changed the frame size by 8 bytes
  (`-0x18` retail vs `-0x20` mine) and shifted every following byte in the
  whole ROM image — the classic address-drift failure mode. Writing
  `self->unk14->unk44 = ...` and `self->unk14->unk48 = 0` as two
  independent re-derivations (no cached local surviving the calls between
  them) let the compiler use a cheap caller-saved `$v1`, reloaded fresh
  right where retail reloads it, and the frame size matched immediately.
  This is the "mention the value multiple times, don't cache across a
  call" idiom, but for a struct-field re-read rather than a redundant
  `move` — worth adding to the general pattern in
  DECOMPILATION_LEARNINGS.md if a second instance turns up.
- **Instruction ORDER (store vs. call-argument setup) around a `jal`'s
  delay slot is source-order-sensitive in a way that isn't just "which
  statement is textually first".** `self->unk14 = BMemPMgrAlloc(0x50);`
  compiled with the STORE and the FOLLOWING call's `li $a0` juggled by the
  scheduler; the version that matched byte-for-byte was writing the
  allocation call directly into the assignment
  (`self->unk14 = BMemPMgrAlloc(0x50);`) rather than staging it through an
  intermediate local (`blockA = BMemPMgrAlloc(0x50); ...; self->unk14 =
  blockA;`), even though both are logically identical. Prefer assigning a
  struct-field/global destination directly from the call expression over
  round-tripping it through a temporary, when a residue looks like two
  independent instructions swapped around a `jal`.

## Provenance

round 11 (2026-09-03), runner charlie, unit code_d294 (fresh carve, first attempt).
Both fixes above were found within the 30-attempt budget (2 rebuild
iterations total).

## Naming

Round 71 (alpha). `func_8001CAF4` -> `SceneNode__SceneNode`, **tier A**. Table slot +0x008 (ctor) of gSceneNodeMethods. Allocates the 0x50-byte GsCOORDINATE2 and 0x28-byte GsCOORD2PARAM, runs the BasicClass ctor, installs gSceneNodeMethods, zeroes fields, then calls `reset`. `Class__Class` constructor convention (BasicClass__BasicClass).

## Proposed field names

For the head to apply by type scope. Each one fails to compile in another unit when renamed in the definition, so this unit did not apply it.

- `SceneNodeObj.unk14` -> `coord2` (tier A): the ctor allocates exactly sizeof(GsCOORDINATE2) = 0x50 for it and Reset runs GsInitCoordinate2 on it; +0x14 of the embedded GsDOBJ2 is `coord2` in LIBGS.H. Accessors: code_d294, code_d294_b, code_d294_c (compiler-measured).
- `UnkOwner_d294.unk14` and `GenericObj_d294.unk14` -> `coord2` (tier A): the same field on the parent and on a sibling object (AttachToParent copies the parent's into `super`). Accessors: code_d294, code_d294_c.
- `SceneNodeSub14.unk44` -> `param` (tier A): GsCOORDINATE2.param, the 0x28-byte GsCOORD2PARAM the ctor allocates. Accessors: code_d294, code_d294_b, code_d294_c.

## Track 6 (round 91, echo): the class is SceneNode

Class6B5CC -> SceneNode (`renametype.py`), tier A for what it is: the ctor allocates a GsCOORDINATE2 (0x50) and a GsCOORD2PARAM (0x28) and the object embeds a GsDOBJ2 at +0x010 (SceneNode__LinkModel passes &attribute to GsLinkObject4); attachToParent/detachFromParent maintain `parent` and coord2->super, a libgs transform hierarchy; sixteen classes derive from it (Actor, Sprite, LightRig, BoxFill, ...). The libgs member types (SceneNodeSub14 = GsCOORDINATE2, SceneNodeSub44 = GsCOORD2PARAM, S16Quad_d294 = SVECTOR) are not yet substituted: including <libgs.h> in SceneNode.h breaks 24 units whose headers declare Sony names their own way (measured this round).


## Round 95 (bravo): moved from include/code_d294.h

The header's banner was rewritten as documentation in round 95; the comments it carried about SceneNode and its libgs types, verbatim:

```c
/* code_d294, code_d294_b, code_d294_c: the methods of SceneNode, whose one
 * definition (object, method table, getter, method prototypes) is
 * include/SceneNode.h since track 4 (round 81). This header keeps what only
 * these three units use: the bounds-box helpers' types and the prototypes of
 * the Psy-Q and utility functions the methods call.
 *
 * GetSceneNodeMethods is a plain no-argument getter (`lui/addiu
 * %hi/%lo(gSceneNodeMethods); jr $ra`). Several units used to call it with
 * one or two arguments through an unprototyped declaration; round 59 measured
 * every one of those arguments as zero-cost (the `jal`'s delay slot holds a
 * callee-save spill), and track 4 dropped them. */

/* ================= PSY-Q IDENTIFICATION (round 50, charlie) =================
 *
 * SceneNode is a POSITIONED 3D OBJECT class built directly on libgs's own
 * scene-graph types, and three of the structs below are Sony's, reached
 * under this project's own placeholder names. This is offset arithmetic
 * against include/psyq/libgs.h, not a resemblance argument -- every field
 * already recorded below lands where Sony's does, and the two sizes the
 * ctor allocates (0x50 and 0x28) are the two Sony struct sizes exactly.
 *
 *   SceneNodeSub14  ==  GsCOORDINATE2   (0x50 bytes)
 *     +0x00 unk0   == flg     the "matrix needs recomputing" flag; already
 *                             documented below as a pending-update flag
 *     +0x04        == coord   MATRIX, 0x20 bytes (s16 m[3][3], pad, s32 t[3])
 *     +0x18 unk18/unk1C/unk20 == coord.t[0..2]   (+0x04 + 0x14 = +0x18)
 *     +0x24 unk24  == workm   MATRIX, the COMPOSED world matrix; already
 *                             recorded below as an address-only span
 *     +0x38 unk38[3] == workm.t[0..2]            (+0x24 + 0x14 = +0x38)
 *     +0x44 unk44  == param   GsCOORD2PARAM *
 *     +0x48 unk48  == super   the PARENT coordinate; SceneNode__AttachToParent's
 *                             "attach" sets it to the owner's own unk14,
 *                             which is exactly what super means
 *     +0x4C        == sub     (not yet touched by any carved function)
 *
 *   SceneNodeSub44  ==  GsCOORD2PARAM   (0x28 bytes)
 *     +0x00 unk0/unk4/unk8 == scale.vx/vy/vz (VECTOR, +0x0C is its pad)
 *     +0x10 vec            == rotate        (SVECTOR; x/y/z/w == vx/vy/vz/pad)
 *     +0x18 pad18          == trans         (VECTOR)
 *   so S16Quad_d294 is an SVECTOR, and the 4096-per-turn angle reading
 *   SceneNode__UpdateRotation's full-turn wrap already established is Sony's own.
 *
 *   SceneNode +0x10 .. +0x1C  ==  an embedded GsDOBJ2
 *     +0x10 unk10 == attribute  (the packed flags word the GetSetBitField
 *                                family sets fields in)
 *     +0x14 unk14 == coord2     (the GsCOORDINATE2 above)
 *     +0x18 unk18 == tmd        (the model data pointer)
 *   SceneNode__LinkModel passes `&self->unk10` to Sony's GsLinkObject4 as
 *   its GsDOBJ2 argument, which only type-checks at this layout, and the
 *   ctor calls GsInitCoordinate2 on the 0x50-byte block.
 *
 * TRACK 4 (round 81, include/SceneNode.h) named the GsDOBJ2 words on the
 * object (attribute, coord2, tmd, id) and GsCOORDINATE2's super/sub, but kept
 * the project's own SceneNodeSub14/SceneNodeSub44 types and their field
 * names (tx/ty/tz, unk24, unk38, param, rotate): retyping them to LIBGS.H's
 * GsCOORDINATE2/GsCOORD2PARAM re-paths every accessor (`coord.t[0]` for `tx`,
 * `workm.t` for `unk38`) in the class's units and in the subclass views that
 * reach these blocks. A later pass can do it with the offsets above.
 * ========================================================================= */

/* The model SceneNode keeps at +0x020 is a TmdModel (TmdModel). Its
 * methods -- TmdModel__GetHull, TmdModel__GetBoundsCount,
 * TmdModel__UpdateBoundsBuffer/GetBoundsBuffer, TmdModel__RaycastFaces --
 * are declared once, in include/TmdModel.h, which code_d294_b.c and
 * code_d294_c.c include themselves (track 4, round 87). */
```

## Round 97 (alpha): Sony's types substituted

SceneNode.h now uses Sony's types for all three: S16Quad_d294 -> SVECTOR (x/y/z/w -> vx/vy/vz/pad), SceneNodeSub44 -> GsCOORD2PARAM (scaleX/Y/Z -> scale.vx/vy/vz; rotate), SceneNodeSub14 -> GsCOORDINATE2 (pad04 -> coord.m, tx/ty/tz -> coord.t[0..2], workm bytes + unk38 -> workm.m + workm.t; flg is unsigned long now, compared only against 0). Each layout was re-checked against include/psyq/libgte.h and libgs.h before its struct was deleted; the table above stands. Every includer of SceneNode.h takes <libgte.h>, <libgpu.h>, <libgs.h> after common.h. Zero bytes.
