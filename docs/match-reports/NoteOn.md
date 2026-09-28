# NoteOn -- STALL: length EXACT 70/70; 62/70 raw word-match (round 47, up from 61/70); first real diff at word 1

> Renamed from `func_800344FC` on 2026-09-23 (tools/rename.py). Address 0x800344fc.

`asm/nonmatchings/libsnd_seqread/NoteOn.s`, vram `0x800344FC`, unit
`libsnd_seqread`. Round 24, runner alpha. Continuation past this unit's
assigned six functions. Round 31, runner bravo (re-verified, still
stalled).

## Round 31 update (runner bravo): re-verified, classification confirmed

Rebuilt the preserved near-miss body and reproduced the title figures
exactly: 44/70 raw word-match, compiled length exact (70/70), first real
diff at word 1. `tools/asm-differ/diff.py NoteOn` confirms every
remaining mismatch is a same-opcode, same-operand REGISTER RENAME
(`$t0`<->`$a2`, `$s0`<->`$s1`, `$t1`/`$t2`/`$t3`<->`$t0`/`$t1`/`$t2`),
exactly as this report's original derivation found. Checked this
function's delay-slot filler (`move $a3,$a2` in the `flag==0` early
return's delay slot) against the possibility raised by
`ContNrpn2.md`'s round-31 finding -- that a delay-slot filler which
"looks dead" can actually be a hidden UNCONDITIONAL memory write the
stalled C only modeled on one branch arm. **That possibility does not
apply here**: this delay slot's instruction is a bare GPR-to-GPR `move`
with no memory effect at all, so there is no hidden write to relocate --
unlike `ContNrpn2`'s `sb $a2,0x16($s0)`, a register move that is
never read again has no observable effect on ANY path, taken or not, and
this report's original derivation already confirmed as much by testing
an explicit equivalent dead statement (`if (flag==0) { a3=a2; return;
}`) with zero effect on the compiled output. No new axis found; not
re-attempted with `decomp-permuter` this round given the budget went to
the higher-yield near-miss cluster instead (see `ContPortaTime.md`,
`ContModulation.md`, `ContPortamento.md`, `SetPitchBend.md`,
`ContNrpn2.md` for this round's five matches). This remains a
genuine register-identity STALL per `CLAUDE.md`'s explicit rule.

## Round 32 update (runner bravo): dead-value reuse closed 17 of 26 residue words -- 44/70 -> 61/70

`tools/decomp-permuter` had never been run against this function (round 31
spent its budget on the higher-yield near-miss cluster instead). Set up a
scaffold from this report's own preserved body; `--debug --stack-diffs`
reproduced the exact residue this report already described (base score 710:
22 register differences, 3 insertions, 3 deletions -- consistent with "a
consistent register rename", the insert/delete pairs being the scorer's way
of expressing a renamed register as add-one/remove-one rather than a
substitution). A 240s `-j 6 --stop-on-zero --best-only` search (confirmed
exit via `timeout`, not an external kill) did not reach zero, but found a
candidate at local score 50 (down from 710) -- a real structural lead, not
noise:

```c
speed = a3;              /* inserted right after `status` is read       */
...
rec->unkA8 = (u8)speed;  /* was (u8)a3 -- reuse the dying `speed` local */
```

`speed` (the per-voice array value used to compute `divided`) is dead after
that computation on the `a3 != 0` path -- exactly the project's documented
"assign the result back into a dying operand" lever
(`DECOMPILATION_LEARNINGS.md`, "Two levers that closed long-standing
near-misses by DELETING a named value"): reusing `speed`'s already-allocated
storage for `(u8)a3` rather than re-reading `a3` fresh at the store site
gives the allocator the same register-reuse opportunity retail's compiler
took. Translated to real C and verified through the full oracle (not the
permuter's local metric, per `docs/MATCHING-GUIDE.md`'s standing caution):
**44/70 -> 61/70**, compiled length still exact (70/70), and the residue
collapsed to a clean two-way register rotation (see below) -- 9 diff rows via
`asm-differ`, matching `funcdiff`'s 70-61=9 exactly (no undercount).

**Two follow-up variants tried, both negative, both real-oracle-verified (not
just permuter-local):**

- Reusing the function's own genuinely-dead 3rd parameter (`a2`, established
  by the original derivation as "silent ABI waste") as the carrier instead of
  `speed` -- i.e. `a2 = a3;` / `rec->unkA8 = (u8)a2;`. This is the sibling
  lever from `DECOMPILATION_LEARNINGS.md`'s "reuse a dead PARAMETER instead"
  entry. **Regressed to 44/70** (back to the original baseline) -- confirming
  that lever's own documented discriminator: it helps when the local exists
  *only* to carry one branch's result to one later use, which `a2` does not
  satisfy any better than `speed` does here, and in this case picking the
  wrong dead carrier undoes the win rather than being neutral.
- Reusing the SAME `a2` idea to carry `a0` instead (across the two `packed =
  (a1<<8)|a0` sites, the OTHER unresolved register in the residue --
  retail's `$t0`, this build's `$a2`): `a2 = a0;` at top, `(s16)a2` in place
  of `a0` at both `packed` sites. **60/70 -- one word worse** than the 61/70
  already reached. Reverted.
- Moving the `speed = a3;` assignment from unconditionally-before-the-guard
  to inside the `(u8)a3 != 0` branch (still semantically identical, since
  `speed` is only read on that path): changed the compiled LENGTH itself
  (209667 bytes of whole-image drift), so this is not a neutral reshape here
  the way it might look -- reverted immediately without further
  investigation.

**Residue that remains (9 words, real-oracle-confirmed, `asm-differ`
realigned):** a clean two-way register rotation, `$t0`(retail, the a0 copy
held live across both calls)<->`$a2`(built) and `$s1`(retail, the masked-a3
copy)<->`$t0`(built) -- i.e. retail's `s1` role is what our `speed` reuse
landed in `$t0` instead of. Not reachable by any of the three variants above
without giving back the word already won; still a genuine register-identity
STALL by CLAUDE.md's rule (no `register T v asm("$N")`/operand-constraint
fix attempted or considered). Left at 61/70, `INCLUDE_ASM` restored --
`build-and-verify.sh` build exit=0, whole image byte-exact, with this
function's near-miss body preserved in `#if 0` (updated above to the
new 61/70 body, superseding the 44/70 one this report originally carried).

