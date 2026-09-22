# Class866E8__Class866E8 -- MATCH (163/163 words, ~7 real rebuild attempts)

> Renamed from `func_8004A534` on 2026-09-22 (tools/rename.py). Address 0x8004a534.

Unit `class_3ac78`, 177-line body, `Class866E8Methods::ctor` (the occupant
of the ctor slot, dispatched by `New_Class866E8`'s `New_Class866E8`
allocator). 8 distinct callee-saved registers (`$s0`-`$s7`, fully
saturated) -- flagged by the head as being in the band that was 0
matched / 4 stalled across 81 pooled samples from rounds 13-14, sent
anyway on the reasoning that the screen deprioritizes rather than forbids
and no report existed on an instance in this unit. **It matched.** See
"On the register-count screen" below.

```c
typedef struct BaseCtorTable_3ac78 BaseCtorTable_3ac78;
struct BaseCtorTable_3ac78 {
    u8 pad0[0x8];
    void (*ctor)(void *self); /* +0x008, standard "further-base ctor first" slot */
};

extern BaseCtorTable_3ac78 *func_800428E4(void);
extern UnkSlotChildObj_3ac78 *func_80048894(void);
extern UnkSlotListObj_3ac78 *new_class_6d940(s32 arg1);
extern GenericObject *func_8004D38C(void);
extern s32 func_80020C5C(void);
extern Vec3_3ac78 D_8008682C;

void Class866E8__Class866E8(Class866E8 *self, Vec3_3ac78 *arg1, s32 arg2)
{
    s32 i;
    UnkSlotEntry_3ac78 *entry;
    GenericObject *obj;
    Class866E8 **cellp;
    u8 *p;
    u8 *end;
    s32 buf[3];

    func_800428E4()->ctor(self);
    self->methods = func_8004D244();

    if (arg1 != NULL) {
        self->unk54 = *arg1;
    } else {
        self->unk54 = D_8008682C;
    }

    self->unk1B0 = 0;
    self->unk1B4 = 0;
    self->unk1B8 = 0;
    self->unk70 = 0;
    self->unk6C = 0;
    self->unkE8 = 0;
    self->unk1E0 = 0;

    for (i = 0; i < 7; i++) {
        entry = &self->unkEC[i];

        entry->unk4 = func_80048894();
        entry->unk4->unk20 = (entry->unk4->unk10 != 0);
        entry->unk4->unk32 = i;
        entry->unk4->methods->slot88(entry->unk4, arg2);

        entry->unk14 = 0;
        entry->unk18 = 0;
        entry->unk2 = i;
        entry->unk0 = 0;

        entry->unk8 = new_class_6d940(0);
        entry->unkC = func_8004D38C();
        entry->unkC->methods->slot4C(entry->unkC, self, &self->unk54);

        entry->unk10 = (Class866E8 **)func_80017B34(0x668);
        if (entry->unk10 == NULL) {
            return;
        }

        buf[0] = 0x400;
        buf[1] = 0;
        buf[2] = 0x400;

        cellp = entry->unk10;
        end = (u8 *)cellp + 0x668;
        p = (u8 *)cellp;
        while (p < end) {
            obj = func_8004D38C();
            *(GenericObject **)p = obj;
            obj->methods->slot4C(obj, entry->unkC, buf);

            buf[0] += 0x800;
            if (buf[0] > 0xA400) {
                buf[0] = 0x400;
                buf[2] += 0x800;
            }

            obj = *(GenericObject **)p;
            obj->methods->slot70(obj, 1);
            obj = *(GenericObject **)p;
            p += 4;
            obj->unk10 |= 0x80000000;
        }
    }

    self->methods->slot10(self, func_80020C5C());
    self->methods->slot40(self);
}
```

## What it does

Class866E8's own ctor. Calls a further-base ctor (`func_800428E4()->ctor(self)`,
the standard "base ctor first, then set own vtable pointer" idiom already
established elsewhere in this project), sets `self->methods`, copies a
3-word block into `self->unk54` (from `arg1` if given, else a default
global), zeroes several scalar fields, then fills `self->unkEC[0..6]`
(7 slot entries): each gets a child object (`func_80048894`), a list object
(`new_class_6d940`), a generic object (`func_8004D38C`) dispatched with the
just-copied `self->unk54` block, and a freshly-allocated 0x668-byte buffer
of pointers -- each pointer itself a `func_8004D38C()`-created object,
initialized via a rect-packing-style budget (`buf[0]`/`buf[2]`, wrapping
at `0xA400` back to `0x400`, stepping `0x800` per cell) and flagged with a
high bit on `unk10` after a `slot70(obj, 1)` dispatch. Finishes with two
more dispatches on `self->methods` (`slot10` with a fresh `func_80020C5C()`
value, then `slot40`, already known gp_rel-blocked as a CALLEE -- irrelevant
here since this function only DISPATCHES to it, never inlines its body).

## New struct ground opened (all additive; class_3ac78.h is unique to this unit)

