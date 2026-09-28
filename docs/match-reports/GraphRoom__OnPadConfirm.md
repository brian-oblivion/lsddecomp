# GraphRoom__OnPadConfirm -- MATCHED (25/25)

> Renamed from `GraphRoomObj__HandleUnscored` on 2026-09-26 (tools/rename.py). Address 0x800581c4.

> Renamed from `func_800581C4` on 2026-09-24 (tools/rename.py). Address 0x800581c4.

Unit: `src/world/dream_scene.c`. Class: `gGraphRoomMethods`, own vtable slot `+0x078`
(resolved via `tools/classtable.py gGraphRoomMethods`).

## Signature

```c
void GraphRoom__OnPadConfirm(D_80087AACObj *self);
```

## Body

```c
void GraphRoom__OnPadConfirm(D_80087AACObj *self) {
    if (self->unk_0x238 == 0) {
        self->methods->slot70(self, 0x10);
        self->methods->slot94(self);
    }
}
```

Names `D_80087AACMethods::slot70`/`slot94` and `D_80087AACObj::unk_0x238`
(additive extension of this unit's own class struct).

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py GraphRoom__OnPadConfirm   # 25/25
```

## Naming (round 75, track 3)

**`GraphRoom__OnPadConfirm`** -- tier B. Own vtable slot +0x078. Its
one guard is `self->scored == 0`, exactly the failure value
`GraphRoom__ScoreDayLog`'s return produces -- so this function's whole
purpose is "when scoring hasn't succeeded, do X" (two further calls,
`slot70(self, 0x10)`/`slot94(self)`, whose own purpose is not established
past that gate). Named for the guard condition rather than guessing what
the two calls display.

## Track 4 (2026-09-26, round 87, alpha): renamed `GraphRoomObj__HandleUnscored` -> `GraphRoom__OnPadConfirm`

The class is unified in `include/graph_room.h` (class id 0x2F130, table `gGraphRoomMethods`, parent TaskCore; `tools/classtable.py gGraphRoomMethods --vs gTaskCoreMethods`). The class name drops round 75's `Obj` suffix (FINISHING-PLAN track 4 step 2); every function prefix moved with it. The unit's GraphRoomObj/GraphRoomMethods views are deleted and `self` is `GraphRoom *`; inherited fields and slots carry TaskCore's and IntermediateBase's names. Zero bytes changed (build-and-verify OK, typeviews --warnings 0 new). Named for its slot, track 4 step 6: +0x078 is TaskCore's `onPadConfirm` (onPadEvent's 0x19 case). Unscored only: playSound (+0x070, was slot70) with 0x10 -- TaskCore__OnPadConfirm's own tone -- and refreshViewValue (+0x094, was slot94).

## Track 7 (2026-09-27, round 97, delta)

`playSound(0x10)` is written `playSound(1 << 4)`: VabStreamObj__PlayTone's index is `program << 4 | tone` (SoundCueSet.h, dream_sys.h), so VAB program 1, tone 0, spelt as title_menu spells its own. Zero bytes changed.
