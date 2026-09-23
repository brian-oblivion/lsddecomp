# ContRpn2 -- MATCHED (31/31 words)

> Renamed from `func_80035154` on 2026-09-23 (tools/rename.py). Address 0x80035154.

`asm/nonmatchings/code_179d8_k/ContRpn2.s`, vram `0x80035154`, unit
`code_179d8_k`. Round 24, runner alpha. Second sibling of `SetProgramChange`
and `ContRpn1` -- identical shape to `ContRpn1`, writes
`unk14` instead of `unk13`.

```c
void ContRpn2(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 counter = rec->unk29;

    rec->unk14 = a2;
    counter = counter + 1;
    rec->unk29 = counter;
    rec->unk88 = ReadDeltaValue(a0, a1);
}
```

## The one lever that mattered

Same as `SetProgramChange`/`ContRpn1`: the `&D_800902E8[a0][a1]`
single-expression lookup, not a named `tbl` temp. See `SetProgramChange.md`
for the full writeup.

## Verification

`./build-and-verify.sh` build exit=2 (clean compile; other unit functions
still queued). `funcdiff.py ContRpn2`: 31/31 words match, no drift.
