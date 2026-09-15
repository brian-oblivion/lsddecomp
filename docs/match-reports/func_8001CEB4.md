> **REOPENED -- ASSIGNABLE, round 42 (2026-09-15).** This function was
> screened as blocked by `nop_mflo_mfhi`. **That blocker is RESOLVED**: maspsx gained
> `--gp-symbols` and `--no-nop-mflo-mfhi` (`tools/patches/maspsx-lsd-flags.patch`,
> passed by the Makefile), the whole image stays byte-exact, and previously
> blocked functions now match (see `docs/research/gp-relative-blocker.md`,
> "RESOLVED"). Everything below is evidence from before the fix: its
> derivation may still be right, its VERDICT is not. Rebuild before believing
> any score in it. The preserved body was rebuilt this round with the flag on and is NOT a match: it is one word longer than retail and differs in register identity (`$s2` where retail has `$s3`), so the report's "nop is the only residue" claim was wrong. Re-derive from asm-differ, not from the body.

# func_8001CEB4 -- TOOLCHAIN BLOCKED (`nop_mflo_mfhi`, sibling of the `addiu_at` blocker)

Unit: `code_d294` (round 14). Occupies `Class6B5CCMethods` vtable slot
`+0x044` (`slot44`, already typed before this round). Logic fully derived
and (modulo the blocker and one residual register-identity difference,
neither of which change the analysis below) matches retail instruction
for instruction. `void func_8001CEB4(Class6B5CCObj *self, s32 flag, void
*data)`.

## The blocker, confirmed with the documented tighter screen

`docs/research/addiu-at-blocker.md`'s 2026-09-01 addendum documents a
SIBLING blocker to the `addiu_at` one: maspsx's `nop_mflo_mfhi` flag
(pinned `True` at `--aspsx-version=2.34`) inserts two `nop`s between an
`mfhi`/`mflo` and a following `mult`/`multu`/`div`/`divu` within the next
two instructions, REGARDLESS of whether retail's own code needed them.
The doc's own screen, run against this function:

```
$ grep -A2 -nE '\b(mflo|mfhi)\b' asm/nonmatchings/code_d294/func_8001CEB4.s | grep -E '\b(mult|multu|div|divu)\b'
32-    /* D718 8001CF18 18004600 */  mult       $v0, $a2
```

A hit. Retail's own disassembly confirms this construct is exactly the
"no nop wanted" case the doc's census found on the majority side (180 vs
65 sites): `mfhi $a0` (`8001CF14`) is immediately followed by `mult $v0,
$a2` (`8001CF18`) with zero nops in between, in the retail binary itself.
The pinned pipeline inserts two `nop`s at that exact point for ANY C that
reaches this instruction pairing -- confirmed empirically: my C compiles
to precisely `mfhi $a0; nop; nop; mult $v0,$a2`, an 8-byte-longer
sequence that shifts every subsequent instruction in the function (and
therefore drifts the whole rest of the build), which is exactly the "one
instruction[-pair] short, everything after shifts" signature.

**This was NOT screened by the coordinator's initial pass** (which
correctly checked `gp_rel` and the literal `addiu $at,$at,%lo`
`addiu_at` pattern -- neither hits here) -- it is the SEPARATE, narrower
screen the same research doc documents for this sibling flag, only
findable by actually reading the function's own `mfhi`/`mult` adjacency
against the doc's addendum. Recorded here so a future runner doesn't
re-derive it: **any function whose disassembly has an `mfhi`/`mflo`
immediately (within 2 instructions) followed by a `mult`/`multu`/`div`/
`divu`, with NO nop in retail's own bytes at that point, is blocked by
this flag exactly like the `addiu_at` family** -- same "operator
escalation, do not experiment" status per CLAUDE.md rule 5.

## Best-reached source (compiles, matches retail's control flow and every
## field/value derived correctly; blocked from byte-exact solely by the
## flag above plus one still-open register-identity residue described below)

```c
void func_8001CEB4(Class6B5CCObj *self, s32 flag, void *data) {
    s32 vals[3];
    Class6B5CCSub44 *dst;
    s16 *field;

    vals[0] = func_8001EC84(data);
    vals[1] = func_8001EC84((u8 *)data + 4);
    vals[2] = func_8001EC84((u8 *)data + 8);
    vals[0] /= 360;
    vals[1] /= 360;
    vals[2] /= 360;
    dst = self->unk14->unk44;
    field = &dst->vec.x;
    if (flag) {
        dst->vec.x = vals[0];
        dst->vec.y = vals[1];
        dst->vec.z = vals[2];
    } else {
        s32 i;

        for (i = 0; i < 3; i++) {
            field[i] = (field[i] + vals[i]) % 4096;
        }
    }
    self->unk14->unk0 = 0;
}
```

Preserved inline per project convention (`#if 0`, never a block comment,
positioned where it would compile back into `src/code_d294.c` in place of
the current `INCLUDE_ASM`):

