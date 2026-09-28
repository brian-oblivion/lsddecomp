# FlushSoundCueSet -- MATCHED (33/33 words)

> Renamed from `func_8002CC84` on 2026-09-18 (tools/rename.py). Address 0x8002cc84.

Unit: `PlacementGridVabSound`. Runner: echo, round 17.

## Result

```c
void FlushSoundCueSet(VabStreamObj *self, SoundCueSet *set) {
    s32 i;
    SoundCueSlot *slot;

    slot = set->slots;
    for (i = 0; i < 3; i++) {
        if (slot->index >= 0) {
            slot->index = self->methods->slot84(self, slot->index);
        }
        slot++;
    }
    set->tag = 0;
}
```

(Types/fields updated to round 52's renames -- `ObjDA34`/`ObjCC34`/
`Slot179D8ECC34` are now `VabStreamObj`/`SoundCueSet`/`SoundCueSlot`;
`obj`/`obj->arr`/`slot->unk0`/`obj->unk0` are now `set`/`set->slots`/
`slot->index`/`set->tag`. Bytes unchanged.)

Byte-exact, 33/33 words.

## Notes

`gVabStreamObjMethods`'s vtable slot +0x084 is NOT this function -- `FlushSoundCueSet`
itself is called directly (its only caller,
`asm/ObjMStyleActor.s:FlushStyleCue`, passes `(*sStyleSceneRefs, &param0->unk14)`
with no vtable indirection), while `FlushSoundCueSet`'s OWN body dispatches
THROUGH `self`'s vtable to slot `+0x084` (`VabStreamObj__StopVoice`, this unit,
matched separately). `set` is the same `SoundCueSet` struct
`InitSoundCueSet` (this unit) initializes -- confirmed by the field shapes
lining up exactly: `set->slots` (3 `SoundCueSlot` entries, stride 0x14, each
checked/updated by field `index`) and `set->tag` being reset to `0` at the
end here, mirroring the `set->tag == 0` init guard `InitSoundCueSet` reads.

**Struct-edit note (per CLAUDE.md's "struct edits are non-local"):** matching
this function surfaced a missing pad in `VabStreamObjMethods` (`TableDA34`
at the time), introduced when
`VabStreamObj__StopVoice`'s slot was added earlier this round -- `slot84` was declared
immediately after `slot7C` with no gap, landing it at byte offset +0x080
instead of the correct +0x084 (there's an unnamed 4-byte slot at +0x080 that
nothing in this unit dispatches through -- this is `VabStreamObj__PlayTone`'s
own slot, established only later once that function was read; not relevant
at the time this bug was fixed). The bug was caught immediately:
first attempt scored 32/33 with the single differing word being
`lw v0,0x84(v0)` (retail) vs `lw v0,0x80(v0)` (mine) -- an off-by-one-slot
table offset, not a residue in this function's own logic. Fixed with
an explicit `u8 pad080[0x084 - 0x080];`, then reran the FULL
`./build-and-verify.sh` (not just this function's `funcdiff`) to confirm
the whole-image SHA1 was still green and that `VabStreamObj__OnBodyReady` and
`VabStreamObj__StopVoice` (both already matched, both readers of this same struct)
were unaffected -- both still score full matches after the fix.

### Proposed learning

A vtable-slot function-pointer struct built up incrementally across several
matched functions (one slot named per match, in slot order) is exactly the
shape CLAUDE.md's struct-edit warning is about, even though each individual
edit only ADDS a new named slot rather than retyping an existing one: get
the gap between two adjacent named slots wrong by the size of one pointer
and the SECOND slot lands on the wrong table entry silently at the C level
(it still compiles, it just calls through the wrong function pointer at
runtime) and shows up ONLY as a one-word offset residue in whichever
function first reads that later slot. Re-run the full oracle, not just the
one function's `funcdiff`, after adding any new slot to a table struct
that already has other slots after it.

## Naming

Renamed `func_8002CC84` -> `FlushSoundCueSet`, tier B. Confirmed NOT a
`gVabStreamObjMethods` vtable slot (its only caller is a direct `jal`, no
vtable indirection); it's the counterpart to `InitSoundCueSet` on the same
`SoundCueSet` struct -- dispatches every populated slot's stored index
through the ACTIVE `VabStreamObj`'s own `StopVoice` slot and clears the
set's guard/tag back to `0`, i.e. "drain the queue and mark it empty
again." "Flush" over "Clear"/"Reset" because the per-slot dispatch through
`StopVoice` is the whole point, not just a reset -- but tier B rather than
A since the in-game reason a caller would flush (rather than let entries
sit) isn't established from this unit alone.

## Track 6 (2026-09-26, round 92, alpha): one SoundCueSet

`include/SoundCueSet.h` now holds the one definition of `SoundCueSet` and
`SoundCueSlot`. It replaced three views: PlacementGridVabSound.c's (named
`tag`/`owner`/`callback`/`slots[].index` only), libsnd_vmanager.c's (named
`note`/`pitchOffset`/`word2`/`word3`, `unk4`/`unk10`/`unk14`) and
include/entity.h's `EntityMoodHandlerArg` (all `unkNN`). Zero bytes; the
whole-image SHA1 is unchanged.

