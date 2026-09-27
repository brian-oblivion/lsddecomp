# SetProgramChange -- MATCHED (31/31 words)

> Renamed from `func_80034614` on 2026-09-23 (tools/rename.py). Address 0x80034614.

`asm/nonmatchings/libsnd_seqread/SetProgramChange.s`, vram `0x80034614`, unit
`libsnd_seqread`. Round 24, runner alpha.

## What it is

A per-channel/slot sequencer-voice setter. `_ss_score` is an array of
pointers to arrays of `Entry90902E8` (a 172/0xAC-byte record), indexed
`[channel][slot]`. `rec->unk12` is a byte offset (already scaled, not an
index) to a currently-active embedded state block; `rec + rec->unk12`
gives a pointer whose `+0x2C` byte is written here, and the shared helper
`ReadDeltaValue` (this unit's frameless leaf; decodes one VLQ-encoded delta
from the voice's event stream and adds it to `rec->unk80`) is called for
its side effect, with its return value cached into `rec->unk88` -- an
overloaded scratch slot every function in this unit's family stores its
own last-computed value into.

```c
void SetProgramChange(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &_ss_score[a0][a1];
    u8 *p = (u8 *)rec + rec->unk12;

    p[0x2C] = a2;
    rec->unk88 = ReadDeltaValue(a0, a1);
}
```

## The one lever that mattered

The FIRST attempt used two statements --
`Entry90902E8 *tbl = _ss_score[a0]; Entry90902E8 *rec = &tbl[a1];` --
and scored 19/31, with the residue being a whole prologue register
reshuffle: retail keeps the `43*slot` multiply-chain result live in a
callee-saved `$s0` from early on and only fetches the table pointer into
`$v0`/`$v1` afterward, while the two-statement form fetched the table
pointer FIRST (computing the `%hi`/`%lo`/`addu`/`lw` sequence before the
multiply chain) and put the multiply-chain result in `$v0` instead,
swapping which value gets promoted to `$s0`.

Collapsing the lookup into one expression -- `&_ss_score[a0][a1]`,
letting the compiler choose its own evaluation order for the two
independent sub-computations rather than a named `tbl` temp forcing the
table fetch first -- closed the whole function on the first rebuild after
the change (19/31 -> 31/31). The same fix was then applied to every other
function in this unit's family sharing this lookup shape.

**Proposed learning:** when a `arr[i][j]`-style double lookup involves two
independent sub-computations (an outer table-pointer fetch, an inner
byte-offset multiply), naming the outer fetch in its own temp variable can
force it to be evaluated FIRST and pin it to a temp register, when retail
computes the inner multiply first and keeps IT in the callee-saved
register instead. Writing the double-index inline lets the compiler order
the two independently, which reproduced retail's register choice for at
least 5 of 6 functions in this unit's family (`SetProgramChange`,
`ContRpn1`, `ContRpn2` matched outright; `SetPitchBend` and
`ReadDeltaValue` still had unrelated residues but this specific prologue
mismatch was gone from all of them).

## Verification

`./build-and-verify.sh` build exit=2 (clean compile, whole-image SHA1 not
yet green -- other functions in this unit and others are still queued).
`funcdiff.py SetProgramChange`: 31/31 words match, no drift.

## Round 97 types pass (echo)

code_179d8_k's local `Entry90902E8` view retired onto `include/SsScore.h`:
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
`build/nonmatching/src/code_179d8_k.c.o` before/after).
