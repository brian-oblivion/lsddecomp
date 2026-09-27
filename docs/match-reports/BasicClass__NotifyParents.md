# BasicClass__NotifyParents — MATCHED (33/33 words)

> Renamed from `BasicClass__func_182cc` on 2026-09-17 (tools/rename.py). Address 0x800182cc.

Unit: `src/TmdRenderer.c`. This is `BasicClassMethods` vtable slot `+0x030`,
`notifyParents(self, s32 flag)` (already documented in
`include/code_8220.h`'s struct comment). Walks `self->parentRefs` and, for
each parent, calls that parent's own `slot38` (`BasicClass__OnNotify`,
already matched in this unit) with `self` as its `arg1` and `flag` passed
through as `arg2`. Since `slot38` only acts `if (arg2 == 1)`, calling
`notifyParents(self, 1)` (as `BasicClass__Finalize`'s finalize path does)
tells every parent holding a reference to `self` to remove `self` as its
own child.

## Final source

```c
void BasicClass__NotifyParents(BasicClass *self, s32 arg1)
{
    BasicClassListNode *cursor = self->parentRefs;
    BasicClass *value;

    for (GetNextBasicClass(&value, &cursor); value != NULL; GetNextBasicClass(&value, &cursor)) {
        value->methods->slot38(value, self, arg1);
    }
}
```

## Notes

Byte-exact on the first attempt. The disassembly's `j` straight to the loop
condition before the first iteration is the classic `for`/`while`
lowering — GCC emits `init; goto cond; body: ...; cond: test; branch`. Since
this function's "condition" is itself a call with a side-effecting output
parameter (`GetNextBasicClass` pops the cursor and writes `*value`), the natural
C spelling is a `for` loop whose init AND increment clauses are both that
same call, with the body running only the vtable dispatch. Writing it as a
`while (GetNextBasicClass(&value, &cursor), value != NULL)`-style comma
expression was unnecessary — the plain `for (call; test; call) { body }`
form reproduces the goto-to-condition shape directly and needs no unusual
syntax.

Register mapping confirms the earlier read of the call site: `a0` = the
value popped off the cursor (`this` for the `slot38` call), `a1` = `self`
(the finalizing object, cast to `void *` for `slot38`'s `arg1`), `a2` =
`arg1` (the finalize flag, passed straight through as `slot38`'s `arg2`) —
matching `BasicClassMethods.slot38`'s signature
`void (*)(BasicClass *self, void *arg1, s32 arg2)` already in the header.

## Naming (round 51, bravo)

`BasicClass__func_182cc` -> `BasicClass__NotifyParents`. **Tier A** -- the
body is a dispatch loop and the loop IS the purpose.

Evidence: walks `self->parentRefs` with `GetNextBasicClass` and calls each
parent's own `+0x038` slot as `slot38(parent, self, event)`. That is
"tell everything holding a reference to me that `event` happened", stated
directly by the code. `tools/classtable.py` across all 60 method tables:
57 carry this exact address in slot `+0x030`, and the three that do not
(`StyleCue11`, `FrameClock__NotifyParents`, `func_80022D0C`) are overrides, so the
slot is genuinely BasicClass's.

`arg1` renamed to `event`: it is passed straight through to `slot38`'s
third parameter, and the base `slot38` acts only on the value 1 while the
overrides forward the same value onward (see
`docs/match-reports/BasicClass__OnNotify.md`). Nothing treats it as a
boolean.

### Proposed field name for the head

`BasicClassMethods::notifyParents` (slot `+0x030`) -> `notifyParents`.
**Tier A**, by the vtable-slot convention (name the slot after the method
it dispatches to). The inherited `notifyParents` is wrong twice: this is the
EMITTER, not an `on*` handler, and it is not finalize-specific --
`BasicClass__Finalize` (finalize) happens to be the only caller in carved
C, which is a fact about how much is carved, not about the slot.
Cross-unit: accessed from `src/class_3bb8c_i.c` and `src/BMemPMgr.c`, so
not mine to rename.

## Round 91 polish (delta, track 7)

Comment trimmed: the slot is named by its field (`onNotify`), not its offset.
