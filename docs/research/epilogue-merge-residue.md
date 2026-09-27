# The `New_X` epilogue-merge residue

**Status: LARGELY CLOSED, 2026-09-02. No permuter was needed.** The rule is
that **`return NULL;` must come textually LAST, after the success return.**
Five instances matched byte-exact in one pass on that basis. One sub-shape
remains open — see "What survived" at the bottom, and read
`docs/match-reports/New_StageMap.md` for the canonical account and the
measurement table.

The framing below was written while the class was open. It is kept because its
census and its catalogue of failing shapes are still accurate and still
useful — but **its central discriminator is wrong**, and the section marked
"The discriminator, stated positively" says the opposite of what is true.
Corrections are inline.

## The shape

Every instance is a `New_X` class allocator. Canonical form, from
`New_TimedTask` (class_39e08):

```
    ori   $a0, $zero, 0x38          ; sizeof the instance
    jal   BMemPMgrAlloc             ; the BasicClass-family allocator
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

**THAT CLAIM IS FALSE, and it is the reason this class stayed open through
~25 attempts and four confirmations.** GCC 2.6.3 merges two such exits
readily. What it is sensitive to is the **textual order of the two returns**,
which none of the shapes tested here varied:

- `return NULL;` **first** (a guard clause) — the NULL block is laid out after
  the body and reached by a `j`. This is the "duplicated epilogue" the claim
  describes, and all three failing forms below are variants of it.
- `return NULL;` **last**, after the success return — the NULL path's value
  materialization lands in the branch delay slot and the branch retargets
  straight to the shared epilogue. **This is retail.**

The three failing families were all sampling the same half of the space. The
lesson is not about the compiler; it is that "I tried several shapes" is only
evidence if the shapes differ along the axis that matters.

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
`docs/match-reports/New_GameApplication.md`. That analysis is informed inference
against a same-vintage GCC tree, not a read of the exact `gcc-2.6.3-psx`
source, which is not vendored here.

## Attempts already spent — do not re-derive these

Roughly 25 build-and-diff attempts across five functions and four units:

- `New_GameApplication` (code_1677c, 23/24) — 14+ attempts. `if`/`goto` reshaping
  both directions, `__asm__("")` in every position, `volatile`.
- `Pad__DispatchEvents` (two broadcast instances).
- `New_TimedTask` (class_39e08, 26/27) — runner: `__asm__("")` after the malloc;
  early return. Head: result variable (0/27); comma-ternary (16/27).
- `New_StageMap` (class_3ac78, 26/27) — 5 reshapes, two with size regressions.

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
| carved, queued now | **5** — `New_DayTask`, `New_TimedTask` (class_39e08); `New_StageMap` (class_3ac78); `New_StreamTask`, `New_TaskCore` (Task) |
| `class_3bb8c` (uncarved) | 11 |
| `Task` (uncarved) | 4 |
| `code_179d8` (uncarved) | 3 |
| `code_d294` (uncarved) | 1 |

Note that three of the five carved instances have **never been attempted** —
they sat in this round's fresh queue below the cut. Their reports do not exist
yet, so `progress.py` counts them FRESH; they are not.

## What survived, and what to do now

**Do not run a permuter on this.** The recommendation that used to stand here —
permute `New_TimedTask` or `New_StageMap` — was based on the false
discriminator above. Both matched by hand, first attempt, once the statement
order was the thing being varied.

### Closed: the `move $v0, $zero` sub-shape

Five instances, all byte-exact, all with `return NULL;` last:

| function | unit | words |
| --- | --- | --- |
| `New_StageMap` | class_3ac78 | 27/27 |
| `New_TimedTask` | class_39e08 | 27/27 |
| `New_DayTask` | class_39e08 | 31/31 |
| `New_StreamTask` | Task | 36/36 |
| `New_TaskCore` | Task | 31/31 |

### Still open: the `nop` sub-shape

`New_GameApplication` (code_1677c, 23/24) is **not** closed, and it is a different
animal. Retail leaves the `beqz` delay slot as a bare `nop` and materializes
nothing at all, relying on `$v0` still holding the allocator's own zero return.
There is no second return expression to reorder. Both the rule above and a
`return self;`-on-the-null-path variant were measured against it and rejected.

**So screen an instance before assuming the rule applies:**

```sh
grep -A1 'beqz' asm/nonmatchings/<unit>/<func>.s | grep -E 'addu *\$v0, *\$zero, *\$zero|nop'
```

`addu $v0, $zero, $zero` in the delay slot → the rule closes it, expect an
ordinary match. A bare `nop` → the open sub-shape; do not spend a budget on it
without a new idea.

### The remaining 19 instances

The census below counts 24 corpus-wide. Five are closed and one is the open
sub-shape, leaving **18 in still-uncarved segments** (`class_3bb8c` 11,
`Task` 4, `code_179d8` 3, `code_d294` 1). Each should be a near-free
match the moment its segment is carved, provided it screens as the
`move $v0, $zero` shape. That makes those four segments materially more
attractive as carve targets than their raw function counts suggest.