Layout verified against every reader: InitSoundCueSet (+0x00 tag, +0x04,
+0x08 owner, +0x0C callback, +0x14 = 10, three 0x14-byte slots from +0x18
whose +0x0 gets -1), ServiceSoundCueSet (per-slot +0x4/+0x8/+0xC/+0x10
reset to -1/0/0x7F/0x40, +0x10 zeroed, callback(owner, set), +0x04
incremented), FlushSoundCueSet (slot +0x0 through stopVoice, +0x00
cleared), Entity__GetProximityRatio (+0x14 divisor), the Entity__MoodCueNN
handlers (+0x04, +0x10, slot 0 +0x4..+0x10, slot 1/2 +0x4/+0x8),
ObjMStyleActor's StyleCueNN `self` (the same offsets) and dream_sys.h's
`SoundCueCallbackArg` (+0x00 == tag 1, +0x04 % 20, slot 0/1 +0x4/+0x8).

Names, tier A, each from what its readers do:

| old (e / l / entity.h) | new | evidence |
| --- | --- | --- |
| slot `index` / `index` / - | `voice` | ServiceSoundCueSet stores playTone's result there (the voice, or -1) and passes it to stopVoice(voice); Flush stops it |
| - / `note` / `unk1C` `unk30` `unk44` | `program` | ServiceSoundCueSet passes `program * 16` as playTone's `index`, which PlayTone splits into program `index >> 4` and tone `index & 0xF` (so tone 0); -1 none, -2 stops the voice |
| - / `pitchOffset` / `unk20` `unk34` `unk48` | `octave` | forwarded unchanged to setPitchOffset, whose parameter is `octave` (pitchOffset = octave * 12 - 24) |
| - / `word2` / `unk24` | `vol` | playTone's `vol` argument after attenuation; default 0x7F |
| - / `word3` / `unk28` | `endVol` | playTone's `endVol` argument after attenuation; default 0x40 |
| `unk4` / `unk4` / `unk4` | `tick` | zeroed by Init, incremented once per service pass; handlers time requests on `tick % N` and `tick == 0`, and reset it with -1 |
| - / `unk10` / `unk10` | `attenuation` | zeroed per tick, then each volume loses (vol / attenuationSteps) per unit; < 0 skips keying; handlers store a proximity ratio in 0..10 |
| `unk14` / `unk14` / `unk14` | `attenuationSteps` | set to 10 by Init; the divisor above, and the scale both proximity helpers map a distance onto |

Why not `note`/`pitchOffset` (libsnd_vmanager.c) or the earlier proposal's
`voiceNTone`/`voiceNPitch` (Entity__MoodCue07.md): the value is neither a
note nor a tone. VabStreamObj__PlayTone's `index` is program << 4 | tone,
and ServiceSoundCueSet always sends tone 0, so what the callback writes is a
VAB program number. The pitch word is the octave setPitchOffset takes, not
a pitch offset (that is what setPitchOffset computes from it). `tick` and
`attenuation` are the earlier proposal's names, kept.

`callback` is typed `SoundCueCallbackFn`, `void (*)(void *owner,
SoundCueSet *set)`; InitSoundCueSet's parameter takes that type and its
first parameter is `sound` (it is unused). The three functions have no
shared prototype: entity.h, dream_sys.c and ObjMStyleActor.c declare them
with their own type for the sound object (TodActor's `arg2` is a
`struct UnkArg2Obj *`), and a header prototype taking `VabStreamObj *`
would warn in each.

## Round 98 (charlie, track 7)

The loop bound is `ARRAY_COUNT(set->slots)` (was 3). Byte-exact.
