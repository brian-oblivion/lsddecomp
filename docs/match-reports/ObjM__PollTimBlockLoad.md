# ObjM__PollTimBlockLoad

> Renamed from `ObjM__TransferToOther` on 2026-09-26 (tools/rename.py). Address 0x800531cc.

> Renamed from `func_800531CC` on 2026-09-24 (tools/rename.py). Address 0x800531cc.

**Unit:** class_3bb8c_l · **Size:** 99 words (0x18C bytes) ·
**Status: MATCHED 99/99**, whole-image SHA1 green. One of the two "large"
functions in this unit's queue.

## What it does

`self`'s "detach from current target" / "link to a new target" logic —
takes `other`, an object of the SAME class (`Obj87034_3bb8c_l *`, confirmed
by the identical field offsets `self` and `other` are both read through:
`0x0`, `0x3C`, `0x50`, `0x60`, `0x68`, `0x80`).

```c
void ObjM__PollTimBlockLoad(Obj87034_3bb8c_l *self, Obj87034_3bb8c_l *other) {
    s32 ret;
    s32 sel;
    void *a1;
    Obj87034Methods_3bb8c_l *m;

    if (self->unk60 != 0) {
        if (other->unk80 != 0) {
            other->methods->slot04(other);
            self->unk60 = 0;
            self->methods->slot80(self);
            ret = self->unk3C->methods->slot108(self->unk3C);
            self->unk3C->methods->slot104(self->unk3C, ret + 0x1E);
        } else if (other->unk3C != 0) {
            sel = self->unk50->unk14;
            m = other->methods;
            if (sel != 2) {
                a1 = self->unk50->unk18;
            } else {
                a1 = self->unk50->unkC;
            }
            m->slot7C(other, a1);
            other->methods->slot04(other);
            self->unk60 = 0;
            self->methods->slot80(self);
        }
    }
    if (self->unk60 == 0) {
        if (self->unk14->unk1B4 == 0 && self->unk68 == 0) {
            self->unk64 = 1;
            self->methods->slot88(self);
        }
    }
}
```

`self->unk3C` (and `self->unk18`, established by a sibling function) is
almost certainly `DreamSys *` — see the header comment on
`DreamSysObj_3bb8c_l`/`DreamSysMethods_3bb8c_l` in `include/class_3bb8c.h`
for the full cross-check against `tools/classtable.py 0x80087BDC`
(`gDreamSysMethods`, `include/DreamSys.h`). Declared as this unit's own
independent minimal view rather than editing `DreamSys.h`, since none of
that header's own named fields cover the offsets this unit reaches
(`+0x050`, `+0x074`, `+0x0FC`, `+0x104`, `+0x108`, `+0x200` all fall inside
its `unknown_functions_0x..` padding arrays there).

## Residues fixed during matching (both genuine, not toolchain)

1. **`StyleConfig::unk14` padding bug.** First draft put `unk14`
   immediately after `unkC` (no gap), landing it at offset 0x10 instead of
   0x14 — one word of missing `u8 pad[...]`. Visible as a wrong field
   offset AND a register-role swap in the diff (retail: `v0`=field value,
   `v1`=literal 2; mine: reversed), both symptoms of the same root cause.
2. **Branch polarity / body-placement, same class as `ObjM__GetGridRecord`'s
   residue.** The natural `if (sel == 2) { a1 = unkC; } else { a1 = unk18;
   }` compiles with the wrong body at the wrong branch target. Writing the
   negated form `if (sel != 2) { a1 = unk18; } else { a1 = unkC; }`
   reproduces retail's `beq`/target layout exactly — see
   `ObjM__GetGridRecord`'s report for the general rule.
3. **A load hoisted out of the `if`/`else`, not visible from the
   disassembly's literal instruction order until diffed against a body
   that DOESN'T hoist it.** Retail loads `other->methods` into a register
   BEFORE testing `sel == 2` (i.e. unconditionally, regardless of which
   branch is taken) rather than at the `slot7C` call site. The idiomatic
   fix is a source-level temp (`m = other->methods;`) placed between the
   `sel` computation and the `if`, matching retail's own instruction
   order — not a scheduling barrier, a real hoist that must be visible in
   the C.

### Proposed learning

When a residue in an `if`/`else` selecting one of two field reads shows a
call site re-deriving a base pointer that was ALREADY available from an
earlier computation, check whether retail computed that base pointer
EAGERLY (before the branch) rather than lazily (at first use) — the fix is
a plain local variable assigned before the `if`, not a barrier or an
`asm("")`.

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_800531CC` | `ObjM__PollTimBlockLoad` | B | see below |

**Evidence.** Private helper (not itself a vtable slot), called only from `ObjM__OnTag1Notify`. The detach-from-current-target / link-to-new-target logic: `other` is confirmed to be the SAME class (`Obj87034_3bb8c_l`) by the identical field offsets both `self` and `other` are read through (`+0x0`, `+0x3C`, `+0x50`, `+0x60`, `+0x68`, `+0x80`).


## Track 4 (2026-09-26, round 89, echo)

Renamed from `ObjM__TransferToOther` (rename.py). **The reading above that `other` is an ObjM is wrong**, and the callers show it (track 4 step 5): its only caller passes `self->timBlockSrc` (+0x058), which `ObjM__InitStyleAndWorld` assigns from `New_TimBlockSrc(PickDailyVariant(...))`. The offsets read on it are TimBlockSrc's (include/TimBlockSrc.h): +0x080 `failed`, +0x03C `loaded`, +0x004 `release`, +0x07C `fadeAllEntries(color)`. So: while `timBlockPending` (+0x060), a failed load releases the source, runs setupSceneStyle and adds 0x1E to the DreamSys's dream time limit; a finished load fades the CLUT rows to the styleConfig colour (+0x00C when styleConfig +0x014 is 2, else +0x018), releases it and runs setupSceneStyle. Then, with nothing pending, the StageMap idle (`unk1B4 == 0`) and not yet `inSession`, it sets `unk64` and runs enterStyleSession. Called once per onTag1Notify event 2, so a poll. Tier B. Signature now `(ObjM *self, TimBlockSrc *src)`; byte-identical.

## Round 94 (track 6, charlie)

`Unk50Struct_3bb8c_l` is `StyleConfig` (ObjM::styleConfig's pointee,
include/class_3bb8c.h), its fields named from their readers: `unk0`/`unk4`
-> `lightDirs`/`lightColors` (the StageMap's setChildParams), `unk8` ->
`ambientColor` (setAmbientColor), `unkC` -> `clearColor` (the viewport's
setClearColor), `unk14` -> `colorMode` (1: the far colour is clearColor; 2:
the TimBlockSrc fades to clearColor), `unk18` -> `farColor` (setFarColor
otherwise), `unk1C` -> `fogNear` (setFogNear). Tier B for `colorMode`, whose
two tested values are read by different methods for different choices.
Zero bytes changed.

## Round 95 (track 7, echo)

Locals `ret` -> `timer`, `sel` -> `colorMode`; the failure path's `0x1E`
is 30 (seconds, getSetDreamTimeLimit's unit per DreamSys.h's
DREAM_TICKS_PER_SECOND). colorMode's values (1, 2) stay literals:
StyleConfig belongs to include/class_3bb8c.h, proposed there as an enum.
