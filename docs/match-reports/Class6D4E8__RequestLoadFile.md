# Class6D4E8__RequestLoadFile — MATCHED (48/48 words)

> Renamed from `func_80027C80` on 2026-09-17 (tools/rename.py). Address 0x80027c80.

Round 45, runner echo (third sitting), `src/code_179d8_q.c`. This class's own
slot +0x06C of `D_8006D4E8`.

## Result

Byte-exact, first attempt.

The body below is the round-51 source, after track-3 naming. The
derivation notes that follow were written in round 45 against the same
code under its `unk` names; only names changed, the image is
byte-identical, and the `## Naming` section at the end of this report
carries the evidence for each one.

```c
/* The class's method table down to +0x058: the one slot
 * Class6D4E8__RequestLoadFile dispatches. `tools/classtable.py D_8006D4E8`
 * resolves that slot to func_80027800 (code_179d8_s), which loads a named
 * file off the disc, so the slot is named for the method it dispatches to.
 * The sibling class D_8006D430 (include/code_171e0.h's
 * Class6D430Methods) leaves the identical offset unnamed -- this
 * stays an independent local view, per the project's multiple-local-views
 * convention, rather than an edit to that shared header. */
typedef struct Methods6D4E8_C80 Methods6D4E8_C80;
struct Methods6D4E8_C80 {
    u8 pad00[0x58];
    /* +0x58 */ void (*loadFile)(void *self, char *name);
};

typedef struct Obj6D4E8_C80 Obj6D4E8_C80;
struct Obj6D4E8_C80 {
    /* +0x00 */ Methods6D4E8_C80 *methods;
    u8 pad04[0x22 - 0x04];
    /* +0x22 */ u16 pendingRequests; /* ++ per queued request, -- per cancel */
    /* +0x24 */ s32 flags;           /* OR-ed bit set; no bit is read here */
};

/* +0x04 of whatever object a still-uninitialized local $s2 points at on this
 * path -- see the Class6D4E8__RequestLoadFile report for why that local is
 * never assigned. Only the one field this store touches is typed, and the
 * object's identity is unknowable from here, so the name stays a
 * placeholder. */
typedef struct UnkC80 UnkC80;
struct UnkC80 {
    u8 pad00[0x04];
    /* +0x04 */ s32 unk04;
};

/* The request op codes are a small enumeration shared with code_179d8_s,
 * which enqueues 2 (open by name), 3 (close), 4 (seek) and 5 (read) from the
 * class's other slots. Only the one this unit itself uses is named. */
#define CD_OP_LOAD_FILE 7

struct Obj6D4E8_282AC;
extern void EnqueueCdRequest(struct Obj6D4E8_282AC *owner, s32 fileIndex,
                             s32 op, s32 param0, s32 param1);
extern s32 FindCdFileIndex(char *name); /* code_179d8_r: name -> table index */
extern s32 gCdAsyncEnabled;

void Class6D4E8__RequestLoadFile(Obj6D4E8_C80 *self, char *name)
{
    UnkC80 *s2;
    s32 idx;

    LockCd();

    if (name != NULL) {
        if (gCdAsyncEnabled != 0) {
            s2->unk04 = 1;
            idx = FindCdFileIndex(name);
            EnqueueCdRequest((struct Obj6D4E8_282AC *)self, idx,
                             CD_OP_LOAD_FILE, 0, 0);
        } else {
            self->methods->loadFile(self, name);

            if (self->pendingRequests == 0) {
                self->flags |= 4;
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
jal   FindCdFileIndex
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
- `FindCdFileIndex` (still `INCLUDE_ASM` in `code_179d8_r.c`, foxtrot's unit)
  takes a single `char *` argument that it passes straight to `strstr` as
  the needle — read from its own disassembly, not guessed — hence `char
  *arg0` here rather than `void *`. `arg1` of `Class6D4E8__RequestLoadFile` is typed the
  same way, since it flows unchanged into both `FindCdFileIndex` and
  `self->methods->loadFile` (spelled `slot58` when this was written).
- `EnqueueCdRequest` (already matched this round, later in this file) takes its
  first parameter as a distinct locally-typed `Obj6D4E8_282AC *` (`Self800282AC` when this was written). Rather than
  editing its existing declaration or pulling that type earlier in the file
  out of ROM order, this function forward-declares an opaque `struct
  Obj6D4E8_282AC;` and casts `self` to `struct Obj6D4E8_282AC *` at the call
  site — the tag is the same one `EnqueueCdRequest`'s own definition later
  completes with `typedef struct Obj6D4E8_282AC Obj6D4E8_282AC;`, so the
  prototypes are identical types and nothing conflicts.
- `self`'s own `+0x58` method-table slot is the identical offset the sibling
  class `D_8006D430` leaves as an unnamed pad in
  `include/code_171e0.h`'s `Class6D430Methods` ("Class6D430__AllocBuffer's own
  slot, unused here"). Rather than editing that shared header — which
  `code_179d8_h.c` and `code_171e0.c` also include —
  this unit keeps its own local view (`Methods6D4E8_C80`/`Obj6D4E8_C80`), per the
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

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027C80` | `Class6D4E8__RequestLoadFile` | B |

**Evidence.** Slot `+0x06C` of `D_8006D4E8` (`tools/classtable.py`). Given a
file name, it either enqueues a `CD_OP_LOAD_FILE` (7) request through
`EnqueueCdRequest` with `FindCdFileIndex`'s file-table index, or -- when
`gCdAsyncEnabled` is 0 -- calls the class's own `+0x058` slot
(`func_80027800`, code_179d8_s) directly, which is the synchronous
load-this-file-by-name method that enqueues the identical op 7 on its own
async path. So both arms request the same thing, which is what `Request`
names; `LoadFile` is the op, read off the slot it dispatches to and off the
op code it enqueues.

**Why tier B, not A.** The behaviour is established, but the class's identity
is not: `Class6D4E8` is a placeholder token for the table address, following
the existing `Class6B5CC__RotateLocalVector` convention in the symbols file.
The unit as a whole is demonstrably the CD-ROM read driver, but no evidence
here says what the developers called this class -- see
`GetClass6D4E8Methods.md`.

**Names left alone.** `UnkC80` and its `unk04`: the store goes through a
register the function never assigns (the shipped bug this report documents),
so the object it lands in is not identifiable from any path. A name here
would be invention.

## Proposed field names

**APPLIED by the head at merge, round 52** -- all four fields, both types
and all five vtable slots below are now in the tree, each one applied
separately with `./build-and-verify.sh` green and byte-exact after it. One
mis-hit had to be resolved by receiver type: `src/code_179d8_h.c:143`
accesses `pendingGeneration` on a `Class6D430 *self`, while the same file's
lines 97/174/175/182 are its OWN `ObjA34_179D8H::unk0C` and were left alone.
The compiler named that mis-hit (`structure has no member named 'unk0C'`),
which is the procedure working in the direction where it can work.

Applied in this unit (all three of its object views are local to the `.c`).
The SAME physical fields carry `unk` names in two sibling units' own local
views; proposing rather than renaming, since those units are not mine:

| unit | type | field | proposed | tier | evidence |
| --- | --- | --- | --- | --- | --- |
| code_179d8_s | `Obj80027480` | `unk22` | `pendingRequests` | A | `EnqueueCdRequest` increments it per queued request; `Class6D4E8__CancelRequests` decrements it once per node it unlinks |
| code_179d8_s | `Obj80027480` | `unk24` | `flags` | A | only ever `|=` a bit (4 here, 0x200 in `func_80027800`) or cleared |
