# TaskCore__SetSubHandle — MATCHED (46/46)

> Renamed from `Obj86B60__SetSubHandle` on 2026-09-25 (tools/rename.py). Address 0x8003cde0.

> Renamed from `func_8003CDE0` on 2026-09-24 (tools/rename.py). Address 0x8003cde0.

**Unit:** task · **Size:** 46 words · **Result:** byte-exact

## What it does

Replaces `self`'s cached sub-resource handle (`self->unk74`). If a path
string is given, tears down the old handle (if one was live), loads a new
one via `New_TimImage` (already matched in `DayTaskStageMap.c`), and runs two
init calls on it. If no path is given, the caller-supplied handle `a2` is
installed directly with no teardown/init calls at all.

```c
void TaskCore__SetSubHandle(Obj86B60 *self, const char *a1, Unk74Obj *a2)
{
    if (a1 != NULL) {
        if (self->unk70 != NULL) {
            self->unk74->methods->slot4(self->unk74);
        }
        self->unk74 = New_TimImage(a1);
        self->unk74->methods->slot78(self->unk74);
        self->unk74->methods->slot5C(self->unk74);
    } else {
        self->unk74 = a2;
    }
    self->unk70 = a1;
}
```

No control-flow reshaping was needed — a straightforward `if`/`else`
translation of the branches matched the CFG immediately, including the
detail that `self->unk70 = a1;` (identical statement) is reproduced once in
each arm by the compiler rather than needing to be written twice by hand.

## Header additions

`include/task.h`:

- New type `Unk74Obj`/`Unk74ObjMethods` for `self->unk74`'s pointee, with
  three call sites resolving three slots:
  - `slot4` at **+0x004** (not +0x000 — see the residue note below),
    return value discarded at its one call site here, so typed `void *`
    rather than assuming a specific return type it is never used for.
  - `slot5C` at +0x05C, `void (*)(Unk74Obj *)`.
  - `slot78` at +0x078, `void (*)(Unk74Obj *)`.
- New fields on `Obj86B60`: `unk70` (`const char *`, a truthy gate that also
  doubles as a cache of the last path passed in) and `unk74` (`Unk74Obj *`),
  carved out of what had been undifferentiated padding
  (`pad05C[0x078-0x05C]`). This is a new field inside previously-unread
  padding, not a change to any already-typed field, so it does not affect
  `task.c`'s 16 already-matched functions.
- `extern Unk74Obj *New_TimImage(const char *path);` — a local retyped view
  of the already-matched `DayTaskStageMap.c` function of the same name, which
  there returns its own unit's local view `SubObjG *`. Per this project's
  established multiple-independent-local-views convention (see
  `DayTaskStageMap`/`class_3bb8c` in DECOMPILATION_LEARNINGS), this unit keeps
  its own view rather than including `DayTaskStageMap.h`.

## The one residue, and how it closed

First attempt (before the `pad000[0x004]` fix below) scored 45/46: one word
off, `lw v0,0x4(v0)` (retail) vs `lw v0,0x0(v0)` (built) at the `slot4` call.
**Cause:** I had declared `slot4` as the FIRST member of
`Unk74ObjMethods`, commented `/* +0x004 */`, but a C struct's first member
always sits at offset 0 regardless of the comment — the comment was
aspirational, not enforced. Fixing it needed an explicit `u8 pad000[0x004];`
before `slot4`, **and** correcting the following padding member's size from
`pad008[0x05C - 0x004]` to `pad008[0x05C - 0x008]` (it had been sized as if
`slot4` started at 0, so it was 4 bytes too large once `slot4` moved to its
correct 4-byte slot at +0x004..+0x008). Both mistakes stacked into the same
diagnostic in different rebuilds (first a wrong slot4 offset, then after the
partial fix, both slot78 and slot5C landed 4 bytes past where they should).

### Proposed learning

**A method-table struct's first member is at struct offset 0 no matter what
offset comment you write next to it.** If the real vtable slot is not at
+0x000, an explicit leading `u8 pad000[N];` is required — and when you add
one, re-derive every subsequent padding member's size from the corrected
running offset, not from the original (now-wrong) one. A single missed
padding-size correction after inserting a leading pad reproduces the exact
same class of off-by-N error one struct member later. (`TaskCore__SetSubHandle`,
Unk74ObjMethods slot4/slot78/slot5C)

## Naming (round 78, naming runner echo)

Renamed `func_` -> `Obj86B60__SetSubHandle`. **Tier A**: A setter for `self->unk74`/`self->unk70` (load-by-path-or-install-directly): pure load-or-install mechanics, no purpose beyond that to guess at -- tier A by the 'a setter is tier A by definition' rule.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__SetSubHandle (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 88)

TimImage is unified (`include/tim_image.h`) and `include/task.h`'s
`Unk74Obj`/`Unk74ObjMethods` view of it is deleted, together with that
header's local `extern` of New_TimImage. The handle is cast to
`TimImage *` (TaskCore.h still types the field `BasicClass *`); its slots are
TimImage's: +0x004 `release`, +0x05C `freeBuffer` (was `slot5C`), and
+0x078, FileResource's `void *slot78` whose occupant is TimImage__Upload,
called through `TimImageUploadFn`. `path` is cast to `char *` for
New_TimImage. Image byte-identical.
