# Dream system headers: process text moved out by the API pass

Track 12 (item `area-dream-sys`, round 106, runner charlie) rewrote these
headers as Doxygen API documentation:

    include/dream_sys.h include/stage_grid.h include/tod_actor.h

and the comments of their units, src/world/dream_sys.c,
src/world/stage_grid.c and src/world/tod_actor.c. Text that justified a C
spelling or recorded a derivation for one function went to that function's
match report, under "History: track 12". This note holds what belongs to no
single function: pointers at the project's tools, and remarks about the
toolchain or retail that the class banners and type comments carried.
`python3 tools/apidoc.py --item area-dream-sys` measures what is left.

## History: the passages as they stood before the pass, verbatim

The source of every excerpt is commit `2aa001d93`.

### include/tod_actor.h

The slot list's comment, a tool pointer:

```c
/* Occupants in gTodActorMethods named at each slot; `tools/classtable.py
 * gEntityMethods --vs gTodActorMethods` lists Entity's overrides. The
```

The class banner's note on the attachToParent cast:

```c
 * it to TodActorAttachToParentFn (a function-pointer cast emits no code).
```

### src/world/tod_actor.c

```c
/* Header word of gModelDataMethods (tools/classtable.py), the class New_ModelData
 * allocates and TodActor.modelData points at. */
```

### include/stage_grid.h

The header carried an unused tag, removed by the pass (no reference in
src/ or include/; the build stayed byte-exact):

```c
struct simplePair {
    s8 x;
    s8 y;
};
```

Its prototype comments were plain `/* @brief */` lines, which Doxygen
skips; the pass rewrote them as `/** */` blocks, checked against the bodies.

### src/world/stage_grid.c

```c
/* The two per-stage tables (splat data), read only here. */
```
