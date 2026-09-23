# ContPortaTime -- MATCHED: 79/79, byte-exact

> Renamed from `func_80034AEC` on 2026-09-23 (tools/rename.py). Address 0x80034aec.

`asm/nonmatchings/code_179d8_k/ContPortaTime.s`, vram `0x80034AEC`, unit
`code_179d8_k`. Round 25, runner alpha (stall). Round 31, runner bravo
(closed).

## Round 31 update (runner bravo): closed via decomp-permuter, register-rescue class cracked

Re-verified round 25's baseline reproduces exactly before touching
anything: 38/79 raw word-match, compiled length 78/79 (one short), with the
correct-offset 0x20-sized `Scratch_80034AEC` struct.

Set up `tools/decomp-permuter` against this function (`tools/setup-permuter.sh`,
base score 110 confirmed against `--debug --stack-diffs` -- 1 deletion + 2
register differences, matching the report's own residue exactly). A
16-way `-j 12` search (`--stop-on-zero --best-only`) found a **score-0
candidate at iteration 36291** (~793 induced errors along the way, none
fatal to the search):

```c
u8 new_var;
...
func_800334F0(rec->unk4C, (((u8 *) rec) + (new_var = rec->unk12))[0x2C], &list);
for (i = 0; i < list.unk0; i++) {
    func_80033260(rec->unk4C, (((u8 *) rec) + new_var)[0x2C], (s16) i, &scratch);
    ...
    func_80036230(rec->unk4C, (((u8 *) rec) + new_var)[0x2C], (s16) i, &scratch);
}
```

**The fix is not caching `p` as a pointer at all.** The stalled body cached
`u8 *p = (u8 *)rec + rec->unk12;` once and reused the pointer at all three
call sites -- the natural C reading, and exactly what forces the allocator
to keep a live pointer register across the loop (needing the `$s0`->`$s4`
rescue retail's compile performs and this one didn't). Caching the raw
BYTE OFFSET instead (`u8 offset = rec->unk12;`) and recomputing
`((u8 *)rec + offset)[0x2C]` at each of the three use sites gives the
allocator a cheap-to-recompute scalar instead of a persistent pointer, and
it stops trying to keep a dedicated register alive across the loop --
which is exactly the condition under which retail's own compile needed
(and got) the rescue `move`. Translated to idiomatic C and verified through
the full pipeline:

```c
void ContPortaTime(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 offset;
    NoteList_800349B0 list;
    Scratch_80034AEC scratch;
    s32 i;

    func_800334F0(rec->unk4C, ((u8 *)rec + (offset = rec->unk12))[0x2C], &list);
    for (i = 0; i < list.unk0; i++) {
        func_80033260(rec->unk4C, ((u8 *)rec + offset)[0x2C], (s16)i, &scratch);
        scratch.unkB = a2;
        func_80036230(rec->unk4C, ((u8 *)rec + offset)[0x2C], (s16)i, &scratch);
    }
    rec->unk88 = func_80035E80(a0, a1);
}
```

Byte-exact, 79/79, no drift; `./build-and-verify.sh` whole-image SHA1
green. This SAME fix (same `offset`-not-`p` idiom) also closed the two
sibling functions sharing this residue class this round --
`ContModulation` (which additionally needed a struct-size fix, see that
report) and `ContPortamento` (which additionally needed an
immediate-canonicalization fix, see that report).

### Proposed learning

When a report's residue is "a rescue `move` retail has and this build
elides, one word short," the fix is not to coerce the ALLOCATOR into
keeping the same value alive across the same span (declaration reordering,
named temporaries at various positions -- all tried and inert per this
report's round-25/27 history) -- it is to stop asking it to keep a
PERSISTENT POINTER alive across the span at all. Caching the narrower
scalar the pointer is built from (here, a `u8` byte offset) and
recomputing the pointer expression at each use site changes what has to
survive the loop, which is a source-level lever distinct from every
"where do I declare this variable" axis this residue class's reports had
already exhausted.

## Original stall report (round 25, runner alpha), preserved below

## What it is

Structurally IDENTICAL to `ContModulation` (same unit, immediately
following it in ROM order): a per-(channel, slot) "dispatch note events"
loop calling `func_800334F0` to get an item count, then `func_80033260`
and `func_80036230` once per item with a scratch struct stamped with `a2`
in between. The only difference from `ContModulation` is WHERE the stamped
byte lands in the scratch struct: offset `0x2B` (relative to the stack
frame) here instead of `0x28` there -- i.e. the scratch struct's stamped
field is at relative offset `0xB` instead of `0x8`.

```c
typedef struct {
    u8 pad0[0xB];
    u8 unkB;    /* +0x0B: byte stamped between the two per-item calls */
    u8 pad9[0x20 - 0xC];
} Scratch_80034AEC;

void ContPortaTime(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 *p = (u8 *)rec + rec->unk12;
    NoteList_800349B0 list;    /* shared with ContModulation, see that report */
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

## Residue -- see `ContModulation.md` for the full derivation

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
reproduce -- see `ContModulation.md` for the full list of axes already
tried and ruled inert/regressive for this residue class, which applies
unchanged to this function.

## Verification

`./build-and-verify.sh` build exit=2 (clean compile, expected -- function
restored to `INCLUDE_ASM`). `funcdiff.py ContPortaTime`: compiled length
78/79 words (one short) with the correct-offset, 0x20-sized scratch struct
above (38/79 raw word-match at that configuration).
