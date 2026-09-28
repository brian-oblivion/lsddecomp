# New_MoviePlayer -- MATCHED (35/35 words)

> Renamed from `func_80045438` on 2026-09-25 (tools/rename.py). Address 0x80045438.

Round 82, runner echo (graphics_resources session, echo #8), 2026-09-25. Unit `graphics_resources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 35/35 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Allocator for gMoviePlayerMethods: BMemPMgrAlloc(0x6C); if non-NULL, call the table's ctor (GetMoviePlayerMethods +0x008) with (obj, a, b, c); a ZERO result returns obj, nonzero frees it and returns NULL. Not referenced from any data word (a direct-call allocator).

Table slot (`tools/classtable.py`): none (allocator; constructs gMoviePlayerMethods).

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable` and `SubBlockTable` sit at the top of
`src/graphics/graphics_resources.c`.

```c
void *New_MoviePlayer(s32 arg0, s32 arg1, s32 arg2) {
    void *obj = BMemPMgrAlloc(0x6C);

    if (obj != NULL) {
        if (((UnprototypedCtorTable *)GetMoviePlayerMethods())->ctor(obj, arg0, arg1, arg2) == 0) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}
```

## Notes

First build. The failing-ctor allocator lever with the test inverted: this class's ctor returns a status (0 = ok), unlike the self-returning ctors in the rest of the unit (`beqz v0 -> return obj`).

## Naming

- **New_MoviePlayer**, tier A. Called from src/app/task.c's TaskCore__TaskCore; externally typed `StreamTaskUnkB4Obj *` there. Opens a CD stream object and drives an MDEC decode/upload state machine (mechanics match the class name; the constructor/finalize/state-machine functions ARE playing a movie).

## Track 4 (2026-09-26, round 89)

Class unified in `include/MoviePlayer.h` (id 0x70, table `gMoviePlayerMethods`, was `D_8006F614`; a direct BasicClass subclass, 0x6C bytes). The unit-local views in `src/graphics/graphics_resources.c` are gone; it returns `MoviePlayer *` and takes `(DrawRect *frame, s32 speed, s32 external)`, calling the typed ctor slot instead of the `UnprototypedCtorTable` cast. Its one caller, StreamTask__StreamTask, passes `(DrawRect *)GetDefaultMovieFrame()`: the ctor hands it to InitFrame, which copies it whole into `frame`/`stripRect` and reads its w/h words, so the three-word StreamTaskInitData is the frame rectangle. Byte-identical; `typeviews.py --warnings` 0 new.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| the size literal | `sizeof(MoviePlayer)` | A | each equals the object size the class header records (and the allocation retail makes); the image is byte-identical |
| `speed` | `cdSpeed` | A | New_CdStream's speed argument (cd_stream.h: < 4 means double speed) |
