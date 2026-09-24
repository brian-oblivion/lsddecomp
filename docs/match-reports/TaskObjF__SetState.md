# TaskObjF__SetState -- MATCH (144/144 words, ~4 attempts)

> Renamed from `Class86E00_3bb8c_g__SetState` on 2026-09-23 (tools/rename.py). Address 0x8004fbe4.

> Renamed from `func_8004FBE4` on 2026-09-23 (tools/rename.py). Address 0x8004fbe4.

Unit `class_3bb8c_g`, class `Class86E00_3bb8c_g`. The largest function in
this round's batch. State-machine driver: normalizes `arg1` to `0x17` if
it equals the current state, fires three unconditional transition calls,
then a dense 5-case `switch` (`jtbl_8001157C`) on the normalized value
that computes a REPLACEMENT state code (several cases resolve it via a
ternary on another call's return), and finally either tears down
`self->unk38`'s array (when the final state lands in `{0x16,0x17}` and
`self->unk24==1`) or just commits `self->unk28 = arg1`.

```c
void TaskObjF__SetState(Class86E00_3bb8c_g *self, s32 arg1)
{
    Class86E00Methods_3bb8c_g *methods = self->methods;
    s32 ret;
    s32 i;

    if (self->unk28 == arg1) {
        arg1 = 0x17;
    }

    methods->slot30(self, arg1);
    methods->slot84(self);
    methods->slot80(self, arg1);

    self->unk5C = 0;
    switch (arg1) {
    case 0x13:
        arg1 = methods->slot50(self) ? 0x11 : 8;
        methods->slot7C(self, arg1);
        break;
    case 0x14:
        if (*(u8 *)self->unk40 == 0) {
            methods->slot58(self, self->unk40, self->unk30, self->unk34);
        }
        ret = methods->slot68(self, self->unk40, self->unk44, self->unk4C,
                              self->unk50, self->unk54, self->unk58);
        arg1 = ret ? 0x16 : 0xC;
        methods->slot7C(self, arg1);
        break;
    case 0x15:
        ret = methods->slot64(self, self->unk40, self->unk54, self->unk58);
        arg1 = ret ? 0x16 : 0x10;
        methods->slot7C(self, arg1);
        break;
    case 0x11:
        methods->slot9C(self);
        break;
    case 0x12:
        methods->slotA8(self);
        break;
    }

    if ((u32)(arg1 - 0x16) < 2) {
        if (self->unk24 == 1 && self->unk38 != NULL) {
            BMemPMgrFree(self->unk3C);
            for (i = 0; i < self->unk2C; i++) {
                BMemPMgrFree(((void **)self->unk38)[i]);
            }
            BMemPMgrFree(self->unk38);
            self->unk38 = NULL;
        }
        self->unk28 = 0;
        self->unk24 = 0;
    } else {
        self->unk28 = arg1;
    }
}
```

## Deriving the shape

