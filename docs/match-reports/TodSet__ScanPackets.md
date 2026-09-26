# TodSet__ScanPackets -- MATCHED (19/19 words)

> Renamed from `func_800453DC` on 2026-09-25 (tools/rename.py). Address 0x800453dc.

Round 82, runner echo (code_33808 session, echo #6), 2026-09-25. Unit `code_33808`.
Byte-exact on the SECOND build (first-build miss described below); whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 19/19 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

`return (u8)self->methods->slot7C(self, arg1, arg2, &buf->entries[buf->count] + 2);` First build spelled the pointer `&buf->entries[buf->count + 2]`: same length, but GCC folded the 0x10 into the scaled index BEFORE adding the base (`addiu a3,a3,16; addu a3,v1,a3`) where retail adds the base first and the 0x10 last (13/15 words). `&entries[count] + 2` matched on the second build.

Table slot (`tools/classtable.py`): D_8006F590 +0x078.

## Source

The unit-local view `DataSrc33808` (a Class6D430 subclass built with the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
/* D_8006F590 +0x078: slot +0x07C over the data past the buffer's counted
 * array. */
u8 TodSet__ScanPackets(DataSrc33808 *self, s32 arg1, s32 arg2) {
    CountedBuf33808 *buf = self->buffer;

    return self->methods->slot7C(self, arg1, arg2, &buf->entries[buf->count] + 2);
}
```

## Notes

- No shared header was edited. `Class6D430.h`, `SceneNode.h`, `BasicClass.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

### Proposed learning

`lw n; sll 2; addu base; ...; addiu K` (base added before the constant) is
`&p->arr[n] + K/4`; `&p->arr[n + K/4]` puts the `addiu K` before the `addu`.

## Naming

- **TodSet__ScanPackets**, tier A. Slot +0x078: forwards to slot7C over the data past the buffer's counted array (shared mechanism with Tod__ScanPackets/ScanTodPackets).

## Track 4 (2026-09-26, round 88, delta)

Now `u8 TodSet__ScanPackets(TodSet *self, u8 *out, u32 *sel)` (include/TodSet.h), as Tod__ScanPackets was typed in round 86: the pass-through arguments are ScanTodPackets' `out`/`sel`, and it calls the named slot `scanTodPackets` (+0x07C) instead of DataSrc33808's unprototyped `slot7C`. The data pointer is `(u32 *)&buf->entries[buf->count] + 2`, the same arithmetic. Bytes unchanged.
