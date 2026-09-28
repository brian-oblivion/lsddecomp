# EnableTeleportsForKind

> Renamed from `func_8005C714` on 2026-09-21 (tools/rename.py). Address 0x8005c714.

**Unit:** DreamAux · **Size:** 22 words · **Status:** MATCHED (22/22 words)

## What it does

Enables teleports (unconditionally, `SetInstantTeleportersEnabled(1)`) for four specific
trigger-type values: `0x4E`, `0xB`, `0x38`, `0x5D`.

## Derivation

```
li    v0, 0x4e
beq   a0, v0, CALL
slti  v0, a0, 0x4f
beqz  v0, .L748
li    v0, 0xb
beq   a0, v0, CALL
li    v0, 0x38
beq   a0, v0, CALL
 nop
j     END
.L748:
li    v0, 0x5d
bne   a0, v0, END
CALL:
jal   SetInstantTeleportersEnabled
 li   a0, 1
END:
...
```

Four constants, mutually exclusive under any natural grouping, reached via a
`< 0x4F` range split (`0xB` and `0x38` are `< 0x4F`; `0x5D` is not). The
straightforward C encodings of this — a single `||` chain, or `if/else if`
nesting — both produced WRONG bytes, and for an instructive reason:

- **A flat `a==A || (a<N && (a==B||a==C)) || a==D`** compiled correctly except
  for a missing 2-word unconditional jump after the `a==C` check: GCC let the
  "no match" fall straight into testing `a==D`, which is harmless behaviorally
  (`a<N` already rules out `a==D`) but is not retail's bytes — retail jumps
  straight to the shared epilogue instead.
- **Nested `if(a==B){call} else if(a==C){call}` inside `if(a<N){...} else if
  (a==D){call}`** is worse: GCC's cross-jump pass merged the `a==C` block's
  tail with the sibling `a==D` block's tail and, in doing so, DROPPED the
  `a==C` comparison outright (visible in `objdump`: `li v0,0x38` followed by
  an unconditional `j`, no `beq`). Two structurally-identical "compare then
  call-or-fall-to-end" blocks became fair game for cross-jumping even though
  their constants differ — a genuine GCC 2.6.3 `-O2` quirk to watch for
  whenever multiple sibling `if`-with-no-`else` blocks end in the same call.

Writing it as **`goto`**, mirroring retail's actual jump graph one-to-one, is
what produced the exact bytes:

```c
void EnableTeleportsForKind(s32 triggerType)
{
    if (triggerType == 0x4E) {
        goto call;
    }
    if (triggerType < 0x4F) {
        if (triggerType == 0xB) {
            goto call;
        }
        if (triggerType == 0x38) {
            goto call;
        }
        return;
    }
    if (triggerType != 0x5D) {
        return;
    }
call:
    SetInstantTeleportersEnabled(1);
}
```

## Proposed learning

When several sibling `if`-blocks with no `else` all end by calling the exact
same function with the exact same arguments, GCC 2.6.3's cross-jump/tail-merge
pass can be OVER-eager: it has been seen to merge two blocks whose GUARD
conditions differ, silently dropping one comparison, when both blocks'
"true" and "fallthrough" tails are structurally identical. If a residue looks
like "a branch just vanished, replaced by an unconditional jump, right where
two sibling blocks share a call", don't reach for `||` or nested `if/else`
first — write the literal jump graph with `goto` instead. This is the second
function in this unit (after `CheckTriggerDayParity`) where the byte-exact shape
depended on avoiding an optimization the compiler is eager to apply to more
"natural" C — worth trying `goto` earlier on this unit's branch-heavy leaves.

## Naming

**EnableTeleportsForKind** — tier B. Called as `EnableTeleportsForKind(record->kind)`
from `ProcessDreamAuxTriggerRecord`, right after `CheckDreamAuxTriggerCondition`
succeeds; unconditionally enables teleports (`SetInstantTeleportersEnabled(1)`) when `kind`
is one of `{0x4E, 0xB, 0x38, 0x5D}`, otherwise a no-op. The mechanics (which
four `kind` values, what they do) are fully derived and documented above;
renamed the formerly-`triggerType` parameter to `kind` to match the caller's
actual argument and this unit's own `TriggerRecord.kind` field. Tier B, not
A, because WHY these four values enable teleports (as opposed to some other
game-meaningful grouping) is not established from this unit alone.

## Round 100 (alpha): track 7, moved from src/world/DreamAux.c and include/DreamAux.h

Parameter `kind` -> `moodIndex`: ProcessDreamAuxTriggerRecord passes the
record's moodIndex, the same byte it passes New_Entity as its mood row. The
literals are written decimal (78, 79, 11, 56, 93): they are mood-row
indices. Left unnamed: nothing shows what those rows are. Byte-identical.
