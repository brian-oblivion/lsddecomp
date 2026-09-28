# VabStreamObj__LoadVagAttrs -- MATCHED 107/107 (round 43)

> Renamed from `func_8002C890` on 2026-09-18 (tools/rename.py). Address 0x8002c890.

Unit `vab_sound`. Previously filed as a `gp_rel` stall (round 17, never
attempted); reopened round 42 once `--gp-symbols`/`--no-nop-mflo-mfhi`
resolved that blocker project-wide. This is `gVabStreamObjMethods`'s own `+0x7C` slot
(the header comment's original guess was already right here, confirmed
against `asm/data/5E140.data.s`): once the VAB body transfer completes
(`VabStreamObj__OnBodyReady` sets `unk58 = 1` and dispatches here), this function pulls
the real VAB header and per-program/per-tone attribute tables out of the
sound driver and caches them on the object.

## Derivation

```c
typedef struct ProgAtrView {
    u8 tones; /* +0x00, program's tone count, written by SsUtGetProgAtr */
    u8 pad1[0x10 - 0x1];
} ProgAtrView;

void VabStreamObj__LoadVagAttrs(VabStreamObj *self)
{
    ProgAtrView prog;
    VagAtrView *pool;
    s32 i;
    s32 j;
    s16 result;

    if (self->attrsReady == 0) {
        return;
    }
    self->methods->slot5C(self);
    self->streamBuffer = sPendingVabBuffer;
    result = SsUtGetVabHdr(self->vabId, &self->vabHdr);
    if (result == -1) {
        return;
    }
    self->vagAttrPool = BMemPMgrAlloc(self->vabHdr.vs << 5);
    if (self->vagAttrPool == NULL) {
        return;
    }
    self->progVagTable = BMemPMgrAlloc(self->vabHdr.ts << 2);
    if (self->progVagTable == NULL) {
        return;
    }
    pool = self->vagAttrPool;
    for (i = 0; i < self->vabHdr.ts; i++) {
        self->progVagTable[i] = pool;
        result = SsUtGetProgAtr(self->vabId, i, &prog);
        if (result == -1) {
            return;
        }
        for (j = 0; j < prog.tones; j++) {
            result = SsUtGetVagAtr(self->vabId, i, j, pool);
            if (result == -1) {
                return;
            }
            pool++;
        }
    }
    if (sVabVolumeInited == 0) {
        SsStart();
        SsSetMVol(0x78, 0x78);
        sVabVolumeInited = 1;
    }
}
```

(Types/fields updated to round 52's renames -- `ProgAtr179D8E`/
`Chunk179D8E` are now `ProgAtrView`/`VagAtrView`; `ObjDA34::unk58/unk10/
unk54/unk2C/unk4C/unk50` are now `attrsReady`/`streamBuffer`/`vabId`/
`vabHdr`/`vagAttrPool`/`progVagTable`. Bytes unchanged.)

Allocates `self->vagAttrPool` as a flat `VagAtr` pool sized `vs` entries
(one per vag in the bank) and `self->progVagTable` as a `ts`-entry pointer
array (one per program); `self->vabHdr.vs`/`self->vabHdr.ts` are `VabHdr`'s
own program/vag counts, filled by `SsUtGetVabHdr` two lines earlier -- see
`VabHdrView`, added to this unit alongside this function. For each program, records where
its tones start in the pool, fetches its `ProgAtr` (only the `tones` count
byte matters here), then walks that many `VagAtr` entries out of the pool via
`SsUtGetVagAtr`, advancing the shared pool pointer once per tone (not once
per program -- confirmed by where retail's `addiu $s2,$s2,0x20` actually sits,
in the INNER loop's own branch-delay slot). On first successful pass through
the whole bank, calls `SsStart` and sets the shared master volume once
(`sVabVolumeInited` guards it from repeating).

## Result

First full-image-correct build: byte-exact.

```
VabStreamObj__LoadVagAttrs: 107/107 words match (file 0x1D090-0x1D23C)
```

`./build-and-verify.sh`: `OK: build matches retail SLPS_015.56`. This was the
last of this unit's original round-17 "BLOCKED" cluster; all seven are now
matched.

### Proposed learning

**Same signedness lesson as `VabStreamObj__VabStreamObj`/`VabStreamObj__AdvanceLoadState`, a THIRD field
on the same struct: `attrsReady` (`ObjDA34::unk58` before round 52) is
`u16`, not `s16`.** The tell was identical -- retail's `lhu v0,0x58(s1)`
where a signed field compiles to `lh`. Three of this struct's four
half-word fields discovered this round (`loadState`, `attrsReady`, and
`bodyTransferPending` which was already `u16` from before this round) are
all unsigned; only `vabId`/`muted` are signed. Worth checking EVERY `s16`
guess against the actual load instruction rather than assuming a
"state/flag" field defaults to signed.

**A guard condition that duplicates the following loop's own entry check
costs a real word, and the mechanism generalizes beyond this function.**
Writing `if (prog.tones > 0) { for (j = 0; j < prog.tones; j++) ... }`
compiles to TWO comparisons against `prog.tones` (the `if` and the loop's
own test) where retail has exactly ONE (`blez` guarding a do-while-shaped
loop). Dropping the redundant `if` and trusting the bare `for` loop's own
zero-iteration handling reproduced retail's single-branch shape exactly.
Anyone who writes an explicit skip-guard in front of a loop whose own
condition already covers the empty case should check whether retail's
instruction count agrees before assuming the guard is free.

**`gVabStreamObjMethods`'s table can and should be read whole from
`asm/data/5E140.data.s` before attempting ANY of its dispatch functions** --
see `VabStreamObj__VabStreamObj.md`'s table for the slots this round pinned down. Two
(`+0x58`, `+0x6C`) are null in retail and never actually exercised at
runtime for an object of this exact base class.

## Naming

Renamed `func_8002C890` -> `VabStreamObj__LoadVagAttrs`, tier A. Confirmed
`gVabStreamObjMethods`'s own +0x7C slot; the whole body is a concrete,
unambiguous sequence of Sony VAB attribute-table fetches
(`SsUtGetVabHdr`/`SsUtGetProgAtr`/`SsUtGetVagAtr`) building exactly the two
per-object tables (`vagAttrPool`, `progVagTable`) those calls fill --
mechanics ARE the purpose here.

## Round 98 (charlie, track 7)

Sony's `ProgAtr` replaces the local `ProgAtrView`; `SsUtGetVabHdr` and
`SsUtGetVagAtr` now carry `<libsnd.h>`'s prototypes, so the call sites cast
vab_stream_obj.h's reduced views: `(VabHdr *)&self->vabHdr`, `(VagAtr *)pool`.
The two allocations are spelled `vs * sizeof(VabStreamVagAtr)` and
`ts * sizeof(VabStreamVagAtr *)` (were `<< 5`, `<< 2`), and the master
volume `VAB_MASTER_VOLUME` (120, was `0x78`). Byte-exact.
