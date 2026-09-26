# MoviePlayer__PollActive -- MATCHED (26/26 words)

> Renamed from `func_80045C94` on 2026-09-25 (tools/rename.py). Address 0x80045c94.

Round 82, runner echo (code_33808 session, echo #7), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 26/26 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

If +0x54 is set: post-increments the global gMoviePollCounter and, when its old value was over 100, resets it to 1 and calls slot +0x044 with self; returns 0. Otherwise clears gActiveMoviePlayer and returns 1.

Table slot (`tools/classtable.py`): gMoviePlayerMethods +0x064.

## Source

The unit-local view `DataSrc33808` (a Class6D430 subclass built with the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.

```c
/* gMoviePlayerMethods +0x064: while +0x54 is set, count calls in gMoviePollCounter and
 * once the count before the increment passes 100, resets it to 1 and calls
 * slot +0x044; returns 0. Otherwise
 * clears gActiveMoviePlayer and returns 1. */
typedef struct Methods45C94 {
    /* +0x000 */ u8 pad0[0x44];
    /* +0x044 */ void (*slot44)();
} Methods45C94;

typedef struct Obj45C94 {
    /* +0x000 */ Methods45C94 *methods;
    /* +0x004 */ u8 pad4[0x50];
    /* +0x054 */ s32 unk54;
} Obj45C94;

extern s32 gMoviePollCounter;
extern DataSrc33808 *gActiveMoviePlayer;

s32 MoviePlayer__PollActive(Obj45C94 *self) {
    if (self->unk54 != 0) {
        if (gMoviePollCounter++ > 100) {
            gMoviePollCounter = 1;
            self->methods->slot44(self);
        }
        return 0;
    }
    gActiveMoviePlayer = NULL;
    return 1;
}
```

## Notes

- First build 10/26, one word long: written `self->methods->slot44();` (zero-argument, since $a0 already holds self and is never set for the call), the `li v1,1` left the `bnez` delay slot and v0/v1 swapped between the function pointer and the constant. Writing `slot44(self)` gave 26/26 and a green image on the second build.
- Contrast TileMap__Load (same session): $a0 also still holds self at its slot +0x078 call, and there the zero-argument form is the one that matched. So neither form is a default; try both.
- No shared header was edited; prototypes for other units' functions are local
  to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

### Proposed learning

A slot call through an unprototyped pointer whose `$a0` already holds `self`
is NOT neutral between `f()` and `f(self)` even though no `move a0` appears in
either: the argument's liveness changes the scheduling of the surrounding
block (here a constant left the branch delay slot and an extra word appeared).
TileMap__Load matched with `f()` in the same situation and MoviePlayer__PollActive only
with `f(self)`, so when one form misses, flip it before anything else.

## Naming

- **MoviePlayer__PollActive**, tier A. Slot +0x064: while the stream is still running, throttles a periodic callback (slot44) every 100 ticks; otherwise clears the active-movie global and reports done.
