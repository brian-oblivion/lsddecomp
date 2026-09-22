# DreamSys__ApplyLinkCommand — MATCHED, 79/79 words, byte-exact

> Renamed from `func_80058C58` on 2026-09-22 (tools/rename.py). Address 0x80058c58.

Unit: `DreamSys` · Size: 79 words · Status: MATCH, whole-image SHA1 green
(`build exit=0`).

## Summary

A flat 13-arm dense jump-table `switch` on `mode - 2`, dispatching mostly
single field stores plus two vtable calls. Pre-analysed by round 24's head
(see `docs/PROGRESS.md`, round 24 "Next round" entry) and left with no report
so it would stay `fresh`.

```c
void DreamSys__ApplyLinkCommand(DreamSys *this, s32 arg1, s32 mode)
{
	if (this->unk_0x6c != 0)
		return;
	if (this->unk_0x70 != 0)
		return;
	if (this->unk_0x908 != 0)
		return;

	switch (mode - 2) {
	case 0:
		this->unk_0xA0 = 1;
		break;
	case 1:
		this->unk_0xA0 = 2;
		break;
	case 2:
		this->unk_0xA4 = 1;
		break;
	case 3:
		this->unk_0xA4 = 2;
		break;
	case 4:
		this->unk_0x88 = 1;
		break;
	case 5:
		if (this->unk_0xA0 == 1)
			this->vt->DreamSys__ChangeMoveMode(this, 4);
		break;
	case 6:
		this->unk_0x88 = 2;
		break;
	case 11:
		this->unk_0x90 = 2;
		break;
	case 12:
		this->unk_0xA0 = 4;
		break;
	case 13:
		this->unk_0x90 = 1;
		break;
	case 14:
		this->unk_0xA0 = 3;
		break;
	case 23:
		this->unk_0x74 = 1;
		break;
	case 32:
		this->vt->DreamSys__RestorePreviousMoveMode(this);
		break;
	case 47:
		break;
	}
}
```

`arg1` ($a1) is a real incoming parameter slot (the third case-5 call
overwrites $a1 with the literal 4 as that call's own argument setup, which is
why $a1 has to exist as a register in the calling convention even though the
body never reads it). Its type/meaning is unconfirmed; kept as a bare `s32`.

## The one real puzzle: jump-table span, not case values

The 13 real arms only go up to `mode - 2 == 32` (`mode == 34`). A switch built
from exactly those 13 case labels compiles (GCC 2.6.3, `-O2`) to a **33-entry**
table (span 0..32) — this was the first attempt and it built clean but
desynced the WHOLE image from the jump-table's own rodata slot onward
(`funcdiff` showed the diff starting at `off=0x1F88`, the table's first word,
confirming an address-drift rather than a body error — see CLAUDE.md
"Address drift").

Retail's table is **48 entries** (span 0..47, i.e. `mode` 2..49), even though
everything from index 33 onward is byte-identical to the `default` target.
Adding one more explicit case — `case 47: break;` (body identical to
`default`, contributes nothing at runtime) — was enough to make GCC widen the
table to the full 0..47 span with every unlisted slot auto-filled to
`default`. That single label is the whole difference between 33 and 48
entries; nothing else needed to change.

This is consistent with the head's round-broadcast on `CheckDreamAuxTriggerCondition`,
generalised to this family of dense switches: **retail's original source had
at least one more explicit `case` at the high end of the range than the
"real" (non-default) arms alone would suggest**, and it can be recovered
purely by width-matching the jump table via `funcdiff`'s address-drift
signal — no register-identity or block-order issue here, just table span.

## Head broadcast answer — bare-`j`-to-join / block-order lever

**No such `j` in this function, and the block-order lever from `CheckDreamAuxTriggerCondition`
does not apply here.** Every one of the 13 real arms in retail ends its own
block with an explicit `j .L80058D84` (the shared epilogue), except the last
one (`case 32`, `DreamSys__RestorePreviousMoveMode` call), which naturally falls off the end of
the switch into the epilogue with no `j` at all — a plain "last arm falls
through" case, not a retail `j`-with-delay-slot the C had to reproduce by
reordering blocks. Writing `case 32` last in source with no trailing `break`
after it would reproduce that identically, but since the switch already
returns after the epilogue naturally I left an explicit `break;` on `case 32`
followed by a no-op `case 47: break;` for range-width purposes only — the
generated code was still byte-exact, so no block-order sensitivity was
observed for this function. The residue this function actually had was the
addressed-mode table-WIDTH puzzle above, not a block-order one.

## Proposed learning

A dense-switch jump table's span can legitimately exceed the highest
non-default case a decompiler infers from the arms alone. Before accepting a
33-vs-48-style shorter table, check `funcdiff`'s FIRST diff offset: if it
lands exactly on the rodata jump-table's own base address (not inside the
function body), the fix is almost certainly a missing high-end `case N: break;`
matching `default`, not a body/order defect. Add one past the table's known
`sltiu` bound (`bound - 1` in the post-subtraction index space) and rebuild.

## Naming

- **Tier B.** A `switch (mode - 2)` over ~14 distinct case values, each setting one or two of the link-state fields (movementBlocked-adjacent unk_0xA0/0xA4/0x88/0x74) or calling DreamSys__ChangeMoveMode / DreamSys__RestorePreviousMoveMode, gated by three busy flags. The individual case semantics are not established, only that this is a mode/command dispatcher.
