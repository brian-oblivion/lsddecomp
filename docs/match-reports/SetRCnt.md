# SetRCnt

**Unit:** libsnd_ssinit · **Size:** 40 instructions · **Status:** MATCHED (40/40 words)

Not a `psyq_*` segment function, but Sony's `SetRCnt` all the same: see
"Identification (round 69, head)" at the end. It and its four neighbours are
`libapi/counter` compiled into game text from a library build the SDK discs
do not carry. (Superseded round-16 note, corrected round 69: this report said
the prototype was absent from `include/psyq/`. `KERNEL.H` lines 156-160
declare all five, `extern long SetRCnt(unsigned long, unsigned short, long);`
among them. The signature below was derived from the call site and body and
agrees with it on arity.)

## What it does

Configures one of the three PSX hardware root counters (`n` in `{0,1,2}`,
masked to 16 bits and range-checked). Clears the counter's mode register,
stores the 16-bit `target` value, then builds a mode bitmask from the
caller's `mode` flags:

- counters 0/1 use base `0x48`, upgraded to `0x49` if `mode & 0x10`
  (IRQ-on-target-ish), with `0x100` added when `mode & 1` is clear
  (free-run vs synchronized-ish).
- counter 2 uses base `0x48`, replaced by `0x248` when `mode & 1` is
  clear.
- all three OR in `0x10` when `mode & 0x1000` is set (IRQ-repeat-ish).

Returns `1` on success, `0` if `n` was out of range.

## The C

**STALE below (pre-round-32): this block still shows the plain-`u16`,
`__asm__("")`-barrier form.** `ResetRCnt.md`/`GetRCnt.md` document that round
32 replaced the barrier with `volatile` on all three `RCntEntry` fields
(load-bearing for those two functions, and it subsumes this function's own
barrier -- removed, `SetRCnt` still verifies 40/40). The struct in
`src/psyq/libsnd_ssinit_libapi_counter.c` is now:

```c
typedef struct {
    volatile u16 count;              /* 0x0 */
    u8  pad2[0x4 - 0x2];
    volatile u16 mode;                /* 0x4 */
    u8  pad6[0x8 - 0x6];
    volatile u16 target;               /* 0x8 */
    u8  padA[0x10 - 0xA];
} RCntEntry;
```
and `SetRCnt`'s own body carries no `__asm__("")` at all. The signedness and
mode-bitmask findings below are unaffected by this and still hold.

```c
/* Shadow copy of the three PSX root-counter register blocks (COUNT/MODE/
 * TARGET, each a hardware halfword, 0x10 apart -- matches the real
 * 0x1F801100/0x1F801110/0x1F801120 hardware spacing). D_8006DCB0 is a
 * pointer to this table, not the table itself. */
typedef struct {
    u16 count;              /* 0x0 */
    u8  pad2[0x4 - 0x2];
    u16 mode;                /* 0x4 */
    u8  pad6[0x8 - 0x6];
    u16 target;               /* 0x8 */
    u8  padA[0x10 - 0xA];
} RCntEntry;

extern RCntEntry *D_8006DCB0;

s32 SetRCnt(s32 n, s16 target, u32 mode)
{
    s32 idx = (u16)n;
    u16 md = 0x48;
    u32 isLow;

    if (idx >= 3) {
        return 0;
    }

    isLow = (u32)idx < 2;
    D_8006DCB0[idx].mode = 0;
    D_8006DCB0[idx].target = target;

    if (isLow) {
        if (mode & 0x10) {
            md = 0x49;
        }
        if (!(mode & 1)) {
            md |= 0x100;
        }
    } else if (idx == 2) {
        if (!(mode & 1)) {
            md = 0x248;
        }
    }

    if (mode & 0x1000) {
        md |= 0x10;
    }

    D_8006DCB0[idx].mode = md;
    return 1;
}
```

## Residue notes (both generalisable to the sibling get/set functions)

