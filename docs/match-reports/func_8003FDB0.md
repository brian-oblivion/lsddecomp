# func_8003FDB0 -- STALL (New_X epilogue-merge residue, 30/31)

Unit `code_2cc8c_e`, carved round 14.

**Correction to an earlier version of this report**, which claimed a full
31/31 match. That reading was taken while `include/code_2cc8c.h` had a
genuine bug (see "What actually happened" below) that broke `code_2cc8c.c`'s
compile and left `build/SLPS_015.56` stale for the whole batch that
introduced `ClassEAC0Obj`. Once the header bug was fixed and the whole unit
re-verified address-by-address against `build/lsdde.map`, this function's
own content -- unlike its neighbours -- turned out to still have a
one-word residue. Filed as a stall now; the earlier claim should be
disregarded.

## Shape

`New_Class6E99C`-shaped allocator, the checked-return-regardless variant:

```c
#if 0
Class6E99CObj *func_8003FDB0(void *a1, s32 a2, s32 a3) {
    Class6E99CObj *self;

    self = func_80017B34(0xA0);
    if (self != NULL) {
        func_800404C0()->ctor(self, a1, a2, a3);
    }
    return self;
}
#endif
```

## Residue

One word: on the allocation-failure path, retail materialises the return
value as `addu $v0, $zero, $zero` (a fresh zero); this body's single merged
`return self;` produces `move $v0, $s0` instead (also zero-valued on that
path, since `self` is `NULL` there, but a different instruction). This is
the project's documented "New_X epilogue-merge residue" class -- see
`docs/research/epilogue-merge-residue.md`, which already states plainly:
*"GCC 2.6.3 (Psy-Q) `-O2` will not merge two function exits carrying
DIFFERENT values into one epilogue"* and every hand-reachable shape found so
far either keeps the value-materializing `move` (like this one, 30/31) or
grows a second epilogue/extra register (worse). Retail plainly does merge
them; the source form that gets there has not been found.

## Attempts (3, beyond the shape above)

1. `if (self == NULL) { return NULL; }` early return instead of a merged
   `if (self != NULL) { ctor(...); }`: worse -- changed the branch offset
   (+1 word) and produced `sw ...; sw ...` in a different order, moving
   further from retail, not closer.
2. `if (self == NULL) { return self; }` (returning the already-null
   variable instead of a literal, to see if GCC would then reuse the
   register uniformly): same regression as attempt 1.
3. Confirmed via `tools/asm-differ/diff.py` that this is the ONLY
   remaining word of difference once the whole unit's address drift was
   resolved -- i.e. this is a clean, isolated one-instruction residue, not
   masked by anything else.

## What actually happened (for the next reader)

This function's retype (`SubHandleObj *` -> `Class6E99CObj *`) was
committed alongside a header change that used `ClassEAC0Obj` at a line
position ABOVE that type's own forward declaration -- a parse error in
`code_2cc8c.c` (a DIFFERENT unit) that made `make` skip the final link
entirely, leaving `build/SLPS_015.56` stale. `funcdiff.py`'s own
STALE-BUILD guard did not catch it because the STALE object here was
`code_2cc8c.c.o`, not `code_2cc8c_e.c.o` -- my own unit's object kept
rebuilding fine, so nothing about the guard's usual signal (source newer
than binary) looked wrong; the binary just never reached the final link
step at all. Caught only by cross-checking EVERY function's LINKED
ADDRESS against `build/lsdde.map` after fixing the forward-declare bug
(`typedef struct ClassEAC0Obj ClassEAC0Obj;` now sits at the top of
`include/code_2cc8c.h` next to the pre-existing `Class6E99CObj` one).

### Proposed learning

**Checking a build's EXIT STATUS is not the same as checking that the
CORRECT files got compiled.** A parse error in a unit you do not own can
leave your own object rebuilding successfully every time while the FINAL
LINK silently never runs, so `build/SLPS_015.56` stays pinned at whatever
it was before your session started. Every funcdiff score read during that
window is comparing against your OWN unedited baseline, not retail --
frequently indistinguishable from real matches for functions that started
as `INCLUDE_ASM` (case 2 in CLAUDE.md's "four ways a score lies", but via a
route that guard doesn't watch: a *different* file's compile failure, not
your own file's staleness or its own still-`INCLUDE_ASM` state). The
targeted check that catches it: after ANY multi-function commit, grep
`build/lsdde.map` for every touched symbol's address and diff it against
the expected retail address (from `asm/nonmatchings/<unit>/<func>.s`'s own
`glabel` line, or just the hex in the function name) -- a mismatch there
means SOMETHING upstream is wrong even when `build exit=` and funcdiff's
own guards say nothing.
