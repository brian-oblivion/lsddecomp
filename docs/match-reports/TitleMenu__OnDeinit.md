# TitleMenu__OnDeinit -- MATCH

> Renamed from `Class86B60__OnDeinit` on 2026-09-26 (tools/rename.py). Address 0x8004d898.

> Renamed from `TitleMenu__RegisterHandlers` on 2026-09-26 (tools/rename.py). Address 0x8004d898.

> Renamed from `func_8004D898` on 2026-09-24 (tools/rename.py). Address 0x8004d898.

Unit `class_3bb8c_d`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TitleMenu__OnDeinit`: 29/29 words match.

## Source

```c
void TitleMenu__OnDeinit(TitleMenu *self)
{
    u32 i;
    u8 *entry;

    i = 0;
    entry = (u8 *)&D_80086DAC;
    for (; i < 2; i++) {
        self->handlerTable->unk0->methods->slot78(self->handlerTable->unk0, &self->unk93, entry);
        entry += 0xC;
    }
}
```

## Derivation and two near-misses

A fixed 2-iteration loop, dispatching through `self->handlerTable->unk0`'s own
vtable and walking an external table (`D_80086DAC`) with an explicit
0xC-byte stride, passing each entry's address (never dereferencing it in
this function).

- **First attempt (21/29):** declared `entry` before `i` and initialized it
  first (`entry = ...; for (i = 0; ...)`). Retail's prologue sets up the
  loop counter (`$s1 = 0`) BEFORE the table pointer (`$s0 = &D_80086DAC`),
  and register allocation followed source order — swapping the two
  initialization statements to `i = 0;` then `entry = ...;` fixed all but
  one word.
- **Second attempt (28/29):** the one remaining residue was `slti` where
  retail has `sltiu` on the loop-bound compare (`$s1 < 2`) — the already-
  documented "an unsigned range check needs an explicit cast/type to reach
  `sltiu`" idiom (round 13). Retyping the counter `s32 i` to `u32 i` closed
  it to byte-exact with no other change.

## Struct changes (additive, `include/class_3bb8c.h`)

- New types `TitleMenuUnkCObj_3bb8c_d` (self->handlerTable's pointee: `unk0` a
  vtable pointer, `unk4` an opaque value used by `TitleMenu__EndCardAccess`) and
  `TitleMenuUnkC0Obj_3bb8c_d` / `TitleMenuUnkC0ObjMethods_3bb8c_d` (the
  vtable `unk0` points to, `slot78` the only reached slot).
- `TitleMenu::handlerTable` -- new field, `TitleMenuUnkCObj_3bb8c_d *`, carved
  from the `pad004` gap.
- `TitleMenu::unk93` -- new field, `u8`, address-of only, carved from the
  `pad04C` gap (between `unk48` and `unkA4`).
- New `extern s32 D_80086DAC;` (address-of only, placeholder type, walked
  with an 0xC-byte stride but never dereferenced by this function).

### Proposed learning

None new -- both residues were already-documented idioms (loop-counter
init statement order, and the unsigned-compare-needs-a-real-unsigned-type
rule from round 13). Filed here as a third confirming instance of each
rather than as new bullets.

## Naming (round 77, naming runner delta)

Renamed `func_8004D898` -> `TitleMenu__OnDeinit`. **Tier B**: Walks a fixed 2-entry external table (`D_80086DAC`, 0xC-byte stride), dispatching each entry's address through `self->handlerTable->unk0`'s own vtable slot alongside `&self->unk93`. Named from the mechanics only (registering fixed table entries with a sub-object); no purpose established for what is being registered.

## Track 4 (2026-09-26, round 88, bravo)

Renamed `TitleMenu__RegisterHandlers` -> `TitleMenu__OnDeinit` (tools/rename.py):
it is the occupant of gTitleMenuMethods +0x050, IntermediateBase's
`onDeinit` slot (IntermediateBase__Deinit's first call), overriding
TaskCore__OnDeinit without an up-call. TaskCore__OnDeinit's own last step is
the same call, `initArgs->unk0`'s +0x078 with `unk93` (and 0); this one makes
it twice with the two 0xC-byte entries of D_80086DAC as the third argument.
`handlerTable` was TaskCore's `initArgs` (IntermediateBaseInitArgs, +0x00C)
all along; `&self->unk93` is TaskCore's `u8 unk93[3]`.
