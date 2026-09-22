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
`jalr` beyond `self` itself (unlike `Class866E8__OnCommand`/`Class86668__SetChildFlag8`, this
function has no second parameter to forward — nothing else reads `$a1` in
its body), so the call is `slot38(self)` only.

`flags36` (`u16` at `Class866E8`+0x36) is a new field established this
round; bit `0x80` gates the dispatch.

## Proposed learning

None beyond what's already documented for `Class866E8` in `Class86668__SetChildFlag8.md`.
