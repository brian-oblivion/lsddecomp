> Renamed from `func_80027FFC` on 2026-09-17 (tools/rename.py). Address 0x80027ffc.

# ResolveFileEntries — MATCHED (53/53 words)

Round 45, runner echo (third sitting), `src/code_179d8_q.c`. Last remaining
function in this unit.

## Result

Byte-exact, third attempt (two intermediate near-misses, see below).

The body below is the round-51 source, after track-3 naming. The derivation
notes that follow were written in round 45 against the same code under its
`unk` names (`Pair16Q`, `CdStatBufQ`, `FileEntryQ`, `arg0`); only names
changed, the image is byte-identical, and the `## Naming` section at the end
carries the evidence for each one.

```c
/* A disc position in the shape Psy-Q's CdlLOC has (minute/second/sector/
 * track), but declared as two s16 rather than four u8: the game's own struct
 * is 2-aligned, which is why a whole-struct assignment of it compiles to
 * lwl/lwr + swl/swr instead of a plain lw/sw (the idiom CLAUDE.md and
 * code_179d8_h.c document). The two halves are never read apart here, so
 * they keep placeholder names. Kept as this unit's own local view, the same
 * shape as code_179d8_h.c's Pair16_179D8H under a different name. */
typedef struct CdLoc16 CdLoc16;
struct CdLoc16 {
    s16 unk0;
    s16 unk2;
};

/* CdSearchFile's output buffer, which is Sony's CdlFILE: pos, size, name[16]
 * = 0x18 bytes (include/psyq/LIBCD.H). The 0x18 was derived here
 * independently, from the span between this local's stack slot (sp+0x50) and
 * the next saved register (sp+0x68), and it is the same figure
 * code_179d8_h.c's func_80028920 derived for the same Sony function. Only
 * the two fields this call site copies out are typed. */
typedef struct CdFileInfo CdFileInfo;
struct CdFileInfo {
    CdLoc16 pos;
    u32 size;
    u8 pad8[0x18 - 0x8];
};

/* One element of the file table: 0x1C bytes of {name, disc position, size}.
 * `name` is passed by its own address (offset 0) to func_800289CC, which
 * builds the full path from it; `pos` and `size` are then filled in from a
 * CdSearchFile lookup on that path, so an entry is a name resolved once and
 * reused as a seek target. gFileTable and gFileTableCount (this unit's
 * SetFileTable / SetFileTableCount) are the array's base and length --
 * FindCdFileIndex (code_179d8_r) walks the identical 0x1C stride over
 * gFileTable doing strstr() against `name`, confirming the layout
 * independently. */
typedef struct CdFileEntry CdFileEntry;
struct CdFileEntry {
    /* +0x00 */ char name[0x14];
    /* +0x14 */ CdLoc16 pos;
    /* +0x18 */ u32 size;
};

extern const char sFileNotFoundMsg[]; /* "File not found. file = %s\n" */
extern s32 CdSearchFile(CdFileInfo *fileInfo, char *path); /* libcd/iso9660.o */
extern void printf(const char *fmt, void *arg1);
extern char *func_800289CC(char *dest, char *suffix); /* code_179d8_r */
extern void InitCdDrive(void);

#define CD_SEARCH_RETRIES 0x65

s32 ResolveFileEntries(CdFileEntry *entries, s32 count)
{
    CdFileEntry *end;
    char path[0x40];
    CdFileInfo info;
    s32 tries;

    end = entries + count;

    InitCdDrive();

    for (; entries < end; entries++) {
        func_800289CC(path, entries->name);

        for (tries = 0; tries < CD_SEARCH_RETRIES; tries++) {
            if (CdSearchFile(&info, path) != 0) {
                goto found;
            }
        }

        printf(sFileNotFoundMsg, path);

    found:
        entries->pos = info.pos;
        entries->size = info.size;
    }

    return 1;
}
```

## Derivation

**The two HARD RULE 6 traps named in the assignment were both real, and both
resolved on the first attempt** — the residue that took two more rounds was
a different, unrelated issue (see below).

1. **`sFileNotFoundMsg` is a rodata string, referenced not retyped.** Grepped
   `asm/data/*.rodata.s` before writing anything: `asm/data/FD8.rodata.s`
   holds `dlabel sFileNotFoundMsg` / `.asciz "File not found. file = %s\n"`. Used
   as `extern const char sFileNotFoundMsg[];` and passed straight to `printf`
   from the first attempt on — no rodata duplication, no whole-image shift.
