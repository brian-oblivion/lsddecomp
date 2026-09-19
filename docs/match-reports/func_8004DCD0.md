> **REOPENED by round 42, AND SINCE WORKED -- marker spent (head, round 43).**
> This is now a DOCUMENTED STALL, not fresh ground: see the three-figure
> verdict below. The round-42 reopening text is kept for history. This function was
> screened as blocked by `gp_rel`. **That blocker is RESOLVED**: maspsx gained
> `--gp-symbols` and `--no-nop-mflo-mfhi` (`tools/patches/maspsx-lsd-flags.patch`,
> passed by the Makefile), the whole image stays byte-exact, and previously
> blocked functions now match (see `docs/research/gp-relative-blocker.md`,
> "RESOLVED"). Everything below is round 43's attempt AFTER the fix.

# func_8004DCD0 -- STALL, round 43

**Length: 2 words short (76/78, retail 0x138 bytes).**
**Raw word-match (unaligned, from the length-shifted window): 3/78.**
**First REAL diff (from `tools/asm-differ/diff.py`, which realigns): retail
vram `0x8004DCE0` has `move $s0, $a1` immediately after the first `sw`; the
built candidate has NO instruction there** -- everything from that point is
a length/placement cascade off this ONE missing instruction, not 75
independent residues. Every earlier line in the diff (`sw $s2`/`sw $s1`
register-number swaps) is a pure rename (`r`), not a real difference.

Unit `class_3bb8c_d`, class `Class86B60`. The round-14 stub recorded 9
`gp_rel` hits and no derivation. This round derived and nearly matched the
whole function; the residue is a register-CLASS choice on one local value,
not a control-flow or field-typing error.

## What's right

Every field, slot, and control-flow SHAPE below is confirmed correct by the
diff: no register-identity mismatch, no wrong branch target, no wrong
literal. The only difference is which HARD REGISTER CLASS the compiler
picks for one local pointer.

```c
typedef struct Arg1DCD0_3bb8c_d Arg1DCD0_3bb8c_d;
struct Arg1DCD0_3bb8c_d {
    s8 b0;
    s8 b1;
    s8 b2;
};

void func_8004DCD0(Class86B60 *self, Arg1DCD0_3bb8c_d *arg1)
{
    u8 buf[3];
    u8 *base;
    u8 v;

    Get_vtable_TaskCore()->slotE4(self, arg1);
    base = buf;
    if (self->unk3C != 0) {
        base[0] = 0;
        base[1] = 0;
        base[2] = 0;
        base += D_8008AA28;
        v = 0x80;
        goto store;
    }
    base[0] = arg1->b0;
    base[1] = arg1->b1;
    base[2] = arg1->b2;
    if (D_8008AA2C < 0x80) {
        base[0] = base[0] + 0x80;
        goto skip;
    }
    base += D_8008AA28;
    v = *base + 0x80;
store:
    *base = v;
skip:
    D_8008AA28++;
    if (D_8008AA28 >= 3) {
        D_8008AA28 = 0;
    }
    D_8008AA2C++;
    if (D_8008AA2C >= 0x101) {
        D_8008AA2C = 0;
    }
    self->unkB0->methods->slotB8(self->unkB0, buf);
}
```

This is preserved verbatim, `#if 0`-wrapped, immediately above the
`INCLUDE_ASM` in `src/class_3bb8c_d.c`.

## Derivation (all confirmed by the diff -- this is not in question)

- `Get_vtable_TaskCore()->slotE4(self, arg1)` -- a NEW slot on
  `BaseTaskCtorTable_3bb8c_c` at +0x0E4 (right after the existing
  `slotE0`, no gap), 2-argument, `self` forwarded then `arg1`. **Confirmed
  correct**: the diff shows the `jal 3dfbc` / `lw v0,0xe4(v0)` / `jalr`
  sequence byte-identical modulo register renames.
- `arg1` is `Arg1DCD0_3bb8c_d *`: three signed bytes (`lb` in the
  disassembly), read once each in the `self->unk3C == 0` path and written
  into `buf`. **Confirmed correct** -- the three `lb`/`sb` pairs match
  exactly (register renames only).
- `self->unk3C` -- a NEW `s32` field at +0x03C on `Class86B60` (splitting
  the existing `pad03C[0x048-0x03C]`), tested `!= 0`. **Confirmed
  correct.**
