# Class81940__AdvanceLoadState -- MATCHED (45/45 words)

> Renamed from `DataSrc39094__SetFlag` on 2026-09-26 (tools/rename.py). Address 0x800489b4.

> Renamed from `func_800489B4` on 2026-09-25 (tools/rename.py). Address 0x800489b4.

Round 82, runner echo (third echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the first build; whole-image SHA1 green, funcdiff 45/45.

## What it does

Slot +0x064 (setFlag override) of gClass81940Methods. State `unk2A` 9 with flag bit
0x80: clear state, `headerReady = 1`, and if `autoLoadData` call slot +0x080
(Class81940__LoadDataBlock). State 10 with bit 0x80: `dataReady = 1`, clear state. Then the
active data source's setFlag(self).

## Source

```c
/* slot +0x064 of gClass81940Methods (setFlag) */
void Class81940__AdvanceLoadState(DataSrc39094 *self) {
    if (self->unk2A == 9) {
        if (self->flags & 0x80) {
            self->unk2A = 0;
            self->headerReady = 1;
            if (self->autoLoadData != 0) {
                self->methods->loadDataBlock();
            }
        }
    } else if (self->unk2A == 10) {
        if (self->flags & 0x80) {
            self->dataReady = 1;
            self->unk2A = 0;
        }
    }
    GetActiveDataSourceMethods()->setFlag((FileResource *)self);
}
```

## Notes

The local view's `DataSrc39094Methods.loadDataBlock` was retyped from `void *` to
`void (*)(void)` (local to `src/code_39094.c`, no other reader), and later in
the same session to the unprototyped `s32 (*)()` once Class81940__LoadDataBlock (its
occupant, which returns s32 and reads `self`) was matched; still byte-exact. Retail does
not set `$a0` before `jalr` on slot +0x080 even though its occupant
Class81940__LoadDataBlock surely reads `self`: `$a0` still holds `self` from entry, but
GCC 2.6.3 would have emitted `move a0,s0` for an explicit argument (as it
does for the setFlag call), so the source call has no arguments.

## Naming

- **Name:** `Class81940__AdvanceLoadState`
- **Tier:** A
- **Evidence:** slot +0x064 (setFlag override); this is where state 9 (header) and state 10 (data block) both complete, established this round in the unit header comment.

## Proposed field names

`self->unk2A` is NOT a `DataSrc39094`-local field: it is FileResource's own
last field (`include/FileResource.h`, `/* +0x02A */ u16 unk2A`), a UNIFIED
shared header this unit does not own and must not edit. This unit gives it a
clear, consistent meaning across four of its own functions
(Class81940__AdvanceLoadState, Class81940__CancelRequests, Class81940__LoadHeader,
Class81940__LoadDataBlock): 0 = idle, 9 = header load in flight, 10 = data
block load in flight.

- **Proposed name:** `loadState`
- **Tier:** B (mechanics -- three-value state used consistently as a
  load-in-progress marker -- established only from this one subclass's
  usage; FileResource.h's own comment says only that it is the last field
  before a subclass's own fields start, with no meaning of its own).
- **Evidence:** every read/write of `unk2A` in `src/code_39094.c` (this
  report; Class81940__CancelRequests, Class81940__LoadHeader,
  Class81940__LoadDataBlock).
- **Caution for the head applying this:** FileResource has sixteen
  subclasses (`typeviews.py --tree`); this proposal is evidenced from ONE of
  them. Renaming the shared field is safe for the build (a name change alone
  is byte-neutral) but should be cross-checked against at least one other
  subclass's usage of `unk2A` before it is taken as the field's general
  meaning.

## Track 4 (2026-09-26, round 87)

Renamed `DataSrc39094__SetFlag` -> `Class81940__AdvanceLoadState` with `rename.py`. The slot is +0x064, FileResource's `setFlag`, which the CD driver calls when a request completes (src/code_179d8_s.c); the bit this body tests, 0x80, is that driver's CD_FLAG_READ_DONE. The body advances the load state (`unk2A` 9 -> 0 with the header ready, maybe starting the data block; 10 -> 0 with the data ready), which is what TimBlockSrc__AdvanceLoadState and VabStreamObj__AdvanceLoadState, the same slot's occupants in two siblings, were named for. Its no-argument call of +0x080 is kept through `Class81940LoadDataBlockNoArgFn` (include/Class81940.h): retail sets no $a0 for it. The class (method table gClass81940Methods, id 0x903, a FileResource subclass) was named `Class81940` for its table address, 0x80081940 (renamed from `D_80081940` to `gClass81940Methods`), as Class6D940 is (FINISHING-PLAN track 4 step 2); the old `DataSrc39094` was the unit's local view name, and dropping its unit suffix leaves `DataSrc`, which every FileResource subclass is. The unified definition is `include/Class81940.h`.
