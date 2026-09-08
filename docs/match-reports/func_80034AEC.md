# func_80034AEC -- STALL: length ONE WORD SHORT (78/79); 38/79 raw word-match; first real diff at word 33 (vram 0x80034B70, register rescue)

`asm/nonmatchings/code_179d8_k/func_80034AEC.s`, vram `0x80034AEC`, unit
`code_179d8_k`. Round 25, runner alpha.

## What it is

Structurally IDENTICAL to `func_800349B0` (same unit, immediately
following it in ROM order): a per-(channel, slot) "dispatch note events"
loop calling `func_800334F0` to get an item count, then `func_80033260`
and `func_80036230` once per item with a scratch struct stamped with `a2`
in between. The only difference from `func_800349B0` is WHERE the stamped
byte lands in the scratch struct: offset `0x2B` (relative to the stack
frame) here instead of `0x28` there -- i.e. the scratch struct's stamped
field is at relative offset `0xB` instead of `0x8`.

```c
typedef struct {
    u8 pad0[0xB];
    u8 unkB;    /* +0x0B: byte stamped between the two per-item calls */
    u8 pad9[0x20 - 0xC];
} Scratch_80034AEC;

void func_80034AEC(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 *p = (u8 *)rec + rec->unk12;
    NoteList_800349B0 list;    /* shared with func_800349B0, see that report */
    Scratch_80034AEC scratch;
    s32 i;

    func_800334F0(rec->unk4C, p[0x2C], &list);
    for (i = 0; i < list.unk0; i++) {
        func_80033260(rec->unk4C, p[0x2C], (s16)i, &scratch);
        scratch.unkB = a2;
        func_80036230(rec->unk4C, p[0x2C], (s16)i, &scratch);
    }
    rec->unk88 = func_80035E80(a0, a1);
}
```

## Residue -- see `func_800349B0.md` for the full derivation

Both of this function's residues are the exact same two identified in the
sibling report, reproduced here confirming they are a CLASS (a shared root
cause across at least two functions in this unit), not a one-off:

1. **One missing instruction, the same register-rescue `move`.** Retail
   computes the "state block" pointer into `$s0` first, then explicitly
   copies it to `$s4` before repurposing `$s0` as the loop counter. This
   build's allocator assigns `$s4` to that pointer directly, eliding the
   rescue -- confirmed identical in this function's own disassembly diff.
2. **Frame-size/struct-size interaction.** As with the sibling, sizing the
   scratch struct to its structurally "correct" total (0x28 bytes here,
   matching the sibling's real region) grows this build's frame 8 bytes
   past retail's `-0x70`; shrinking the SAME struct's declared size to
   0x20 (while keeping the stamped field's own offset, `0xB`, correct)
   recovers retail's exact frame size and moves the raw word-match from
   28/79 to 38/79 in one rebuild -- the identical lever, at the identical
   byte delta, as the sibling function.

No new axes were tried here beyond confirming the sibling's two findings
reproduce -- see `func_800349B0.md` for the full list of axes already
tried and ruled inert/regressive for this residue class, which applies
unchanged to this function.

## Verification

`./build-and-verify.sh` build exit=2 (clean compile, expected -- function
restored to `INCLUDE_ASM`). `funcdiff.py func_80034AEC`: compiled length
78/79 words (one short) with the correct-offset, 0x20-sized scratch struct
above (38/79 raw word-match at that configuration).
