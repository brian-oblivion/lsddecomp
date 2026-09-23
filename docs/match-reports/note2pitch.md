# note2pitch — MATCHED (47/47 words)

> Renamed from `func_8002DF7C` on 2026-09-23 (tools/rename.py). Address 0x8002df7c.

`code_179d8_l`, vram `0x8002DF7C`, file offset `0x1E77C`. Frameless, 47 words
(0xBC bytes). Takes no arguments; return value is used by its only caller
(`StartNote` in `code_179d8_m`) as `andi $a1, $v0, 0xFFFF`, i.e. a 16-bit
result.

## What it computes

Reads three bytes from small globals — `D_8008EA0E`, `D_8008EA1C`,
`D_8008EA1D` — and indexes into a 194-entry `u16` table at `D_8006DAD8`
(monotonically increasing values 0x1000..0x2000, then a 0x0000 sentinel;
looks like a fixed-point scale/interpolation table, semantics not needed to
match it byte-for-byte):

```c
extern u8 D_8008EA0E;
extern u8 D_8008EA1C;
extern u8 D_8008EA1D;
extern u16 D_8006DAD8[];

s32 note2pitch(void) {
    s32 a0;
    s32 q12;
    s16 rem12;
    u8 a2;
    u16 v1;

    a0 = (s16)(D_8008EA0E + 0x3C - D_8008EA1C);
    q12 = a0 / 12;
    a2 = D_8008EA1D >> 3;
    rem12 = a0 - q12 * 12;
    if (a2 >= 16) {
        a2 = 15;
    }
    v1 = D_8006DAD8[a2 + rem12 * 16];
    if ((s16)(q12 - 5) > 0) {
        v1 <<= (s16)(q12 - 5);
    } else if ((s16)(q12 - 5) < 0) {
        v1 = (u16)v1 >> -(s16)(q12 - 5);
    }
    return v1;
}
```

## How it was derived

The retail asm opens with `lui $a1,(0x2AAAAAAB>>16)` — the header's carve
comment flagged this as "the signed-divide-by-3 magic-number idiom", but the
actual divisor is **12, not 3**: verified empirically through the pinned
reproducer pipeline (`tools/gcc263/cpp | cc1 | maspsx --expand-div --addiu-at
| as`, per CLAUDE.md's "escalate, do not experiment" recipe — used here just
to test candidate C, not to change the toolchain). `int t = a/12; int r = a -
t*12;` reproduces retail's exact magic-multiply/sign-correct/`*12`-via-
shift-add-shift sequence; `a/3` and `a/6` do not (wrong constant or missing
the extra post-`mfhi` `sra 1`).

Two non-obvious shape requirements, both confirmed by bisecting against the
pinned pipeline instruction-by-instruction:

1. **The initial difference must be an explicit `(s16)` cast on an `s32`
   local, not an implicit truncation into a declared-`s16` variable.**
   `s16 a0 = D_8008EA0E + 0x3C - D_8008EA1C;` lets GCC 2.6.3 prove the
   byte-arithmetic result already fits 16 bits and **elide the truncation
   entirely** — which also, as a side effect, introduces a spurious 8-byte
   `addiu $sp,$sp,-8` / `+8` bracket around the division that retail does not
   have (a phantom frame, allocated and never touched — reminiscent of the
   documented "allocated-but-unused frame" lever, except here it's an
   artifact of the *elided*-truncation path specifically, not of live-local
   count). Declaring `a0` as `s32` and writing the cast explicitly
   (`a0 = (s16)(...)`) forces GCC to actually emit the `sll 16`/`sra 16` pair
   — matching retail's literal instructions — and the phantom frame
   disappears.
2. **The remainder must be its own genuinely separate `s16 rem12` local**,
   not a reuse of `a0`. Declaring it separately makes GCC materialize its
   16-bit truncation *before* the branch (scheduled unconditionally, since
   it doesn't depend on the branch outcome) as a bare `sll $v0,$a0,16`, then
   fuse the later `*16` scaling into the branch-target's right-shift
   (`sra $v0,$v0,12` instead of `sra $v0,$v0,16`) via CSE on the same
   register. This exactly reproduces retail's split
   `sll...16` (delay slot) / `sra...12` (post-label) pair.

### The one instruction that would not move by reshaping expressions

The index computation `a2 + rem12*16` compiles to `addu $v0,$v0,$a2` (fresh
shift result as `rs`, `a2` as `rt`) under every phrasing tried: swapped
addition order, a separate `idx` temp assigned in one or two statements,
`+=` on `a2`, a ternary instead of `if` for the clamp, declaring `a2` as
`s32`/`u32`. Retail has the operands the other way,
`addu $v0,$a2,$v0` (`0x00c21021` vs the reshapes' `0x00461021`) — same
result, different encoding, and it was the function's *only* remaining
diff (46/47 words) until this was found.

**The lever that worked: `a2`'s declared width, not its expression shape.**
Changing `a2`'s type from `u32`/`s32` to `u8` (or `s16`/`u16`) flips the
`addu` operand order to match retail exactly, with every other instruction
in the function unchanged. `u8` is also the semantically honest type — `a2`
is `D_8008EA1D >> 3` where `D_8008EA1D` is itself `u8`, so the shifted value
never exceeds 5 bits. Filed here rather than as a stall because it resolved
cleanly; worth generalizing (see proposed learning below) since it is the
same *width-drives-codegen* family as the round 23 `s16`-locals lever, just
manifesting as an operand-order flip on a commutative add instead of a
narrowing-cost difference.

## Result

`build-and-verify.sh` exits 0 (whole-image SHA1 verifies).
`tools/funcdiff.py note2pitch` reports 47/47 words match.

### Proposed learning

A local's declared WIDTH can flip which operand of a commutative `add`
becomes `rs` vs `rt` — not just its narrowing cost. When a byte-identical
computation differs from retail only in operand order on an `addu`/`add`
whose result and register assignment otherwise match, before accepting it
as a register-identity stall, try shrinking the type of one addend to the
narrowest width the value can actually hold (matching its source, e.g. a
shifted byte value as `u8` rather than `u32`/`s32`). Confirmed once here;
worth a second data point before promoting further.
