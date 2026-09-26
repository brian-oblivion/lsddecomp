# Class866E8__LoadElementResources -- MATCHED round 73 (150/150, exact length, whole-image SHA1 green)

> Renamed from `func_8004BE54` on 2026-09-24 (tools/rename.py). Address 0x8004be54.

REVISITED, round 73: MATCHED 150/150 on the first changed build (split the twice-assigned `info` local); names/types not relevant (existing local views reused unchanged)

## Round 73 (bravo) -- revisit, MATCHED

**Preserved body rebuilt first, unchanged from the `#if 0` block:** 142/150,
`insertions 1 / deletions 1`, positional skeleton diffs 8, exact length.
Residue exactly as filed: retail loads `info` (`hdr->field10`) into `$v0`
at both 0x8004BE84 and 0x8004BEC8 and reuses `$v0` for the sum; the build
put `info` in `$a1` both times.

**Lever: split a variable reused for two values (delta's round-73
`Class86F88__LoadResources` lever).** The body had ONE `info` local assigned twice from
the same expression, on either side of the `res->methods->slot4(res)` call.
Adding `ResInfo866E8 *info2;` and using it for the second assignment
(`info2 = hdr->field10; req.field0 = (s32)info2 + info2->unk4 +
info2->unk8;`) gave **150/150, ins/del 0/0, `build exit=0`, OK: build
matches retail** on the first build.

Checked hypothesis for the cause: as one pseudo, `info` is live in two
basic blocks, so it goes to global-alloc, which gave it `$a1`; split, each
piece lives in one block and local-alloc ties it to the `addu` that
consumes it, i.e. `$v0`. The eight words that survived three structural
axes (reorder, hoist, drop `hdr`) and a 34k permuter search were all this
one allocation decision.

Also measured: removing the existing `__asm__("")` barrier after
`gpu = (*slot)->unk14;` regresses to 145/150 (the three outBuf loads
schedule above the `unk14` load), so it stays. It is an ordering barrier
only, allowed by HARD RULE 6.

Levers tried, one per line:
- split `info` into `info`/`info2`: 150/150 (MATCH)
- drop the `__asm__("")` barrier on the matched body: 145/150 (reverted)

### Proposed learning

A local ASSIGNED TWICE FROM THE SAME EXPRESSION on either side of a call,
where retail puts both loads in the register that then holds the result
(`lw v0,F(s); lw v1,4(v0); addu v0,v0,v1`), is two variables in the source.
One name makes a multi-block pseudo that global-alloc places in a free
argument register. Same mechanism as delta's `Class86F88__LoadResources`, but here in
a leaf-like straight-line prologue with no loop and no callee-saved
rotation, so the tell is a caller-saved register mismatch instead.

## Earlier history (superseded by the match above)

#### Old title: Class866E8__LoadElementResources -- STALL, close (142/150 words; round 40 permuter-found improvement, up from 132/150)

> **ROUND 47 (charlie): Gate 1b re-verified 142/150, no drift** (rebuild
> via `make clean && make extract` then the standard `#if 0`->`#if 1`
> swap). Identical `info`/`hdr` register-identity residue at
> `0x8004BE84`-`0x8004BEDC`, unchanged from round 46.
>
> **Permuter check 3, run explicitly:** round 40's scaffold recorded
> `--debug --stack-diffs` as base score 135, 27 register differences,
> ZERO reorderings/insertions/deletions -- matching this function's own
> real-build residue description (a pure register-allocation choice, no
> instruction-shape difference) exactly. This function is NOT one of the
> five confirmed `class_3bb8c`/`Obj866E8` scaffold-mismatch cases
> (`Class866E8__ApplyRateEntries`, `Class866E8__ComputeFootprintDescriptor`, `Class866E8__SplitFootprintSlot`, `Class866E8__BuildFootprintSlots`,
> `IsPointOutOfBounds`); its 34,293-iteration search stands as a real,
> validated negative, not a voided one.
>
> Checked the 12th lever (hoist a field pair used on every path into
> locals PER `if`) against the `info`/`hdr` residue: does not apply. The
> two `info = hdr->field10;` reads sit either side of a straight-line
> `if (res != 0) { res->methods->slot4(res); }` whose own condition/body
> only touch `res`, not `info`/`hdr` -- there is no `if` whose condition
> and body BOTH read the same field pair for the lever's precondition
> to bite. **Disposition unchanged: 142/150.** No new attempt made;
> `INCLUDE_ASM` untouched throughout.

> **ROUND 46 (charlie): Gate 1b re-verified 142/150, no drift; this
> round's two new levers checked against the residue and neither applies
> -- SKIPPING without spending a new attempt.** Rebuilt the committed
> 142/150 body from a clean `INCLUDE_ASM` baseline: confirmed 142/150,
> exact length, identical `info`/`hdr` register-identity residue at
> `0x8004BE84`-`0x8004BEDC`, unchanged from round 41.
>
> **Beq/bne delay-slot-polarity lever: does not apply.** The remaining
> residue is straight-line register identity in a load/store sequence with
> no branch anywhere near it -- there is no delay slot to read.
>
> **Split-combined-declaration lever: does not apply, because it is
> already in that form.** `hdr`/`info` are already declared at the
> function's top and assigned as separate statements (`hdr = entry->unk4;`
> / `info = hdr->field10;`), not combined declare-plus-initializer -- the
> exact shape the lever asks for is what round 27's original body already
> used. Nothing to split.
>
> This residue has now survived three independent axes (round 39:
> reordering `res = target->unk2C;` to match retail's interleaving, and
> hoisting `info->unk4`; round 41: removing the `hdr` local entirely,
> regressed hard to 17/150 with drift) plus this round's check of both new
> project levers. **Disposition unchanged: 142/150, `INCLUDE_ASM`
> restored, `git diff` against the round-40 commit is empty.** Not
> re-attempted further this round -- consistent with this round's own
> guidance to skip a function whose cheap levers are demonstrably spent
> rather than re-confirm it a further time.

