# DreamSys__SetTickCallbacks — MATCHED

> Renamed from `func_80059610` on 2026-09-22 (tools/rename.py). Address 0x80059610.

Round 2026-08-30, runner ALPHA, unit `DreamSys`. 23/23 words, full match.

## Source

```c
void DreamSys__SetTickCallbacks(DreamSys *this, s32 arg1, s32 arg2)
{
	this->vt->DreamSys__SelectCallback98(this, arg1);
	this->vt->DreamSys__SelectCallback80(this, arg2);
}
```

## Derivation

Vtable slot `0x134` in `gDreamSysMethods` (confirmed with
`tools/classtable.py gDreamSysMethods`). Two straight-line virtual calls: the
first passes `a1` unchanged (the callee-preserved register from entry — the
disassembly never touches `$a1` before the first `jalr`), the second passes
`a2` (saved into `$s1` across the first call, since `$a1`/`$a2` are
caller-saved). Confirmed the call targets against the class table: `0x13C` is
`DreamSys__SelectCallback98`, `0x138` is `DreamSys__SelectCallback80` — both already had signatures
`(DreamSys *this, s32 arg1)` from a previous round's header work, which this
function's calling convention (register position of the second argument)
independently corroborates.

## Proposed learning

None beyond what's already documented — a clean first-attempt match, no
residue.

## Naming

- **Tier B.** Chains DreamSys__SelectCallback98(arg1) then DreamSys__SelectCallback80(arg2) -- the enable counterpart of DreamSys__ClearTickCallbacks.
