# Dream scene headers: process text moved out by the API pass

Track 12 (item `area-dream-scene`, round 106, runner bravo) rewrote these
headers as Doxygen API documentation:

    include/actor.h include/dream_aux.h include/graph_room.h
    include/item_list.h include/objm.h include/style_effect.h
    include/variant_sprite.h

and split the long comment banners of `src/world/dream_scene.c` and
`src/world/dream_aux.c` into the headers' class docs and the functions'
own docs. Text that justified a C spelling or recorded a derivation for one
function went to that function's match report, under "History (source
comments moved in track 12, round 106)". This note holds what belongs to no
single function. `python3 tools/apidoc.py --item area-dream-scene` measures
what is left.

## History: the passages as they stood before the pass, verbatim

The source of every excerpt is commit `7762aff8f`.

### Tool pointers

The headers named the tool that lists a class's overrides. The slots now
carry `@see` the occupant; the command still answers the question:

    python3 tools/classtable.py <subclass table> --vs <base table>

`include/item_list.h`, above ItemListMethods:

```c
/* BasicClass's slots (overrides: +0x008 ItemList__ItemList, +0x00C
 * Finalize, +0x010 AddChild, +0x014 RemoveChild, +0x018 RemoveAllChildren,
 * +0x038 OnNotify; `tools/classtable.py gItemListMethods --vs gBasicClassMethods`),
 * then this class's own. +0x064..+0x078 are NULL in the table. */
```

`include/actor.h`, the class banner and the comment above ACTOR_SLOTS:

```c
 * Three classes derive from it directly (`typeviews.py --tree`): TodActor
```

```c
/* Occupants in gActorMethods named at each slot; `tools/classtable.py
 * <subclass table> --vs gActorMethods` lists a subclass's overrides. The
```

### Addresses

`include/actor.h`'s class banner placed the local move vector by address
(the low halves of the vram addresses of sActorLocalMove[0], [1] and
sActorLocalMoveZ); the header now names the symbol only:

```c
 * s16 vector at sActorLocalMove (x at ABA4, y at ABA6, z at ABA8), apply it through
```
