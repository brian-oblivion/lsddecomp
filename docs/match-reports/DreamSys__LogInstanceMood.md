# DreamSys__LogInstanceMood

**Unit:** DreamSys · **Size:** 12 instructions · **Status:** MATCHED (12/12 words)

## What it does

Vtable `+0x1F8`. A thin wrapper: calls `this->vt->LogMood(this,
&this->entityMoods, source)`.

## The C

```c
void DreamSys__LogInstanceMood(DreamSys *this, MoodGraphPoint *source)
{
	this->vt->LogMood(this, &this->entityMoods, source);
}
```

`&this->entityMoods` comes from the raw offset `0x154`, which resolves to
`entityMoods` exactly (`areaMoods` is a `MoodGraphContributor`, 0x10 bytes,
starting at `0x144`; `entityMoods` immediately follows at `0x154`). The
called vtable slot (`this->vt->LogMood`) was confirmed at `+0x208` via
`tools/classtable.py gDreamSysMethods`, matching the already-declared
`LogMood` field exactly -- `DreamSys__LogMood` itself is `+0x208`
(`0x8005B5B8`), the function right after `DreamSys__ClearMoodGraph`
(`+0x204`) in address order, same pattern the whole table follows.

## Provenance

round 2026-08-30-c, runner ALPHA, unit DreamSys (whole-unit second pass).
