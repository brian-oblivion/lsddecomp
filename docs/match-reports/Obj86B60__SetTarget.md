# Obj86B60__SetTarget — MATCH (110/110 words)

> Renamed from `func_8003CE98` on 2026-09-24 (tools/rename.py). Address 0x8003ce98.

**Unit:** code_2cc8c_b · round 12, one of the unit's 5 round-11-straggler
functions (fresh ground, no prior report existed for any of the five).

## What it does

The constructor/setter for `self->unk4C` (the `Unk4CObj *` target
descriptor): stores the argument, then builds four parallel per-slot
arrays sized off a null-terminated name list, then constructs or reuses
a handle object and fills each array slot from it.

```c
void Obj86B60__SetTarget(Obj86B60 *self, Unk4CObj *a1)
{
    char **list;
    s32 count;
    s32 size;
    Unk64Elem **arr;
    Unk74Obj *handle;
    s32 i;

    self->unk4C = a1;
    if (a1 == NULL) {
        return;
    }

    list = a1->unk1C;
    count = 0;
    while (*list++ != NULL) {
        count++;
    }
    size = count * 4;
    arr = BMemPMgrAlloc(size);
    self->unk54 = arr;
    self->unk5C = BMemPMgrAlloc(size);
    self->unk60 = BMemPMgrAlloc(size);
    self->unk64 = BMemPMgrAlloc(size);
    self->unk50 = count;

    if (a1->unk0 != NULL) {
        handle = func_8003B39C(a1->unk0);
        handle->methods->slot78(handle);
        handle->methods->slot5C(handle);
    } else {
        handle = a1->unk4;
    }

    list = a1->unk1C;
    if (*list != NULL) {
        i = 0;
        do {
            void *extra = a1->unk24[i];
            s32 len = func_80013348(*list);

            *arr = New_Obj6EAC0(handle, len, *list);
            arr++;
            if (extra != NULL) {
                self->unk58 = i;
                self->methods->slotF8(self, extra, handle);
            }
            list++;
            i++;
        } while (*list != NULL);
    }

    self->unk68 = New_ClassEAC0(D_8008A8E8, D_8008A8F0, 0);
    a1->unk4 = handle;
}
```

## New struct knowledge (Unk4CObj)

This function is the FIRST one in the unit to load `*(unk4C+0)` and
`*(unk4C+4)` — the unit's own header comment previously stated (correctly,
for the 16 functions attempted through round 11) that nothing ever loads
offset 0. That statement is now WRONG in general and has been corrected in
`include/code_2cc8c.h`'s `Unk4CObj` comment; it was true only of the
evidence available at the time.

- `+0x000 const char *unk0` — a path, passed to `func_8003B39C(unk0)` when
  non-NULL to build `unk4`. Truthy-only gate also independently confirmed
  by `Obj86B60__ReleaseTarget` (round 12, same unit).
- `+0x004 Unk74Obj *unk4` — either the freshly-constructed handle (when
  `unk0` is set) or a pre-existing handle passed straight through
  (`a1->unk4`), written back at the end either way. Matches the EXACT
  `func_8003B39C(path)` + `slot78` + `slot5C` idiom `Obj86B60__SetSubHandle`
  already uses for `self->unk74` — same `Unk74Obj` type, different field.
- `+0x01C char **unk1C` — a null-terminated string array, DISTINCT from
  the already-known `+0x018 void **unk18` (adjacent field, same shape).
  Walked with `func_80013348` (strlen) and `New_Obj6EAC0` to build each
  `self->unk54[i]`/`self->unk64[i]` entry.

New `Obj86B60Methods` slot: `+0x0F8 slotF8(self, void *a1, Unk74Obj *a2)`.

