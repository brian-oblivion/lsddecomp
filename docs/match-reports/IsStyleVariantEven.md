# IsStyleVariantEven -- MATCHED, round 44 (2026-09-15)

> Renamed from `func_8005630C` on 2026-09-23 (tools/rename.py). Address 0x8005630c.

Unit `ObjMStyleActor`. **5/5 words, byte-exact.** First build after reopening.

## What it was

Carved round 17, filed as `gp_rel` blocked. Round 42 resolved `gp_rel` via
`--gp-symbols`; round 44's head reopened this unit's stub as `REOPENED --
ASSIGNABLE` (see the file banner in `src/world/ObjMStyleActor.c`). Never attempted
until now.

## Derivation

```
/* 46B0C 8005630C 7804828F */  lw    $v0, %gp_rel(sStyleVariant)($gp)
/* 46B10 80056310 00000000 */  nop
/* 46B14 80056314 01004230 */  andi  $v0, $v0, 0x1
/* 46B18 80056318 0800E003 */  jr    $ra
/* 46B1C 8005631C 01004238 */   xori $v0, $v0, 0x1
```

`sStyleVariant` is a `.sbss` word (`asm/data/7B46C.sbss.s`), already in
`config/gp-symbols.txt`, and is written elsewhere as a plain flag (`sw $a0,
%gp_rel(sStyleVariant)($gp)` in the still-stubbed `RegisterStyleConfig`, this same
round's assignment in the sibling unit). Declared locally in this unit as
`extern s32 sStyleVariant;` -- it is not owned by either of my units, so it
stays a local extern rather than going in `class_3bb8c.h` (the third owner,
`ObjMStyleActor`, is still an uncarved monolith and would just as legitimately
carry its own local view later).

```c
extern s32 sStyleVariant;

s32 IsStyleVariantEven(void) {
    return (sStyleVariant & 1) ^ 1;
}
```

**Order matters, and the obvious spelling gets it backwards.** `!(sStyleVariant
& 1)` compiles to the same two instructions -- `andi` then `xori` -- but with
GCC 2.6.3 scheduling the `xori` ahead of the `andi` into the position before
the `jr`, and the `andi` into the delay slot: the reverse of retail. Writing
the mask-then-flip explicitly, `(x & 1) ^ 1`, gets `andi` scheduled first and
`xori` into the `jr`'s delay slot, matching retail on the first build after
reopening.

### Proposed learning

For a 2-instruction `andi $r,$r,1` / `xori $r,$r,1` pair feeding a `jr`, write
`(x & 1) ^ 1`, not `!(x & 1)` -- same semantics, but GCC 2.6.3 schedules the
two into opposite instruction order depending on which C spelling you use.

## Naming

**Tier A.** Free function, pure leaf: a getter whose mechanics ARE its purpose (`(sStyleVariant & 1) ^ 1`, i.e. true when `sStyleVariant` is even). No caller-side evidence needed beyond the body itself.
