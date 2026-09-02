# The `New_X` epilogue-merge residue

**Status: OPEN, but NOT a toolchain blocker.** The pinned compiler demonstrably
produces retail's form — we have not found the source that makes it do so. That
distinction matters: the two open items in `docs/research/` (`gp-relative-blocker`,
`addiu-at-blocker`) are cases where the toolchain cannot express what retail
contains. This one is a *search problem over source shapes*, which makes it a
permuter target and explicitly **not** an operator escalation.

## The shape

Every instance is a `New_X` class allocator. Canonical form, from
`func_8004A130` (class_39e08):

```
    ori   $a0, $zero, 0x38          ; sizeof the instance
    jal   func_80017B34             ; the BasicClass-family allocator
     sw   $s0, 0x10($sp)
    addu  $s0, $v0, $zero           ; s0 = self
    beqz  $s0, .Lepilogue
     addu $v0, $zero, $zero         ; <-- RETAIL: v0 = 0 in the DELAY SLOT
    jal   <Get_vtable for the class>
     nop
    addu  $a0, $s0, $zero
    lw    $v0, 0x8($v0)             ; the ctor slot
    addu  $a1, $s1, $zero
    jalr  $v0
     addu $a2, $s2, $zero
    addu  $v0, $s0, $zero           ; v0 = self on the success path
  .Lepilogue:                       ; ONE epilogue, shared by both paths
    ...
    jr    $ra
```

Two exits carrying **different values** (`0` and `self`), merged into **one**
epilogue, with the constant materialized in the branch's delay slot.

## The residue

Every source form tried so far produces `addu $v0, $s0, $zero` in that delay
slot instead of `addu $v0, $zero, $zero`. Value-identical — `$s0` is 0 on that
path — but the wrong **source register**, so it is one word off.

Scores land at 26/27, 23/24, 18/19: always exactly one word.

**This is NOT fixable with `register T v asm("$N")` or an asm operand
constraint.** Both are banned project rules (CLAUDE.md rule 6), and this is the
exact case the rule exists for: the residue *is* a register identity.

## The discriminator, stated positively

Round 2026-09-02 established this, and it is the useful form of the finding:

> **GCC 2.6.3 (Psy-Q), `-O2`, will not merge two function exits that carry
> different values into one epilogue.** Introducing a second `return` — as a
> guard clause, an if/else with a shared trailing return, or a comma-ternary —
> reliably **duplicates the entire epilogue** instead.

So the source shapes fall into two families, and both fail:

| family | source form | result |
| --- | --- | --- |
| **One exit, value from `self`** | `if (self) { ctor(...); } return self;` | One epilogue (right), but the constant is never materialized — delay slot gets `move $v0,$s0`. **26/27.** |
| **Forced single exit** | early `return NULL`; comma-ternary; result variable | Either a **second epilogue** (early-return and comma-ternary are byte-identical: 140396 bytes differ outside range) or an **extra callee-saved register** for the result variable (frame grows `-0x20`→`-0x28`, `sw $s3` appears). **16/27 and 0/27.** |

Retail is neither. It has one epilogue *and* the materialized constant.

The cheap tell that you are in the second family: funcdiff's **"differs OUTSIDE
this range"** warning with a six-figure byte count. That means the function
changed size and every later address shifted — you are not one reshape away,
you are in the wrong family. Spot it and stop.

For the mechanism underneath (`fill_eager_delay_slots` in `reorg.c`, its
`mostly_true_jump` static prediction of `EQ`-against-zero, and why an
`__asm__("")` barrier cannot reach it), see the root-cause hypothesis in
`docs/match-reports/new_class_6d3c8.md`. That analysis is informed inference
against a same-vintage GCC tree, not a read of the exact `gcc-2.6.3-psx`
source, which is not vendored here.

## Attempts already spent — do not re-derive these

Roughly 25 build-and-diff attempts across five functions and four units:

- `new_class_6d3c8` (code_1677c, 23/24) — 14+ attempts. `if`/`goto` reshaping
  both directions, `__asm__("")` in every position, `volatile`.
- `func_80025D10` (two broadcast instances).
- `func_8004A130` (class_39e08, 26/27) — runner: `__asm__("")` after the malloc;
  early return. Head: result variable (0/27); comma-ternary (16/27).
- `func_8004A4C8` (class_3ac78, 26/27) — 5 reshapes, two with size regressions.

Two runners on different units reached this class independently in one round and
classified it identically without either seeing the other's work.

## Corpus census

Mechanical, over every non-`psyq` `.s` in the tree. Signature: a `beqz $sN`
whose delay slot is `addu $v0,$zero,$zero`, with a later `addu $v0,$sN,$zero`.

```sh
python3 - <<'PY'
import re,glob,os
d=re.compile(r'addu\s+\$v0,\s*\$zero,\s*\$zero'); b=re.compile(r'\bbeqz\s+\$s\d')
v=re.compile(r'addu\s+\$v0,\s*\$s\d,\s*\$zero')
for f in glob.glob("asm/nonmatchings/*/*.s")+glob.glob("asm/*.s"):
    n=os.path.basename(f)
    if n.startswith("psyq_") or n=="header.s": continue
    L=open(f,errors="ignore").read().splitlines(); c=0
    for i,l in enumerate(L):
        if b.search(l) and i+1<len(L) and d.search(L[i+1]) \
           and v.search("\n".join(L[i+2:i+30])): c+=1
    if c: print(f"{c:4d}  {f}")
PY
```

**24 instances corpus-wide** as of 2026-09-02:

| where | instances |
| --- | --- |
| carved, queued now | **5** — `func_80049608`, `func_8004A130` (class_39e08); `func_8004A4C8` (class_3ac78); `func_8003B854`, `func_8003BE94` (code_2c054) |
| `class_3bb8c` (uncarved) | 11 |
| `code_2cc8c` (uncarved) | 4 |
| `code_179d8` (uncarved) | 3 |
| `code_d294` (uncarved) | 1 |

Note that three of the five carved instances have **never been attempted** —
they sat in this round's fresh queue below the cut. Their reports do not exist
yet, so `progress.py` counts them FRESH; they are not.

## What to do about it

**Permute one instance, not five.** The search is unusually well-posed: a known
27-word target, a known 26/27 floor to beat, and a residue confined to one
delay slot. `func_8004A130` or `func_8004A4C8` are the best-posed sites — both
are 26/27 with fully-typed headers already committed, so the permuter starts
from a compiling body.

If a permuter run closes it, the source form generalizes immediately to all 24:
they are the same allocator (`func_80017B34`) with a different size constant and
a different `Get_vtable`. If only UB or duplicate-arm forms reach zero, mark the
class permuter-exhausted here and stop paying for it per instance.

**Do not** escalate this as a toolchain lead, and do not spend a further manual
attempt budget on any individual instance. Both have now been done.
