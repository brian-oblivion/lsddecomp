# Class86F88__Class86F88 -- MATCHED (107/107, round 73)

> Renamed from `Class86F88_3bb8c_j__Class86F88_3bb8c_j` on 2026-09-24 (tools/rename.py). Address 0x80051ac8.

> Renamed from `func_80051AC8` on 2026-09-24 (tools/rename.py). Address 0x80051ac8.

> **ROUND 19 (bravo) UPDATE.** Found and fixed a genuine, INDEPENDENT
> correctness bug in the preserved body, unrelated to the register-identity
> class: `len = (len + ((u32)len >> 31)) >> 1;` promotes the WHOLE
> expression to `u32` (C's usual arithmetic conversions, triggered by the
> `(u32)` cast on one addend), so the outer `>> 1` compiled to `srl`
> (logical) where retail has `sra` (arithmetic) -- confirmed by direct
> `objdump` comparison, independent of the funcdiff word score. Fixed with
> an explicit cast back to signed before the final shift:
> `len = (s32)(len + ((u32)len >> 31)) >> 1;`. This is the standard
> truncating-divide-by-2 idiom already documented elsewhere in this
> project (`Class6D3C8__SetDayFromTickCount`'s family) -- worth a general note: **the cast
> that makes the sign-bit extraction well-defined can silently poison the
> whole expression's signedness if not scoped tightly.**
>
> Also matched two more of retail's exact scheduling choices, found by
> reading `asm-differ` past the register-rotation noise:
> - The SECOND `BMemPMgrAlloc(...)` allocation call's size argument is
>   `self->unk10 * 4` (a fresh reload from the struct field), not
>   `count * 4` (the cached local) -- asymmetric with the FIRST call,
>   which does use the register directly. Retail's own asm shows this
>   exact asymmetry (`sll a0,s4,2` for the first call vs `lw a0,0x10(s2)`
>   then `sll a0,a0,2` for the second).
> - The `p = arg1;` cursor reset for the second (fill) loop happens
>   UNCONDITIONALLY, positioned between the size-reload and the
>   `self->unk18 = ...` assignment -- BEFORE the `if (count > 0)` guard is
>   even tested, not inside it. Moving the reset there (out of the
>   conditional block) reproduced retail's structural placement.
>
> Neither fix moved the funcdiff word count (stayed nominally 6-8/107,
> fluctuating slightly with each change, still 2 words longer than
> retail's 107 -- `objdump` confirms `0x1b4` = 109 words compiled either
> way) -- **the remaining gap is the SAME register-identity rotation round
> 9 already diagnosed and round 13 confirmed inert to declaration order.**
> The word-count not moving despite two confirmed real structural matches
> is itself a data point: this residue's SCORE is dominated entirely by
> the register rotation, and structural/scheduling fixes underneath it are
> invisible to a raw word-count diff until the rotation itself closes.
> Restored to `INCLUDE_ASM` (see updated preserved body at the bottom of
> this file); the two structural matches and the bug fix are kept in the
> preserved body for whoever resumes this function.
>
> **Not re-attempted this round: round 9's suggested next step** ("derive
> retail's register-to-value mapping, match first-use order to the
> prologue's `sw` order"). This round's own brief independently confirms
> declaration/introduction order is inert across 10 attempts and 4
> functions now (including this one's own two prior tests) -- that
> suggested next step is a variant of the same inert axis and was not
> re-run given that finding.

---

Unit: `src/class_3bb8c_j.c`. `self` is `Class86F88_3bb8c_j`. This is `Class86F88_3bb8c_j`'s
own constructor -- the occupant of `Class86F88Methods_3bb8c_j::ctor` (+0x008),
reached indirectly by `New_Class86F88`
(`GetClass86F88Methods()->ctor(self, arg0, arg1)`).

## Class identity (ROUND 75 correction -- read this before the rest of the file)

This unit's local type for `self` was originally named `Class86ED0` and
described as having vtable `D_80086ED0`. Both were wrong, and the error
predates this round: `include/class_3bb8c.h`'s own round-15 HEAD NOTEs
(search "D_80086ED0 and gClass86F88Methods") already documented it and deferred
the fix. Settled by address, not by guess:

- `func_80051A4C` (this unit, now named `Get_vtable_Obj86ED0`) returns
  `&D_80086ED0` directly -- but `D_80086ED0` is `Obj86ED0`'s OWN table
  (42 slots, `tools/classtable.py D_80086ED0`), a DIFFERENT class
  established independently by class_3bb8c_i. It has NOTHING to do with
  this constructor's class.
- This class's REAL table is `gClass86F88Methods`, reached through
  `GetClass86F88Methods()` (class_3bb8c_k, MATCHED) -- `tools/classtable.py
  gClass86F88Methods` places every one of this unit's remaining functions
  (this ctor at +0x008, plus `Class86F88__Finalize`/`AddChild`/
  `RemoveChild`/`RemoveAllChildren`/`NotifyChild`/`ResetCounters`/
  `LoadResources`/`ReleaseResources`/`AddChildAndSetState`/
  `RemoveCachedChildren` at +0x00C/+0x010/+0x014/+0x018/+0x038/+0x040/
  +0x044/+0x048/+0x04C/+0x050) at those exact slots. So the identity IS
  resolvable by `classtable.py` after all -- the earlier "no classtable
  entry exists for it" note was itself part of the same mistake (it
  looked up the wrong global).
- class_3bb8c_k's OWN local view of this same gClass86F88Methods table already
  carries the name `Class86F88`/`Class86F88Methods` in the shared header,
  established independently from ITS OWN call sites. Reusing that bare
  name here would collide (both visible in this translation unit through
  the shared header), so this unit keeps its own, now-correctly-targeted
  local name: `Class86F88_3bb8c_j`/`Class86F88Methods_3bb8c_j`.

Nothing about the matched BYTES was ever affected by any of this (a type
name is not codegen) -- `build-and-verify.sh` and `tools/check-nonmatching.sh`
stayed green throughout. Only the class attribution and `self`'s type
name were wrong. See `src/class_3bb8c_j.c`'s file header comment for the
short version, and `Get_vtable_Obj86ED0.md` for the getter-side half of
this same correction.

## Semantics (established with reasonable confidence from the disassembly)

`arg1` is a NUL-terminated array of string/opaque pointers; `arg2` is a
mode flag (0 or 1), also stashed into `self->unkC`.

1. Chain the base ctor: `Get_vtable_BasicClass()->ctor(self);` then
   `self->methods = GetClass86F88Methods();` (this really is `Class86F88Methods_3bb8c_j
   *GetClass86F88Methods(void)` -- NOT a separate "ctor table" type: this same
   getter is what `New_Class86F88` dereferences `->ctor` on, and here its
   return is assigned DIRECTLY as `self->methods`, so both call sites
   type-check against one declared return type).
2. Count `arg1`'s entries (walk until a NULL pointer) -> `count`.
3. `self->unk10 = count`; allocate two `count`-sized arrays via
   `BMemPMgrAlloc`: `self->unk18` (one individually-allocated buffer per
   entry) and `self->unk1C` (a flat `s32[count]` of per-entry lengths --
   NOT individually-allocated; freed as ONE block by the already-matched
   `Class86F88__Finalize`, which is what fixed this typing).
4. If `count > 0`: `self->unk14 = 0` (running max, see below), then for
   each entry `i`: `len = func_80013348(arg1[i])` (a string-length-ish
   helper; when `arg2 == 1`, `len` is HALVED via the standard
   truncating-division-by-2 idiom, `(len + ((u32)len >> 31)) >> 1`);
   `self->unk1C[i] = len`; `self->unk18[i] = BMemPMgrAlloc(len + 4)`;
   fill it via `DecodeFullWidthSjis` (arg2==1) or `strcpy` (otherwise) from
   `arg1[i]`; update `self->unk14` to the running max of `len`.
5. `self->unkC = arg2; Class86F88__ClearCachedRefs(self);` (the already-matched
   3-field reset) `; self->methods->slot40(self);`.

## Best body reached (round 19: nominal 6/107, but with a real bug fixed and
## two more scheduling matches than round 9's version -- see the update at
## the top of this file for why the word count did not move)

```c
#if 0
/* stalesyms --fix 2026-09-22: func_80013348 -> strlen -- names retrofitted so this body links as written; the residue it recorded is unverified until rebuilt. */
void Class86F88__Class86F88(Class86F88_3bb8c_j *self, void **arg1, s32 arg2)
{
    void **p;
    s32 count;
    s32 index;
    s32 len;

    Get_vtable_BasicClass()->ctor(self);
    self->methods = GetClass86F88Methods();

    count = 0;
    for (p = arg1; *p != NULL; p++) {
        count++;
    }

    self->unk10 = count;
    self->unk18 = BMemPMgrAlloc(count * 4);
    p = arg1;
    self->unk1C = BMemPMgrAlloc(self->unk10 * 4);

    if (count > 0) {
        self->unk14 = 0;
        for (index = 0; index < self->unk10; index++) {
            len = strlen(*p);
            if (arg2 == 1) {
                len = (s32)(len + ((u32)len >> 31)) >> 1;
            }
            self->unk1C[index] = len;
            self->unk18[index] = BMemPMgrAlloc(len + 4);
            if (arg2 == 1) {
                DecodeFullWidthSjis(self->unk18[index], *p);
            } else {
                strcpy(self->unk18[index], *p);
            }
            if (self->unk14 < len) {
                self->unk14 = len;
            }
            p++;
        }
    }

    self->unkC = arg2;
    Class86F88__ClearCachedRefs(self);
    self->methods->slot40(self);
}
#endif
```

Round 9's earlier body (kept for the record -- superseded by the above):
the `max = self->unk14; self->unk14 = max; if (max < len) ...` dance was
an attempt to model a running max via a LOCAL variable; retail never
caches it in a register at all, re-reading and re-writing
`self->unk14` directly each iteration (`lw v0,0x14(s2)` / `slt` /
conditional `sw`) -- the plain `if (self->unk14 < len) self->unk14 = len;`
above reproduces that shape with no local needed.

Declarations it needs (all already live in `src/class_3bb8c_j.c`'s
`Class86F88_3bb8c_j`/`Class86F88Methods_3bb8c_j` definitions, plus these locally-scoped
externs which were removed when the function was restored to
`INCLUDE_ASM` -- re-add if resuming):

```c
/* stalesyms --fix 2026-09-22: func_80013348 -> strlen -- names retrofitted so this body links as written; the residue it recorded is unverified until rebuilt. */
extern s32 strlen(void *arg0);
extern void DecodeFullWidthSjis(void *dst, void *src);
extern char *strcpy(char *dest, char *src);
```

## Residue class: REGISTER IDENTITY, not instruction count

The compiled length was chased down to EXACTLY 107 words (matching
retail) after two rounds of reshaping, but the REGISTERS retail assigns
to `self`/`arg1`/the counting-vs-filling cursor/`count`-then-`index`
are consistently different from what this body produces, cascading
through the whole function:

- Retail keeps the `arg1` PARAMETER itself alive in one register (`$s0`)
  for the WHOLE function, and uses a completely separate cursor register
  (`$s3`) for BOTH the counting walk and (freshly reset from `$s0`) the
  filling walk. My first attempt aliased `p = arg1;` at declaration,
  which let GCC allocate `p` into the position retail reserves for
  `arg1` itself, demoting `arg1` to a different register than retail's
  -- worth 2 words on its own (score 9/107, "differs outside range"
  ~116KB).
- Removing that alias (declaring `p` separately and writing
  `for (p = arg1; ...)`) fixed the register-vs-value mapping for the
  FIRST loop but not fully: mine still allocates a DIFFERENT overall
  rotation (`arg1`->`$s4`, cursor->`$s3`, later reused as `$s4`=index)
  than retail's (`arg1`->`$s0` permanently, `count`/`index`->`$s4`
  exclusively, never aliased with `arg1`). This closed one word (10/107)
  but the register IDENTITY mismatch remains pervasive from word ~80
  onward (everything after the `self->unk14` running-max block).
