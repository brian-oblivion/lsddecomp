# DataSrc39094__SetFlag -- MATCHED (45/45 words)

> Renamed from `func_800489B4` on 2026-09-25 (tools/rename.py). Address 0x800489b4.

Round 82, runner echo (third echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the first build; whole-image SHA1 green, funcdiff 45/45.

## What it does

Slot +0x064 (setFlag override) of D_80081940. State `unk2A` 9 with flag bit
0x80: clear state, `headerReady = 1`, and if `autoLoadData` call slot +0x080
(DataSrc39094__LoadDataBlock). State 10 with bit 0x80: `dataReady = 1`, clear state. Then the
active data source's setFlag(self).

## Source

```c
/* slot +0x064 of D_80081940 (setFlag) */
void DataSrc39094__SetFlag(DataSrc39094 *self) {
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
    GetActiveDataSourceMethods()->setFlag((Class6D430 *)self);
}
```

## Notes

The local view's `DataSrc39094Methods.loadDataBlock` was retyped from `void *` to
`void (*)(void)` (local to `src/code_39094.c`, no other reader), and later in
the same session to the unprototyped `s32 (*)()` once DataSrc39094__LoadDataBlock (its
occupant, which returns s32 and reads `self`) was matched; still byte-exact. Retail does
not set `$a0` before `jalr` on slot +0x080 even though its occupant
DataSrc39094__LoadDataBlock surely reads `self`: `$a0` still holds `self` from entry, but
GCC 2.6.3 would have emitted `move a0,s0` for an explicit argument (as it
does for the setFlag call), so the source call has no arguments.

## Naming

- **Name:** `DataSrc39094__SetFlag`
- **Tier:** A
- **Evidence:** slot +0x064 (setFlag override); this is where state 9 (header) and state 10 (data block) both complete, established this round in the unit header comment.

## Proposed field names

`self->unk2A` is NOT a `DataSrc39094`-local field: it is Class6D430's own
last field (`include/Class6D430.h`, `/* +0x02A */ u16 unk2A`), a UNIFIED
shared header this unit does not own and must not edit. This unit gives it a
clear, consistent meaning across four of its own functions
(DataSrc39094__SetFlag, DataSrc39094__CancelRequests, DataSrc39094__LoadHeader,
DataSrc39094__LoadDataBlock): 0 = idle, 9 = header load in flight, 10 = data
block load in flight.

- **Proposed name:** `loadState`
- **Tier:** B (mechanics -- three-value state used consistently as a
  load-in-progress marker -- established only from this one subclass's
  usage; Class6D430.h's own comment says only that it is the last field
  before a subclass's own fields start, with no meaning of its own).
- **Evidence:** every read/write of `unk2A` in `src/code_39094.c` (this
  report; DataSrc39094__CancelRequests, DataSrc39094__LoadHeader,
  DataSrc39094__LoadDataBlock).
- **Caution for the head applying this:** Class6D430 has sixteen
  subclasses (`typeviews.py --tree`); this proposal is evidenced from ONE of
  them. Renaming the shared field is safe for the build (a name change alone
  is byte-neutral) but should be cross-checked against at least one other
  subclass's usage of `unk2A` before it is taken as the field's general
  meaning.
