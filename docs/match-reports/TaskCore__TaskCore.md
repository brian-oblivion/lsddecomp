# TaskCore__TaskCore

> Renamed from `TaskCoreObj__TaskCoreObj` on 2026-09-25 (tools/rename.py). Address 0x8003bf10.

> Renamed from `func_8003BF10` on 2026-09-23 (tools/rename.py). Address 0x8003bf10.

**Unit:** code_2c054 · **Size:** 62 words · **Status:** MATCHED (62/62)

## Summary

This occupies `TaskCoreMethods`'s (`gTaskCoreMethods`) own slot `+0x008` (per
`classtable.py gTaskCoreMethods`), and is called from this unit's `StreamTask__StreamTask`
(`StreamTaskObj`'s own ctor) as `Get_vtable_TaskCore()->slot08(self, a1, a2, a3)`.
It is a **base-class constructor**: it briefly points `self->methods` at its
own table (`gTaskCoreMethods`) before the derived ctor (`StreamTask__StreamTask`)
overwrites it with the real `gStreamTaskMethods` table right after this call
returns — the classic constructor-chaining shape, now confirmed directly
rather than inferred.

```c
void TaskCore__TaskCore(StreamTaskObj *self, s32 a1, s32 a2, StreamTaskUnkB4Obj *a3) {
    StreamTaskUnkB4Obj *tmp;
    TaskCoreMethods *core;

    Get_vtable_IntermediateBase()->slot08(self);
    core = Get_vtable_TaskCore();
    self->methods = (StreamTaskObjMethods *)core;
    core->slotD8(self, a1);
    if (a2 != 0) {
        self->unk48 = New_VabStreamObj(a2);
    } else {
        self->unk48 = a3;
    }
    self->unk44 = a2;
    self->methods->slotD4(self, 0, 0);
    tmp = New_TileAtlas(0);
    self->unk80 = tmp;
    tmp = New_TileMap(0, tmp);
    self->unk7C = tmp;
    self->unk78 = New_BgLayer(tmp, 1);
    self->methods->slot40(self);
}
```

**UPDATE (this same round, after `TaskCore__Finalize`):** `a3`, `tmp`,
`unk48`, `unk7C`, and `unk80` are shown here already retyped from the
original `s32` guess to `StreamTaskUnkB4Obj *`/`StreamTaskUnkB4Obj *`. See
the "Field-type correction" section below — this changed no compiled bytes,
only the header's declared types.

## New structure discovered

- `Get_vtable_IntermediateBase()->slot08(self)`: new `TaskUtilMethods` slot `+0x008`
  (`gIntermediateBaseMethods+0x008 = IntermediateBase__IntermediateBase`, extern, void, discarded return).
- `Get_vtable_TaskCore()->slotD8(self, a1)`: new `TaskCoreMethods` slot `+0x0D8`
  (`gTaskCoreMethods+0x0D8 = TaskCore__SetTarget`, extern, void).
- `self->methods->slotD4(self, 0, 0)`: new `StreamTaskObjMethods` slot
  `+0x0D4` (`gStreamTaskMethods+0x0D4 = TaskCore__SetSubHandle`, extern, void).
- New fields: `unk44` (`+0x044`, `s32`, set from `a2`), `unk48` (`+0x048`,
  originally guessed `s32`, either a call result or `a3` verbatim), `unk7C`
  (`+0x07C`, originally guessed `s32`, call result), `unk80` (`+0x080`,
  originally guessed `s32`, call result). **See "Field-type correction"
  below — all three are actually `StreamTaskUnkB4Obj *`.**
- `self->unk78` (already `StreamTaskUnk78Obj *` from `TaskCore__OnDeinit`) is set
  here from `New_BgLayer`'s return — confirms the pointer type again.
  **`StreamTaskUnk78Obj` itself was later folded into `StreamTaskUnkB4Obj`,
  see below.**
- Four new plain externs (`New_VabStreamObj`, `New_TileAtlas`, `New_TileMap`,
  `New_BgLayer`), none in this unit; typed purely from this call site's own
  register usage. **Return types corrected, see below.**

## Field-type correction (added after `TaskCore__Finalize`, same round)

`unk48`, `unk7C`, and `unk80` were typed `s32` here purely because nothing
available at the time contradicted it — they're only ever *written* in this
function, never dereferenced. `TaskCore__Finalize` (this unit's next queued
function, address order 0x8003C008, right after this one) reads all three
back and dereferences each as `field->methods->slot04(field)`, which is
impossible for a plain integer. Retyped all three (and `TaskCore__TaskCore`'s own
`a3` parameter and `tmp` local, and `New_VabStreamObj`/`New_TileAtlas`/
`New_TileMap`/`New_BgLayer`'s signatures) to `StreamTaskUnkB4Obj *` in
`include/code_2c054.h`. Also, `StreamTaskUnk78Obj`/`StreamTaskUnk78Methods`
(the type `unk78` used up to this point) is retired and folded into
`StreamTaskUnkB4Obj` — see `TaskCore__Finalize.md` for the five-way evidence.

**None of this changed a single compiled byte.** A pointer and an `s32` are
the same register width; the retype is pure relabeling. A full rebuild after
the change reconfirmed all nine of this round's prior matches
(`StreamTask__OnPadConfirm` through this function) byte-exact before writing
`TaskCore__Finalize`'s own body. This is the concrete instance of this round's
"field whose only known use is a call result carries no type evidence"
lesson — see `TaskCore__Finalize.md`'s own proposed learning.

## Two residues, same root cause, both from this round's "value reused after
a `jalr`" lever

**Residue 1 (caught before it cost a build cycle):** the natural first draft
called `Get_vtable_TaskCore()` TWICE — once for `self->methods = Get_vtable_TaskCore();`
and again for `Get_vtable_TaskCore()->slotD8(...)`. Retail calls it **once**: the
delay slot after the call is a plain `move a0, s1` (not a repeat call), and
`self->methods = v0; v0->slotD8(self, a1);` reuses the SAME register (the
intervening `sw` doesn't clobber it). Writing `TaskCoreMethods *core =
Get_vtable_TaskCore();` and using `core` for both the assignment and the call
fixed a 45-word-shifted build in one step — visible immediately in
`asm-differ` as a genuinely duplicated `jal 3dfc0` in the built column that
retail does not have.

**Residue class not hit a second time:** the tail chain
(`New_TileAtlas`→`New_TileMap`→`New_BgLayer`, each result both stored to
a field and fed to the next call) was written with a `tmp` local from the
start, informed directly by residue 1 — no second build cycle needed for it.

## Third-learning check (per head's request)

**Needed, and this is the cleanest case yet: not "self->field re-read after
a jalr" but "a called singleton accessor's OWN return value re-read after a
jalr."** The head's original framing (`self->field`) undersells the general
form slightly — the actual rule is *any* live value used again after an
intervening `jalr` needs a local, whether that value came from a struct field
or, as here, directly from a call's return register. `self->methods` itself
was never re-read across a call in this function (it's read fresh once,
right before the final `slot40` call, same as `StreamTask__StreamTask`); the residue
was entirely about the `Get_vtable_TaskCore()` return value's reuse.

## Proposed learning

**Widen the "value reused after an intervening `jalr` needs a local"
lever explicitly to call-return values, not just `self->field` reads.**
`Get_vtable_TaskCore()`/`Get_vtable_IntermediateBase()`-style singleton accessors are called
routinely in this unit, and a call site that both stores the returned
pointer AND immediately dereferences it for a vtable lookup is exactly the
shape that needs a local — the disassembly tell is a literal duplicated
`jal` to the same target with no argument-setup difference between the two,
which reads as obviously wrong once you see it side by side in
`asm-differ`, but is easy to write by reflex when translating two
back-to-back C statements that both mention `Get_vtable_TaskCore()`.

## Naming

**TaskCoreObj__TaskCoreObj** -- tier A. Occupies `gTaskCoreMethods`'s own
ctor slot `+0x008`; confirmed a base-class constructor by its own body
(`self->methods = (StreamTaskObjMethods *)core;`, temporarily pointing the
object at its own table before the derived `StreamTask__StreamTask`
overwrites it). `Class__Class` convention, same precedent as
`StreamTask__StreamTask`.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from TaskCoreObj__TaskCoreObj (tools/rename.py). Occupant of +0x008 (`ctor`). Calls IntermediateBase's ctor first, and each subclass ctor (StreamTaskObj, TitleMenu, GraphRoomObj) calls this one first. Parameters named from the body: `target` goes to setTarget (+0x0D8), `soundBankPath` (both subclass ctors pass "ETC\ETCSE") to New_VabStreamObj, whose result, or the caller's `sound`, is +0x048. The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 87, VabStreamObj)

`include/code_2c054.h`'s local `extern StreamTaskUnkB4Obj
*New_VabStreamObj(char *)` is deleted. `src/code_2c054.c` includes
`include/VabStreamObj.h` instead. The existing `(BasicClass *)` cast into
`TaskCore::sound` is unchanged, and the whole image stays byte-identical.

## Track 4 (2026-09-26, round 88, alpha)

TaskCore::bgLayer (+0x078) is `struct BgLayer *` (include/BgLayer.h, was `BasicClass *`), so the New_BgLayer result is stored uncast; the TileMap argument is cast to BgLayer.h's `struct Map44294 *` (no code). include/code_2c054.h's local extern of New_BgLayer is gone. Byte-identical.

Later the same round (alpha, second class): TileMap unified (`include/TileMap.h`): TaskCore::tileMap is `struct TileMap *` (was `BasicClass *`), and the local `tmp` that holds the TileAtlas and then the TileMap is `void *` (was `StreamTaskUnkB4Obj *`), so the three casts on the tileAtlas/tileMap/New_BgLayer lines are gone. Byte-identical.

Later the same round (alpha, third class): TileAtlas unified (`include/TileAtlas.h`): TaskCore::tileAtlas (+0x080) is `struct TileAtlas *` (was `BasicClass *`), and New_TileAtlas is declared by include/TileAtlas.h (include/code_2c054.h's `StreamTaskUnkB4Obj *` view is deleted). `tmp` stays `void *` (it is reused for the TileMap). Byte-identical.
