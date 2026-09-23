# Class876FC__PlaceModelChildren -- MATCHED (84/84 words)

> Renamed from `func_80056858` on 2026-09-23 (tools/rename.py). Address 0x80056858.

Unit `class_3bb8c_s`. `self` is the owning `LinkNode`; this is the function
that either attaches a fresh child (via `class_3bb8c_o.c`'s
`New_BaseObjO`/`Class6B5CC__LinkModel` plus this unit's own `AttachWithRotScale`) or
re-touches an existing one (`self->arr7C[i]->methods->slotB8`), driven by its
own `reuse` argument.

## Classification

Clean on all four carve-time screens. The only real lever needed was
CACHING `self->unk6C` into a local before the loop, rather than re-reading it
through the pointer each time.

## Body

```c
extern Vec3S D_800877EC;
extern s32 D_800877F8[];

void Class876FC__PlaceModelChildren(LinkNode *self, s32 reuse) {
    Vec3S accum;
    LinkNode **p;
    s32 i;
    s32 count = self->unk6C;

    if (count == 0) {
        return;
    }
    accum = D_800877EC;
    p = self->arr7C;
    for (i = 0; i < 2; i++, p++) {
        if (count < 3) {
            accum.x += *(s16 *)self->unk68 * D_800877F8[count];
        } else {
            accum.y += D_800877F8[count];
        }
        if (reuse) {
            LinkNode *child = *p;
            child->methods->slotB8(child, &accum);
        } else {
            LinkNode *child = New_BaseObjO();
            *p = child;
            Class6B5CC__LinkModel(child, self->unk20);
            AttachWithRotScale(*p, self, &accum, self->unk64, self->unk68);
        }
    }
}
```

## Notes -- residue this round, and how it closed

The FIRST attempt read `self->unk6C` directly (as `self->unk6C`) at each of
its five use sites instead of caching it, on the theory that GCC 2.6.3 would
CSE the repeated pointer dereference into a register automatically. It did
not: retail keeps this value in `$s4` for the whole function, never reloading
it even across three intervening calls, which only happens if the SOURCE
caches it into a plain local (a simple register-resident scalar is safe to
keep live across an opaque call under this compiler's conservative alias
model; a repeated `self->unk6C` dereference is not, because a call could in
principle write through some other pointer to that same memory). The first
attempt scored 29/84 with the whole function's frame size wrong (`-0x40`
instead of retail's `-0x48`) because two fewer registers needed saving.
Caching `count = self->unk6C` up front and using `count` everywhere else
fixed it outright -- one word.

The three-word residue seen along the way (`D_800877EC`'s/`D_800877F8`'s own
`%lo` immediates and one `jal` target, all off by exactly 4) was pure address
drift from `Class876FC__BuildRandomSprites` (this unit's sixth function this round) not yet
being byte-exact -- not a real defect in this function. `./build-and-verify.sh`
confirmed 0 bytes differing once that stall was resolved by restoring its
`INCLUDE_ASM`.

### Proposed learning

**A repeated `self->field` dereference across a loop containing function
calls is NOT automatically cached into a register by this compiler, even
when the field provably cannot change.** If retail keeps a value live in a
callee-saved register across intervening calls without ever reloading it,
and your C reads the SAME pointer-dereferenced field at each use site instead
of caching it in a local first, expect a wrong (smaller) stack frame and a
wrong register-saved set, not just a content residue -- the missing local
changes how many callee-saved registers the function needs at all.
