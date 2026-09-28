# SetDreamAuxWorld

> Renamed from `func_8005C650` on 2026-09-21 (tools/rename.py). Address 0x8005c650.

**Unit:** code_4cd08 · **Size:** 34 words · **Status:** MATCHED round 43
(34/34, byte-exact whole-image build).

## History

Filed BLOCKED in round 2026-08-30-a on 6 `%gp_rel` references (the first to
`gDreamAuxStage`). Round 42 resolved the gp-relative blocker with
`--gp-symbols`/`--no-nop-mflo-mfhi` (`docs/research/gp-relative-blocker.md`,
"RESOLVED"). This function was never actually attempted under the old
toolchain -- the round-42 stub carried no derivation to rebuild, just the
blocker classification. Round 43 derived and matched it fresh.

## What it does

The initializer for this unit's five `%gp_rel` globals plus a one-shot
"spawn an Entity per DreamAux slot" loop:

```c
extern void *New_Entity(void *arg0, void *arg1, void *arg2);
extern s32 gDreamAuxStage;
extern s32 gDreamAuxStageMap;
extern s32 gDreamAuxWorld;
extern s32 gDreamAuxSound;
extern s32 gDreamAuxFrameClock;

void SetTeleportsEnabled(s32 triggerType);

void SetDreamAuxWorld(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4)
{
    DreamAuxSlot *slot = gDreamAuxSlots;
    u32 i;

    gDreamAuxStage = a0;
    gDreamAuxStageMap = a1;
    gDreamAuxWorld = a2;
    gDreamAuxSound = a3;
    gDreamAuxFrameClock = a4;

    for (i = 0; i < 1; i++) {
        s32 buf[4];
        buf[3] = (s32)slot->obj;
        slot->entity = New_Entity((void *)(i + 0x62), buf, (void *)gDreamAuxSound);
        slot++;
    }
    SetTeleportsEnabled(a0);
}
```

Two derivation points worth recording:

- **The `for (i = 0; i < 1; i++)` idiom needs an UNSIGNED loop variable to
  reproduce retail's `beqz` backward-branch.** m2c's decompilation (with
  `--sig`) reproduced the body exactly but left the loop counter type
  ambiguous; a first pass with `s32 i` built clean and scored 33/34, one
  word off at file offset `0x4CEAC`/vram `0x8005C6AC` -- retail is
  `beqz $s0, .L8005C68C` (`f7ff0012`), the `s32` build emitted
  `blez $s0, ...` (`f7ff001a`). GCC 2.6.3 lowers a *signed* `for (i=0;i<1;i++)`
  to a `<=0` backward test (since it cannot assume `i` never goes negative)
  but a matched sibling in this same unit, `ReleaseDreamAuxModels`
  (`for (done = 0; done < 1; done++)` with `u32 done`), already demonstrated
  the `beqz` form for the identical "run once" shape. Switching `i` to `u32`
  reproduced `beqz` and closed the last word. **This unit now has two
  confirmed data points for the "run this loop once" idiom, and both need
  the counter unsigned to match.**
- **The struct field this function writes did not exist yet.** `DreamAuxSlot`
  (`include/code_4cd08.h`) previously had `void *obj; u8 unk4[0x10];`. This
  function's `sw $v0, 0x4($s1)` after the `New_Entity` call writes a pointer
  at the slot's offset 0x4, so `unk4` is now split into a named `void *entity`
  plus the remaining `u8 unkC[0xC]` (unread by anything in this unit still).
  Total size is unchanged (0x14), so this is a pure rename/split, not a
  layout change -- confirmed safe by the whole-image build immediately after
  editing the header (no other function in this unit or elsewhere references
  `DreamAuxSlot` at an offset past 0x4 yet).

The `New_Entity` call's second argument is a 4-word (0x10-byte) local stack
buffer; only its last word (offset 0xC, i.e. `buf[3]`) is written before the
call, with the current slot's pre-existing `obj` field. Nothing in this
function reads any other word of the buffer, and no field of it beyond that
one word is otherwise constrained -- the buffer exists here purely to give
`New_Entity`'s `arg1` a valid stack address at the frame layout retail uses
(confirmed by the frame size: `addiu sp, sp, -0x30`, matched exactly with 4
saved registers + this one 4-word local + no other locals).

