# MoviePlayer__PullFrame -- MATCHED (58/58 words)

> Renamed from `func_80045AD8` on 2026-09-25 (tools/rename.py). Address 0x80045ad8.

Round 82, runner echo (code_33808 session, echo #9), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 58/58 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Pull the next movie frame: unless the stream has ended (+0x48), ask the stream object at +0x60 (its +0x06C, with &data, &size, 0x800000) for data; with a nonzero result, when size is nonzero flip the frame index at +0x3C and DecDCTvlc the data into that frame's buffer (+0x14[index]), hand the data back (+0x070), and on a negative result mark the end (+0x48 = 1) and call the stream's +0x054; return 0. Return 1 when ended or when there was no data.

Table slot (`tools/classtable.py`): D_8006F614 +0x058.

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808`, `CountedBuf33808` and `Req44858` sit at the
top of / earlier in `src/code_33808.c`.

```c
/* D_8006F614 +0x058: unless the stream has ended (+0x48), pull the next
 * frame from the object at +0x60 (its +0x06C); 1 when there is none. With
 * data, flip the frame index at +0x3C and VLC-decode into that frame's
 * buffer, then hand the sector buffer back (+0x070); a negative result
 * marks the end (+0x48) and calls that object's +0x054. 0. */
typedef struct StreamMethods45AD8 {
    /* +0x000 */ u8 pad0[0x54];
    /* +0x054 */ void (*slot54)();
    /* +0x058 */ u8 pad58[0x14];
    /* +0x06C */ s32 (*slot6C)();
    /* +0x070 */ void (*slot70)();
} StreamMethods45AD8;

typedef struct Stream45AD8 {
    /* +0x000 */ StreamMethods45AD8 *methods;
} Stream45AD8;

typedef struct Obj45AD8 {
    /* +0x000 */ u8 pad0[0x14];
    /* +0x014 */ u32 *frames[2];
    /* +0x01C */ u8 pad1C[0x20];
    /* +0x03C */ s32 frameIndex;
    /* +0x040 */ u8 pad40[8];
    /* +0x048 */ s32 unk48;
    /* +0x04C */ u8 pad4C[0x14];
    /* +0x060 */ Stream45AD8 *unk60;
} Obj45AD8;

/* LIBPRESS.H */
extern int DecDCTvlc(u32 *bs, u32 *buf);

s32 MoviePlayer__PullFrame(Obj45AD8 *self) {
    u32 *data;
    s32 size;
    s32 r;

    if (self->unk48 == 0) {
        r = self->unk60->methods->slot6C(self->unk60, &data, &size, 0x800000);
        if (r != 0) {
            if (size != 0) {
                self->frameIndex ^= 1;
                DecDCTvlc(data, self->frames[self->frameIndex]);
            }
            self->unk60->methods->slot70(self->unk60, data);
            if (r < 0) {
                self->unk48 = 1;
                self->unk60->methods->slot54(self->unk60);
            }
            return 0;
        }
    }
    return 1;
}
```

## Notes

Fifth build. Early-return shape (`if (self->unk48 != 0) return 1; ... if (r == 0) return 1; ...`) measured 39/58, one word long: the final `if (r < 0)` branch stole `move v0,zero` from its target where retail fills the delay slot with the store's `li v0,1` from the fall-through. `r <= -1` and the inverted `if (r >= 0) return 0;` changed nothing; `r == -1` is wrong (40/58). Nesting the whole body under `if (unk48 == 0) { ...; if (r != 0) { ...; return 0; } } return 1;` -- one shared `return 1` at the bottom -- matches. Local views `StreamMethods45AD8`/`Stream45AD8`/`Obj45AD8` just above; DecDCTvlc is Sony's (LIBPRESS), extern only.

## Naming

- **MoviePlayer__PullFrame**, tier A. Slot +0x058: unless the stream has ended, pulls the next compressed frame from the CD stream and VLC-decodes it into the flipped frame buffer.
