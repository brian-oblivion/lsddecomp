# TaskCore__Reset

> Renamed from `TaskCoreObj__Reset` on 2026-09-25 (tools/rename.py). Address 0x8003c11c.

> Renamed from `func_8003C11C` on 2026-09-23 (tools/rename.py). Address 0x8003c11c.

**Unit:** Task · **Size:** 48 words · **Status:** MATCHED (48/48)

## Summary

An "init defaults" function: four forwarding calls through `self->methods`,
then eight literal field stores.

```c
void TaskCore__Reset(StreamTaskObj *self) {
    StreamTaskObjMethods *methods = self->methods;
    methods->slot6C(self, -1);
    methods->slotA4(self, &sTaskCoreDefaultColors[0], &sTaskCoreDefaultColors[3], &sTaskCoreDefaultColors[6]);
    methods->slot9C(self, 1);
    methods->slotA0(self, 1);
    self->unk84 = 9;
    self->unk28 = 3;
    self->unk2C = 0x12C;
    self->unk30 = 0x40;
    self->unk9C = 0;
    self->unkA0 = 0;
    self->unk34 = 1;
    self->unk3C = 0;
}
```

## Evidence

- `self->methods` (`StreamTaskObjMethods*`) is dereferenced four times across
  four `jalr`s. Reused this round's now-familiar tell: writing
  `self->methods->slotXX(...)` inline four times gave a **one-word-short**
  build (47/48, and everything downstream shifted, tripping funcdiff's
  "differs outside range" warning). A local `StreamTaskObjMethods *methods =
  self->methods;` fixed it in one step — same root cause as
  `TaskCore__OnDeinit`'s `TaskCoreObj *obj` local this round: a value read once
  and used again after an intervening indirect call needs to be pinned in a
  local, or GCC reloads it from memory instead of keeping it live in a
  register, changing the instruction count.
- Three new `StreamTaskObjMethods` slots, all void-returning (discarded
  results), confirmed to exist via `tools/classtable.py gStreamTaskMethods`:
  `+0x09C = TaskCore__SetFadeCallbackEnabled`, `+0x0A0 = TaskCore__SetFadeOutCallbackEnabled`,
  `+0x0A4 = TaskCore__SetColors` (all extern, other units).
- `slotA4`'s three pointer arguments are `&sTaskCoreDefaultColors[0]`, `&sTaskCoreDefaultColors[3]`,
  `&sTaskCoreDefaultColors[6]` — a `lui`/`addiu` to the symbol with offsets added, never
  dereferenced here, so `sTaskCoreDefaultColors` is declared as a plain `extern u8[]`
  with unknown real element shape.
- Eight new `s32` fields carved out of what was padding: `unk28`, `unk2C`,
  `unk30`, `unk3C`, `unk84`, `unk9C`, `unkA0`, plus `unk34` (a field already
  named from `TaskCore__OnDeinit` in this round, here just given its default
  value `1`, confirming the two functions' guesses agree).

## Proposed learning

**Confirmed for a second time this round, now with a 4-call fan-out instead
of 2:** any field read through an unknown indirect call chain (`self->methods
-> ... -> jalr`) that is used again after ANY intervening `jalr` needs an
explicit local, full stop — not just "when it looks expensive to
re-fetch". The number of calls it must survive doesn't change the fix, only
how many words are at stake if it's missed (2 calls cost 44 words in
`TaskCore__OnDeinit`; here, 4 calls cost 1 word and shifted the whole rest of the
build). Suspect this reflexively whenever a `self->field` (or
`self->a->field`) appears more than once with a `jalr` between the
occurrences.

## Naming

**TaskCoreObj__Reset** -- tier A. Occupies `gTaskCoreMethods` slot `+0x040`,
the same cross-class "Reset" slot number as `StreamTask__Reset`
(`StageMap__Reset`/`SceneNode__Reset` precedent); sets eight fields to
fixed literal defaults, the same shape as every other confirmed `Reset` in
this codebase.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from TaskCoreObj__Reset (tools/rename.py): the class prefix. Occupant of +0x040 (IntermediateBase's `resetCounters`, the ctor's last call). Keeps "Reset" rather than the slot's name: it sets eight defaults and makes four slot calls and does not up-call. The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (2026-09-27, round 99, alpha)

- `D_8006E860` -> `sTaskCoreDefaultColors` (tier A): nine bytes of .data,
  three RGB triples {0, 0, 0}, {0, 0, 0}, {128, 128, 128}, passed to setColors
  as base, clear and the third colour; only this unit reaches it.
- `unk28` -> `otLength` (tier A): onInit hands it to the viewport's
  setOtLength.
- `unk30` -> `packetSize` (tier B): onInit hands it to the viewport's
  `setUnk48`, and Viewport__InitOt sizes each buffer's packet area as
  `unk44 * unk48`. Every writer of the `unk48` side passes 64 (Viewport's ctor,
  this function), while the `unk44` side varies with the screen (2000
  Viewport's default, 1200 DreamSys, 300 here, 400 TitleMenu and GraphRoom),
  which reads as a packet count times a fixed packet size. The Viewport banner
  leaves which is which open; this name is the reading above, not a
  measurement.
- `unk2C` (-> `packetCount`) and `unk34` (-> `clearOnDeinit`) have accessors
  in class_3bb8c_d.c / ObjMStyleActor.c: proposed, not renamed.

### History moved from include/code_2c054.h

The colour table was first declared as "a rodata table reached only by
ADDRESS (`lui`/`addiu`, no `lw`/`sw` here), passed as three pointers 3 bytes
apart, never decoded further, so typed as a plain byte array". It is in
.data (asm/data/5E140.data.s), not rodata.