> **ROUND 41 (alpha): Gate 1b re-verified 142/150, no drift; one new
> structural variant tried on the remaining 8-word `info`/`hdr` residue,
> REGRESSED hard.** Rebuilt the exact preserved body from a clean
> `INCLUDE_ASM` baseline first: confirmed 142/150, exact length, identical
> residue at `0x8004BE84`-`0x8004BEDC` (retail loads `info` into `$v0`,
> this build into `$a1`), unchanged from round 40.
>
> This function was staffed as the second highest-priority "close the
> residue the lever did NOT touch" target. `hdr` is used exactly three
> times in the whole function (`hdr = entry->unk4;` then `info =
> hdr->field10;` twice), all clustered at the top -- the region the residue
> lives in. Tried dropping the `hdr` local entirely and inlining
> `entry->unk4->field10` at both use sites (a plausible register-pressure
> reduction, since it removes one live callee-saved value from the whole
> function's allocation). **Regressed hard: 17/150, with 184282 bytes of
> drift outside the function's own range** -- the compiled length itself
> shrank by a word, not just a worse register choice. Reverted immediately;
> restored to the confirmed 142/150 body with `hdr` intact, matching round
> 40's own committed form exactly (`git diff` against it is empty).
>
> **Verdict unchanged: this is the already-documented `info`/`hdr`
> register-identity chain**, confirmed by round 39 to survive both
> reordering (moving `res = target->unk2C;` to interleave with `info`'s
> load) and hoisting (`info->unk4` before the first `target->unk10`
> computation) independently, and now a THIRD axis (removing the `hdr`
> local altogether) this round -- all three inert or actively worse. Per
> project rule 6, not to be forced with a register pin. Not attempted
> further; time went to `Class866E8__FindElementForPosition`'s fresh permuter search instead
> (a one-word residue with a real prior signal is a better use of a
> bounded permuter run than a fourth structural probe of an
> already-triple-confirmed register-identity chain).

> **ROUND 40 (bravo): 132/150 -> 142/150, first-ever permuter search on
> this function.** Staffed here specifically because the function had
> never been searched.
>
> **Gate 1b:** rebuilt the exact preserved 132/150 body from a clean
> `INCLUDE_ASM` baseline first -- confirmed 132/150, no drift, identical
> residue. Honest.
>
> **Scaffold:** `--debug --stack-diffs` reported base score 135, 27
> register differences, 0 reorderings/insertions/deletions -- matching
> this report's own description (a systemic register-allocation choice
> rooted at `info`'s first load, no instruction-shape differences).
> Trusted.
>
> **Search:** `timeout 900 ... -j 4 --stack-diffs --stop-on-zero
> --best-only`, backgrounded. **34293 iterations. No `rc` captured** -- the
> same "wrapping shell torn down before the trailing echo runs" trap as
> this round's `Class866E8__BuildRateEntries` search (and round 17's `Class866E8__ApplyRateEntries`
> before it). No permuter workers remained in the process list once
> checked, consistent with the 900s bound having fired; recorded as
> uncaptured rather than inferred as `124`.
>
> **The lead:** the best candidate improved in four steps (135 -> 85 -> 75
> -> 70 -> 65, the last improvement at iteration 27227) and never reached
> 0. Two changes, both kept:
> 1. The `gpu = (*slot)->unk14; gpu->unk0 = 0;` reload-then-store replaced
>    with a direct `(*slot)->unk14->unk0 = 0;` -- the SAME lever that
>    closed part of `Class866E8__BuildRateEntries`'s residue this same round (see that
>    report's proposed learning).
> 2. The `found`-path tail's combined pointer-cast-and-dereference,
>    `(*slot)->unk38 = *(EntryChildObj **)((u8 *)entry->unk10 + off2);`,
>    split into an explicit named local first: `EntryChildObj **next =
>    (EntryChildObj **)((u8 *)entry->unk10 + off2); (*slot)->unk38 =
>    *next;`.
>
> **Translated as found and re-verified through the full oracle:** isolated
> single-function test (sibling `Class866E8__BuildRateEntries` restored to `INCLUDE_ASM`
> while measuring) gives **142/150, no drift**. `asm-differ` confirms
> EVERYTHING from `0x8004BEE0` through the epilogue now matches retail
> byte-for-byte -- both fixes closed their targeted sites completely, with
> no new residue introduced anywhere else in the function. The entire
> remaining 8-word residue is the already-documented `info`/`hdr`
> register-identity chain at the very top of the function
> (`0x8004BE84`-`0x8004BEDC`), unchanged and untouched by this round's fix.
>
> This is the SECOND function in this unit this round where the permuter
> found the "redundant reload, re-derive inline instead" lever on a
> residue this report's own five-round history had filed under plain
> register identity without ever isolating this specific mechanism. See
> `Class866E8__BuildRateEntries`'s report for the shared proposed learning. **Disposition:
> 142/150, exact length, `INCLUDE_ASM` restored, preserved body updated in
> `src/class_3bb8c.c`.**

