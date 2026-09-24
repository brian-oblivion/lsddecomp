# Class86F88__GetCursorIndex -- MATCH

> Renamed from `func_80052B54` on 2026-09-24 (tools/rename.py). Address 0x80052b54.

Unit `class_3bb8c_k`, round 15. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py Class86F88__GetCursorIndex`: 3/3 words match.

This is vtable slot `+0x09C` of `gClass86F88Methods`, the LAST slot in that table
(`tools/classtable.py gClass86F88Methods` reports 39 slots ending here). Not
declared in `Class86F88Methods` since nothing in this unit dispatches
through it.

## Source

```c
s32 Class86F88__GetCursorIndex(Class86F88 *self)
{
    return self->unk28;
}
```

## Notes

A plain one-field getter. Matched first attempt.

## Naming

Round 75 (bravo, track 3). `func_80052B54` -> `Class86F88__GetCursorIndex`, **tier A**.

Slot +0x09C (`tools/classtable.py gClass86F88Methods`), a getter returning `cursorIndex`. The caller TaskObjF__OnItemSelected (class_3bb8c_g) calls it through the child's +0x09C on result 2 and stores the value as `selectedItem`.

Class86F88, per the round-75 pass, is a scrolling list selector: up to 4 visible rows of 26-character item text, a highlighted cursor row, a horizontal column offset (see the unit header comment of `src/class_3bb8c_k.c`).
