# GetActiveDataSourceDriverMode

> Renamed from `func_80026FAC` on 2026-09-18 (tools/rename.py). Address 0x80026fac.

**Unit:** game_shell · **Size:** 15 words · **Status:** MATCHED, round 43
(2026-09-15, runner bravo). 15/15 words, byte-exact whole-image build.

## History

Never attempted (round-2026-08-29-a/30-a stub, `gp_rel`-blocked before any
derivation). Round 42 resolved `gp_rel`. Round 43 derived this fresh from
`asm/nonmatchings/code_171e0/GetActiveDataSourceDriverMode.s`; matched on the first build.

## What it does

Same `if`/`else` tail-call shape as `GetActiveDataSourceMethods` (see that report for the
full residue analysis), mode `0x13` this time:

```
beq $v1, $v0(0x13), .L80026FD0   # equal -> GetCdDriverMode
  jal GetNullDriverMode               # fallthrough (not equal)
  j .L80026FD8
.L80026FD0:
  jal GetCdDriverMode
.L80026FD8:
```

Both callees (`GetCdDriverMode`, `GetNullDriverMode`) are still uncarved
(`asm/code_179d8.s` / `asm/nonmatchings/PlacementGridVabSound/GetNullDriverMode.s`).
Treated as `s32`-returning per CLAUDE.md's tail-call caution (no positive
void evidence, so default to non-void).

## Final body

```c
extern s32 GetCdDriverMode(void);
extern s32 GetNullDriverMode(void);

s32 GetActiveDataSourceDriverMode(void) {
    if (sActiveDataSource == 0x13) {
        return GetCdDriverMode();
    } else {
        return GetNullDriverMode();
    }
}
```

## Proposed learning

See `GetActiveDataSourceMethods.md` — third instance of the "if/else, both arms tail-call"
family in this unit.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80026FAC` | `GetActiveDataSourceDriverMode` | B |

**Evidence.** Same if/else tail-call shape as `GetActiveDataSourceMethods`,
forwarding to `GetCdDriverMode`/`func_8002C448` -- the getter side of
`SetActiveDataSourceDriverMode`.

## History moved from src/code_171e0.c (round 99, charlie, track 7)

The two `arity-ok:` comments on the callee externs carried retail
addresses. They now keep only the reason `externcheck.py` needs. Moved
here unchanged:

> `extern s32 GetCdDriverMode(void);` -- arity-ok: the definition takes
> (s32 *outMode2) and the body reads $a0 (`beqz a0` at 0x80027EF8), but
> GetActiveDataSourceDriverMode's tail call sets nothing -- retail's jal at
> 0x80026FD0 has a nop delay slot
>
> `extern s32 GetVabDriverMode(void);` -- arity-ok: same as above -- the
> definition takes (s32 *arg0) and the body reads $a0 (`beqz a0` at
> 0x8002C448), but the call at 0x80026FC0 has a nop delay slot and sets
> nothing