> **ROUND 39 (charlie): 130/150 -> 132/150. Both `or` sites closed by the
> hoist-lever combination; the `addu` site and the `info` register-identity
> chain did not yield to it.** Rebuilt per Gate 1b: confirmed 130/150,
> identical residue to round 32's recorded figure. This round's headline
> lever ("hoist both values before either is consumed") does not describe
> either `|=` site directly -- each is a single field reload OR'd with one
> already-live constant (`flagBit`), not two independently-loaded values --
> but `Class866E8__FindElementForPosition`'s companion finding (a residue that survives operand-
> order-alone and hoist-alone independently has not been shown to survive
> their COMBINATION) does apply, since this report already confirmed
> operand-order-alone is inert here (round 27/32) and never tried a hoist.
>
> Applied to both `(*slot)->unk10 |= flagBit;` sites (the `idxVal == -1`
> branch's copy AND the previously-flagged "second" copy in the `else`
> branch):
> ```c
> s32 t = (*slot)->unk10;
> (*slot)->unk10 = t | flagBit;
> ```
> **Both closed, one word each: 130 -> 131 (else-branch site) -> 132
> (idxVal==-1 site).** Confirmed via direct `.o` disassembly, not just the
> score: both `or` instructions now read `or v0,v0,s6` byte-identical to
> retail, matching-length, no drift introduced by the extra block-scoped
> local.
>
> The SAME hoist tried on the third residue this report documents (the
> `found`-path tail's `entry->unk10 + off2` -> `addu`) did **NOT** help:
> hoisting `entry->unk10` into a named local (`u8 *base2 = ...;`), tried
> with BOTH operand orders (`base2 + off2` and `off2 + base2`), produced the
> IDENTICAL residue in all three variants (unhoisted, hoisted-natural-order,
> hoisted-reversed-order). Unlike the `|=` sites, this operand is consumed
> only once (no repeated dereference to eliminate), so the hoist has nothing
> to change here -- the lever's benefit at the `|=` sites came specifically
> from turning a compound-assignment's implicit re-dereference into an
> explicit named value, which this site never had.
>
> Also tried and confirmed inert: reordering `res = target->unk2C;` to sit
> immediately after `info = hdr->field10;` (matching retail's own
> interleaved instruction order exactly, confirmed via direct disassembly
> comparison) -- ZERO change to the `info` register-identity residue (still
> `$a1` here vs `$v0` in retail, propagating through the whole function);
> and hoisting `info->unk4` into a local before the first `target->unk10`
> computation -- also zero change. The `info` register-identity chain (the
> majority of the remaining 18 words, including the `gpu`/`b`/`c`/`d` group
> at `0x8004BFC4`-`0x8004BFDC` which turns out to be a coordinated
> ONE-REGISTER ROTATION -- `gpu` retail=v0/built=a1, `b` retail=v1/built=v0,
> `c` retail=a0/built=v1, `d` retail=a1/built=a0 -- not four independent
> swaps) reads as a single systemic allocation choice rooted at `info`'s
> very first load, not reachable through any of the source-level levers
> tried across this or prior rounds. Not pursued further (register pinning
> is banned per project rule 6).
>
> **Disposition: 132/150, exact length, `INCLUDE_ASM` restored, preserved
> body updated in `src/class_3bb8c.c`.**

> **ROUND 32 (bravo2): re-verified, no new attempt.** Rebuilt the exact
> preserved body from a clean `INCLUDE_ASM` baseline: confirmed 130/150, no
> drift, identical residue (register identity for `info` plus the two
> already-twice-confirmed commutative `or`/`addu` operand-order instances).
> Not re-attempted -- both classes are proven inert by direct testing
> already recorded in this report. Time went to `Class866E8__FindElementForPosition` (matched)
> instead.

> **ROUND 27 (delta): re-verified, no new attempt.** Rebuilt the exact
> preserved body from a clean `INCLUDE_ASM` baseline: confirmed 130/150, no
> drift, identical residue (both classes below unchanged). Both residue
> classes are already proven inert by direct testing in this same report
> (register identity for `info`/a handful of others -- banned to fix per
> project rule 6; the `or`/`addu` commutative-canonicalization class --
> confirmed inert TWICE, including once in this very function), so no new
> structural variant was attempted this round; time went to `Class866E8__UpdateFootprintTracking`
> (matched) and the two other stalls' re-verification instead, per this
> round's staffing guidance.

Unit: `class_3bb8c`. Slot `Obj866E8Methods::slot104` (already declared in the
header as `void (*slot104)(Obj866E8 *self, Elem *entry);`, matching this
function's own signature exactly -- confirmed via the vtable data at
`0x800867EC` in `asm/data/76DC8.data.s`, `0x104` past the table base
`0x800866E8`). Not toolchain-blocked: no `gp_rel` hit, no
`addiu $at,$at,%lo` hit, no dense-`switch`/`jr $v0` dispatch in
`asm/nonmatchings/class_3bb8c/Class866E8__LoadElementResources.s`.

**This is the first C ever attempted against this function.** The previous
round's report (structural analysis only) is superseded by this one -- every
struct/type/control-flow claim below was derived AND test-compiled this
round, not just read off the disassembly.

## What it does

`self` (`$a0`) is never dereferenced -- confirmed again this round, matches
the previous report's finding. `entry` (`$a1`) is an `Elem*` (matches the
one real call site, `Class866E8__OnNotifyTag1`'s `self->methods->slot104(self, e)`).

1. `hdr = entry->unk4` (already-established `ElemTarget*`). `target =
   entry->unk8` -- **new field**, a per-frame GPU link/load coordinator
   (own vtable, own `slot78`).
2. `info = hdr->field10` -- **new `ElemTarget` field at +0x010**, a small
   size/offset header block (`ResInfo866E8`: `s32 unk4`, `s32 unk8`).
   `target->unk10 = (s32)info + info->unk4`; `target->unk14 = 0`.
3. If `target->unk2C` (a `LinkResource*`, its own tiny vtable) is non-NULL,
   call its self-only teardown slot (`vtbl[1]`, i.e. `+0x004`).
4. Reload `info` (fresh read of `hdr->field10`, NOT cached -- retail
   genuinely re-derives it). Build a 0x10-byte load-request local
   (`BE54LoadReq`, only `field0` written = `info + info->unk4 +
   info->unk8`) and call `New_LinkResource(&req)`, storing the result into
   `target->unk2C`. Zero the "found" flag in the loop's outBuf.
5. Loop forever: `idxVal = target->methods->slot78(target, &outBuf, i)`.
   - `idxVal == 0`: return (done).
   - `idxVal == -1`: resolve `*(entry->unk10 + off1)` (an
     `EntryChildObj*`, "already linked" path), OR the flag bit, zero
     `unk20`/`unk18`.
   - otherwise (`idxVal` is itself a pointer, "resolved link result"):
     pick the array slot via `off2` (if `outBuf.flag2`, also `off2 += 4`)
     or `off1`; write `unk20 = idxVal`; **reload** `unk18` and
     `GsLinkObject4`'s `tmd` argument from `((LinkResEntry*)
     ent->unk20)->unk10` (memory reload, not the live `idxVal` register --
     see residue below); `GsLinkObject4(tmd, (u8*)ent + 0x10, 0)` -- `ent`
     has an embedded `GsDOBJ2` at its own `+0x010` (its existing
     `unk10`/bitflags field IS `GsDOBJ2::attribute`, already established
     by `Class866E8__ResetElementCells`); fill `ent->unk14` (**new field**, a
     `GsCOORDINATE2`-shaped `EntryGpu*`) from the loop's outBuf, then its
     own `unk44` sub-object (**new type** `EntryGpuVec`, three halfwords),
     then `ent->unk36` (**new field**, a halfword written DIRECTLY on
     `ent`, not on `gpu` -- the previous round's structural report
     mislabeled this as `gpu->unk36`).
   - Either way: OR `0x80000000` into `ent->unk10`.
   - If `outBuf.found`: `ent->unk38 = *(entry->unk10 + off2)` (a
     singly-linked "process next" pointer, matches `EntryChildObj::unk38`'s
     existing doc), `continue` (no `i`/`off1` bump).
   - Else: `off1 += 4`; `ent->unk38 = 0`; `i++`.

## New header additions (all additive, in `include/class_3bb8c.h`)

- `Elem::unk8` -- `LinkTarget866E8 *`, replaces `pad08[0x0C-0x08]` exactly
  (was 4 bytes, still 4 bytes).
- `ElemTarget::field10` -- `ResInfo866E8 *` at `+0x010`, carved out of the
  old `pad004[0x02A-0x004]` (38 bytes -> 12 + 4 + 22, same total).
- `EntryChildObj::unk14` -- `EntryGpu *` at `+0x014`, replaces
  `pad14[0x18-0x14]` exactly (4 bytes both ways).
- `EntryChildObj::unk36` -- `s16` at `+0x036`, carved out of the old
  `pad24[0x38-0x24]` (20 bytes -> 18 + 2, same total).
- Forward typedefs only for `LinkTarget866E8`, `ResInfo866E8`, `EntryGpu`
  (top-of-file block) -- their FULL bodies live in `class_3bb8c.c`, not the
  header, since none of the 11 sibling units touch them. This is the
  "own local view, own file" half of the project's shared-header
  discipline; only the field additions on the three already-shared types
  above are in the header, and each is additive (no existing declaration
  changed).

No existing declaration was retyped or removed anywhere in this pass.

## Best body reached (132/150, correct length, zero drift -- round 39 update: both `|=` sites hoisted)

```c
/* ElemTarget::field10's pointee -- a small size/offset header block. */
struct ResInfo866E8 {
    u8 pad0[0x4];
    s32 unk4;   /* +0x004 */
    s32 unk8;   /* +0x008 */
};