- A third attempt repositioning the cursor-reset statement (moving
  `p = arg1;` to between the two `BMemPMgrAlloc` allocation calls, to
  match retail's own odd mid-allocation-sequence placement of
  `addu s3,s0,zero`) made it WORSE (6/107), confirming the position
  alone isn't the lever either.

## What was tried and the permuter's result

A 4-minute permuter search (`-j 6 --stop-on-zero`, base score 2050 --
22 register diffs, 4 reorderings, 10 insertions, 7 deletions per
`--debug`) found no zero; best reached was 1490 after ~2000 iterations,
nowhere close. This function's search space (7 local values needing a
specific register rotation across ~30 live instructions) appears to be
beyond what an unguided permuter search closes quickly, unlike the
smaller single-residue functions in this same unit
(`Obj86ED0__ResetAllAndFinish`, `Class86F88__AddChildAndSetState`) where 14-5000 iterations sufficed.

**Not re-staffed without a plan beyond "try more permuter time" or "try
every p/q aliasing permutation by hand."** A structured next step: derive
retail's exact register-to-value mapping first (already partially done
above), then hand-write the C so each VALUE's first-use ORDER in the
source matches the order retail's `sw`s in the prologue save them
(`$s2`=self, `$s0`=arg1, `$s5`=arg2, `$s4`=count, `$s3`=cursor,
`$s1`=len/index, `$s6`=const 1) -- i.e. treat the prologue save order as
a proxy for GCC 2.6.3's live-range-introduction order and try to
reproduce THAT sequence of first-uses in the C, rather than reshaping by
trial and error against the funcdiff score alone.

