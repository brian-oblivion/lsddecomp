# New_class_65650

**Unit:** code_55dd4 · **Size:** 31 words (0x7C bytes) · **Status:** MATCHED
(31/31 words, whole-image `./build-and-verify.sh` green)

## What it does

The `New_X` allocator for the class whose method table is `D_8008A6C4`
(resolved with `tools/classtable.py D_8008A6C4 --vs 0x800878D4`; see
`include/code_55dd4.h` for the full inheritance chain). Allocates a 0x98-byte
instance through the game allocator `func_80017B34`, fetches the class's own
vtable via `func_80066818()` (a `Get_vtable`-style accessor, matched
separately, see its own report), and calls the constructor slot (`+0x008`)
with `(self, arg1, arg2)`. Unlike the simpler `New_X` shapes documented
elsewhere in this project (`func_80025B34`, `new_class_6d3c8`), **this one
also checks the constructor's own return value**: on constructor failure it
frees the object (`func_80017CFC`) and returns `NULL` instead of leaving it
allocated.

```c
void *New_class_65650(void *arg1, void *arg2)
{
    Class65650 *self;
    Class65650Methods *vt;

    self = (Class65650 *)func_80017B34(0x98);
    if (self == NULL) {
        return NULL;
    }
    vt = func_80066818();
    if (vt->ctor(self, arg1, arg2) != NULL) {
        return self;
    }
    func_80017CFC(self);
    return NULL;
}
```

## A third `New_X` sub-shape, and the `goto` lever did not apply here

The head's broadcast (round 2026-08-30-a, `func_80025B34`) established that for
a two-way early exit sharing one epilogue with *different* return values,
`goto fail; ...; fail: return OTHER;` is needed to match byte-for-byte — a
plain `if (...) return OTHER;` costs an extra `j`+`nop`.

This function is a **third sub-shape**, distinct from both `func_80025B34`
(ignores the constructor's return, one early exit) and `new_class_6d3c8`
(returns the allocation unconditionally, no early exit at all): it has **two**
early-exit-shaped branches (the allocation null check, and the constructor
failure check) plus a genuine three-way return (`NULL` on alloc failure,
`self` on success, `NULL` again — via a different path, after freeing — on
constructor failure).

Written with the straightforward, non-`goto` C above, it matched **31/31 on
the first successful build**, with no residue at all. So for this sub-shape,
**the `goto` lever was not needed.** A plausible reason: retail's own control
flow already converges through a *shared* epilogue block reached from three
places (not two), and with that many convergent paths GCC 2.6.3's block
layout puts the plain `if`/`return` bodies exactly where retail has them
without requiring the delay-slot trick that a strict two-way branch needs.
This is not a full explanation, just the empirical result — worth recording
so the next runner does not spend attempts reflexively applying the `goto`
rewrite to every `New_X` residue.

## Notes on the header

`include/code_55dd4.h` types `func_80017B34` as taking a single `s32 size`
parameter (matching the correction the head made to `class_16334.h`'s
`func_80017B34` prototype in the same round) — the call site here sets only
`$a0` before `jal`.

### Proposed learning

Not every `New_X` allocator is the same sub-shape. At least three exist in
this codebase: (1) ignore the constructor's return, single early exit
(`func_80025B34`, needs `goto`); (2) unconditional return, no early exit
(`new_class_6d3c8`, open stall); (3) check *both* the allocation and the
constructor's return, two early exits converging on one epilogue
(`New_class_65650`, matched with plain `if`/`return` — no `goto` needed).
Identify which shape a given `New_X` is (does the retail asm test the
constructor's own `$v0` after the `jalr`?) before assuming a `goto` rewrite is
required.