/* target->unk2C's pointee -- only its self-only teardown slot is reached. */
typedef struct LinkResourceMethods LinkResourceMethods;
typedef struct LinkResource {
    LinkResourceMethods *methods;   /* +0x000 */
} LinkResource;
struct LinkResourceMethods {
    u8 pad0[0x4];
    void (*slot4)(LinkResource *self);  /* +0x004 */
};

/* slot78's own non-0/non-(-1) return value -- a resolved link-target
 * record, read only for its `unk10` (tmd base address). */
typedef struct LinkResEntry {
    u8 pad0[0x10];
    s32 unk10;   /* +0x010 */
} LinkResEntry;

/* Elem::unk8's pointee -- coordinates the per-frame GPU link/load loop. */
typedef struct LinkTarget866E8Methods LinkTarget866E8Methods;
struct LinkTarget866E8 {  /* forward-typedef'd in class_3bb8c.h */
    LinkTarget866E8Methods *methods;  /* +0x000 */
    u8 pad004[0x010 - 0x004];
    s32 unk10;                          /* +0x010 */
    s32 unk14;                            /* +0x014 */
    u8 pad018[0x02C - 0x018];
    LinkResource *unk2C;                    /* +0x02C */
};
struct LinkTarget866E8Methods {
    u8 pad000[0x78];
    s32 (*slot78)(LinkTarget866E8 *self, void *outBuf, s32 arg2); /* +0x078 */
};