2. **The unaligned struct copy is the STANDARD alignment-2 idiom (not
   bravo's inverse).** Retail's `lwl $v0,0x53(sp)` / `lwr $v0,0x50(sp)` then
   `swl $v0,-1(s2)` / `swr $v0,-4(s2)` is a 4-byte struct whose two `s16`
   fields get merged into one `lwl`/`lwr` halfword-pair load — exactly the
   case CLAUDE.md documents (all-`s16` struct → alignment 2 → whole-struct
   assignment compiles to `lwl`/`lwr` + `swl`/`swr`), not the byte-by-byte
   case bravo found the inverse for. `code_179d8_h.c` had already named this
   exact shape `Pair16_179D8H` for a different call site on the same
   `CdSearchFile` output struct; this report reuses the same field shape
   under its own per-call-site type name (`Pair16Q`), not the shared header.

**What actually took three attempts was the loop/control-flow shape, in two
separate pieces:**

- **Attempt 1 (5/53, 261406 bytes out-of-range): the inner retry loop's
  "give up after 101 tries" fallthrough needed to be a `goto`, not an
  `if (tries == 0x65)` check after the loop.** Retail reaches the `printf`
  call ONLY by falling out of the inner loop naturally (no branch at all
  between the loop's own exit test and the `printf` setup); the "found"
  path takes a single `bnez` DIRECTLY from right after `CdSearchFile`'s
  return check to the struct-copy code, skipping the `printf` block
  entirely. A `break` statement cannot express this — `break` targets
  "immediately after the loop", which in this shape is where `printf` sits.
  Writing `if (tries == 0x65) printf(...);` after a `break`-using loop
  compiled a real `bne` comparison retail does not have (GCC 2.6.3 did not
  fold the redundant check away, contradicting the hope that it would).
  Switching the found-case to `goto found;`, with the label sitting right
  before the struct copy (past the `printf` statement), reproduced retail's
  shape exactly: the loop's natural exit falls straight into `printf` with
  no test, and the early exit jumps straight past it with no test either —
  because neither path needs to ask a question the C's own structure already
  answers.
- **Attempt 2 (8/53, 305650 bytes out-of-range): one extra `move` from using
  a separate loop-cursor local instead of mutating the parameter.** With
  `FileEntryQ *cur = arg0;` walking the array via `cur`, `end = arg0 + count`
  computed from `arg0` puts `arg0` in one register and then needs a second
  register plus an explicit `move` to seed `cur` for the loop. Retail's `$s1`
  is the SAME register from the first `move s1,a0` all the way through the
  loop increment — i.e. the parameter itself is the loop pointer, mutated in
  place, exactly alpha's broadcast lever ("mutate a parameter in place
  instead of a fresh local when retail reuses the arg's own register").
  Dropping `cur` and writing `for (; arg0 < end; arg0++)` directly against
  the parameter removed the spurious `move` and let the register allocator
  land on `$s1` for everything, matching retail's whole-function allocation.

### Proposed learning

**A "give up after N tries, otherwise proceed" retry loop where the
"proceed" path jumps PAST an intervening statement (not just to the loop's
own exit) needs a `goto`, not a `break` plus a post-loop `if`.** `break`
only ever targets the position immediately following the loop, which is
right for a loop whose exit is the very next statement, but not when there
is a statement — like a "not found" diagnostic print — sitting between the
loop and the code both paths eventually share. GCC 2.6.3 did not fold the
resulting `if (loop-exhausted)` check away even though it is
control-flow-redundant; write the `goto` retail's own branch shape implies
instead of hoping the compiler proves the check unnecessary.

This complements alpha's parameter-mutation lever from earlier this round:
both symptoms (a spurious comparison instruction, a spurious `move`) come
from the same root cause — writing a fresh local where retail's C reused an
existing value/register directly.

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027FFC` | `ResolveFileEntries` | A |
| `D_800107D8` | `sFileNotFoundMsg` | A |

**Evidence.** For each element of the array it is handed, it builds a path
from the element's `name` (`func_800289CC`), retries `CdSearchFile` on that
path up to 101 times, prints `"File not found. file = %s\n"` if all of them
fail, and then copies the search result's position and size back into the
element. Turning names into disc positions is the entire function; tier A.

**Types and fields established here**, all local to this `.c`:

- `CdStatBufQ` -> `CdFileInfo`, fields `unk0`/`unk4` -> `pos`/`size`. The
  0x18-byte buffer is Sony's `CdlFILE` exactly (`include/psyq/LIBCD.H`:
  `CdlLOC pos; u_long size; char name[16];` = 4 + 4 + 16). Round 45 derived
  the 0x18 independently from the stack-slot span; this round matched the
  shape to the declared Sony struct, which also confirms field order. Tier A.
- `Pair16Q` -> `CdLoc16`. The 4-byte, 2-aligned position pair. It has the
  same CONTENT as Sony's `CdlLOC` (minute/second/sector/track) but not the
  same declaration: `CdlLOC` is four `u_char` and would be 1-aligned, while
  retail's `lwl`/`lwr` + `swl`/`swr` copy proves the game's own struct was
  2-aligned, i.e. two 16-bit members. Worth knowing before anyone "fixes" the
  local view by substituting `CdlLOC`: that substitution would change the
  alignment and the copy would stop matching. The two halves keep placeholder
  names -- nothing in this corpus reads them apart.
- `FileEntryQ` -> `CdFileEntry`, `unk14`/`unk18` -> `pos`/`size`. Filled
  straight from `CdFileInfo.pos`/`.size`; `code_179d8_r` confirms the 0x14
  name field independently by `strstr`-ing it. Tier A.
- `CD_SEARCH_RETRIES` for the bare `0x65`.

## Proposed field names

**APPLIED by the head at merge, round 52** -- all four fields, both types
and all five vtable slots below are now in the tree, each one applied
separately with `./build-and-verify.sh` green and byte-exact after it. One
mis-hit had to be resolved by receiver type: `src/code_179d8_h.c:143`
accesses `pendingGeneration` on a `Class6D430 *self`, while the same file's
lines 97/174/175/182 are its OWN `ObjA34_179D8H::unk0C` and were left alone.
The compiler named that mis-hit (`structure has no member named 'unk0C'`),
which is the procedure working in the direction where it can work.

| unit | type | field | proposed | tier | evidence |
| --- | --- | --- | --- | --- | --- |
| code_179d8_s | `Rec80028448` | `unk14` | `pos` | A | same 0x1C record; filled from `CdSearchFile`'s `CdlFILE.pos` |
| code_179d8_s | `Rec80028448` | `unk18` | `size` | A | same record; `func_80027800` divides it by 0x800 to get a sector count |
| code_179d8_s | `StatBuf80027` | `unk0`/`unk4` | `pos`/`size` | A | it is `CdlFILE`; see above |
| code_179d8_h | `StatBuf179D8H` | `unk0`/`unk4` | `pos`/`size` | A | same Sony struct, same call |
