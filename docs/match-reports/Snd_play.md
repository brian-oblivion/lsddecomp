# Snd_play -- MATCHED (11/11 words)

> Renamed from `func_8003410C` on 2026-09-23 (tools/rename.py). Address 0x8003410c.

`asm/nonmatchings/code_179d8_i/Snd_play.s`, vram `0x8003410C`, unit
`code_179d8_i` (round 21, 2026-09-06).

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