/* EntryChildObj::unk14's pointee. */
typedef struct EntryGpuVec {
    u8 pad0[0x10];
    s16 unk10;   /* +0x010 */
    s16 unk12;   /* +0x012 */
    s16 unk14;   /* +0x014 */
} EntryGpuVec;
struct EntryGpu {
    s32 unk0;                    /* +0x000 */
    u8 pad4[0x18 - 0x4];
    s32 unk18;                    /* +0x018 */
    s32 unk1C;                      /* +0x01C */
    s32 unk20;                        /* +0x020 */
    u8 pad24[0x44 - 0x24];
    EntryGpuVec *unk44;                  /* +0x044 */
};

/* slot78's own stack-allocated outBuf, 0x38 bytes -- fields established
 * purely from this function's own reads of it. */
typedef struct BE54OutBuf {
    u8 pad0[0xC];
    s32 b;          /* +0x00C */
    s32 c;            /* +0x010 */
    s32 d;              /* +0x014 */
    u8 pad18[0x1A - 0x18];
    u16 h1;               /* +0x01A */
    u8 pad1C[0x2E - 0x1C];
    u16 h2;                 /* +0x02E */
    s32 flag2;                /* +0x030 */
    s32 found;                  /* +0x034 */
    u8 pad38[0x8];               /* trailing bytes never read/written by this function */
} BE54OutBuf;

typedef struct BE54LoadReq {
    s32 field0;
    u8 pad4[0xC];
} BE54LoadReq;

extern LinkResource *New_LinkResource(BE54LoadReq *req);
extern void GsLinkObject4(s32 tmd, void *objp, s32 n);

