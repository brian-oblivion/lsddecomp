# InitCdDrive — MATCHED (20/20 words)

> Renamed from `func_80027E78` on 2026-09-17 (tools/rename.py). Address 0x80027e78.

Round 45, runner echo (second sitting), `src/code_179d8_q.c`.

## Result

Byte-exact on the first attempt.

```c
/* libcd/sys entry points (lib/libcd/sys.o, linked since round 34) --
 * per-call-site typed for this unit, per the code_179d8_h.c convention. */
extern s32 CdSetDebug(s32 arg0);
extern s32 CdControlB(u_char com, void *param, void *result);

extern s32 sCdDriveInited;

void InitCdDrive(void)
{
    u8 mode;

    if (sCdDriveInited != 0) {
        return;
    }

    CdSetDebug(0);
    mode = 0x80;
    while (CdControlB(0xE, &mode, 0) == 0) {
    }
    sCdDriveInited = 1;
}
```

## Derivation

`sCdDriveInited` is a one-shot guard: early-return if already set. Otherwise
`CdSetDebug(0)`, store `0x80` (= `CdlModeSpeed`) into a one-byte stack local,
then a do-while retry loop calling `CdControlB(0xE /* CdlSetmode */, &mode,
0)` until it returns nonzero, then set the guard.

`CdSetDebug`/`CdControlB` are libcd/sys.o entry points (Psy-Q `libcd`,
linked since round 34 per `src/code_179d8_h.c`'s header comment) — real
signatures are in `include/psyq/libcd.h` (`int CdSetDebug(int level);`,
`int CdControlB(u_char com, u_char *param, u_char *result);`). Followed the
existing `code_179d8_h.c` convention of a unit-local, per-call-site `extern`
rather than including `LIBCD.H` — this call site never touches `result`, so
it's typed `void *` here instead of the header's `u_char *`, no behavioral
difference.

The do-while loop reloading `a0 = 0xE` in the branch's delay slot on every
iteration is the standard GCC 2.6.3 shape for a constant argument not kept
live across a call inside a loop — matches `while (CdControlB(...) == 0);`
directly, no manual unrolling needed.

### Proposed learning

None new — confirms the libcd/sys.o per-call-site `extern` convention from
`code_179d8_h.c` transfers cleanly to a second, independently-carved unit.

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027E78` | `InitCdDrive` | A |
| `D_8008A858` | `sCdDriveInited` | A |

**Evidence.** Guarded by a one-shot flag, it calls `CdSetDebug(0)` and then
retries `CdControlB(0x0E, &mode, 0)` with `mode = 0x80` until it is accepted,
then sets the flag. `0x0E` is Psy-Q's `CdlSetmode` and `0x80` its
`CdlModeSpeed` (double speed) -- `include/psyq/libcd.h`. So: put the drive
into double-speed mode, once. Both constants are now spelled
`CD_CMD_SETMODE` / `CD_MODE_DOUBLE_SPEED` in the `.c` rather than as bare
literals; they are NOT spelled with Sony's own macro names, because this unit
declares libcd per call site and including `LIBCD.H` would collide with those
declarations.

`sCdDriveInited` takes the unit-static `s` prefix: no other unit in `src/`
references it (`grep -rn D_8008A858 src/` before the rename: zero hits outside
this file).

### Proposed learning

**`tools/rename.py` leaves `config/gp-symbols.txt` in a state
`tools/gpsyms.py --check` calls stale, and the build stays GREEN while it is
in that state -- so the oracle does not catch it.** `rename.py` lists
`config/gp-symbols.txt` among the text files it rewrites, so a renamed sdata
symbol keeps its gp-relative treatment and the image stays byte-identical;
what it does not do is regenerate the file, and `gpsyms.py` writes the
generated names in a different order, so `--check` reports stale until the
generator is run. A naming runner that renames any `.sdata`/`.sbss` symbol
must therefore finish with `python3 tools/gpsyms.py` and one more
`./build-and-verify.sh` before committing -- the green build in between is
real, but the committed tree would otherwise fail `gpsyms.py --check` for the
next person. Measured this round across 12 sdata renames.

## Round 96 (track 6, echo)

The unit takes `<libcd.h>`: `CdSetDebug`/`CdControlB` are Sony's prototypes
and the local `CD_CMD_SETMODE`/`CD_MODE_DOUBLE_SPEED` are Sony's own
`CdlSetmode` (0x0E) and `CdlModeSpeed` (0x80). Byte-exact.

## Track 7 (round 101, echo): comments moved here, and names

CdControlB's result argument `0` is spelled `NULL` (libcd.h: `u_char
*result`).
