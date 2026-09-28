# SetDataDirectory -- MATCHED 3/3 words, round 42 (2026-09-15)

> Renamed from `func_800270AC` on 2026-09-27 (tools/rename.py). Address 0x800270ac.

> **VERDICT CORRECTED, round 42 (2026-09-15). THIS FUNCTION IS MATCHED.**
> It was blocked by `gp_rel`, which is RESOLVED this round: maspsx gained
> `--gp-symbols` / `--no-nop-mflo-mfhi` (`tools/patches/maspsx-lsd-flags.patch`),
> the whole image is byte-exact with the flags on, and this function was one of
> the live tests -- `gDataDirectory = value;`, as this report predicted. The C is in `src/app/GameApplicationFileResource.c`. Everything below is the
> pre-fix record and is kept as evidence.

> **REOPENED -- WAS ASSIGNABLE, SINCE MATCHED (marker spent), round 42 (2026-09-15).** This function was
> screened as blocked by `gp_rel`. **That blocker is RESOLVED**: maspsx gained
> `--gp-symbols` and `--no-nop-mflo-mfhi` (`tools/patches/maspsx-lsd-flags.patch`,
> passed by the Makefile), the whole image stays byte-exact, and previously
> blocked functions now match (see `docs/research/gp-relative-blocker.md`,
> "RESOLVED"). Everything below is evidence from before the fix: its
> derivation may still be right, its VERDICT is not. Rebuild before believing
> any score in it.

# SetDataDirectory

**Unit:** GameApplicationFileResource · **Size:** 3 instructions · **Status:** STALLED, class TOOLCHAIN

## What it does

A setter: `gDataDirectory = value;`. `gDataDirectory` is another slot in the same
`.sdata` region as `gActiveDataSource` (file `0x7b008`; see
`asm/data/7B008.sdata.s`), initialized in retail to `0x8006D4A8` — a pointer
value. `D_8006D4A8` itself sits right at the tail of the `gFileResourceMethods` method
table as splat has that table carved (see `include/GameApplicationFileResource.h`), which may
mean the table's boundary was drawn one word short and `gDataDirectory` actually
points at the start of a separate, still-unidentified global — not resolved
here.

## Residue

Retail: `sw $a0, %gp_rel(gDataDirectory)($gp)` — one instruction. Compiling
`extern void *gDataDirectory; void SetDataDirectory(void *value) { gDataDirectory = value; }`
under this project's pinned `-G0` produces the two-instruction absolute
`lui`/`sw` form instead — same root cause as `LockActiveDataSource` (full isolated
reproducer there): cc1's own `-G` value gates whether it emits the
`.sdata`/`.extern NAME,SIZE` hint that gp-relative addressing needs, and this
project pins `-G0` for `CC_FLAGS`.

Because the size differs from retail's, this also shifts every function
after it in the file — this was caught before it corrupted the surrounding
functions' addresses (funcdiff's "differs OUTSIDE this range" warning fired
immediately), and reverted before build-and-verify was trusted for anything
else in the unit.

## Preserved body

```c
extern void *gDataDirectory;

void SetDataDirectory(void *value) {
    gDataDirectory = value;
}
```

## Proposed learning

See `docs/match-reports/LockActiveDataSource.md` for the general finding (`.sdata`
+ pinned `-G0` blocks any C-level access to a real small-data global). Worth
adding here specifically: **a stalled `%gp_rel` function's wrong instruction
count doesn't just fail its own diff — it shifts every later function in the
same translation unit**, so `funcdiff`'s "differs OUTSIDE this range" warning
on an otherwise-plausible-looking function is a strong signal to check
earlier functions in the same file for a still-unresolved size mismatch
before trusting anything downstream.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

Not renamed. `SetDataDirectory` is a setter (`gDataDirectory = value;`) and
`GetDataDirectory` its getter (`return gDataDirectory;`). `gDataDirectory` is a real
`.sdata` global initialized to `0x8006D4A8`, which sits at (or just past)
the tail of the gFileResourceMethods method table as splat has it carved -- possibly
meaning the table boundary is one word short and this actually points at a
separate, unidentified global. No caller of either function was found
anywhere in the tree, so there is no usage evidence to lean on either. A
wrong tier-A/B guess here (e.g. asserting this is some kind of shared-vtable
pointer cache) would be worse than the placeholder. Kept as
`SetDataDirectory`/`GetDataDirectory`; write down what is known and revisit once
`D_8006D4A8` or a caller is identified.

## Naming (round 99, charlie, track 7)

This supersedes the round-52 "not renamed" entry above. The rename tools
rewrote the names in that section, so it now reads oddly: it describes the
state before these renames.

| was | now | tier |
| --- | --- | --- |
| `func_800270AC` | `SetDataDirectory` | A |
| `func_800270B8` | `GetDataDirectory` | A |
| `D_8008A854` | `gDataDirectory` | A |

**Evidence.** Round 52 found no caller. There are three now, and they agree:

- **Readers.** `BuildCdFilePath` (CdDriver) and `CdStream__Open`
  (src/cd/CdStream.c) both build `"\\" + GetDataDirectory() + name + ";1"` and
  pass the result to the CD file lookup. So the value is the directory
  part of an ISO9660 path, and it sits between the root `\` and the file
  name.
- **Writer.** `GameApplication__GameApplication` (GameApplicationFileResource) calls
  `SetDataDirectory(GetDefaultDataDirectory())` once at startup. `GetDefaultDataDirectory`
  returns `gDefaultDataDirectory`, whose retail initialiser is `&D_8008A958`, the
  `.sdata` string `"CDI\\"`. That string ends in the separator, which is
  what the readers need, because they put nothing between it and the name.
- **Default.** `gDataDirectory`'s retail initialiser is `0x8006D4A8`. The
  byte there is `0x00`, so the value is an empty string and paths resolve
  from the disc root until the application installs `"CDI\\"`. That word is
  also the last word of `gFileResourceMethods` as splat has that table
  carved, which is what made round 52 suspicious. It is a `""` literal that
  the table's carve swallowed, not a table slot.

The pair is a plain setter and getter, which is tier A by definition, and
the readers agree on what the value is for. `char *` is the true type. The
unit's own declarations say `char *` since round 99. GameApplicationFileResource still
declares `SetDataDirectory(s32)` and CdStream.c declares
`void *GetDataDirectory(void)`. Both are left to their owners and proposed.
