# BaseObjO__func_5748c -- MATCHED (14/14 words)

> Renamed from `func_8005748C` on 2026-09-18 (tools/rename.py). Address 0x8005748c.

Unit: `class_3bb8c_o` (round 17). A shared `BasicClass`-inherited slot
occupant (`slotC4`), already independently confirmed `void` from BOTH
`Entity.h` and `code_55dd4.h`'s `Class65650Methods::slotC4` (both tables
hold this exact function at `+0xC4`, per `Entity.h`'s own comment). Tail-
calls `DreamSys__ApplyOffsetSlotAndNotify` (still `INCLUDE_ASM`, sibling unit
`class_3bb8c_p`) with a fixed global address and a literal `6`.

## Final source

```c
extern s32 D_8008ABA8;
extern void DreamSys__ApplyOffsetSlotAndNotify(BaseObjO *self, void *arg0, s32 arg1, s32 arg2, s32 arg3);

void BaseObjO__func_5748c(BaseObjO *self, s32 arg1, s32 arg2) {
    DreamSys__ApplyOffsetSlotAndNotify(self, &D_8008ABA8, arg1, arg2, 6);
}
```

## Derivation

Register setup before the `jal`: `$a0` untouched (still `self`), `$a1` =
`&D_8008ABA8` (freshly computed, overwriting the incoming `arg1`'s old
register), `$a2` = the ORIGINAL `arg1` (saved into `$a2` before `$a1` is
overwritten), `$a3` = the original `arg2`, and one stack word (`$sp+0x10`)
= the literal `6`. This is a genuine 5-argument call (4 registers + 1
stack slot, o32 ABI), matching `DreamSys__ApplyOffsetSlotAndNotify(self, &D_8008ABA8, arg1,
arg2, 6)` written left-to-right in C.

**Kept `void`, a bare statement call rather than `return DreamSys__ApplyOffsetSlotAndNotify(...)`
-- per CLAUDE.md's own explicit warning about this exact slot.**
`Entity.h`'s comment on `EntityMethods::slotC4` documents that retyping
this SHARED slot to a non-`void` return breaks an ALREADY-MATCHED sibling
function (`func_8005E160`, which relies on GCC tail-merging two identical
`void`-typed `slotC4(this,0x32,0)` call sites reached from different
branches -- a non-`void` return stops the merge and costs that function 4
words). `DreamSys__ApplyOffsetSlotAndNotify` itself is declared `void` here purely as a local
call-site typing choice consistent with that constraint; its own real
return type (if any) is unconfirmed and irrelevant to this call site, which
discards it either way.

`D_8008ABA8` is declared as an arbitrary scalar (`extern s32 D_8008ABA8;`)
since only its address is ever taken here, never its value.

### Proposed learning

None -- a direct application of the already-documented `slotC4`-must-stay-
`void` constraint to a NEW occupant of the same shared slot, confirming it
generalizes past the two instances (`func_8005FA64`, `func_8005E160`)
`Entity.h` already names.

## Naming

**`BaseObjO__func_5748c` -- tier C.** Class is known (occupies the shared
`slotC4` `BasicClass`-inherited slot, confirmed by `tools/classtable.py`
against `Entity.h`/`code_55dd4.h`'s independent readings of the same
address), but the function tail-calls a still-`INCLUDE_ASM` sibling-unit
function (`DreamSys__ApplyOffsetSlotAndNotify`, `class_3bb8c_p.c`) with a fixed global address
and a literal mode value `6`, and no occupant of `slotC4` anywhere in the
codebase has an established purpose either (`Entity.h`'s own comment on
this exact slot only documents a MUST-STAY-`void` return-type constraint,
not what the slot means). Kept the tier-C `Class__func_xxxxx` form.
