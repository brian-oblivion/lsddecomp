# DreamSys__GetSetMoveMode

> Renamed from `func_8005A168` on 2026-09-22 (tools/rename.py). Address 0x8005a168.

**Unit:** DreamSys · **Size:** 7 instructions · **Status:** MATCHED (7/7 words)

## What it does

Vtable slot `+0x180` (first of a five-function run resolved this round via
`tools/classtable.py DREAMSYS_METHODS` -- see `DreamSys__ChangeMoveMode.md`,
`DreamSys__RestorePreviousMoveMode.md`, `DreamSys__SetGateFlags.md`, `DreamSys__SetTickPeriod.md` for the rest;
`+0x180`..`+0x190` map onto the five functions at consecutive addresses
`0x8005A168`..`0x8005A1EC`, confirmed by the tool, not assumed). A
bounds-checked setter that returns the OLD value: if `value >= 0`, writes it
into BOTH `unk_0xAC` and `unk_0xB0`; always returns the pre-call value of
`unk_0xAC`. Also called directly (not through the vtable) by
`DreamSys__SetMoveOverride` as `(this, 1)`.

## The C

```c
s32 DreamSys__GetSetMoveMode(DreamSys *this, s32 value)
{
	s32 old;

	old = this->unk_0xAC;
	if (value >= 0) {
		this->unk_0xAC = value;
		this->unk_0xB0 = value;
	}
	return old;
}
```

## New fields: `unk_0xAC` / `unk_0xB0`, a "current/paired" pair

Split out of the old `unknown_values_0xAC[24]` gap, alongside `unk_0xBC`
(see `DreamSys__StopVoice.md`, a STALL touching the same gap). Together with
`DreamSys__ChangeMoveMode` (paired copy-on-change) and `DreamSys__RestorePreviousMoveMode` (unconditional
`0xAC = 0xB0` copy), the three functions read like get/set/sync accessors
on a "target vs. current" pair, but no confirmed semantic name yet.

## Provenance

round 2026-08-30-b, runner ALPHA, address range
`0x80058774`-`0x8005A1EC`.

## Naming

- **Tier B.** Get-or-set (sentinel value < 0 means "get only") that forces moveMode and previousMoveMode to the SAME new value when setting -- distinct from DreamSys__ChangeMoveMode below, which preserves history.
