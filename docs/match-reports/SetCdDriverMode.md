# SetCdDriverMode — MATCHED (48/48 words)

> Renamed from `func_80027F18` on 2026-09-17 (tools/rename.py). Address 0x80027f18.

Round 45, runner echo (second sitting), `src/cd/cd_driver.c`.

## Result

Byte-exact, second attempt (one intermediate near-miss, see below).

The body below is the round-51 source, after track-3 naming. The
derivation notes that follow were written in round 45 against the same
code under its `unk` names; only names changed, the image is
byte-identical, and the `## Naming` section at the end of this report
carries the evidence for each one.

```c
extern s32 GetDrawSystem(void); /* returns sDrawSystem, a singleton object */
extern s32 ServiceCdDriver(void);
extern s32 sCdBusy;
extern s32 sCdAsyncEnabled;
extern s32 sCdSyncQueueMode;
extern s32 sCdUseVSyncCallback;

/* The singleton GetDrawSystem returns; only the slot this call site
 * dispatches (+0x84 of its method table) is typed here. That slot is handed
 * either ServiceCdDriver or 0, so it installs and clears a callback -- named
 * for what this one call site does with it, which is all the evidence
 * there is. */
typedef struct ObjF18Methods ObjF18Methods;
struct ObjF18Methods {
    u8 pad00[0x84];
    void (*setCallback)(void *self, void *cb);
};

typedef struct ObjF18 ObjF18;
struct ObjF18 {
    ObjF18Methods *methods;
};

s32 SetCdDriverMode(s32 async, s32 mode2, s32 useVSyncCallback)
{
    ObjF18 *obj;

    if (sCdBusy == 0) {
        if (useVSyncCallback == 0) {
            obj = (ObjF18 *)GetDrawSystem();

            if (sCdAsyncEnabled == 0) {
                if (async != 0) {
                    obj->methods->setCallback(obj, (void *)ServiceCdDriver);
                }
            } else {
                if (async == 0) {
                    obj->methods->setCallback(obj, 0);
                }
            }
        }

        sCdUseVSyncCallback = useVSyncCallback;
        sCdAsyncEnabled = async;
        sCdSyncQueueMode = mode2;

        return 1;
    }

    return 0;
}
```

## Derivation

Two wrinkles, both caught by the first attempt's near-miss (21/48, with
drift) and corrected on the second:

1. **The whole-body guard is `if (cond == 0) { body; return 1; } return 0;`,
   not `if (cond != 0) { return 0; } body; return 1;`.** Retail's
   `sCdBusy != 0` check branches DIRECTLY to the shared `move v0,zero`
   tail already sitting at the very end of the function (right after the
   `return 1` tail), rather than to a duplicate `v0=0`/jump pair inlined at
   the top. Writing the guard as an early `if (cond) return 0;` makes GCC
   duplicate that tail at the entry instead of reusing the one at the end,
   adding two words and shifting everything after. Wrapping the entire rest
   of the function in `if (sCdBusy == 0) { ...; return 1; }` followed by
   a single trailing `return 0;` reproduces retail's single physical copy.
2. **The virtual dispatch takes an explicit `self` argument, not just the
   callback.** `obj->methods->setCallback(callback)` (`slot84` when this was written) compiles the callback into
   `$a0` (the sole argument register for a one-arg call); retail sets
   `$a0 = obj` and `$a1 = callback` — i.e. the slot's real signature is
   `setCallback(self, cb)`, consistent with CLAUDE.md's "explicit `this` first
   parameter" convention for this codebase's hand-rolled method tables. The
   near-miss diff showed `a0`/`a1` register roles and the materialized
   `&ServiceCdDriver` address swapped between them, which was the tell.

`GetDrawSystem` is declared exactly as `DayTaskStageMap.c` already declares it
(`extern s32 GetDrawSystem(void);`, cast to a pointer type at the call
site) — reused convention, not a new one. `ServiceCdDriver` (this unit,
matched earlier this round) needed only a forward `extern s32
ServiceCdDriver(void);` since this function sits earlier in ROM order.

### Proposed learning

**A guard of the shape "if (early-exit condition) return CONST;" at the top
of a function is not always how the source read — check whether retail's
branch target is a physically separate `return CONST` block placed
elsewhere (often right at the function's tail) before assuming an
early-return at the top.** GCC 2.6.3 only reuses one copy of a trivial
return sequence when the source itself has one physical `return` statement;
an early-return at the top and a matching trailing return are DIFFERENT
statements and get compiled as two separate code blocks. Where retail's
would-be-early-exit branch jumps to code positioned at the very end (past
the "normal path" return), the C is `if (cond == 0) { body; return X; }
return Y;`, not `if (cond) return Y; body; return X;` — same semantics,
different word count.

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027F18` | `SetCdDriverMode` | B |
| `D_8008A8A4` | `sCdUseVSyncCallback` | A |

**Evidence for the function.** It refuses (returns 0) while `sCdBusy`, and
otherwise stores its three arguments into `sCdUseVSyncCallback`,
`sCdAsyncEnabled` and `sCdSyncQueueMode` and returns 1 -- the write half of the
pair `GetCdDriverMode` reads back. `GameApplicationFileResource.c`'s `SetActiveDataSourceDriverMode` calls it
in a `do {} while (fn(...) == 0)` loop, i.e. "retry until the driver accepts
the new mode", which is what the refusal-while-busy return value is for.
Parameters are now named `async`, `mode2`, `useVSyncCallback`. Tier B: the
second argument is unidentified (see `GetCdDriverMode.md`), so the function's
full contract is not established.

**Evidence for `sCdUseVSyncCallback`.** Every one of its five readers is
`if (sCdUseVSyncCallback != 0) VSyncCallback(...)` -- register or clear the
tick. And this function only installs the ALTERNATIVE delivery path (the
`+0x84` slot of the singleton `GetDrawSystem` returns, handed
`ServiceCdDriver` or 0) when the argument is zero. So the flag chooses which
of two callbacks drives the service; tier A.

**Slot name.** `ObjF18Methods.slot84` -> `setCallback`: this call site hands
that slot either `ServiceCdDriver` or `0`, which is install/clear and nothing
else. Tier B -- one call site is thin evidence for another class's slot, and
the type stays a per-call-site local view.

## Track 4 (2026-09-26, round 87, bravo)

The local view of the DrawSystem singleton quoted above is gone; the unit takes DrawSystem, its method table and GetDrawSystem from `include/draw_system.h` (gDrawSystemMethods unified). Byte-identical.

## Track 7 (round 101, echo): comments moved here, and names

Parameter `mode2` -> `syncQueueMode` (it is stored in `sCdSyncQueueMode`),
local `obj` -> `drawSystem` (GetDrawSystem's result); `setCallback(obj, 0)`
is `setCallback(drawSystem, NULL)`. GetCdDriverMode's `outMode2` is
`outSyncQueueMode` for the same reason.
