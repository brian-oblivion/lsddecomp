# MoviePlayer__Play -- MATCHED (59/59 words)

> Renamed from `func_800457C0` on 2026-09-25 (tools/rename.py). Address 0x800457c0.

Round 82, runner echo (code_33808 session, echo #9), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 59/59 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Start a movie: only when no movie is active (gActiveMoviePlayer NULL). Calls MoviePlayer__MarkPlaying(self) first when +0x68 is set, keeps arg2 at +0x5C and opens `name` on the stream object at +0x60 (its +0x044, with 100). On success it becomes the active movie (gActiveMoviePlayer = self), clears +0x40/+0x3C/+0x48/+0x44, sets +0x4C, keeps arg3/arg4 at +0x54/+0x58 and calls DrawSystem +0x078 with (&gMovieFrameRect, &self->rect at +0x20); returns 0. Returns 1 when the open fails, 0 when a movie is already active.

Table slot (`tools/classtable.py`): D_8006F614 +0x040.

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808`, `CountedBuf33808` and `Req44858` sit at the
top of / earlier in `src/code_33808.c`.

```c
/* D_8006F614 +0x040: start playing -- only when no movie is active
 * (gActiveMoviePlayer): optionally MoviePlayer__MarkPlaying first (+0x68), keep `arg2` at
 * +0x5C, open `name` on the stream object at +0x60 (its +0x044, 100); 1 when
 * that fails. Otherwise become the active movie, reset the state words, keep
 * `arg3`/`arg4` at +0x54/+0x58 and register gMovieFrameRect with the frame
 * rectangle (+0x20) through DrawSystem +0x078. 0. */
typedef struct StreamMethods457C0 {
    /* +0x000 */ u8 pad0[0x44];
    /* +0x044 */ s32 (*open)();
} StreamMethods457C0;

typedef struct Stream457C0 {
    /* +0x000 */ StreamMethods457C0 *methods;
} Stream457C0;

typedef struct Obj457C0 {
    /* +0x000 */ u8 pad0[0x20];
    /* +0x020 */ s16 rect[4];
    /* +0x028 */ u8 pad28[0x14];
    /* +0x03C */ s32 unk3C;
    /* +0x040 */ s32 unk40;
    /* +0x044 */ s32 unk44;
    /* +0x048 */ s32 unk48;
    /* +0x04C */ s32 unk4C;
    /* +0x050 */ s32 unk50;
    /* +0x054 */ s32 unk54;
    /* +0x058 */ s32 unk58;
    /* +0x05C */ s32 unk5C;
    /* +0x060 */ Stream457C0 *unk60;
    /* +0x064 */ s32 unk64;
    /* +0x068 */ s32 unk68;
} Obj457C0;

typedef struct DrawSysMethods457C0 {
    /* +0x000 */ u8 pad0[0x78];
    /* +0x078 */ void (*slot78)();
} DrawSysMethods457C0;

typedef struct DrawSys457C0 {
    /* +0x000 */ DrawSysMethods457C0 *methods;
} DrawSys457C0;

extern DataSrc33808 *gActiveMoviePlayer;
extern s32 gMovieFrameRect;
extern void *GetDrawSystem(void);
void MoviePlayer__MarkPlaying();

s32 MoviePlayer__Play(Obj457C0 *self, char *name, s32 arg2, s32 arg3, s32 arg4) {
    DrawSys457C0 *ds;

    if (gActiveMoviePlayer == NULL) {
        if (self->unk68 != 0) {
            MoviePlayer__MarkPlaying(self);
        }
        self->unk5C = arg2;
        if (self->unk60->methods->open(self->unk60, name, 100) == 0) {
            gActiveMoviePlayer = (DataSrc33808 *)self;
            self->unk40 = 0;
            self->unk3C = 0;
            self->unk4C = 1;
            self->unk48 = 0;
            self->unk44 = 0;
            self->unk54 = arg3;
            self->unk58 = arg4;
            ds = GetDrawSystem();
            ds->methods->slot78(ds, &gMovieFrameRect, self->rect);
            return 0;
        }
        return 1;
    }
    return 0;
}
```

## Notes

Ninth build. Every shape with the failing open as an early `if (open(...) != 0) return 1;` measured 41/59, 2 words short: retail keeps TWO separate `move v0,zero` returns (the body's tail `j <epi>; move v0,zero` and the `gActiveMoviePlayer != NULL` target right before the epilogue), mine merged them. Tried and unchanged (41/59): no return at the end of the body; a `ret` variable for either or both returns; `goto done` to a label at the end; an explicit `else { return 0; }`. The early `if (gActiveMoviePlayer != NULL) return 0;` first is far worse (25/59, it inlines the return). What matches is the SUCCESS path nested under `if (open(...) == 0) { ...; return 0; } return 1;`: the `return 1` block then sits between the two `return 0`s when jump optimisation runs, so they cannot be merged, and reorg afterwards steals its `li v0,1` into the `bnez` delay slot and deletes the block -- leaving exactly retail's layout. The matched-already MoviePlayer__DrawStrip lost its unit-local `extern DrawSys45BC8 *GetDrawSystem(void);` in favour of one `extern void *GetDrawSystem(void);` declared here (first use), with a cast at its call; its bytes are unchanged (whole image green).

## Naming

- **MoviePlayer__Play**, tier A. Slot +0x040: starts playing a named movie file when none is already active, registers the frame rectangle with the draw system.

## Track 4 (2026-09-26, round 87, bravo)

The `DrawSys457C0` view is gone; the call goes through `include/DrawSystem.h`. DrawSystem +0x078 is `clearImage(self, u8 *color, DrawRect *rect)` (occupant DrawSystem__ClearImage), so this call clears the frame rectangle at +0x20 with `gMovieFrameRect` as the COLOR (a zero word: black), not "registers gMovieFrameRect with the rectangle". The source now reads `ds->methods->clearImage(ds, (u8 *)&gMovieFrameRect, (DrawRect *)self->rect)`; pointer casts only, byte-identical. Proposed (not done, not this class): rename `gMovieFrameRect` -> `gMovieClearColor`.
