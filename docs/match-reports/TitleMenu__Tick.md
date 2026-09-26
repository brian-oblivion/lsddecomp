# TitleMenu__Tick -- MATCH

> Renamed from `Class86B60__Tick` on 2026-09-26 (tools/rename.py). Address 0x8004d9d4.

> Renamed from `func_8004D9D4` on 2026-09-24 (tools/rename.py). Address 0x8004d9d4.

Unit `class_3bb8c_d`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TitleMenu__Tick`: 58/58 words match.

## Source

```c
void TitleMenu__Tick(TitleMenu *self)
{
    void (*fn)(TitleMenu *);

    Get_vtable_TaskCore()->slot90(self);
    switch (self->state) {
    case 1:
        self->unk38 = 0;
        self->unkA4->methods->slotF0(self->unkA4, 0, 1);
        fn = self->methods->slot94;
        break;
    case 2:
        fn = self->methods->slot130;
        break;
    case 3:
        fn = self->methods->slot134;
        break;
    case 4:
        self->unk38 = 2;
        fn = self->methods->slot94;
        break;
    default:
        return;
    }
    fn(self);
}
```

## Derivation

A 5-valued dispatch (`self->state` in `{0,1,2,3,4}`) whose `case 1` and
`case 4` branches share a physical call site (`self->methods->slot94`) and
whose `case 2`/`case 3` reach the SAME call site with a different function
pointer. Modeled as the round-12 "crossjump-merge-safe local function
pointer variable" idiom (`void (*fn)(TitleMenu *)`) rather than retyping
any of `slot94`/`slot130`/`slot134` to force the merge -- all three are
independent slots with the same signature, so unifying the call physically
through one local variable reproduces retail's single shared `jalr` site
without touching any vtable's typing.

**One near-miss (55/58), and it changed a struct field's real type.** My
first reading of `case 4`'s `sw $a0, 0x38($s0)` took it at face value as
`self->unk38 = self;` (`$a0` still holding `self` from the very first
call's argument setup, never reassigned along that control path since none
of the intervening comparisons touch `$a0`). That produced the right VALUE
in this specific run only by accident of a stale register -- and it scored
wrong twice over: the switch's own comparison constant (`self->state ==
2`) also landed in the WRONG register ($v0` instead of retail's `$a0`).
Both residues resolved together once re-read correctly: `$a0` is never
reassigned between the initial `ori $a0, $zero, 0x2` (materializing the
switch's `case 2` comparand) and the `case 4` store, so retail is not
reading a leftover `self` -- it is reading the leftover LITERAL `2`,
reused because `case 4` stores that same constant into `unk38`. Writing
`self->unk38 = 2;` let the compiler keep the constant resident in one
register across the whole function and closed both residues in the same
build; this is also why `TitleMenu::unk38` is typed `s32`, not a pointer.

## Struct changes (additive, `include/class_3bb8c.h`)

- `TitleMenu::unk38` (s32; `0` or `2`, see the retyping story above),
  `TitleMenu::state` (s32, the dispatch value) -- new fields.
- `TitleMenuMethods::slot94`/`slot130`/`slot134` -- three new slots, all
  `void (*)(TitleMenu *self)`.

### Proposed learning

**A leftover register's identity is not evidence of what it holds --
trace every intervening comparison, not just calls, before trusting a
"stale register still holds X" reading.** The existing "a delay slot
belongs to the taken path" and "jal's delay slot carries the PRECEDING
call's value" entries are both about calls; this is the same trap for
comparison/branch instructions with no calls between them at all. Here the
tell that the "self" reading was wrong was a SECOND, seemingly unrelated
residue (the switch's own comparison register) resolving simultaneously
once the constant-reuse reading replaced it -- a useful pattern to watch
for when two residues in one function both vanish from a single source
change: it usually means they shared one cause, not two.

## Naming (round 77, naming runner delta)

Renamed `func_8004D9D4` -> `TitleMenu__Tick`. **Tier B**: Unconditionally calls the base class's own `slot90`, then dispatches on the state field `state` among three of this class's own handler slots. Named for the "unconditional base call, then per-state sub-dispatch" shape; the individual state meanings are not established.

## Field rename

`TitleMenu::unk58` -> `state`, **tier B**. Checked with the
compiler-ownership recipe (renamed the field alone in the `TitleMenu`
struct definition, rebuilt default build and
`tools/check-nonmatching.sh`): the accessor set landed entirely inside
`src/class_3bb8c_d.c` (this function's own `switch`, and
`TitleMenu__CommitNameEntry`'s read/write), so this was renamed directly
rather than proposed. `unk58` also names unrelated fields on other structs
in this same header (e.g. `TaskObjF::unk58`) -- untouched, since their own
definitions were not edited.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `Get_vtable_TaskCore()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 88, bravo)

TitleMenu is unified in include/TitleMenu.h (TASKCORE_SLOTS/TASKCORE_FIELDS plus its own). `state` (+0x058) is TaskCore's `activeSlot` and `unk38` its `result` (TaskCore__Init returns it: 0 on activeSlot 1, 2 on activeSlot 4, the value GameApplication__PollGraphRoomStatus retries on); slot94 is refreshViewValue, slot130/134 this class's updateMemcardSaveWithIcon/updateMemcardSaveStatus. Byte-identical (whole image green, 0 new warnings, nonmatching green).
