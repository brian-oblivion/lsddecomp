# PlaySound -- MATCHED (38/38 words)

> Renamed from `func_8002F20C` on 2026-09-20 (tools/rename.py). Address 0x8002f20c.

Unit: `src/code_179d8_m.c`. Round 24, runner bravo.

## Result

Byte-exact. `./build-and-verify.sh` green (`build exit=0`, whole-image SHA1
matches). `funcdiff.py`: `38/38 words match (file 0x1FA0C-0x1FAA4)`, no
out-of-range drift.

## Final C

```c
extern u16 D_8008EA26;
extern u8 D_8008E9D0;
extern u8 D_8008EA1B;

extern s32 func_8002CF18(s32 a0);
extern void func_8002DDBC(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4);

void PlaySound(s32 a0, s32 a1, s32 a2, s32 a3) {
    s32 v0;

    D_8008EA1B = 0x7F;
    v0 = func_8002CF18(0xFF) & 0xFF;
    D_8008EA26 = v0;
    if (v0 < D_8008E9D0) {
        func_8002DDBC(*(u8 *)&D_8008EA26, a0 & 0xFFFF, a1 & 0xFFFF, a2 & 0xFFFF, a3 & 0xFFFF);
    }
}
```

## Shape

The 4-argument sibling of `PlayFixedSound` (same report has the full
derivation of the mixed-width `D_8008EA26` access and the two wrong turns
that preceded the working form -- read that first). Same gate: force
`D_8008EA1B`, call `func_8002CF18(0xFF)`, mask, stash into `D_8008EA26`,
compare to `D_8008E9D0`, and conditionally forward into `func_8002DDBC`.

The only structural difference from `PlayFixedSound` is the callee's
argument SOURCE: where `PlayFixedSound` passed two of its own narrowed
arguments plus two hardcoded constants (`0x80FF`, `0x5FC8`), this function
passes all four of ITS OWN arguments, each independently narrowed to
`u16`, as the callee's four non-channel arguments (`a0&0xFFFF`,
`a1&0xFFFF`, `a2&0xFFFF`, `a3&0xFFFF` -> callee's 2nd/3rd/4th/5th
positions). Confirms `func_8002DDBC`'s signature derived from the sibling:
`(channel_byte, s32, s32, s32, s32)`.

Reused the `D_8008EA26`/`D_8008E9D0`/`D_8008EA1B`/`func_8002CF18`/
`func_8002DDBC` declarations verbatim from `PlayFixedSound` (moved up to
before `PlaySound`, the first user in ROM order -- these two functions
are adjacent modulo the frameless `ClearNoiseVoices` between them). One build
snag along the way: declaring them only immediately above `PlayFixedSound`
left `PlaySound` (earlier in file/ROM order) compiling against
undeclared identifiers -- `undeclared (first use this function)` on
`D_8008EA1B` et al. Fixed by hoisting the shared declarations to above the
first (ROM-order-earliest) user instead of duplicating them.

No new learning beyond `PlayFixedSound`'s report; this one is corroborating
evidence for that report's `sh`-then-`lbu` idiom and for `func_8002DDBC`'s
signature.

## Naming

**PlaySound** (was `func_8002F20C`) -- Tier B. Allocates a free voice via
`func_8002CF18` (code_179d8_l, "voice-steal candidate scan") and, if one
is available (`v0 < D_8008E9D0`, the voice count), keys it on via
`func_8002DDBC` (code_179d8_l, confirmed to write the PS1 SPU's key-on
registers) forwarding all four caller-supplied parameters unchanged. The
generic name reflects that this is the "just play it" wrapper, in
contrast to PlayFixedSound's hardcoded-parameter variant.
