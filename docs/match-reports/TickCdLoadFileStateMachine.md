# TickCdLoadFileStateMachine -- MATCHED (round 46): 88/88 words, byte-exact

> Renamed from `func_800286E4` on 2026-09-18 (tools/rename.py). Address 0x800286e4.

**Unit:** CdDriver · **Round 46** · **MATCHED**

## What it does

The sibling tick of the CD-read state machine implemented by
`TickCdStateMachine` (see that report for the full state description), called
from `ServiceCdDriver` when `sCdTickStep == 2`. Identical to `TickCdStateMachine`
in every state except:

- **state 2, `CdSync` reports "still busy, same phase"**: `TickCdStateMachine`
  calls `ResetCdStateMachine()` (full reset); this function instead just
  advances to phase 7 (`SetCdState(7)`) -- i.e. re-issue the read
  directly rather than resetting.
- **state 8, `CdReadSync` succeeds (`v1 == 0`)**: after
  `ResetCdStateMachine()`, this function additionally swaps two globals
  (`sCdSeekParam = sCdSavedSeekParam; sCdSavedSeekParam = NULL;`) that `TickCdStateMachine`
  does not touch at all.

## Round 45's stall, and what closed it

Round 45 left this two words from done (86/88), with the single residue a
`beq`/`bne` polarity flip and its paired jump target on the `state==2,
CdSync==5` check. Every C shape tried there put the `newstate = 1;`
assignment **inside** one arm of the `if`, either as
`if (v1 != 5) goto L_end; newstate = 1; goto L_set;` or its textual
mirror image `if (v1 == 5) { newstate = 1; goto L_set; } goto L_end;` --
both normalize to the identical `bne ...; goto L_end` encoding under GCC
2.6.3 -O2.

Retail's actual encoding sets `$a0 = 1` **unconditionally, in the branch's
own delay slot**, regardless of which way the branch goes:

```
ori  $v0, $zero, 0x5
beq  $v1, $v0, .L80028820      # v1==5: go apply newstate
ori  $a0, $zero, 0x1            # delay slot -- ALWAYS executes
j    .L80028828                  # v1!=5: fall through to L_end
nop
```

That is the signature of the assignment happening in source **before** the
comparison, not inside either branch arm:

```c
newstate = 1;
if (v1 == 5)
    goto L_set;
goto L_end;
```

This is a shape round 45 never tried -- both its attempts kept the
assignment conditional on the branch outcome (in the taken arm, or
equivalently in the fall-through arm via De Morgan). Hoisting it to run
unconditionally before the test let GCC schedule it into the branch delay
slot exactly as retail does, and the function came out byte-exact on the
first build after making that one change.

**Permuter:** not used. The fix was found by re-reading retail's own delay
slot before reaching for a search -- the `ori $a0, 1` sitting unconditionally
in the delay slot of the `beq` is direct positive evidence of where the
assignment sits in source, and it pointed straight at the untried shape.
Recording per the round's permuter-first instruction: none of the three
checks (correctness/cost/base-score-agreement) were run because the
lever closed without one.

## Final body (byte-exact)

