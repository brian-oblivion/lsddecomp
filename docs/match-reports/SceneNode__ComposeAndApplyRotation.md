# SceneNode__ComposeAndApplyRotation — MATCHED

> Renamed from `Class6B5CC__ComposeAndApplyRotation` on 2026-09-26 (tools/rename.py). Address 0x8001d950.

> Renamed from `func_8001D950` on 2026-09-18 (tools/rename.py). Address 0x8001d950.

Unit: `code_d294`. Round 13, runner delta. 54/54 words, full match.

## Signature

```c
void SceneNode__ComposeAndApplyRotation(SceneNodeObj *self, void *arg1, void *arg2, void *arg3, s32 count);
```

`count` is the 5th argument, passed on the incoming stack (`lw $a2, 0x78($sp)`
inside the function — the caller's stack-arg slot, at `newsp + 0x68(own frame)
+ 0x10`).

## What it does

```c
void SceneNode__ComposeAndApplyRotation(SceneNodeObj *self, void *arg1, void *arg2, void *arg3, s32 count) {
    u8 buf2[0x20];
    u8 buf1[0x20];
    UnkOwner_d294 *node;

    self->methods->slot84(self, buf1, 1);

    node = self->unkC;
    if (node != NULL) {
        do {
            node->methods->slot84(node, buf2, 1);
            func_80015BFC(buf2, buf1);
            node = node->next;
        } while (node != NULL);
    }

    ApplyMatrixToSVArray(arg2, arg3, count, buf1);
    if (arg1 != NULL) {
        ApplyMatrixToSVArray(arg1, arg1, 1, buf1);
    }
}
```

Fills `buf1` (a 0x20-byte, MATRIX-shaped buffer) from `self`'s own `+0x84`
vtable slot, then walks the `self->unkC` intrusive list (each node's own
`+0x84` slot filling `buf2`, folded into `buf1` via `func_80015BFC` — a PsyQ
matrix-compose primitive that loads its first argument into GTE control
regs 0-4 via `ctc2`), then uses `buf1` as `ApplyMatrixToSVArray`'s own "out"
argument twice: once for `(arg2, arg3, count)`, once more for
`(arg1, arg1, 1)` when `arg1` is non-NULL.

## New class knowledge

- `self->unkC` (`UnkOwner_d294 *`) is walked as a **singly-linked list**:
  each node has its own self-referential `next` pointer at `+0x00C` —
  MEASURED (`lw $s0, 0xC($s0)` with a `bnez` back-edge). This did not
  contradict the existing "owner" reading of `unkC`; it extends it.
- Both `SceneNodeObj` and `UnkOwner_d294` have a vtable slot at the exact
  same offset `+0x084`, same `(self, void *out, s32 flag)` shape. Read as a
  shared-ancestor method neither class overrides differently at this slot,
  though the ancestor itself is out of this unit's reach.
- `func_80015BFC` (`the 0x2258..0x8220 Psy-Q block (now linked from lib/, formerly asm/psyq_2258.s)`, PsyQ, not decompiled): loads its own
  `arg0` into GTE control regs 0-4 via `ctc2` (5 words — the packed MATRIX
  rotation part) and combines it with `arg1`. This is the evidence behind
  sizing `buf1`/`buf2` at `0x20` bytes each (a MATRIX-shaped local) rather
  than guessing from field access alone — neither buffer's contents are
  ever read by this function itself, only forwarded.

## The one real attempt: a declaration-order register-pressure trap

First attempt (declaring `buf1` before `buf2`, matching the order they're
first *used* in the source) compiled clean but scored 3/54 with the usual
big "differs outside range" warning (a 5th callee-saved register, `s1`,
appeared where retail has none). Comparing `objdump -d
build/src/code_d294.c.o` against retail's disassembly showed the actual
cause: GCC 2.6.3 recognized `&buf2` as loop-invariant (same address every
iteration of the `self->unkC` walk) and hoisted it into a persistent
callee-saved register (`s1`) *before* the loop, then reused it both inside
the loop and for the `func_80015BFC` call — pushing `arg1`/`arg2`/`arg3`
each one register higher (`s2`/`s3`/`s4` instead of retail's `s1`/`s2`/`s3`)
and growing the frame's register-save area by one word.

Retail's own disassembly does NOT hoist this address — it recomputes
`addiu $a1, $sp, 0x10` fresh at both use sites, every iteration, and needs
only 4 saved registers (`s0`-`s3`) total, one per live-across-the-whole-
function value (`self`/walker, `arg1`, `arg2`, `arg3`).

**Fix: declare `buf2` before `buf1`.** Swapping the two local declarations'
textual order (no other change) made GCC stop hoisting `&buf2` into a
register and instead recompute it at each use site, landing the exact
4-register allocation and matching frame size retail has. This was the
only attempt needed after the swap — full match immediately.

## Header changes

`include/code_d294.h`:

- `SceneNodeMethods`: typed `+0x084` (`slot84`, `void (*)(SceneNodeObj*,
  void*, s32)`) out of the `pad060` span that used to run `0x060`-`0x08C`.
- `UnkOwnerMethods_d294`: typed `+0x084` (`slot84`, same shape, receiver
  `UnkOwner_d294*`), padded from `+0x018` (previously ended at `+0x018`).
- `UnkOwner_d294`: added `next` (`UnkOwner_d294 *`) at `+0x00C`, splitting
  the `pad04[0x014-0x004]` span that used to cover it.
- New extern `func_80015BFC(void *arg0, void *arg1)`, PsyQ library.

## Proposed learning

**A local array/buffer referenced from inside a loop, whose address is
loop-invariant, can get hoisted into an EXTRA callee-saved register by GCC
2.6.3's `-O2` — and the trigger observed here was purely the TEXTUAL
DECLARATION ORDER of two same-shaped local buffers, not their usage
pattern.** Both attempts had identical control flow, identical field
accesses, identical everything except which of two `u8[0x20]` locals was
declared first; only one ordering matches retail's "recompute the address
every time, spend no register on it" codegen. When a stall's residue is "one
extra callee-saved register than retail, otherwise structurally identical,"
and the function has two or more same-typed/same-sized local buffers, try
permuting their DECLARATION order before looking for anything else — it is
cheap (one edit, one rebuild) and, per this instance, decisive. This is a
new discriminator, not yet in DECOMPILATION_LEARNINGS's "reading a residue"
list; adjacent to the existing "prologue callee-save stores in the wrong
order" entry but distinct — that one is about SPILL ORDER of already-needed
registers, this one is about whether an EXTRA register gets allocated at
all.

## Head-broadcast levers (round 13) — applicability check

- **Lever 1 (`~x + 1` vs `-x`, `nor`+`addiu` vs `negu`):** does not apply.
  No negation of any kind appears in this function's residue or body.
- **Lever 2 (N independently-incrementing walkers need N differently-based
  view types):** does not apply. This function has exactly one walker
  (`node`, over `self->unkC`'s list), not a strided dual-walk over one
  array.

## Naming (round 54, bravo, track 3)

Renamed from `func_8001D950` via `tools/rename.py`. **Tier B** -- slot
`+0x0A4` occupant. Builds `self`'s own rotation matrix via
`self->methods->slot84` (occupant `SceneNode__GetRotMatrix`, proposed
`SceneNode__GetRotMatrix` below -- not renamed, cross-unit reference),
folds in every `self->unkC` list node's own `slot84` output via
`MulMatrix2` (a compose step), then applies the composed matrix to one
or two vertex arrays via `ApplyMatrixToSVArray`. Name describes the
measured mechanics (compose a matrix by walking a list, then apply it);
whether `self->unkC` is a parent-hierarchy chain in the game sense is a
reasonable reading, not an independently confirmed one. Purely local to
this unit + its header for the FUNCTION rename; the underlying vtable
FIELD name (`slotA4`) is proposed, not renamed -- see below, it is also
dispatched from `code_d294.c` (a different unit).

## Round 95 (bravo): Sony's declarations

`MulMatrix2` now comes from `<libgte.h>`, `MATRIX *MulMatrix2(MATRIX *m0,
MATRIX *m1)`. The two 0x20-byte `u8` stack buffers became `MATRIX` locals,
passed by address. Byte-identical: same frame, same offsets.

## Round 100 (delta): track 7

Parameters `arg1`/`arg2`/`arg3` -> `vec`/`dst`/`src` (SceneNode.h's names;
ApplyMatrixToSVArray takes dst first). Locals `buf1`/`buf2`/`node` ->
`rot`/`parentRot`/`parent`.

### History: the comments in src/code_d294_b.c before this pass, verbatim

```c
/* Fills buf1 from self's own +0x84 slot, then folds in every node of the
 * self->unkC list (each node's own +0x84 slot combined into buf1 via
 * MulMatrix2) before using buf1 as ApplyMatrixToSVArray's own "out" argument,
 * twice: once for (arg2, arg3, count), once more for (arg1, arg1, 1) when
 * arg1 is non-NULL. */
```
