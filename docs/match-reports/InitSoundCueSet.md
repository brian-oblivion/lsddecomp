# InitSoundCueSet -- MATCHED (20/20 words)

> Renamed from `func_8002CC34` on 2026-09-18 (tools/rename.py). Address 0x8002cc34.

Unit: `code_179d8_e`. Runner: echo, round 17.

## Result

```c
s32 InitSoundCueSet(void *unused, SoundCueSet *set, s32 tag, void *owner, s32 callback) {
    SoundCueSlot *slot;
    s32 count;
    s32 sentinel;

    if (set->tag != 0) {
        return 0;
    }
    slot = set->slots;
    sentinel = -1;
    count = 2;
    set->tag = tag;
    set->owner = owner;
    set->callback = callback;
    do {
        slot->index = sentinel;
        count--;
        slot++;
    } while (count >= 0);
    set->unk4 = 0;
    set->unk14 = 10;
    return 1;
}
```

with `SoundCueSlot`/`SoundCueSet` declared at the top of the unit (a small
3-slot init-guarded table, unrelated to `VabStreamObj` -- this function is
NOT a `gVabStreamObjMethods` vtable slot; checked
`classtable.py gVabStreamObjMethods`'s slot list and it's absent). Both
types, and every parameter name here, were renamed round 52 --
`ObjCC34`/`Slot179D8ECC34` are now `SoundCueSet`/`SoundCueSlot`, and the
former `obj`/`arg2`/`arg3`/`arg4` parameters are `set`/`tag`/`owner`/
`callback` (see "Naming" below for the evidence).

Byte-exact, 20/20 words.

## Notes

`set` (the caller's own `this->unk58`/`&this->unk9C` pair, at the one
visible caller in `asm/nonmatchings/Entity/Entity__StartSoundCue.s`) is a
lazily-initialized slot table: guarded by `set->tag == 0`, fills 3
`SoundCueSlot` entries (stride 0x14) with a `-1` sentinel, sets
`set->unk4 = 0` and `set->unk14 = 10`, and stores the three constructor
arguments (`tag`/`owner`/`callback`) into `set->tag`/`owner`/`callback`.
`arg0` (this unit's own first parameter, `unused`) is genuinely dead --
never referenced in the body -- but must stay in the signature: the real
second parameter (`set`) arrives in `$a1`, not `$a0`, at every call site.

Two issues before the match, both from CLAUDE.md's own listed traps:

1. **Return type.** First attempt declared this `void`, following the
   "looks like a pure side-effecting init function" reading. Retail's
   `bnez` (early-exit) delay slot materializes `$v0 = 0`, and the
   fall-through end of the function separately sets `$v0 = 1` right before
   the final `sw`/`jr` -- both values otherwise looking like dead delay-slot
   filler until read as CLAUDE.md's tail-call/wrapper-ambiguity warning in
   reverse: **retail's `s32`-returning function has `return 0;` on the
   already-initialized path and `return 1;` on the did-the-work path**, and
   a `void` reading of the same bytes is indistinguishable from a distance
   but wrong. Declaring it `s32` with those two explicit `return`s turned
   both "stray" constant loads into exactly the instructions retail has, in
   exactly the position retail has them (the early one folds into the
   branch's delay slot, matching `move $v0,zero`; the late one lands right
   before the final store, matching `li $v0,1`).
2. **Delay-slot filler / materialization order.** Once the return type was
   fixed, an ordering residue remained: retail materializes the `-1`
   sentinel constant (`$a0`) BEFORE the loop counter constant `2` (`$v0`);
   my code (evaluating the constant inline inside the loop body as a literal
   `-1`) materialized the loop counter first instead. Introducing an
   explicit `sentinel` local, assigned before `count`, forced the same
   materialization order and closed the gap.

### Proposed learning

A "looks void" init/setter function with a `bnez`/`beqz` early-exit whose
delay slot loads a suspicious constant (especially `0`) is worth checking
for the CLAUDE.md tail-call-ambiguity trap even when there is no tail call
at all: an EARLY-RETURN guard's delay slot and a LATE, seemingly-dead
constant load right before the final store are the two-sided signature of
an `s32` function with `return 0;` / `return 1;` on its two paths, not two
independent scheduling artifacts. Confirmed here as a second, independent
instance of the class `code_179d8_d.c`'s `Table6D940::slot0C`/`slot64`
comment already names generically ("a `void` wrapper around an `s32` tail
call is byte-identical") -- this one has no tail call at all, so the trap
generalises past that specific phrasing.

## Naming

Renamed `func_8002CC34` -> `InitSoundCueSet`, tier B, and its
struct/parameters accordingly (`ObjCC34` -> `SoundCueSet`,
`Slot179D8ECC34` -> `SoundCueSlot`, `arg2`/`arg3`/`arg4` -> `tag`/`owner`/
`callback`). Evidence: this unit's only caller list (`Entity.c`,
`DreamSys.c`, `class_3bb8c_n.c`) shows `arg2` is always a small caller-side
tag value -- concretely `this->moodIndex + 1` in `Entity.c` -- `arg3` is
always the caller's own `this` pointer, and `arg4` is a value indexed out
of (or read directly from) a function-pointer table in every caller
(`this->vt->func_8005A1F4` in `DreamSys.c`; `D_800874B0[sub->unk6]` in
`class_3bb8c_n.c`, and `D_800874B0` is itself a 14-slot class table per
`classtable.py --scan`). So `owner`/`callback` are tier A by mechanics
(store the caller's own context and a function-pointer-shaped value,
verbatim); `InitSoundCueSet` as the whole function's name is tier B: it's
an unambiguous guarded init (`Init`), but the exact in-game trigger for
these sound cues (the caller's own "mood" framing) isn't independently
re-derived from THIS unit, only cross-referenced from the callers'.
`unused` kept as-is (dead in the body); `set->unk4`/`set->unk14` kept
unnamed (set but never read by this unit's own functions, no evidence for
a name beyond "some default/config word").
