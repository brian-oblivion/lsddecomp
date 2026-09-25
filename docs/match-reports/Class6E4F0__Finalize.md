# Class6E4F0__Finalize

> Renamed from `func_8003B024` on 2026-09-25 (tools/rename.py). Address 0x8003b024.

**Round 81 (delta)** · **Unit:** code_2b78c · **Size:** 2 words · **Status:** MATCHED (2/2 words, whole-image SHA1 green)

## What it does

Empty. It is the `+0x00C` (finalize) override in D_8006E4F0's method table
(`classtable.py 0x8006E4F0 --vs 0x8006B58C`), inherited verbatim by
Class6D3C8 (D_8006D3C8). So this class's finalize does NOT chain to
BasicClass__Finalize.

```c
void Class6E4F0__Finalize(Class6E4F0 *self) {
}
```

`jr $ra; nop` -- no derivation. The parameter is the slot's signature
(BASICCLASS_SLOTS finalize), not something the bytes show.

## Naming

Kept as `Class6E4F0__Finalize`: renaming is head work (symbols file). Suggested
tier-B name `Class6E4F0__Finalize` (slot identity, from the table).
