# Graphics core headers: process text moved out by the API pass

Track 12 (item `area-graphics-core`, round 106, runner charlie) rewrote
these headers as Doxygen API documentation:

    include/char_sprite.h include/draw_system.h include/frame_clock.h
    include/light_rig.h include/requested_file.h include/scene_node.h
    include/screen_sprite.h include/sprite.h

Text that justified a C spelling or recorded a derivation for one function
went to that function's match report, under "History: track 12". This note
holds what belongs to no single function: the pointers at the project's
tools that the headers carried, and the class-model derivations stated in
the class banners. `python3 tools/apidoc.py --item area-graphics-core`
measures what is left in the headers.

## History: the passages as they stood before the pass, verbatim

The source of every excerpt is commit `8f08d326f`.

### Tool pointers

`include/scene_node.h`, the class banner and the slot macro's comment:

```c
 * Methods: src/graphics/scene_node.c. Subclasses:
 * `python3 tools/plan.py classes` (Actor, Sprite, LightRig, BoxFill and
 * more); they expand SCENENODE_FIELDS and SCENENODE_SLOTS first.
```

```c
/* The slots, each named for the occupant in gSceneNodeMethods; `python3
 * tools/classtable.py <subclass table> --vs gSceneNodeMethods` lists a
 * subclass's overrides. */
```

`include/draw_system.h`, the class banner:

```c
 * DrawSystem -- the game's screen/graphics singleton, class id 0x1, method
 * table gDrawSystemMethods, a direct BasicClass subclass (`tools/classtable.py
 * gDrawSystemMethods --vs gBasicClassMethods`: overrides only the ctor, adds seventeen
 * slots). Methods in src/graphics/draw_system.c; no class derives from it.
```

`include/frame_clock.h`, the class banner:

```c
 * FrameClock -- class id 0x5, method table gFrameClockMethods (22 slots), a
 * direct BasicClass subclass (its ctor calls GetBasicClassMethods()->ctor
 * first; `classtable.py gFrameClockMethods --vs gBasicClassMethods` overrides the
 * ctor, finalize, removeParentRef and notifyParents and adds seven slots).
```

`include/sprite.h`, the class banner and the slot macro's comment:

```c
 * projected from the inherited coordinate. Methods in src/graphics/sprite.c; four
 * classes derive from it (`typeviews.py --tree`): ScreenSprite (0x144, the
 * screen-space sprite, include/screen_sprite.h), CharSprite (0x1144, one 8x8
 * font character, include/char_sprite.h), TextRow (0x11144) and gVariantSpriteMethods (0x1F44,
 * src/world/dream_scene.c).
```

```c
/* SceneNode's slots, then this class's own. Occupants in gSpriteMethods
 * named at each own slot; `tools/classtable.py gSpriteMethods --vs
 * gSceneNodeMethods` lists the overrides of the inherited ones (Sprite__Sprite,
 * Sprite__Reset, Sprite__UpdateRotation, Sprite__SetDisplay,
 * Sprite__SetSemiTrans, Sprite__SetSemiTransRate, Sprite__Update). */
```

`include/screen_sprite.h`, the class banner and the slot macro's comment:

```c
 * The ctor chains to Sprite's first (GetSpriteMethods()->ctor with abr 0 and
 * a NULL fourth argument), and CharSprite's ctor chains to this one, so the
 * id tree (0x44 -> 0x144 -> 0x1144) is the ctor chain. Two classes derive
 * from it (`typeviews.py --tree`): CharSprite (0x1144, one 8x8 font
 * character, include/char_sprite.h, own fields from +0x0A8), which expands
 * these macros, and TextRow (0x11144, below CharSprite, include/text_row.h),
 * which expands CharSprite's.
```

```c
/* Sprite's slots, then this class's own. `tools/classtable.py
 * gScreenSpriteMethods --vs gSpriteMethods` lists the overrides of the
 * inherited ones (ScreenSprite__ScreenSprite, ScreenSprite__Reset,
 * ScreenSprite__AttachToParent). */
```

`include/char_sprite.h`, the slot macro's comment:

```c
/* ScreenSprite's slots, then this class's own. `tools/classtable.py
 * gCharSpriteMethods --vs gScreenSpriteMethods` lists the overrides of the
 * inherited ones (CharSprite__CharSprite, CharSprite__Reset). */
```

`include/light_rig.h`, the slot macro's comment:

```c
/* SceneNode's slots, then this class's own. `tools/classtable.py
 * gLightRigMethods --vs gSceneNodeMethods` lists the overrides of the
 * inherited ones (LightRig__LightRig, __Finalize, __Reset,
 * __DispatchLinkCommand). */
```

### Class-model derivations

`include/char_sprite.h`, the class banner (the id-tree reading and the
object-size derivation):

```c
 * The ctor chains to ScreenSprite's first (GetScreenSpriteMethods()->ctor
 * with cell 0x20's rect and 0), so the id tree (0x144 -> 0x1144) is the ctor
 * chain. One class derives from it: TextRow (0x11144, include/text_row.h),
 * which expands these macros.
```

```c
 * The object is 0xAC bytes (New_CharSprite): `cellIndex` at +0x0A8 is the
 * one field this class's methods touch, and 0xAC is sizeof rounded to the
 * word alignment the method pointer gives the struct. TextRow's own u8
 * fields start at +0x0A9, inside that padding, which a flat expansion
 * of CHARSPRITE_FIELDS puts exactly there.
```

The same banners' ctor-chain readings for LightRig and the register notes
for SceneNode, Sprite and LightRig went to the reports of
`LightRig__LightRig`, `SceneNode__TryAttachNearby` and `Sprite__Sprite`;
ColorRgb's layout note (draw_system.h) and SpriteGs's (sprite.h) went to
`Sprite__SetColor`'s, and the moveImage slot's to `DrawSystem__MoveImage`'s.
