# ServiceCdDriver — MATCHED (49/49 words)

> Renamed from `func_800280EC` on 2026-09-17 (tools/rename.py). Address 0x800280ec.

Round 45, runner echo (second sitting), `src/cd/CdDriver.c`. Its address is
taken 3x elsewhere in the slice (this function itself, twice as a
`VSyncCallback` argument, once by `StartCdService`) — a function pointer.

## Result

Byte-exact, second attempt (one intermediate near-miss, see below).

The body below is the round-51 source, after track-3 naming. The
derivation notes that follow were written in round 45 against the same
code under its `unk` names; only names changed, the image is
byte-identical, and the `## Naming` section at the end of this report
carries the evidence for each one.

```c
extern s32 GetBMemPMgrBusy(void); /* TmdRenderer */
extern s32 gCdUseVSyncCallback;
extern s32 gCdTickStep;
extern void TickCdStateMachine(void); /* CdDriver: state-machine step 1 */
extern void TickCdLoadFileStateMachine(void); /* CdDriver: state-machine step 2 */
extern s32 sCdQueueEnabled;
extern void VSyncCallback(void (*cb)(void));

/* The class's method table down to +0x068 (see GetCdDriverMethods's
 * class-map comment above); only the one slot this call site dispatches is
 * typed, following the pad-to-offset convention include/GameApplicationFileResource.h uses
 * for gFileResourceMethods's own table. tools/classtable.py resolves +0x068 to
 * CdDriver__RunRequestQueue (CdDriver), which walks the gCdRequestQueue request list,
 * dispatches each request through its owner's own slots and frees it with
 * FreeCdRequestNode -- so the slot is named for what that method does. */
typedef struct Methods6D4E8_80EC Methods6D4E8_80EC;
struct Methods6D4E8_80EC {
    u8 pad00[0x68];
    void (*runRequestQueue)(void);
};

s32 ServiceCdDriver(void)
{
    if (sCdLock != 0) {
        return 0;
    }

    if (GetBMemPMgrBusy() != 0) {
        return 0;
    }

    if (gCdUseVSyncCallback != 0) {
        VSyncCallback(0);
    }

    if (gCdTickStep == 1) {
        TickCdStateMachine();
    } else if (gCdTickStep == 2) {
        TickCdLoadFileStateMachine();
    }

    if (sCdQueueEnabled != 0) {
        ((Methods6D4E8_80EC *)GetCdDriverMethods())->runRequestQueue();
    }

    if (gCdUseVSyncCallback != 0) {
        VSyncCallback((void (*)(void))ServiceCdDriver);
    }

    return 0;
}
```

## Derivation

Two early-return guards (the `sCdLock` latch, then `GetBMemPMgrBusy()`
gp_rel getter from `TmdRenderer`) both return literal `0` — **not** the
callee's own return value, even for the `GetBMemPMgrBusy()` guard. This was
the one wrinkle: an intermediate attempt captured `GetBMemPMgrBusy()`'s result
in a local and did `return result;`, reasoning that the branch target skips
straight to the epilogue with the call's return value still live in `$v0`
and therefore needing no extra move. That built clean (49/49 minus one word)
but was wrong — retail's delay slot for that `bnez` is `move v0,zero`, which
executes unconditionally (MIPS delay-slot semantics: it runs whether the
branch is taken or not) and is only semantically meaningful on the
branch-taken (early-return) path, where it overwrites the call's result
with 0 right before falling into the epilogue. So both guards are plain
`return 0;`, and the disassembly's `move v0,zero` in that slot is not
evidence of anything conditional — it is the same "if (cond) return 0;"
idiom as the first guard, just with the zeroing sharing a delay slot instead
of getting a fallthrough instruction of its own.

Body: an optional `VSyncCallback(0)` (`gCdUseVSyncCallback`), a two-way dispatch on
`gCdTickStep` (1 -> `TickCdStateMachine`, 2 -> `TickCdLoadFileStateMachine`, both in the
sibling `CdDriver` unit — declared extern here per the
per-call-site-typed convention `CdDriver.c` already established for
cross-unit libcd calls, now confirmed to apply to cross-unit game-code calls
too), an optional virtual dispatch through `gCdDriverMethods`'s own table slot
+0x68 (guarded by `sCdQueueEnabled`), and finally an optional
self-re-registration as a `VSyncCallback` (its own address, cast — the
callback type is `void (*)(void)` and this function is typed `s32 (void)`
for its early-return-0 paths, so the cast is required and harmless: nothing
ever reads a return value through the callback).

### Proposed learning

**A `bnez`/`beqz` branch's delay-slot instruction runs on BOTH paths, and
when it sets a register to a constant right before the branch target's
epilogue, that is ordinary `if (cond) return CONST;` — not evidence the
constant depends on the branch direction.** Getting this backwards (deciding
the delay-slot zero must apply only to the not-taken path, so the taken path
should preserve the call's live return value) produces C that still
compiles and still looks plausible, and the resulting diff is a single
clean word, not a structural mismatch — cheap to miss on a skim of the
diff output.

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800280EC` | `ServiceCdDriver` | A |
| `D_8008A890` | `sCdQueueEnabled` | A |

**Evidence.** This is the driver's tick, and it is installed as one: both
`StartCdService` and this function itself pass its address to
`VSyncCallback`, and `SetCdDriverMode` passes the same address to the
singleton's `+0x84` callback slot when the VSync path is off. One call does
all the periodic work there is -- skip if `sCdLock` is held or
`GetBMemPMgrBusy` says no; step `CdDriver`'s CD state machine
(`TickCdStateMachine` for `gCdTickStep == 1`, `TickCdLoadFileStateMachine` for 2); drain the
request queue through the class's own `+0x068` slot; re-arm itself. "Service"
is the one word that covers a tick that both advances a state machine and
drains a queue.

**`sCdQueueEnabled`.** Its only reader is the guard on the `+0x068` dispatch
here, and `tools/classtable.py` resolves that slot to `CdDriver__RunRequestQueue`
(CdDriver), which walks `gCdRequestQueue`, dispatches each request and frees
it with `FreeCdRequestNode`. So the flag gates queue processing specifically --
not the tick, which still runs the state machine while the flag is clear.
Tier A.

**Slot name.** `Methods6D4E8_80EC.slot68` -> `runRequestQueue`, named for the
method `classtable.py` resolves it to, per track 3's vtable-slot rule.

## Track 7 (round 101, echo): comments moved here, and names

`gCdTickStep`'s `1`/`2` are spelled `CD_TICK_STATE_MACHINE` /
`CD_TICK_LOAD_FILE` (CdDriver.h); `VSyncCallback(0)` is `VSyncCallback(NULL)`.
