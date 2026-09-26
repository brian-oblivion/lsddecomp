# CheckDreamAuxWorldState

> Renamed from `func_8005CD58` on 2026-09-21 (tools/rename.py). Address 0x8005cd58.

**Unit:** code_4cd08 · **Size:** 20 words · **Status:** MATCHED round 43
(20/20, byte-exact whole-image build).

## History

Filed BLOCKED in round 2026-08-30-a on one `%gp_rel` reference (to
`gDreamAuxWorld`) plus one `addiu_at` indexed-load. Round 42 resolved both
(`--gp-symbols`, and `addiu_at` back in round 21). Never actually attempted
under the old toolchain -- the stub carried no derivation. Round 43 derived
and matched it fresh.

## What it does

A predicate: call the object at global `gDreamAuxWorld`'s vtable slot 0x80 (byte
offset 0x200) with no argument but `self`, and compare the (word-sized)
result against a per-`idx` signed byte from a small lookup table
`D_80088D16`.

```c
typedef s32 (*TriggerWorldFn80)(TriggerWorld *self);

bool CheckDreamAuxWorldState(s32 idx)
{
    TriggerWorld *w = (TriggerWorld *)gDreamAuxWorld;
    s32 val = D_80088D16[idx];
    s32 result = ((TriggerWorldFn80)w->vtable[0x80])(w);

    return val == result;
}
```

## Derivation notes (three attempts to byte-exact)

1. **First pass (1/20, 136532 bytes of drift):** wrote the natural
   "call-then-compare" order, `s32 result = fn(w); return result == D_80088D16[idx];`.
   Retail computes the `D_80088D16[idx]` byte lookup and caches it in a
   callee-saved register (`$s0`) *before* the `jalr`, then uses it after the
   call returns; writing the array access as part of the `return` expression
   put its evaluation entirely after the call, costing 2 extra words and
   shifting everything downstream. Fix: give the byte lookup its own local,
   declared (and thus evaluated) before the call.
2. **Second pass (11/20, 81195 bytes of drift):** with the lookup pulled into
   `s8 val = D_80088D16[idx];`, ordering matched, but the byte load became
   `lbu` (zero-extend) immediately followed by `sll $s0,24 / sra $s0,24`
   (manual sign-extend) placed AFTER the call, instead of retail's single
   `lb` (sign-extending load) before the call. This is exactly the documented
   idiom in `docs/DECOMPILATION_LEARNINGS.md`'s round-10 entry: **"A byte
   copy is `lbu` under `-funsigned-char` no matter what you write — unless
   the value passes through a wider (`s32`) local first, which forces
   `lb`."** `D_80088D16` is a `s8[]`, but the LOCAL holding the read result
   needs to be `s32`, not `s8`, to get the sign-extending load at the point
   of read rather than a lazy re-extend at the point of use. Changing
   `s8 val` to `s32 val` fixed the load and closed the size gap (18/20, no
   more out-of-range drift).
3. **Third pass (18/20 -> 20/20):** the remaining two words were an
   `xor`/`sltiu` destination-register swap: `return result == val;` computed
   the boolean into `$v0` (the call's own return register) directly, where
   retail computes `xor $s0, $s0, $v0` (destination `$s0`, the byte-lookup's
   register) and only then `sltiu $v0, $s0, 1`. Swapping the comparison to
   `return val == result;` (operand order matching retail's `xor $s0,$s0,$v0`
   reading `$s0` first) reproduced the exact register choice. Byte-exact on
   this change.

`gDreamAuxWorld`'s declaration (`extern s32 gDreamAuxWorld;`, shared with
`SetDreamAuxWorld`) predates this function; it is cast to `TriggerWorld *` at
the point of use here rather than declared as a pointer at file scope, since
`SetDreamAuxWorld` treats the same global as a generic `s32` parameter store.
`TriggerWorldFn80` started as a local typedef distinct from `TriggerWorldFn`
(vtable slot 0x22, different arity) and was promoted into
`include/code_4cd08.h` once `AdjustDreamAuxTriggerOffset` (matched immediately after, same
round) turned out to need the identical alias -- see that function's report.
`D_80088D16` (a small `s8[]` lookup table, layout otherwise unknown) is now
declared in `include/code_4cd08.h` alongside this unit's other module-owned
data tables.

## Proposed learning

Reinforces (does not add) the round-10 `lbu`-vs-`lb` learning already on
file, plus a new data point: **the destination register of a boolean
`xor`/`sltiu` equality test follows the SOURCE ORDER of the `==` operands**,
not just their values -- `a == b` and `b == a` are logically identical but
compiled to different destination registers here, and only the order
matching retail's own `xor` operand order (`$s0, $s0, $v0` -> "cached value
first") produced the byte-exact result. Cheap to check by operand-swap
before treating a residue like this as a deeper stall.

## Naming

**CheckDreamAuxWorldState** — tier A. A pure predicate: calls
`gDreamAuxWorld`'s vtable slot 0x80 (self-only) and compares the result
against a per-`idx` entry of `D_80088D16`. The mechanics (query the world,
compare) ARE the name; tier A by the pure-leaf rule even though what the
world's vtable-0x80 slot itself represents is unknown.

## Track 4 (2026-09-26, round 88, bravo)

The view `*gDreamAuxWorld` is cast to is renamed `DreamAuxWorld` /
`DreamAuxWorldFn80` in include/code_4cd08.h (was `TriggerWorld` /
`TriggerWorldFn80`, same `{ void **vtable; }` shape, so the call is
unchanged). The name `TriggerWorld` now belongs to the class D_8006F40C
(include/TriggerWorld.h), whose table is 0x8C bytes: this call loads byte
+0x200 of its object's table (`lw v0,512(v0)`), so gDreamAuxWorld is not a
TriggerWorld. Its real class is unresolved. Bytes unchanged.