`New_Entity`'s first parameter is declared `void *` in `include/Entity.h`
(`src/Entity.c`), but every call so far (including this one) passes what is
clearly an integer id (`i + 0x62`, i.e. `moodIndex` per `Entity__Entity`'s
own reading of that argument). The cast to `(void *)` here is cosmetic --
GCC 2.6.3 does not care about the mismatch for either codegen or scoring, and
`New_Entity`'s own signature is out of scope for this unit to change. Its
prototype and the five new `%gp_rel` globals are declared locally in
`code_4cd08.c` (not in `code_4cd08.h`), per the shared-header rule: `Entity.c`
owns `New_Entity`, this unit only calls it.

## Proposed learning

Add to the corpus: **an unsigned loop counter is required for the
`for (i = 0; i < 1; i++)` "run once" idiom to reproduce retail's `beqz`
backward branch** -- a signed counter of the same shape compiles to `blez`
instead, one word different, easy to miss since both are logically correct.
Two independent instances now confirm it in this unit alone
(`ReleaseDreamAuxModels`, `SetDreamAuxWorld`).

## Naming

**SetDreamAuxWorld** — tier B. Installs its five parameters into the unit's
shared context globals (`gDreamAuxStage`, `gDreamAuxWorld` and three still-
unnamed siblings), spawns one entity into `gDreamAuxSlots[0].entity` via
`New_Entity`, then calls `SetTeleportsEnabled`. Called from
`ObjM__SetupSceneStyle` (`class_3bb8c_k.c`), itself a per-object/per-level setup
routine. "World" reflects `gDreamAuxWorld`'s own established role (cast
`TriggerWorld*`, dispatched through vtable slots 0x80/0x22 elsewhere in the
unit) -- but this function's OWN purpose (why these five values, together,
constitute entering a "world") is inferred from usage, not proven, hence B.

## Track 4 (2026-09-26, round 88, echo)

`DreamAuxSlot::entity` holds New_Entity's result and is now `struct Entity *`; the call passes the mood index without a `(void *)` cast.

Byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 88)

The third parameter is retyped `s32 a2` -> `DreamSys *world`, and the global
it is stored in, gDreamAuxWorld, `s32` -> `DreamSys *` (code_4cd08.c). Its
only caller, ObjM__SetupSceneStyle (class_3bb8c_k), passes its DreamSys
`target`; code_4cd08 calls +0x200 of its table (getDreamColor) and passes
it as Entity's peer. Byte-identical.

## Round 100 (alpha): track 7, moved from src/code_4cd08.c and include/code_4cd08.h

Parameters named from the caller (ObjM__SetupSceneStyle, class_3bb8c_k.c,
whose own declaration already says `stage, grid, world, sound, clock`, and
ObjM.h: unk14 the StageMap, unk10 the FrameClock, TimedTask's sound the
VabStreamObj): a0..a4 -> stage, stageMap, world, sound, frameClock. The
globals it installs were renamed with tools/rename.py: D_8008ABFC ->
gDreamAuxStageMap (tier A), D_8008AC04 -> gDreamAuxSound (tier A, New_Entity's
`sound`), D_8008AC08 -> gDreamAuxFrameClock (tier A, TodActor's attachToParent
`companion`), and typed `struct VabStreamObj *` / `struct FrameClock *`. The
entity's mood row is DREAM_AUX_FIRST_MOOD + i (98); `buf` -> `desc`
(New_Entity's descriptor, ModelData in word +0x00C). Byte-identical.

The extern comments, as they stood:

```c
extern StageMap *D_8008ABFC; /* the grid manager: SetDreamAuxWorld's a1; Entity__AttachToParent keeps it as the entity's grid */
extern DreamSys *gDreamAuxWorld; /* the player DreamSys: class_3bb8c_k passes its `target` */
```
