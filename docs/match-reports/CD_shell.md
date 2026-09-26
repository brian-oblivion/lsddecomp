# CD_shell

> Renamed from `func_8002A400` on 2026-09-23 (tools/rename.py). Address 0x8002a400.

**Unit:** libcd_bios · **Size:** 68 words · **Status:** MATCHED (68/68 words)

## What it does

If `D_8006D904 < D_8006D614` (a periodic-service counter is behind), saves
and clears the `D_8006D5FC` callback, prints a one-shot "waiting" message
while `D_8006D60C & 0x10` stays set (calling `CD_cw(1,0,0,0)` each
iteration), then retries `CD_cw(0x16, D_8006D908, 0, 0)` printing a
"still waiting" message until it succeeds, restores `D_8006D5FC`, and
catches `D_8006D904` up to `D_8006D614`.

## The C

```c
void CD_shell(void)
{
    s32 saved;
    s32 counter = 0;

    if (D_8006D904 < D_8006D614) {
        saved = D_8006D5FC;
        D_8006D5FC = 0;

        while (D_8006D60C & 0x10) {
            if ((u8)counter == 0) {
                func_80025AE4(D_80010A40);
            }
            counter++;
            CD_cw(1, 0, 0, 0);
        }

        while (CD_cw(0x16, D_8006D908, 0, 0)) {
            CD_cw(1, 0, 0, 0);
            func_80025AE4(D_80010A50);
        }

        D_8006D5FC = saved;
        D_8006D904 = D_8006D614;
    }
}
```

## Notable findings

**`counter = 0;` had to move to the declaration itself (unconditional,
before the `if`), not as the first statement inside the `if` body.**
Retail's `move $s0, $zero` executes BEFORE the `slt`/`beqz` that decides
whether to enter the `if` at all -- i.e. it is computed regardless of
whether the guarded block runs. This is a case where the INITIALIZER
position in C (`s32 counter = 0;` vs `counter = 0;` as the first statement
of the `if`) is directly observable in the compiled instruction order, not
just a style choice.

**The "print once" idiom is `if (counter == 0) { print(); } counter++;`,
NOT `if (counter == 0) { counter++; print(); }`.** Both are functionally
identical (`counter` only ever needs to go from 0 to nonzero once), but
they compile to different instruction placement. Retail folds the
UNCONDITIONAL `counter++` into the **branch's own delay slot** (the `bnez`
that skips the print when `counter != 0`) -- which only works because the
increment is the literal next statement after the `if` closes and doesn't
depend on anything the `if`-branch computed. Writing the increment inside
the conditional (`if (counter==0) { counter++; ...}`) makes GCC schedule it
into the CALL's delay slot instead (still correct C, but 2 words off from
retail, both being the same `addiu $s0,$s0,1` instruction at a different
address).

### Proposed learning

**A "print/act once inside a loop" flag variable's increment belongs
OUTSIDE the `if`, immediately after it closes, not inside the guarded
block** -- when a residue shows the same single instruction present in
both builds but at two different addresses (not a missing/extra
instruction, an instruction-PLACEMENT difference), check whether an
unconditional side effect was written inside a conditional it doesn't
need to be inside; hoisting it to immediately after the `if` lets GCC's
delay-slot filler place it where retail's does.
