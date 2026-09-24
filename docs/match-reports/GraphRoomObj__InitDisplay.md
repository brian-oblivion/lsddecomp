# GraphRoomObj__InitDisplay -- MATCHED (26/26)

> Renamed from `func_80058078` on 2026-09-24 (tools/rename.py). Address 0x80058078.

Unit: `src/class_3bb8c_t.c`. Class: `gGraphRoomMethods`, own vtable slot `+0x040`
(resolved via `tools/classtable.py gGraphRoomMethods`).

## Signature

```c
void GraphRoomObj__InitDisplay(D_80087AACObj *self);
```

## Body

```c
extern char D_80011778[];

void GraphRoomObj__InitDisplay(D_80087AACObj *self) {
    self->unk_0x84 = 5;
    self->unk_0x2C = 0x190;
    self->methods->slotD4(self, D_80011778, 0);
    self->methods->slot6C(self, 0xA);
}
```

Uses `D_80011778` (the `"ETC\HGRAPH.TIM"` string, one of the two
standalone strings this unit's own header comment names -- resolved by
symbol, declared locally as `extern char D_80011778[];` since it is not
referenced by any other unit). Names `D_80087AACMethods::slot6C/slotD4`
and `D_80087AACObj::unk_0x2C/unk_0x84` (additive extensions).

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py GraphRoomObj__InitDisplay   # 26/26
```
