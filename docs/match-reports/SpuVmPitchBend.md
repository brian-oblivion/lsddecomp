# SpuVmPitchBend -- MATCHED (60/60 words)

> Renamed from `ApplyPitchBendToAllVoices` on 2026-09-23 (tools/rename.py). Address 0x8002f610.

> Renamed from `func_8002F610` on 2026-09-20 (tools/rename.py). Address 0x8002f610.

Unit: `src/libsnd_vmanager.c`. Round 24, runner bravo.

## Result

Byte-exact, first attempt. `./build-and-verify.sh` green (`build exit=0`,
whole-image SHA1 matches). `funcdiff.py`: `60/60 words match (file
0x1FE10-0x1FF00)`, no out-of-range drift.

## Final C

```c
extern u16 D_8008EA22;

extern s32 func_80032148(s16 a0, s16 a1);
extern s16 SpuVmPBVoice(s16 a0, s16 a1, s16 a2, s16 a3, u16 a4);

s32 SpuVmPitchBend(s16 a0, s16 a1, s16 a2, u16 a3) {
    s16 i;
    s32 sum;

    func_80032148(a1, a2);
    D_8008EA22 = a0;
    sum = 0;
    for (i = 0; i < spuVmMaxVoice; i++) {
        sum += SpuVmPBVoice(i, a0, a1, a2, a3);
    }
    return sum;
}
```

## Shape

Set up (call `func_80032148(a1, a2)` for its side effect, return value
discarded; stash `a0` into the "currently selected channel" scratch global
`D_8008EA22`, same idiom as `SpuVmNoiseOn`'s `D_8008EA26` but write-only
here), then sum `SpuVmPBVoice(i, a0, a1, a2, a3)` over
`i = 0 .. spuVmMaxVoice-1`, returning the accumulated total.

## CORRECTION (round 24, second pass): `SpuVmPBVoice`'s 2nd parameter is `s16`, not `s32`

While attempting `SpuVmPBVoice` itself this round (stalled at 77/138, see
its own report), the reasoning below -- that this function's per-iteration
re-narrowing of `a0` PROVES the callee's second parameter is wider (`s32`)
than its neighbours -- turned out to conflate two different things. The
MIPS o32 ABI represents an `s16` argument as a properly sign-extended
32-bit value in its register REGARDLESS of whether the callee's own
parameter is declared `s16` or `s32`; the caller-side widening instructions
this report originally pointed to are consistent with EITHER declared
width at the callee, so they cannot by themselves distinguish "callee
parameter is `s32`" from "callee parameter is `s16`, and this compiler
generation just re-derives narrow locals at every use regardless of
whether the register is already correctly formed" (the SAME lever this
report already invokes for why the re-narrowing happens every loop
iteration in the first place). `SpuVmPBVoice`'s own body only ever uses
its second parameter ONCE, sign-extended right before a single comparison
-- the same one-shot shape every OTHER `s16` parameter in that function
has, with no second, wider-range use anywhere to justify `s32`. Retyping it
`s16` (and updating this function's own `extern` declaration to match) was
re-verified: `SpuVmPitchBend` still matches 60/60 with zero drift, byte for
byte identical to before the retype. The prototype above reflects the
corrected typing; the walkthrough below is kept for its still-valid part
(that the caller's OWN parameter types, not the callee's declared width
alone, are what the narrowing instructions are conversions between) but its
central claim -- "the callee's 2nd parameter must be wider" -- did not
survive independent verification against the callee's own body.

## Deriving the parameter widths from `libsnd_vmanager.c`'s cross-unit guesses

`libsnd_vmanager.c` already carries prototype GUESSES for both callees, typed
per-call-site from that unit's own register setup (its own comment: "none
of these callees have an established prototype yet, so these are local
guesses"):

```c
extern s32 func_80032148(s16 a0, s16 a1);
extern s16 SpuVmPBVoice(s16 a0, s32 a1, s16 a2, s16 a3, u16 a4);
```

Read cold, `SpuVmPBVoice`'s second parameter being `s32` while its
neighbours are `s16`/`s16`/`u16` looks suspicious -- but it is exactly what
this function's own disassembly demands, and it resolved a question this
report would otherwise have had to answer from scratch: **which of this
function's four parameters get sign-extended every loop iteration, and
why.**

Retail re-derives THREE of this function's four parameters from scratch on
every iteration, each via its own `sll #16`/`sra #16` pair even though none
of them change inside the loop:

- The loop counter `i` (naturally `s16`, matching `SpuVmNoiseOff`'s
  already-documented idiom of a local needing re-sign-extension at every
  use).
- `a0` (this function's own first parameter) -- re-narrowed EVERY
  iteration purely to satisfy `SpuVmPBVoice`'s SECOND parameter being
  `s32`: passing a genuinely-`s16`-declared `a0` to a wider `s32` parameter
  is an implicit sign-extending conversion, done fresh at each call site
  rather than cached, because this compiler generation does not preserve a
  "this register is already correctly sign-extended" invariant across
  loop iterations for a narrow-typed local.
- `a1` (this function's second parameter) -- same story, passed to
  `SpuVmPBVoice`'s third (`s16`) parameter.
- `a2` (third parameter) -- same, fourth (`s16`) parameter.

`a3` (fourth parameter) is the odd one out: it is zero-extended (`andi
0xffff`), never sign-extended, matching a `u16`-declared parameter passed
to `SpuVmPBVoice`'s fifth (`u16`, stack-passed) argument.

Without `libsnd_vmanager.c`'s prototype already on record, the natural first
guess would have been "all four call arguments are `s32`" (no narrowing
needed at the call, contradicting the disassembly) or "the callee's second
parameter is `s16` like its siblings" (which would make `a0`'s repeated
narrowing pointless from the callee's perspective, since s16-to-s16 needs
no widening) -- either wrong guess would have cost at least one attempt
reading the diff to notice the real shape. Matched first try by trusting
the cross-unit guess and typing THIS function's own parameters to be
consistent with it (`s16 a0, s16 a1, s16 a2, u16 a3`).

### Proposed learning

**When a cross-unit callee already has a documented prototype (even one
explicitly marked "a local guess, not authoritative"), type the CALLER's
own parameters to make the guessed prototype's implicit conversions
produce the exact narrowing instructions the disassembly shows**, rather
than re-deriving the callee's signature independently. A guessed prototype
with an asymmetric width (one parameter wider than its neighbours) is
itself a clue about the CALLER's own parameter types, not just the
callee's -- the caller-side narrowing instructions are the conversion
FROM the caller's declared type TO the callee's declared type, so an
odd-one-out parameter width on the callee side often means an ordinary
(non-odd) type on the caller side, and vice versa.

## Naming

**Superseded, round 71 (track 2):** the track-3 game name
`ApplyPitchBendToAllVoices` is replaced by Sony's own name -- fingerprint
EXACT masked 1.00 vs libsnd/vmanager `SpuVmPitchBend` (discs 3.3/3.5;
`libsnd/vm_pb` on 3.6). This is Sony's SDK code, not decompiled game logic;
track 2 names those functions and moves them out of tracks 1/1b/3.

**SpuVmPitchBend** (was `func_8002F610`) -- Tier B. Calls
Sony's `SpuVmVSetUp` once, then runs `SpuVmPBVoice` over every
voice (`0..spuVmMaxVoice`) with the same identity/depth arguments, summing
its 0/1 return into a count. The uncertain part is `SpuVmVSetUp`'s own
role (an SDK function, not renamed here) -- named for what THIS function
visibly does (batch-apply a bend, report how many voices it affected),
not for what the SDK call underneath it is for.
