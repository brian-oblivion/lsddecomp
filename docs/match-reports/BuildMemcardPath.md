# BuildMemcardPath

> Renamed from `func_8004F32C` on 2026-09-20 (tools/rename.py). Address 0x8004f32c.

**Unit:** TitleMenuTaskObjF · **Size:** 26 words (0x68) · **Status:** MATCH

## What it does

`char *BuildMemcardPath(McDevicePath *dest, s32 selector, char *suffix)`.
Copies a fixed 6-byte PS-X BIOS memory-card device-name template
("bu10:" when `selector` is nonzero, "bu00:" otherwise —
`asm/data/7B008.sdata.s`, `gMcDevicePath1`/`gMcDevicePath0`) into `dest`, appends
`suffix` with this project's own `strcat` (matched elsewhere,
`src/app/GameApplicationFileResource.c`), and returns `dest`.

This function's `dest` argument is **not** the `TaskObjF` class this unit's
other queued functions operate on (see `TaskObjF__ForEachEvent`'s report for that
class) — it plainly overwrites its own object's first 6 bytes, which would
corrupt a vtable pointer at offset 0 if it were the same kind of object.
Treated as an unrelated small helper.

## Key lever

**Select the SOURCE POINTER first, then do ONE struct copy** — not a
struct copy inside each branch of the `if`. The natural first attempt
(`if (selector) *dest = gMcDevicePath1; else *dest = gMcDevicePath0;`) duplicates
the whole 8-instruction unaligned-copy sequence into both arms (10 words
too long, 0x90 vs retail's 0x68). Choosing a `McDevicePath *src` in the
`if`/`else` and doing the assignment once afterward matches exactly.

## Struct

`McDevicePath` is a 6-byte all-`s8` struct (natural alignment 1) — this
reproduces retail's unaligned `lwl`/`lwr` + two `sb` copy, the same idiom
already documented for `Descriptor10` in `include/class_3bb8c.h`.

## Header additions (`include/class_3bb8c.h`, additive only)

- `McDevicePath` (new type) and its two extern instances `gMcDevicePath1`
  ("bu10:") / `gMcDevicePath0` ("bu00:").
- `extern char *strcat(char *dest, char *src);` — **an extern for a
  function outside this unit** (matched in `src/app/GameApplicationFileResource.c`, declared in
  `include/GameApplicationFileResource.h`; this header had no prior declaration of it, so
  this is a fresh, independent one, not an edit to an existing
  declaration).

## Extern arity (round 59)

**Verdict: arity-ok idiom**, and the clearest case in the round: one symbol,
two call sites in ONE unit, with different argument counts, both byte-load-bearing.

**Callee evidence** (the matched definition in `src/ui/TitleMenuTaskObjF.c`):
`char *BuildMemcardPath(McDevicePath *dest, s32 selector, char *suffix)` — three
real arguments, `$a2` being the suffix string it appends.

**SUPERSEDED, round 75 (see the correction at the end): `TitleMenuTaskObjF` now declares a real three-argument prototype.** Original reasoning, kept as history -- **why `src/ui/TitleMenuTaskObjF.c` must declare it unprototyped.** Its two call sites
pass different numbers of arguments, and retail's bytes show both:

```
8004ea54:  lw    a1,12(v0)                 <- TaskObjF__OpenAndReadMemcardFile: $a0/$a1 only, no $a2
8004ea58:  jal   8004f32c <BuildMemcardPath>

8004ece4:  lw    a1,12(v0)                 <- TaskObjF__ProbeCardFreeSpace: $a0/$a1 ...
8004ece8:  lui   a2,0x8009                 <- ... and $a2, the &sMcTempFileSuffix suffix
8004ecec:  addiu a2,a2,-21844
8004ecf0:  jal   8004f32c <BuildMemcardPath>
```

`TaskObjF__OpenAndReadMemcardFile`'s call (`BuildMemcardPath(pathBuf, self->unkC)`) relies on `$a2`
already holding a usable suffix pointer from the caller — the dead-argument
idiom — while `TaskObjF__ProbeCardFreeSpace`'s call materialises one. No single prototype
compiles both: a 3-parameter one makes the first call `too few arguments`, a
2-parameter one makes the second `too many`. The unspecified parameter list is
the only spelling, and it is the same idiom this unit already uses for
`strcpy`/`strcat`.

**Declaration sites changed:** none (arity unchanged). `/* arity-ok: ... */`
added to both copies, `src/ui/TitleMenuTaskObjF.c:89` and `:383`. Oracle green.

## Naming (round 60, track 3)

`func_8004F32C` -> `BuildMemcardPath`. **Tier A.** Free function (first
parameter is a `dest` buffer, not a `self` of any established class, so
named `VerbNoun` per convention rather than `Class__Method`). Mechanics
and purpose both evident from the body alone: copies the BIOS `bu10:`/
`bu00:` device-name template selected by `selector` into `dest`, appends
`suffix`, and returns `dest` -- the whole point of the function is
building a memory-card path string.

## Head correction, round 75

The "must declare it unprototyped" reasoning above is disproved. Round 75
matched `TaskObjF__OpenAndReadMemcardFile` (TitleMenuTaskObjF) by calling this function with THREE
arguments; that call site's third argument is forwarded from its own third
parameter, already in `$a2`, so no set-up is emitted and it read as a
two-argument call. `src/ui/TitleMenuTaskObjF.c` now declares
`extern void *BuildMemcardPath(void *dest, s32 selector, void *suffix);`
and every call site in both units passes three. See `TaskObjF__OpenAndReadMemcardFile.md`.

## Round 94 (track 6, charlie): history moved from include/class_3bb8c.h

Round 34's note: five of the six prototypes that sat in the header were the
PSX BIOS file trampolines, Sony's `open`/`read`/`lseek`/`close`/`delete`,
linked from libapi/a50, a52, a51, a54, a69. They left the SHARED header
deliberately: a prototype for a function another unit (here a Sony object)
defines belongs in the `.c` that calls it, and `open`/`read`/`close` are
generic enough that a future include/psyq prototype (measured 2026-09-12:
none of the shipped headers declares them, only comments and O_* macros)
would collide in whichever including unit pulled both in; the two callers
also disagree about the first argument's type. Each caller (class_3bb8c_e.c,
class_3bb8c_f.c) carries its own `extern`. StampSaveTitleFileLetter stayed:
game code, src/class_3bb8c_g.c, matched round 45 (60/60 words; was
gp_rel-blocked, resolved round 42). `DeviceName866E8` is `McDevicePath`
(round 94); its comment's pointer to "the same idiom documented for
Descriptor10 above" was stale and is gone.

