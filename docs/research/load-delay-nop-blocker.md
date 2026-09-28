# The load-delay nop before a store-to-symbol macro (RESOLVED, round 63)

**Status: RESOLVED 2026-09-21 by maspsx `--nop-at-expansion`, passed by the
Makefile. Byte-exact across the whole image with every C unit recompiled from
scratch. Found by runner bravo in round 62 on `SsUtKeyOn`, reproduced by
round 62's head and again here before adoption.**

## The construct

cc1 emits an indexed load followed by a store-to-symbol macro of the loaded
register, with its own `#nop` hint between them:

```
lbu   $2,0($4)
#nop
sb    $2,G
```

ASPSX 2.34 decides the load-delay hazard BEFORE expanding `sb $2,G` into
`lui $at,%hi(G)` plus `sb $2,%lo(G)($at)`, so it keeps the nop. maspsx decides
AFTER expansion, sees the `lui $at` interposed between load and use, and
drops it. Its own debug output says so: `Reuse of '$2'. 'sb $2,G' does not use
$at`. maspsx already has the other behaviour as `nop_at_expansion`, enabled
only for `--aspsx-version` below 2.30, in the same group as `addiu_at` and
`nop_lw_lw`. The flag turns that one boolean on alone.

## Reproducer (pinned pipeline)

```c
extern unsigned char G;
void f(unsigned char *p) { G = p[0]; }
```

Without the flag: `lbu; sb` (2 words before the `j`). With it: `lbu; nop; sb`,
which is retail's shape at every site.

## Census

Over the retail disassembly (asm/, data excluded): every indexed load whose
next non-nop instruction is `lui $at` followed by a store of the loaded
register through `$at`. Counted 2026-09-21: **28 sites with the nop, 0
without.** Round 62 counted 40 including the linked Sony objects, which this
scan cannot see. None lies in a function written as C, which is why the
image stayed green for 62 rounds: 25 sit inside `INCLUDE_ASM` bodies (11 in
`SsUtKeyOnV`, 9 in `SsUtKeyOn`, 2 in `SpuVmKeyOn`, 1 each in
`SetupStyleSpawnParamsRandom`, `SsUtChangePitch`, `CD_cw`) and 3 in a `psyq_*`
segment.

## What it retracts

Round 26's attribution of these missing words to GCC's delay-slot filler.
cc1 never emits the `lui`; the residue was below cc1 and no source shape
could reach it. `SsUtKeyOn`'s 11-word gap was 9 of these nops plus 2
guard words.

## Screen

`tools/nearmiss.py` reports the construct tagged `(RESOLVED-not-a-blocker)`
like the other three, so a report that blames it can be told apart from a
live block.
