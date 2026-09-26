# TodActor__func_800661D4

> Renamed from `Class65650__func_800661D4` on 2026-09-26 (tools/rename.py). Address 0x800661d4.

> Renamed from `func_800661D4` on 2026-09-24 (tools/rename.py). Address 0x800661d4.

**Unit:** code_55dd4 · **Size:** 16 words (0x40 bytes) · **Status:** MATCHED
(16/16 words, whole-image `./build-and-verify.sh` green)

## What it does

`TodActorMethods` slot `+0x124`. Reads `self->arg2` (`+0x58`, the
constructor's stashed third parameter, of an unidentified class — see
`include/code_55dd4.h`'s new `UnkArg2Obj`/`UnkArg2Methods`, typed only at its
`+0x080` slot since that is all this function needs). If non-NULL, calls
that object's own vtable slot `+0x080` with **four** arguments: the object
itself, this function's own second parameter forwarded verbatim, and the
literal `0x6E` twice (once as `$a2`, once — separately, in the delay slot of
the `jalr` — as `$a3`).

```c
void TodActor__func_800661D4(TodActor *self, void *arg1)
{
    UnkArg2Obj *obj;

    obj = self->arg2;
    if (obj != NULL) {
        obj->methods->slot80(obj, arg1, 0x6E, 0x6E);
    }
}
```

`arg1` is never assigned inside this function — retail leaves `$a1`
untouched from entry, so whatever this function's own caller (some other,
unidentified class, since this is a virtual slot others call through) passed
as the second parameter rides straight through to the sub-call. This is the
same "forward an unused-here argument into a callee" shape as
`TodActor__SetupModelData`/`TodActor__AcquireModelData`.

Matched on the direct translation, no reshaping. `$v0` is never set on the
skip path (when `obj == NULL`), so the return value is meaningless there —
typed `void`.

### Proposed learning

When a virtual slot's own second parameter is never read in its body but is
passed on to a sub-call, that is enough evidence for the parameter's
existence and forwarding — you do not need to know the *caller* of the slot
to derive this, only that the sub-call's corresponding register is left
unset (i.e. retains its function-entry value) at the call site.

## Naming

Round 75 (charlie), track 3.

- `TodActor__func_800661D4` (was `func_800661D4`), tier C. Occupies +0x124 (classtable). If arg2 (+0x58, the ctor's third parameter) is set, calls arg2->methods->slot80(arg2, arg, 0x6E, 0x6E). Neither arg2's class nor slot80's occupant is identified and no caller of +0x124 on this class was found, so the placeholder stays with the class prefix.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/TodActor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
