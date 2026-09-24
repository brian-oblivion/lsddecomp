# Class86F88__ReleaseRows -- MATCH

> Renamed from `func_8005278C` on 2026-09-24 (tools/rename.py). Address 0x8005278c.

Unit `class_3bb8c_k`, round 15. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py Class86F88__ReleaseRows`: 36/36 words match.

This is vtable slot `+0x090` of `D_80086F88` (not declared in
`Class86F88Methods` since nothing in this unit dispatches through it).

## Source

```c
void Class86F88__ReleaseRows(Class86F88 *self)
{
    s32 count;
    s32 i;
    u8 unused[8];

    if (!self->unk50) {
        return;
    }
    count = self->unk10;
    i = 0;
    if (count >= 5) {
        count = 4;
    }
    if (count <= 0) {
        return;
    }
    do {
        self->unk40[i]->methods->release(self->unk40[i]);
        self->unk40[i] = NULL;
        i++;
    } while (i < count);
}
```

## Notes

Walks `self->unk40[0..count-1]` (an array of up to 4 `Class86F88Elem *`,
clamped from `self->unk10`), releasing each through its own vtable's
`+0x004` slot (the same shared base-class implementation documented for
`GenericReleaseMethods_3bb8c_d` elsewhere in this header -- confirmed by
this call site to be the same shape on `Class86F88ElemMethods`) and
nulling the slot. Four separate residues, closed one at a time with
`funcdiff.py` re-measured after each:

1. **`count = self->unk10;` read too early.** Written as the declaration's
   initializer, it compiled as an unconditional load before the
   `self->unk50` guard; retail only reads it after the guard passes.
   Moved to its own statement after the guard.
2. **The array walk needed indexing, not a hand-rolled incrementing
   pointer.** An explicit `Class86F88Elem **p = self->unk40; ...; p++;`
   walk reproduced the wrong addressing (`p` pointing directly at
   `unk40[0]`, offset folded into the pointer, dereferenced at offset 0)
   where retail keeps `self` itself as the walking base and a CONSTANT
   `+0x40` at each access, incrementing the base by 4. Plain
   `self->unk40[i]` reproduced this exactly -- GCC's own strength
   reduction turned the index into the same "increment the base register,
   fixed offset" shape retail uses.
3. **`i = 0;` needed to be its own statement between reading `count` and
   the `count >= 5` clamp, not the `for` loop's implicit initializer.**
   In retail's disassembly `i = 0` sits in the delay slot of the CLAMP
   branch (so it runs unconditionally regardless of which way the clamp
   goes); a `for (i = 0; i < count; i++)` loop instead materializes it
   right before the loop, one block too late. Also needed a `do`/`while`
   in place of `for`'s own guard: with a `for`, GCC emitted a REDUNDANT
   `i < count` check immediately after the `count <= 0` guard already
   established, that retail does not have -- the disassembly falls
   straight from the `blez count,END` check into the loop body with no
   second test.
4. **An 8-byte frame-size gap** (retail's frame is `0x28` bytes, the
   straightforward C compiled to `0x20`), closed with a dead, unreferenced
   `u8 unused[8];` local (this project's established "GCC reserves stack
   space for a completely dead local" idiom, now with another confirmed
   instance).