### Proposed learning

A function whose funcdiff residue is "correct total length, wrong
registers throughout, cascading from partway through the body" is
categorically different from an entangled multi-defect residue
(func_8005F544's class) or a genuine one-instruction residue
(Obj86ED0__ResetAllAndFinish's class) -- it is the project's established "register
saturation" pattern (`func_8003D73C`) but WITHOUT retail saturating all
9 callee-saved registers (retail here uses only 7 of 9: s0-s6), so the
`grep -oE 'sw +\$s[0-9]'` == 8/9 predictor does not catch it. Consider
extending that predictor's write-up to note that a HIGH but
sub-saturated s-register count (6-7) is also a register-identity risk
factor when the function has several independent long-lived values
(here: self, arg1, arg2, a running count/index, a cursor, and a
per-entry computed value) -- the exact count of live values, not just
the callee-saved register COUNT, seems to be what predicts this.

---

## Round 73 (charlie) — NON_MATCHING body promoted

Track 1b: the round-19 preserved body above (with the `srl`/`sra` fix and
the two scheduling matches) is now live in `src/class_3bb8c_j.c` under
`#ifdef NON_MATCHING`, with the verified build still taking the `#else`
`INCLUDE_ASM` branch. `./build-and-verify.sh` stayed green (no bytes
changed) and `tools/check-nonmatching.sh` compiles and link-resolves it.
Hand-derived, not a permuter candidate -- this is the same body already
reviewed across rounds 9, 13 and 19 above.

NON_MATCHING body promoted, round 73

This does not retire the still-outstanding track-1 REVISIT.

---

## Round 73 (delta) -- REVISIT: MATCHED 107/107

**Preserved body rebuilt first, as given in the `#ifdef NON_MATCHING` block
(compiled live):** 6/107 nominal, `insertions 8 / deletions 8`, positional
skeleton diffs 101. objdump size 0x1B4 = 109 words (2 long), 116318 bytes
differing outside the range. That matches the round-19 figures, so the body
was faithful.

**The residue was never register identity.** It was three control-flow
and source-shape differences. They produced the wrong length, and the
length shifted everything after it, which read as a register rotation.
Read retail's asm directly:

1. **No `if (count > 0)` guard.** Retail's `blez v1` tests `self->unk10`
   (the rotated `for` loop's entry test), and `sw zero,0x14(s2)` sits in its
   delay slot, so `self->unk14 = 0` is unconditional before the loop.
2. **The running max is a ternary, not an `if`.** Retail's
   `lw v0,0x14(s2); sw v0,0x14(s2); slt; beqz; sw s1,0x14(s2)` stores the
   old value back unconditionally. `if (self->unk14 < len) self->unk14 = len;`
   omits that store (one word short).
   `self->unk14 = (self->unk14 < len) ? len : self->unk14;` emits it.
3. **`i = 0; p = arg1;` come BEFORE the BasicClass ctor call.** Retail
   schedules `move s4,zero` / `move s3,s0` into the prologue, between the
   register saves.

Also: the counting loop is `while (*p++ != NULL) i++;` (retail's `addiu
s3,s3,4` sits in the `bnez` delay slot and runs on exit too), count and
index are ONE variable `i` (both live in `$s4`), and the halving is plain
`len /= 2` (s32), which emits the `srl 31 / addu / sra` sequence directly.

Builds, all through `./build-and-verify.sh`:

| # | variant | funcdiff |
| --- | --- | --- |
| 1 | preserved body verbatim | 6/107, ins/del 8/8, skel 101, 109 words |
| 2 | retail-shaped: no `count>0` guard, `unk14 = 0` before the loop, `while (*p++)`, single `i`, `len /= 2`, `if`-max | 66/107, 5/5, skel 39, 106 words |
| 3 | 2 + ternary max | 98/107, 4/4, skel 9, length exact |
| 4 | 3 + `i = 0; p = arg1;` moved above the ctor call | **107/107, 0/0, whole image OK** |

Levers from this round's brief, checked:
- Call arity: not relevant. `BMemPMgrAlloc`, `strlen`, `strcpy`,
  `DecodeFullWidthSjis` and the ctor/`slot40` calls all write exactly
  the registers their declarations name.
- Dead-parameter reuse: not applied. No parameter is reassigned.
  `$s0` (arg1) is reused by GCC for `i*4` after arg1's last read, which is
  ordinary global-alloc sharing of disjoint pseudos.
- Struct copy / global-alloc priority: not relevant; the residue was
  structural.

REVISITED, round 73: MATCHED 107/107 (control-flow shape: unconditional `unk14 = 0`, ternary max, init before the ctor call); names/types not relevant

### Proposed learning

**A 2-word length error under a "register-identity rotation" verdict is
not a register residue.** Rounds 9, 13 and 19 filed this as rotation. The
6/107 figure came from the length defect's address drift. Once the
length was exact the registers were already right. Discriminator: objdump
size != the `.s`'s declared size means read the control flow first.

**`x = (x < y) ? y : x;` on a memory lvalue emits an unconditional
store-back of the old value** (`lw; sw; slt; beqz; sw`). The `if` form
omits it. Tell: a load of a field immediately stored back to the same
field. (Class86F88__Class86F88, one word, 66 -> 98/107.)

## Naming

- `Class86F88__Class86F88` -- tier A. The ctor occupant (classtable.py gClass86F88Methods +0x008), named per the "constructors Class__Class" convention (e.g. Class869D8__Class869D8). Parses arg1 as a NUL-terminated pointer array and allocates two parallel unk10-length arrays -- mechanics well established via asm-differ across rounds 9/13/19/73.
