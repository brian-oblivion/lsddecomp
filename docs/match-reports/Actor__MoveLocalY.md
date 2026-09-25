# Actor__MoveLocalY -- MATCHED (14/14)

> Renamed from `DreamSys__ApplyOffsetSlot1` on 2026-09-25 (tools/rename.py). Address 0x800574fc.

> Renamed from `func_800574FC` on 2026-09-19 (tools/rename.py). Address 0x800574fc.

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys`, own vtable slot `+0x0CC`
(base-class-inherited; resolved via `tools/classtable.py DREAMSYS_METHODS`).

## Signature

```c
void Actor__MoveLocalY(DreamSys *self, s32 val, void *extra);
```

## Body

```c
void Actor__MoveLocalY(DreamSys *self, s32 val, void *extra) {
    Actor__MoveAlongLocalAxis(self, &D_8008ABA4[1], val, extra, 8);
}
```

Sibling of `Actor__MoveLocalX` (see that report for the shared helper and the
`D_8008ABA4[2]` array discovery): same shape, writes element 1 instead of
element 0, and passes `8` instead of `7` as the trailing constant.

## Naming

**`Actor__MoveLocalY` -- tier B.** Sibling of
`Actor__MoveLocalX` (see that report's Naming section): same
reasoning, element 1 instead of 0.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py Actor__MoveLocalY   # 14/14
```
