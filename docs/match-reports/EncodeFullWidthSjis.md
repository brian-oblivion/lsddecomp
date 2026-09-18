> Renamed from `func_80041020` on 2026-09-18 (tools/rename.py). Address 0x80041020.

# EncodeFullWidthSjis -- MATCHED, round 38 (31/31). Closed by permuter search from round 37's 20/31 exact-length register-identity near-miss: reading the second byte's value through a fresh local instead of reusing the first byte's variable made GCC's allocator match retail's 3-way register assignment exactly.

**Round 35 (runner delta) confirmation at the bottom of this file: reconfirmed
19/31 after rebuilding the preserved body; a bare `__asm__("")` barrier tried
at two positions, both inert (no score or diff-set change).**

Unit: `src/code_2cc8c_f.c`. Round 27 (second pass), runner bravo, off the
head's fresh-ground list.

## Screens (clean)

```
grep -n 'gp_rel' asm/nonmatchings/code_2cc8c_f/EncodeFullWidthSjis.s            -> no hits
grep -A2 -nE '\b(mflo|mfhi)\b' ... | grep -E '\b(mult|multu|div|divu)\b'  -> no hits
```

## Result

`./build-and-verify.sh` GREEN with `INCLUDE_ASM` restored (self-tested by
splicing the preserved body back in and rebuilding). Went from the prior
round's single attempt (7/31, WRONG length, unisolated) to **31/31 exact
length, 19/31 raw word-match, zero out-of-range drift**. The two-byte
expansion shape from the prior report is confirmed correct; what closed
the length gap was a chain of type/shape fixes on the SECOND output
byte's computation, described below.

## Shape (confirmed, matches the prior report's derivation)

Per source byte `c`: write `(c < 0x30) ? 0x81 : 0x82` at the first output
position, advance `dst`; then write `(c < 0x60 && c != 0x20) ? c + 0x1F :
c + 0x20` at the second, advance `dst` again; advance `src` by 1. Both
`dst` increments are unconditional (confirmed: both live in delay slots
that always execute). Terminates on `*src == 0`, NUL-terminates `dst`,
returns the final `dst`.

**One correction to the prior report's formula**: it read the second
byte's rule as `(c < 0x60) ? (c == 0x20 ? c+0x1F : c+0x20) : c+0x20`,
which has the `c==0x20` case backwards. Tracing the actual branch
polarity (`bne $v1,$a3,L…` — branch AWAY from the `c+0x20` computation
when `c != 0x20`) shows `c==0x20` produces `c+0x20` (the SAME formula as
the `c>=0x60` case), and it is `c<0x60 && c!=0x20` that produces
`c+0x1F`. Both cases share ONE instruction (`addiu $v0,$v1,0x20`) in
retail, reached either by falling through from the outer test or by
falling through from the inner one — the shape that closed the length
gap (see below).

## What closed the length gap, in order

1. **The second byte's value needs an unsigned-int-width local, not a
   `u8`.** A `u8 c2` used in `if (c2 < 0x60)` then again in `if (c2 !=
   0x20)` then again in `c2 + 0x1F`/`c2 + 0x20` compiles an EXTRA `andi
   $a2,$v1,0xff` before the first comparison — confirmed in ISOLATION
   through the pinned pipeline (a minimal `u8 c2 = *src; if (c2<0x60)
   {...}` reproduces the extra `andi` on its own, nothing to do with
   surrounding context). Retail has no such mask; declaring the SAME
   local `u32 c2` instead (still assigned from `*src`, a `u8*`) drops it,
   matching the sibling class already documented (`func_8002FAC4`'s
   `chan = call() & 0xFF` staying `s32` to get `slt` not `sltu` — same
   family: a byte-sized VALUE reused across multiple comparisons needs a
   wider LOCAL type even though the underlying quantity never exceeds a
   byte).
