# TryStartStyleCue -- MATCHED (45/45 words), class_3bb8c_n

> Renamed from `func_8005556C` on 2026-09-23 (tools/rename.py). Address 0x8005556c.

Round 46 (second sitting, alpha). Byte-exact, whole-image SHA1 verified.

## Round 47 (bravo) addendum: signature widened again, this time NOT a dead param

While deriving `FindNearestStyleCueEntry` (this unit, cold-fresh this round -- see its
own report), found that its first real instruction reads and branches on
`$a2`, and objdump of THIS function's already-matched object shows nothing
sets `$a2` before that call -- `$a2` still holds `TryStartStyleCue`'s own third
parameter (`ctx`, called "dead" in `TickStyle`'s round-46 report,
correctly, from `TryStartStyleCue`'s OWN body's point of view) forwarded
silently because no other value needed that register in between. Widened
the call site from `FindNearestStyleCueEntry(&arg0->unk4, &arg0->unk10)` to
`FindNearestStyleCueEntry(&arg0->unk4, &arg0->unk10, arg2)`, and widened the forward
declaration accordingly. Rebuilt: **`TryStartStyleCue` still scores 45/45**,
whole-image SHA1 still verifies -- the added argument costs nothing because
the register already held the right value. This is the argument-side
counterpart to round 46's own "already-matched signature can be too narrow"
finding -- there it was about the CALLEE's params; here it is about a call
site under-declaring what it silently forwards.

## Signature

```c
ObjN14 *TryStartStyleCue(ObjN14 *arg0, s32 *arg1, void *arg2, void *arg3);
```

(`arg2`/`arg3` were already added as dead params by `TickStyle`'s
round-46 report; `arg2` turned out to be genuinely forwarded, per above --
"dead" was correct for THIS function's own body, not for what it passes on.)

Returns `NULL` on allocation failure or `arg0` itself on success (a
self-returning validity idiom, not the sub-object) -- confirmed straight off
the epilogue: the null path sets `v0 = 0`, the success path sets
`v0 = s1` (== `arg0`), both feeding the same `jr ra`.

## New externs

```c
extern s32 gStyleTargetObj;                                 /* fresh copy -- see below */
extern void *FindNearestStyleCueEntry(void *arg0, s32 *arg1, void *arg2);  /* forward decl, own unit,
                                                          111w, STALL -- widened round 47,
                                                          see FindNearestStyleCueEntry.md */
extern s32 D_800874B0[];                                /* 14-slot table, class_3bb8c_r.c's D_800874B0 */
extern s32 InitSoundCueSet(s32 arg0, void *arg1, s32 arg2, void *arg3, s32 arg4);
```

`InitSoundCueSet` is defined in `src/code_179d8_e.c`
(`s32 InitSoundCueSet(void *unused, ObjCC34 *obj, s32 arg2, void *arg3, s32
arg4)`); this call site only needs `void *`/`s32` at the ABI level (matches
the looser local signatures `Entity.c` and `DreamSys.c` already use for the
same cross-unit call, per the multiple-independent-local-views convention).
`gStyleTargetObj` is redeclared fresh here (not reusing the copy later in this
file for `FlushStyleCue`/`StopStyleCueIfNear`) because this function's ROM
address is earlier -- same pattern as `StyleUpdateEffectSlots`'s fresh `gStyleVariant`
copy. `D_800874B0` is `class_3bb8c_r.c`'s already-identified 14-function
table (its own `ParamMethods` slot list); here it is read as a raw `s32`
bit pattern (a function pointer forwarded opaquely as `InitSoundCueSet`'s 5th
argument, which just stores it into `obj->unkC` -- confirmed by reading that
function's body) rather than typed as a function-pointer array, since
nothing here calls through it.

## Body

```c
ObjN14 *TryStartStyleCue(ObjN14 *arg0, s32 *arg1, void *arg2, void *arg3) {
    ObjN14Sub *sub;

    sub = (ObjN14Sub *) FindNearestStyleCueEntry(&arg0->unk4, &arg0->unk10, arg2);
    if (sub != 0) {
        arg0->unk0 = sub;
        InitSoundCueSet(*(s32 *) gStyleTargetObj, &arg0->unk14, sub->unk6, arg0, D_800874B0[sub->unk6]);
        if (sub->unk6 == *arg1) {
            *arg1 = -sub->unk6;
        }
        sub->unk6 = -sub->unk6;
        return arg0;
    }
    return 0;
}
```

`sub->unk6` (an `s8`) is re-read from memory THREE separate times after the
`InitSoundCueSet` call (index computation, the `==` compare, and the final
negate) -- matches retail exactly (a fresh `lb`/`lbu` at each site, never
cached in a register across the call), because the compiler cannot prove
`InitSoundCueSet` doesn't write back through the `&arg0->unk14` alias.

## Lever: which branch is the "early return" determines an extra `j` or not

First attempt wrote the natural failure-first idiom
(`if (sub == 0) { return 0; } <success continues inline>`), which cost ONE
extra word: GCC placed the `return 0` stub *inline*, right after the check,
forcing the success path (the longer remainder of the function) to jump
*over* it to reach the epilogue. Retail instead treats the SUCCESS branch as
the `if`-body and lets the trivial `return 0` fall out at the very end,
sharing the epilogue directly with no extra jump:

```c
if (sub != 0) {
    ...
    return arg0;
}
return 0;
```

Same logic, opposite polarity -- but this ordering lets GCC put the short
`v0 = 0` stub immediately before the shared `jr ra` epilogue (no jump needed
to reach it) while the long success path ends with the one jump it already
needs to skip over that stub. Reproduced immediately once tried; no permuter
needed.

### Proposed learning

**A same-length-but-shifted-by-one-word residue on a "check a pointer,
return NULL or continue" function is worth re-polarizing the guard**: write
the LONGER continuation as the `if`-body and the trivial early-return as the
final fall-through statement, not the reverse. GCC 2.6.3 prefers to tuck a
short return stub immediately before the shared epilogue (reachable by a
direct branch, no extra jump) rather than keep it inline right after the
check (which forces the longer path to jump over it). This differs from the
round-45 "invert the guard" lever (headline confirmed, one-word-cost
corollary withdrawn) in scope: that one was about which value falls into
which arm; this one is about which arm's CODE ends up adjacent to the
epilogue, and it cost a real word here.

## Attempts

2 (first: null-check-first idiom, one word short/shifted; second: success-
first idiom, byte-exact).

## Naming

**`TryStartStyleCue`, tier B.**

Looks up a nearby record via `FindNearestStyleCueEntry`; on success,
claims it into `arg0->entry`, starts `InitSoundCueSet` on the slot's
embedded `cueSet`, and toggles the claimed entry's sign tag so it will not
be picked twice. Called from `TickStyle` for each of the two
`gStyleCueSlots` when that slot is empty. "TryStart" over a bare "Start"
because failure (returning `NULL`) is a real, handled path, not an error --
the slot stays empty and `TickStyle` retries next frame (implicit from the
call site's `gStyleCueSlots[i] = TryStartStyleCue(...)` pattern, no
error-log or assert on failure). MATCHED, 45/45.
