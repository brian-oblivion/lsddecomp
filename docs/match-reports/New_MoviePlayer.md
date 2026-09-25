# New_MoviePlayer -- MATCHED (35/35 words)

> Renamed from `func_80045438` on 2026-09-25 (tools/rename.py). Address 0x80045438.

Round 82, runner echo (code_33808 session, echo #8), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 35/35 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Allocator for D_8006F614: BMemPMgrAlloc(0x6C); if non-NULL, call the table's ctor (GetMoviePlayerMethods +0x008) with (obj, a, b, c); a ZERO result returns obj, nonzero frees it and returns NULL. Not referenced from any data word (a direct-call allocator).

Table slot (`tools/classtable.py`): none (allocator; constructs D_8006F614).

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808` and `CountedBuf33808` sit at the top of
`src/code_33808.c`.

```c
void *New_MoviePlayer(s32 arg0, s32 arg1, s32 arg2) {
    void *obj = BMemPMgrAlloc(0x6C);

    if (obj != NULL) {
        if (((Ctor33808 *)GetMoviePlayerMethods())->ctor(obj, arg0, arg1, arg2) == 0) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}
```

## Notes

First build. The failing-ctor allocator lever with the test inverted: this class's ctor returns a status (0 = ok), unlike the self-returning ctors in the rest of the unit (`beqz v0 -> return obj`).

## Naming

- **New_MoviePlayer**, tier A. Called from src/code_2c054.c's TaskCoreObj__TaskCoreObj; externally typed `StreamTaskUnkB4Obj *` there. Opens a CD stream object and drives an MDEC decode/upload state machine (mechanics match the class name; the constructor/finalize/state-machine functions ARE playing a movie).
