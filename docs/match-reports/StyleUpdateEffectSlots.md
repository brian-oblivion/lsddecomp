# StyleUpdateEffectSlots -- MATCHED (34/34 words), dream_scene

> Renamed from `func_80054C74` on 2026-09-23 (tools/rename.py). Address 0x80054c74.

Round 46 (second sitting, alpha). Byte-exact, whole-image SHA1 verified.
Matched on the first build.

## Signature

```c
void StyleUpdateEffectSlots(void *arg0);
```

## Struct: new local view `ObjE0C8`

`sStyleEffectSlots` (already declared `extern void *sStyleEffectSlots[];` for the
already-matched `StyleReleaseEffectSlots`, which just forwards it to
`ReleaseBasicClassArray(sStyleEffectSlots, sStyleEffectSlotCount)`) holds pointers to objects that this
function dispatches *directly*, one at a time, through a method table at
object offset 0 -- the same `ObjAB54`-style pattern already established
earlier in this unit, just at a different slot offset:

```c
typedef struct ObjE0C8 ObjE0C8;
typedef struct ObjE0C8Methods ObjE0C8Methods;
struct ObjE0C8Methods {
    u8 padEC[0xEC];
    void (*slotEC)(ObjE0C8 *self, void *arg1); /* +0x0EC */
};
struct ObjE0C8 {
    ObjE0C8Methods *methods; /* +0x000 */
};
```

Local view only (not `include/class_3bb8c.h`): the multiple-independent-
local-views convention applies, and this is the only place in the executable
that dispatches slot `+0xEC` on this array's elements (no other caller found
via `grep -rn 80054C74`).

`sStyleVariant`/`sStyleEffectSlotCount`/`sStyleEffectSlots` are declared `extern` a second time,
verbatim, ahead of this function -- ROM order puts `StyleUpdateEffectSlots` textually
*before* `StyleReleaseEffectSlots`'s own copy of the same three externs, so a fresh set
was added here rather than hoisting the existing ones (repeated identical
`extern` declarations are legal C89 and this keeps each function's own
declarations next to it, matching the file's existing style).

## Body

```c
void StyleUpdateEffectSlots(void *arg0) {
    s32 i;
    ObjE0C8 *obj;

    if (sStyleVariant < 0) {
        return;
    }
    for (i = 0; i < sStyleEffectSlotCount; i++) {
        obj = (ObjE0C8 *) sStyleEffectSlots[i];
        obj->methods->slotEC(obj, arg0);
    }
}
```

Straightforward guard + `for` loop; GCC 2.6.3 rotates it into the classic
pretest-then-post-condition shape retail shows (`bltz`/`blez` guard before
the loop body, `bnez` back-edge test at the bottom) -- no manual restructuring
needed, matched directly from the natural C shape.

## Attempts

1 (matched on the first build).

## Naming

**`StyleUpdateEffectSlots`, tier B.**

Iterates `sStyleEffectSlots[0 .. sStyleEffectSlotCount)` dispatching
`slotEC(obj, arg0)` on each -- the per-frame update half of the
`StyleBuildEffectSlots`/`StyleUpdateEffectSlots`/`StyleReleaseEffectSlots`
triad. MATCHED, 34/34, first build.

## Track 4 (2026-09-26, round 88, charlie)

The `ObjE0C8` view is gone: each slot is a `StyleEffect` (include/StyleEffect.h) and the +0x0EC call is StyleEffect__Update through `StyleEffectUpdateFn` (the slot keeps Actor's `setPendingExtra` type; a cast, no code). `arg0` is the position (`LongVec3 *`), which Update forwards to UpdateByKind. Image byte-identical.

## Round 93 polish (delta, track 7)

### Naming

Round 93: locals `pos`, `slot`.
