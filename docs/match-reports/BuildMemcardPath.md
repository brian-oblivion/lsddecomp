# BuildMemcardPath

> Renamed from `func_8004F32C` on 2026-09-20 (tools/rename.py). Address 0x8004f32c.

**Unit:** class_3bb8c_f · **Size:** 26 words (0x68) · **Status:** MATCH

## What it does

`char *BuildMemcardPath(DeviceName866E8 *dest, s32 selector, char *suffix)`.
Copies a fixed 6-byte PS-X BIOS memory-card device-name template
("bu10:" when `selector` is nonzero, "bu00:" otherwise —
`asm/data/7B008.sdata.s`, `D_8008AA9C`/`D_8008AAA4`) into `dest`, appends
`suffix` with this project's own `strcat` (matched elsewhere,
`src/code_171e0.c`), and returns `dest`.

This function's `dest` argument is **not** the `TaskObjF` class this unit's
other queued functions operate on (see `TaskObjF__ForEachEvent`'s report for that
class) — it plainly overwrites its own object's first 6 bytes, which would
corrupt a vtable pointer at offset 0 if it were the same kind of object.
Treated as an unrelated small helper.

## Key lever

**Select the SOURCE POINTER first, then do ONE struct copy** — not a
struct copy inside each branch of the `if`. The natural first attempt
(`if (selector) *dest = D_8008AA9C; else *dest = D_8008AAA4;`) duplicates
the whole 8-instruction unaligned-copy sequence into both arms (10 words
too long, 0x90 vs retail's 0x68). Choosing a `DeviceName866E8 *src` in the
`if`/`else` and doing the assignment once afterward matches exactly.

## Struct

`DeviceName866E8` is a 6-byte all-`s8` struct (natural alignment 1) — this
reproduces retail's unaligned `lwl`/`lwr` + two `sb` copy, the same idiom
already documented for `Descriptor10` in `include/class_3bb8c.h`.

## Header additions (`include/class_3bb8c.h`, additive only)

- `DeviceName866E8` (new type) and its two extern instances `D_8008AA9C`
  ("bu10:") / `D_8008AAA4` ("bu00:").
- `extern char *strcat(char *dest, char *src);` — **an extern for a
  function outside this unit** (matched in `src/code_171e0.c`, declared in
  `include/code_171e0.h`; this header had no prior declaration of it, so
  this is a fresh, independent one, not an edit to an existing
  declaration).

## Extern arity (round 59)

**Verdict: arity-ok idiom**, and the clearest case in the round: one symbol,
two call sites in ONE unit, with different argument counts, both byte-load-bearing.

**Callee evidence** (the matched definition in `src/class_3bb8c_f.c`):
`char *BuildMemcardPath(DeviceName866E8 *dest, s32 selector, char *suffix)` — three
real arguments, `$a2` being the suffix string it appends.

**Why `src/class_3bb8c_e.c` must declare it unprototyped.** Its two call sites
pass different numbers of arguments, and retail's bytes show both:

```
8004ea54:  lw    a1,12(v0)                 <- func_8004EA38: $a0/$a1 only, no $a2
8004ea58:  jal   8004f32c <BuildMemcardPath>

8004ece4:  lw    a1,12(v0)                 <- func_8004ECCC: $a0/$a1 ...
8004ece8:  lui   a2,0x8009                 <- ... and $a2, the &D_8008AAAC suffix
8004ecec:  addiu a2,a2,-21844
8004ecf0:  jal   8004f32c <BuildMemcardPath>
```

`func_8004EA38`'s call (`BuildMemcardPath(pathBuf, self->unkC)`) relies on `$a2`
already holding a usable suffix pointer from the caller — the dead-argument
idiom — while `func_8004ECCC`'s call materialises one. No single prototype
compiles both: a 3-parameter one makes the first call `too few arguments`, a
2-parameter one makes the second `too many`. The unspecified parameter list is
the only spelling, and it is the same idiom this unit already uses for
`strcpy`/`strcat`.

**Declaration sites changed:** none (arity unchanged). `/* arity-ok: ... */`
added to both copies, `src/class_3bb8c_e.c:89` and `:383`. Oracle green.

## Naming (round 60, track 3)

`func_8004F32C` -> `BuildMemcardPath`. **Tier A.** Free function (first
parameter is a `dest` buffer, not a `self` of any established class, so
named `VerbNoun` per convention rather than `Class__Method`). Mechanics
and purpose both evident from the body alone: copies the BIOS `bu10:`/
`bu00:` device-name template selected by `selector` into `dest`, appends
`suffix`, and returns `dest` -- the whole point of the function is
building a memory-card path string.
