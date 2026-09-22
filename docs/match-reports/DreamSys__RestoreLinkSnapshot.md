# DreamSys__RestoreLinkSnapshot

> Renamed from `func_8005B990` on 2026-09-22 (tools/rename.py). Address 0x8005b990.

**Unit:** DreamSys · **Size:** 36 words · **Status:** MATCHED (36/36 words, full build verified byte-exact)
**Vtable slot:** `DREAMSYS_METHODS +0x224`

See `docs/match-reports/DreamSys__SaveLinkSnapshot.md` for the full writeup -- this
function is the restore half of a save/restore pair with `DreamSys__SaveLinkSnapshot`
(the save half), documented together since neither makes sense read in
isolation. This file exists so `tools/progress.py` sees a report for this
function's own name specifically.

## The C

```c
void DreamSys__RestoreLinkSnapshot(DreamSys *this)
{
	DreamSysUnk14 *p = this->unk_0x14;

	*p = this->unk14Snapshot;
	*p->unk_0x44 = this->unk14TailSnapshot;
	p->unk_0x0 = 0;
}
```

## Provenance

round 2026-09-02, runner ALPHA, unit DreamSys.
