# SpuVmNoiseOnWithAdsr -- MATCHED (38/38 words)

> Renamed from `PlaySound` on 2026-09-23 (tools/rename.py). Address 0x8002f20c.

> Renamed from `func_8002F20C` on 2026-09-20 (tools/rename.py). Address 0x8002f20c.

Unit: `src/psyq/libsnd_vmanager.c`. Round 24, runner bravo.

## Result

Byte-exact. `./build-and-verify.sh` green (`build exit=0`, whole-image SHA1
matches). `funcdiff.py`: `38/38 words match (file 0x1FA0C-0x1FAA4)`, no
out-of-range drift.

## Final C

```c
extern u16 D_8008EA26;
extern u8 spuVmMaxVoice;
extern u8 D_8008EA1B;

extern s32 SpuVmAlloc(s32 a0);
extern void vmNoiseOn2(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4);

void SpuVmNoiseOnWithAdsr(s32 a0, s32 a1, s32 a2, s32 a3) {
    s32 v0;

    D_8008EA1B = 0x7F;
    v0 = SpuVmAlloc(0xFF) & 0xFF;
    D_8008EA26 = v0;
    if (v0 < spuVmMaxVoice) {
        vmNoiseOn2(*(u8 *)&D_8008EA26, a0 & 0xFFFF, a1 & 0xFFFF, a2 & 0xFFFF, a3 & 0xFFFF);
    }
}
```

## Shape

The 4-argument sibling of `SpuVmNoiseOn` (same report has the full
derivation of the mixed-width `D_8008EA26` access and the two wrong turns
that preceded the working form -- read that first). Same gate: force
`D_8008EA1B`, call `SpuVmAlloc(0xFF)`, mask, stash into `D_8008EA26`,
compare to `spuVmMaxVoice`, and conditionally forward into `vmNoiseOn2`.

The only structural difference from `SpuVmNoiseOn` is the callee's
argument SOURCE: where `SpuVmNoiseOn` passed two of its own narrowed
arguments plus two hardcoded constants (`0x80FF`, `0x5FC8`), this function
passes all four of ITS OWN arguments, each independently narrowed to
`u16`, as the callee's four non-channel arguments (`a0&0xFFFF`,
`a1&0xFFFF`, `a2&0xFFFF`, `a3&0xFFFF` -> callee's 2nd/3rd/4th/5th
positions). Confirms `vmNoiseOn2`'s signature derived from the sibling:
`(channel_byte, s32, s32, s32, s32)`.

Reused the `D_8008EA26`/`spuVmMaxVoice`/`D_8008EA1B`/`SpuVmAlloc`/
`vmNoiseOn2` declarations verbatim from `SpuVmNoiseOn` (moved up to
before `SpuVmNoiseOnWithAdsr`, the first user in ROM order -- these two functions
are adjacent modulo the frameless `SpuVmNoiseOff` between them). One build
snag along the way: declaring them only immediately above `SpuVmNoiseOn`
left `SpuVmNoiseOnWithAdsr` (earlier in file/ROM order) compiling against
undeclared identifiers -- `undeclared (first use this function)` on
`D_8008EA1B` et al. Fixed by hoisting the shared declarations to above the
first (ROM-order-earliest) user instead of duplicating them.

No new learning beyond `SpuVmNoiseOn`'s report; this one is corroborating
evidence for that report's `sh`-then-`lbu` idiom and for `vmNoiseOn2`'s
signature.

## Naming

**Superseded, round 71 (track 2):** the track-3 game name `PlaySound` is
replaced by Sony's own name -- fingerprint EXACT masked 1.00 vs
libsnd/vmanager `SpuVmNoiseOnWithAdsr` (discs 3.3/3.5; `libsnd/vm_noise` on
3.6). This is Sony's SDK code, not decompiled game logic; track 2 names
those functions and moves them out of tracks 1/1b/3.

**SpuVmNoiseOnWithAdsr** (was `func_8002F20C`) -- Tier B. Allocates a free voice via
`SpuVmAlloc` (libsnd_vmanager, "voice-steal candidate scan") and, if one
is available (`v0 < spuVmMaxVoice`, the voice count), keys it on via
`vmNoiseOn2` (libsnd_vmanager, confirmed to write the PS1 SPU's key-on
registers) forwarding all four caller-supplied parameters unchanged. The
generic name reflects that this is the "just play it" wrapper, in
contrast to SpuVmNoiseOn's hardcoded-parameter variant.

## History (moved from src/libsnd_vmanager.c, comments pass)

The arity-ok note on the SpuVmAlloc prototype above SpuVmNoiseOnWithAdsr read:

> arity-ok: the callee (still INCLUDE_ASM, 0x8002CF18) reads NO argument register, but this unit's argument is byte-load-bearing -- retail emits `li a0,0xff` in the delay slot at 0x8002F244

## History (moved from src/psyq/libsnd_vmanager.c, round 103)

The comments above SpuVmNoiseOnWithAdsr's local declarations of
D_8008EA26 and D_8008EA1B, before both moved to include/libsnd_internal.h:

> /* "Currently selected channel" scratch global: written as a side
>  * effect, then re-read from the global (not a cached register) a few
>  * instructions later; this is why the declaration at the top of the file
>  * is `volatile u16`.  Genuinely needs
>  * `volatile`: without it, this compiler proves (from the narrow range
>  * of the values stored here) that the re-read is redundant and elides
>  * it entirely, which retail's disassembly shows it does NOT do.
>  * `volatile` alone reproduces retail's separate store/reload exactly
>  * -- reading it back through `*(u8 *)&D_8008EA26` (a plain, NON-
>  * volatile-qualified pointer type) still folds to retail's compact
>  * `lui`+`lbu` two-instruction form; it is specifically a
>  * VOLATILE-QUALIFIED POINTER TYPE (`volatile u8 *`) that defeats the
>  * addressing fold, not the underlying object's volatility. */
> /* Flag byte forced on unconditionally at entry. */

And the note on D_8008EA26 at the top of libsnd_vmanager.c:

> +0x1A: the voice being keyed. volatile u16, the spelling SpuVmInit,
> SpuVmNoiseOn and SpuVmKeyOff need (the note above SpuVmNoiseOnWithAdsr);
> SpuVmKeyOnNow's NON_MATCHING body reads it as a plain s16 through a cast
> (SpuVmKeyOnNow.md: neither scalar nor volatile).
