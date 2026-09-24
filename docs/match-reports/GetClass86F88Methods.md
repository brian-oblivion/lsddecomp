# GetClass86F88Methods -- MATCH

> Renamed from `func_80052B60` on 2026-09-24 (tools/rename.py). Address 0x80052b60.

Unit `class_3bb8c_k`, round 15. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py GetClass86F88Methods`: 4/4 words match.

## Source

```c
Class86F88Methods *GetClass86F88Methods(void)
{
    return &gClass86F88Methods;
}
```

## Notes

A plain no-argument accessor returning `&gClass86F88Methods`, the same shape as
`GetClass86668Methods`/`gClass86668Methods` documented elsewhere in this project (base
class table getters). `gClass86F88Methods` is this unit's own class's vtable
(39 slots, `tools/classtable.py gClass86F88Methods`); the words at `+0x054`,
`+0x058`, `+0x07C`..`+0x09C` of that table are this unit's own
`Class86F88__SetState`/`Class86F88__TickClosing`/`Class86F88__ScrollRight`.../`Class86F88__GetCursorIndex`, all
matched this round. Matched first attempt.

## Naming

Round 75 (bravo, track 3). `func_80052B60` -> `GetClass86F88Methods`, **tier A**.

Returns &gClass86F88Methods. Used as the ctor table by New_Class86F88 and Class86F88__Class86F88 (class_3bb8c_j). Named like GetClass86668Methods/GetObjMMethods.
