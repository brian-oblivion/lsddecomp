# Class86B60__Dtor -- MATCH

> Renamed from `func_8004D704` on 2026-09-24 (tools/rename.py). Address 0x8004d704.

Unit `class_3bb8c_d`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py Class86B60__Dtor`: 33/33 words match.

## Source

```c
void Class86B60__Dtor(Class86B60 *self)
{
    if (self->unkAC != NULL) {
        self->unkAC->methods->release(self->unkAC);
        self->iconHandle->methods->release(self->iconHandle);
    }
    Get_vtable_TaskCore()->slot0C(self);
}
```

## Derivation

This is `Class86B60`'s own destructor (the first function of `class_3bb8c_d`,
immediately continuing `class_3bb8c_c`'s work on the same class). Structure:

- `self->unkAC` and `self->iconHandle` are two owned sub-objects, each released
  through the shared BasicClass-family `release` slot at `+0x004`
  (`GenericReleaseObj_3bb8c_d`, a local independent view of the same shared
  slot `include/code_8220.h`'s `BasicClassMethods::release` occupies).
- `Get_vtable_TaskCore()` is the class hierarchy's shared base-class method table
  getter (same real global, `gTaskCoreMethods`, as `include/code_2c054.h`'s
  `TaskCoreMethods`). Its `+0x00C` slot is called unconditionally last —
  the base class's own destructor forward.

**First-attempt near-miss, corrected on re-reading the branch target.** My
first attempt read `unkAC`'s null check as guarding only the `unkAC` release,
with `iconHandle`'s release unconditional right after. That produced a `beqz`
whose immediate was too *short* (offset 7 words vs retail's 15) — retail's
single label `.L8004D75C` sits **after both calls**, so the guard covers
both releases together, not just the first. There is no separate null check
on `iconHandle` at all; retail relies on invariant "if `unkAC` is set, `iconHandle` is
too." Moving `iconHandle`'s release inside the `if` closed the gap to
byte-exact on the next build.

## Struct changes (additive, `include/class_3bb8c.h`)

- New type `GenericReleaseObj_3bb8c_d` / `GenericReleaseMethods_3bb8c_d`
  (release-only local view, same convention as `GenericTagInst_3bb8c_c`).
- `Class86B60::unkAC` **retyped** from `s32` to
  `GenericReleaseObj_3bb8c_d *` — same size (4 bytes), no layout change.
  Previously inferred only from a `zeroed` write in `Class86B60__Class86B60`, which
  is consistent with either reading; this function is what proves it is
  dereferenced through a vtable.
- `Class86B60::iconHandle` — new field, was anonymous padding
  (`pad0A8[0x0AC-0x0A8]`), now named and typed the same as `unkAC`.
- `BaseTaskCtorTable_3bb8c_c::slot0C` — new slot, `void (*)(void *self)`.
  Same offset AND arity as `code_2c054.h`'s independently-derived
  `TaskCoreMethods::slot0C` on the same real global (`gTaskCoreMethods`) —
  cross-unit confirmation, not a coincidence.
- Also added (needed by later functions in this same round, grouped into
  one edit to avoid re-touching this struct repeatedly): `slot60`,
  `slot90` on `BaseTaskCtorTable_3bb8c_c`. See their own functions'
  reports for the evidence.

### Proposed learning

None new beyond what's already written down — this is a second confirmed
instance of "a `beqz`/`bnez` guard's true extent is measured from where its
LABEL actually sits, not from where the guarded-looking code appears to
end," which is really the existing "branch targets disagree" family. Filing
here rather than promoting a new bullet since it's the same lesson applied
to a two-statement block instead of one.

## Naming (round 77, naming runner delta)

Renamed `func_8004D704` -> `Class86B60__Dtor`. **Tier A**: matches the
BasicClass-family destructor shape (release owned sub-objects, then
forward to the base class's own dtor slot). Field `unkA8` renamed to
`iconHandle` in the same round (compiler-ownership check: accessor set
entirely inside `src/class_3bb8c_d.c`).

## Proposed field names

**Head, round 77:** `unkAC -> saveCtrl` APPLIED by type scope (11 accessors, class_3bb8c_c/_d).

`Class86B60::unkAC` has a real accessor outside this unit
(`src/class_3bb8c_c.c`'s `Class86B60__Class86B60` zeroes it), so per the
compiler-ownership rule this is a PROPOSAL, not a rename. Also posted to
the round-77 broadcast.

- **`unkAC` -> `saveCtrl`, tier B.** The `New_TaskObjF`-constructed
  sub-object (see `Class86B60__BeginMemcardSave`) that this function
  guards both releases on, and whose `slot6C` dispatch carries
  `D_8008A9D0` ("BISLPS-01556", Sony's memcard save-header game-ID
  string). Named for its role driving the memcard-save sequence this
  unit's own functions establish
  (`Class86B60__BeginMemcardSave`/`Class86B60__EndMemcardSave`/
  `Class86B60__UpdateMemcardSaveWithIcon`/
  `Class86B60__UpdateMemcardSaveStatus`); the exact protocol is not
  established.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `Get_vtable_TaskCore()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
