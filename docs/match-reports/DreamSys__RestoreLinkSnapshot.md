# DreamSys__RestoreLinkSnapshot

> Renamed from `func_8005B990` on 2026-09-22 (tools/rename.py). Address 0x8005b990.

**Unit:** DreamSys · **Size:** 36 words · **Status:** MATCHED (36/36 words, full build verified byte-exact)
**Vtable slot:** `gDreamSysMethods +0x224`

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

## Naming

- **Tier A.** Restore counterpart of DreamSys__SaveLinkSnapshot: copies the two snapshots back, then clears unk_0x14->unk_0x0 to 0. Called on the "link attempt failed / mismatched" paths of DreamSys__NotifyLinkAttempt and DreamSys__WallLink.

## Round 97 (alpha): Sony's GsCOORDINATE2

SceneNodeSub14 is deleted: SceneNode.coord2 is Sony's GsCOORDINATE2 (flg; MATRIX coord, whose t is the offset from the parent; MATRIX workm, whose t is the world position; param, super, sub -- 0x50 bytes, offset for offset). Accessors here follow the compiler's list: tx/ty/tz -> coord.t[0]/[1]/[2], unk38 -> workm.t; a local that holds coord.t or workm.t is `long *` (MATRIX.t is long[3]; s32 is int); any cast to GsCOORDINATE2 * is gone. Byte-identical.