### Round 32 lever checklist (this function)

- **Lever 1 (narrow `volatile`)**: not applicable -- no fold/fusion residue
  here to defeat, the residue is pure register identity.
- **Lever 2 (register-identity verdict is a hypothesis, check for a reused
  scratch value)**: **APPLIED, and it is what closed 17 of 26 words.** The
  "consistent register rename" verdict was correct as a description but
  incomplete as a diagnosis -- the actual mechanism was a value (`speed`)
  going dead one statement earlier than the C exploited.
- **Lever 3 (emission order != source order)**: not separately implicated;
  the fix here was value-reuse, not reordering.
- **Lever 4 (permuter negative is evidence about one search, not the
  function)**: confirmed the corollary too -- the permuter's own best score
  (50) undersold the real win by permuter-units, and its case only became
  useful once translated and verified through the real oracle rather than
  trusted at face value.
- **Lever 5 (asm-differ/permuter compare text, not opcodes)**: not
  implicated this round; no `addiu`/`ori` ambiguity found in this residue.

## What it is

A per-channel/slot "note dispatch" function. Reads a per-voice "speed"
value from an `s16` array embedded at `rec+0x4E`, indexed by the same
runtime byte offset (`rec->unk12`) this unit's other functions use to
locate a currently-active state block; multiplies it by the (byte-masked)
4th parameter and divides by **127**, using the classic GCC
signed-divide-by-constant magic-multiply idiom
(`mult`/`mfhi`/`add`/`sra`/`sra 31`/`subu`). **The divisor was recovered
by brute-force reproduction, not derivation**: the pinned pipeline was
run for `(a*b)/N` across a range of plausible small `N`, and `N=127`
alone reproduces retail's exact magic constant `0x81020409` with shift
`6` -- see the "Escalate, do not experiment" section of CLAUDE.md for why
this is the correct way to pin a magic-multiply constant rather than
guessing from context.

