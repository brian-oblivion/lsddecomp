# Pad__DispatchEvents

> Renamed from `func_80025D10` on 2026-09-24 (tools/rename.py). Address 0x80025d10.

**Unit:** class_16334 · **Size:** 65 words (0x104 bytes) · **Status:** MATCHED
(byte-exact, whole-image `./build-and-verify.sh` green) · Worked by the head in
round 2026-08-30-a.

## What it does

The Pad class's per-frame button-event dispatch — slot `+0x48`
(`dispatchEvents`) of `D_8006D370`. It runs after `Pad__UpdateMasks` has refreshed
the three edge masks, turns each set bit into an event code, and delivers the
codes to the instance's overridable `onButtonEvent` handler (slot `+0x30`).

Three things about it are worth recording because they are not guessable from
the field names:

1. **The scan is priority-ordered per bit, not per mask.** For each of the 16
   table entries it tests released, then pressed, then held, and emits at most
   *one* event for that bit. A button that is both newly pressed and held
   yields only the pressed event.
2. **The event code is `base + bitIndex`, and the three bases are 16 apart** —
   `0x02` held, `0x12` pressed, `0x22` released. Since the index is always
   below 16 the three ranges (`0x02`–`0x11`, `0x12`–`0x21`, `0x22`–`0x31`) do
   not overlap, so the single `s32` carries both the kind and the button.
3. **Delivery is in REVERSE collection order.** Events are appended forward into
   a 16-entry stack array and then walked back down from the last one. The
   highest-numbered button is handled first.

`D_8008B388` is confirmed here as a 16-entry mask table indexed `0..15` — the
runtime copy that `Pad__LoadButtonTable` fills from Psy-Q's `D_80010764`.

## Derivation

Prologue guard, from the disassembly:

```
lw   $t1, 0x10($s3)          ; held
lw   $t0, 0x14($s3)          ; released
lw   $a3, 0x18($s3)          ; pressed
bnez $t1, .L80025D50
 addiu $s0, $sp, 0x10        ; p = events
bnez $t0, .L80025D54
 addu  $a1, $zero, $zero     ; i = 0
beqz $a3, .L80025DF4         ; all three zero -> return
```

i.e. `if (held == 0 && released == 0 && pressed == 0) return;`, with the
short-circuit evaluated as three separate branches. `.L80025D50` re-does `i = 0`
only because the second branch already put it in its own delay slot.

The tail is the reverse walk:

```
addiu $s0, $s0, -0x4         ; p--
addiu $a0, $sp, 0x10         ; &events[0], in a TEMP
lw    $v0, 0x0($s3)
sltu  $v1, $s0, $a0
lw    $s2, 0x30($v0)         ; onButtonEvent, hoisted out of the loop
bnez  $v1, .L80025DF4
 addu $s1, $a0, $zero        ; only NOW copied to the callee-saved reg
```

That `&events[0]`-in-a-temp-then-copied shape is the tell for writing the loop
as `for (p--; p >= events; p--)` with the array named directly in the condition,
rather than hoisting a `base` variable by hand. With a hand-written
`base = events;` before the guard, GCC allocates `$s1` immediately and compares
against it, which is one word shorter than retail and scored 38/65. Naming the
array in the condition lets GCC's own loop-invariant hoisting produce the
preheader copy, which is retail exactly: **61/65**.

## Residue and the scheduling barrier

At 61/65 the only difference left was the ORDER of the four prologue register
stores:

```
retail:  sw s3,0x5c ; move s3,a0 ; sw ra,0x60 ; sw s2,0x58 ; sw s1,0x54 ; sw s0,0x50
mine:    sw s3,0x5c ; move s3,a0 ; sw s0,0x50 ; sw ra,0x60 ; sw s2,0x58 ; sw s1,0x54
```

Same four registers, same four stack offsets, same everything else in the
function. Only the sequence differs — `sw s0` scheduled three slots early.

A bare `__asm__("")` as the first statement closes it, 65/65, image green. This
is the **permitted** form under CLAUDE.md rule 6, and the test the rule
prescribes was actually run rather than assumed: with the barrier removed the
build is 61/65 with *identical register allocation* (`$s0` = p, `$s1` = base,
`$s2` = handler, `$s3` = self, in both). It changes instruction order only, so
it is not a register pin.

Ten source shapes were tried before reaching for it, all still 61/65 or worse,
so the barrier is not standing in for an unexplored idea:

| shape | result |
| --- | --- |
| six permutations of the local declaration order | 61/65 each — declaration order does not reach the prologue scheduler at all |
| `p = events;` hoisted above the three mask loads | 61/65 |
| the three mask loads written in reverse order | 52/65 (worse) |
| guard as `if (!(held \|\| released \|\| pressed))` | 61/65 |
| guard positive, whole body nested in `if (held \|\| released \|\| pressed) { … }` | 61/65 |
| **bare `__asm__("")` as the first statement** | **65/65, image green** |

The declaration-order result is the generalizable half: six orders, one score.

### Proposed learning

**Prologue callee-save store ORDER is not reachable from C.** When the only
residue is the sequence of the `sw $sN, off($sp)` stores — identical registers,
identical offsets — declaration order, statement order and guard spelling all
leave it untouched (six declaration permutations produced one identical score).
A bare `__asm__("")` as the function's first statement is the lever, and it is
the permitted form: verify by removing it and confirming the register
allocation is unchanged, which is exactly the test CLAUDE.md rule 6 states.

**Name an array directly in a loop condition rather than hoisting a `base`
local.** `for (p--; p >= events; p--)` and a hand-hoisted `base = events;` are
not equivalent to GCC 2.6.3: the hand-hoisted form allocates the callee-saved
register before the guard and compares against it, losing the
`temp → compare → copy-in-the-delay-slot` preheader shape that retail has. Here
that was worth 23 words (38/65 → 61/65). Let GCC hoist its own loop invariants.