```c
void TickCdLoadFileStateMachine(void)
{
    s32 state;
    s32 v1;
    s32 newstate;
    void *tmp;

    LockCd();
    state = sCdState;

    if (state == 2)
        goto L_state2;
    if (state < 3) {
        if (state == 1)
            goto L_state1;
        goto L_end;
    }
    if (state == 7)
        goto L_state7;
    if (state == 8)
        goto L_state8;
    goto L_end;

L_state1:
    if (CdControlF(2, (u8 *)sCdSeekParam + 0x14) == 0)
        goto L_end;
    newstate = 2;
    goto L_set;

L_state2:
    v1 = CdSync(1, NULL);
    if (v1 == state)
        goto L_busy;
    if (v1 < 3) {
        if (v1 == 0)
            goto L_count;
        goto L_end;
    }
    newstate = 1;
    if (v1 == 5)
        goto L_set;
    goto L_end;

L_busy:
    newstate = 7;
    goto L_set;

L_count:
    sCdTimeoutCounter++;
    if (sCdTimeoutCounter < 0x259)
        goto L_end;
    newstate = 1;
    goto L_set;

L_state7:
    if (CdRead(sCdReadSectorCount, sCdReadBuffer, 0x80) == 0)
        goto L_end;
    newstate = 8;
    goto L_set;

L_state8:
    v1 = CdReadSync(1, 0);
    if (v1 == -1) {
        newstate = 1;
        goto L_set;
    }
    if (v1 != 0)
        goto L_end;
    ResetCdStateMachine();
    tmp = sCdSavedSeekParam;
    sCdSavedSeekParam = NULL;
    sCdSeekParam = tmp;
    goto L_end;

L_set:
    SetCdState(newstate);

L_end:
    UnlockCd();
}
```

### Proposed learning

When a same-length, same-shape residue is a single `beq`/`bne` polarity
flip, look at what sits in the **branch's own delay slot** in retail
before trying more `if`/`goto` rephrasings of the comparison itself. A
value set unconditionally in the delay slot (i.e. it executes on both
the taken and fall-through paths) is retail telling you the assignment in
source ran **before** the branch, not inside one of its arms -- the two
phrasings look equivalent at the C level (both assign `newstate` exactly
once before it's used) but only one puts the store where the delay slot
can absorb it "for free" under -O2's scheduling. Round 45 tried both
`if (v1 != 5) goto X; newstate = 1;` and its De Morgan mirror
`if (v1 == 5) { newstate = 1; ... }` -- both keep the assignment inside an
arm and both compiled to the same (wrong) encoding; hoisting it above the
`if` entirely was the untried third option and it matched immediately.

## Naming

**Tier B.** The state machine's other tick function, selected by
`ServiceCdDriver` when `sCdTickStep == 2`. Grepping every `sCdTickStep = 2`
assignment in cd_driver.c finds exactly one: `CdDriver__LoadFile`, which
`docs/match-reports` for the class's method table (`GetCdDriverMethods`'s
own comment, cd_driver.c) identifies via `tools/classtable.py` as the
`loadFile` slot (+0x58) of class `D_6D4E8` -- i.e. the
`CdDriver__RequestLoadFile` worker. The two mechanical differences from
`TickCdStateMachine` both make sense for that one operation: on the
phase-2 "still busy" signal it proceeds straight into the read phase
(`newstate = 7`) instead of resetting, because a LoadFile always intends a
read to follow the seek; and on a successful read it restores
`sCdSeekParam` from `sCdSavedSeekParam`, because `CdDriver__LoadFile` is the one
call site that stashes the caller's previous `sCdSeekParam` there before
overwriting it with the file it looked up (`sCdSavedSeekParam =
sCdSeekParam; ... sCdSeekParam = rec;`). "LoadFile" names the operation this
function is used for, established by the classtable evidence above, not a
guess -- kept tier B because the report can name the caller and the effect
but not independently confirm from this unit alone why LoadFile specifically
needs the differences (as opposed to it merely being how retail happened to
implement it).

## Round 101 (track 7 polish): the goto dispatch is a switch

Rewritten the same way as TickCdStateMachine (see its report): a switch on
sCdState's `CD_STATE_*`, an inner switch on CdSync's `CdlComplete` /
`CdlNoIntr` / `CdlDiskError` in that natural order, `newState` and one
`SetCdState(newState)` after it. Byte-exact on the first build. Round 45's
residue, the `== 5` test's polarity, is what the natural case order gives
here; TickCdStateMachine needs `CdlDiskError` first to get the other
encoding. The `tmp` temporary is gone: `sCdSeekParam = sCdSavedSeekParam;
sCdSavedSeekParam = NULL;` compiles to the same load/store order.
Constants as TickCdStateMachine's report lists.