- `Class866E8Methods::slot10` (+0x010, replacing a 4-byte pad) -- the ctor's
  own dispatch, `(self, s32 arg1)`.
- `Class866E8::unk54` (`Vec3_3ac78`, a new 3-word type, +0x054..+0x05F,
  carved out of the `pad03C` range) and `unk6C`/`unk1B0`/`unk1E0` (each a
  zeroed `s32`, carved out of existing single-word pad ranges).
- `UnkSlotEntry_3ac78::unk2` (u16, was `pad2`) and `unk14`/`unk18` (two new
  `s32`s, replacing the trailing `pad14[0x1C-0x14]`).
- `UnkSlotChildMethods_3ac78::slot88` (+0x088, appended directly after the
  existing `slot84` with no gap) and `UnkSlotChildObj_3ac78::unk10`/`unk20`/
  `unk32` (new fields carved out of existing opaque padding).
- `GenericMethodsHeader::slot4C`/`slot70` and `GenericObject::unk10` --
  additive extensions of a struct already used by two OTHER already-matched
  functions in this unit (`Class866E8__OnNotify`, `Class866E8__OnCommand`). Verified safe:
  neither touches the new slots/fields, and the whole-image SHA1 stayed
  green immediately after this specific edit, before any of this function's
  own body was written.

## Two real defects found, one MISREAD structural detail corrected along the way

1. **The `self->unk54` copy is a WHOLE-STRUCT assignment, not three
   separate field copies.** First read of the disassembly mis-transcribed
   the shape as interleaved load/store/load/store/load/store; the actual
   instructions are three loads THEN three stores (batched) -- the same
   "whole-struct `=` compiles to a batched block move" idiom as
   `HistoryBlock_3ac78` (documented elsewhere in this header). Writing
   `self->unk54 = *arg1;` / `self->unk54 = D_8008682C;` instead of
   field-by-field fixed a register-swap-and-shift residue immediately
   (23/163 -> 70/163 in one change).
2. **`new_class_6d940` takes an argument, not zero.** Retail sets
   `$a0 = 0` right after the `slot88` dispatch and never touches it again
   before the `jal` -- the "leftover register is a forwarded/explicit
   argument" tell, same family as this round's `class_3bb8c_i` slot-arity
   fixes, except here the argument is a plain literal `0` rather than
   forwarded. Declaring it `(s32 arg1)` and calling `new_class_6d940(0)`,
   plus reordering the four field-zeroing statements to match retail's
   actual order (`unk14, unk18, unk2, unk0`, not declaration order),
   fixed a second residue (70/163 -> 86/163).
3. **The very last residue was a single missing `move`.** After both fixes
   above, the function matched exactly except for a uniform 1-word address
   shift starting at `entry->unk10`'s reload into the loop cursor: retail
   emits `lw v0,0x10(s1); move s0,v0` (load into `$v0`, THEN copy to the
   persistent `$s0`) where the direct `p = (u8 *)entry->unk10;` phrasing
   compiled to a single `lw s0,0x10(s1)` (load straight into `$s0`).
   Introducing an explicit intermediate step did NOT fix it by itself
   (`Class866E8 **cellp = entry->unk10; p = (u8 *)cellp;` still compiled
   to one instruction) -- what fixed it was additionally computing `end`
   from `cellp` BEFORE `p`, i.e. `end = (u8 *)cellp + 0x668;` written
   textually before `p = (u8 *)cellp;`. That extra use of `cellp` between
   its definition and `p`'s assignment was enough to make GCC stage the
   value through `$v0` rather than target `$s0` directly. 86/163 -> 163/163.

## On the register-count screen

This is the first matched instance in the "8 distinct callee-saved
registers" band (previously 0/4 pooled). It does not overturn the
screen -- CLAUDE.md is explicit that it deprioritizes, not forbids, and
one instance does not out-weigh four stalls -- but it is worth recording
as a counter-example for calibration: a fully-saturated register file is
not automatically fatal when the body is long, straight-line, and has few
genuinely independent live values fighting for the SAME registers at the
SAME time (here, most of the register pressure comes from long-lived
values -- `self`, `arg2`, the loop index, the entry pointer, three loop
bounds/step constants -- that are each read/written in disjoint stretches
of the function rather than all contending across one tight expression).
The residues that showed up were exactly the two structural-shape
misreads above plus one single-instruction register-staging quirk, not an
unrecoverable "one value too many, no register to hold it" wall.

### Proposed learning

An explicit intermediate variable does not by itself force a compiler to
stage a value through a scratch register before committing it to a
persistent one -- GCC 2.6.3 will still fuse `local = expr; named = local;`
into one instruction if `local` has no OTHER use in between. What worked
here was giving the intermediate variable a second, real use (computing
`end` from it) BEFORE the assignment that needed the extra `move` --
i.e. the lever is "does this value get used more than once between its
definition and the point where retail shows a staged copy," not merely
"is there a named temp in the C."
