# GetDefaultMovieFrame — MATCH (4/4 words)

> Renamed from `GetDefaultStreamTaskInitData` on 2026-09-27 (tools/rename.py). Address 0x8003dfcc.

> Renamed from `func_8003DFCC` on 2026-09-19 (tools/rename.py). Address 0x8003dfcc.

**Unit:** code_2cc8c · **Size:** 4 instructions

## What it does

A tiny getter: returns the address of the static table `gDefaultMovieFrame`, a
3-word struct per `include/code_2c054.h`'s own `StreamTaskInitData` local
view (that unit's `StreamTask__StreamTask` uses it as the 5th/stack argument to a
constructor call, and `GetDefaultMovieFrame()`'s return value feeds the same
3-word copy). This unit never dereferences it, only returns its address,
so it is declared here as an opaque `u8[]`.

## The C

```c
extern u8 gDefaultMovieFrame[];

void *GetDefaultMovieFrame(void)
{
    return gDefaultMovieFrame;
}
```

## Provenance

round 12 (2026-09-03), runner alpha, unit code_2cc8c. Matched on the
first build.

## Naming

**GetDefaultMovieFrame** (renamed from `func_8003DFCC`, round 55,
runner alpha). Tier A: pure leaf getter, returns `&gDefaultMovieFrame`
(formerly `D_8006E854`), already declared `StreamTaskInitData *func_8003DFCC(void)`
in `include/code_2c054.h` and used there (`code_2c054.c`) as the fallback
default when a caller supplies no init data -- "Default" is the confirmed
mechanic (a fixed fallback constant), not a guess.


## Track 6 (2026-09-27, round 99, runner bravo)

Returns `DrawRect *` now (was `void *` here and a `StreamTaskInitData *` prototype in code_2c054.h; the local type is deleted, StreamTask__StreamTask.md). The C above is the round-12 match; the live body is `return &gDefaultMovieFrame;` over `extern DrawRect gDefaultMovieFrame;`. Byte-identical.

Names, through `tools/rename.py`:

- `gDefaultStreamTaskInitData` -> **`gDefaultMovieFrame`**, tier B. The data at 0x8006E854 is `{x 640, y 0, w 320, h 240}`. Measured uses: StreamTask__StreamTask passes it as New_MoviePlayer's `frame` (MoviePlayer::frame, the rect `play` clears and `drawStrip` walks); it is StreamTask's `initData` default; TaskCore__OnInit clears it when a task has no sub handle. "Movie frame" names the player's use; "Default" is StreamTask's fallback. Why TaskCore clears it is not established.
- `GetDefaultStreamTaskInitData` -> **`GetDefaultMovieFrame`**, tier A: a leaf getter named for what it returns. (rename.py rewrote the earlier Naming section's identifiers in place; its history reads "renamed from func_8003DFCC, round 55" to the name that function had then, GetDefaultStreamTaskInitData.)