2. **The outer/inner test needs to be ONE combined condition
   (`c2 < 0x60 && c2 != 0x20`), not a nested `if`.** A nested
   `if (c2<0x60) { if (c2!=0x20) {...} else {...} } else {...}` compiles
   an extra `j`-over-the-`c+0x20`-computation (the `c==0x20` and `c>=0x60`
   arms end up as TWO separate copies of the same instruction, needing a
   jump to reach the second from the first) — confirmed in isolation
   (2 extra words, `j` + duplicate `li`). The combined `&&` form lets
   GCC route BOTH the `c==0x20` and `c>=0x60` cases to the SAME
   `addiu $v0,$v1,0x20` instruction via fallthrough, with no jump at all
   — byte-identical to retail once isolated. This is the "share the
   join point via a combined condition" shape, a new instance of the
   project's "textually-first block becomes fallthrough" family but
   for a **compound** boolean rather than a single comparison.
3. **A single `*dst++ = v;` store per output byte** (one named `v`
   local set inside the `if`/`else`, one store after), not two
   independent `*dst++ = ...` statements inside each arm. The latter
   compiles correctly in VALUE but produces a completely different,
   drifted instruction count (confirmed: reverting to two per-arm
   stores regressed from 19/31 exact-length to 2/31 with whole-image
   drift).

All three were isolated and confirmed through the pinned pipeline on
minimal reproducers (`tools/gcc263/cpp | cc1 | maspsx | as`) before being
applied to the real function, per CLAUDE.md's "escalate, do not
experiment" recipe used here just to test C shapes, not the toolchain.

## The residue that did not close: cursor re-sync scheduling, 12 words

Every remaining difference is about WHEN a second, lagging cursor
register gets (re-)synced to the advancing `dst` pointer — never a
content or value difference. Retail keeps `dst` (`$a0`) as the
"next write position" and a SEPARATE register (`$a2`) as "the position
just written," re-syncing `$a2 := $a0` immediately after EACH increment
of `$a0`, so each `sb` store always targets `$a2` one step behind:

```
[initial]      $a2 = $a0                (in the entry beqz's delay slot)
loop:
               $a0++                     (first increment)
               ...compute first byte...
               sb  ..., 0($a2)           (STORE via the lagging register)
               $a2 = $a0                 (re-sync immediately)
               ...compute second byte, $a0++ unconditionally in a delay slot...
               sb  ..., 0($a2)           (SECOND store, still via the now-stale $a2)
               ...
               $a2 = $a0                 (re-sync for next iteration, in the loop-back delay slot)
```

My best build's SECOND store targets `$a0` directly (no lagging
register at all — `sb v0,0(a0)` where retail has `sb v0,0(a2)`), and its
SECOND increment of `$a0` lands one instruction later than retail's.
Content, values, and branch targets are all identical; only the
register holding each store's address, and the exact cycle the second
increment executes on, differ.

**Two reshapes tried this round, both regressed sharply — recorded so
the next attempt does not re-spend on them:**

1. **An explicit second `cur` pointer variable**, synced to `dst`
   immediately after each `dst++` and used as the store target
   (literally transcribing the register relationship traced above):
   regressed to 3/31 with drift (33 words built). Introducing a NAMED
   second pointer, even one that is semantically a pure alias of `dst`,
   changes the compiler's whole scheduling decision rather than
   reproducing retail's specific one.
2. **Two separate `*dst++ = v;`/`*cur = v;` statement pairs per arm**
   (see item 3 above): also regressed sharply. Same family as #1 —
   any attempt to make the "lag" explicit in the source made things
   worse, not better.

This is the SAME "redundant cursor cache" residue class already
documented on this unit's sibling stalls, `DecodeFullWidthSjis` and (by
extension) `func_800407F8` — an argument/pointer used only as a store
TARGET, never re-read, still gets its OWN register in retail, and no
C-level reshape tried on any of the three functions in this family has
reproduced it. Per CLAUDE.md, fixing which physical register holds a
value (`register T v asm("$N")` or an operand constraint) is banned; a
bare `__asm__("")` was not tried this round given the "changes ORDER
only" test does not obviously apply here (the residue is a REGISTER
CHOICE across two stores of the same value's lineage, not an
instruction-order swap within one already-correct set) — worth trying
next, but not assumed to help.

### Proposed learning

