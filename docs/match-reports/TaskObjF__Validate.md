# TaskObjF__Validate

> Renamed from `func_8004F9D8` on 2026-09-20 (tools/rename.py). Address 0x8004f9d8.

**Unit:** title_menu · **Size:** 75 words (0x12C) · **Status:** MATCH

## What it does

`s32 TaskObjF__Validate(TaskObjF *self)`. A validation gate: calls
`self->methods->slot44(self)`, then `self->methods->slot4C(self, &b10,
&b14, &b18)` (three stack-local out-parameters), then
`self->methods->slot48(self)`. If `slot4C`'s return was nonzero AND `b14
== 0` AND `b18 != 0`, returns 1 immediately. Otherwise picks an error code
(`2` if `slot4C` returned 0; `3` if `b10 != 0`; `4` if `b14 != 0`; left
**genuinely uninitialized** if `b18 != 0` — matching retail's own
behaviour exactly, confirmed against the raw disassembly, not a bug in the
derivation; `5` or `6` depending on `self->unk24 == 1`), dispatches
`self->methods->slot7C(self, code)`, and returns 0.

## Levers

1. **The early-return check's two sub-conditions are a short-circuited
   `&&`, not two sequential top-level `if`s.** Writing them as `if
   (slot4CRet != 0) { if (buf14 == 0 && buf18 != 0) return 1; }` (rather
   than three flattened, `goto`-chained `if`s reaching the same place)
   reproduces retail's own doubled, apparently-redundant re-test of
   `slot4CRet` right after the early-return check — retail's second test
   of the SAME value it already branched on moments earlier is not dead
   code, it is what a fresh, separate top-level `if (slot4CRet == 0) code
   = 2; else if (...)` chain naturally recompiles to.
2. **The dispatch tail is a single shared call site reached by `goto`,
   one per error-code computation** (`goto dispatch;` from the `b18 != 0`
   case, natural fallthrough from the `if/else if` chain otherwise) — this
   reproduced retail's own single `.L8004FACC` (one fetch, one `jalr`,
   five predecessors) exactly, with no extra instructions. This is the
   "generic pool of gotos into one call" pattern, and it worked cleanly
   here (contrast `TaskObjF__BeginSave`'s stall, where retail's OWN tail is NOT
   a single shared fetch+call — see that report).
3. **`buf10 != 0` / `buf14 != 0`, not `== 0`.** The first working
   transcription had these inverted (`buf10 == 0 -> code = 3`); this
   compiles to the SAME instruction count and even the SAME later layout,
   but with `beqz`/`bnez` swapped on two branches — a pure sense/polarity
   bug in the initial reading of the disassembly, not a compiler
   scheduling residue. Caught by funcdiff showing only those two words
   differing on an otherwise byte-identical body.

See `TaskObjF__ForEachEvent`'s report for the `TaskObjF` class this function's
`self` belongs to.

## Naming (round 60, track 3)

`func_8004F9D8` -> `TaskObjF__Validate`. **Tier B.** Its own report
already calls it "a validation gate": calls three vtable slots
(`slot44`/`slot4C`/`slot48`), decides an ok/error `statusCode`, and
dispatches through `slot7C` on failure. Mechanics are clear; what the
three slots' own implementations actually validate (owned by a subclass
outside this unit) is not established here, so the name describes the
gate itself rather than what it checks.

## Round 95 (track 7, charlie)

### Naming and constants

`slot4CRet`/`buf10`/`buf14`/`buf18` -> `ok`/`error`/`cardChanged`/
`formatted`, from what TaskObjF__CheckCardStatus writes (title_menu.c):
its return is 0 when a card call timed out or failed (EvSpTIMOUT, EvSpERROR), the first out is
set on EvSpERROR, the second when _card_info reports EvSpNEW (a new card),
the third is cleared when _card_load reports EvSpNEW (unformatted). `code`
-> `state`: 2 `NO_CARD` ("NOCONECT"), 3 `CARD_ERROR`, 4 `CARD_CHANGED`,
5/6 `UNFORMATTED_LOAD`/`UNFORMATTED_SAVE` ("UNFORM1"/"UNFORM2", chosen by
opMode). The `formatted != 0` branch is unreachable (the early return took
that case) and leaves `state` unset; it stays, with a MATCHING line, as the
goto that lever 2 above measured. typeviews' baseline warning for it now
names `state`. Zero bytes.
