# Class866E8__OnNotifyTag1

> Renamed from `func_8004BD14` on 2026-09-24 (tools/rename.py). Address 0x8004bd14.

**Unit:** class_3bb8c · **Size:** 80 words · **Status:** MATCHED (first attempt).

## Result

```c
void Class866E8__OnNotifyTag1(Obj866E8 *self, void *arg1, s32 mode) {
    s32 i;
    Elem *e;
    s32 curMode;

    if (mode != 2) {
        return;
    }
    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (e->unk4->unk2E != 0) {
            e->unk4->unk2E = 0;
            self->methods->slot88(self, 7, e, i);
        }
        curMode = self->unk1B0;
        if (curMode == 1 && e->flag != 0) {
            if (e->unk4->unk2C != 0) {
                self->methods->slot104(self, e);
                e->unk4->unk2C = 2;
                e->flag = 0;
                if (--self->unk1B4 == 0) {
                    self->unk1B4 = 0;
                    self->unk1B0 = 0;
                    self->unk1B8 = curMode;
                }
            } else if (e->unk4->unk2A == 0) {
                e->flag = 0;
            }
        }
    }
}
```

`self->arr[7]` walk with stride `0x1C` (matching `Elem`'s already-established
size). `arg1` (the class_3ac78 independent view of this same slot,
`slot100`, is `void (*)(Class866E8 *self, void *arg1, s32 arg2)`) is truly
unused here -- confirmed by register tracing, `$a1` is never read.

`curMode = self->unk1B0;` is the interesting piece: `self->unk1B0` is read
ONCE per outer-loop iteration into a local, checked against `1`, and later --
AFTER `self->unk1B0` has ALREADY been zeroed a few lines down in the same
branch -- written back into `self->unk1B8`. Reproducing this needs the
explicit local (`curMode`), not a second read of `self->unk1B0`, because by
the time of the final store the field no longer holds `1`.

This function is also where `ElemTarget`'s `+0x02A`/`+0x02C`/`+0x02E` fields
(all read/written through `e->unk4`) were established, and where
`Obj866E8Methods::slot88`/`slot104` got their signatures (both cross-checked
against class_3ac78's independent view of the same vtable, which names them
identically in arity if not in exact parameter types).

### Proposed learning

**A field read once into a local, tested, and later WRITTEN BACK to a
different field after the ORIGINAL field has itself been overwritten in the
interim, needs the explicit local -- re-reading the original field at the
write-back site would read the now-stale (already-zeroed) value.** A second
instance of the "value reused after an intervening write needs an explicit
local" family already documented in DECOMPILATION_LEARNINGS.md, but the
INTERVENING operation here is a plain field STORE (`self->unk1B0 = 0;`), not
a call -- worth generalizing that entry's discriminator ("what sits BETWEEN
the reads") to include a direct sibling-field write, not just a call or an
aliasing-suspect memory op.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004BD14` | `Class866E8__OnNotifyTag1` | B | Occupant of `D_800866E8` +0x100 (`slot100`). `class_3ac78`'s `Class866E8__OnNotify` (already matched) dispatches this slot, unconditionally, only when `(sender->methods->header & 0xF) == 1` -- i.e. only for one sender class tag. Follows the SAME "OnNotifyTagN" naming convention already established elsewhere in this codebase for identical per-tag dispatch targets (`Unk18Obj__OnNotifyTag5`/`Unk18Obj__OnNotifyTag1`, `src/code_2cc8c_d.c`). `mode` here is the caller's own `command`, but the body itself early-returns unless `mode == 2` -- narrower than "every tag-1 notification", which the name does not claim (only that this IS the tag-1 dispatch target). |