Two new fields on this unit's `Entry90902E8` local struct view:
`unk74` (u16, a nonzero-gated dispatch enable) and `unkA8` (s16, a cached
masked-byte parameter).

Body as originally reached (round 24/31, 44/70 -- **superseded, see the round
32 update above for the current 61/70 body preserved in `src/`**):

```c
void NoteOn(s16 a0, s16 a1, s32 a2, s32 a3)
{
    Entry90902E8 *rec = &_ss_score[a0][a1];
    u8 offset = rec->unk12;
    s16 speed = *(s16 *)((u8 *)rec + 0x4E + offset * 2);
    s32 divided = ((u8)a3 * (s32)speed) / 127;
    u8 *ptr = offset + (u8 *)rec;
    u16 flag = rec->unk74;
    u8 status = ptr[0x17];

    if (flag == 0) {
        return;
    }
    if ((u8)a3 != 0) {
        s16 packed = (a1 << 8) | a0;
        s16 note = rec->unk4C;
        u8 vol = ptr[0x2C];
        SpuVmKeyOn(packed, note, vol, (u8)a3, (u16)divided, status);
        rec->unkA8 = (u8)a3;
    } else {
        s16 packed = (a1 << 8) | a0;
        s16 note = rec->unk4C;
        u8 vol = ptr[0x2C];
        SpuVmKeyOff(packed, note, vol, (u8)a3);
    }
}
```

Current best body (round 32, 61/70 -- this is what `src/psyq/libsnd_seqread.c`
actually preserves in `#if 0` now):

```c
void NoteOn(s16 a0, s16 a1, s32 a2, s32 a3)
{
    Entry90902E8 *rec = &_ss_score[a0][a1];
    u8 offset = rec->unk12;
    s16 speed = *(s16 *)((u8 *)rec + 0x4E + offset * 2);
    s32 divided = ((u8)a3 * (s32)speed) / 127;
    u8 *ptr = offset + (u8 *)rec;
    u16 flag = rec->unk74;
    u8 status = ptr[0x17];

    speed = a3;
    if (flag == 0) {
        return;
    }
    if ((u8)a3 != 0) {
        s16 packed = (a1 << 8) | a0;
        s16 note = rec->unk4C;
        u8 vol = ptr[0x2C];
        SpuVmKeyOn(packed, note, vol, (u8)a3, (u16)divided, status);
        rec->unkA8 = (u8)speed;
    } else {
        s16 packed = (a1 << 8) | a0;
        s16 note = rec->unk4C;
        u8 vol = ptr[0x2C];
        SpuVmKeyOff(packed, note, vol, (u8)a3);
    }
}
```

The 3rd parameter (`a2`) is genuinely unused in every reachable path --
same "silent ABI waste" idiom already documented for other functions in
this project (`SpuVmSeKeyOn`, `func_800319B4`, and this unit's own
`SeqPlay`).

## Levers that mattered

- **`status = ptr[0x17]` must be computed UNCONDITIONALLY, before the
  `flag == 0` guard**, even though it is only ever CONSUMED inside the
  `(u8)a3 != 0` branch. Retail's disassembly loads it in the shared
  prefix before either branch point. Moving the read to inside the
  branch (the more "obvious" placement, since that is the only place it
  is used) would change which instructions execute on the early-return
  path and is a length/scheduling hazard; keeping it at the top,
  unconditionally, matches retail exactly.
- **Recompute the masked byte parameter, `(u8)a3`, FRESH at every use
  site (the multiply, each branch's condition, each call argument, and
  the final field store) rather than caching it once in a named
  local.** An earlier attempt cached it as `u8 vel = a3 & 0xFF;` used
  everywhere; this compiled 4 words SHORT (66/70) because retail
  performs the SAME mask operation independently at each site (`andi
  t1,s1,0xff` for the multiply, then a SEPARATE `andi a3,a3,0xff` at each
  call site) rather than reusing one cached value. Same family as this
  unit's other "redundant re-read" idiom (`ContResetAll`'s quadruple
  `rec->unk12` reads).
