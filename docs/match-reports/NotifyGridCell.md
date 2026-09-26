# NotifyGridCell

> Renamed from `func_8004B2D4` on 2026-09-22 (tools/rename.py). Address 0x8004b2d4.

**Unit:** class_3ac78 · **Size:** 18 words · **Status:** MATCHED (18/18 words)

## What it does

`Class866E8`'s slot +0x0E0-adjacent helper: if `self` is non-NULL and a flag
bit is set on it, calls its own slot +0x038.

## Derivation

```
beqz $a0, .L8004B30C          ; if (self == NULL) skip
lhu  $v0, 0x36($a0)           ; self->flags36
andi $v0, $v0, 0x80
beqz $v0, .L8004B30C          ; if (!(flags36 & 0x80)) skip
lw   $v0, 0x0($a0)
lw   $v0, 0x38($v0)           ; ->slot38
jalr $v0                      ; self->methods->slot38(self), no extra args
```

Slot +0x038 is `Class866E8__OnNotify` (also this unit, still `INCLUDE_ASM`, not
implemented this round). No literal/forwarded args are set up before the
`jalr` beyond `self` itself (unlike `Class866E8__DispatchLinkCommand`/`Class86668__PlaySound`, this
function has no second parameter to forward — nothing else reads `$a1` in
its body), so the call is `slot38(self)` only.

`flags36` (`u16` at `Class866E8`+0x36) is a new field established this
round; bit `0x80` gates the dispatch.

## Proposed learning

None beyond what's already documented for `Class866E8` in `Class86668__PlaySound.md`.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B2D4` | `NotifyGridCell` | B | A free function, not a vtable slot and not a method of `Class866E8` -- its `self` is a GRID CELL, which is why it is named `VerbNoun` rather than `Class__Method`. Its only caller is `Class866E8__DispatchToRectCells`, which passes a cell out of an element's grid and then every cell chained behind it. The body dispatches the cell's own `+0x038` slot when the cell is non-NULL and its `flags36 & 0x80` is set; `include/code_8220.h` establishes `+0x038` as `BasicClassMethods::onNotify`. The two extra parameters are forwarded implicitly -- the call sets up no registers, so `$a1`/`$a2` still hold this function's own incoming arguments, which is exactly why the signature was widened in an earlier round. |

Parameters renamed: `self` -> `cell`, `arg1` -> `sender`, `arg2` -> `command`.
A parameter rename does not move a byte; whole-image SHA1 re-verified.

Tier B rather than A because "grid cell" comes from the caller, not from this
body, and because `flags36`'s bit `0x80` has no established meaning.
