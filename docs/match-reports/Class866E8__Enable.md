# Class866E8__Enable — MATCHED (3/3 words)

> Renamed from `Class866E8__func_8004B570` on 2026-09-24 (tools/rename.py). Address 0x8004b570.

> Renamed from `func_8004B570` on 2026-09-24 (tools/rename.py). Address 0x8004b570.

`Obj866E8`'s vtable slot +0x0EC (`D_800866E8`, resolved with
`tools/classtable.py 0x800866E8` — a different, independently-typed local
view of the same table already exists in `include/class_3ac78.h` as
`Class866E8Methods`; see `include/class_3bb8c.h`'s header comment for why
this unit keeps its own, per the project's multiple-local-views
convention). This is the first function of `class_3bb8c`'s newly-carved
first slice, and the first match report for it.

## Disassembly

```
ori $v0, $zero, 0x1
jr  $ra
 sw $v0, 0x70($a0)
```

## Final C

```c
void Class866E8__Enable(Obj866E8 *self) {
    self->unk70 = 1;
}
```

**Return type corrected by the head from `s32` to `void`** (byte-identical;
re-verified 3/3 with the whole-image SHA1 green). The body is three words —
`ori $v0, $zero, 1` / `jr $ra` / `sw $v0, 0x70($a0)` in the delay slot — and
the literal has to be materialized in *some* register to be stored, with
`$v0` the natural first choice. So `void` and `s32` are byte-identical here
and the bytes carry NO information about the return type. This is the same
trap as the documented "a `void` wrapper around an `s32` tail call is
byte-identical", in its store-instead-of-tail-call form: the value in `$v0`
is a side effect of needing a register, not a returned result.

What broke the tie is the slot, not this function. `Class866E8__Enable` occupies
`+0x0EC` of `D_800866E8`, and a cross-table survey of that offset
(`tools/classtable.py` over all 60 tables) finds five distinct occupants —
`TaskCore__FindPrevFreeSlot` (the base implementation, shared by four separate class
tables), `Class6E99C__PopPosition`, `Class876FC__Update`, `Actor__SetPendingExtra` and this one.
`TaskCore__FindPrevFreeSlot`'s body ends in a bare `jr $ra` after a `jalr`, materializing
no return value on either of its two paths. A base implementation that
returns nothing is evidence the SLOT is `void`, so an override asserting
`s32` is the less supported reading. None of the other four occupants is
matched yet, so this can be revisited when one is.

The original `return self->unk70 = 1;` form was not wrong about the bytes —
only about what the bytes prove.

## New struct knowledge (`include/class_3bb8c.h`)

- `Obj866E8::unk70` (s32) — a flag, set to 1 here.

## Attempts

1 (matched on first attempt).

### Proposed learning

None new — a direct instance of an already-documented idiom.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B570` | `Class866E8__Enable` | A | Occupant of `D_800866E8` +0x0EC. Body is exactly `self->enabled = 1;`. Paired with `Class866E8__Disable` (+0x0F0, same struct, clears the same field) and cross-confirmed by `class_3ac78`'s own INDEPENDENT local view of the same field, already named `enabled` there (`docs/match-reports/Class866E8__UpdateIfEnabled.md`, round 67) from the identical set/clear evidence. A pure setter of a named boolean field is tier A by the naming rule's own "getter/clamp/list-push" clause. |

| field | name | tier | evidence |
| --- | --- | --- | --- |
| `Obj866E8::unk70` | `enabled` | A | Set 1 here, cleared 0 (after a `slotC0` teardown dispatch) by `Class866E8__Disable`. Same field, same evidence, and the same conclusion `class_3ac78`'s independent view already reached for its own copy of this struct -- see `Class866E8__UpdateIfEnabled.md`. Renamed in `include/class_3bb8c.h`'s own `Obj866E8` definition; rebuild after the rename touched only `src/class_3bb8c.c` (`Class866E8__Enable`/`Class866E8__Disable`), confirming no other unit accesses this struct's `unk70`/`enabled` field. |
