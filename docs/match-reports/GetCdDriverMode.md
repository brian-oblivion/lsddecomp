# GetCdDriverMode — MATCHED (8/8 words)

> Renamed from `func_80027EF8` on 2026-09-17 (tools/rename.py). Address 0x80027ef8.

Round 45, runner echo (second sitting), `src/CdDriver.c`.

## Result

Byte-exact on the first attempt.

```c
extern s32 gCdSyncQueueMode;
extern s32 gCdAsyncEnabled;

s32 GetCdDriverMode(s32 *a0)
{
    if (a0 != NULL) {
        *a0 = gCdSyncQueueMode;
    }
    return gCdAsyncEnabled;
}
```

## Derivation

Straight read of the disassembly: `beqz $a0, .L80027F0C` guards a store of
`gCdSyncQueueMode` (gp_rel) into `*a0`; fallthrough loads `gCdAsyncEnabled` (gp_rel) into
`$v0` and returns it unconditionally. Both globals are in the same
`gCdAsyncEnabled..sCdQueueEnabled` sdata block this unit's other getter/setters touch
(see `GetCdDriverMethods`'s header comment in the `.c` for the class map). No
class/struct involvement — plain scalar globals, plain optional-out-param
shape.

### Proposed learning

None beyond what's already on file for this unit (gp_rel getter/setter
pairs, section-of-declaration determines gp_rel vs lui/addiu). This is
another instance of the same shape, not a new lever.

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027EF8` | `GetCdDriverMode` | B |
| `D_8008A85C` | `gCdAsyncEnabled` | A |
| `gCdSyncQueueMode` | *(kept -- see below)* | C |

**Evidence for the function.** It is the read half of `SetCdDriverMode`:
it returns `gCdAsyncEnabled` and, through an optional out-parameter, the
second value that function stored. `GameApplicationFileResource.c`'s `GetActiveDataSourceDriverMode` routes to
it for the CD source and to `GetVabDriverMode` for the sound source -- and that
function is the same shape over that source's own two globals, which
independently confirms "read back the two mode values" rather than anything
specific to the CD. Tier B: the pairing is certain, the second value is not.

**Evidence for `gCdAsyncEnabled`.** Every reader branches the same way: when
it is non-zero, the class's methods in `CdDriver` set up the state machine
(or `EnqueueCdRequest` a node) and return immediately; when it is zero they
run a blocking `do { v = CdSync(0,0); } while (v == 0);` spin to completion.
`CdDriver__RequestLoadFile` shows the same split -- queue a request versus
call the method directly. `SetCdDriverMode` also uses it to decide whether to
install or clear the service callback. Asynchronous operation is what the
flag switches on; tier A.

**`gCdSyncQueueMode` keeps its placeholder, on purpose.** Its only writer stores
`SetCdDriverMode`'s second argument verbatim, and its only readers are this
getter and five guards in `CdDriver` of the form
`gCdAsyncEnabled == 0 && gCdSyncQueueMode == 0` -- i.e. "neither mode is on, take
the plain synchronous path". That tells you the two are alternative modes and
nothing at all about what the second one is. Naming it would be invention;
the `.c` now carries this paragraph as a comment instead.

## Extern arity (round 59)

**Verdict: arity-ok idiom.** `src/GameApplicationFileResource.c`'s `(void)` declaration stays.

**Callee evidence** (`0x80027EF8`): the very first instruction is `beqz a0,...`
— `$a0` is read before it is written, so the definition in
`src/CdDriver.c` (`s32 GetCdDriverMode(s32 *outMode2)`) is right: one real
argument, an optional out-pointer that is written only when non-NULL.

**Why the `(void)` extern is right anyway.** Its only carved caller,
`GetActiveDataSourceDriverMode` (this unit, matched), takes no arguments of its
own and sets none:

```
80026fd0:  jal   80027ef8 <GetCdDriverMode>
80026fd4:  nop                            <- no $a0 setup, in retail
```

`$a0` at that point is whatever `GetActiveDataSourceDriverMode`'s own caller
left there, and the callee's NULL test consumes it. Giving the extern its real
parameter would force this call site to materialise an argument retail does
not have. Same shape as `GetVabDriverMode` two lines down.

**Declaration sites changed:** none (arity unchanged). `/* arity-ok: ... */`
added to `src/GameApplicationFileResource.c:230`. Oracle green.

## Track 7 (round 101, echo): comments moved here, and names

Parameter `outMode2` -> `outSyncQueueMode`: it receives `gCdSyncQueueMode`.