- **The packed dispatch id (`(a1<<8)|a0`) needs an explicit `s16`
  truncate-and-sign-extend, not a bare `s32`.** Declaring `packed` as
  `s32` dropped the `sll $a0,0x10`/`sra $a0,0x10` pair retail has right
  after the `or`; declaring it `s16` (letting the assignment itself
  trigger the cast) restored it. This is the SAME lever as the "return
  type discarded" family, but for an ARGUMENT rather than a return value:
  the compiled bytes alone do not reveal an argument's width, only its
  value, so the narrower type had to be inferred from the extra
  instruction pair retail emits to enforce it.

Together these took the function from 21/70 (initial transcription,
before the divisor was confirmed and before either lever above) to
44/70, with the compiled length exact throughout the process once the
divisor was right.

## The residue that did NOT close as of round 31 (26 words, all one class) -- HISTORICAL, see round 32 update above

Every remaining difference was a REGISTER RENAME with the identical
opcode and identical semantic operands -- confirmed by diffing register
symbols only:

```
$t0 (retail, the raw copy of a0 kept for the later packed-id OR) <-> $a2 (built)
$s0 (retail, the rec pointer)                                     <-> $s1 (built)
$t1/$t2/$t3 (retail, three temporaries)                            <-> $t0/$t1/$t2 (built), each shifted by one
```

The mechanism is visible in one delay slot: retail's `beqz $v0,END`
(the `flag == 0` early return) has `move $a3,$a2` in its delay slot --
a value that is providably NEVER READ again on that path, since the
function returns immediately after. This is the SAME class this
session's `SeqPlay` hit (a delay slot filled with a
computation that only APPEARS meaningful, reusing the caller's
otherwise-dead 3rd argument register because it happened to be free).
Tried (round 31): writing the equivalent dead statement explicitly in C
(`if (flag == 0) { a3 = a2; return; }`) to see whether making it a real
statement rather than relying on GCC to invent one would pin `$a2`
elsewhere and free `$t0` for the a0-copy -- **no change at all** in the
compiled output (44/70 both ways). Reverted rather than kept, since it
adds a confusing dead statement for zero benefit.

**Round 32 closed 17 of these 26 words** -- see the round 32 update above.
The `$s0`/`$s1` swap and the `$t1`/`$t2`/`$t3` shift are gone; what remains is
the narrower `$t0`/`$a2` and `$s1`/`$t0` two-way rotation described there.
Per CLAUDE.md's explicit rule this is still a register-identity STALL: same
instructions, different registers, and the axes tried past the round-32 win
did not move it further. Not spending further attempts on this axis this
round.

## Verification

`./build-and-verify.sh` build exit=0 with `NoteOn` restored to
`INCLUDE_ASM` (near-miss body preserved above and in `src/` as `#if 0`, now
the round-32 61/70 version). `funcdiff.py NoteOn` against the
near-miss build (prior to reverting): 61/70 words match, compiled length
exact (70/70). Whole-image `build-and-verify.sh` re-confirmed byte-exact
after reverting to `INCLUDE_ASM`.

## Round 39 update (runner alpha): re-verified; hoist-both-before-either checked (not applicable); two new reshapes tried, both negative

Rebuilt the round-32 preserved body: reproduces exactly, 61/70, length
exact, first real diff at word 1, same two-way register rotation
(`$t0`(retail)<->`$a2`(built) for the a0 copy, `$s1`(retail)<->`$t0`(built)
for the masked-a3 copy).

**Hoist-both-before-either: not applicable.** This function's residue is
not a swapped-adjacent-load-pair shape; `asm-differ` shows every remaining
diff is a same-opcode register rename (`move`/`andi`/`or`), never a
reordered pair of loads or multiplies. There is no adjacent load/mult
shape here for the lever to act on.