**A byte value reused across two-or-more comparisons/arithmetic
operations needs a WIDER local type (`u32`, not `u8`) to avoid a
spurious re-mask, even when every value in play fits in a byte — a
fourth confirmed instance of the "declared width is a codegen decision"
family, and specifically the same lever as `func_8002FAC4`'s
`chan`/`slt`-vs-`sltu` finding from round 27's other unit.** Isolated
in under a second through the pinned pipeline; always test single-use
vs. multi-use locals separately when a `u8`/`u16` variable's declared
width is suspect.

**A compound condition (`a && b`) sharing a fallthrough target with the
FAILURE case of BOTH sub-conditions is a distinct instance of the
"textually-first block becomes fallthrough" family, and it generalizes
past the two-way `if`/`else` this project had previously documented it
for.** When retail's disassembly shows two logically-different "false"
outcomes (here: `c>=0x60` and `c==0x20`) converging on the SAME
instruction via fallthrough with no jump between them, write the
positive case as ONE combined `&&` condition rather than nesting two
`if`s — nesting forces GCC to duplicate the shared instruction and jump
to reach it a second time.

## Preserved body (best attempt, 19/31 raw words match at EXACT 31-word length -- residue is a lagging-cursor register choice, banned to fix further without a fresh lever)

```c
#if 0
u8 *EncodeFullWidthSjis(u8 *dst, u8 *src) {
    if (*src != 0) {
        do {
            u8 c1 = *src;
            u32 c2;
            u8 v;
            *dst++ = (c1 < 0x30) ? 0x81 : 0x82;
            c2 = *src;
            if (c2 < 0x60 && c2 != 0x20) {
                v = c2 + 0x1F;
            } else {
                v = c2 + 0x20;
            }
            *dst++ = v;
            src++;
        } while (*src != 0);
    }
    *dst = 0;
    return dst;
}
#endif
```

## Round 35 confirmation: reconfirmed, barrier lever tried at two positions, both inert

Runner delta, `code_2cc8c_f`, off the head's fresh-ground list.