New external: `New_ClassEAC0(void*, void*, s32) -> Unk68Obj*` (lives in the
still-uncarved `code_2cc8c_d` segment, not this unit's function — declared
`extern` for this unit's own local view of what it returns). Its two
pointer args, `D_8008A8E8`/`D_8008A8F0`, are only ever address-taken here,
so they're typed minimally (`s32[2]`/`char[4]`) matching their observed
byte layout in `asm/data/7B008.sdata.s`.

New `self->unk68` field (`Unk68Obj *`, dispatched via `->methods->slot4`
etc. by `Obj86B60__ReleaseTarget`/`Obj86B60__CommitElementScroll`/`Obj86B60__RefreshSlotView` — see those
reports/stalls for the rest of `Unk68ObjMethods`).

## Getting to the match

Fully hand-decoded from the disassembly (no m2c seed used — this
constructor-shaped function has enough distinct field accesses that manual
tracing was faster and more reliable than typing a guess).

First C attempt scored close (matching register colors on ~55% of words)
but had one extra live variable (`outPtr`, a second walking pointer
alongside `arr`) forcing an 8th callee-saved register the retail build
doesn't use — removing it (reusing `arr` itself as the walking pointer
after the initial `self->unk54 = arr` store) fixed the register count.

Second residue: the fill loop originally cached `*list` into a named
`char *name` local and reused it for both the `strlen` call and the
`New_Obj6EAC0` call. Retail RELOADS `*list` from memory for the second
use instead of keeping it in a register across the `strlen` call —
matched by simply not naming it (`func_80013348(*list)` /
`New_Obj6EAC0(handle, len, *list)`, two separate dereferences).

After those two fixes: 106/110, remaining 4 words were pure address-drift
from the OTHER 4 still-`INCLUDE_ASM` functions in this unit at the time —
resolved automatically once `Obj86B60__ReleaseTarget` and `Obj86B60__UpdateSlotElements` were also
matched (they precede this address range... actually follow it; the drift
was in the trailing `D_8008A8E8`/`New_ClassEAC0` references, downstream
data/code whose absolute addresses depend on total image size).

### Proposed learning

When a value is read from memory, used once, then read from the SAME
memory location again for a second use a few instructions later (rather
than kept in a register across the gap), the C very likely does NOT name
that value in a local variable — write the two dereferences at their two
call sites instead of caching. Caching it forces the compiler to keep it
live in a register across whatever's in between (here, another call),
which if retail's own register count doesn't support, shows up as a
whole-function register-numbering shift (every s-register off by one)
rather than a localized diff.

## Naming (round 78, naming runner echo)

Renamed `func_` -> `Obj86B60__SetTarget`. **Tier B**: Stores `a1` into `self->unk4C` (the cross-unit 'target' descriptor, named from six independent functions' evidence, see Unk4CObj's own header comment) and builds four parallel per-slot arrays from it. Mechanics (constructor for the target association) are clear; what the target itself represents in the game is not.

## Proposed field names

Not renamed here -- both fields are CROSS-UNIT (read by `func_8003C63C`/
`func_8003CA1C` in `src/code_2cc8c.c`, verified by attempting the rename and
reading the compiler's own error list: both moved from "0 errors" to errors
in `code_2cc8c.c` specifically, none elsewhere). Proposing for the head to
apply at merge (type scope: rename the definition, rebuild, fix exactly the
accessors the compiler lists, in both units):

- `Obj86B60::unk4C` -> `target` (tier B). Established across six functions
  (see `Unk4CObj`'s own header comment); this report's own function is its
  constructor/setter.
- `Unk4CObj::unk24` -> `slotEntries`, type `SlotEntry **` (tier B, retype +
  rename). Every dereference in `code_2cc8c_b.c` (`Obj86B60__CommitElementScroll`,
  `Obj86B60__RefreshSlotView`, `Obj86B60__CancelElementScroll`, `Obj86B60__SetSlotCursor`)
  already casts it to `SlotEntry *`/`(SlotEntry *)...` locally; `func_8003CA1C`
  (code_2cc8c.c, not attempted) reads it as a generic word-pointer array and
  would need `(void **)self->unk4C->slotEntries` or an equivalent cast, a
  one-line fix at that one call site.
- `Unk4CObj::unk10[3]` -> `unselectedColor` (tier B). `Obj86B60__CancelElementScroll`/
  `Obj86B60__SetSlotCursor` both feed this buffer to the OLD/outgoing
  element's `slotB8` right before (or without) a `slot60(elem,1)`
  highlight-on call on the NEW one -- the colour an item reverts to when it
  stops being the current selection, not the selection's own colour.
  `func_8003C63C` (code_2cc8c.c, not attempted) only takes its address, so a
  rename there is a pure rename, no cast needed.
