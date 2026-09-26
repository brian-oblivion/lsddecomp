# Application__NoOpSlot48

> Renamed from `Class6E4F0__NoOpSlot48` on 2026-09-26 (tools/rename.py). Address 0x8003b108.

> Renamed from `func_8003B108` on 2026-09-25 (tools/rename.py). Address 0x8003b108.

**Round 81 (delta)** · **Unit:** code_2b78c · **Size:** 2 words · **Status:** MATCHED (2/2 words, whole-image SHA1 green)

## What it does

Empty. Slot `+0x048` of gApplicationMethods, the third of the class's own four
slots; GameApplication inherits it unchanged. No caller dispatches it in carved
code that this unit sees.

```c
void Application__NoOpSlot48(Application *self) {
}
```

## Naming

**Round 81 (delta), track 3.** Renamed `func_8003B108` -> `Application__NoOpSlot48`.
**Tier A**: a pure empty leaf at the +0x048 slot (no established method
name to dispatch as, since no carved caller invokes it by name); the
mechanics -- it is slot 48 and it does nothing -- are the whole claim.
