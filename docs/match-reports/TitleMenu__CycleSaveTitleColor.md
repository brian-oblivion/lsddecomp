# TitleMenu__CycleSaveTitleColor -- MATCHED, round 75 (78/78, whole image OK)

> Renamed from `TitleMenu__TickNameFieldCursor` on 2026-09-26 (tools/rename.py). Address 0x8004dcd0.

> Renamed from `Class86B60__TickNameFieldCursor` on 2026-09-26 (tools/rename.py). Address 0x8004dcd0.

> Renamed from `func_8004DCD0` on 2026-09-24 (tools/rename.py). Address 0x8004dcd0.

REVISITED, round 75: MATCHED 78/78 in 11 builds; names/types used (the
`Arg1DCD0_3bb8c_d` type became the type of the local `buf`, copied whole).

**Lever: source shape, four levers, no permuter.** The round-43 residue
("register-class choice on one local pointer, no C reshaping reproduces it")
was a set of source-shape differences, one of which (the struct copy) the
round-43 body could not show because its `lbu`s were read as `lb`s-with-renames.

## Baseline (preserved round-43 body rebuilt live, before any change)

`insertions 9 / deletions 9` (opcode-level), 3/78 raw, 76/78 length (2 short).

## Levers, in order, with scores

| # | change | funcdiff | reading |
| --- | --- | --- | --- |
| 0 | preserved body | 3/78, ins/del 9/9, 2 words short | baseline |
| 1 | `base = buf;` moved ABOVE `GetTaskCoreMethods()->slotE4(...)` | 47/78, 7/7, length exact | `$s1` appears: `base` now crosses a call, so it gets a callee-saved reg; sched1 still sinks the `addiu s1,sp,0x10` into the `beqz` delay slot, which is why retail shows it after the call |
| 2 | separate `p = base + sSaveTitleColorChannel` per arm (not `base += ...`) | 40/78, 7/7 | store address in `$v1` like retail; s1/s2 swapped (transient) |
| 3 | `v` typed `s32` | same | `addiu 0x80` instead of `-0x80` (QImode const canonicalisation) |
| 4 | `s32` temps for the three bytes | drift | `lb` appears (retail loads are SIGN-extending) but CSE then reuses the register for `buf[0]+0x80` where retail reloads with `lbu` |
| 5 | one reused temp | drift | reload appears, but one register instead of retail's three |
| 6 | **`buf` is an `Arg1DCD0_3bb8c_d` and `buf = *arg1;`** | drift, only regs + one extra reload left | the 3-byte BLKmode struct copy is lb,lb,lb then sb,sb,sb into three regs, and CSE does not know memory after it -> `lbu 0x10(sp)` reload |
| 7 | `if (++sSaveTitleColorChannel >= 3)` instead of `sSaveTitleColorChannel++; if (sSaveTitleColorChannel >= 3)` | 71/78, 0/0 | `andi v0,0xff` instead of a reload of the global |
| 8 | drop `v`/`goto store`: `*p = 0x80;` and `*p += 0x80;` in if/else arms (GCC cross-jumps the shared `sb v0,0(v1)`) | 70/78, 1/1 | `v` now in `$v0` like retail; index reg wrong |
| 9 | `p = sSaveTitleColorChannel + base` | 70/78, 1/1 | no change |
| 10 | **no `p`: `base[sSaveTitleColorChannel] = 0x80;` / `base[sSaveTitleColorChannel] += 0x80;`** | **78/78, build exit=0, whole image OK** | index loaded straight into the address register |

## Matched source

```c
typedef struct Arg1DCD0_3bb8c_d Arg1DCD0_3bb8c_d;
struct Arg1DCD0_3bb8c_d {
    s8 b0;
    s8 b1;
    s8 b2;
};

void TitleMenu__CycleSaveTitleColor(TitleMenu *self, Arg1DCD0_3bb8c_d *arg1)
{
    Arg1DCD0_3bb8c_d buf;
    u8 *base;

    base = (u8 *)&buf;
    GetTaskCoreMethods()->slotE4(self, arg1);
    if (self->unk3C != 0) {
        base[0] = 0;
        base[1] = 0;
        base[2] = 0;
        base[sSaveTitleColorChannel] = 0x80;
    } else {
        buf = *arg1;
        if (sSaveTitleColorFrame < 0x80) {
            base[0] += 0x80;
        } else {
            base[sSaveTitleColorChannel] += 0x80;
        }
    }
    if (++sSaveTitleColorChannel >= 3) {
        sSaveTitleColorChannel = 0;
    }
    sSaveTitleColorFrame++;
    if (sSaveTitleColorFrame >= 0x101) {
        sSaveTitleColorFrame = 0;
    }
    self->nameField->methods->slotB8(self->nameField, &buf);
}
```

