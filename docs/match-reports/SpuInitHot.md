# SpuInitHot -- MATCHED (8/8 words)

> **Head, round 78: SONY CODE, identified under FINISHING-PLAN track 2 as `SpuInitHot`** (libspu/s_ih, `_SsInit`'s mode!=0 arm opposite `SpuInit`): EXACT `sdkname.py` fingerprint, 4-way AMBIGUOUS, settled by position and body; evidence on its symbols-file `identified` line. It counts as library now; the `## Naming` section below that kept `func_` is superseded.

> Renamed from `func_80036AA8` on 2026-09-24 (tools/rename.py). Address 0x80036aa8.

`asm/nonmatchings/libsnd_cres/SpuInitHot.s`, vram `0x80036AA8`, unit
`libspu_s_ih` (split off `libsnd_cres` in round 34; this report predates
the split and had not been updated). One-line wrapper: `func_80038E44(1)`.

```c
/* func_80038E44 is defined in the uncarved psyq_SpuSetMute unit; its own
 * body is a single straight-line path (no branches) ending in a chain of
 * global stores with $v0 never touched afterward -- genuinely void, not
 * just an unobserved return. */
extern void func_80038E44(s32 a0);

void SpuInitHot(void)
{
    func_80038E44(1);
}
```

Unlike `func_80036024`/`func_80036044` (which wrap `func_80038D74`, a
function that DOES compute a real return value right before its own `jr
$ra`), `func_80038E44` is a single straight-line body with no branches at
all, ending in a chain of global stores with `$v0` never referenced
afterward anywhere in the function. Per CLAUDE.md's rule on one-line
wrappers, the DEFAULT is a real return type absent positive void evidence --
this is exactly that positive evidence: one exit path, and it never touches
`$v0` for a return purpose. Declared `void`.

No residue -- matched on the first attempt.

## Verification

`./build-and-verify.sh` exit 0 (full-image SHA1 match, all 15 matched
functions in this unit still green). `funcdiff.py SpuInitHot`: 8/8 words.

## Naming (round 78, track 3)

Kept as `SpuInitHot` -- **tier C**, and deliberately not a class-scoped
`Class__func_xxxxx` form, because it is a free function: it is not a method
(no vtable slot, no `this`, `tools/classtable.py --scan` has no entry
touching this address) and has exactly one caller.

Evidence considered and why it falls short of tier A/B:
- **Body**: a single unconditional call, `func_80038E44(1)`. No branch, no
  computed value, no field access -- the mechanics ARE "call this one other
  function with the constant 1", which says nothing about game purpose.
- **Callee**: `func_80038E44` is itself unnamed, uncarved, and lives in the
  "game's own libspu build" gap (`0x29644..0x2976C`, no SDK disc covers it;
  see `src/libspu_s_ih.c`'s header and `config/splat.slps01556.lsdde.yaml`
  line ~1131). It is not a placed Sony object (`tools/sdkstalls.py` has no
  hit for `SpuInitHot`), so this is not the "give no game name to
  anything Sony owns" case -- but its own purpose is equally undetermined
  (round 78 did not investigate `func_80038E44`'s body; out of scope for
  this one-function unit), so there is nothing to inherit a name from.
- **Caller**: the ONLY caller is `_SsInit` (`src/libsnd_ssinit.c`), which
  branches `if (arg0 == 0) SpuInit(); else SpuInitHot();` -- i.e. this is
  the alternate sound-init path taken when `_SsInit`'s own argument is
  nonzero. `_SsInit` itself is reached only through `SsInit` (calls
  it with `arg0 = 0`) and `SsInitHot` (`arg0 = 1`), both one-line
  wrappers with no report evidence yet for what selects between them
  (`docs/match-reports/SsInit.md`, `SsInitHot.md`). A single
  caller does not meet the tier-A bar ("two or more callers that agree"),
  and inventing a purpose from the branch alone (e.g. "muted"/"quiet" init)
  would be a guess CLAUDE.md's naming rule specifically forbids ("name what
  the code does, never what you guess it is for").

Net: mechanics are trivial and purpose is unestablished two levels up the
call chain. `SpuInitHot` stays a placeholder; the next runner to name
`func_80038E44` (wherever it eventually carves) may retroactively unlock a
tier-B/A name here.

## File history

Round 90 (track 8) renamed the carve unit code_179d8_f_b to
`src/libspu_s_ih.c`, for the Sony module it holds. Its banner is moved here
verbatim, as it stood before the rename.

```c
/*
 * code_179d8_f_b -- the TAIL half of the old code_179d8_f slice, split off in
 * round 34 (2026-09-12) when Sony's `libsnd/stop.o` was linked into the middle
 * of it. File 0x272A8..0x272C8, vram 0x80036AA8..0x80036AC8: ONE function.
 *
 * WHY THE SPLIT EXISTS. `func_800368E8`, `func_80036A54` and `func_80036A7C`
 * are Sony's `Snd_stop`, `SsSeqStop` and `SsSepStop` (`libsnd/stop`, Psy-Q
 * 3.3, 0x1C0 text covering exactly those three). All three had been MATCHED as
 * C; reclassifying them out of the game count is the correction CLAUDE.md asks
 * for, not a regression. A placed object cannot live inside a `c` segment, so
 * the slice had to become [c][o][c] and the second `c` needed its own name.
 * The first half kept `code_179d8_f`.
 *
 * A ONE-FUNCTION UNIT IS FINE and has precedent (`class_3bb8c_v`). Nothing
 * here is a candidate for folding into a neighbour: `code_179d8_f` ends at the
 * object, and 0x272C8 onward is the psyq_SpuSetMute block.
 *
 * NO RODATA ATTACH CAME WITH THIS HALF, and that is measured, not assumed:
 * the old code_179d8_f owned zero `jtbl_` references and the splat yaml's
 * rodata slot list names no `.rodata, code_179d8_f` line at all. So unlike
 * round 33's libsnd_ssinit_libapi_counter there was nothing to move, and a link failure of
 * the form `undefined reference to '.L8003....'` would mean something else.
 *
 * Declarations: keep anything that encodes THIS unit's reading next to the
 * code, in this file. Do not create a shared code_179d8*.h -- the sibling
 * slices are staffed independently and a shared header is what makes their
 * merges collide.
 *
 * ROUND 78: the one function is Sony's `SpuInitHot` (libspu/s_ih),
 * identified by the head under FINISHING-PLAN track 2 -- EXACT fingerprint,
 * first function of the libspu run, and the call site: `_SsInit` calls it on
 * mode!=0 where it calls `SpuInit` on mode==0. It calls func_80038E44 (still
 * parked unidentified; SpuInit's own callee) with 1. It counts as library;
 * nothing in this unit is game code.
 */
```

## History (moved from src/libspu_s_ih.c, comments pass)

The file's banner carried its edge evidence:

> What decided its edges (python3 tools/tuboundary.py): the placed object
> libsnd/stop precedes it ("start edge possible"), and the placed object
> libspu/s_sm (SpuSetMute) follows it. One function between two objects,
> so nothing to merge with.
>
> The carve history is in docs/match-reports/SpuInitHot.md, "File history".
