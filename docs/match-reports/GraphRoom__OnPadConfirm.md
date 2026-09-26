# GraphRoom__OnPadConfirm -- MATCHED (25/25)

> Renamed from `GraphRoomObj__HandleUnscored` on 2026-09-26 (tools/rename.py). Address 0x800581c4.

> Renamed from `func_800581C4` on 2026-09-24 (tools/rename.py). Address 0x800581c4.

Unit: `src/class_3bb8c_t.c`. Class: `gGraphRoomMethods`, own vtable slot `+0x078`
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
