# StyleTeardown -- MATCHED, round 46 (2026-09-15)

> Renamed from `func_80054D30` on 2026-09-23 (tools/rename.py). Address 0x80054d30.

Unit `class_3bb8c_n`. **29/29 words, byte-exact.** First build.

## What it was

Fresh ground, carved round 45, never attempted. Already declared in the
shared header (`include/class_3bb8c.h:2557`, `extern void
StyleTeardown(void);`) and called with no arguments from
`src/class_3bb8c_l.c:173`, which fixed its signature before any decompiling
started here.

## Derivation

```
addiu $sp, $sp, -0x20
sw    $ra, 0x18($sp)
sw    $s1, 0x14($sp)
jal   StyleFlushDecoration
 sw   $s0, 0x10($sp)
jal   StyleReleaseDecorSet
 addu $s1, $zero, $zero
jal   StyleReleaseEffectSlots
lui   $s0, %hi(gStyleCueSlots)
addiu $s0, $s0, %lo(gStyleCueSlots)
.L80054D5C:
lw    $a0, 0x0($s0)
jal   FlushStyleCue
 addiu $s1, $s1, 0x1
sw    $v0, 0x0($s0)
slti  $v0, $s1, 0x2
bnez  $v0, .L80054D5C
 addiu $s0, $s0, 0x4
lw    $v0, %gp_rel(gStyleGrid)($gp)
beqz  $v0, .L80054D8C
sw    $zero, %gp_rel(gStyleGrid)($gp)
.L80054D8C:
...
jr $ra
```

Calls the three just-matched one-shot-flag helpers unconditionally, then
loops twice over `gStyleCueSlots[i]`, replacing each element with
`FlushStyleCue`'s return (`FlushStyleCue` always returns 0, so this clears
the two-slot array), then clears the `gStyleGrid` flag if set. Since
`FlushStyleCue` is defined later in this unit (higher ROM address) but
called here, it needs a forward declaration -- matching the pattern already
used for `ObjM__StartFadeUp` in `src/class_3bb8c_m.c`.

`gStyleCueSlots` holds two elements of a local per-unit type introduced here,
`ObjN14` (named for its two accessed fields: `unk0`, address-taken then
chased for a byte at `+0x6`; `unk14`, only ever address-taken and handed to
`FlushSoundCueSet`/`ServiceSoundCueSet` by `FlushStyleCue` and `ServiceStyleCueIfNear`
respectively -- see their own reports). `gStyleCueSlots` is `.sbss`
(`asm/data/7B46C.sbss.s`), adjacent to the other `D_8008ACxx` globals this
class family already uses.

```c
typedef struct ObjN14Sub ObjN14Sub;
struct ObjN14Sub {
    u8 pad0[0x6];
    s8 unk6; /* +0x006 */
};

typedef struct ObjN14 ObjN14;
struct ObjN14 {
    ObjN14Sub *unk0; /* +0x000 */
    u8 pad4[0x14 - 0x4];
    s32 unk14; /* +0x014 */
};

extern s32 FlushStyleCue(ObjN14 *arg0);

extern s32 gStyleGrid;
extern ObjN14 *gStyleCueSlots[2];

void StyleTeardown(void) {
    s32 i;

    StyleFlushDecoration();
    StyleReleaseDecorSet();
    StyleReleaseEffectSlots();
    for (i = 0; i < 2; i++) {
        gStyleCueSlots[i] = (ObjN14 *) FlushStyleCue(gStyleCueSlots[i]);
    }
    if (gStyleGrid != 0) {
        gStyleGrid = 0;
    }
}
```

An ordinary indexed `for` loop over `gStyleCueSlots[i]` compiled to retail's
pointer-increment loop (`$s0 += 4` each iteration) with no rewriting needed
-- GCC 2.6.3 -O2 does that strength reduction on its own here.

### Proposed learning

None beyond confirming the standing forward-declaration idiom
(`class_3bb8c_m.c`'s `ObjM__StartFadeUp` precedent) generalises cleanly to a
function defined in the SAME slice rather than the same file examined
before.

## Naming

**`StyleTeardown`, tier B.**

Calls `StyleFlushDecoration`, `StyleReleaseDecorSet`, `StyleReleaseEffectSlots`
unconditionally, then flushes both `gStyleCueSlots[2]` entries via
`FlushStyleCue`, then clears `gStyleGrid`. Called from
`src/class_3bb8c_l.c`'s `ObjM__TeardownStyle` (itself calling `self->methods->slot84`
and `ReleaseDreamAuxEntities()`, an end-of-scene-style teardown), which is the
evidence for "Teardown" over a narrower "Reset" -- it releases every
resource `TickStyle` builds, matching a scene-exit shape rather than a
per-frame reset. MATCHED, 29/29, first build.

## Round 93 polish (delta, track 7)

### Comments moved here from src/class_3bb8c_n.c

Verbatim as they stood before the round-93 comment pass (identifiers already carry this round's renames).

```c
/* Local view only: `FlushStyleCue` (defined later in this unit, in strict
 * ROM order) takes one of these two per-slot objects. `StyleCueEntryView`
 * is the SAME record `FindNextStyleCueInRange` returns as `EntrySlot *`
 * below -- a second independent local view of one struct, per the
 * multiple-independent-local-views convention, not merged with it: this
 * view only ever touches `cue` (+0x6, an `EntrySlot::count`-typed
 * byte, address-taken then chased and toggled), where `EntrySlot` names the
 * rest. `StyleCueSlot::entry` is that claimed record, released by
 * `FlushStyleCue`/`ServiceStyleCueIfNear`. `posX`/`posZ` are the slot's own 2D
 * (X/Z) position, read by `IsStyleCueNear`'s distance check; `lastDist` is
 * that check's own last-computed distance (also the out-parameter
 * `FindNextStyleCueInRange` writes). `cueSet` is only ever address-taken,
 * as an embedded sub-object handed to `FlushSoundCueSet`/`ServiceSoundCueSet`
 * (same discard-return caveat as `include/Entity.h`'s `unk9C` -- a field
 * only ever address-taken carries no evidence about its own declared
 * type). */
```
