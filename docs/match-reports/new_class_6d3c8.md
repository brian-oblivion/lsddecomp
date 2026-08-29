# new_class_6d3c8

**Unit:** code_1677c · **Size:** 24 words (0x60 bytes) · **Status:** STALLED 23/24

## What it does

The `New_X` allocator for the class whose method table is `D_8006D3C8`
(resolved with `tools/classtable.py 0x8006D3C8 --vs 0x8006B58C`): allocates
a 0x2C-byte instance via `func_80017B34` (the game's allocator, elsewhere
inside `code_8220.s`), and on success calls the class's constructor —
slot `+0x008` of its own method table, `func_80025FDC` — through the table
fetched from `func_800269E0()`. Returns the allocated pointer regardless of
whether the constructor ran (matches the `New_DreamSys` shape documented in
`docs/research/class-framework.md`).

`func_800269E0` is a plain accessor with **no parameters** — confirmed from
its own disassembly (`asm/nonmatchings/code_171e0/func_800269E0.s`, owned by
another unit): it never reads `$a0`, just returns `&D_8006D3C8`. The call
site doesn't set up `$a0` before calling it either — `$a0` is only prepared
*after* the call returns, for the subsequent `jalr`. This is the same shape
as the `Get_vtable_DreamSys()` example in class-framework.md.

## Derivation

```
addiu $sp, $sp, -0x20
sw    $s1, 0x14($sp)
addu  $s1, $a0, $zero      ; s1 = arg
ori   $a0, $zero, 0x2C
sw    $ra, 0x18($sp)
jal   func_80017B34         ; malloc(0x2C)
 sw   $s0, 0x10($sp)
addu  $s0, $v0, $zero        ; s0 = self
beqz  $s0, .L80025FC4
 nop                          ; <-- RETAIL: nop.  MINE: "move v0,s0" (residue)
jal   func_800269E0()         ; note: no args set up for this call
 nop
addu  $a0, $s0, $zero          ; a0 = self, for the NEXT call
lw    $v0, 0x8($v0)             ; ctor slot
nop
jalr  $v0
 addu $a1, $s1, $zero            ; a1 = arg
addu  $v0, $s0, $zero             ; v0 = self (ctor's return value is discarded)
.L80025FC4:
...
jr    $ra
```

Written as:

```c
Class6D3C8 *new_class_6d3c8(void *arg) {
    Class6D3C8 *self = func_80017B34(0x2C);

    if (self != 0)
        func_800269E0()->ctor(self, arg);
    return self;
}
```

This reproduces every instruction **except one**: retail leaves the `beqz`
delay slot as `nop`; every C shape I tried compiles it to `move $v0, $s0`
instead — a byte-identical-value but extra/duplicate copy of the same
computation retail performs anyway at the merge point (0x167c0, right after
the `jalr`). 23 of 24 words match; the one miss is purely a delay-slot
filler choice, never a register-identity change.

## Residue

**Class: instruction order / delay-slot filler duplication — not a register
mismatch.** Confirmed via `tools/asm-differ/diff.py new_class_6d3c8`: the
only diff line is `nop` (target) vs `move v0,s0` (mine) at the branch's
delay slot; every other instruction, including the *real* `move v0,s0` at
0x167c0, matches exactly.

None of the following changed the result (all still 23/24, or made it worse
by duplicating the epilogue):

- `if (self != NULL) { ... } return self;` in every spelling I tried
  (`!= NULL`, `!= 0`, `if (self)`, braces vs no braces, `self` vs `void *`
  vs `Class6D3C8 *` typed, ctor field typed to return `void` vs
  `Class6D3C8 *`, discarding vs. assigning the ctor's return, a separate
  `vtable` temporary at function scope or block scope).
- `goto done; ... done: return self;` — control-flow-equivalent to the
  above, same result (confirms this isn't about early-return vs.
  fall-through; it's the same after GCC's jump optimization).
