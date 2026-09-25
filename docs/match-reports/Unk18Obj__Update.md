# Unk18Obj__Update — MATCHED

> Renamed from `func_8003EEC0` on 2026-09-23 (tools/rename.py). Address 0x8003eec0.

Unit: `code_2cc8c_d`. Round 14, runner delta. 99/99 words, full match (4
real attempts).

## Signature

```c
void Unk18Obj__Update(Unk18Obj *self);
```

`Unk18ObjMethods`'s own `+0x09C` slot occupant (`slot9C`, dispatched by
`Unk18Obj__OnNotifyTag5`, this round).

## What it does

Per-frame update, guarded by `self->unk70` (only runs once
`Unk18Obj__InitOt`'s init has succeeded — that function stalled this round,
see its report, but this one is unaffected). Notifies `slotA0` if
`self->unk10->unkC` is set (passing `self->unk10` itself as the 2nd arg —
see below), updates three sub-objects (`unk40`/`unk4C`/`unk54`),
conditionally re-notifies a PsyQ helper when `unk54` is 1 or 3, resets
`self->unk30`'s pointee, recomputes `self->unk98` from `(unk50-unk4C) /
(1<<unk3C)`, forwards the current `unk74`-indexed slot to two more
helpers, dispatches `slotA0` again with `self->unkAC`, and finally — if
`self->unk10` is set — walks it to its list tail (`Unk18Obj__GetTail`,
already matched this round) and dispatches `slotA0` a third time with that
tail.

```c
void Unk18Obj__Update(Unk18Obj *self) {
    s32 idx;
    Unk18Obj *tail;

    if (self->unk70 == 0) {
        return;
    }

    if (self->unk10->unkC != NULL) {
        ((void (*)(Unk18Obj *, GenericObj *))self->methods->slotA0)(self, self->unk10);
    }

    GsSetProjection((Unk18Obj *)self->unk40);
    GsSetNearClip(self->unk4C);
    func_8003FC70(self->unk54);

    if (self->unk54 == 1 || self->unk54 == 3) {
        u8 *rawBytes = (u8 *)&self->unk5B;
        func_80024AE4(rawBytes[0], rawBytes[1], rawBytes[2]);
        func_8003FD4C(self->unk60, self->unk40);
    }

    func_8003F2AC(&self->unk14);
    *(s32 *)self->unk30 = 0;

    self->unk98 = (u32)(self->unk50 - self->unk4C) / (u32)(1 << self->unk3C) + 1;

    idx = self->unk74;
    GsSetWorkBase(*(s32 *)((u8 *)self + 0x88 + idx * 4));

    idx = self->unk74;
    func_8003FC18(0, 0, *(s32 *)((u8 *)self + 0x78 + idx * 4));

    ((void (*)(Unk18Obj *, void *))self->methods->slotA0)(self, self->unkAC);

    if (self->unk10 != NULL) {
        tail = Unk18Obj__GetTail((Unk18Obj *)self->unk10);
        ((void (*)(Unk18Obj *, Unk18Obj *))self->methods->slotA0)(self, tail);
    }
}
```

## What each real attempt fixed

1. **`slotA0`'s first dispatch needs a 2nd argument, `self->unk10` itself
   (not just `self`).** Retail loads `self->unk10` into `$a1` for the
   `->unkC` truthy check, and that register is STILL `self->unk10` (never
   overwritten) when the `jalr` fires — an implicit "leftover register"
   2nd argument, same family as this project's existing "unused parameter
   shows up in the caller as genuinely uninitialized" idiom, except here
   the leftover value is meaningful, not garbage. Writing the call
   explicitly as `slotA0(self, self->unk10)` reproduces it.
2. **The division must be unsigned.** `self->unk50 - self->unk4C`, divided
   by `1 << self->unk3C`, is a plain C `/` on signed `s32` fields by
   default — which maspsx expands with an extra `INT_MIN`/`-1` overflow
   check (an `li $at,-1` guard) that retail's own `divu` (no such check)
   doesn't have. Casting both operands to `u32` before dividing drops the
   guard and matches.
3. **The division's operands must be computed inline in ONE expression,
   not through an intermediate `s32 divisor = 1 << self->unk3C;`
   variable.** Splitting it out moved the `1 << unk3C` computation (an
   `li $v0,1`) to BEFORE the preceding `*(s32*)self->unk30 = 0;` store,
   where retail computes it only after the subtraction, interleaved with
   it. One inline expression `(self->unk50 - self->unk4C) / (1 <<
   self->unk3C)` reproduces retail's own instruction order.
4. **`self->unk5B`'s three bytes are read UNSIGNED here (`lbu`), despite
   being SIGNED (`lb`) at their own writer, `Unk18Obj__SetFarColor`.** The same
   memory, read from two different call sites, disagrees on signedness —
   not a contradiction, just two independent readings of an opaque field.
   Cast to `u8 *` at this call site rather than retyping `SByte3_d294`
   (which would break the writer's own already-matched codegen).

## Self-caught bug: another forgotten pad, this time from THIS round's own
## earlier work

Adding `unkC` to `GenericObj` (needed for the `self->unk10->unkC` check
above) left a 4-byte gap unaccounted: `unkC` (`void *`, 4 bytes at `+0x00C`)
ends at `+0x010`, but the already-existing `unk14` (round 13) needs
`+0x014` — a forgotten `u8 pad010[0x014-0x010];`. Same failure mode as
`Unk18Obj__OnNotify`'s report earlier this round (compiles clean, `build exit`
non-zero, whole-image SHA1 off by one byte, `cmp -l` + `lsdde.map`
localizes it — this time inside `Unk18Obj__AddChild`, round 13). Third live
instance of this exact bug class within two rounds; see the Proposed
learning below.

## Header changes

`include/code_2cc8c.h`:
- `GenericObj` gains `unkC` (`+0x00C`, `void *`) plus the corrective
  `pad010` gap.
- `Unk18Obj` gains `unk4C`/`unk50` (both `s32`, splitting the old
  `pad04C[0x054-0x04C]`) and `unk98` (`s32`, splitting `pad094`).
- `Unk18ObjMethods` gains `slotA0` — kept as an untyped `void *` and cast
  per call site, since this function alone dispatches it with three
  different arities (`self` alone is never actually used — see fix #1
  above — but `self`+`GenericObj*`, `self`+`Unk18AcObj*`, and
  `self`+`Unk18Obj*` all appear).
- Corrected two earlier comments that conflated "which function OBSERVED
  this slot" with "which function OCCUPIES it" — `slot9C`'s real occupant
  is this function (not `Unk18Obj__OnNotifyTag5`, which only dispatches it), and
  `slotA4`'s real occupant is `Unk18Obj__Flip` (not `Unk18Obj__OnNotifyTag1`, ditto).
- New externs: `GsSetNearClip`/`func_8003FC70`/`func_8003FD4C`/
  `GsSetWorkBase` (all `asm/code_2cc8c_e.s`, next slice, uncarved) and
  `func_80024AE4` (PsyQ, `asm/psyq_GsLinkObject4.s`).

## Proposed learning

**Third instance in two rounds of "insert a new field, forget the leading
pad, break a DIFFERENT already-matched function silently."** (First:
`Class6B5CC__DetachAttachedChildren`/round 13's delta, a different unit entirely. Second:
`Unk18Obj__Finalize`/this round's `Unk18Obj__OnNotify`. Third: this report.) All
three were caught the same way — `build exit` non-zero with no compile
error, localized via `cmp -l` (1-based!) + `lsdde.map`. This is now
clearly not a one-off: **any struct field insertion in this project should
be followed by an immediate arithmetic gap-check** (does the new field's
end offset equal the very next field's start offset, by inspection, not
just by trusting the `[hi - lo]` macro syntax to be self-correcting — it
computes the SPAN correctly but does nothing to catch a missing span
between two named fields).

## Naming

`Unk18Obj__Update` -- tier A. The `slot9C` occupant, dispatched by `Unk18Obj__OnNotifyTag5`; the existing (pre-round-73) report already described it in these exact terms as a "per-frame update" (light mode, fog, ref view, both `GsClearOt` halves, notifying child objects through `slotA0`), independently of this round's naming pass -- a description of MECHANICS, evident from the body, which is what a tier-A name requires.

## Proposed field names

Not applied -- these fields are exclusive to this unit (confirmed: grepped
every other unit that includes `code_2cc8c.h`, none touch `Unk18Obj`), so
by the letter of track 3's rule they COULD be renamed here; left as
proposals instead because their own evidence is materially weaker than the
six fields this round did rename (`lightMode`/`clearColor`/`farColor`/
`fogNear`/`otReady`/`otIndex`, all with an unambiguous single Sony-API
consumer) -- these have no such single clean consumer, several are read at
MULTIPLE sites with different apparent roles, and the project's own
guidance is that a wrong tier-A name is worse than `func_`/`unkNN`. Left for
a future pass (or track 4) with more time to cross-check every call site.

- `unk3C` -> `otLenShift` (tier B): `4 << unk3C` is the OT tag-array byte
  size at both allocation sites in `Unk18Obj__InitOt`, i.e. `unk3C` is
  log2(OT entry count). Also gates `Unk18Obj__SetUnk44`/`Unk18Obj__SetUnk48`
  indirectly via `otReady`, and feeds `Unk18Obj__Update`'s own
  `unk98 = (unk50-unk4C)/(1<<unk3C)+1` -- consistent with "a shift/step
  size", but the exact unit (bytes? OT slots? something else) is not
  proven.
- `unk40` -> not proposed. Overloaded: `Unk18Obj__Update` both passes it as
  a plain value to `SetFogNear`'s 2nd argument AND casts it to
  `(Unk18Obj *)` for `GsSetProjection`. Either it is genuinely two
  different things depending on caller-supplied contents (a raw word the
  caller sometimes puts a pointer in), or one of the two call sites is
  itself worth a second look before naming the field.
- `unk44`/`unk48` -> `otPacketSize`/`otPacketCount` or the reverse (tier C):
  `unk48 * unk44` is the packet-area byte size in `Unk18Obj__InitOt`; which
  operand is "count" and which is "stride" is not distinguishable from a
  commutative multiply alone.
- `unk4C` -> not proposed. Sole use is `GsSetNearClip(self->unk4C)`, an
  UNCARVED callee (still `func_`, no signature evidence beyond "takes one
  word"), so naming the field ahead of that callee would be a pure guess.
- `unk50` -> `otPacketLimit` or similar (tier C): only use is
  `unk98 = (unk50 - unk4C) / (1 << unk3C) + 1` in `Unk18Obj__Update`, i.e.
  a range endpoint paired with `unk4C`. No stronger evidence than that.
- `unk78`/`unk7C` -> `otA`/`otB` (tier B): the two `GsOT` header base
  addresses `Unk18Obj__InitOt` builds and `Unk18Obj__Update`/
  `Unk18Obj__Flip` index by `otIndex`.
- `unk80`/`unk84` -> `otATags`/`otBTags` (tier B): the two tag arrays
  (`base + 0x14`), same pairing.
- `unk88`/`unk8C` -> `otAPackets`/`otBPackets` (tier B): the two packet
  areas (`base + 0x14 + (4 << unk3C)`), same pairing.
- `unk90` -> `notifyCount` (tier B): incremented unconditionally, once per
  call, by `Unk18Obj__OnNotifyTag5`. Never read anywhere in this unit.
- `unk98` -> `otPacketRange` or similar (tier C): `(unk50-unk4C) /
  (1<<unk3C) + 1`, computed each `Unk18Obj__Update` call; never read back
  anywhere in this unit either, so it may be purely an out-parameter for a
  caller this unit does not see.

Posted to the round-73 broadcast for visibility; not applied by this
runner.
