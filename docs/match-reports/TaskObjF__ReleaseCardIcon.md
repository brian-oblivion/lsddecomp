# TaskObjF__ReleaseCardIcon -- MATCH

> Renamed from `TaskObjF__TickCardIcon` on 2026-09-26 (tools/rename.py). Address 0x8004ff40.

> Renamed from `Class86E00_3bb8c_g__TickCardIcon` on 2026-09-23 (tools/rename.py). Address 0x8004ff40.

> Renamed from `func_8004FF40` on 2026-09-23 (tools/rename.py). Address 0x8004ff40.

Unit `class_3bb8c_g`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TaskObjF__ReleaseCardIcon`: 20/20 words match.

## Source

```c
void TaskObjF__ReleaseCardIcon(Class86E00_3bb8c_g *self)
{
    if (self->unk70 != NULL) {
        self->unk70 = self->unk70->methods->slot4(self->unk70);
    }
}
```

First attempt, byte-exact. `Class86E00_3bb8c_g::unk70`'s slot4 is an
"advance" call: its return value is stored back into the same field,
unlike the ordinary release pattern seen elsewhere in this project
(discard the return, unconditionally null the field) -- see the struct
comment for why this earned its own dedicated pointee type rather than
reusing the shared release-only view.

## New class discovered: `Class86E00_3bb8c_g` / `D_80086E00`

This is the first function in a brand new 29-slot class table
(`D_80086E00`, resolved with `tools/classtable.py 0x80086E00`), unrelated
by inheritance to any table already known in this header (`--vs` against
`gBasicClassMethods`/`gClass866E8Methods`/`gTitleMenuMethods`/`gTaskObjFMethods` showed no shared run of
slots). Found by searching the retail binary for raw pointer values
matching this unit's own function addresses (each of the 12 fresh
functions plus the 3 non-`gp_rel` blocked ones appears EXACTLY ONCE, in one
contiguous table starting at file offset `0x77648` / vram `0x80086E48`,
slots `+0x048`..`+0x074`). Slots `+0x004`..`+0x03C` belong to
`class_3bb8c_e`/`class_3bb8c_f` (confirmed by `grep`ping their `.c` files
for those function names) -- this round's other two runners sharing
`include/class_3bb8c.h`.

**Every new type is suffixed `_3bb8c_g`, including the class name itself**
-- unlike the single-owner `TitleMenu`/`NodeGuardedViewport`/`GridCell` already
in this header. Since three units reach into this ONE real table this
round, an unsuffixed `Class86E00` risks a same-named, differently-shaped
definition arriving from `class_3bb8c_e` or `class_3bb8c_f` at merge time
with no conflict marker -- the exact round-13 silent-collision hazard the
shared-header rules exist to prevent. This is a purely additive,
brand-new struct (nothing existed at these offsets before), so there is
no existing-declaration retype to flag for this function.

## Struct additions (additive, `include/class_3bb8c.h`)

- `Class86E00_3bb8c_g` / `Class86E00Methods_3bb8c_g` -- new class,
  opaque below `+0x028` (owned by the other two units).
- `Class86E00_3bb8c_g::unk70` and its dedicated pointee
  `Class86E00Unk70Obj_3bb8c_g` (`slot4` returns `Class86E00Unk70Obj_3bb8c_g
  *`, not `void`).
- Also pre-declared, while surveying the whole unit before writing any
  code, the remaining struct surface this unit's other 11 fresh functions
  need: `Class86E00SubObj_3bb8c_g` (self->unk78/unk7C's shared pointee),
  `GenericSlot9CObj_3bb8c_g` (TaskObjF__OnItemListResult's arg1), and
  `Class86E00Methods_3bb8c_g`'s `slot10`/`slot78`/`slot7C`/`slot8C`/
  `slot90`/`slot94`/`slotA0`/`slotAC`, plus `extern` declarations for two
  external helpers this unit calls but does not own
  (`New_ItemList`, `New_TextEntry`). These are exercised by later
  functions in this unit; see their own reports for confirmation.

### Proposed learning

None new -- the "release slot's return value can be used or discarded
per-caller" pattern is already implicit in the existing "empty-bodied
vtable occupant is not evidence the slot takes no arguments" family, just
applied to a return value instead of a parameter.

## Naming

`TaskObjF__ReleaseCardIcon` (was `func_8004FF40`), tier B: calls
`self->unk70`'s own generic per-step "advance" slot and stores the
result back (the same `self->field = self->field->methods->slot4(...)`
shape recurring across many unrelated classes in this project, e.g.
`class_39e08.c`, `code_2cc8c_b.c`, `code_55dd4.c` -- read here as an
ordinary per-frame/per-step tick of the loaded card icon object). What
"advancing" the icon actually changes on screen is not established.

## Track 4 (2026-09-26, round 89)

Renamed from `TaskObjF__TickCardIcon`. `cardIcon` (+0x070) is only ever assigned from `New_ScreenSprite` (TaskObjF__LoadCardIcon), and +0x004 of gScreenSpriteMethods is `BasicClass__Release` (`tools/classtable.py gScreenSpriteMethods`), which finalizes, frees and returns NULL. So the call stores NULL back: the function releases the icon, it does not tick it. Its one caller is TaskObjF__SetState's second call (slot +0x084, every path), right before loadCardIcon (+0x080) makes the next one.
