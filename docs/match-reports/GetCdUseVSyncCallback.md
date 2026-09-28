# GetCdUseVSyncCallback -- MATCHED (round 45: 3/3 words, first-ever attempt)

> Renamed from `func_80028B6C` on 2026-09-21 (tools/rename.py). Address 0x80028b6c.

Unit: `src/code_179d8_s.c` (carved mid-round 17, 2026-09-04). Size: 3 words
(0xC bytes), file offset `0x1936C`, vram `0x80028B6C`.

The report below (kept for history) marked this a `gp_rel` blocker, citing
a 2026-08-29 `-G` experiment that was rejected by the operator. That
blocker was RESOLVED in round 42 by the maspsx flag `--gp-symbols` (see
CLAUDE.md's "Open toolchain blockers" and `docs/research/gp-relative-blocker.md`,
"RESOLVED") -- the resolution mechanism was never the rejected `-G`
experiment; it is a table-driven flag that fills in gp-relative addressing
for symbols splat itself declares in `.sdata`/`.sbss`. Round 45 is this
function's first-ever attempt, and it confirms the pattern this round's
broadcast channel has independently confirmed several times over (alpha,
delta, bravo, echo): a function whose only recorded cause was `gp_rel`
matches byte-exact on the first rebuild, no reshaping needed.

## Evidence

```
lw $v0, %gp_rel(gCdUseVSyncCallback)($gp)
jr $ra
nop
```

`gCdUseVSyncCallback` is a plain `s32` in `.sdata` (`asm/data/7B048.sdata.s`:
`dlabel gCdUseVSyncCallback` / `.word 0x00000001`), so the getter is exactly:

```c
extern s32 gCdUseVSyncCallback;

s32 GetCdUseVSyncCallback(void) {
    return gCdUseVSyncCallback;
}
```

Built to `0xC` bytes (`objdump -t build/src/code_179d8_s.c.o`), matching
retail's `nonmatching GetCdUseVSyncCallback, 0xC` header exactly. `funcdiff.py`
reports 3/3 words match in-range (some out-of-range drift was present at
measurement time from this runner's other in-progress stalls elsewhere in
the same round, not from this function -- the whole-image build is
byte-exact once this and the round's other matches are folded in
together, confirmed by `./build-and-verify.sh`).

## Disposition

MATCHED. Committed as C.

---

# (round 17/42, superseded) GetCdUseVSyncCallback -- STALL (gp-relative blocker, not attempted)

> **REOPENED -- ASSIGNABLE, round 42 (2026-09-15).** This function was
> screened as blocked by `gp_rel`. **That blocker is RESOLVED**: maspsx gained
> `--gp-symbols` and `--no-nop-mflo-mfhi` (`tools/patches/maspsx-lsd-flags.patch`,
> passed by the Makefile), the whole image stays byte-exact, and previously
> blocked functions now match (see `docs/research/gp-relative-blocker.md`,
> "RESOLVED"). Everything below is evidence from before the fix: its
> derivation may still be right, its VERDICT is not. Rebuild before believing
> any score in it.

Unit `code_179d8_s`, carved mid-round 17 (2026-09-04). **Not attempted.**

## Classification

```sh
grep -n 'gp_rel' asm/nonmatchings/code_179d8_s/GetCdUseVSyncCallback.s
```

Hit. The function is 3 instructions long and is a bare gp-relative load and
return, so there is nothing else in it to derive.

The pinned pipeline (`-G0` at both cc1 and `as`) cannot emit the
one-instruction `$gp` form from C; it emits the two-instruction absolute
`lui`/`lw` pair, and the extra word shifts every function after it in the
translation unit. See `docs/research/gp-relative-blocker.md` -- the `-G`
experiment was run in 2026-08-29 with operator authorisation and REJECTED.
Operator's call.

No C was written and no score was measured; the screen ran at carve time.

## Naming (round 64, runner alpha)

`func_80028B6C` -> `GetCdUseVSyncCallback`, tier A. Pure getter -- the whole
body is `return gCdUseVSyncCallback;` -- and per CLAUDE.md/FINISHING-PLAN.md
track 3, "a pure leaf whose mechanics ARE its purpose (a getter, a clamp, a
list push) is tier A by definition." `gCdUseVSyncCallback` itself was
already properly named (not a placeholder) before this round, by
`src/code_179d8_s.c`'s own header comment ("the driver mode:
gCdAsyncEnabled and gCdUseVSyncCallback, set through SetCdDriverMode"); no
further rename needed there.
