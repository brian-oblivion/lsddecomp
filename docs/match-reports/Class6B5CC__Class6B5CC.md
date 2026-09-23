# Class6B5CC__Class6B5CC

> Renamed from `func_8001CAF4` on 2026-09-23 (tools/rename.py). Address 0x8001caf4.

**Unit:** code_d294 · **Size:** 44 words · **Status:** MATCHED (44/44 words)

## What it does

`Class6B5CC`'s own constructor — vtable slot `+0x008` of `D_8006B5CC`
(confirmed directly: `tools/classtable.py D_8006B5CC` names `Class6B5CC__Class6B5CC`
as the occupant of that slot). Takes an already-allocated `self` (the
allocation itself is `New_Class6B5CC`, a separate `New_X` wrapper, not this
function). Allocates two sub-blocks (`self->unk14`, 0x50 bytes, then
`self->unk14->unk44`, 0x28 bytes), frees the first and bails out if the
second allocation fails, otherwise calls the BasicClass base constructor
(`Get_vtable_BasicClass()->ctor(self)`), overwrites `self->methods` with this
class's own vtable (`GetClass6B5CCMethods()`, i.e. `&D_8006B5CC`), zeroes several
freshly-added fields, and finally calls its own virtual init hook
(`self->methods->slot40`, `func_8001CE30` — still queued) before returning
`self` unconditionally.

## The C

```c
void *Class6B5CC__Class6B5CC(Class6B5CCObj *self) {
    void *blockB;

    self->unk14 = func_80017B34(0x50);
    if (self->unk14 == NULL) {
        return NULL;
    }
    blockB = func_80017B34(0x28);
    self->unk14->unk44 = blockB;
    if (blockB == NULL) {
        func_80017CFC(self->unk14);
        return NULL;
    }
    Get_vtable_BasicClass()->ctor(self);
    self->methods = GetClass6B5CCMethods();
    self->unk20 = 0;
    self->unk18 = 0;
    self->unkC = NULL;
    self->unk14->unk48 = 0;
    self->methods->slot40(self);
    return self;
}
```

## Two source-shape lessons, both confirmed by measured attempts

- **Do not cache a re-read field in a local across a call sequence.** A
  first attempt cached `self->unk14` once into a local `sub` and reused it
  for both `sub->unk44 = ...` and, much later (after 3 intervening calls),
  `sub->unk48 = 0`. That forced the compiler to promote the cache to a
  callee-saved register (`$s1`), which changed the frame size by 8 bytes
  (`-0x18` retail vs `-0x20` mine) and shifted every following byte in the
  whole ROM image — the classic address-drift failure mode. Writing
  `self->unk14->unk44 = ...` and `self->unk14->unk48 = 0` as two
  independent re-derivations (no cached local surviving the calls between
  them) let the compiler use a cheap caller-saved `$v1`, reloaded fresh
  right where retail reloads it, and the frame size matched immediately.
  This is the "mention the value multiple times, don't cache across a
  call" idiom, but for a struct-field re-read rather than a redundant
  `move` — worth adding to the general pattern in
  DECOMPILATION_LEARNINGS.md if a second instance turns up.
- **Instruction ORDER (store vs. call-argument setup) around a `jal`'s
  delay slot is source-order-sensitive in a way that isn't just "which
  statement is textually first".** `self->unk14 = func_80017B34(0x50);`
  compiled with the STORE and the FOLLOWING call's `li $a0` juggled by the
  scheduler; the version that matched byte-for-byte was writing the
  allocation call directly into the assignment
  (`self->unk14 = func_80017B34(0x50);`) rather than staging it through an
  intermediate local (`blockA = func_80017B34(0x50); ...; self->unk14 =
  blockA;`), even though both are logically identical. Prefer assigning a
  struct-field/global destination directly from the call expression over
  round-tripping it through a temporary, when a residue looks like two
  independent instructions swapped around a `jal`.

## Provenance

round 11 (2026-09-03), runner charlie, unit code_d294 (fresh carve, first attempt).
Both fixes above were found within the 30-attempt budget (2 rebuild
iterations total).
