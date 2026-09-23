# TaskObjF__AdvanceState -- MATCH (111/111 words, ~4 attempts)

> Renamed from `Class86E00_3bb8c_g__AdvanceState` on 2026-09-23 (tools/rename.py). Address 0x80050034.

> Renamed from `func_80050034` on 2026-09-23 (tools/rename.py). Address 0x80050034.

Unit `class_3bb8c_g`, class `Class86E00_3bb8c_g`. State-transition dispatcher
driven by `self->unk28` (dense `switch`, retail compiles it to
`jtbl_80011594`): three case groups each fire a pair of `slot8C`/`slot7C`
(or `slot78`/`slot74`) transition calls, with one group (`unk28==0xE`)
additionally rebuilding two path/name buffers first.

```c
void TaskObjF__AdvanceState(Class86E00_3bb8c_g *self)
{
    Class86E00Methods_3bb8c_g *methods = self->methods;

    switch (self->unk28) {
    case 2:
    case 4:
    case 0xA:
    case 0xE:
        methods->slot8C(self, 0);
        if (self->unk28 == 0xE) {
            strcpy((char *)self->unk40, self->unk30);
            strcat((char *)self->unk40, ((char **)self->unk3C)[(s32)self->unk80]);
            strcpy((char *)self->unk44, ((char **)self->unk38)[(s32)self->unk80]);
        }
        if (self->unk24 == 2) {
            methods->slot78(self, self->unk40, self->unk44, self->unk48,
                             self->unk4C, self->unk50, self->unk54, self->unk58);
        } else if (self->unk24 == 1) {
            methods->slot74(self, self->unk40, self->unk44, self->unk54, self->unk58);
        }
        break;
    case 6:
        methods->slot8C(self, 0);
        methods->slot7C(self, 7);
        break;
    case 3:
    case 5:
    case 8:
    case 9:
    case 0xC:
    case 0xD:
    case 0x10:
        methods->slot8C(self, 0x10);
        methods->slot7C(self, 0x17);
        break;
    }
}
```

## Deriving the shape

The dense range check (`(self->unk28 - 2) unsigned < 0xF`, i.e.
`self->unk28` in `2..0x10`) plus `jtbl_80011594`'s 15 entries in
`asm/nonmatchings/class_3bb8c_g/TaskObjF__AdvanceState.s` give the case grouping
directly by READING the table, not by guessing: entries land on only four
distinct labels, so it's four case groups, not fifteen. **The one place
this cost an attempt: index 11 (`self->unk28==0xD`) lands on the SAME
label as `3/5/8/9/0xC/0x10`** -- easy to miss by eye since `0xD` sits
between two OTHER groups' members (`0xC` above it goes to the same place,
`0xE` above THAT goes to a different one) and reading the table
line-by-line rather than value-by-value drops it. Missing it produces a
build that's byte-exact everywhere except ONE rodata word inside the
jump table itself (the generated table entry for the omitted case),
outside the function's own instruction range -- `funcdiff` still flagged
it because it's included in the function's line range in the `.s` file.

Every case body's shared tail (`slot8C(self, N); [slot7C(self, M);]`)
matches the SAME `self->methods->slot8C`/`slot7C` shared-tail idiom
already established by `TaskObjF__ForceIdleFromState`/`TaskObjF__TickStateDelay`/`TaskObjF__OnCommand`/
`TaskObjF__OnItemSelected` elsewhere in this unit (`slot7C`'s header comment already
documents it as a common tail for exactly this reason).

The `self->unk28==0xE` sub-case's three calls read `self->unk40` as a
`char *` destination and `self->unk44` likewise -- both fields are
ALREADY typed `s32` in the header (established by `TaskObjF__AttachChildA`/
`TaskObjF__OnCommand`, which forward them as opaque `s32` args to
`slot78`/`slot4C`). Cast at the use site (`(char *)self->unk40`) rather
than retyping the field, per the project's documented
"reinterpretation is free, retyping a field read elsewhere is not"
convention -- `self->unk40`/`unk44` are read as plain `s32` by THREE
already-matched functions in this same file.

`self->unk80` is similarly already typed `void *` in the header
(`TaskObjF__OnItemSelected`'s own return-value store), but here it's used as an
INTEGER ARRAY INDEX (shifted left 2, added to a base pointer) -- cast
`(s32)self->unk80` at the use site, same free-reinterpretation reasoning,
same field, opposite direction (pointer read as an integer here instead
of an integer read as a pointer).

Two fields and one method slot are new (this function is the first to
touch them), added ADDITIVELY to `include/class_3bb8c.h`, splitting
existing padding, total size unchanged:
- `Class86E00_3bb8c_g::unk24` (`s32`, `+0x024`) -- the secondary
  `slot78`-vs-`slot74` dispatch code, nested inside the `unk28`-driven
  switch's shared case body.
- `Class86E00_3bb8c_g::unk30` (`char *`, `+0x030`) -- `strcpy`'s source
  for `self->unk40` in the `unk28==0xE` sub-case.
- `Class86E00_3bb8c_g::unk3C` (`void *`, `+0x03C`) -- base of a pointer
  array indexed by `(s32)self->unk80`, the SAME shape as the
  already-typed `unk38` right below it (which the SAME sub-case indexes
  the identical way for `self->unk44`'s own `strcpy`).
- `Class86E00Methods_3bb8c_g::slot74` (`+0x074`, 4 args, immediately
  before the already-typed `slot78` at `+0x078`) -- the `self->unk24==1`
  alternative to `slot78`'s `self->unk24==2` call, same first two
  arguments (`unk40`, `unk44`), only the trailing two of `slot78`'s four
  (`unk54`, `unk58` instead of `unk4C`/`unk50`/`unk54`/`unk58`).

## The one real residue: `switch` case ORDER vs comparison order

First working version used a genuine nested `switch (self->unk24) { case
2: ...; case 1: ...; }` for the `slot78`/`slot74` alternative -- scored
64278/64279 (one word short, plus the jtbl miss above, fixed separately).
GCC 2.6.3 lowered that inner switch to a sequential-compare form that
checks `self->unk24 == 1` FIRST regardless of the case order written in
source (apparently normalizing to ascending case-value order for a
2-case non-table switch), where retail checks `== 2` FIRST. Replacing the
inner `switch` with explicit `if (self->unk24 == 2) {...} else if
(self->unk24 == 1) {...}` forces the source's own written comparison
order to survive into codegen and closed this to byte-exact.

### Proposed learning

For a 2-3 value dispatch too small to become a jump table, don't assume a
`switch` statement preserves the CASE LABEL order you wrote it in --
GCC 2.6.3 appears to normalize small non-table switches to ascending
value order for the sequential compares. If retail's compare order
doesn't match ascending, use `if`/`else if` instead, which does preserve
source order literally.

## Naming

`TaskObjF__AdvanceState` (was `func_80050034`), tier B: dispatch
on the CURRENT `self->unk28`, with one group (`0xE`) additionally rebuilding
two path/name buffers before firing the shared `slot8C`/`slot78`-or-`slot74`
tail -- the same "reset then transition" mechanics `TaskObjF__SetState`
already fires from the opposite direction (after choosing a NEW state).
Named for the mechanics (moves the state machine forward from wherever it
currently is); which real transition each of the four case groups performs
in game terms is not established.
