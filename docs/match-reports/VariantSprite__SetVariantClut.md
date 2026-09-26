# VariantSprite__SetVariantClut -- MATCHED (14/14)

> Renamed from `Class879C4__SetVariantClut` on 2026-09-26 (tools/rename.py). Address 0x80057dbc.

> Renamed from `D800879C4__SetVariantClut` on 2026-09-26 (tools/rename.py). Address 0x80057dbc.

> Renamed from `func_80057DBC` on 2026-09-25 (tools/rename.py). Address 0x80057dbc.

Round 47 (runner charlie). Unit: `src/class_3bb8c_q.c`, a BRAND NEW carve
(this unit did not exist before round 47). Class: table `gVariantSpriteMethods` (49
slots, resolved with `tools/classtable.py 0x800879C4`) -- this is slot40,
the function tail-called by this class's own ctor `VariantSprite__VariantSprite` (see
that report, `src/class_3bb8c_p.c`) as `self->methods->slot40(self,
arg1)` once construction is otherwise complete.

Frameless leaf, zero `addiu $sp, $sp, -N`. The unit's carve-note history
had this filed as `addiu_at`-blocked; that construct was RESOLVED in
round 21 (`--addiu-at`), and `tools/uncarved.py` measures this function
blocker-clean. Matched first attempt, no iteration needed.

## Signature

```c
void VariantSprite__SetVariantClut(D_800879C4Obj_q *self, s32 arg1);
```

`D_800879C4Obj_q` is THIS unit's own local view of the class -- the
neighbouring `class_3bb8c_p.c` already carries its own, smaller local
view (`D_800879C4Obj`, only the vtable pointer and `+0xA4`) of the SAME
table/object, established from `VariantSprite__VariantSprite`/`New_VariantSprite`. Per the
project's multiple-independent-local-views convention this unit does not
edit that file; it defines its own struct sized for what these two
functions read/write. The allocator (`class_3bb8c_p.c`'s `New_VariantSprite`)
sizes the object at `0xA8` bytes, which every offset in both views stays
inside.

## Body

```c
typedef struct D_800879C4Obj_q D_800879C4Obj_q;
struct D_800879C4Obj_q {
    u8 pad00[0x58];
    s32 unk58;                  /* +0x058 */
    s32 unk5C;                  /* +0x05C */
    s32 unk60;                  /* +0x060 */
    u8 pad64[0x74 - 0x64];
    s16 unk74;                  /* +0x074 */
    s16 unk76;                  /* +0x076 */
    u8 pad78[0x80 - 0x78];
    s16 unk80;                  /* +0x080 */
    s16 unk82;                  /* +0x082 */
    u8 pad84[0xA0 - 0x84];
    s32 unkA0;                  /* +0x0A0 */
};

extern const s16 gVariantSpriteClutX[];
extern const s16 gVariantSpriteClutY[];

void VariantSprite__SetVariantClut(D_800879C4Obj_q *self, s32 arg1) {
    self->unkA0 = arg1;
    self->unk74 = gVariantSpriteClutX[arg1 * 2];
    self->unk76 = gVariantSpriteClutY[arg1 * 2];
}
```

## The rodata shape

`gVariantSpriteClutX` and `gVariantSpriteClutY` are splat's own dlabels in
`asm/data/76DC8.data.s`, 2 bytes and 6 bytes long respectively, sitting
back to back (`gVariantSpriteClutX` at `0x80087AA4`, `gVariantSpriteClutY` immediately
after at `0x80087AA6`) -- conceptually one 8-byte array of two
`{s16 a; s16 b;}` entries, but retail takes TWO SEPARATE `%hi`/`%lo`
bases (one lui/addiu pair per table) rather than a single struct-array
base, so declaring one combined struct type would not reproduce the two
relocations splat already emitted. The only spelling that does is two
parallel `s16[]` externs, each indexed at `arg1 * 2` (i.e. `arg1 * 4`
bytes -- retail computes the shift once, `sll $a1, $a1, 2`, and reuses it
for both address calculations, which the two-array C form reproduces
without any extra hoisting). Values, read at the real stride:
`gVariantSpriteClutX = {0x03D0, 0x03E0}`, `gVariantSpriteClutY = {0x01FF, 0x01FF}`.

## Verification

`./build-and-verify.sh` -- whole-image SHA1 matches retail. Committed
alongside `VariantSprite__UpdateScale` (same unit, same commit, ROM-address order
preserved).

### Proposed learning

Two splat dlabels that are contiguous in memory and individually shorter
than the addressing stride the code actually uses (`gVariantSpriteClutX` is 2
bytes but addressed at a 4-byte stride) are not necessarily one struct
array that a single C declaration should unify -- check whether the
disassembly takes ONE relocation or TWO before merging them. Here it's
two, so the correct C is two parallel arrays over one strided index, not
one struct-array declaration.

## Naming

Round 79 naming pass (runner echo). The body above is the round-47 match;
the live source now spells the struct `D800879C4Obj` with the field names
below, byte-identical.

| name | tier | evidence |
| --- | --- | --- |
| `VariantSprite__SetVariantClut` (was `func_80057DBC`) | A | Pure leaf whose mechanics are its purpose: stores its argument at `+0xA0` and loads `+0x74`/`+0x76` from a two-entry table indexed by it. `+0x64` is an embedded GsSPRITE: the base class's init `InitGsSprite` (asm/psyq_322b4.s) writes attribute `+0`, w/h `+8`/`+A`, tpage `+C` (from `GetTPage`), u/v `+E`/`+F`, cx/cy `+0x10`/`+0x12` (from the image), r,g,b = 0x80 at `+0x14..0x16`, mx/my = w/2,h/2 at `+0x18`/`+0x1A`, scalex/scaley = 0x1000 at `+0x1C`/`+0x1E`, rotate = 0 at `+0x20`, all at GsSPRITE offsets relative to `+0x64`; `Viewport__DrawNode` (asm/psyq_2864.s) passes `self+0x64` to `GsSortSprite` for tag-0x44 objects (header 0x1F44). So `+0x74`/`+0x76` are GsSPRITE.cx/cy. The values {0x3D0, 0x1FF} and {0x3E0, 0x1FF} are 16-aligned VRAM x on the bottom line, which is where CLUTs go. "Variant": the same argument is the ctor's arg1 (`VariantSprite__VariantSprite`), which also selects the texture cell `&gVariantSpriteCells[arg1]` (u,v = (0x00,0x20) or (0x10,0x20), 16x16) handed to the base ctor. Slot `+0x040` per `tools/classtable.py gVariantSpriteMethods`. |
| class prefix `D800879C4` | -- | The prefix the tree already uses for this table's class (`VariantSprite__VariantSprite`, `New_VariantSprite`, class_3bb8c_p.c's `D800879C4Obj`/`D800879C4Methods`); `classtable.py` resolves slots `+0x040` and `+0x048` of `gVariantSpriteMethods` to these two functions. No evidence yet for a game-level class name. |
| `gVariantSpriteClutX` / `gVariantSpriteClutY` (were `D_80087AA4` / `D_80087AA6`) | A | Read only here, into GsSPRITE.cx / .cy. Really one `{s16 x, y}[2]` array; retail takes two relocations, so it stays two externs (see "The rodata shape"). |
| field `spriteClutX` / `spriteClutY` (`+0x74`/`+0x76`) | A | GsSPRITE.cx/cy, above. Unit-local struct: renamed in place. |
| field `variant` (`+0xA0`) | B | Mechanics certain (the index that picks the CLUT and, in the ctor, the texture cell); no reader of `+0xA0` on this class found (`Viewport__DrawNode` reads `+0xA0` only on tag 0x144 objects, a different class). |

