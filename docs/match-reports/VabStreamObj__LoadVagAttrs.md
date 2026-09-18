> Renamed from `func_8002C890` on 2026-09-18 (tools/rename.py). Address 0x8002c890.

# VabStreamObj__LoadVagAttrs -- MATCHED 107/107 (round 43)

Unit `code_179d8_e`. Previously filed as a `gp_rel` stall (round 17, never
attempted); reopened round 42 once `--gp-symbols`/`--no-nop-mflo-mfhi`
resolved that blocker project-wide. This is `gVabStreamObjMethods`'s own `+0x7C` slot
(the header comment's original guess was already right here, confirmed
against `asm/data/5E140.data.s`): once the VAB body transfer completes
(`VabStreamObj__OnBodyReady` sets `unk58 = 1` and dispatches here), this function pulls
the real VAB header and per-program/per-tone attribute tables out of the
sound driver and caches them on the object.

## Derivation

```c
typedef struct ProgAtr179D8E {
    u8 tones; /* +0x00, program's tone count, written by SsUtGetProgAtr */
    u8 pad1[0x10 - 0x1];
} ProgAtr179D8E;

void VabStreamObj__LoadVagAttrs(ObjDA34 *self)
{
    ProgAtr179D8E prog;
    Chunk179D8E *pool;
    s32 i;
    s32 j;
    s16 result;

    if (self->unk58 == 0) {
        return;
    }
    self->methods->slot5C(self);
    self->unk10 = gPendingVabBuffer;
    result = SsUtGetVabHdr(self->unk54, &self->unk2C);
    if (result == -1) {
        return;
    }
    self->unk4C = func_80017B34(self->unk2C.vs << 5);
    if (self->unk4C == NULL) {
        return;
    }
    self->unk50 = func_80017B34(self->unk2C.ts << 2);
    if (self->unk50 == NULL) {
        return;
    }
    pool = self->unk4C;
    for (i = 0; i < self->unk2C.ts; i++) {
        self->unk50[i] = pool;
        result = SsUtGetProgAtr(self->unk54, i, &prog);
        if (result == -1) {
            return;
        }
        for (j = 0; j < prog.tones; j++) {
            result = SsUtGetVagAtr(self->unk54, i, j, pool);
            if (result == -1) {
                return;
            }
            pool++;
        }
    }
    if (gVabVolumeInited == 0) {
        func_80032998();
        SsSetMVol(0x78, 0x78);
        gVabVolumeInited = 1;
    }
}
```

Allocates `self->unk4C` as a flat `VagAtr` pool sized `vs` entries (one per
vag in the bank) and `self->unk50` as a `ts`-entry pointer array (one per
program); `self->unk2C.vs`/`self->unk2C.ts` are `VabHdr`'s own program/vag
counts, filled by `SsUtGetVabHdr` two lines earlier -- see `VabHdr179D8E`,
added to this unit alongside this function. For each program, records where
its tones start in the pool, fetches its `ProgAtr` (only the `tones` count
byte matters here), then walks that many `VagAtr` entries out of the pool via
`SsUtGetVagAtr`, advancing the shared pool pointer once per tone (not once
per program -- confirmed by where retail's `addiu $s2,$s2,0x20` actually sits,
in the INNER loop's own branch-delay slot). On first successful pass through
the whole bank, calls `func_80032998` and sets the shared master volume once
(`gVabVolumeInited` guards it from repeating).

## Result

First full-image-correct build: byte-exact.

```
VabStreamObj__LoadVagAttrs: 107/107 words match (file 0x1D090-0x1D23C)
```

`./build-and-verify.sh`: `OK: build matches retail SLPS_015.56`. This was the
last of this unit's original round-17 "BLOCKED" cluster; all seven are now
matched.

### Proposed learning

**Same signedness lesson as `VabStreamObj__VabStreamObj`/`VabStreamObj__Update`, a THIRD field
on the same struct: `ObjDA34::unk58` is `u16`, not `s16`.** The tell was
identical -- retail's `lhu v0,0x58(s1)` where a signed field compiles to
`lh`. Three of this struct's four half-word fields discovered this round
(`unk2A`, `unk58`, and `unk5A` which was already `u16` from before this
round) are all unsigned; only `unk54`/`unk56` are signed. Worth checking
EVERY `s16` guess against the actual load instruction rather than assuming
a "state/flag" field defaults to signed.

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
