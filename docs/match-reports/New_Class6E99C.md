# New_Class6E99C -- MATCHED (31/31 words)

> Renamed from `func_8003FDB0` on 2026-09-20 (tools/rename.py). Address 0x8003fdb0.

Unit `code_2cc8c_e`, carved round 14.

> **UPDATE (targeted permuter pass, round 17).** MATCHED, no permuter
> needed. `docs/research/epilogue-merge-residue.md` (updated 2026-09-02,
> after this function's stall was filed) documents the real discriminator
> for this whole residue class: **`return NULL;` must come textually LAST,
> after the success return**, not as an early guard clause. This function's
> preserved body used a single merged `if (self != NULL) { ctor(...); }
> return self;` -- neither family the research doc describes. Splitting
> into two explicit returns with `self` first and `NULL` last closed it on
> the first try:
>
> ```c
> Class6E99CObj *New_Class6E99C(void *a1, s32 a2, s32 a3) {
>     Class6E99CObj *self;
>
>     self = BMemPMgrAlloc(0xA0);
>     if (self != NULL) {
>         GetClass6E99CMethods()->ctor(self, a1, a2, a3);
>         return self;
>     }
>     return NULL;
> }
> ```
>
> Verified: `./build-and-verify.sh` exit 0 (full-image SHA1 match),
> `funcdiff.py` 31/31.

## Shape

`New_Class6E99C`-shaped allocator, the checked-return-regardless variant.

## Residue (closed)

One word: on the allocation-failure path, retail materialises the return
value as `addu $v0, $zero, $zero` (a fresh zero); the earlier single-merged
`return self;` form produced `move $v0, $s0` instead (also zero-valued on
that path, since `self` is `NULL` there, but a different instruction). This
was the project's documented "New_X epilogue-merge residue" class -- see
`docs/research/epilogue-merge-residue.md` for the full discriminator
(textual order of the two `return`s, not "does GCC merge exits with
different values" as an earlier draft of that research doc claimed).

## Attempts (3, beyond the merged-return shape, all pre-dating the fix)

1. `if (self == NULL) { return NULL; }` early return instead of a merged
   `if (self != NULL) { ctor(...); }`: worse at the time -- changed the
   branch offset (+1 word) and produced `sw ...; sw ...` in a different
   order. (This is the "`return NULL;` FIRST" family the research doc later
   confirmed always fails -- consistent with what closed it being the
   OPPOSITE order.)
2. `if (self == NULL) { return self; }`: same regression as attempt 1.
3. Confirmed via `tools/asm-differ/diff.py` that the one-word residue was
   clean and isolated, not masked by anything else.

## What actually happened (kept for the record -- a real process lesson)

This function's retype (`SubHandleObj *` -> `Class6E99CObj *`) was
committed alongside a header change that used `ClassEAC0Obj` at a line
position ABOVE that type's own forward declaration -- a parse error in
`code_2cc8c.c` (a DIFFERENT unit) that made `make` skip the final link
entirely, leaving `build/SLPS_015.56` stale. `funcdiff.py`'s own
STALE-BUILD guard did not catch it because the STALE object here was
`code_2cc8c.c.o`, not `code_2cc8c_e.c.o` -- this unit's own object kept
rebuilding fine, so nothing about the guard's usual signal (source newer
than binary) looked wrong; the binary just never reached the final link
step at all. Caught only by cross-checking EVERY function's LINKED
ADDRESS against `build/lsdde.map` after fixing the forward-declare bug.

### Proposed learning

**Checking a build's EXIT STATUS is not the same as checking that the
CORRECT files got compiled.** A parse error in a unit you do not own can
leave your own object rebuilding successfully every time while the FINAL
LINK silently never runs, so `build/SLPS_015.56` stays pinned at whatever
it was before your session started. The targeted check that catches it:
after ANY multi-function commit, grep `build/lsdde.map` for every touched
symbol's address and diff it against the expected retail address -- a
mismatch there means SOMETHING upstream is wrong even when `build exit=`
and funcdiff's own guards say nothing.

**Superseded by this round's fix:** the class is not "unfixable, retail
merges what GCC won't" -- it is a textual-order-of-returns sensitivity, and
once the research doc identified that, this function (and its sibling
`New_BoxFill`) matched on the very first correctly-shaped attempt.

## Naming (round 61, track 3)

**`New_Class6E99C`** -- tier A. Standard `New_X`-shaped allocator: allocates
a fixed 0xA0 bytes (`Class6E99CObj`'s own size) and, on success, dispatches
its ctor through `GetClass6E99CMethods()->ctor(...)` before returning it;
matches the project's established `New_X` convention (`New_Entity`,
`New_DreamSys`, `New_Obj6EAC0`, etc.) exactly. Mechanics fully determine the
name; no game-purpose claim beyond "allocate and construct one".
