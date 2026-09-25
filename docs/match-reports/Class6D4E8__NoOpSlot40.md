# Class6D4E8__NoOpSlot40 -- MATCHED (2/2 words)

> Renamed from `func_800272C8` on 2026-09-25 (tools/rename.py). Address 0x800272c8.

Unit `code_179d8_o`, round 26 (2026-09-09). A bare `jr $ra; nop` leaf --
retail's own compiled body for this vtable slot is genuinely empty.

## What it is

```c
void Class6D4E8__NoOpSlot40(void)
{
}
```

`D_8006D4E8 + 0x040` (per `tools/classtable.py`), a new slot the base
"BasicClass" table (`D_8006B58C`, 14 slots) does not have at all -- see
`New_Class6D4E8.md`. An empty `void(void)` function needs no stack frame
at `-O2` (nothing to save, nothing to compute), so it compiles to exactly
`jr $ra` / `nop` regardless of what argument list the slot's OTHER
overrides elsewhere in the project might actually carry -- an empty body
produces identical bytes for any parameter list, so the true arity (if any)
is undetermined by this function alone and not needed to match it.

Matched on the first attempt.

## Naming

Round 79 (delta).

- **`Class6D4E8__NoOpSlot40`** (was `func_800272C8`) -- **tier A** (a pure
  leaf whose mechanics are its purpose: it does nothing). It is D_8006D4E8
  slot +0x040, which is null in Class6D430's table, so Class6D4E8 fills an
  abstract slot with an empty body. Naming follows the existing
  `ObjM__NoOpSlot40` / `Actor__NoOpSlotD8` convention. What the slot is
  FOR is not established here: no caller of +0x040 has been traced.
- Not Sony: `sdkname.py` reports EXACT hits (SsUtVibrateOff, __nulldev,
  KeyOnCheck, ...), but every empty 2-word Sony function matches any empty
  body, so those hits carry no evidence. The function is referenced from a
  game class table, sits between two game methods of the same class, and
  `progress.py`/`config/sdk-in-game.txt` count it as game code.
