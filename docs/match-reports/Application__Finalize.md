# Application__Finalize

> Renamed from `Class6E4F0__Finalize` on 2026-09-26 (tools/rename.py). Address 0x8003b024.

> Renamed from `func_8003B024` on 2026-09-25 (tools/rename.py). Address 0x8003b024.

**Round 81 (delta)** · **Unit:** code_2b78c · **Size:** 2 words · **Status:** MATCHED (2/2 words, whole-image SHA1 green)

## What it does

Empty. It is the `+0x00C` (finalize) override in D_8006E4F0's method table
(`classtable.py 0x8006E4F0 --vs 0x8006B58C`), inherited verbatim by
Class6D3C8 (D_8006D3C8). So this class's finalize does NOT chain to
BasicClass__Finalize.

```c
void Application__Finalize(Application *self) {
}
```

`jr $ra; nop` -- no derivation. The parameter is the slot's signature
(BASICCLASS_SLOTS finalize), not something the bytes show.

## Naming

**Round 81 (delta), track 3.** Renamed `func_8003B024` -> `Application__Finalize`
(slot named like the method it dispatches: it is the +0x00C dtor/finalize
override the `BASICCLASS_SLOTS` macro types). **Tier A**: a pure empty leaf
whose mechanics ARE its purpose -- it is the finalize slot and it does
nothing, full stop; nothing about "why" is claimed.