The two `goto`s the round-43 body needed are gone: retail's `j .L8004DD84`
(first arm into the shared `sb v0,0(v1)`) is GCC cross-jumping the identical
tails of `base[D] = 0x80` and `base[D] += 0x80`, not a shared label in the
source.

### Proposed learning

- **A callee-saved register holding a value whose only set sits AFTER a call
  can be a value the source computed BEFORE the call.** sched1 sinks the set
  past the call (into a later delay slot) but the pseudo was already counted
  as call-crossing, so it still gets an `$sN`. Tell: an `$sN` saved in the
  prologue whose first write is after the only call it could have crossed.
  Round 43 read this as "retail spends a third s-reg for no reason".
- **`lb`,`lb`,`lb` then `sb`,`sb`,`sb` into three distinct registers, and a
  later RELOAD of a byte just stored, is a small struct copy** (BLKmode move:
  CSE forgets memory after it). Per-field assignment gives `lbu` and reuses
  the register. Same family as "8-byte pair as one struct copy".
- **`x++; if (x >= K)` on a `u8` global reloads the global; `if (++x >= K)`
  gives the `andi 0xFF` re-mask.**
- **An address register that is also the index's load target
  (`lbu v1,idx; addu v1,s1,v1`) is `base[idx]` written inline, not a named
  `p = base + idx`.**

---

## History (round 43 STALL report, superseded)

> **REOPENED by round 42, AND SINCE WORKED -- marker spent (head, round 43).**
> This is now a DOCUMENTED STALL, not fresh ground: see the three-figure
> verdict below. The round-42 reopening text is kept for history. This function was
> screened as blocked by `gp_rel`. **That blocker is RESOLVED**: maspsx gained
> `--gp-symbols` and `--no-nop-mflo-mfhi` (`tools/patches/maspsx-lsd-flags.patch`,
> passed by the Makefile), the whole image stays byte-exact, and previously
> blocked functions now match (see `docs/research/gp-relative-blocker.md`,
> "RESOLVED"). Everything below is round 43's attempt AFTER the fix.

# TitleMenu__CycleSaveTitleColor -- STALL, round 43

**Length: 2 words short (76/78, retail 0x138 bytes).**
**Raw word-match (unaligned, from the length-shifted window): 3/78.**
**First REAL diff (from `tools/asm-differ/diff.py`, which realigns): retail
vram `0x8004DCE0` has `move $s0, $a1` immediately after the first `sw`; the
built candidate has NO instruction there** -- everything from that point is
a length/placement cascade off this ONE missing instruction, not 75
independent residues. Every earlier line in the diff (`sw $s2`/`sw $s1`
register-number swaps) is a pure rename (`r`), not a real difference.

Unit `title_menu`, class `TitleMenu`. The round-14 stub recorded 9
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

void TitleMenu__CycleSaveTitleColor(TitleMenu *self, Arg1DCD0_3bb8c_d *arg1)
{
    u8 buf[3];
    u8 *base;
    u8 v;

    GetTaskCoreMethods()->slotE4(self, arg1);
    base = buf;
    if (self->unk3C != 0) {
        base[0] = 0;
        base[1] = 0;
        base[2] = 0;
        base += sSaveTitleColorChannel;
        v = 0x80;
        goto store;
    }
    base[0] = arg1->b0;
    base[1] = arg1->b1;
    base[2] = arg1->b2;
    if (sSaveTitleColorFrame < 0x80) {
        base[0] = base[0] + 0x80;
        goto skip;
    }
    base += sSaveTitleColorChannel;
    v = *base + 0x80;
store:
    *base = v;
skip:
    sSaveTitleColorChannel++;
    if (sSaveTitleColorChannel >= 3) {
        sSaveTitleColorChannel = 0;
    }
    sSaveTitleColorFrame++;
    if (sSaveTitleColorFrame >= 0x101) {
        sSaveTitleColorFrame = 0;
    }
    self->nameField->methods->slotB8(self->nameField, buf);
}
```

This is preserved verbatim, `#if 0`-wrapped, immediately above the
`INCLUDE_ASM` in `src/ui/title_menu.c`.

## Derivation (all confirmed by the diff -- this is not in question)

- `GetTaskCoreMethods()->slotE4(self, arg1)` -- a NEW slot on
  `BaseTaskCtorTable_3bb8c_c` at +0x0E4 (right after the existing
  `slotE0`, no gap), 2-argument, `self` forwarded then `arg1`. **Confirmed
  correct**: the diff shows the `jal 3dfbc` / `lw v0,0xe4(v0)` / `jalr`
  sequence byte-identical modulo register renames.
- `arg1` is `Arg1DCD0_3bb8c_d *`: three signed bytes (`lb` in the
  disassembly), read once each in the `self->unk3C == 0` path and written
  into `buf`. **Confirmed correct** -- the three `lb`/`sb` pairs match
  exactly (register renames only).