void Class866E8__LoadElementResources(Obj866E8 *self, Elem *entry) {
    ResInfo866E8 *info;
    ElemTarget *hdr;
    LinkTarget866E8 *target;
    LinkResource *res;
    EntryChildObj **slot;
    u8 *base;
    EntryGpu *gpu;
    EntryGpuVec *vec;
    s32 b;
    s32 c;
    s32 d;
    s32 h1;
    s32 idxVal;
    s32 flagBit;
    s32 i;
    s32 off1;
    s32 off2;
    BE54OutBuf outBuf;
    BE54LoadReq req;

    hdr = entry->unk4;
    target = entry->unk8;
    info = hdr->field10;
    target->unk10 = (s32)info + info->unk4;
    target->unk14 = 0;
    res = target->unk2C;
    if (res != 0) {
        res->methods->slot4(res);
    }
    info = hdr->field10;
    req.field0 = (s32)info + info->unk4 + info->unk8;
    target->unk2C = New_LinkResource(&req);
    outBuf.found = 0;

    i = 0;
    flagBit = 0x80000000;
    off1 = 0;
    off2 = 0x640;
    for (;;) {
        idxVal = target->methods->slot78(target, &outBuf, i);
        if (idxVal == 0) {
            return;
        }
        if (idxVal == -1) {
            s32 flags10a;
            slot = (EntryChildObj **)((u8 *)entry->unk10 + off1);
            flags10a = (*slot)->unk10;
            (*slot)->unk10 = flags10a | flagBit;
            (*slot)->unk20 = 0;
            (*slot)->unk18 = 0;
        } else {
            base = (u8 *)entry->unk10;
            if (outBuf.flag2 != 0) {
                slot = (EntryChildObj **)(base + off2);
                off2 += 4;
            } else {
                slot = (EntryChildObj **)(base + off1);
            }
            (*slot)->unk20 = idxVal;
            (*slot)->unk18 = ((LinkResEntry *)(*slot)->unk20)->unk10;
            GsLinkObject4(((LinkResEntry *)(*slot)->unk20)->unk10, (u8 *)(*slot) + 0x10, 0);
            gpu = (*slot)->unk14;
            __asm__("");
            b = outBuf.b;
            c = outBuf.c;
            d = outBuf.d;
            gpu->unk18 = b;
            gpu->unk1C = c;
            gpu->unk20 = d;
            vec = (*slot)->unk14->unk44;
            vec->unk10 = 0;
            h1 = outBuf.h1;
            vec->unk14 = 0;
            vec->unk12 = h1;
            (*slot)->unk36 = outBuf.h2;
            gpu = (*slot)->unk14;
            gpu->unk0 = 0;
            {
                s32 flags10 = (*slot)->unk10;
                (*slot)->unk10 = flags10 | flagBit;
            }
        }
        if (outBuf.found) {
            (*slot)->unk38 = *(EntryChildObj **)((u8 *)entry->unk10 + off2);
            continue;
        }
        off1 += 4;
        (*slot)->unk38 = 0;
        i++;
    }
}
```

## How this got from "never compiled" to 130/150 -- the levers that mattered

Starting from the previous round's structural analysis (no C, no build), a
first literal transcription compiled clean but landed at **35/150 with heavy
drift** (frame size wrong by 8 bytes, several instructions genuinely
missing). Closing that gap took four distinct, independently-necessary
fixes, worth recording since each is a small, generalizable lever:

1. **A stack-local outBuf's true size includes bytes it never reads.**
   `BE54OutBuf`'s last USED field (`found`, at `+0x034`) is not the
   struct's true end -- retail's next local (`BE54LoadReq`) sits 8 bytes
   further out (`$sp+0x50`, not `$sp+0x48`). Padding the struct to `0x38`
   (adding an unread `pad38[0x8]`) fixed the whole function's frame size
   from `0x78` to the correct `0x80` in one change. **A local scratch
   struct's size is measured from where the NEXT local lands, not from the
   last field your own code touches.**
2. **A duplicated field access across an if/else's two branches costs a
   real, extra load — hoist it once, shared.** Writing
   `entry->unk10` separately inside BOTH the `if (outBuf.flag2)` and
   `else` branches (even though it's the textually-identical expression in
   both) compiled to TWO separate loads; retail has ONE, shared before the
   branch. GCC 2.6.3 does not CSE this across a branch on its own --
   assigning `base = (u8 *)entry->unk10;` once, before the `if`, was
   required.
3. **A value already live in a register still needs to be re-derived from
   memory if retail re-derives it.** `idxVal` (`slot78`'s return) is
   usable directly as a pointer, and using it that way compiles clean and
   correct -- but SHORTER than retail, which re-reads the identical value
   via `ent->unk20` (the field it was JUST stored into) instead of keeping
   `idxVal` live. Routing both the `unk18` fill and `GsLinkObject4`'s `tmd`
   argument through `((LinkResEntry *)(*slot)->unk20)->unk10` instead of
   `((LinkResEntry *)idxVal)->unk10` recovered this. Same idiom as
   `Unk14Obj::unk0` in `Class866E8__BuildRateEntries`'s report and `u14b` in
   `Class866E8__ComputeFootprintDescriptor`'s -- a THIRD independent confirmation this round.
4. **A cached pointer needs to be RE-cached (not reused) at each natural
   "batch" boundary, and the boundary is where retail's own delay-slot
   nop pattern changes.** The `gpu = ent->unk14` value is used for three
   separate groups of field writes (the `b`/`c`/`d` triple, the `unk44`
   sub-object triple, and the lone `unk0` zero). Caching `gpu` ONCE for
   all three groups was too coarse (skips reloads retail has); reloading
   it at the start of EVERY SINGLE field write was too fine (adds reloads
   retail doesn't have, and 3 extra delay-slot nops since nothing
   independent was left to fill them). The right grain — reassign `gpu =
   ent->unk14` fresh at the START of each of the three groups, reuse it
   for every write WITHIN that group — reproduced retail's own load count
   exactly. The general form: **when a group of a few field-stores through
   the same pointer sits between value-changing operations elsewhere
   (here, unrelated stores through `vec`), match retail's own grouping of
   loads-shared-per-batch rather than picking one caching granularity for
   the whole function.**

A fifth, smaller lever closed one more word: an independent load
(`outBuf.b`/`.h1`) can fill an otherwise-empty delay slot that retail
deliberately leaves as an explicit `nop` (because retail's own scheduling
context didn't have that load ready yet). Where this happened, an EXPLICIT
value read positioned to fill the SAME delay slot retail leaves empty
(`h1 = outBuf.h1;` placed AFTER the store that retail's `nop` follows,
not before) reproduced the `nop`; a bare `__asm__("");` scheduling barrier
(permitted -- only reorders, never changes register identity, verified by
the test in CLAUDE.md) pinned an earlier such case where source order
alone wasn't enough to stop GCC from hoisting a later, independent load
across it.

**A sixth lever, `flagBit = 0x80000000;` as an explicit named local
assigned right where the loop-setup constants are, fixed a
three-instruction SCHEDULING reorder** (retail loads the `0x80000000`
literal before zeroing `off1`; an un-named literal used only via `|=` deep
in the loop body left GCC free to schedule its materialization wherever it
liked, and it chose differently from retail). Giving the constant an
explicit source-level home, at the point retail's own instruction order
implies, recovered the exact sequence. Same class as (2)'s hoist, but for
a compiler-materialized literal rather than a field read.

## Residue: two classes, both already documented project-wide, both
## re-confirmed independently on fresh ground this round

### 1. Register identity (majority of the 18 remaining words as of round 39)

`info` (`ResInfo866E8*`, `hdr->field10`) lives in `$v0` in retail, `$a1`
here -- consistent across BOTH of its uses (the `target->unk10` computation
and the `req.field0` computation before `New_LinkResource`). A handful of
other pointers in the `idxVal`-is-a-pointer branch (the second/third `gpu`
reload, one `vec` reload) show the same single-register swap with
otherwise byte-identical surrounding instructions. Tried and confirmed
inert: swapping the LOCAL DECLARATION ORDER of `info` relative to
`hdr`/`target` (moved `info` to declare first) -- zero change in the
compiled output, consistent with `Class866E8__ApplyRateEntries`'s round-13 finding that
declaration order does not drive `$s`-register assignment in this
codebase's GCC 2.6.3 build. Per CLAUDE.md/MATCHING-GUIDE, this is the
textbook register-identity stall: **not** pursued with `register T v
asm("$N")` or an operand constraint (both banned).

### 2. Commutative-op operand-order canonicalization (`or`/`addu`) -- BOTH `or` instances CLOSED in round 39, `addu` still open

Two `or` instances (both `(*slot)->unk10 |= flagBit;` sites, one per branch)
originally compiled to `or v0,v0,s6` in retail vs `or v0,s6,v0` here, plus a
third instance, the `found`-path tail's `entry->unk10 + off2`, compiling to
`addu v0,s1,v0` in retail vs `addu v0,v0,s1` here. Operand-order-alone was
**tried and confirmed inert** for all three (rounds 27/32): rewriting a `|=`
explicitly as `flagBit | (*slot)->unk10` produced the IDENTICAL `or
v0,s6,v0` -- zero change, matching this round's `Class866E8__FindElementForPosition` finding that
a commutative op's compiled register operand order is independent of source
text order ALONE.

**Round 39 closed BOTH `or` instances by combining that operand-order
observation with a hoist**, per `Class866E8__FindElementForPosition`'s own generalisation (a
residue immune to each lever separately has not been shown immune to their
combination): `s32 t = (*slot)->unk10; (*slot)->unk10 = t | flagBit;` at
each site, turning the compound assignment's implicit re-dereference into an
explicit named value. Both sites now compile to `or v0,v0,s6`, byte-identical
to retail. **The SAME hoist, applied to the `addu` site (hoisting
`entry->unk10` into a named local, tried with both operand orders), did NOT
move it** -- all three variants (unhoisted, hoisted-natural-order,
hoisted-reversed-order) produced the identical residue. The difference: the
`|=` sites each had a REPEATED implicit dereference (read-modify-write on the
same field) for the hoist to eliminate; the `addu` site's `entry->unk10` is
read exactly once regardless, so there is nothing for a hoist to change
there. **The lever is not "hoist any commutative operand," it is specifically
"hoist a value that would otherwise be re-dereferenced."**

## Attempts

~10 iterations from a fresh derivation (no prior C, no prior build) to
130/150: initial transcription (35/150, heavy drift) -> outBuf size fix
(frame length correct) -> `entry->unk10` hoist across the if/else (closed
the double-load) -> `idxVal`-vs-memory-reload fix (per-field, closed one
residue class but overshot length by re-deriving `gpu` too eagerly,
regressing to 160/150-equivalent overlength) -> `gpu` reload granularity
fixed to per-group (154 words) -> `b`/`c`/`d` load/store batching (151
words) -> `h1` load repositioned to match retail's actual delay-slot
choice (150/150 length, no more drift) -- at this point CORRECT LENGTH
was reached, remaining residue purely cosmetic -> `flagBit` named local
(fixed a 3-instruction scheduling reorder, 129 -> 130) -> declaration-order
swap on `info` (no change, inert, per residue class 1) -> commutative-op
reversal on both remaining `or`/`addu` sites (no change, inert, per residue
class 2, confirmed twice within this same attempt).

### Proposed learnings

- **A local scratch struct's declared size should be measured against
  where the NEXT stack local lands, not against the last offset your own
  code reads or writes.** `funcdiff`'s drift warning plus a direct `.o`
  frame-size check (`addiu sp,sp,-N` in the prologue) is the fast way to
  catch this -- an 8-byte frame-size gap traces directly to one struct's
  missing trailing padding.
- **GCC 2.6.3 does not eliminate a duplicated field-access expression
  across the two arms of an `if`/`else`, even when it's the textually
  identical subexpression in both.** If retail shares one load across a
  branch, the SOURCE must hoist it explicitly (a named local assigned once
  before the `if`); writing the same expression in both arms compiles to
  two loads, not one, silently costing exactly the frame-size-sized amount
  of drift that then obscures every OTHER residue in the function.
- **"Cache a pointer, reuse it across many field writes" is not a single
  invariant policy -- match retail's own GROUPING.** When a value is
  written via the same base pointer several times with unrelated
  operations interleaved (here: stores through a different pointer `vec`
  in between), retail's compiler re-derives the base pointer once per
  GROUP of writes, not once for the whole function and not once per single
  write. Getting the granularity wrong in EITHER direction changes the
  instruction count, which is a much louder and more actionable signal
  than register identity -- fix instruction COUNT before worrying about
  which register anything lives in.
- **The `or`/`addu` commutative-operand-order class is now confirmed
  twice, on two unrelated functions, with two different operators.** Any
  future near-miss whose LAST couple of residue words are a same-opcode,
  reversed-operand-register diff on a commutative op should skip source
  reordering entirely (proven inert twice) and file it directly under this
  class.
- **(Round 39) That same class splits into two sub-cases once a hoist is
  tried alongside the operand-order check, and only one of them yields.**
  A compound-assignment `|=`/`+=` site (an implicit re-dereference) closes
  under "hoist the field into a named local, THEN combine" -- both `or`
  sites here did. A plain single-use commutative operand (read once, never
  re-dereferenced) does not, no matter which operand gets hoisted or which
  order it's written in -- the `addu` site here didn't, across three
  variants. Screen for "is this operand implicitly re-read by a compound
  assignment" before spending an attempt on the hoist-combination lever.

## ROUND 38 (alpha, salvaged by head): preserved body REBUILT and confirmed at 130/150 words

Runner alpha was staffed onto this unit and died to an infrastructure error
(org API 403) with nothing committed. The head restored the worktree and
re-measured this report's preserved body directly, by flipping its
`#if 0` guard to `#if 1` and commenting out the matching `INCLUDE_ASM`,
one body at a time against the whole-image oracle.

