# TimBlockSrc__AdvanceLoadState -- MATCHED (183/183 words)

> Renamed from `func_80043200` on 2026-09-25 (tools/rename.py). Address 0x80043200.

Round 82, runner echo (code_33808 session, echo #9), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 183/183 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

The gTimBlockSrcMethods loader's state machine, run under Lock/UnlockActiveDataSource and only once the read has completed (flags bit 0x80). State 9 (header sector read by the ctor TimBlockSrc__TimBlockSrc): copy the 0x24-byte header -- a u32 block count and eight u32 file offsets -- from the sector buffer (+0x34) into the 0x24-byte buffer, free the sector buffer, take the largest offset (FindMaxTimBlockSize) as the new sector-buffer size, allocate the object array (+0x30, count words) and the sector buffer (+0x34, size at +0x38), seek (own +0x04C) to offset[0] and read (own +0x054) one block: state 10. State 10: wrap the block in a new gTimArraySrcMethods source (New_TimArraySrc(NULL)), buffer = the sector buffer, size 0, its +0x34 = &self->entries (+0x40, the CLUT base TimArraySrc__BuildImages adds to), run its setFlag (TimArraySrc__BuildImages, which builds the TimImages) and +0x078; count it at +0x2C, then seek/read the next block (state 10 again) or, after the last, free the sector buffer, clear +0x34/+0x38/+0x2A, set +0x3C and call the active driver's setFlag. Either allocation failing sets +0x80.

Table slot (`tools/classtable.py`): gTimBlockSrcMethods +0x064 (setFlag override).

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable`, `CountedBuf33808` and `Req44858` sit at the
top of / earlier in `src/code_33808.c`.

```c
/* gTimBlockSrcMethods +0x064: the loader's state machine, under the data-source
 * lock. State 9 (header sector read): copy the 0x24-byte header (a count
 * and eight file offsets) out of the sector buffer into the buffer, free
 * the sector, allocate the object array (+0x30) and a sector buffer of the
 * largest offset (+0x34/+0x38), seek to the first block and read it: state
 * 10. State 10 (a block read): hand the block to a new gTimArraySrcMethods source
 * (its CLUT base the entries at +0x40), run its setFlag and +0x078, and
 * read the next block -- or, after the last, free the sector buffer, mark
 * +0x3C done and run the active driver's setFlag. An allocation failure
 * sets +0x80. */
typedef struct Hdr43200 {
    u8 bytes[0x24];
} Hdr43200;

extern void LockActiveDataSource(void);
extern void UnlockActiveDataSource(void);
u32 FindMaxTimBlockSize(FileResource *self);
void *New_TimArraySrc(s32 arg0);

void TimBlockSrc__AdvanceLoadState(Obj43068 *self) {
    DataSrc33808 **p;
    s32 max;
    s32 n;

    LockActiveDataSource();
    switch (self->unk2A) {
        case 9:
            if (self->flags & 0x80) {
                *(Hdr43200 *)self->buffer = *(Hdr43200 *)self->sector;
                BMemPMgrFree(self->sector);
                max = FindMaxTimBlockSize((FileResource *)self);
                self->unk30 = (s32)BMemPMgrAlloc(*(u32 *)self->buffer * 4);
                if (self->unk30 == 0) {
                    goto fail;
                }
                self->sector = BMemPMgrAlloc(max);
                if (self->sector == NULL) {
                    goto fail;
                }
                self->unk38 = max;
                self->methods->seek((DataSrc33808 *)self, ((u32 *)self->buffer)[1], 0);
                self->methods->read((DataSrc33808 *)self, self->sector, max);
                self->unk2A = 10;
            }
            break;
        case 10:
            if (self->flags & 0x80) {
                n = self->unk2C;
                p = (DataSrc33808 **)self->unk30 + n;
                *p = New_TimArraySrc(0);
                (*p)->buffer = self->sector;
                (*p)->bufferSize = 0;
                (*p)->unk34 = (s32)self->entries;
                n++;
                (*p)->methods->setFlag(*p);
                ((void (*)())(*p)->methods->slot78)(*p);
                self->unk2C = n;
                if (n < *(u32 *)self->buffer) {
                    self->methods->seek((DataSrc33808 *)self, ((u32 *)self->buffer)[n + 1], 0);
                    self->methods->read((DataSrc33808 *)self, self->sector, self->unk38);
                    self->unk2A = 10;
                } else {
                    BMemPMgrFree(self->sector);
                    self->sector = NULL;
                    self->unk38 = 0;
                    self->unk2A = 0;
                    self->unk3C = 1;
                    GetActiveDataSourceMethods()->setFlag((FileResource *)self);
                }
            }
            break;
    }
    goto out;
fail:
    self->unk80 = 1;
out:
    UnlockActiveDataSource();
}
```

## Notes

Second build (the first was already 183/183; the second only added local Lock/UnlockActiveDataSource prototypes, which the unit declares further down, to avoid implicit declarations). The shape: a two-case `switch (self->unk2A)` (retail `beq 9; beq 10; j <out>`), `goto fail` for both allocation failures with the `fail:` block (`unk80 = 1`) after the switch and a `goto out` over it; cc1 cross-jumps the two identical `read(...); unk2A = 10;` tails into retail's shared `L80043460`. The header copy is a struct assignment through a `u8 bytes[0x24]` view (`Hdr43200`, alignment 1), which is what produces retail's runtime-aligned block move (`or; andi 3; beqz` choosing a lw/sw loop or a lwl/lwr loop, 0x20 bytes, then one 4-byte tail). Uses the `Obj43068` view TimBlockSrc__TimBlockSrc introduced.

## Naming

- **TimBlockSrc__AdvanceLoadState**, tier B. setFlag override implementing the two-state (9 header-read, 10 block-read) loader state machine described in the function's own header comment.

## Track 4 (2026-09-25, round 83, bravo)

Occupant of FileResource's `setFlag` slot (+0x064); keeps its name because the body is the whole loader state machine, not a flag set. Correction to the prose above: the 0x24-byte header is a count, four file offsets (+0x04) and four sizes (+0x14, what FindMaxTimBlockSize maximises), not eight offsets. The class (id 0xF03, table `gTimBlockSrcMethods`) is unified as `TimBlockSrc` in `include/TimBlockSrc.h`. Any source block above is the pre-unification spelling; the live body in `src/code_33808.c` takes the unified types, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).


## Track 4 (2026-09-26, round 88, runner alpha)
TimArraySrc unified (include/TimArraySrc.h): TimBlockSrc's `blocks` is now `struct TimArraySrc **`, so `p` is a TimArraySrc ** with no cast, New_TimArraySrc(NULL), the store to +0x034 is `clutBase`, and slot78 is called through TimArraySrcUploadFn. Byte-identical.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `9` / `10` / `0` | `TIMBLOCK_LOAD_HEADER` / `TIMBLOCK_LOAD_BLOCK` / `TIMBLOCK_LOAD_IDLE` (enum TimBlockLoadState, include/TimBlockSrc.h) | A | 9 is set by the ctor before the header read and its branch parses the header; 10 is set before each block read and its branch consumes a block; 0 after the last |
| `0x80` | `CD_FLAG_READ_DONE` | A | src/code_179d8_s.c's name for the flags bit the CD driver sets when a read request completes; both branches wait on it after a read() |
| `((u32 *)buffer)[0]`, `[1]`, `[n + 1]` | `Buf434DC` `count`, `offsets[0]`, `offsets[n]` | A | the header's words from +0x04 are what each block read seeks to; the words from +0x14 (FindMaxTimBlockSize) size the buffer every block is read into, so they are the sizes |
| `* 4` | `* sizeof(*self->blocks)` | A | the TimArraySrc pointer array |

MATCHING line kept on the header copy: `Hdr43200` is a byte array so the copy is byte-aligned, which is what gives retail's runtime alignment test (`or; andi 3; beqz`) choosing a word or an lwl/lwr loop.
