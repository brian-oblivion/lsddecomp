# Obj86ED0__SetState -- MATCHED (42/42 words)

> Renamed from `func_800512C8` on 2026-09-24 (tools/rename.py). Address 0x800512c8.

Unit `class_3bb8c_i`, carved round 14.

`Obj86ED0`'s vtable slot 0x054, called by `Obj86ED0__TickState` (that unit's own
sibling function, still `INCLUDE_ASM` this round) with a literal `4`.
Resets `unk30`, returns early for `arg1 < 2`, then dispatches on `arg1`:
`{2,3}` detaches `unk34` and calls this class's own `slot48`
(`Obj86ED0__ReleaseCardResources`) before stashing `arg1` into `unk2C`; `4` calls the
UNMODIFIED base `notifyParents` (`BasicClass__NotifyParents`, confirmed not
overridden via `tools/classtable.py gObj86ED0Methods --vs D_8006B58C`) with
`unk2C` as its argument, reached through `self->methods` since that table
slot is identical to the base's either way.

```c
void Obj86ED0__SetState(Obj86ED0 *self, s32 arg1)
{
    self->unk30 = 0;
    if (arg1 < 2) {
        return;
    }
    switch (arg1) {
    case 2:
    case 3:
        self->methods->removeChild(self, self->unk34);
        self->methods->slot48(self);
        self->unk2C = arg1;
        break;
    case 4:
        self->methods->notifyParents(self, self->unk2C);
        break;
    }
}
```

## Residue and how it was closed (2 attempts)

Retail's own branch shape is unusual for a simple range dispatch: after the
early `arg1 < 2` return, it tests `arg1 < 4` and branches TO the `{2,3}`
body on TRUE (rather than skipping AROUND it on false), then falls through
to an `arg1 == 4` test, then an EXPLICIT unconditional jump past both bodies
for anything else.

- **Attempt 1**: `if (arg1 < 4) { ... } else if (arg1 == 4) { ... }` scored
  14/42 (also shorter than retail by 8 bytes, drifting `New_Obj86ED0`'s own
  `jal` target downstream). Disassembly showed GCC inverted the first test
  (`beqz`, skip-around) rather than retail's `bnez`-to-body, and merged the
  `else if` into one fallthrough chain with an extra `bne` retail does not
  have.
- **Attempt 2**: rewrote as two independent `if` blocks, the first ending in
  an explicit `return;` (semantically identical to the `else if` for a
  `void` function). Scored IDENTICALLY (14/42, same shape) -- GCC 2.6.3's
  control-flow lowering treats `if(A){...return;} if(B){...}` and
  `if(A){...} else if(B){...}` as the same construct once it reaches this
  stage, so this was not actually a different lever from attempt 1.
- **Attempt 3**: `switch (arg1) { case 2: case 3: ...; break; case 4: ...;
  break; }`. This matched 42/42 immediately. A `switch` lowering for a
  small case set produces exactly retail's "test true jumps INTO the case
  body, explicit jump PAST all bodies for no match" shape -- the two
  adjacent case labels sharing one body is what generates the `arg1 < 4`
  range test (GCC already knows `arg1 >= 2` is established going in) instead
  of an `==` chain.

### Proposed learning

When a residue shows retail branching TOWARD a body on the taken branch
(rather than skipping around it) AND an explicit unconditional jump past
ALL bodies for the unmatched case, suspect a `switch` even if the case
values look like they would fit an `if`/`else if` chain just as well --
under this GCC, the two source forms are NOT equivalent at the RTL level
once integer promotion/multiple-case-labels are involved, even though an
`if`/`else if` chain and a `return`-terminated sequential-`if` chain (which
ARE semantically identical for a void function) compile IDENTICALLY to each
other. Don't spend a second attempt re-testing that particular pair.