- **Two comparisons on the same masked value use different signedness in
  retail, and the C has to say so explicitly.** `idx >= 3` compiles to
  `slti` (signed) but `idx < 2` compiles to `sltiu` (unsigned) -- same
  register (`t0`), same function, same masked-to-u16 value. Declaring
  `idx` as plain `s32` gets the first comparison right and the second
  wrong (`sltiu` vs `slti`); the fix is an explicit `(u32)idx < 2` at the
  second site only. Do not "fix" this by making `idx` unsigned throughout
  -- that flips the FIRST comparison instead. This looks arbitrary from
  the C but is a straightforward GCC 2.6.3 codegen fact for `slti`'s
  16-bit immediate range: worth checking on any sibling function with
  more than one range check on the same masked index.
- **A store got hoisted into a jump's delay slot when retail leaves it a
  real `nop`.** Without intervention, `D_8006DCB0[idx].target = target;`
  (an ordinary, independent store) got scheduled by cc1's delay-slot
  filler to execute *after* the following `beqz`, i.e. into its delay
  slot -- legal (the store doesn't affect the branch condition) but not
  what retail did. A bare `__asm__("");` placed immediately after the
  store (see the C above) blocks exactly this one hoist without touching
  anything else, and there's positive evidence it's an ORDER-only fix, not
  a register-identity one: with vs. without the barrier, every other
  instruction and every register assignment in the function is byte-
  identical -- only the position of that one `sh` instruction (and,
  necessarily, everything textually after it) moves. Per CLAUDE.md's
  test this is exactly the permitted case.
  **Placement is unexpectedly narrow, though** -- the same barrier one
  statement earlier (between the two field stores) or one statement later
  (between the store and the `if`) each produced a WORSE diff (moved the
  early `li $v0` computation, or introduced a second, redundant `sh` /
  `move`). Only immediately-after-the-target-store worked. Treat barrier
  placement as something to verify instruction-by-instruction, not assume
  transitively.

### Proposed learning

Two are proposed for consolidation into DECOMPILATION_LEARNINGS.md:

1. Two comparisons against the SAME masked/truncated local can legitimately
   compile to different signedness (`slti` vs `sltiu`) in the same GCC
   2.6.3 function. Don't assume one C type for the variable settles both;
   check each comparison's instruction independently and cast locally
   (e.g. `(u32)idx < N`) where retail disagrees with the variable's
   natural type.
2. A bare `__asm__("");` scheduling barrier's effect is highly
   position-sensitive -- it can fix one delay-slot-fill hoist while
   *breaking* a different, unrelated instruction's placement if put one
   statement off. Confirm order-only-ness (per CLAUDE.md's test) at the
   EXACT placement used, not the general vicinity.

## Provenance

round 16 (2026-09-04), runner delta, unit libsnd_ssinit (fresh carve).
Matched after ~4 attempts (signedness fix, branch-direction/guard-clause
shape not needed here since it already matched, and the scheduling
barrier).

## Naming

Round 69 (delta). `gRCntRegs` (was `D_8006DCB0`): pointer to the 3-entry
shadow of the PSX root-counter register blocks (COUNT/MODE/TARGET, matching
the real `0x1F801100`/`0x1F801110`/`0x1F801120` hardware spacing) -- tier B,
established by this function's own pre-existing doc comment and used
identically by `GetRCnt`/`ResetRCnt`.

## Identification (round 69, head)

Sony's `SetRCnt`, `libapi/counter`: the five functions at 0x80032B18 are that module's exports in its own order (SetRCnt, GetRCnt, StartRCnt, StopRCnt, ResetRCnt), with the first three offsets exact against the 3.5/3.6 `counter.o` and the last two 4 and 8 bytes later (a library build the discs do not carry, so no object can be linked); `KERNEL.H` prototypes agree on arity. Two evidence kinds per FINISHING-PLAN track 2. 

## History (moved from src/libsnd_ssinit_libapi_counter.c, comments pass)

The comment on the root-counter shadow table, above SetRCnt, ended:

> The three hardware fields are `volatile` because they ARE memory-mapped
> registers, and that is load-bearing for matching as well as correct:
> without it GCC reorders the table load against the index arithmetic and
> hoists stores into unconditional-jump delay slots retail leaves as `nop`.
> It closed GetRCnt and ResetRCnt in round 32 -- the first of
> which had been filed for three rounds as an unfixable register-identity
> residue -- and it SUBSUMES the `__asm__("")` barrier SetRCnt used to
> carry (removed in the same round; SetRCnt still verifies 40/40).
> See docs/match-reports/ResetRCnt.md for the mechanism.