- The shared-tail structure: one arm reaches the byte store via a real `j`
  (retail's `j .L8004DD84`), the other via fallthrough -- the explicit
  `goto store` / label idiom already documented in
  `docs/DECOMPILATION_LEARNINGS.md` ("explicit `goto` to a SHARED label...
  whenever retail shows one arm reaching a merge point via a real jump and
  the other via fallthrough"). **Confirmed correct**: the `j <target>`
  placement and the branch structure both match; this was the single
  biggest lever that took the function from a totally different
  instruction stream (an earlier attempt without the `goto`, which
  diverged from word 1) to the near-miss reported here.
- `D_8008AA28` (`u8`, rolling 0/1/2 index, `asm/data/7B12C.sdata.s` --
  declared as a full `.word` there but accessed only via `lbu`/`sb`) and
  `D_8008AA2C` (`s32`, rolling counter) -- both **confirmed correct**,
  including the `andi v0,v0,0xff` re-mask retail emits after storing
  `D_8008AA28` back (the ordinary compiled shape of an unsigned-char
  increment-and-wrap).
- `self->unkB0->methods->slotB8(self->unkB0, buf)` -- reuses this round's
  `func_8004DCD0`... (no -- reuses the slot round 43 ALSO establishes
  in this same function; see header changes). **Confirmed correct.**

## The one open residue

Retail keeps the pointer `base = buf` (used at `base[D_8008AA28]` in TWO
different, non-adjacent basic blocks) in a THIRD callee-saved register
(`$s1`, alongside `self`=`$s2` and `arg1`=`$s0`) for the whole function --
saved in the prologue, restored in the epilogue, 2 extra words (8 bytes)
retail spends that this candidate does not. Every structural variant tried
here gets GCC 2.6.3 -O2 to allocate the equivalent value into a
CALLER-saved temp (`$a0`/`$a1`) instead, which is 2 instructions cheaper
and therefore 2 words short of retail's length.

**Four variants tried, all producing the IDENTICAL registerclass outcome**
(confirmed via `tools/asm-differ/diff.py`, not just funcdiff's score):

1. `p = &buf[D_8008AA28]` computed fresh in each branch, no `base`
   variable at all -- gave a COMPLETELY different, much worse mismatch
   (diverges from word 1; this is what "no explicit goto" looks like, see
   above).
2. `base = buf;` (unconditional, before the `if`) + a separate `p =
   &base[D_8008AA28];` pointer per branch -- 76/78, `base` in `$a1`.
3. Same as (2) but using `base[i]` consistently everywhere (including the
   plain `buf[0]`/`buf[1]`/`buf[2]` writes) instead of mixing `buf[i]`/
   `base[i]` -- identical result, `base` still in `$a1`.
4. `base += D_8008AA28;` (folding the offset into `base` itself, no
   separate `p`) -- identical result again, still `$a1`.
5. Removing `base` entirely, reading `D_8008AA28` and computing the store
   address only at a SINGLE shared point (`buf[D_8008AA28] = v;` after the
   goto, with `v` computed per-branch) -- this changes which BLOCK reads
   `D_8008AA28` (once, not twice), which does NOT match retail (retail's
   disassembly has two separate `lbu` reads of `D_8008AA28`, one per
   branch) -- rejected on structural grounds, not just score.

None of these is a register-identity mismatch in the sense CLAUDE.md's
hard rule addresses (no WRONG value ends up in the wrong place at runtime
-- it is purely a matter of which class of register the compiler spends on
a value that is otherwise handled identically), so it is not something an
`asm()` fix would even be tempting for. It reads as the SAME "GCC 2.6.3
local-alloc heuristic" class documented in `docs/DECOMPILATION_LEARNINGS.md`
round 12 ("retail saturates the callee-saved register file") but in
miniature and in the opposite direction: retail spends a THIRD s-register
on a value referenced only 3 times, and no C reshaping tried here
reproduces that spend.

## Permuter

Ran `tools/decomp-permuter` on variant (4) above (300s, `-j 4`,
`--stack-diffs`, no `PERM` macros -- pure randomization). **Setup note for
whoever resumes this**: `tools/setup-permuter.sh`'s generated
`compile.sh` did NOT include `--gp-symbols`/`--no-nop-mflo-mfhi` in its
`MASPSX_FLAGS` (it predates round 42's fix landing in the Makefile) --
patched by hand in the (gitignored) `permuter-work/func_8004DCD0/compile.sh`
for this run; `tools/setup-permuter.sh` itself was NOT touched (out of
this unit's scope). Without that patch the base score is nonsense (>3000,
comparing against a `%hi`/`%lo`-relocated build of the candidate against a
`%gp_rel` retail target). With it: base score 2050, best found 1186 after
~26,500 iterations, never reaching 0. Read as "not closed in 26,500
iterations under a 300s/4-job bound" -- not permuter-exhausted.

## Proposed learning

**A permuter helper script's own `compile.sh` can predate a toolchain fix
that landed in the real Makefile, and nothing about running it warns you.**
`tools/setup-permuter.sh` hardcodes `MASPSX_FLAGS` independently of
`Makefile`'s own, and round 42's `--gp-symbols`/`--no-nop-mflo-mfhi` never
propagated into it. For any function that touches a `%gp_rel` global (which
is most of this unit, post round-42), a permuter run through the
unmodified script scores every candidate against a MISCOMPILED base,
producing enormous, meaningless base scores that look like "far from
matching" when the candidate may already be very close. Diagnostic: if a
permuter base score for a `gp_rel`-touching function is in the thousands
immediately after a `--gp-symbols`-clean manual build scored under 100
in-range words, suspect the generated `compile.sh`, not the candidate.
This is a fix to `tools/setup-permuter.sh` itself (out of this unit's
scope to make) -- flagging for the head/operator rather than patching it
project-wide from a runner worktree.
