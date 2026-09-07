# func_8005DF9C — MATCHED (byte-exact, whole-image `build exit=0`)

Unit: `Entity_b` · Size: 36 words · Round 23 (2026-09-07), head. Fresh ground
(no prior report).

## The match

```c
/* arg1 is unused here; the canonical declaration in include/Entity.h has it
 * and func_8005DABC passes 0. Do not drop it -- `conflicting types`. */
void func_8005DF9C(Entity *this, s32 arg1) {
    if (D_80089EAB[this->moodIndex * 0x10] < 0 &&
        D_80089EAC[this->moodIndex * 0x10] != 0 &&
        func_8005E02C(this, D_80089EAC[this->moodIndex * 0x10] << 9)) {
        this->methods->slot30(this, 0xA);
    }
}
```

with, locally in `src/Entity_b.c`:

```c
extern s32 func_8005E02C(Entity *this, s32 arg1);
```

Three gates, all branching to the SAME epilogue, so it is one flat `&&` chain
rather than nested `if`s — read off the fact that all three of `bgez`, `beqz`
and the post-call `beqz` target `.L8005E018`.

`D_80089EAC[...]` is written three times and GCC common-subexpression-eliminates
it to one `lb`; no local is needed and adding one is not what retail did. Both
table reads share the single `sll $a1, $v0, 4` row index, likewise from CSE.

`func_8005E02C`'s return is tested with `beqz` straight off the `jal`, so it is
value-returning, not `void`.

## The one thing that cost an attempt: a canonical declaration with an UNUSED parameter

`func_8005DF9C` takes **two** arguments. The second is never read — `$a1` is
clobbered by the row index on the first instruction that uses it — so nothing
in this function's own disassembly reveals it exists. Writing the obvious
one-parameter signature is a **`conflicting types` compile error** against the
long-standing `extern void func_8005DF9C(Entity *this, s32 arg1);` in
`include/Entity.h`, put there by whoever matched the caller
(`src/Entity.c` passes `func_8005DF9C(this, 0)`).

**And that error produces ZERO hits on `error:` and `parse error`** — it was
caught only by the `\*\*\* \[[^]]*\.o\]` alternative added to the oracle grep in
round 21. Without it the build would have exited 2 with no diagnostic and
funcdiff would have reported a full match from the previous build (the function
was still `INCLUDE_ASM` then). The staleness guard did also fire, but as the
documented BACKSTOP.

### Proposed learning

**A function's parameter LIST is not fully recoverable from its own body, and
the canonical declaration is the evidence — match it rather than deriving it.**
An argument the callee never reads leaves no trace: the caller passes it, the
callee's disassembly is silent, and the natural signature is wrong. This is the
argument-count sibling of the existing rule that a byte match tells you nothing
about a one-line wrapper's RETURN type.

So before writing a signature, `grep -rn '<func>' include/ src/` and adopt any
declaration you find verbatim. Round 23 hit this twice in one session — here,
and on `func_80013348` where a `const` qualifier the canonical declaration
lacked produced the identical error class — and both times the error was
invisible to the text patterns in the oracle grep. **That makes this a
compile-time argument for keeping `\*\*\* \[[^]]*\.o\]` in the chain**, not a
style preference: the two cheapest signature mistakes available both fail
silently without it.
