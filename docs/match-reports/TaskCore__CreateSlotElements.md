# TaskCore__CreateSlotElements — MATCHED (66/66)

> Renamed from `Obj86B60__CreateSlotElements` on 2026-09-25 (tools/rename.py). Address 0x8003d5cc.

> Renamed from `func_8003D5CC` on 2026-09-24 (tools/rename.py). Address 0x8003d5cc.

**Unit:** code_2cc8c_b · **Size:** 66 words · **Result:** byte-exact

## What it does

The constructor that FILLS `self->unk64[idx]` — the array the sibling
functions (`TaskCore__BroadcastToSlotElements`, `TaskCore__BroadcastToSlots`, `TaskCore__BeginElementScroll`,
`TaskCore__SetSlotCursor`) all walk as `Unk64Elem **`. Given a caller-supplied "source
list" descriptor (`SrcDesc *a1`, a NULL-terminated array of C strings plus
one extra word), it counts the strings, allocates a same-sized array of
`Unk64Elem *`, installs it (and the count, and `a1`'s extra word) into
`self->unk64[idx]`/`self->unk5C[idx]`/`self->unk60[idx]`, then populates
the new array by resolving each string through `New_TextRow` (with its
length from `func_80013348`, a strlen-shaped helper already matched
elsewhere in the project).

```c
void TaskCore__CreateSlotElements(Obj86B60 *self, SrcDesc *a1, void *a2)
{
    char **list;
    s32 idx;
    s32 count;
    Unk64Elem **buf;

    list = a1->unk18;
    idx = self->unk58;
    count = 0;
    while (*list++ != NULL) {
        count++;
    }
    buf = BMemPMgrAlloc(count * 4);
    self->unk64[idx] = (void *)buf;
    self->unk60[idx] = a1->unk4;
    self->unk5C[idx] = count;

    list = a1->unk18;
    if (*list != NULL) {
        do {
            s32 len = func_80013348(*list);

            *buf = New_TextRow(a2, len, *list);
            list++;
            buf++;
        } while (*list != NULL);
    }
}
```

## Header additions

`include/code_2cc8c.h`:

- New type `SrcDesc` for the 2nd parameter — a descriptor unrelated to
  `Obj86B60`'s own class hierarchy (no method-table dispatch anywhere in
  this function). Only its two touched fields are modelled: `unk4` (`s32`)
  and `unk18` (`char **`, NULL-terminated).
- `extern s32 func_80013348(char *s);` — already matched in
  `code_171e0.c`/`src/code_171e0.c` as a strlen-shaped helper; this unit
  keeps its own local view (established convention).
- `extern Unk64Elem *New_TextRow(void *ctx, s32 len, char *name);` — not
  previously seen in this project. Typed from this call site: its return
  value is stored directly into the same array `TaskCore__BroadcastToSlotElements`,
  `TaskCore__BroadcastToSlots`, `TaskCore__BeginElementScroll` and `TaskCore__SetSlotCursor` all walk as
  `Unk64Elem *`, which settles the return type without needing to see
  the function's own body.

No existing declaration's type or offset changed.

## The one residue, and how it closed

First attempt used a plain pre-test `while (*list != NULL) { count++;
list++; }` for the counting loop and compiled 2 WORDS LONGER than retail
(68 vs. retail's 66) — confirmed directly with `objdump -d
build/src/code_2cc8c_b.c.o` (the "outside this range" warning was six
figures because the size mismatch shifted everything after this function
in the whole linked image, not because of anything genuinely wrong
elsewhere).

Reading the disassembly closely: retail's pointer increment
(`addiu s0,s0,4`) sits in the delay slot of the loop's OWN `bnez`/`beqz`
branch, meaning it executes unconditionally on every pass through the
check — including the FINAL pass that finds the null terminator and exits
the loop. That is the `while (*p++) {}` post-increment idiom (see
DECOMPILATION_LEARNINGS, previously closed on `strcat` and reused this
round on `TaskCore__FindNextFreeSlot`'s free-slot search): the pointer always advances
one PAST where it stopped, and nothing after the loop cares because
`list` is reloaded fresh from `a1->unk18` before the second loop anyway.
**Fix: `while (*list++ != NULL) { count++; }`.** Matched immediately, no
compensating decrement needed since the incremented pointer is never read
again before being reloaded.

### Proposed learning

**Third confirmed instance of the post-increment `while (*p++)` idiom in
this unit alone** (after `strcat` project-wide and `TaskCore__FindNextFreeSlot`'s
free-slot search this round) — and the first where the "off" pointer value
is simply never used again, so no compensating `p--;`/`i--;` was needed at
all. The tell that distinguishes this from an ordinary counting loop is
the SAME as before (the increment sits in the branch's own delay slot,
executing even on the exiting pass) — but the FOLLOW-UP question worth
adding is "is the incremented value read again before being
reinitialized?" If not, the idiom is a pure size-fix with no cleanup
needed, which is easy to miss when the naive `while` translation looks
completely reasonable and the only symptom is a two-word size mismatch
rather than a visible register residue.

## Naming (round 78, naming runner echo)

Renamed `func_` -> `Obj86B60__CreateSlotElements`. **Tier B**: Builds `self->itemLists[idx]` (allocates an array sized off a NULL-terminated string list, resolves each string through `New_TextRow`) and installs the count/initial cursor -- the constructor for one slot's own item list, called at whatever slot is currently `activeSlot`. Pairs with TaskCore__ReleaseSlotElements.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__CreateSlotElements (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
