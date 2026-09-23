# FlushSoundCueSet -- MATCHED (33/33 words)

> Renamed from `func_8002CC84` on 2026-09-18 (tools/rename.py). Address 0x8002cc84.

Unit: `code_179d8_e`. Runner: echo, round 17.

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
`asm/class_3bb8c_n.s:FlushStyleCue`, passes `(*gStyleTargetObj, &param0->unk14)`
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