**Traced a genuinely new finding while re-reading the disassembly: retail's
delay-slot `move $a3,$a2` (in `beqz $v0,.L800345FC`'s delay slot, i.e. the
`flag==0` early-return branch) executes UNCONDITIONALLY (delay slots always
execute), so on the fallthrough (`flag!=0`) path retail's register `$a3`
holds parameter `a2`'s original value for the REST of the function --
including at the two `andi a3,a3,0xff` sites (0x24DB8, 0x24DF8) that feed
the masked byte argument to `SpuVmKeyOn`/`SpuVmKeyOff`.** This raised
the hypothesis that retail's actual passed value at those two call sites is
`(u8)a2`, not `(u8)a3` as currently modeled. **Tested directly and
REJECTED**: replacing both `(u8)a3` call arguments with `(u8)a2` regressed
to 59/70 (worse, and it touched many more words than the two call sites,
confirming a2's reuse there would cascade register pressure differently
than retail's actual shape) -- reverted immediately. The `move $a3,$a2` is
therefore reconfirmed as this report's own already-documented genuinely-
dead delay-slot filler (retail's `andi a3,a3,0xff` at both sites must
therefore be operating on `$a3`'s TRUE value in retail's own allocation,
meaning retail's mapping of "which physical register holds which C value"
differs from this build's in a way that is NOT reproducible by changing
which C value is read -- consistent with the register-identity-only
classification already on file, not a semantic mis-transcription).

**Second reshape tried**: an explicit `s16 ch = a0;` local, used for both
`packed = (a1 << 8) | ch` sites in place of `a0` directly (mirroring
`Snd_setVabAttr`'s round-39 "fresh local" experiment on its own channel/arg5
swap). **Inert** -- byte-identical output, 61/70 unchanged.

**Third reshape tried**: an unconditional dead `a2 = a3;` statement placed
right after the existing `speed = a3;` line (before the `flag == 0` check),
to see whether reproducing the delay slot's "always executes" semantics
as an explicit statement (rather than round 31's inside-the-if attempt)
would shift anything now that round 32's `speed = a3` fix is in place.
**Inert** -- byte-identical, 61/70 unchanged. GCC 2.6.3 eliminates the
dead assignment to the now-unused parameter regardless of where it is
placed relative to the branch.

Three independent negative results this round (two new reshapes plus the
hoist-both non-applicability check) all consistent with the existing
register-identity STALL classification. `INCLUDE_ASM` unchanged,
`build-and-verify.sh` confirmed byte-exact after every experiment.

## Round 46 update (runner charlie): declare-then-assign split of round 39's `ch` local, tried and INERT

Rebuilt round 32/39's preserved body: reproduces exactly, 61/70, length
exact, the same `$t0`/`$a2` and `$s1`/`$t0` two-way register rotation.

Round 39 tried an explicit `s16 ch = a0;` (combined declare+init) used at
both `packed = (a1 << 8) | ch` sites in place of `a0` -- byte-identical,
no change. This session found a NEW axis on a different function
(`GetCdFileEntry.md`, round 46): the same value declared as `T x; x =
expr;` (split into two statements) instead of `T x = expr;` (combined)
can land in a different register class, closing that function outright.

**Tried here: `s16 ch;` declared with the other locals, `ch = a0;`
assigned as its own statement right after `speed = a3;`, used at both
`packed` sites in place of `a0`.** Rebuilt and measured: **61/70,
byte-identical to both the baseline and round 39's combined-form
attempt** -- no change in either direction. Reverted (`git checkout --
src/psyq/libsnd_seqread.c`), `build-and-verify.sh` reconfirmed byte-exact.

Combined with the `Snd_setVabAttr` round-46 result (same split-lever
experiment, there actively REGRESSIVE), this function's result shows the
lever's effect is not even consistently neutral-vs-harmful across
functions -- here it is fully inert, there it cost 10 words. The
discriminator from `GetCdFileEntry` (a value crossing a call boundary,
timing already independently confirmed correct) is not present in either
of these two functions' residues, which is consistent with the lever not
applying to either. Not attempting further variations of this lever on
this function; the register-identity STALL classification stands.

## Round 47 (runner echo): permuter search on the CURRENT 61/70 body -- real +1 word (61/70 -> 62/70), no zero

This function's earlier permuter history (round 32) searched the OLD
44/70 body before the "reuse dying `speed`" lever was found; the CURRENT
61/70 body (round 32's own result, re-verified rounds 39/46) had never
itself been scaffolded or searched. Flagged this round as a genuinely
fresh search target for exactly that reason.

**All three pre-checks run, in order:**
- **(a)** Scaffold built from the current 61/70 body (`tools/setup-
  permuter.sh NoteOn <seed>`); base compiles.
- **(b)** `--debug --stack-diffs`: base score 50, **`Insertions: 0`,
  `Deletions: 0`, Register Differences: 10, Reorderings: 0** -- a clean
  0/0, matching this report's own characterization of the residue as a
  "clean two-way register rotation" exactly.
- **(c)** This is the textbook AGREE case per this round's discriminator:
  0/0 in the scaffold, and the real build's own residue (already
  established across rounds 32/39/46 as pure register renames, zero
  drift) is also 0/0. Searched with confidence.

**Search: `timeout 600 ... -j 8 --stop-on-zero --best-only`, confirmed
exit via the 600s timeout.** ~91000+ iterations. One output directory
(`output-45-1`, score 45 down from base 50) -- read its `score.txt` and
`diff.txt`; no zero.

**The winning mutation:** wrap the `flag == 0` early return in
`do { return; } while (0);` instead of a bare `return;`. This LOOKS like
pure syntactic noise (semantically identical to a bare `return`), and the
report almost dismissed it as such without checking -- but the real
oracle disagrees: dropped into `src/psyq/libsnd_seqread.c` and rebuilt,
**`funcdiff.py` reports 62/70, one MORE real word than the 61/70 baseline,
with `build-and-verify.sh` showing zero drift outside this function's own
range.** `asm-differ` confirms the residue shrank from 9 words to 8: the
`$s0`/`$s1` swap this report's round-32 update already closed stays
closed, and one of the two remaining rotations (`$s1`(retail)<->`$t0`
(built) for the masked-a3 copy) partially resolves -- retail's `$a3` now
matches directly at one of its two use sites (`24da0`), leaving a
narrower residue than before (see `asm-differ` output: the `$t0`/`$a2`
a0-copy rotation and one `$a3`/`$t0` site remain, down from three
distinct rotations).

**Kept as the new best (62/70).** `INCLUDE_ASM` restored; this body (with
the `do { return; } while (0);` wrapper) is what `src/psyq/libsnd_seqread.c`
preserves in `#if 0` now.

### Proposed learning

**Wrapping a single-statement early return in `do { ... } while (0)` is
not always inert, even though it is semantically a no-op.** GCC 2.6.3
apparently gives a `do`/`while(0)`-wrapped `return` a different internal
basic-block shape than a bare `return` inside an `if`, and that shape
difference can change register allocation elsewhere in the SAME function
(here: which physical register survives past the branch to be reused for
a later masked-byte copy). The permuter found this by syntactic accident
(scoring it as a tiny local improvement, 50->45, that looked like it
might just be diff-algorithm noise); the lesson is to verify EVERY
permuter candidate through the real oracle before dismissing it as
noise, even ones that read as obviously-equivalent C -- this project's
"a permuter score is a LEAD, never a RESULT" caution cuts both ways: a
suspicious-looking transform can still be a real lever, not just a
suspicious-looking non-transform can be a fake one. Worth trying on
other single-statement early-return residues in this unit before
assuming a bare `return` is always neutral.

## Round 48 update (runner charlie): fresh permuter search against the CURRENT (round 47) 62/70 body -- not closed in 81231 iterations under load (rc=124)

Rebuilt round 47's preserved body in isolation first: `build exit=2`, no
compile errors, `funcdiff.py` reports 62/70, no staleness warning, length
exact (70/70) -- matches the title exactly.

**All three checks run, in order, per this round's broadcast discriminator:**

- **(a)** Built a fresh minimal scaffold (`tools/setup-permuter.sh`) from
  the current 62/70 body -- the FIRST scaffold built against this specific
  body (round 47's own search was against this same body, but round 47's
  scaffold predates the do-while(0) win being folded back in cleanly as a
  standalone scaffold -- re-scaffolded from scratch this round to confirm).
  Base compiles clean.
- **(b)** `--debug --stack-diffs`: **base score 45** -- `Stack Differences: 0`,
  `Branch Differences: 0`, `Register Differences: 9`, `Reorderings: 0`,
  `Insertions: 0`, `Deletions: 0`. A clean pure-register-rotation signature,
  matching this report's own residue description exactly (register-identity
  only, no drift).
- **(c)** This is the AGREE case: the scaffold's 0/0 insertions/deletions
  matches the real build's own zero-drift, exact-length 62/70 residue.
  Searched with confidence.

**Search: `timeout 900 ... -j 6 --stop-on-zero --best-only`, confirmed exit
via the 900s bound (`ps` polling; the process was gone at the next check,
consistent with the timeout firing rather than an external kill).
81231 iterations** (machine was under load this round -- 4 runners searching
simultaneously, measured load average ~38 on a 32-core box, so the true
per-core rate was roughly a third of idle). **No zero, and no output
directory was ever created** (`--best-only` only writes when a candidate
beats the held best; the held best remained the base score of 45 for the
entire run) -- i.e. not even a single sub-45 candidate was found this time,
unlike round 47's own search against an earlier scaffold framing which did
find a real (if small) improvement early on.

**Not marking permuter-exhausted**, per this round's explicit standing rule
against upgrading a bounded negative -- phrased as: not closed in 81231
iterations under load (rc=124, own bound). A different seed framing (this
report already has two: round 32's old-44/70-body framing which found the
`speed=a3` reuse lever, and round 47's post-fix framing which found the
`do-while(0)` lever) might still find something; this round's search, run
against what should be the SAME logical scaffold as round 47's, simply
did not reproduce a further improvement within budget.

`INCLUDE_ASM` unchanged (no source edit made this round beyond the
scaffold, which lives outside `src/`), `build-and-verify.sh` confirmed
byte-exact throughout (no `src/` change to verify against).

### Round 48 disposition

Still 62/70, length exact, 8-word residue (two-way register rotation,
`$t0`/`$a2` a0-copy + one `$a3`/`$t0` site, per round 47's own
`asm-differ` reading). Treating as **spent for this round's search
budget**; not a certificate against a future search with a different seed
shape (per this project's standing rule that a permuter negative is
evidence about ONE search, not the function).

## Round 49 update (runner charlie): fresh 900s search (35666 iterations, rc=124, no zero); round-32 dead-parameter negative re-checked against the CURRENT do-while(0) state, still NEGATIVE

Rebuilt round 47/48's preserved 62/70 body in isolation first: `build
exit=2`, no compile errors, `funcdiff.py` reports 62/70, no staleness
warning, length exact -- matches the title exactly.

**Check 3, run fresh:** new scaffold from the current 62/70 body; base
compiles. `--debug --stack-diffs`: **base score 45** -- `Register
Differences: 9`, everything else 0. Exact match to round 48's own
scaffold figures. AGREE case; searched with confidence.

**Search: `timeout 900 ... -j 4 --stop-on-zero --best-only`, confirmed
exit via the 900s bound (rc=124, own file). 35666 iterations** (this
round ran with more contention than round 48's own 81231-iteration run
at `-j 6` -- lower `-j` chosen deliberately to leave headroom for a
concurrent search this round on `SeqPlay`, see that report). **No
output directory was created** -- the held best remained the base score
of 45 for the entire run, i.e. not even a single sub-45 candidate was
found this time, matching round 48's own outcome exactly.

**Separately, applied this round's "a recorded negative is scoped to the
state it was measured in" discipline (head's mid-round broadcast) to
round 32's own on-file negative for this function.** Round 32 tried
reusing the genuinely-dead 3rd parameter `a2` to carry `a0` across the
two `packed = (a1 << 8) | a0` sites (`a2 = a0;` at top, `(s16)a2` in
place of `a0` at both sites) and got 60/70 -- one word worse than the
61/70 baseline of the time. That test predates round 47's `do { return; }
while (0)` rewrite, which changed this function's compiled shape (61/70
-> 62/70) and is exactly the kind of "something else changed since"
condition the discipline asks to re-check for. **Re-applied the identical
`a2 = a0;` / `(s16)a2` substitution to the CURRENT (post-do-while) body
and rebuilt: 26/70, with 262999 bytes of whole-image drift and the
compiled length itself changed** -- a much harder regression than round
32's own 60/70 finding against the older body, not an improvement
unlocked by the intervening state change. Reverted immediately;
`build-and-verify.sh` reconfirmed byte-exact after revert. This
particular lever is now confirmed inert-or-worse independent of the
do-while(0) state; the "negative scoped to state" discipline is worth
checking but does not automatically rescue every old negative.

**Disposition:** still 62/70, length exact, same 8-word two-way register
rotation. Two independent full-budget searches (round 48: 81231
iterations at `-j 6`; this round: 35666 iterations at `-j 4`) against the
same post-round-47 scaffold have now found nothing beyond the base score,
and the one plausible untried hand-lever from the function's own history
reconfirms negative under the current state. `INCLUDE_ASM` unchanged,
`build-and-verify.sh` confirmed byte-exact throughout.

### Proposed learning

The head's round-49 "a recorded negative is scoped to the state it was
measured in" discipline is worth applying systematically, but it is a
lead to CHECK, not a lever to trust -- of the two functions in this unit
where an old negative was re-tried against a since-changed state this
round (this one, and `SeqPlay`'s duplicate-branch permuter
candidates), both re-confirmed negative rather than being unlocked. The
discipline earns its keep by ruling things back in cheaply, not by
guaranteeing they will move.

**NON_MATCHING body promoted, round 66** (runner charlie): permuter-candidate provenance (round 47 `do { return; } while (0)` mutation) reviewed for semantics -- behaviorally identical to the bare `return;` it replaces, no UB, no dead branch. Promoted.

## Round 97 types pass (echo)

code_179d8_k's local `Entry90902E8` view retired onto `include/SsScore.h`:
the same 0xAC-byte (`SS_SEQ_TABSIZ`) `_ss_score[access][seq]` record that
libsnd_cres, libsnd_decre and libsnd_vmanager already use. The header gained
this unit's fields by splitting padding (no offset, size or existing type
moved); field names stay offset-only (`unkNN`) as the header's convention for
Sony-only fields, with each one's mechanics in its comment. The unit's
`(u8 *)rec + unk12 + 0x17/0x2C` and `(s16 *)((u8 *)rec + 0x4E + ch * 2)`
arithmetic became the header's per-channel arrays `unk17[16]` (pan),
`unk2C[16]` (program) and `unk4E[16]` (volume): `unk12` is the event's MIDI
channel (GetSeqData stores a status byte's low nibble), not a byte offset to
an "embedded state block" as the old local comment read it. Byte-exact
unchanged; the NON_MATCHING object is identical too (objdump of
`build/nonmatching/src/code_179d8_k.c.o` before/after).

`unk74`, settled: the old local view called it a "nonzero-gated dispatch
enable flag". It is the sequence's left volume (`SpuVmSetSeqVol` stores voll
there, clamped to 0x7F; `SpuVmSetVol` scales by it / 127). NoteOn's only use
is `lhu 0x74` then `beqz` to the return, ahead of both the key-on and key-off
arms, so a sequence whose left volume is 0 plays no note events. That is one
use of the volume, not a second meaning; SsScore.h's comment now says both.
No reader needed a signedness change (`lhu` here, `u16` in the header).

The old local comment's other readings, kept for the record: `unk88` "a
scratch slot every function stores its last computed value into, genuinely
overloaded" -- it is the ticks to the next event (every handler stores
ReadDeltaValue's result; SeqPlay counts it down); `unk4C` read as a "note"
at the SpuVm* call sites -- it is the VAB id (`SsUtGetProgAtr(vabId, prog,
...)`, and CC0 bank select stores it); the 0x2C byte read as "vol" -- it is
the channel's program (SetProgramChange stores it).
