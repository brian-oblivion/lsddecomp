# maspsx: `is_reorder` ignores maspsx's own emitted `.set noreorder`

**Status: OPEN toolchain lead. Operator's call. NOT blocking any work — the
source-level workaround is known, safe and already used in matched code.**

Found by round 13's runner charlie while matching `TransformAndCullPoly` (a GTE
hand-written function). **Independently reproduced in isolation by the head
before escalating**, per CLAUDE.md's rule that a lead reaches the operator only
with a reproducer attached. Both halves below reproduce in under a second
through the pinned pipeline.

## The behaviour

maspsx inserts a defensive `nop` after any real branch/jump mnemonic when it
believes the assembler is in reorder mode. It tracks that belief in a single
flat flag, `self.is_reorder`, updated by literally matching each *processed*
line against `.set\tnoreorder` / `.set\treorder` — a real TAB on both sides.

maspsx's own `.ent` handling **emits** a `.set noreorder` line into the output
for gas's benefit, but does not route that line back through `process_line`.
So `is_reorder` stays at its default `True` for the whole of a hand-written
`__asm__` block, even though the emitted stream it is producing already says
`noreorder`.

Two consequences, and the second is the nastier one.

### 1. An unbracketed branch loses its delay slot

`/tmp/t_nobracket.c`:

```c
void probe(void) {
    __asm__ volatile (
        "beq $3, $2, 1f\n\t"
        " ori $2, $zero, 0x1\n\t"
        "sw $2, 0x78($5)\n\t"
        "1:\n\t"
        "nop\n\t");
}
```

Piped through `cpp | cc1 | maspsx.py` (exact invocation in CLAUDE.md's
"Escalate, do not experiment"), the output is:

```
.set	noreorder          <- maspsx emitted this itself, line 13
...
beq $3, $2, 1f
nop  # DEBUG: branch/jump  <- INSERTED
ori $2, $zero, 0x1         <- displaced OUT of the delay slot
sw $2, 0x78($5)
```

`ori` was written as the delay-slot instruction and unconditionally executes
in real MIPS; after insertion it executes only on the fallthrough path. That
is a **control-flow semantic change**, not a scheduling difference. Note the
`.set noreorder` already present at line 13 of maspsx's own output — it is
inserting the nop anyway, which is what pins the cause on `is_reorder` rather
than on the directive being absent.

Adding the block's own tab-delimited bracket suppresses it:

```c
".set\tnoreorder\n\t"   /* first line of the block */
...
".set\treorder\n\t"     /* last line */
```

```
beq $3, $2, 1f
ori $2, $zero, 0x1     <- delay slot preserved, no nop
```

`beqz`/`bnez` are untouched, but **not for any reason about the code** — only
the real `beq` is in maspsx's `branch_mnemonics` list; the pseudo-forms are
simply absent from it. Do not read the pseudo-forms working as evidence that
the block is fine.

### 2. Leaving the bracket unclosed eats cc1's OWN epilogue nop

The flag has no scope — it is flat, not a stack — so `noreorder` asserted
inside an asm block persists past the end of the block. cc1 emits its trailing
`j $31` with no nop of its own, relying on reorder mode to supply one.
`/tmp/t_unclosed.c`, identical but with the closing `.set\treorder` removed:

```
beq $3, $2, 1f
ori $2, $zero, 0x1
1:
nop

j	$31
.end	probe          <- no nop after j $31
```

Compare the correctly-closed version, where `j $31` is followed by
`nop  # DEBUG: branch/jump`. Unclosed, the function ends on a bare jump and
**the next function's first word occupies what should be the delay slot.**
Charlie hit this as a real 57/58 residue before diagnosing it, and it is the
reason the closing bracket is not optional.

## Why this is a lead and not a rule change

The workaround is entirely source-level: bracket the block. That is a source
construct, available to any runner, and `TransformAndCullPoly`, `ProjectTriFace` and
`ProjectQuadFace` are byte-exact with it. **Nothing is blocked**, so this does
not join the two open blockers in urgency.

What is worth the operator's attention is the shape of the failure, because it
is silent in the direction that matters: an unbracketed block assembles, links,
and produces a function that is *nearly* right, so it presents as an ordinary
one-word residue in the function you are working on — or, in the unclosed case,
as damage to the FOLLOWING function, which is somebody else's already-matched
code. Neither presents as a maspsx diagnostic.

Candidate remedies, none tested, all the operator's call:

- Route maspsx's own `.ent`-emitted `.set noreorder` through `process_line` so
  `is_reorder` reflects the stream maspsx is actually producing.
- Give `is_reorder` a scope/stack so a block-local `noreorder` cannot leak past
  the block.
- Leave the tool alone and keep the source-level bracket as the documented
  requirement. **This is the current de facto state and it works.**

**Do not bump the maspsx version as the remedy** — the same caution as
`docs/research/addiu-at-blocker.md`. The pinned version is load-bearing for the
retail bytes, and this behaviour is a tracking bug with a working source
workaround, not a missing feature.

## Reproducing

Both files are inlined above in full; no path into anyone's checkout is
required. The pipeline:

```sh
tools/gcc263/cpp -Iinclude -Iinclude/psyq -undef -lang-c -nostdinc -Dmips -D__GNUC__=2 /tmp/t.c \
  | tools/gcc263/cc1 -mips1 -mcpu=3000 -quiet -G0 -O2 \
  | .venv/bin/python3 tools/maspsx/maspsx.py --aspsx-version=2.34 --dont-force-G0 --expand-div
```

Grep the output for `# DEBUG: branch/jump` — that comment is maspsx labelling
its own insertions, and it is the fastest way to see whether any block in a
unit is affected.
