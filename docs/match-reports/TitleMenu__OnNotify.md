# TitleMenu__OnNotify -- MATCH

> Renamed from `Class86B60__OnNotify` on 2026-09-26 (tools/rename.py). Address 0x8004d788.

> Renamed from `TitleMenu__ForwardIfTagB` on 2026-09-26 (tools/rename.py). Address 0x8004d788.

> Renamed from `func_8004D788` on 2026-09-24 (tools/rename.py). Address 0x8004d788.

Unit `title_menu`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TitleMenu__OnNotify`: 35/35 words match.

## Source

```c
void TitleMenu__OnNotify(TitleMenu *self, GenericHeaderObj_3bb8c_d *arg1, s32 arg2)
{
    GetTaskCoreMethods()->slot38(self, arg1, arg2);
    if ((arg1->methods->header & 0xF) == 0xB) {
        self->methods->slot138(self, arg1, arg2);
    }
}
```

## Derivation

Two calls: an unconditional forward of all three parameters to the shared
base class table's `slot38`, then a runtime-type-id gate on `arg1` (its
vtable's header word, low nibble compared against the literal `0xB`) that
conditionally forwards the same three parameters to `self`'s own `slot138`.
First attempt, byte-exact -- the single-`if` shape matched the disassembly's
branch structure directly with no residue.

`self`'s concrete type is inferred as `TitleMenu *` by unit continuity
(every other function in this file is a `TitleMenu` method) rather than
proven by any raw field access in this specific function -- it is reached
purely through vtable dispatch here, so the byte match constrains only the
POINTER's identity/arity, not which named struct it is. Worth flagging for
whoever next resolves a caller of this function.

## Struct changes (additive, `include/class_3bb8c.h`)

- New type `GenericHeaderObj_3bb8c_d` / `GenericHeaderMethods_3bb8c_d` --
  reads a vtable's header word as a full `s32` (`lw` then `andi`), NOT the
  existing `GenericTagInst_3bb8c_c`/`GenericTagMethods_3bb8c_c` (which reads
  only the low BYTE via `lbu`, established by `GridCell__DispatchLinkCommand` in
  `title_menu`). Reusing the byte-typed struct here would have emitted
  the wrong load width.
- `BaseTaskCtorTable_3bb8c_c::slot38` -- new slot, `void (*)(void *self,
  void *arg1, s32 arg2)`.
- `TitleMenuMethods::slot138` -- new slot, same 3-argument signature.

### Proposed learning

None new. This is a clean instance of the already-documented
"`arg->methods->header & 0xF` is a runtime type ID" shape (round 13), just
the first time this unit's own `slot38`/`slot138` pair exercises it.

## Naming (round 77, naming runner delta)

Renamed `func_8004D788` -> `TitleMenu__OnNotify`. **Tier B**: Forwards to the base class's `slot38` unconditionally, then to its own `slot138` only when `arg1`'s vtable header low nibble == 0xB -- the same runtime-type-id-gated forward shape already named `GridCell__DispatchLinkCommand` in `title_menu.c`. Purpose of tag 0xB itself not established.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/task_core.h, track 4 round 84) now go through `GetTaskCoreMethods()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 88, bravo)

Renamed `TitleMenu__ForwardIfTagB` -> `TitleMenu__OnNotify` (tools/rename.py):
it is the occupant of gTitleMenuMethods +0x038, BasicClass's `onNotify` slot,
and its body is an onNotify override -- the base onNotify
(IntermediateBase__OnNotify, through GetTaskCoreMethods()), then this class's
own +0x138 (`onTagBValue`, TitleMenu__OnCardEvent) with the same
(sender, event) when the SENDER's class-id low nibble is 0xB. Its parameters
are now onNotify's: `BasicClass *sender` (the `GenericHeaderObj_3bb8c_d`
view is gone; BasicClassMethods::header is the same full `s32` word, `lw`
then `andi 0xF`) and `s32 event`.

## Track 7 (round 96, echo)

Constant: the `0xB` is `TASKOBJF_CLASS_ID` (include/task_objf.h, added:
gTaskObjFMethods word +0x000 is 0xB, a single-nibble id, so `& 0xF` is its
kind-of test). The 0xF mask stays a literal.
