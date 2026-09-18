> Renamed from `func_8002CC84` on 2026-09-18 (tools/rename.py). Address 0x8002cc84.

# FlushSoundCueSet -- MATCHED (33/33 words)

Unit: `code_179d8_e`. Runner: echo, round 17.

## Result

```c
void FlushSoundCueSet(ObjDA34 *self, ObjCC34 *obj) {
    s32 i;
    Slot179D8ECC34 *slot;

    slot = obj->arr;
    for (i = 0; i < 3; i++) {
        if (slot->unk0 >= 0) {
            slot->unk0 = self->methods->slot84(self, slot->unk0);
        }
        slot++;
    }
    obj->unk0 = 0;
}
```

Byte-exact, 33/33 words.

## Notes

`D_8006DA34`'s vtable slot +0x084 is NOT this function -- `FlushSoundCueSet`
itself is called directly (its only caller,
`asm/class_3bb8c_n.s:func_800557DC`, passes `(*D_8008AC7C, &param0->unk14)`
with no vtable indirection), while `FlushSoundCueSet`'s OWN body dispatches
THROUGH `self`'s vtable to slot `+0x084` (`VabStreamObj__StopVoice`, this unit,
matched separately). `obj` is the same `ObjCC34` struct
`InitSoundCueSet` (this unit) initializes -- confirmed by the field shapes
lining up exactly: `obj->arr` (3 `Slot179D8ECC34` entries, stride 0x14, each
checked/updated by field `unk0`) and `obj->unk0` being reset to `0` at the
end here, mirroring the `obj->unk0 == 0` init guard `InitSoundCueSet` reads.

**Struct-edit note (per CLAUDE.md's "struct edits are non-local"):** matching
this function surfaced a missing pad in `TableDA34`, introduced when
`VabStreamObj__StopVoice`'s slot was added earlier this round -- `slot84` was declared
immediately after `slot7C` with no gap, landing it at byte offset +0x080
instead of the correct +0x084 (there's an unnamed 4-byte slot at +0x080 that
nothing in this unit dispatches through). The bug was caught immediately:
first attempt scored 32/33 with the single differing word being
`lw v0,0x84(v0)` (retail) vs `lw v0,0x80(v0)` (mine) -- an off-by-one-slot
`TableDA34` offset, not a residue in this function's own logic. Fixed with
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
