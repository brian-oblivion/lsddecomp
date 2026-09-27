# FindMaxTimBlockSize -- MATCHED (23/23 words)

> Renamed from `MaxOfBufferWords` on 2026-09-26 (tools/rename.py). Address 0x800434dc.

> Renamed from `func_800434DC` on 2026-09-25 (tools/rename.py). Address 0x800434dc.

Round 82, runner echo (code_33808 session, echo #7), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 23/23 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Returns the largest (unsigned) of the `count` words at +0x14 of the buffer (count is its word 0). Retail's 8-byte leaf frame with no stack use is produced by the plain for loop as written.

Table slot (`tools/classtable.py`): none (not in any table).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/code_33808.c`.

```c
/* The largest of the buffer's `count` words from +0x14. */
typedef struct TimBlockHeader {
    /* +0x00 */ u32 count;
    /* +0x04 */ u8 pad4[0x10];
    /* +0x14 */ u32 vals[1];
} TimBlockHeader;

u32 FindMaxTimBlockSize(FileResource *self) {
    TimBlockHeader *buf = self->buffer;
    u32 i;
    u32 max = 0;

    for (i = 0; i < buf->count; i++) {
        if (max < buf->vals[i]) {
            max = buf->vals[i];
        }
    }
    return max;
}
```

## Notes

- Byte-exact on the first build.
- Not referenced by any data word (`grep` of asm/data finds no pointer to it); called from code elsewhere or unused.
- No shared header was edited; prototypes for other units' functions are local
  to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **FindMaxTimBlockSize**, tier B. Free helper: the largest of the buffer's counted words from +0x14; used only by TimBlockSrc__AdvanceLoadState to size the block sector buffer.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `MaxOfBufferWords` | `FindMaxTimBlockSize` (rename.py) | A | a pure leaf: the largest of the header's `count` words from +0x14, which are the block sizes (see TimBlockSrc__AdvanceLoadState's row); its one caller sizes the block buffer with it |
| `TimBlockHeader` `pad4[0x10]`, `vals[1]` | `offsets[4]`, `sizes[4]` | A | as above |