**Result: `130/150 words`, compiling and LINKING cleanly (zero hits on the
compile-error grep).** The recorded figure is accurate and the body is real.

This is the Gate 1b "rebuild before trusting" check, and it matters here
because round 37 measured roughly one inherited body in six carrying a false
drift-free claim, plus one body that could never have linked at all (it
called a symbol since renamed, so its figure had measured nothing). **All
four preserved bodies in `class_3bb8c` were rebuilt this round and all four
are honest** — `Class866E8__FindElementForPosition` 68/70, `ComputeCellWorldOffsets` 58/73,
`Class866E8__BuildRateEntries` 125/140, `Class866E8__LoadElementResources` 130/150. No stale figure and no
never-linked body in this unit.

No new lever was tried — alpha died before attempting one. This is a
confirmation, not a negative result, and the function's permuter status is
unchanged.


## ROUND 40 (HEAD, SUPERSEDED -- kept only for the hazard it records)

**This section's verdict is WRONG and the entry above is correct.** While
runner bravo was between turns, the head read its saved permuter candidate,
noticed it carried the same dead-reload construct that had just gained 12
words on the sibling `Class866E8__BuildRateEntries`, tried to apply it, and **failed to build**
-- ``LinkResource' undeclared``. The head recorded the lever here as
**UNTESTED** and flagged it as the next round's cheapest step. Bravo then
resumed and tested it properly, reaching **142/150**.