`jtbl_8001157C` (`asm/nonmatchings/class_3bb8c_g/TaskObjF__SetState.s`) has 6
entries for the range check `(arg1 - 0x11) unsigned < 5`; the 6th (index 5,
`arg1==0x16`, unreachable given the `< 5` guard) is a dead `0x00000000`
placeholder, matching the same "dense switch, one unreachable padding
slot" shape already seen elsewhere in this project. The 5 live entries
land on only 5 distinct labels this time (no shared groups, unlike
`TaskObjF__AdvanceState`'s table) -- one `case` per label.

`BMemPMgrFree` is already declared GLOBALLY in `include/class_3bb8c.h`
(`extern void *BMemPMgrFree(void *ptr);`, established generic pool
deallocator, `docs/match-reports/BMemPMgrFree.md`) -- no local
declaration needed. Its return is discarded at all four call sites here
(a legal, if unusual, use -- the project's own
`docs/match-reports/TaskObjF__FreeUnusedBuffers.md` documents the OTHER common pattern,
storing the return back into the freed slot; this function is the "just
free it" variant).

The loop bound comparison is `slt` (signed), not `sltu` -- confirms `i`
stays plain `s32`, not the unsigned counter the "sltiu on a loop bound"
lever (this round's head broadcast) would otherwise suggest; that lever
only applies where the COMPARISON itself is unsigned.

Four method slots and two struct fields are new here (first function to
touch them), added ADDITIVELY to `include/class_3bb8c.h`:
- `slot30`/`slot50`/`slot58`/`slot64`/`slot68`/`slot80`/`slot84`/`slot9C`/
  `slotA8` on `Class86E00Methods_3bb8c_g` (`TaskObjF__AdvanceState`, matched
  earlier this round, had already claimed `slot74`/`slot78`/`slot7C`/
  `slot8C` in the same pad ranges -- every new slot here slots into the
  gaps between them without touching a previously-placed one).
- `Class86E00_3bb8c_g::unk2C` (`s32`, `+0x02C`, the teardown loop bound)
  and `::unk34` (`s32`, `+0x034`, `slot58`'s 3rd argument) -- both land
  in single-word pads `TaskObjF__AdvanceState` had already left around its own
  `unk30`/`unk38`/`unk3C`, so no pad resizing was needed, just splitting.

`self->unk40` (read as `*(u8 *)self->unk40` here, and forwarded opaquely
elsewhere) and `self->unk4C` (`u8`, promoted to a full word for `slot68`'s
call, same promotion `slot78` already does with it in `TaskObjF__OnCommand`)
are both ALREADY established fields -- no retyping, just the same
free-reinterpretation-at-the-use-site pattern used throughout this unit
this round.

## The two real residues

**First attempt (before touching this function at all) mis-declared the
Methods struct padding**: an errant `u8 pad0A0[0x0A4-0x0A0];` was placed
BEFORE `slotA0` instead of after it while splitting the pad range for
`slot9C`/`slotA8`, which would have silently pushed `slotA0` (an
ALREADY-established, already-matched slot used by `TaskObjF__OnCommand`) to the
wrong offset. Caught before ever building, by re-reading the edited
struct rather than trusting the diff -- worth flagging as the single
easiest way to introduce a non-local struct-layout break this round,
since `slotA0`'s own call site sits in a DIFFERENT already-matched
function in this same file and a wrong offset there would have shown up
only as an unrelated whole-image SHA1 failure, per the project's own
documented "any struct edit can break an unrelated matched function"
hazard.

**First BUILDING attempt used `self->methods->slotXX(...)` throughout
(no cached local), scoring nowhere near close (drift, `s2`/self->methods
never cached)** -- this function needs the SAME `Class86E00Methods_3bb8c_g
*methods = self->methods;` caching idiom `TaskObjF__AdvanceState` (matched earlier
in this same round) already required, for the identical reason: retail
loads `self->methods` into `$s2` ONCE and every `slotXX` call reads
through that register, where re-reading `self->methods` at each call site
costs an extra `lw`+`nop` per call and cascades into a full register
reshuffle.

**Second residue, isolated after fixing the above:** first working
version listed the `switch` cases in ASCENDING VALUE order
(`0x11, 0x12, 0x13, 0x14, 0x15`) as read off the jump table. Byte-exact on
the FIRST 0x94 bytes (the range check and jump table itself matched, since
jump table entries just point at wherever GCC placed each case's code —
value-to-label mapping isn't affected by block order) but wrong from
there: GCC emits case bodies in the PHYSICAL order they appear in the
SOURCE, and retail physically lays out `0x13`, `0x14`, `0x15` FIRST,
THEN `0x11`, `0x12` last (readable directly off the `.s` file's label
order: `.L8004FC78`(0x13) -> `.L8004FC9C`(0x14) -> `.L8004FD10`(0x15) ->
`.L8004FD3C`(shared `slot7C` tail) -> `.L8004FD54`(0x11) ->
`.L8004FD60`(0x12)). Reordering the `switch` cases in the C source to
match that physical label order (not the numeric case-value order) closed
this to byte-exact.

### Proposed learning

For a JUMP-TABLE switch (unlike the earlier small sequential-compare
`switch` in `TaskObjF__AdvanceState`), the jump table's VALUE-TO-LABEL mapping is
insensitive to case order in source -- but the physical BYTE POSITION of
each case's generated code is NOT; it follows source order. When
reconstructing a dense switch, read the `.s` file's LABEL ORDER (not just
which value maps to which label) and write the `case` clauses in that
same order, even though a value-sorted `case` listing would produce an
identical jump table and looks more natural to write.

## Naming

`TaskObjF__SetState` (was `func_8004FBE4`), tier B. Mechanics
clear (normalizes the requested state, fires the three transition-entry
callbacks, computes a possibly-replaced state code for five specific
requested values, then either commits `self->unk28` or tears the object
down and resets to 0), but which real-world states `0x11`-`0x17` and the
switch's case values name is not established -- the numeric state codes
are kept as literals rather than invented enum names.
