# ComputeStyleCueFalloff -- MATCHED (36/36 words)

> Renamed from `func_8005627C` on 2026-09-23 (tools/rename.py). Address 0x8005627c.

Unit: `dream_scene` (round 17 continuation). The shared helper every
`sStyleCueCallbacks` slot occupant calls first: reads a small signed tag byte off
`ctx->methods`, looks it up with a NEGATIVE index into a 15-entry global
table, and returns a chained division result.

## Final source

```c
typedef struct StyleCueParam StyleCueParam;
typedef struct StyleCueParamMethods {
    u8 pad0[0x6];
    s8 tag; /* +0x006 */
} StyleCueParamMethods;
struct StyleCueParam {
    StyleCueParamMethods *methods; /* +0x000 */
    s32 kind;
    u8 pad8[0x8];
    s32 falloff;
    u8 pad14[0x8];
    s32 unk1C, unk20, unk24, unk28, unk2C, unk30, unk34, unk38, unk3C;
    u8 pad40[0x4];
    s32 unk44, unk48, unk4C, unk50;
};

extern s32 sStyleCueDistanceTable[];

s32 ComputeStyleCueFalloff(StyleCueParam *ctx) {
    s32 t = sStyleCueDistanceTable[-ctx->methods->tag];
    s32 q = t / ctx->unk28;

    return ctx->falloff / q;
}
```

## Derivation

- **Placement.** This function's own ROM address, `0x8005627C`, is AFTER
  all 14 `sStyleCueCallbacks` occupants that call it (they run `0x80055A88` ..
  `0x80056238`; this function sits right before the blocked
  `IsStyleVariantEven`). Its definition lives at that later position in the
  file, with a forward declaration (`s32 ComputeStyleCueFalloff(StyleCueParam *ctx);`)
  near the top where `StyleCueParam` is defined -- getting this wrong the first
  time (defining it inline with the struct, ahead of the 14 callers)
  miscompiled the whole file: every one of the 14 callers' own bodies
  still matched byte-for-byte, but every `jal ComputeStyleCueFalloff` in them
  encoded the WRONG target address, and `funcdiff`'s per-function windows
  read as near-total garbage until the ordering was fixed. Round 17's own
  hard rule about strict ROM-address order is not decorative.
- **`sStyleCueDistanceTable[-tag]`, not a hand-transcribed `base - tag*4`.** Retail
  computes `&sStyleCueDistanceTable - tag*4` via `sll`/`subu` (tag is small and
  negative, e.g. -1..-14, landing INSIDE the 15-word table declared at
  `sStyleCueDistanceTable` itself, confirmed directly against
  `asm/data/76DC8.data.s`). Writing the array-index form with a negated
  index lets GCC regenerate the identical `sll`/`subu` address
  computation, rather than transcribing the lowering by hand.
- **Two chained signed divisions**, each compiling to the ordinary `div`
  + zero/overflow `break` guard sequence this pinned toolchain always
  emits for `/`RESULT -- no special handling needed, `t / ctx->unk28` and
  `ctx->falloff / q` reproduce both guard blocks directly.
- `ctx` (the "a0" parameter every one of the 14 callers forwards
  unchanged) is the SAME `StyleCueParam` type used for the "self"/target
  parameter in every leaf -- see those reports and the file banner for why
  a single type was used for both roles.

### Proposed learning

- **Getting the position of a function that OTHER functions call before
  it wrong (in terms of ROM order, not in terms of C declaration order)
  makes the callers' OWN scores UNREADABLE, not just the misplaced
  function's.** All 14 legitimate call sites showed as near-total
  mismatches purely from an encoded absolute jump target, even though
  their own bodies were already byte-correct -- diagnosing this required
  recognizing that funcdiff's window read garbage while asm-differ's
  address-anchored view showed the SAME instructions at SHIFTED addresses,
  the signature of pure ordering drift rather than a real body bug.

## Naming