- `self->unk3C` -- a NEW `s32` field at +0x03C on `TitleMenu` (splitting
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
- `sSaveTitleColorChannel` (`u8`, rolling 0/1/2 index, `asm/data/7B12C.sdata.s` --
  declared as a full `.word` there but accessed only via `lbu`/`sb`) and
  `sSaveTitleColorFrame` (`s32`, rolling counter) -- both **confirmed correct**,
  including the `andi v0,v0,0xff` re-mask retail emits after storing
  `sSaveTitleColorChannel` back (the ordinary compiled shape of an unsigned-char
  increment-and-wrap).
- `self->nameField->methods->slotB8(self->nameField, buf)` -- reuses this round's
  `TitleMenu__CycleSaveTitleColor`... (no -- reuses the slot round 43 ALSO establishes
  in this same function; see header changes). **Confirmed correct.**

## The one open residue

Retail keeps the pointer `base = buf` (used at `base[sSaveTitleColorChannel]` in TWO
different, non-adjacent basic blocks) in a THIRD callee-saved register
(`$s1`, alongside `self`=`$s2` and `arg1`=`$s0`) for the whole function --
saved in the prologue, restored in the epilogue, 2 extra words (8 bytes)
retail spends that this candidate does not. Every structural variant tried
here gets GCC 2.6.3 -O2 to allocate the equivalent value into a
CALLER-saved temp (`$a0`/`$a1`) instead, which is 2 instructions cheaper
and therefore 2 words short of retail's length.

**Four variants tried, all producing the IDENTICAL registerclass outcome**
(confirmed via `tools/asm-differ/diff.py`, not just funcdiff's score):

1. `p = &buf[sSaveTitleColorChannel]` computed fresh in each branch, no `base`
   variable at all -- gave a COMPLETELY different, much worse mismatch
   (diverges from word 1; this is what "no explicit goto" looks like, see
   above).
2. `base = buf;` (unconditional, before the `if`) + a separate `p =
   &base[sSaveTitleColorChannel];` pointer per branch -- 76/78, `base` in `$a1`.
3. Same as (2) but using `base[i]` consistently everywhere (including the
   plain `buf[0]`/`buf[1]`/`buf[2]` writes) instead of mixing `buf[i]`/
   `base[i]` -- identical result, `base` still in `$a1`.
4. `base += sSaveTitleColorChannel;` (folding the offset into `base` itself, no
   separate `p`) -- identical result again, still `$a1`.
5. Removing `base` entirely, reading `sSaveTitleColorChannel` and computing the store
   address only at a SINGLE shared point (`buf[sSaveTitleColorChannel] = v;` after the
   goto, with `v` computed per-branch) -- this changes which BLOCK reads
   `sSaveTitleColorChannel` (once, not twice), which does NOT match retail (retail's
   disassembly has two separate `lbu` reads of `sSaveTitleColorChannel`, one per
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
patched by hand in the (gitignored) `permuter-work/TitleMenu__CycleSaveTitleColor/compile.sh`
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

## Naming (round 77, naming runner delta)

Renamed `func_8004DCD0` -> `TitleMenu__CycleSaveTitleColor`. **Tier B**: Guarded by `self->unk3C`, cycles a 3-byte colour-like buffer through a rolling index (`sSaveTitleColorChannel`, 0-2) and a counter (`sSaveTitleColorFrame`, wraps at 0x101), then forwards the buffer to `self->nameField`'s `slotB8`. Read as a counter-driven colour/blink update on the name field's cursor; the exact visual effect is not established.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/task_core.h, track 4 round 84) now go through `GetTaskCoreMethods()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 88, bravo)

TitleMenu is unified in include/title_menu.h (TASKCORE_SLOTS/TASKCORE_FIELDS plus its own). The broadcastToSlots override (+0x0E4). Arg1DCD0_3bb8c_d was ColorRgb (include/sprite.h: three `s8`, the same layout), and the name field's +0x0B8 is setColor(ColorRgb *); `unk3C` is TaskCore's `inputMode`. Byte-identical (whole image green, 0 new warnings, nonmatching green).

## Track 7 (round 96, echo)

Naming: `D_8008AA28` -> `sSaveTitleColorChannel`, `D_8008AA2C` ->
`sSaveTitleColorFrame` (tier A: the lit channel 0..2 and the frame
counter, both read only here). Locals `buf` -> `rgb`, `base` ->
`channels`. Constants (unit-local, decimal): `SAVE_TITLE_LIT` 128 (the
lit level and what is added), `SAVE_TITLE_RED_FRAMES` 128,
`SAVE_TITLE_CYCLE_FRAMES` 257.

Comment moved here from the unit: "MATCHED round 75 (was STALL round
43). `base` is taken BEFORE the first call (so it crosses a call and gets
$s1), `buf = *color` is one struct copy (ColorRgb is three `s8`: three
`lb`, then three `sb`), and each arm indexes `base[D_8008AA28]` directly."
The unit keeps a one-line MATCHING note.