```c
#if 0
void func_8001CEB4(Class6B5CCObj *self, s32 flag, void *data) {
    s32 vals[3];
    Class6B5CCSub44 *dst;
    s16 *field;

    vals[0] = func_8001EC84(data);
    vals[1] = func_8001EC84((u8 *)data + 4);
    vals[2] = func_8001EC84((u8 *)data + 8);
    vals[0] /= 360;
    vals[1] /= 360;
    vals[2] /= 360;
    dst = self->unk14->unk44;
    field = &dst->vec.x;
    if (flag) {
        dst->vec.x = vals[0];
        dst->vec.y = vals[1];
        dst->vec.z = vals[2];
    } else {
        s32 i;

        for (i = 0; i < 3; i++) {
            field[i] = (field[i] + vals[i]) % 4096;
        }
    }
    self->unk14->unk0 = 0;
}
#endif
```

## Derivation notes (everything below is independent of the blocker and
## can be trusted by whoever revisits this once the flag question is
## resolved)

- Three `func_8001EC84` reads (`data+0`/`+4`/`+8`, the same 3-entry
  `{s16,s16}` table `func_8001D008` also reads), each divided by 360
  (magic `0xB60B60B7`, shift 8 -- verified against the pinned `cc1`,
  `int f(int x){return x/360;}` reproduces the exact `mult`/`mfhi`/
  `addu`/`sra 8`/`sra 31`/`subu` sequence byte-for-byte).
- `flag != 0`: overwrite `dst->vec.x/unk12/unk14` (three new `s16` fields
  on `Class6B5CCSub44`, see below) directly with the divided values
  (truncated to 16 bits on store, same as `func_8001D008`'s `(s16)`-cast
  pattern but here implicit via the destination's own `s16` type).
- `flag == 0`: accumulate each divided delta into the EXISTING field
  value and wrap the sum modulo 4096 (a full-turn wrap for a PSX-native
  4096-per-circle angle unit) -- `(field[i] + vals[i]) % 4096` reproduces
  retail's `bgez`/`+0xFFF`-if-negative/`sra 12`/`sll 12`/`subu` idiom
  exactly (verified against the pinned `cc1`: `int f(int x){return x %
  4096;}` matches byte-for-byte -- this is C's own truncating-toward-zero
  `%`, not a floor-mod, despite superficially resembling one).
- **The `field = &dst->vec.x;` pointer must be computed UNCONDITIONALLY,
  before the `if`, not inside the `else` branch alone** -- moving it
  outside fixed an early residue (retail materializes it in the `beqz`
  branch's own delay slot, i.e. on both paths, even though only the
  `else` path ever reads it back).
- **Remaining register-identity residue (independent of the blocker,
  would still need resolving even if the flag were fixed):** retail keeps
  the `field` pointer live in `$a3` across the loop-counter
  initialization (`move a2,zero`) all the way to its first use inside the
  loop (`move a0,a3`); my C's equivalent value lands in `$v0` instead and
  needs an extra `move a0,v0` at a shifted position. Same word count in
  this sub-region, different register -- not investigated further since
  the function is blocked regardless of this residue.

## New struct knowledge (`include/code_d294.h`, additive; already committed
## alongside `func_8001D008`, documented once here to avoid duplication)

`Class6B5CCSub44` (the retyped target of `Class6B5CCSub14::unk44`) gains
`unk10`/`unk12`/`unk14` (three `s16` fields, immediately following the
`s32 unk0`/`unk4`/`unk8` fields `func_8001D008` established) -- see the
struct's own comment in `include/code_d294.h` and `func_8001D008`'s match
report for the shared derivation.

### Proposed learning

**A second, narrower screen belongs alongside the standing `gp_rel`/
`addiu_at` blocker screen: `grep -A2 -nE '\b(mflo|mfhi)\b'
asm/nonmatchings/<unit>/<func>.s | grep -E '\b(mult|multu|div|divu)\b'`,
per `docs/research/addiu-at-blocker.md`'s 2026-09-01 addendum.** This
function did not hit either of the two screens the round's assignment
explicitly named, and only surfaced as blocked after ~50 words of
otherwise-correct derivation produced a mystifyingly relocated call
target (`jal 1ec84` wanted, `jal 1ec90` produced) -- the tell that
something upstream in the SAME function was 8 bytes too long. Screening
both patterns before starting would have saved the derivation time (though
not wasted, since the derived body is complete and correct pending the
flag).

## Head note, round 13: field names in the body above were retargeted

`Class6B5CCSub44`'s `unk10`/`unk12`/`unk14` no longer exist under those
names. Runner delta, matching `func_8001D4DC` in the sibling unit
`code_d294_b` in the same round, measured that same byte range as an
`S16Quad_d294` and named it `vec`. The head unified the two views on
delta's, since a struct copy through `vec` is what an already-MATCHED
function uses, and updated the preserved body above to `vec.x`/`vec.y`/
`vec.z` so that it still compiles as preserved (project rule: a preserved
body must compile where it sits, with every declaration it needs).

`vec.w` at +0x016 exists too, and is a byte range this report had recorded
as still opaque. Nothing else about the derivation changes.