Rebuilt the preserved body first (per this round's "build any inherited body
once before trusting its score" instruction): links and scores exactly
19/31 at the correct 31-word length, no drift. `build exit=2`, no
compile-error hits — a real, fresh measurement, matching the recorded score.

The round-27 report left "a bare `__asm__("")` was not tried this round...
worth trying next" open. Tried it at two positions this round:

1. Between the first `*dst++ = ...;` store and the second byte's `c2 = *src;`
   read.
2. Between the `if`/`else` computing `v` and the second `*dst++ = v;` store.

**Both produced byte-identical output to the un-barriered body: 19/31, same
diff set, no drift.** Unlike on the sibling `DecodeFullWidthSjis` (where the same
lever this round REGRESSED the score sharply at both positions tried there),
here it did nothing measurable at either position. So the barrier's effect
is position- and function-sensitive, not a generic tool for this residue
class — it is not a safe lever to reach for by default, and it is not
established as harmful either; it simply did not move anything here.

**Verdict unchanged: 19/31, exact length, stall.** No new lever found this
round. `INCLUDE_ASM` restored; `./build-and-verify.sh` clean.

### Proposed learning

**A bare `__asm__("")` scheduling barrier's effect on the redundant-cursor-
cache residue is NOT consistent across sibling functions in the same class**
— inert on `EncodeFullWidthSjis`, sharply regressive on `DecodeFullWidthSjis` (same
round, same source-level lever, two different bodies). Do not generalize a
single measurement of this barrier to "the barrier is safe/inert for this
residue class" or "the barrier is harmful for this residue class" — it must
be re-measured per function, and a null result on one sibling is not
evidence for or against trying it on another.


---

## ROUND 37 (head): the "redundant cursor cache" class DISSOLVES here too. Structure now exact; residue is pure register identity.

This function was cross-filed with `DecodeFullWidthSjis` as an instance of a
"redundant cursor cache" toolchain class -- the shared claim being that a
second cursor register is one "every C form tried collapses into one".
**That claim was false for the sibling and it is false here.** It measured
one idiom (a single destination pointer) rather than the compiler.

Retail's source keeps **two** destination variables, and the store goes
through the copy, not the parameter:

```c
d = dst;      /* move a2,a0  -- the PRE-increment value */
dst++;        /* addiu a0,a0,1 */
*d = lead;    /* sb v1,0(a2)  -- stores through the COPY */
```

Written that way — twice per iteration, since this transcoder emits two
bytes per input byte — GCC 2.6.3 reproduces retail's instruction sequence
**exactly**: correct length (31/31), correct CFG, every opcode and every
immediate in retail's own slot.

Body reached (20/31, exact length):

```c
#if 0
u8 *EncodeFullWidthSjis(u8 *dst, u8 *src) {
    u8 *d;
    u32 c;
    u32 v;
    u32 lead;

    if (*src != 0) {
        do {
            d = dst;
            dst++;
            c = *src;
            if (c >= 0x30) {
                lead = 0x82;
            } else {
                lead = 0x81;
            }
            *d = lead;
            d = dst;
            dst++;
            c = *src;
            if (c < 0x60 && c != 0x20) {
                v = c + 0x1F;
            } else {
                v = c + 0x20;
            }
            src++;
            *d = v;
        } while (*src != 0);
    }
    *dst = 0;
    return dst;
}
#endif
```

### The entire remaining residue is ONE renaming, in three values

| value | retail | this build |
| --- | --- | --- |
| `d` (the store cursor) | `$a2` | `$a3` |
| the `0x20` constant | `$a3` | `$t0` |
| `lead` | `$v1` | `$a2` |

All seven differing words are that substitution and nothing else:

```
10 off=0x031848 retail=81000334 built=81000634   li   v1/a2, 0x81
11 off=0x03184C retail=82000334 built=82000634   li   v1/a2, 0x82
12 off=0x031850 retail=0000c3a0 built=0000e6a0   sb   v1,0(a2) / a2,0(a3)
13 off=0x031854 retail=21308000 built=21388000   move a2,a0 / a3,a0
19 off=0x03186C retail=02006714 built=02006814   bne  v1,a3 / v1,t0
23 off=0x03187C retail=0000c2a0 built=0000e2a0   sb   v0,0(a2) / v0,0(a3)
27 off=0x03188C retail=21308000 built=21388000   move a2,a0 / a3,a0
```

**This is a REGISTER-IDENTITY stall in the project's precise sense, so it is
NOT fixable here:** `register T v asm("$N")` and extended-asm operand
constraints are banned project rules, and removing such a construct would
change WHICH REGISTER holds a value rather than instruction order. Filed as
a stall accordingly.

### What moved and what did not

- The branch-polarity form matters and is worth one word: writing
  `if (c >= 0x30) lead = 0x82; else lead = 0x81;` yields retail's
  `bnez` with `li 0x81` in the delay slot (19/31 -> **20/31**). The
  inverted spelling `if (c < 0x30) lead = 0x81; else lead = 0x82;`
  produces `beqz` with the constants swapped.
- **Block-scoping the temporaries (`c`, `v`, `lead` declared inside the
  `do` block) is neutral** -- 20/31, byte-identical output. It does not
  shorten `lead`'s live range enough to hand it `$v1`.
- Two separate stores (`*dst++ = 0x81;` / `*dst++ = 0x82;` in the two arms)
  is WRONG and drifts the length: retail has ONE store of a selected value.

### Disposition

Exact length, exact structure, pure register-identity residue, and **never
permuter-searched**. Reshaping the source is the only permitted lever on a
register-identity residue, which is exactly what the permuter does. **Prime
round-38 permuter target; seed from the body above.** No search was run here
because five runners were saturating the host.

### Proposed learning

See `DecodeFullWidthSjis.md` for the general form. The addition from this
function: **the two-cursor idiom is `d = dst; dst++; *d = x;` -- the
LONGHAND of `*dst++ = x`, which is NOT equivalent in codegen.** The
post-increment spelling collapses to a single register and stores through
the parameter; the longhand keeps retail's separate copy.

**And a correction while dissolving the class: it had TWO members, not
three.** `func_800407F8` has been cross-filed with these two since round
18 as an "analogous stall" -- but it was **MATCHED 11/11 in round 19**, by
an unrelated whole-struct-assignment lever, and the cross-references were
never updated. A class assembled by cross-reference keeps counting a
member after that member has been closed, because the closing round
updates the function's own report and not the reports that point at it.

---

## ROUND 38 (bravo): MATCHED, 20/31 -> 31/31, via permuter

Rebuilt the round-37 preserved body first, per Gate 1b's "rebuild before
trusting" instruction: it links and scores exactly **20/31 at the correct
31-word length**, no drift, confirming the title is not stale.

Set up and ran the permuter from that body:

```
tools/setup-permuter.sh EncodeFullWidthSjis <seed>
PATH=$PWD/permuter-work/bin:$PATH \
  .venv/bin/python3 tools/decomp-permuter/permuter.py --debug --stack-diffs \
    permuter-work/EncodeFullWidthSjis
```

`--debug` confirmed the base score against the report before searching:

```
Register Differences:          12  (5)
[EncodeFullWidthSjis] base score = 60
```

12 register differences at penalty 5 each = 60, matching the report's own
tally of the 3-way renaming (`d`, the `0x20` constant, `lead`) across
seven differing words exactly.

Bounded search:

```
timeout 600 env PATH=$PWD/permuter-work/bin:$PATH \
  .venv/bin/python3 tools/decomp-permuter/permuter.py -j 6 \
    --stop-on-zero --best-only --stack-diffs permuter-work/EncodeFullWidthSjis
```

Zero found at **iteration 158**, `permuter rc=0` (ran to completion). The
winning candidate copies the second byte's value into a fresh local
(`new_var = c;`) before using it in the second `if`/arithmetic, instead of
reusing `c` directly:

```c
c = *src;
new_var = c;
if (new_var < 0x60 && new_var != 0x20) {
    v = new_var + 0x1F;
} else {
    v = new_var + 0x20;
}
```

That single change (a copy into a second-use local, not any change to the
arithmetic or branch shape) is enough to make GCC 2.6.3's register
allocator land on retail's exact assignment for `d`, the `0x20` constant,
and `lead`/`v1` all at once -- the whole 3-way renaming was one allocation
decision, triggered by whether the second byte's value has its own local
or is read through the same variable used for the first byte's `c`.

Translated to idiomatic C (named the local `trail`, paired with the
existing `lead`, rather than keeping the permuter's `new_var`) and
re-verified through the real oracle:

```
build exit=0
OK: build matches retail
EncodeFullWidthSjis: 31/31 words match (file 0x31820-0x3189C)
```

**Byte-exact. `INCLUDE_ASM` replaced with real C.** Final matched body:

```c
u8 *EncodeFullWidthSjis(u8 *dst, u8 *src) {
    u8 *d;
    u32 c;
    u32 v;
    u32 lead;
    u32 trail;

    if (*src != 0) {
        do {
            d = dst;
            dst++;
            c = *src;
            if (c >= 0x30) {
                lead = 0x82;
            } else {
                lead = 0x81;
            }
            *d = lead;
            d = dst;
            dst++;
            c = *src;
            trail = c;
            if (trail < 0x60 && trail != 0x20) {
                v = trail + 0x1F;
            } else {
                v = trail + 0x20;
            }
            src++;
            *d = v;
        } while (*src != 0);
    }
    *dst = 0;
    return dst;
}
```

### Disposition

**MATCHED, 31/31.** This closes the last live member of the "redundant
cursor cache" class in this unit -- see `DecodeFullWidthSjis.md`'s round-38
section for the corrected final membership (two live members, both now
closed).

### Proposed learning

**A register-identity residue that renames several UNRELATED-looking
values together (here: a pointer copy, a comparison constant, and a
result variable) can be one allocation decision with one trigger, not
three separate register-pinning problems.** The permuter found that
reading a reused byte value through a SECOND local, rather than the
SAME variable already holding the first byte's value, was enough to flip
all three simultaneously. This is a variant of the project's existing
"declared width/reuse affects codegen" family (see this report's own
`u8`-vs-`u32` finding above and `func_8002FAC4`'s `chan` finding), but the
lever here is introducing a SEPARATE variable for a second, unrelated use
of what is conceptually the same kind of value (a decoded byte), not
widening one variable's type. Worth trying on any register-identity stall
where one source variable is read for two textually-separate purposes
across the function body.
