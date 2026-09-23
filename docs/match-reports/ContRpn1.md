# ContRpn1 -- MATCHED (31/31 words)

> Renamed from `func_800350D8` on 2026-09-23 (tools/rename.py). Address 0x800350d8.

`asm/nonmatchings/code_179d8_k/ContRpn1.s`, vram `0x800350D8`, unit
`code_179d8_k`. Round 24, runner alpha. Sibling of `SetProgramChange` and
`ContRpn2` -- same shape, different byte-field written and a
retrigger counter incremented instead.

```c
void ContRpn1(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
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

Same as `SetProgramChange`: collapsing `D_800902E8[a0]` + `&tbl[a1]` into one
`&D_800902E8[a0][a1]` expression fixed a prologue register-order mismatch
(retail keeps the slot-multiply result in `$s0` from early on; a named
`tbl` temp forced the table-pointer fetch to be evaluated first instead).
See `SetProgramChange.md` for the full writeup; not repeated here.

## Verification

`./build-and-verify.sh` build exit=2 (clean compile; other unit functions
still queued). `funcdiff.py ContRpn1`: 31/31 words match, no drift.
