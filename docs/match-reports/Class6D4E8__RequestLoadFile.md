> Renamed from `func_80027C80` on 2026-09-17 (tools/rename.py). Address 0x80027c80.

# Class6D4E8__RequestLoadFile — MATCHED (48/48 words)

Round 45, runner echo (third sitting), `src/code_179d8_q.c`. This class's own
slot +0x06C of `D_8006D4E8`.

## Result

Byte-exact, first attempt.

```c
/* This class's own +0x058 slot. The sibling class D_8006D430 (see
 * include/code_171e0.h's UnkFlagsObjMethods_171e0) has the identical slot
 * unnamed as "func_80026B08's own slot, unused here" -- kept as an
 * independent local view here, per the project's multiple-local-views
 * convention, rather than editing that shared header (code_179d8_h.c and
 * code_171e0.c also include it this round). */
typedef struct SelfC80Methods SelfC80Methods;
struct SelfC80Methods {
    u8 pad00[0x58];
    /* +0x58 */ void (*slot58)(void *self, char *arg1);
};

typedef struct SelfC80 SelfC80;
struct SelfC80 {
    /* +0x00 */ SelfC80Methods *methods;
    u8 pad04[0x22 - 0x04];
    /* +0x22 */ u16 unk22;
    /* +0x24 */ s32 unk24;
};

/* +0x04 slot of whatever object a still-uninitialized local $s2 points at
 * on this path -- see below for why that local is never assigned; only the
 * one field this store touches is typed. */
typedef struct UnkC80 UnkC80;
struct UnkC80 {
    u8 pad00[0x04];
    /* +0x04 */ s32 unk04;
};

struct Self800282AC;
extern void EnqueueCdRequest(struct Self800282AC *arg0, s32 arg1, s32 arg2,
                           s32 arg3, s32 arg4);
extern s32 func_800284C4(char *arg0); /* code_179d8_r */
extern s32 D_8008A85C;

void Class6D4E8__RequestLoadFile(SelfC80 *self, char *arg1)
{
    UnkC80 *s2;
    s32 idx;

    LockCd();

    if (arg1 != NULL) {
        if (D_8008A85C != 0) {
            s2->unk04 = 1;
            idx = func_800284C4(arg1);
            EnqueueCdRequest((struct Self800282AC *)self, idx, 7, 0, 0);
        } else {
            self->methods->slot58(self, arg1);

            if (self->unk22 == 0) {
                self->unk24 |= 4;
            }
        }
    }

    UnlockCd();
}
```

## The uninitialized-`$s2` hypothesis: CONFIRMED

The previous sitting flagged this function as carrying "a genuine
uninitialized-local-pointer game bug: a store to a `$s2`-based address where
`$s2` is never assigned anywhere in the function." That is exactly what
retail's disassembly shows and exactly what reproducing it (rather than
working around it) needed:

```
ori   $v0, $zero, 0x1
jal   func_800284C4
 sw   $v0, 0x4($s2)      <-- $s2 never loaded/assigned anywhere in this function
```

`$s2` is callee-saved (spilled at `0x20($sp)` in the prologue, restored in
the epilogue) but never written on ANY path inside the function — it carries
whatever the caller happened to leave in `$s2`, and this function stores `1`
through offset `+0x4` of that garbage address. The register is genuinely
never assigned; this is not an artifact of some branch this run failed to
trace.

Reproducing it was direct, not a workaround: declare a local pointer
(`UnkC80 *s2;`) with no initializer, and use it exactly once, at the exact
point retail does (`s2->unk04 = 1;`), inside the one `if` branch where retail
performs the store. No `volatile`, no fake initializer, no inline asm — the
project's rule against "fixing" this with those tools was respected by not
touching it at all. GCC 2.6.3's register allocator picked `$s2` on its own:
`$s1` and `$s0` were already claimed by `self` and `arg1` (the two
parameters, both live across the whole function), so `s2` — used only inside
the nested `if`, with no other live local competing for a callee-saved slot
at that point — got the next free one. That is exactly retail's own
allocation, and the whole-image SHA1 matched on the first build.

**Verdict for the round-45 assignment: the hypothesis is a genuine
uninitialized-local-pointer bug in the shipped game, and it is ordinary,
matchable C.** No path in this function ever assigns anything that could
plausibly become `s2`'s value; there is no missed branch, no forgotten load.
The fix was simply to write the C that has an uninitialized local and let
the compiler place it, exactly as CLAUDE.md's per-function loop already
allows.

## Other derivation notes

- The early `if (arg1 != NULL)` guard corresponds to retail's `beqz $s0,
  .L80027D1C` jumping straight to the shared `UnlockCd(); return;` tail
  — a single early-return-free `if` wrapping the whole body reproduces this
  with no duplicated tail, the same shape CLAUDE.md/prior reports document
  for this unit.
- `func_800284C4` (still `INCLUDE_ASM` in `code_179d8_r.c`, foxtrot's unit)
  takes a single `char *` argument that it passes straight to `strstr` as
  the needle — read from its own disassembly, not guessed — hence `char
  *arg0` here rather than `void *`. `arg1` of `Class6D4E8__RequestLoadFile` is typed the
  same way, since it flows unchanged into both `func_800284C4` and
  `self->methods->slot58`.
- `EnqueueCdRequest` (already matched this round, later in this file) takes its
  first parameter as a distinct locally-typed `Self800282AC *`. Rather than
  editing its existing declaration or pulling that type earlier in the file
  out of ROM order, this function forward-declares an opaque `struct
  Self800282AC;` and casts `self` to `struct Self800282AC *` at the call
  site — the tag is the same one `EnqueueCdRequest`'s own definition later
  completes with `typedef struct Self800282AC Self800282AC;`, so the
  prototypes are identical types and nothing conflicts.
- `self`'s own `+0x58` method-table slot is the identical offset the sibling
  class `D_8006D430` leaves as an unnamed pad in
  `include/code_171e0.h`'s `UnkFlagsObjMethods_171e0` ("func_80026B08's own
  slot, unused here"). Rather than editing that shared header — which
  `code_179d8_h.c` (charlie, this round) and `code_171e0.c` also include —
  this unit keeps its own local view (`SelfC80Methods`/`SelfC80`), per the
  project's multiple-independent-local-views convention.

### Proposed learning

**When a report says a store goes through a register that "is never
assigned anywhere in the function," verify that claim directly against the
disassembly (no `lw`/`la`/`addu` into that register on any path), then
reproduce it as a genuinely uninitialized local — do not treat it as a
blocker.** GCC 2.6.3's register allocation for such a variable is
deterministic given the other locals' liveness (here: the next free
callee-saved register after the parameters already occupying `$s0`/`$s1`),
so an uninitialized local in the right place in the C source lands in the
same register retail's shipped bug used. This is distinct from a permuter
candidate that *branches* on an uninitialized read (disqualified as
scorer-exploiting UB); a straight-line store through an uninitialized
pointer, on a path retail demonstrably takes unconditionally once entered,
is just matching the code that shipped.