The head's build failure was real but its cause was local and avoidable, and
**the hazard is worth keeping even though the conclusion drawn from it was
premature**:

**A permuter scaffold's `base.c` is NOT the project's C.**
`tools/setup-permuter.sh` emits a FLATTENED translation unit -- the function
plus its own inlined preamble of typedefs. Pasting that body straight into
`src/class_3bb8c.c` cannot work, because this function's local types live in
this report's `#if 0` block, not in the unit. Bring the declarations across
with the body, build, *then* believe the figure.

That failure was loud and cost a minute. **The dangerous version is the quiet
one**: a scaffold body that happens to compile in the unit while binding a
DIFFERENT type of the same name would produce a real-looking number for the
wrong code. This is round 33's never-linked-body hazard one layer down.

The transferable lesson about the head's own error is separate and simpler:
**a "cannot test this" verdict reached by one failed build is not a
measurement.** The correct disposition was to bring the declarations across,
which is what bravo did.

---

## (superseded) ROUND 40 (bravo's search, head's analysis): first-ever permuter search, no zero -- but the saved candidate carries THE SAME LEVER that gained 12 words on Class866E8__BuildRateEntries, and it is UNTESTED here

**Provenance.** Runner bravo built the scaffold and ran the search, then ended
its session waiting on a notification that was never coming, without writing
this report. The head recovered the artifacts and wrote this entry. **Nothing
below was applied to this function** -- read the last section as the next
step, not as a result.

### The search

| field | value |
| --- | --- |
| iterations | **20367** |
| base score | 135 |
| best score reached | **65** (`output-65-1`; also 70, 75, 85 saved) |
| zero found | no |
| stop reason | own `timeout` bound |

So this function is now genuinely searched, and the figure is recorded so the
next round ranks on cost rather than re-deriving it.

### The untested lever, and why it is the cheapest thing left here

Reduced to statements, the score-65 candidate makes two real changes among a
great deal of the permuter's own reformatting noise. The first is the
**identical construct** that took the sibling `Class866E8__BuildRateEntries` from 125/140 to
**137/140** this same round -- eliminating a redundant reload of a pointer
already held in a local:

```c
/* before */
gpu = (*slot)->unk14;
gpu->unk0 = 0;

/* after */
(*slot)->unk14->unk0 = 0;
```

The second hoists `*(EntryChildObj **)((u8 *)entry->unk10 + off2)` into a
named local before dereferencing it.

**The same mutation surfacing as the best candidate on two different functions
in one round is the reason this is worth flagging rather than filing.** On the
sibling it was worth 12 words.

### Why it was not tested here, stated plainly

The head applied it and the build failed: ``LinkResource' undeclared``. The
permuter's `base.c` is a FLATTENED scaffold with its own preamble, so its body
is not directly pasteable into `src/class_3bb8c.c` -- this function's local
types live in this report's own `#if 0` block and must come across with it.
That is a ten-minute job and a runner's, not a head's, and doing it badly
would have produced a figure that measured the wrong thing.

**This is a new instance of a documented hazard, in a new place.** Round 33's
lesson was that a preserved body calling a symbol that no longer exists could
never have linked, so its figure measured nothing. The same trap is waiting
one step earlier: **a permuter scaffold's `base.c` is not the project's C**,
and treating it as a drop-in is how a body that never compiled acquires a
number. Bring the declarations, then build, then believe the figure.

### Next step

Apply both halves of the candidate to the preserved body below (with its
typedefs), build, and measure. Try the reload elimination ALONE first -- on
the sibling that single change carried the entire 12-word gain, and a combined
result says nothing until each component is measured alone (round 39's
head-side trap on `Viewport__InitOt`).

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004BE54` | `Class866E8__LoadElementResources` | B | Occupant of `gClass866E8Methods` +0x104 (`slot104`), and its own identity slot -- `Class866E8__OnNotifyTag1` dispatches `self->methods->slot104(self, e)` which resolves to this same function. Body: releases the element's old link resource, issues a new `New_LinkResource` resource-load request, then loops `target->methods->slot78` building `GsLinkObject4`-linked GPU records into the element's cell array (`entry->unk10`). Matches `src/class_3ac78.c`'s own unit-header narrative almost verbatim: "The queries that build those rectangles, and an element's resource AND GPU sides, live in class_3bb8c*" -- this function IS that resource-and-GPU side. |

## Track 4 (2026-09-26, round 87, echo)

`LinkTarget866E8` and `BE54OutBuf` were views of Class6D940 and its placement record; both are gone for `include/Class6D940.h` (`buffer`, `bufferSize`, `linkResource`; `Class6D940Placement` x/y/z/rotY/unk2E/chained/next). The slot78 call casts the inherited `void *slot78` to `Class6D940ResolveEntryFn` (no code). The local LinkResource view went with it: `linkResource` is `Class6D430 *` and its +0x004 is `release`. Byte-identical.
