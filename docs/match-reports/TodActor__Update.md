# TodActor__Update

> Renamed from `Class65650__Update` on 2026-09-26 (tools/rename.py). Address 0x80065b80.

> Renamed from `TodActor__OnClass6EF50Notify` on 2026-09-25 (tools/rename.py). Address 0x80065b80.

> Renamed from `func_80065B80` on 2026-09-24 (tools/rename.py). Address 0x80065b80.

**Unit:** code_55dd4 · **Size:** 29 words (0x74 bytes) · **Status:** MATCHED
(29/29 words, whole-image `./build-and-verify.sh` green)

## What it does

Dispatches on a small integer `val`: if `2`, calls the class's own vtable
slot `+0x108` (`TodActor__Tick`) on `self`; if `4`, calls slot `+0x004`
(`BasicClass__Release`, inherited). Both are checked independently (two
separate `if`s, not `if/else if`), matching the two independent `bne`
guards in the disassembly.

```c
void TodActor__Update(TodActor *self, void *arg1, s32 val)
{
    if (val == 2) {
        self->methods->slot108(self);
    }
    if (val == 4) {
        self->methods->slot04(self);
    }
}
```

## The residue: a parameter that must exist but must not be *used*

First attempt wrote both calls as `slot108(self, arg1)` /
`slot04(self, arg1)`, forwarding this function's own second parameter —
reasoning by analogy with `TodActor__SetupModelData`'s forwarding shape. That compiled
29 words too **long** (30 vs 29) and every word from the prologue on
differed: with `arg1` referenced after a call (the first `if`'s call sits
between the two uses), GCC has to assume the callee might clobber `$a1` and
spills it into a saved register (`$s1`) across the call — an extra
`sw`/`lw` pair neither branch in retail has.

Retail's own bytes prove the opposite is true: the **first** call
(`slot108`) doesn't set up `$a0` OR `$a1` in its delay slot at all (a bare
`nop`) — it relies on both still holding their function-entry values,
including `$a1`, un-clobbered by anything in between. That is only
consistent with the call site **never naming a second argument** — not
"the value happens to survive", but "the C source never asked the compiler
to track it as live here". Removing `arg1` from both call sites (typing
`slot108`/`slot04` as taking only `self` in `src/code_55dd4.c`) is what
matched: with no reference to `arg1` after the call, GCC never spills it,
and `$a1` is left holding whatever it already had — which, from the
*outside*, looks like "forwarded", but which the source never actually
says.

`arg1` is kept in `TodActor__Update`'s own signature (unused) purely because
retail's caller-side convention still passes something in `$a1` here and a
future caller in this same slot family may turn out to need it — but this
function's *body* must not reference it.

### Proposed learning

Refines the "forward an argument nobody names" pattern from
`TodActor__PlayTone.md`: forwarding only reproduces retail when the forwarded
value is used **at most once, with no intervening call** (or, as here, used
zero times but still physically present in the register at entry). The
moment a source explicitly re-uses a parameter *after* a call that might
clobber it, GCC 2.6.3 must spill it to a saved register — so if retail's
disassembly shows a later call relying on an argument register that
survived an EARLIER call untouched, the source cannot have named that
argument at either call site. Check whether the surviving register's use
is genuinely referenced by the C, or merely a side effect of the callee
never being asked to preserve it.

## Naming

Round 75 (charlie), track 3.

- `TodActor__Update` (was `func_80065B80`), tier B. Occupies +0x098, which SceneNode__OnNotify (code_d294.c) dispatches to when the sender's header tag is 5 (TAG_CLASS6EF50, FrameClock). Code 2 calls tick (+0x108), code 4 calls release. Mechanics known; what Class6EF50 is (the tag-5 companion held in BaseObjO companion2) is not, hence B. Entity overrides this slot as Entity__Update.

## Track 4 (2026-09-25, round 85, alpha)

Renamed from `TodActor__OnClass6EF50Notify`. Override of +0x098, SceneNode's `update` (the slot SceneNode__OnNotify routes a class-5 FrameClock sender's events to), named for its slot as Entity's override of the same slot (Entity__Update) already is: code 2 runs tick (+0x108), code 4 release. The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/TodActor.h`. Any source block above is the pre-unification spelling; the live body in `src/code_55dd4.c` takes the unified types and slot names, byte-identical.

## Track 7 (round 99, bravo)

`sender`, `event`; events 2 and 4 are `FRAMECLOCK_EVENT_RUNNING` and `FRAMECLOCK_EVENT_FLAG14` (include/FrameClock.h, whose banner names this release on event 4): the ticker is the FrameClock this object keeps as Actor.ticker. Byte-identical.