## Proposed field names

- `class_3bb8c_p.c`, `D800879C4Methods::postConstruct` (`+0x040`) -> `setVariantClut`, tier A: the slot resolves to this function (`tools/classtable.py gVariantSpriteMethods`), and slots are named like the method they dispatch to. Only that one local view; do NOT touch `GraphRoomMethods::postConstruct` in class_3bb8c_t.c, which is a different class's slot. Not applied here (another unit's file).

## Track 4 (2026-09-26, round 87, alpha)

Renamed from `D800879C4__SetVariantClut` (tools/rename.py); the class is
`VariantSprite` (`include/VariantSprite.h`). It occupies the inherited `reset`
slot (+0x040) and keeps its own name: the body does something narrower and
different from Sprite__Reset (it never binds a texture; it records the
variant and repoints the CLUT), so the slot-name rule's exception applies.
The slot keeps SceneNode's type; the ctor calls this through
`VariantSpriteResetFn` because it passes the variant. Return type `void` (the
merge's +0x040 CONFLICT: the old `void *` was the ctor's local reading; see
`VariantSprite__VariantSprite`'s report). The local view's `spriteClutX/Y` are
now `sprite.cx` / `sprite.cy` (Sprite.h's SpriteGs, the same s16 at
+0x074 / +0x076). Byte-identical.

## Track 6 (2026-09-26, round 93, bravo)

The class `Class879C4` is now `VariantSprite` (`include/VariantSprite.h`,
`python3 tools/renametype.py Class879C4 VariantSprite`), tier B: the
mechanics are certain and are the whole of what the class adds to Sprite --
`variant` (0 or 1) picks the texture cell the Sprite ctor binds
(`gVariantSpriteCells`) and the CLUT row the reset slot sets
(`gVariantSpriteClutX/Y`). What the sprites are in the game is not
established (their only builder is StyleEffect, kinds 2 and 3, and every
path passes variant 0), which is why it is not tier A. The table, getter,
allocator, methods and the three data tables followed the class name.
The same tool run rewrote `Class879C4` tokens inside this report's older
history prose (the known renametype behaviour pending an operator
decision); those lines were left as the tool wrote them.

History moved here from `src/class_3bb8c_q.c`'s banner: the unit is ROM
0x485BC..0x48738, and its two functions were named in round 79 (tiers
above). `variant` (+0x0A0) keeps its name: it is written here and read by
nothing, which the header now says.

## Proposed (not applied)

- `VariantSprite__UpdateScale`'s `s16 *ratios` -> `Ratio16 *ratios`
  (SceneNode.h's num/den pair, the type `Sprite__UpdateRotation` already
  takes): the body reads `ratios[0]/[1]` and `[2]/[3]` as two such pairs.
  A body edit in `src/class_3bb8c_q.c`, so it is track 7's, not this
  pass's.
