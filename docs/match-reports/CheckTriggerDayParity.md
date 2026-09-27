# CheckTriggerDayParity

> Renamed from `CheckTriggerParity` on 2026-09-27 (tools/rename.py). Address 0x8005c9a4.

> Renamed from `func_8005C9A4` on 2026-09-21 (tools/rename.py). Address 0x8005c9a4.

**Unit:** code_4cd08 · **Size:** 14 words · **Status:** MATCHED (14/14 words)

## What it does

Parity check against a candidate record: `true` if the record's side/parity
byte at offset `0x2` is zero (no constraint), otherwise `true` iff that byte
disagrees with `coordParity`'s own parity (`coordParity % 2 + 1`, computed
with C's round-toward-zero `%`, not `& 1` — see below).

Called from `TryDreamAuxTrigger` as `CheckTriggerDayParity(thirdParam, candidateRecord)`
where `candidateRecord` comes from `LookupDreamAuxTrigger`, one of this unit's
stage-table lookups (6-byte-stride records, same size as `StageSpawn` /
`StaticLinkTrigger` in `include/DreamSys.h`, but the field this function reads
is loaded with `lb` — a SIGNED byte — while both of those structs' offset-2
field is an unsigned `MapTile`. Under `-funsigned-char` a signed load only
happens for an explicit `s8` field, so this is either a third, still-unnamed
record type or those structs are slightly wrong at this offset. Not resolved
here — this function only ever touches that one byte, so it is typed as
`s8 *entry` rather than committed to a specific struct.

## Derivation

```
lb    $v1, 0x2($a1)
nop
beqz  $v1, .L8005C9D4
 ori  $v0, $zero, 0x1          ; delay slot: v0 = 1 unconditionally
srl   $v0, $a0, 31
addu  $v0, $a0, $v0
sra   $v0, $v0, 1
sll   $v0, $v0, 1
subu  $v0, $a0, $v0            ; v0 = a0 % 2 (round-toward-zero remainder)
addiu $a0, $v0, 1              ; a0 = (a0 % 2) + 1  -- NOTE: result lands in $a0
xor   $v0, $v1, $a0
sltu  $v0, $zero, $v0
.L8005C9D4:
jr    $ra
 nop
```

The `srl`/`addu`/`sra`/`sll`/`subu` sequence is GCC 2.6.3's standard expansion
of `%` by a power of two for a SIGNED dividend (bias by the sign bit before
the arithmetic shift, so the remainder keeps the dividend's sign) — confirms
the source used `%`, not `& 1`, matching CLAUDE.md's residue list.

```c
bool CheckTriggerDayParity(s32 coordParity, s8 *entry)
{
    bool result = true;

    if (entry[2] != 0) {
        coordParity = coordParity % 2 + 1;
        result = entry[2] != coordParity;
    }
    return result;
}
```

Two things were needed to get the exact bytes, both worth remembering:

1. **Shape.** `result = true; if (cond) { result = ...; }` — a default value
   set *before* the guarding branch, unconditionally overwritten inside it —
   is what lets GCC schedule the default into the branch's delay slot with no
   extra jump. Writing it as two `if`/`else` return statements, or as a
   short-circuit `||`, both produced a same-*length* but differently-shaped
   function (an extra `j` in one case, branches instead of `xor`+`sltu` in the
   other) — same semantics, wrong bytes.
2. **Register identity.** `coordParity % 2 + 1` written into a fresh
   expression put the result in `$v0`; retail has it in `$a0` (the now-dead
   parameter register, reused). Assigning back into `coordParity` itself
   (`coordParity = coordParity % 2 + 1;`) rather than introducing an implicit
   temporary fixed it — no `register asm` needed, just reusing the variable
   whose register was already free.

## Proposed learning

A "default value, then conditionally overwritten in an `if` with no `else`"
shape is a scheduling opportunity GCC 2.6.3 takes: the default assignment
moves into the guarding branch's delay slot for free. The semantically
equivalent `if/else` (two assignments, one join) or `||` (short-circuit)
forms do NOT get this treatment here — they cost an extra instruction or
change comparison codegen entirely. When a residue is "same value, one extra
instruction, or a `beq` where retail has `xor`+`sltu`", try the default-value
shape before anything more exotic.

## Naming

**CheckTriggerDayParity** — tier A. A pure predicate over `entry`'s side/parity
byte at offset `0x2` and a caller-supplied `coordParity`: true when the byte
is 0 (no constraint) or when it disagrees with `coordParity`'s own parity.
The mechanics (a parity comparison) ARE the name, tier A by the pure-leaf
rule. Dropped the unit-specific "DreamAux" prefix other functions here carry
since this predicate reads a byte offset shared with `TriggerRecord.parity`
without being proven to be the same field (see the header's own caveat on
`TriggerRecord`'s offset-0x2 comment) -- "Trigger" alone reflects that
looser confidence.

## Round 100 (alpha): track 7, moved from src/code_4cd08.c and include/code_4cd08.h

## Naming (round 100)

**CheckTriggerDayParity** (was CheckTriggerParity) -- tier A. Its first
argument is the day (TryDreamAuxTrigger's `day`, DreamSys
getCurrentDayAndYear), not a coordinate: dayParity 0 passes every day, 1 odd
days, 2 even days. It takes DreamAuxTriggerEntry * (field `dayParity`, was
`entry[2]`). Byte-identical.

The function comment, as it stood:

```c
/* True when `entry`'s side/parity byte (offset 0x2) disagrees with
 * `coordParity`'s own parity. `entry` is a candidate spawn/link record from
 * one of this unit's stage tables (see LookupDreamAuxTrigger); its layout beyond this
 * one byte is not yet known here, so it is addressed by byte offset rather
 * than through a named struct. A parity byte of 0 means "no side constraint",
 * hence the early `true`. */
```
