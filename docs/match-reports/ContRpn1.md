# ContRpn1 -- MATCHED (31/31 words)

> Renamed from `func_800350D8` on 2026-09-23 (tools/rename.py). Address 0x800350d8.

`asm/nonmatchings/libsnd_seqread/ContRpn1.s`, vram `0x800350D8`, unit
`libsnd_seqread`. Round 24, runner alpha. Sibling of `SetProgramChange` and
`ContRpn2` -- same shape, different byte-field written and a
retrigger counter incremented instead.

```c
void ContRpn1(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &_ss_score[a0][a1];
    u8 counter = rec->unk29;

    rec->unk13 = a2;
    counter = counter + 1;
    rec->unk29 = counter;
    rec->unk88 = ReadDeltaValue(a0, a1);
}
```

`rec->unk29` is a retrigger/step counter: read, incremented, and stored
back, with the caller's byte argument written to a DIFFERENT fixed field
(`unk13`) in between -- retail's own instruction order reads the counter,
stores the new byte, THEN increments and stores the counter back, and the
C above reproduces that exactly with no reordering needed.

## The one lever that mattered

Same as `SetProgramChange`: collapsing `_ss_score[a0]` + `&tbl[a1]` into one
`&_ss_score[a0][a1]` expression fixed a prologue register-order mismatch
(retail keeps the slot-multiply result in `$s0` from early on; a named
`tbl` temp forced the table-pointer fetch to be evaluated first instead).
See `SetProgramChange.md` for the full writeup; not repeated here.

## Verification

`./build-and-verify.sh` build exit=2 (clean compile; other unit functions
still queued). `funcdiff.py ContRpn1`: 31/31 words match, no drift.

## Round 97 types pass (echo)

libsnd_seqread's local `Entry90902E8` view retired onto `include/SsScore.h`:
the same 0xAC-byte (`SS_SEQ_TABSIZ`) `_ss_score[access][seq]` record that
libsnd_cres, libsnd_decre and code_179d8_j already use. The header gained
this unit's fields by splitting padding (no offset, size or existing type
moved); field names stay offset-only (`unkNN`) as the header's convention for
Sony-only fields, with each one's mechanics in its comment. The unit's
`(u8 *)rec + unk12 + 0x17/0x2C` and `(s16 *)((u8 *)rec + 0x4E + ch * 2)`
arithmetic became the header's per-channel arrays `unk17[16]` (pan),
`unk2C[16]` (program) and `unk4E[16]` (volume): `unk12` is the event's MIDI
channel (GetSeqData stores a status byte's low nibble), not a byte offset to
an "embedded state block" as the old local comment read it. Byte-exact
unchanged; the NON_MATCHING object is identical too (objdump of
`build/nonmatching/src/libsnd_seqread.c.o` before/after).
