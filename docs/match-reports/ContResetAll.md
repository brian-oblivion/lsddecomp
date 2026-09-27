# ContResetAll -- MATCHED 51/51, round 24 (head)

> Renamed from `func_80034D90` on 2026-09-23 (tools/rename.py). Address 0x80034d90.

> **VERDICT OVERTURNED, round 24 (2026-09-08). This function is MATCHED, and
> the class it was filed under needed a SCOPE BOUNDARY rather than another
> attempt.**
>
> Runner alpha filed the 2-word residue as the project's SETTLED
> commutative-operand-order canonicalization class -- retail `addu v0,s0,v0`
> against built `addu v0,v0,s0`, twice, same opcode, same operands, positions
> swapped. That screen matched exactly, the citation was accurate, and
> `DECOMPILATION_LEARNINGS` does tell you not to spend attempts reordering
> commutative operands. Alpha also tested a reshape (`u8 *ptr = rec->unk12 +
> (u8*)rec;`) which regressed to 34/51, and stopped -- correctly, by the
> guidance as written.
>
> **What closed it was REGROUPING, not reordering.** The body wrote
>
> ```c
> ((u8 *)rec)[rec->unk12 + 0x2C] = rec->unk12;   /* rec + (off + 0x2C) */
> ```
>
> and retail wants
>
> ```c
> *((u8 *)rec + rec->unk12 + 0x2C) = rec->unk12; /* (rec + off) + 0x2C */
> ```
>
> Same two changes on the `+0x17` store. Both `addu`s flip to retail's order
> and the function goes 49/51 -> **51/51**, whole-image SHA1 green.
>
> This is round 23's `&arr[i + j]` versus `arr + i + j` lever, which is
> already written down -- reaching the same `addu` the commutative class
> claims is unreachable. The two are not in conflict once the boundary is
> stated, and the boundary is the useful part:
>
> - The commutative class holds where the two addends are genuinely
>   SYMMETRIC, and there the operand order is an RTL canonicalization no
>   source order reaches. That is what round 20 measured, and the decisive
>   datum stands: both C operand ORDERS gave the same wrong output.
> - It does NOT hold where one addend is a BASE POINTER and the expression
>   can be re-associated. Grouping decides which value is `rs`, and grouping
>   is not operand order -- alpha's reshape changed the order and failed
>   because order is exactly the axis the class rules out.
>
> **The tell that this unit's own matched functions already had the answer:**
> three of alpha's matches in `code_179d8_k` use `(u8 *)rec + rec->unk12`
> (base first) and match. The stall used the subscript form. When a function
> disagrees with its own already-matched siblings' idiom, try the siblings'
> idiom before accepting a class verdict.


`asm/nonmatchings/code_179d8_k/ContResetAll.s`, vram `0x80034D90`, unit
`code_179d8_k`. Round 24, runner alpha.

## What it is

A per-channel/slot "reset voice" function: calls two already-matched
zero-argument helpers from `libsnd_cres` (`func_80036044`,
`func_80036518`), then resets several fields of the active embedded state
block (selected by the runtime byte offset `rec->unk12`, same idiom as
`SetProgramChange`'s family) to fixed sentinel values, and finally calls the
shared `ReadDeltaValue` VLQ-decode helper, caching its return into
`rec->unk88`.

```c
void ContResetAll(s16 a0, s16 a1)
{
    Entry90902E8 *rec = &_ss_score[a0][a1];

    func_80036044();
    func_80036518();

    ((u8 *)rec)[rec->unk12 + 0x2C] = rec->unk12;
    rec->unk13 = 0;
    rec->unk14 = 0;
    *(s16 *)((u8 *)rec + 0x4E + rec->unk12 * 2) = 0x7F;
    ((u8 *)rec)[rec->unk12 + 0x17] = 0x40;
    rec->unk88 = ReadDeltaValue(a0, a1);
}
```

Note `((u8 *)rec)[rec->unk12 + 0x2C] = rec->unk12;` reads `rec->unk12`
TWICE, with no cached local -- retail's own disassembly does two separate
`lbu` instructions for the same field (once for the address, once for the
stored value), which is what this reproduces; caching it in a local
variable would collapse those into one load and change the instruction
count.

## The lever that mattered

Collapsing `_ss_score[a0]` + `&tbl[a1]` into `&_ss_score[a0][a1]` (same
fix as `SetProgramChange`, see that report) took this from 37/51 to 49/51.

## The residue that did not close (2 words, one class)

```
25604: TARGET addu v0,s0,v0    CURRENT addu v0,v0,s0    (word 29, first real diff)
25630: TARGET addu v0,s0,v0    CURRENT addu v0,v0,s0    (word 40)
```

Both are the SAME computation (`ptr = rec + rec->unk12`, appearing twice
in the source: once for the `+0x2C` store, once for the `+0x17` store)
hitting the project's already-SETTLED commutative-operand-order
canonicalization class -- same opcode, same two operands, order swapped.
Tried: hoisting the pointer into an explicit `u8 *ptr = rec->unk12 +
(u8*)rec;` local (operands written in the reverse order) -- this did NOT
reproduce retail's order and additionally disturbed unrelated scheduling
elsewhere in the function (dropped the score to 34/51 with the `move
a0,s2`/`move a1,s1` prologue setup pushed later), so it was reverted
rather than kept as a regression. Not re-attempted further per
`DECOMPILATION_LEARNINGS.md`'s explicit guidance not to spend attempts
reordering commutative operands.

## Verification

`./build-and-verify.sh` build exit=2 (clean compile). `funcdiff.py
ContResetAll`: 49/51 words match, compiled length exact.
