> Renamed from `func_80057C14` on 2026-09-19 (tools/rename.py). Address 0x80057c14.

# DreamSys__DispatchLinkCommand -- MATCHED (22/22)

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys`, own vtable slot `+0x0E0`
in the shared base table `D_800878D4` (`include/DreamSys.h`'s
`DreamSysBaseMethods::slot0xE0` already carried a comment naming this
exact function as its resolution). Not overridden at the top `DreamSys`
level, which instead has its own distinct `+0xE0` implementation
(`DreamSys__WallLink`, `LinkWall` in `include/DreamSys.h`).

## Signature

```c
void DreamSys__DispatchLinkCommand(DreamSys *self, void *arg1, s32 count);
```

## Body

```c
void DreamSys__DispatchLinkCommand(DreamSys *self, void *arg1, s32 count) {
    GetClass6B5CCMethods()->slot9C(self, arg1, count);
}
```

Sibling of `DreamSys__DispatchLinkCommandAndTryAttach` (see that report for `GetClass6B5CCMethods()` and this
unit's local `DreamSysBasicSlots` view): same single unconditional call
through the shared base table's `+0x09C` slot, but no second conditional
dispatch.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py DreamSys__DispatchLinkCommand   # 22/22
```