- An early `return self;` / `return NULL;` inside the `if` — this does NOT
  reproduce a single shared epilogue the way retail's does; GCC 2.6.3
  emitted a **second, separate** epilogue instead (function grew from 24 to
  30+ words, diff went to 14/24 with a "differs outside range" warning).
  So retail's single-epilogue shape is confirmed correct; the extra
  instruction is not compensating for a missing merge.
- A bare `__asm__("")` and a stronger `__asm__ volatile ("" ::: "memory")`
  placed immediately after the malloc call, and again as the first
  statement inside the `if` body — **no effect on this particular
  instruction**, in either position.

**Root-cause hypothesis** (read against GCC's `reorg.c`, from a same-vintage
old-GCC tree — `southpark-decomp/tools/build-gcc-pm-sp/gcc-papermario/
reorg.c` — the actual `gcc-2.6.3-psx` source isn't vendored in this repo,
so this is informed inference, not a confirmed read of the exact compiler):
this is `fill_eager_delay_slots`, not ordinary instruction scheduling, which
is why an `asm("")` blockage (meant for the ordinary scheduler) doesn't
touch it. Its static branch predictor treats an `EQ`-against-zero condition
(`mostly_true_jump`, the `case EQ: return 0;` arm) as "probably not taken",
which routes it to try stealing delay-slot filler material from the
**fallthrough** thread first (`fill_slots_from_thread` with
`own_fallthrough`). Walking that thread, `s0` (self) is callee-saved and
untouched by the intervening calls, so `move v0,s0` — the same instruction
retail keeps once, at the true merge point — is a legal, side-effect-free
steal, and it duplicates rather than moves because the merge point still
needs its own copy on the branch-not-taken path... except retail's own
branch-not-taken path is exactly this same fallthrough-owned block, so by
this reasoning retail should hit the identical steal. It doesn't, which
means either `redundant_insn` (same file) found this exact copy *already
implied* before the branch in retail's RTL and skipped re-emitting it, or
some earlier optimizer pass (constant/copy propagation ahead of `reorg.c`)
shaped retail's RTL slightly differently in a way that isn't visible in the
final assembly for any instruction except this one. I could not find a
source-level lever that reliably suppresses or reproduces that shaping in
14+ real build-and-diff attempts.

## Preserved body

```c
#if 0
typedef struct Class6D3C8 Class6D3C8;

typedef struct Class6D3C8Methods {
    s32 header;
    void *unk04;
    Class6D3C8 *(*ctor)(Class6D3C8 *self, void *arg);       /* +0x008 func_80025FDC */
    void *unk0C;                                            /* +0x00C func_8003B024 */
    /* ... see include/Class6D3C8.h for the rest, already committed. */
} Class6D3C8Methods;

struct Class6D3C8 {
    Class6D3C8Methods *methods;
    u8 unk04[0x1C];
    void *arg;
    s32 unk24;
    void *dreamSys;
};

extern Class6D3C8 *func_80017B34(s32 size);
extern Class6D3C8Methods *func_800269E0(void);

Class6D3C8 *new_class_6d3c8(void *arg) {
    Class6D3C8 *self = func_80017B34(0x2C);

    if (self != 0)
        func_800269E0()->ctor(self, arg);
    return self;
}
#endif
```

(`include/Class6D3C8.h`, already committed and used by other stalls in this
unit, has the full struct — only the `ctor` slot is typed concretely so far.)

## Proposed learning

`New_X` allocator wrappers (`malloc` → null check → call ctor through the
class's own method-table slot `+0x008` → return the allocation regardless of
the ctor's own return value) are likely to recur across most/all of this
game's ~60 classes, per class-framework.md. This exact shape hit a **single
delay-slot residue that no amount of `if`/`goto`/temp-variable restructuring
or `__asm__("")` barrier fixed** — worth trying a permuter pass (once set up)
on the very first `New_X` that reaches this point, rather than re-deriving
the same 20+ manual attempts per class. If a permuter run ever finds a
source shape that closes this, promote it here immediately; it would
unblock every other `New_X` wrapper in the game at once.
