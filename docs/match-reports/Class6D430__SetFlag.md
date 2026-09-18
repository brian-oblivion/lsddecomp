> Renamed from `func_80026C88` on 2026-09-18 (tools/rename.py). Address 0x80026c88.

# Class6D430__SetFlag

**Unit:** code_171e0 · **Size:** 5 instructions · **Status:** MATCHED (5/5 words)

## What it does

Sets bit 0 of a flags word at offset `0x24` of its argument. It is itself
slot `+0x064` of the `D_8006D430` method table (see `GetClass6D430Methods`'s
report), so its real signature is fixed by whatever that slot is called with
elsewhere — here, just `this`.

## Derivation

```
lw    $v0, 0x24($a0)
 nop
ori   $v0, $v0, 0x1
jr    $ra
 sw   $v0, 0x24($a0)
```

Read-modify-write on a single word, no other fields touched. `$a0` is treated
as `struct-pointer + 0x24`, so it gets a minimal local struct (`Class6D430`
in `include/code_171e0.h`) with padding up to that offset — following
CLAUDE.md's guidance to name a field rather than write raw pointer arithmetic,
while being honest that nothing earlier than `0x24` is known yet:

```c
typedef struct Class6D430 {
    u8 pad0[0x24];
    s32 flags;
} Class6D430;

void Class6D430__SetFlag(Class6D430 *this) {
    this->flags |= 1;
}
```

No return value is set up (nothing moves a fresh value into `$v0` before the
`jr`; whatever's left there from the `ori` is incidental), so `void` is a safe
read here — this is not the "one-line wrapper tail-calling a non-void
function" trap from the runner brief, since there's no callee at all.

## Proposed learning

None new.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80026C88` | `Class6D430__SetFlag` | B |

**Evidence.** `+0x064` slot: `this->flags |= 1;` -- a read-modify-write on
the single flags word this round also named (see the field-rename commit).
Mechanics fully known (sets bit 0); what bit 0 signals in the game is not
established, so named for the mechanic only.

**Field renamed alongside it.** `unknown_value_0x24` -> `flags` (in
`include/code_171e0.h`'s `Class6D430`): `grep -rn -- '->unknown_value_0x24\b'
src/` had zero hits outside `code_171e0.c`, so unlike almost every other
field in this struct (see `## Proposed field names

**APPLIED by the head at merge, round 52** -- all four fields, both types
and all five vtable slots below are now in the tree, each one applied
separately with `./build-and-verify.sh` green and byte-exact after it. One
mis-hit had to be resolved by receiver type: `src/code_179d8_h.c:143`
accesses `pendingGeneration` on a `Class6D430 *self`, while the same file's
lines 97/174/175/182 are its OWN `ObjA34_179D8H::unk0C` and were left alone.
The compiler named that mis-hit (`structure has no member named 'unk0C'`),
which is the procedure working in the direction where it can work.` in this unit's other
reports / the broadcast), this one is owned solely by this unit and renames
directly rather than needing a head-applied cross-unit proposal.
