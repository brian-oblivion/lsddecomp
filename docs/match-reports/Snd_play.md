# Snd_play -- MATCHED (11/11 words)

> Renamed from `func_8003410C` on 2026-09-23 (tools/rename.py). Address 0x8003410c.

`asm/nonmatchings/libsnd_decre/Snd_play.s`, vram `0x8003410C`, unit
`libsnd_decre` (round 21, 2026-09-06).

## Summary

Trivial forwarding wrapper: sign-extends both `s16` args and forwards to
`SeqPlay` (uncarved, lives in `asm/code_179d8_tail.s`). Unlike its
neighbours `func_8003370C`/`func_80032D00`, retail does **not**
sign-truncate `$v0` after the call -- positive evidence the wrapper's own
return type is a plain `s32` (or the callee's return is simply forwarded
untouched), not `s16`. First attempt.

```c
extern s32 SeqPlay(s16 a0, s16 a1);

s32 Snd_play(s16 a0, s16 a1)
{
    return SeqPlay(a0, a1);
}
```

## Verification

`./build-and-verify.sh` exit 0 (full-image SHA1 match). `funcdiff.py
Snd_play`: 11/11 words.

### Proposed learning

None new -- reconfirms CLAUDE.md's existing note that a wrapper's own
return type must be read from whether `$v0` gets post-processed after the
call, not assumed void just because a frame is paid for.

## File history

Round 34 (2026-09-12) linked `libsnd/replay` and `libsnd/vs_vab` into the
middle of the `libsnd_decre` slice, leaving Snd_play as the one-function unit
`src/code_179d8_i_b.c`. Round 98 (track 8) renamed it `src/libsnd_play.c`
for the Sony module it is (`libsnd/play.o`, 0x2C, the unit's exact size).

## History (moved from src/libsnd_play.c, comments pass)

The arity-ok note on the SeqPlay prototype read:

> arity-ok: the definition's third parameter is never read (round 69, docs/match-reports/SeqPlay.md); its return type is void there, see PROGRESS round 69

The file's banner carried its edge evidence:

> Edges. Before: libsnd/vs_vab, a placed Sony object. After:
> libsnd_seqread.c, where tuboundary.py reads "start edge possible" -- the
> rodata is silent -- and the edge is kept because play and seqread are
> separate objects in every libsnd build.

## History (source comments moved in track 12, round 106)

From `src/psyq/libsnd_play.c`:

> "Every byte here is Sony's: libsnd/play.o is 0x2C of .text, one function,
> byte-identical on the 3.0, 3.3 and 3.5 discs (3.5 calls it _SsSndPlay), and
> this unit is exactly 0x2C. It is carried as C rather than linked because
> `psyq_sdk.py match` finds that body at four places in retail (AMBIGUOUS x4);
> the Snd_play symbol is pinned here by the libsnd object that calls it by
> relocation."
