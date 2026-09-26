# TaskObjF__TickStateDelay -- MATCH

> Renamed from `Class86E00_3bb8c_g__TickStateDelay` on 2026-09-23 (tools/rename.py). Address 0x80050280.

> Renamed from `func_80050280` on 2026-09-23 (tools/rename.py). Address 0x80050280.

Unit `class_3bb8c_g`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TaskObjF__TickStateDelay`: 48/48 words match.

## Source

```c
void TaskObjF__TickStateDelay(Class86E00_3bb8c_g *self)
{
    s32 old;
    s32 newVal;

    if (self->unk28 == 7) {
        old = self->unk5C;
        newVal = old + 1;
        self->unk5C = newVal;
        if (old < 6) {
            return;
        }
        self->methods->slot7C(self, 0x13);
    } else if (self->unk28 == 0xB) {
        old = self->unk5C;
        newVal = old + 1;
        self->unk5C = newVal;
        if (old < 6) {
            return;
        }
        self->methods->slot7C(self, 0x14);
    } else if (self->unk28 == 0xF) {
        old = self->unk5C;
        newVal = old + 1;
        self->unk5C = newVal;
        if (old < 6) {
            return;
        }
        self->methods->slot7C(self, 0x15);
    }
}
```

## Derivation: three attempts, two different mistakes

The raw disassembly reads backwards on a quick skim: `slti $v0, $v0, 6`
followed by `bnez $v0, END` looks like "if count < 6, proceed" but is
actually "if count < 6, SKIP" (branch away) -- `m2c` caught this
immediately and is what unblocked the rest. The counter INCREMENT and
STORE happen unconditionally (in the branch's own delay slot), and the
notification call only fires once the counter has reached 6 or more.

- **Attempt 1 (single reused local, 3/48 -- wrong register identity):**
  `v0 = self->unk5C; self->unk5C = v0 + 1; if (v0 < 6) return; ...` put
  the OLD value and the INCREMENTED value in swapped physical registers
  relative to retail ($v0 holds "old" in retail, used for both the
  increment source and the comparison; my single-variable form let the
  compiler pick the opposite assignment). Confirmed via
  `objdump -d build/src/class_3bb8c_g.c.o` before touching anything else,
  per the standing "cross-check the real compiled length/shape" rule.
- **Attempt 2 (separate `old`/`newVal` locals, still wrong -- this time a
  genuine LENGTH mismatch, not a register swap):** giving the incremented
  value its own name fixed the register identity (confirmed byte-for-byte
  against retail's `lw $v0`/`addiu $v1`/`slti $v0` sequence via objdump),
  but the function came out 4 words SHORT. Cause: I had also merged all
  three cases' calls into ONE `self->methods->slot7C(self, arg1);`
  statement after the `if`/`else if` chain (via a shared local variable in
  one sub-attempt, then via a single physical call site in another).
  Retail duplicates BOTH the `self->methods` load AND the `->slot7C` load
  independently in EACH branch, and only the trailing `jalr`+delay-slot
  pair is shared physically (the first two branches literally jump into
  the third branch's own call instruction). A single deferred call
  statement -- whether via a local function-pointer variable or written
  once at the end -- under-duplicates relative to that.
- **Fix:** write `self->methods->slot7C(self, K);` as its own complete,
  separately-written statement in EACH branch (three literal, textually
  near-identical call statements, differing only in the constant). GCC
  2.6.3 still shares the trailing `jalr`/delay-slot pair across all three
  -- this is a case where the earlier "GCC 2.6.3 does NOT cross-jump-merge
  two syntactically identical call+assignment SEQUENCES" finding (round
  13) does not block the merge, because there is no assignment here, only
  a void call; only the final two instructions are bit-identical and
  eligible to share.

## Struct changes (additive, `include/class_3bb8c.h`)

None new -- `slot7C` was already declared while surveying the unit
(`TaskObjF__ReleaseCardIcon`'s report).

### Proposed learning

**A crossjump-mergeable tail is not "one call, deferred through a
function-pointer local" and it is not "duplicate everything" either --
it is "write the full call statement in each branch and let the compiler
decide how much of it to share."** The round-12 "local function pointer
variable" lever (used successfully for `Class86B60__Tick`'s DIFFERENTLY-typed
slots colliding on one call site) is for the case where the branches call
DIFFERENT typed slots; here they call the SAME slot with different
arguments, and forcing it through one shared fetch under-duplicates
relative to what retail's own compiler produced. Diagnose which one
applies from the disassembly's OWN duplication pattern (are the fetch
instructions repeated per branch, or does only the final call repeat?)
before choosing a lever, rather than always reaching for the fanciest
merge idiom on sight of a shared tail.

## Naming

`TaskObjF__TickStateDelay` (was `func_80050280`), tier B: for
three specific states (`7`, `0xB`, `0xF`) increments a per-object counter
(`self->unk5C`) every call and only fires the state transition once the
counter reaches 6 -- read as a per-call (per-frame) wait/delay gate ahead
of a state change, hence "Tick". Which real-world delay this counts down
(and why exactly 6) is not established.