**Tier B.** Free function (called with `ctx`, not a `self` of its own type in the class-method sense), so no `Class__` prefix. The name reflects the mechanism, not the game meaning: `sStyleCueDistanceTable` (the same table `IsStyleCueNear`/`FindNextStyleCueInRange`, code_8220_b.c/dream_scene.c, index with `dist < table[...]` -- confirming it holds distance thresholds) is read at a NEGATIVE tag index, divided down, and used as the divisor for `ctx->falloff`'s own value; `falloff` is used by every `StyleCueNN` occupant that calls this helper first. What the resulting scaled quantity represents in the running game (a cue repeat count? a duration?) is not established.

## Track 6 (2026-09-26, round 92, alpha): `set` is a SoundCueSet

dream_scene.c's `StyleCueParam` used to type both parameters of every
StyleCueNN callback and of ComputeStyleCueFalloff; its old comment called it
"very likely a SoundCueSet-shaped object" but kept one local type because
nothing confirmed `self` and `ctx` were the same object. They are not, and
the two now have different types:

- The second parameter (was `self`, now `set`) is the SoundCueSet
  (include/sound_cue_set.h). ServiceSoundCueSet calls `callback(owner, set)`,
  and every offset the callbacks write agrees: +0x04 (was `kind`, the value
  every callback dispatches on) is `tick`, +0x10 (was `falloff`) is
  `attenuation`, and +0x1C..+0x50 (was `unk1C`..`unk50`) are
  `slots[0..2].program/octave/vol/endVol`. The callbacks' `-1` store to
  +0x04 is the same restart the Entity__MoodCueNN handlers do.
- The first parameter (`ctx`) is the owner, dream_scene.c's
  `StyleCueSlot`: TryStartStyleCue passes the slot as InitSoundCueSet's
  owner and `&slot->cueSet` (+0x14) as the set. So `StyleCueParam` is now a
  local view of StyleCueSlot: `methods` (+0x00) is the claimed record,
  renamed `entry`, whose +0x06 `tag` is dream_scene's `countSign`;
  `falloff` (+0x10) is `lastDist`; and `unk28` (+0x28) is
  `cueSet.attenuationSteps` (+0x14 + 0x14). ComputeStyleCueFalloff therefore
  scales the slot's last distance into 0..attenuationSteps against the cue's
  distance limit, as Entity__GetProximityRatio does for Entity.

Locals `kind` became `tick`. Zero bytes.

## Track 7 (2026-09-27, round 96, charlie)

- Locals `t` / `q` are now `range` / `stepDist`: `range` is the cue's
  `sStyleCueDistanceTable` row, the same threshold `IsStyleCueNear` and
  `FindNextStyleCueInRange` compare `dist < table[...]` against, and
  `stepDist` is that range split into `attenuationSteps` (10) steps, so the
  result is the target's distance counted in steps: 0 at the slot, 10 at the
  edge of the range (sound_cue_set.h: attenuation 10 leaves only `vol % 10`).
  Zero bytes (a local's name is not in the object).

Two comments in `src/world/dream_scene.c` lost their history (it is in the
Derivation above) and now read as documentation. What they said, verbatim:

```c
/* ComputeStyleCueFalloff's own ROM address (0x8005627C) is AFTER all 14 slot
 * occupants below (it sits right before the blocked IsStyleVariantEven), so
 * its definition lives in that position further down this file to keep
 * strict ROM-address order -- forward-declared here since every occupant
 * calls it. */

/* A 15-entry table indexed with the NEGATIVE of `ctx->entry->countSign`
 * (`sStyleCueDistanceTable - tag*4`, i.e. `sStyleCueDistanceTable[-tag]` for `tag` in [-14, 0]).
 * `asm/data/76DC8.data.s` confirms exactly 15 words at this address. */
```

The unit-local record view's +0x006 field is `cue`, not `countSign`: the
name dream_scene.c's own view (`StyleCueEntryView::cue`) already gives the
same byte, which IsStyleCueNear reads as `sStyleCueDistanceTable[-cue]`
exactly as this function does. It is a cue index, not a count. Zero bytes.
