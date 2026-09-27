# ContRpn2 -- MATCHED (31/31 words)

> Renamed from `func_80035154` on 2026-09-23 (tools/rename.py). Address 0x80035154.

`asm/nonmatchings/code_179d8_k/ContRpn2.s`, vram `0x80035154`, unit
`code_179d8_k`. Round 24, runner alpha. Second sibling of `SetProgramChange`
and `ContRpn1` -- identical shape to `ContRpn1`, writes
`unk14` instead of `unk13`.

```c
void ContRpn2(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &_ss_score[a0][a1];
    u8 counter = rec->unk29;

    rec->unk14 = a2;
    counter = counter + 1;
    rec->unk29 = counter;
    rec->unk88 = ReadDeltaValue(a0, a1);
}
```

## The one lever that mattered

Same as `SetProgramChange`/`ContRpn1`: the `&_ss_score[a0][a1]`
single-expression lookup, not a named `tbl` temp. See `SetProgramChange.md`
for the full writeup.

## Verification

`./build-and-verify.sh` build exit=2 (clean compile; other unit functions
still queued). `funcdiff.py ContRpn2`: 31/31 words match, no drift.

## Round 97 types pass (echo)

code_179d8_k's local `Entry90902E8` view retired onto `include/SsScore.h`:
the same 0xAC-byte (`SS_SEQ_TABSIZ`) `_ss_score[access][seq]` record that
libsnd_cres, code_179d8_i and code_179d8_j already use. The header gained
this unit's fields by splitting padding (no offset, size or existing type
moved); field names stay offset-only (`unkNN`) as the header's convention for
Sony-only fields, with each one's mechanics in its comment. The unit's
`(u8 *)rec + unk12 + 0x17/0x2C` and `(s16 *)((u8 *)rec + 0x4E + ch * 2)`
arithmetic became the header's per-channel arrays `unk17[16]` (pan),
`unk2C[16]` (program) and `unk4E[16]` (volume): `unk12` is the event's MIDI
channel (GetSeqData stores a status byte's low nibble), not a byte offset to
an "embedded state block" as the old local comment read it. Byte-exact
unchanged; the NON_MATCHING object is identical too (objdump of
`build/nonmatching/src/code_179d8_k.c.o` before/after).
