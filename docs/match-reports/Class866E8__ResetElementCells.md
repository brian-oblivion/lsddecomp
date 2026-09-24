# Class866E8__ResetElementCells

> Renamed from `func_8004C0AC` on 2026-09-24 (tools/rename.py). Address 0x8004c0ac.

**Unit:** class_3bb8c · **Size:** 43 words · **Status:** MATCHED (5 attempts).

## Result

```c
void Class866E8__ResetElementCells(Obj866E8 *self, Elem *entry) {
    EntryChildObj **p;
    EntryChildObj **end;

    if (entry->unk4->unk30 >= 0) {
        entry->unk4->methods->slot7C(entry->unk4, entry);
        end = entry->unk10 + 0x19A;
        for (p = entry->unk10; p < end; p++) {
            (*p)->unk10 |= 0x80000000;
            (*p)->unk20 = 0;
            (*p)->unk18 = 0;
        }
    }
}
```

## Derivation

`entry` is one of `Obj866E8::arr[7]`'s own elements (class_3ac78's
independent view of the same class calls it `UnkSlotEntry_3ac78`, confirming
the `+0x010` array-of-pointers field this function reads).
`entry->unk4->methods->slot7C(entry->unk4, entry)` dispatches through the
TARGET object's own method table (`entry->unk4->methods`), not `self`'s --
the call's `$a0` is `entry->unk4` itself, not `self`, established by tracing
which register held which value at the `jalr`.

The loop scans `entry->unk10` (an array of pointers) across a fixed 0x668
BYTE span (`0x19A` = 410 pointer-sized elements, confirmed
`0x19A*4 == 0x668` exactly) and unconditionally sets a flag bit and zeroes two
fields on every element in range.

## Residue and the fix

First four attempts stalled at various partial scores (16/43, with a
"differs outside range" warning on two of them) chasing what looked like a
`p`/`end` REGISTER-ROLE swap: retail computes `p = entry->unk10` into `$v0`
first, then an explicit `move $a0,$v0` establishes `p`'s home register while
`end` (`$a1`) is computed directly from the SAME `$v0` -- one extra `move`
retail has that a straightforward `p = entry->unk10; end = p + 0x19A;`
optimizes away entirely (GCC just puts the loaded value directly into
whichever register `p` ends up in, no separate copy).

**The fix: mention `entry->unk10` TWICE in the source, once for each of `end`
and `p`'s initializers, in that order** (`end = entry->unk10 + 0x19A; for (p =
entry->unk10; ...)`) -- not through an intermediate named local. GCC's CSE
still collapses this to a SINGLE `lw` (recognizing the two mentions are the
same value), but because `end`'s initializer is evaluated (and its temporary
established) before `p`'s, the compiler is left needing an explicit `move` to
give `p` its own register out of the already-computed temporary -- exactly
retail's shape. A cached-through-one-local version, or reversing which of
`p`/`end` is initialized first, either failed to produce the extra `move` at
all or shifted it to the wrong register pairing.

### Proposed learning

**When retail has a "redundant" `move` establishing a loop pointer from an
already-loaded value used to also compute a second, related quantity (an
`end`/bound pointer, typically) — try mentioning the SAME source expression
TWICE, in the order [dependent-quantity-first, loop-variable-second], rather
than through a single cached local.** GCC's own CSE still emits one load, but
this specific write order is what makes it need the extra `move` rather than
folding the loop variable directly into the temporary's register. This is a
constructive counterpart to the broadcast-#1/#2 "redundant move" lever, for
the case where the extra `move` genuinely IS present in retail rather than
being a smell to remove.
