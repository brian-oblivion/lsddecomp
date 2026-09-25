# MoviePlayer__Abort -- MATCHED (36/36 words)

> Renamed from `func_80045A38` on 2026-09-25 (tools/rename.py). Address 0x80045a38.

Round 82, runner echo (code_33808 session, echo #8), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 36/36 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Only when self is the object in gActiveMoviePlayer: set +0x48 = 1, clear +0x54, call the +0x60 object's +0x048 with it, set +0x44 = 1; then, if +0x64 is clear, call the +0x60 object's +0x07C with (obj, 0, 0) (clearing the callback MoviePlayer__Stop installed), set +0x64 = 1 and +0x44 = 1 again.

Table slot (`tools/classtable.py`): D_8006F614 +0x04C.

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808` and `CountedBuf33808` sit at the top of
`src/code_33808.c`.

```c
typedef struct Methods458B8 {
    /* +0x000 */ u8 pad0[0x48];
    /* +0x048 */ void (*slot48)();
    /* +0x04C */ u8 pad4C[0xC];
    /* +0x058 */ void (*slot58)();
    /* +0x05C */ u8 pad5C[0x20];
    /* +0x07C */ void (*slot7C)();
} Methods458B8;

typedef struct Sub458B8 {
    /* +0x000 */ Methods458B8 *methods;
} Sub458B8;

typedef struct Obj458B8 {
    /* +0x000 */ u8 pad0[0x3C];
    /* +0x03C */ s32 unk3C;
    /* +0x040 */ s32 unk40;
    /* +0x044 */ s32 unk44;
    /* +0x048 */ s32 unk48;
    /* +0x04C */ s32 unk4C;
    /* +0x050 */ s32 unk50;
    /* +0x054 */ s32 unk54;
    /* +0x058 */ u8 pad58[8];
    /* +0x060 */ Sub458B8 *unk60;
    /* +0x064 */ s32 unk64;
} Obj458B8;

/* D_8006F614 +0x04C: when this is the object in gActiveMoviePlayer, set +0x48,
 * clear +0x54, call the +0x60 object's +0x048, set +0x44, and the first
 * time (+0x64 clear) clear that object's +0x07C callback and set +0x64. */
void MoviePlayer__Abort(Obj458B8 *self) {
    Obj458B8 *cur = (Obj458B8 *)gActiveMoviePlayer;

    if (cur == self) {
        cur->unk48 = 1;
        cur->unk54 = 0;
        cur->unk60->methods->slot48(cur->unk60);
        cur->unk44 = 1;
        if (cur->unk64 == 0) {
            cur->unk60->methods->slot7C(cur->unk60, 0, 0);
            cur->unk64 = 1;
            cur->unk44 = 1;
        }
    }
}
```

## Notes

First build. The same through-the-global shape as MoviePlayer__Stop (a local `cur` copy of gActiveMoviePlayer). The duplicated `unk44 = 1` is real source: retail stores it in the bnez delay slot (both paths) and again inside the if. Views Methods458B8/Obj458B8 (defined at MoviePlayer__Stop) gained slot48 and unk54 additively.

## Naming

- **MoviePlayer__Abort**, tier A. Slot +0x04C: hard-stops the active movie and clears its frame-ready callback the first time.