## Round 95 (track 7, charlie)

### Naming

`selector` -> `cardSlot` (both callers pass `self->cardSlot`), `src` ->
`device`. The `(char *)dest` casts stay: `McDevicePath` is an all-`s8`
struct (include/class_3bb8c.h, alignment 1 for the whole-struct copy), so it
has no `char` member to name. Zero bytes.

### Moved from src/ui/TitleMenuTaskObjF.c

The forward declaration's comment, replaced by one line:

```c
/* Forward declarations: these are defined later in this file (strict
 * ROM-address order), but earlier functions call them. The class's own
 * methods are prototyped in include/TaskObjF.h (track 4, round 89); these
 * two are the unit's plain helpers.
 *
 * `BuildMemcardPath`'s entry here fixes a real Gate-0 warning (round 60):
 * `TaskObjF__TryReadMemcardFile` (line ~52) calls it before its line-226 definition, and
 * without a prototype in scope cc1 implicitly declares it as returning
 * `int`, then complains at the real definition ("type mismatch with
 * previous implicit declaration", "was previously implicitly declared to
 * return `int'"). This is a same-file forward-declaration gap, not a
 * cross-unit signature disagreement -- both the call site and the
 * definition are in this .c, so the fix is simply adding the prototype
 * here like its neighbours. Confirmed byte-identical after the fix
 * (`TaskObjF__TryReadMemcardFile` is already MATCHED and stays MATCHED): a pointer
 * return value lives in `$v0` either way, so the implicit-int reading
 * never produced different code, only a diagnostic. */
```
