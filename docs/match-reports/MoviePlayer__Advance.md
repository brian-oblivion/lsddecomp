# MoviePlayer__Advance -- MATCHED (60/60 words)

> Renamed from `func_80045948` on 2026-09-25 (tools/rename.py). Address 0x80045948.

Round 82, runner echo (GraphicsResources session, echo #9), 2026-09-25. Unit `GraphicsResources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 60/60 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Per-tick stream step, active only when `self` is the object in sActiveMoviePlayer. With the stream running (+0x50 nonzero): call the stream object's +0x050 with (1, +0x5C); if +0x50 then reads negative, the loop count at +0x58 is consumed (`loops == 0 || --loops == 0`) and at the end the stream's +0x064 is called; then self->+0x50 = 0, +0x64 = 1, return 0. Stopped (+0x50 zero) with +0x64 set: tail-return its own +0x068. Otherwise falls off the end (v0 unset -- on the +0x64 path it happens to hold that zero).

Table slot (`tools/classtable.py`): gMoviePlayerMethods +0x048.

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable`, `SubBlockTable` and `ResourceSourceArgs` sit at the
top of / earlier in `src/graphics/GraphicsResources.c`.

```c
/* gMoviePlayerMethods +0x048: when this is the object in sActiveMoviePlayer -- with the
 * stream running (+0x50), call the stream object's +0x050 (1, +0x5C); if
 * +0x50 then went negative, count down the loops left at +0x58 and at the
 * last one (or with none) call the stream's +0x064; clear +0x50, set +0x64,
 * 0. Stopped with +0x64 set: tail-return its own +0x068. */
typedef struct StreamMethods45948 {
    /* +0x000 */ u8 pad0[0x50];
    /* +0x050 */ void (*slot50)();
    /* +0x054 */ u8 pad54[0x10];
    /* +0x064 */ void (*slot64)();
} StreamMethods45948;

typedef struct Stream45948 {
    /* +0x000 */ StreamMethods45948 *methods;
} Stream45948;

typedef struct Methods45948 {
    /* +0x000 */ u8 pad0[0x68];
    /* +0x068 */ s32 (*slot68)();
} Methods45948;

typedef struct Obj45948 {
    /* +0x000 */ Methods45948 *methods;
    /* +0x004 */ u8 pad4[0x4C];
    /* +0x050 */ s32 unk50;
    /* +0x054 */ s32 unk54;
    /* +0x058 */ s32 loops;
    /* +0x05C */ s32 unk5C;
    /* +0x060 */ Stream45948 *unk60;
    /* +0x064 */ s32 unk64;
} Obj45948;

s32 MoviePlayer__Advance(Obj45948 *self) {
    Obj45948 *cur = (Obj45948 *)sActiveMoviePlayer;

    if (cur == self) {
        if (cur->unk50 == 0) {
            if (cur->unk64 == 0) {
                goto out;
            }
        } else {
            cur->unk60->methods->slot50(cur->unk60, 1, cur->unk5C);
            if (cur->unk50 < 0) {
                if (cur->loops == 0 || --cur->loops == 0) {
                    cur->unk60->methods->slot64(cur->unk60);
                }
            }
            self->unk50 = 0;
            self->unk64 = 1;
            return 0;
        }
        return cur->methods->slot68(cur);
    }
out:
    ;
}
```

## Notes

Third build. Body-first (`if (unk50 != 0) {...} if (unk64) return slot68(cur);`) was 13/60: retail tests the stopped case first and places the slot68 call LAST behind a `j`. `if (unk50 == 0) { if (unk64 == 0) return 0; } else {...; return 0;} return slot68(cur);` gave that layout at 57/60, with one extra `move v0,zero` (retail's `beqz` has a nop delay slot: no return value on that path); `return cur->unk64;` there is identical. `goto out;` to a label at the very end (fall off, no value) matches; so does the mirror `if (unk64 != 0) goto call;` with the call behind a label at the bottom. Local views `Methods45948`/`StreamMethods45948`/`Stream45948`/`Obj45948` just above.

## Naming

- **MoviePlayer__Advance**, tier A. Slot +0x048: while the stream is running, advances the CD stream and, once negative (finishing), decrements the loop counter and stops the stream at zero.

## Track 4 (2026-09-26, round 87)

The +0x060 object is a CdStream (include/CdStream.h, unified this round). `Stream45948`/`StreamMethods45948` are deleted; `unk60` is `CdStream *`; slot50 is `startRead`, slot64 is `mute`. MoviePlayer's own view and field names are unchanged. Zero bytes changed.

## Track 4 (2026-09-26, round 89)

Class unified in `include/MoviePlayer.h` (id 0x70, table `gMoviePlayerMethods`, was `D_8006F614`; a direct BasicClass subclass, 0x6C bytes). The unit-local views in `src/graphics/GraphicsResources.c` are gone; Obj45948/Methods45948 are gone; the tail call through +0x068 is `decodeFrame`, `unk64` -> `started`, `unk5C` -> `frameCount`. Byte-identical; `typeviews.py --warnings` 0 new.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `unk50` | `pendingStart` | B | see MoviePlayer__MoviePlayer |

MATCHING line at `out:;`: retail returns no value on that path.
