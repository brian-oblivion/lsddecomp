# TimBlockSrc__TimBlockSrc -- MATCHED (80/80 words)

> Renamed from `func_80043068` on 2026-09-25 (tools/rename.py). Address 0x80043068.

Round 82, runner echo (GraphicsResources session, echo #9), 2026-09-25. Unit `GraphicsResources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 80/80 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Constructor: the active driver's ctor, install gTimBlockSrcMethods, clear +0x2C/+0x30/+0x3C/+0x34/+0x38, and lay out four 16-byte channel entries at +0x40: shift = gTimBlockClutShift, mask = 1 << shift, +4 = 0, +6 = 0x1E0 + the running sum of masks, +8 = 0x100, +0xA = 1 (TimBlockSrc__SetEntryShift later rewrites one entry's shift/mask, TimBlockSrc__FadeEntry its vector at +0xC). Then allocate a 0x24-byte header buffer and a 0x800-byte sector buffer (+0x34); with both, adopt the header as the data-source buffer (size 0x24), set state 9 at +0x2A, clear +0x80, open `name` (own +0x044 with 1, 0) and read one sector into +0x34 (own +0x054).

Table slot (`tools/classtable.py`): gTimBlockSrcMethods +0x008 (its allocator New_TimBlockSrc passes one argument).

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable`, `SubBlockTable` and `ResourceSourceArgs` sit at the
top of / earlier in `src/GraphicsResources.c`.

```c
/* gTimBlockSrcMethods +0x008: constructor -- the active driver's, then this table;
 * clear +0x2C..+0x3C and lay out the four channel entries at +0x40 (shift
 * gTimBlockClutShift, its mask, consecutive slots from 0x1E0); then adopt a 0x24-byte
 * header buffer (state 9 at +0x2A), allocate the 0x800-byte sector buffer
 * at +0x34, open `name` and read the first sector into it. */
typedef struct Ent43068 {
    /* +0x00 */ u16 shift;
    /* +0x02 */ u16 mask;
    /* +0x04 */ u16 unk4;
    /* +0x06 */ u16 addr;
    /* +0x08 */ u16 unk8;
    /* +0x0A */ u16 unkA;
    /* +0x0C */ u8 padC[4];
} Ent43068;

typedef struct Obj43068 {
    FILERESOURCE_FIELDS(DataSrc33808Methods);
    /* +0x02C */ s32 unk2C;
    /* +0x030 */ s32 unk30;
    /* +0x034 */ void *sector;
    /* +0x038 */ s32 unk38;
    /* +0x03C */ s32 unk3C;
    /* +0x040 */ Ent43068 entries[4];
    /* +0x080 */ s32 unk80;
} Obj43068;

extern s16 gTimBlockClutShift;

void TimBlockSrc__TimBlockSrc(Obj43068 *self, char *name) {
    Ent43068 *e;
    void *hdr;
    s32 i;
    u16 addr;
    s16 shift;
    u16 mask;

    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetTimBlockSrcMethods();
    self->unk2C = 0;
    self->unk30 = 0;
    self->unk3C = 0;
    self->sector = NULL;
    self->unk38 = 0;
    addr = 0;
    mask = 1 << gTimBlockClutShift;
    shift = gTimBlockClutShift;
    for (i = 0; i < 4; i++) {
        e = &self->entries[i];
        e->shift = shift;
        e->mask = mask;
        e->unk4 = 0;
        e->addr = addr + 0x1E0;
        addr += mask;
        e->unk8 = 0x100;
        e->unkA = 1;
    }
    hdr = BMemPMgrAlloc(0x24);
    if (hdr != NULL) {
        self->sector = BMemPMgrAlloc(0x800);
        if (self->sector != NULL) {
            self->bufferSize = 0x24;
            self->buffer = hdr;
            self->unk2A = 9;
            self->unk80 = 0;
            self->methods->open((DataSrc33808 *)self, name, 1, 0);
            self->methods->read((DataSrc33808 *)self, self->sector, 0x800);
        }
    }
}
```

## Notes

Twenty-ninth build (four families of levers, all needed together): (1) an `s16 shift` local holding gTimBlockClutShift -- with s32 the frame was 0x20 against retail's 0x28 and `s32 unused[2]` fixed only the frame; the s16 local gives the 0x28 frame by itself (no unused array); (2) `u16 addr` AND `u16 mask` locals: with s32 (or u32) either one the running sum lost retail's `addu v0,a3,a0; move a3,v0` pair (one word short); s16/u16 for both is what matches; (3) the invariant order: `mask = 1 << gTimBlockClutShift; shift = gTimBlockClutShift;` puts the `sllv` ahead of the `move t0,v1` copy -- `shift = ...; mask = 1 << shift;`, `mask = 1 << (shift = ...)` and `mask = 1 << gTimBlockClutShift` after the shift all leave them swapped (78/80); (4) the loop as an index (`e = &self->entries[i]`) -- a walking `e++` pointer was 51/80; `addr` as a strength-reduced `i * mask`/`i << shift` was worse (13-21/80), as was re-reading `e->mask`. Local views `Ent43068`/`Obj43068` just above (Obj6F0B8/Ent6F0B8 further down are untouched).

## Naming

- **TimBlockSrc__TimBlockSrc**, tier B. Constructs a sector-header + block loader with 4 CLUT palette-fade channel entries at +0x40; mechanics described, no external caller names this class (ObjMStyleActor.c's own caller comment calls its return type opaque).

## Track 4 (2026-09-25, round 83, bravo)

Occupant of +0x008. Fields now named: +0x2C `blockCount`, +0x30 `blocks`, +0x38 `sectorSize`, +0x3C `loaded`, +0x80 `failed`; each entry's +0x04..+0x0A the RECT `clutX`/`clutY`/`clutW`/`clutH` this ctor lays out (0, 0x1E0 + i * mask, 0x100, 1). The class (id 0xF03, table `gTimBlockSrcMethods`) is unified as `TimBlockSrc` in `include/TimBlockSrc.h`. Any source block above is the pre-unification spelling; the live body in `src/GraphicsResources.c` takes the unified types, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `0x1E0` | `CLUT_FADE_Y` (480) | A | the ramps' clutY base here, FadeClutRow's src.y base, and TimArraySrc__BuildImages' CLUT-row origin: the VRAM row the fade CLUTs start at |
| `0x100` | `CLUT_COLORS` (256) | A | the width of each ramp's CLUT RECT; FadeClutRow reads and writes rows of that many 16-bit colours |
| `0x24` | `sizeof(TimBlockHeaderBytes)` | A | the header buffer's allocation and bufferSize, the size AdvanceLoadState copies |
| `0x800` | `CD_SECTOR_SIZE` (2048) | A | the first read of the file, whose first 36 bytes are the header |
| `9` | `TIMBLOCK_LOAD_HEADER` | A | set before the header-sector read; AdvanceLoadState's branch for it parses the header |
| `4` | `ARRAY_COUNT(self->entries)` | A | the loop fills `entries[4]` |

## History (moved from include/TimBlockSrc.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
 * carries a byte of this class's layout, so they expand FILERESOURCE's macros
 * directly (round 83).
```
