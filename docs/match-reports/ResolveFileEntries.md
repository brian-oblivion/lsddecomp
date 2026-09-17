> Renamed from `func_80027FFC` on 2026-09-17 (tools/rename.py). Address 0x80027ffc.

# ResolveFileEntries — MATCHED (53/53 words)

Round 45, runner echo (third sitting), `src/code_179d8_q.c`. Last remaining
function in this unit.

## Result

Byte-exact, third attempt (two intermediate near-misses, see below).

```c
/* A 4-byte, alignment-2 pair -- the idiom CLAUDE.md/code_179d8_h.c document
 * for a struct whose whole-struct assignment compiles to lwl/lwr + swl/swr
 * instead of a plain lw/sw. Kept as this unit's own local view (per-call-site
 * typed, same shape as code_179d8_h.c's Pair16_179D8H, different name so
 * nothing is shared across units). */
typedef struct Pair16Q Pair16Q;
struct Pair16Q {
    s16 unk0;
    s16 unk2;
};

/* CdSearchFile's own output buffer. Only the first two fields this call
 * site copies out are named; sized to 0x18 bytes total because that is
 * exactly the span between this local's stack slot (sp+0x50) and the next
 * saved register (sp+0x68) -- independently confirms the same 0x18-byte
 * figure code_179d8_h.c's func_80028920 derived for the same Sony
 * function's output struct (StatBuf179D8H). */
typedef struct CdStatBufQ CdStatBufQ;
struct CdStatBufQ {
    Pair16Q unk0;
    u32 unk4;
    u8 pad8[0x18 - 0x8];
};

/* This class's per-entry array element, 0x1C bytes: `name` is passed
 * directly (as its own address, offset 0) to func_800289CC as the path
 * suffix; unk14/unk18 are filled from a CdSearchFile lookup on that path.
 * gFileTable (this unit's own SetFileTable/set) and gFileTableCount (func_8002
 * 7FE4/FF0) are this array's base pointer and element count -- func_800284C4
 * (code_179d8_r) walks the identical 0x1C stride over gFileTable doing
 * strstr() against `name`, confirming the layout independently. */
typedef struct FileEntryQ FileEntryQ;
struct FileEntryQ {
    /* +0x00 */ char name[0x14];
    /* +0x14 */ Pair16Q unk14;
    /* +0x18 */ u32 unk18;
};

extern const char sFileNotFoundMsg[]; /* "File not found. file = %s\n" */
extern s32 CdSearchFile(CdStatBufQ *statBuf, char *path); /* lib/libcd/iso9660.o */
extern void printf(const char *fmt, void *arg1);
extern char *func_800289CC(char *dest, char *suffix);
extern void InitCdDrive(void);

s32 ResolveFileEntries(FileEntryQ *arg0, s32 count)
{
    FileEntryQ *end;
    char path[0x40];
    CdStatBufQ buf;
    s32 tries;

    end = arg0 + count;

    InitCdDrive();

    for (; arg0 < end; arg0++) {
        func_800289CC(path, arg0->name);

        for (tries = 0; tries < 0x65; tries++) {
            if (CdSearchFile(&buf, path) != 0) {
                goto found;
            }
        }

        printf(sFileNotFoundMsg, path);

    found:
        arg0->unk14 = buf.unk0;
        arg0->unk18 = buf.unk4;
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
