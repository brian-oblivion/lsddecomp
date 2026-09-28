# func_8002BCEC -- CONVERTED to a linked SDK object (round 34). NOT game code, NOT a stall.

> **ROUND 34 (2026-09-12), head. THIS FUNCTION IS NOW LINKED FROM SONY'S OWN
> OBJECT `libcd/iso9660.o` (Psy-Q 3.3) AND IS `CD_cachefile`.** It was
> an INCLUDE_ASM stall; the object owns its bytes, so the C is gone from
> `src/` and the game-code count shrank by it -- the correction CLAUDE.md asks
> for, not a regression. The run `libcd/iso9660` + `libc2/strcmp` +
> `libc2/strncmp` tiles 0x1BE40..0x1C92C and crosses the libcd_bios /
> PlacementGridVabSound boundary; both units trimmed. Whole-image SHA1 green. Nothing
> here is assignable and there is no stall left to work. The text below is the
> pre-conversion record.

## Original report

# func_8002BCEC — STALL (best compiled 172/175, 3 words SHORT; raw word-match 49/175 under that drift; first real diff at vram 0x8002BDB8 / file 0x1C5B8)

> **ROUND 32 (2026-09-12), head. THIS FUNCTION IS SONY LIBRARY CODE AND
> CANNOT BE MATCHED BY WRITING C. DO NOT STAFF A RUNNER ONTO IT.**
>
> `func_8002BCEC` (0x8002bcec, 175 words) lies FULLY inside **`libcd/iso9660.o`** (Psy-Q Psy-Q 3.3),
> an object already placed in `config/psyq-objects.txt` and verified against
> retail by relocation-masked exact match. Its bytes come from Sony's object,
> not from anything cc1 will produce from a game source file, so no source
> shape reaches a match and every attempt is spent for certain.
>
> The correct disposition is CONVERSION, not decompilation: see
> `docs/SDK-OBJECTS-GUIDE.md`. Detect the whole class with
> `python3 tools/sdkstalls.py`.
>
> **The analysis below is not wrong, it is aimed at the wrong target**, and
> it is kept because the reading of what the routine DOES is still accurate
> and still useful when the object is placed. Only the premise that it is
> game code is retracted.

**Round 27 update (post head review):** residue 1's original verdict —
filed below as "register-identity, not reachable" — was WRONG and has been
corrected. A differing register COUNT is not register identity (same
allocation, different names); it is allocation pressure, and the project
has a documented lever for that (a fresh named local competing for a
callee-saved register — `DECOMPILATION_LEARNINGS.md`, "a fresh local that
only carries one branch's result... is a register-identity risk"). Six
variants were tried post-review, including the head's own suggested
inline-cast-expression form and a fully-dead-parameter check; none closes
it, and the corrected finding is below under "Residue 1, corrected". The
2-word gap is real and unresolved, but it is a REGISTER-COUNT/instruction-
selection trade-off, not identity, and the next attempt should not be told
"do not try" on this one.

`PlacementGridVabSound`, vram `0x8002BCEC`, file offset `0x1C4EC`, 175 instructions
(0x2BC bytes). Blocker-clean per the carve census (no `gp_rel`, no forward
`mflo`/`mfhi`-then-`mult`/`div`). FRESH ground, no report existed before this
round, no prior attempt history to inherit.

## What it computes: `CD_cachefile`'s directory-cache refresh

Confirmed by the rodata strings (`asm/data/120C.rodata.s`): `"CD_cachefile:
dir not found\n"`, `"CD_cachefile: searching...\n"`, `"\t(%02x:%02x:%02x)
%8d %s\n"`, `"CD_cachefile: %d files found\n"`, plus two literal directory
pseudo-entries at `D_80010C94`/`D_80010C98`/`D_80010C9A` that decode to
`.` (`0x002E`) and `..` (`0x2E2E` + a `0x00` terminator byte). This is the
companion to `func_8002B94C`'s already-documented `CD_newmedia` (see its own
match report, `docs/match-reports/func_8002B94C.md`) — that function walks
the ROOT directory into `D_8008B9F4[]`; this one takes an `id` (a 1-based
index into a DIFFERENT lookup, `D_8008B9CC[]`, resolving to a directory's
own read-handle), re-reads THAT directory's sector via the already-matched
`func_8002BFA8`, and walks ITS directory-record chain into a SEPARATE
0x18-stride cache table, `D_8008B3F0[]`.

1. Short-circuits to `return 1` if `id` already equals the last id cached
   (`D_8006D938`, a one-entry "which directory is loaded" memo — the same
   symbol `func_8002B94C` clears to 0 on ITS OWN successful load, confirming
   the two functions share this memo).
2. `func_8002BFA8((void *)1, D_8008B9CC[id].handle, D_8008CFF0)` re-reads the
   directory's sector into the shared `D_8008CFF0` scratch buffer (the SAME
   buffer `func_8002B94C`/`func_8002BFA8` use); must return 1 or the function
   prints and returns -1.
3. Walks the ISO9660 directory-record chain at `D_8008CFF0`, one record per
   iteration, up to 0x40 entries or until the cursor reaches `_svm_sreg_buf`
   (the same address-only sentinel `func_8002B94C` uses). Record layout
   matches the ISO9660 standard exactly (length @0, ext-attr length @1,
   LE location-of-extent @2 unaligned, LE data-length @0xA unaligned,
   file-identifier length @0x20, file identifier @0x21) — independent
   confirmation the earlier `func_8002B94C` derivation is right, and that
   this is the SAME record shape read from a DIFFERENT starting sector.
4. Per record: hands the raw (unconverted) extent-LBA to `func_800292F4`
   (uncarved elsewhere; presumably an LBA-to-MSF conversion, since the
   per-entry debug print's `%02x:%02x:%02x` format is exactly an MSF
   triple), copies the data-length verbatim into the cache entry's own
   size field, and fills the entry's 0x10-byte name buffer one of three
   ways: entry 0 gets the literal `.` constant, entry 1 gets the literal
   `..` constant + terminator, and every entry from 2 on gets the record's
   own file identifier copied via the already-matched `func_8002C014`
   and NUL-terminated.
5. After the loop: caches `id` into `D_8006D938`, zero-terminates the name
   field of the cache slot ONE PAST the last written entry (mirrors
   `func_8002BC40`'s `.id == 0` empty-slot convention, just on a different
   field of a different table), and returns 1.

Every diagnostic print is gated behind `D_8006D608` — 0 silent, >0 errors,
>=2 per-entry trace — same convention as `func_8002B94C`/`func_8002B640`.

## Best-derived body (compiles to 172/175 words, 3 words SHORT; 49/175 raw
match under that drift; structurally correct almost everywhere once
realigned — see below)

```c
/* A 4-byte, alignment-1 view used only to force the unaligned lwl/lwr +
 * swl/swr load/store shape two of IsoDirRecord's fields need -- the same
 * idiom libcd_bios.c's own `UWord` type uses for the identical purpose
 * (all-u8 members so the struct's own alignment is 1, forcing GCC to use
 * an unaligned move rather than assuming a 4-byte-aligned `lw`/`sw`). */
typedef struct UWord {
    u8 b0, b1, b2, b3;
} UWord;

/* One ISO9660 directory record from the D_8008CFF0 buffer. Only the
 * fields func_8002BCEC itself reads are named. */
typedef struct IsoDirRecord {
    u8 len;          /* +0x00, length of directory record -- also this
                      * function's own "next record" advance and its
                      * end-of-listing test (0 means no more records) */
    u8 extAttrLen;   /* +0x01, unused here */
    UWord extentLBA; /* +0x02, location of extent (LE), unaligned --
                      * handed to func_800292F4 for conversion */
    u8 pad06[0x0A - 0x06];
    UWord dataLen;   /* +0x0A, data length (LE), unaligned -- copied
                      * verbatim into the cache entry's own size field */
    u8 pad0E[0x20 - 0x0E];
    u8 nameLen;      /* +0x20, length of file identifier */
    char name[1];    /* +0x21, file identifier, nameLen bytes, not
                      * NUL-terminated in the record itself */
} IsoDirRecord;

/* The 0x18-stride cache entry this function builds, one per IsoDirRecord,
 * up to 0x40 of them. Only the fields this function itself touches are
 * named. */
typedef struct EntryB3F0 {
    u8 msf[3];   /* +0x00, filled by func_800292F4 from extentLBA, not
                  * written directly here */
    u8 pad3;
    UWord size;  /* +0x04, copied verbatim from IsoDirRecord::dataLen */
    char name[0x18 - 0x08]; /* +0x08 */
} EntryB3F0;

extern EntryB3F0 D_8008B3F0[0x40];

/* An INDEPENDENT extern for D_8008B3F0's own +0x4 field (D_8008B3F0 == this
 * symbol - 4), used ONLY by the per-iteration diagnostic print's byte-offset
 * read below. Retail computes that read via a FRESH lui/addiu of this exact
 * symbol, not by adding 4 to the live D_8008B3F0 base register the write two
 * lines above also uses -- if the read is written through the same
 * `(u8*)D_8008B3F0 + off + 4` expression as the write, GCC hoists a THIRD
 * induction register shared between them (confirmed: without this split the
 * build saves 8 callee registers instead of retail's 7). Declaring the
 * read's target as its own symbol denies the compiler the syntactic link. */
extern s32 D_8008B3F4[];

extern u8 D_8008CFF0[]; /* PVD/dir-listing buffer -- libcd_bios.c's own
                         * comment on this symbol */
extern u8 _svm_sreg_buf[]; /* upper-bound sentinel on the scan cursor --
                         * address-only use, per libcd_bios.c's comment */

/* This function's own "id -> handle" lookup, a DIFFERENT 0x2C-stride table
 * from this file's own Entry8008B9F4 -- D_8008B9CC is not a multiple of
 * 0x2C away from D_8008B9F4, so it is not the same array under a different
 * index origin. Only the field this function reads is named. */
typedef struct EntryB9CC {
    void *handle;
    u8 pad4[0x2C - 4];
} EntryB9CC;
extern EntryB9CC D_8008B9CC[];

extern s32 D_8006D608;
extern s32 D_8006D938;

/* CD_cachefile diagnostics (confirmed via asm/data/120C.rodata.s) */
extern u8 D_80010C58[]; /* "CD_cachefile: dir not found\n" */
extern u8 D_80010C78[]; /* "CD_cachefile: searching...\n" */
extern u16 D_80010C94;  /* 0x002E -- ".", NUL-terminated, packed as a u16 */
extern s16 D_80010C98;  /* 0x2E2E -- "..", first two chars packed as a s16 */
extern s8 D_80010C9A;   /* 0x00 -- "..", NUL terminator */
extern u8 D_80010C9C[]; /* "\t(%02x:%02x:%02x) %8d %s\n" */
extern u8 D_80010CB8[]; /* "CD_cachefile: %d files found\n" */

extern s32 func_8002BFA8(void *p0, void *p1, void *p2);        /* matched, this unit */
extern void func_8002C014(char *dest, char *src, s32 count);   /* matched, this unit */
extern void func_800292F4(void *arg0, s32 *outBuf);
extern void printf(const char *fmt, ...); /* Psy-Q printf wrapper */

s32 func_8002BCEC(s32 id)
{
    IsoDirRecord *rec;
    EntryB3F0 *slot; /* the current cache entry -- only ever used as a
                      * pointer VALUE (func_800292F4's outBuf argument), so
                      * it earns its own strength-reduced register rather
                      * than being re-derived from `off` each time. */
    u8 *name;         /* &current entry's name[0] -- likewise only ever used
                      * as a pointer value (func_8002C014's dest, the %s
                      * argument, and the direct index/index-1 writes). */
    s32 off;           /* running BYTE offset of the current entry within
                      * D_8008B3F0, used for every access that is NOT
                      * itself passed on as a pointer value. */
    s32 count;

    if (id == D_8006D938) {
        return 1;
    }

    if (func_8002BFA8((void *)1, D_8008B9CC[id].handle, D_8008CFF0) != 1) {
        if (D_8006D608 > 0) {
            printf(D_80010C58);
        }
        return -1;
    }

    count = 0;
    if (D_8006D608 >= 2) {
        printf(D_80010C78);
    }

    slot = D_8008B3F0;
    name = (u8 *)slot + 8;
    off = 0;
    rec = (IsoDirRecord *)D_8008CFF0;
    for (;;) {
        if (rec->len == 0) {
            break;
        }

        {
            UWord tmp = rec->extentLBA;
            func_800292F4(*(void **)&tmp, (s32 *)slot);
        }
        {
            EntryB3F0 *entry = (EntryB3F0 *)((u8 *)D_8008B3F0 + off);
            entry->size = rec->dataLen;
        }

        if (count == 0) {
            *(u16 *)D_8008B3F0[0].name = D_80010C94;
        } else if (count == 1) {
            *(s16 *)D_8008B3F0[1].name = D_80010C98;
            D_8008B3F0[1].name[2] = D_80010C9A;
        } else {
            func_8002C014((char *)name, rec->name, rec->nameLen);
            name[rec->nameLen] = 0;
        }

        slot++;
        if (D_8006D608 >= 2) {
            printf(D_80010C9C, *((u8 *)D_8008B3F0 + off),
                          *((u8 *)D_8008B3F0 + off + 1),
                          *((u8 *)D_8008B3F0 + off + 2),
                          *(s32 *)((u8 *)D_8008B3F4 + off), (char *)name);
        }

        name += 0x18;
        count++;
        rec = (IsoDirRecord *)((u8 *)rec + rec->len);
        off += 0x18;
        if (count >= 0x40 || (u8 *)rec >= _svm_sreg_buf) {
            break;
        }
    }

    D_8006D938 = id;
    if (count < 0x40) {
        D_8008B3F0[count].name[0] = 0;
    }
    if (D_8006D608 >= 2) {
        printf(D_80010CB8, count);
    }
    return 1;
}
```

Self-tested: splicing this body back in place of the `INCLUDE_ASM` and
running `./build-and-verify.sh` gives `build exit=2` with no compile-error
grep hits and `funcdiff.py` reporting the length/word-match figures in the
title (no INCLUDE_ASM warning, no stale-mtime warning). It is NOT
byte-exact and was restored to `INCLUDE_ASM` per the hard rule.

## What is solid

Once the residues below are accounted for, the body matches retail almost
instruction-for-instruction (verified with `tools/asm-differ/diff.py`,
which realigns past the 3-word gap): the whole record-scan structure, the
`i*0x2C`-style table-lookup multiply for `D_8008B9CC[id]`, both unaligned
4-byte copies (the `UWord` idiom, confirmed AGAIN to generalize — third
function project-wide after `func_8002B94C`'s two), the 3-way
count-dispatch (literal `.`/`..`/copied-name), the debug-print argument
list, and — after two register-allocation fixes below — the exact
callee-saved register SET (`s0`-`s6`, matching retail's 7, frame `-0x40`
matching exactly).

## Two distinct residues, worked hard, neither fully closed

### Residue 1 (CLOSED): the size-field write's own extra induction register

**Symptom.** A direct `*(UWord *)((u8*)D_8008B3F0 + off + 4) = rec->dataLen;`
— the obvious spelling of "copy the data-length into the cache entry's
`+4` field" — compiles with **8** callee-saved registers (`s0`-`s8`) against
retail's **7**, adding 2 words (frame `-0x48` vs `-0x40`) that ripple through
the entire rest of the image (funcdiff's drift warning fires: ~299450
bytes differ outside the function's own window).

**Cause, isolated.** GCC hoists `(u8*)D_8008B3F0 + off + 4` into ITS OWN
loop-carried induction register (stride `0x18`, alongside `off` itself)
because it is referenced from an UNCONDITIONAL statement (the write runs
every iteration) — the SAME symbol's `+0`/`+1`/`+2` byte reads do NOT get
this treatment because they only execute inside the `D_8006D608 >= 2`
guard. Retail's own instructions confirm it does NOT do this: the write's
address is recomputed FRESH every iteration via a scratch register
(`addiu v0,s5,4` / `addu v0,s1,v0`), not maintained.

**Fix found:** wrap the write in a block-scoped `EntryB3F0 *entry = (...)`
whose SCOPE ends immediately after one use, and reach the `+4` field via
`entry->size = ...` (struct field access) rather than a raw
`(u8*)base+off+4` cast. This alone drops the register count back to 7 and
the frame back to `-0x40`, with the LENGTH matching retail exactly (this is
where the "no drift" figure below comes from before the check residue
reopens a gap of its own).

**New sub-residue, NOT closed:** with this fix, the resulting `swl`/`swr`
immediates are `7`/`4` where retail has `3`/`0` — i.e. GCC bakes retail's
"+4" into the ADD when the pointer targets the FIELD directly (`UWord
*sizeField = (UWord*)(base+off+4)`, correct immediates, but the OLD 8-register
problem returns), and bakes it into the STORE immediate when the pointer
targets the STRUCT BASE (`entry->size`, correct 7-register count, wrong
immediates).

### Residue 1, corrected (round 27, post head review): a register-COUNT trade-off, NOT register identity

The head correctly rejected the paragraph above's classification: a
DIFFERING REGISTER COUNT (7 vs 8) is allocation pressure, not identity
(identity is the SAME allocation under different names, which is banned to
fix; a count difference has a documented lever —
`DECOMPILATION_LEARNINGS.md`'s "a fresh local... is a register-identity
risk, reuse a dead PARAMETER instead"). Six variants were tried, all
measured on the same build/diff pair:

| destination spelling | source spelling | registers | `swl`/`swr` immediates |
| --- | --- | --- | --- |
| `entry->size` (new local, struct-base ptr) | `rec->dataLen` | 7 (matches) | `7`/`4` (wrong) |
| `entry->size` (new local) | `*(UWord*)((u8*)rec+0xA)` (raw cast) | 7 (matches) | `7`/`4` (wrong) |
| `*(UWord*)((u8*)D_8008B3F0+off+4)` — head's exact suggested inline-cast, NO named pointer anywhere | `*(UWord*)((u8*)rec+0xA)` (raw cast) | 8 (extra) | `3`/`0` (matches) |
| `UWord *sizeField = ...; *sizeField = ...` (new local, field ptr) | `rec->dataLen` | 8 (extra) | `3`/`0` (matches) |
| `slot->size` — REUSING the already-live `slot` (no new name at all) | `rec->dataLen` | 7 (matches) | `7`/`4` (wrong) |
| `*(UWord*)((u8*)slot+4)` — same reused `slot`, raw-cast form | `rec->dataLen` | 7 (matches) | `7`/`4` (wrong) — **byte-identical output to the row above**, proving struct-field-access vs. raw-pointer-arithmetic syntax makes NO difference once the base is the same register |
| `EntryB3F0 *base = D_8008B3F0;` (new UNCHANGING local, separate from `slot`) then `*(UWord*)((u8*)base+off+4)` | `rec->dataLen` | 8 (extra) | `3`/`0` (matches) |

**The head's own suggested form (row 3) was tried exactly as specified and
reproduces the SAME 8-register/correct-immediate outcome as the
already-tried `sizeField` variant** — it is not the untried third
combination it was hoped to be; the "no named pointer" property does not
matter here (row 6 proves this directly: the SAME address computed through
an EXISTING, already-live pointer variable, with zero new names, still
gets the "wrong" 7/4 immediates — so introducing a name is not what causes
the wrong immediates, and removing one is not what fixes them).

**What the six rows actually show:** the register count and the
`swl`/`swr` immediate split are the SAME axis, not two independent knobs.
Whenever the compiled address for the store is "the base register (however
obtained) plus a small constant, materialized fresh via one extra
instruction" — matching retail's own `addiu v0,s5,4` / `addu v0,s1,v0` — no
variant tried reaches this. Every spelling that keeps the register count
at 7 does so by folding `+4` into the STORE's own immediate field
(reusing an existing base register with no extra `addiu`), and every
spelling that produces the correct `3`/`0` immediates does so by
computing a genuinely new address value that needs its own register.
Retail's shape — extra `addiu` instruction, no extra persistent register —
was not reached by any of the six.

**Dead-parameter lever checked and does not apply:** `func_8002BCEC` has
exactly one parameter, `id`, and it is not dead — it is read at the very
top (`id == D_8006D938`), used for the `D_8008B9CC[id]` lookup, and
written back to `D_8006D938` at the end. There is no spare dead parameter
to reuse as an allocation-neutral carrier, so that specific lever from
`DECOMPILATION_LEARNINGS.md` does not have a target here.

**Corrected verdict:** this is a genuine, well-measured 2-word residue in
the register-COUNT/instruction-selection family — not register identity,
and not something CLAUDE.md's identity ban applies to. It remains OPEN. A
future attempt should look for a THIRD address-computation shape neither
"new register, right immediate" nor "existing register, wrong immediate"
produces — possibly by finding what makes retail's compiler choose the
`addiu`-into-scratch-register form specifically, which none of the six
tried source expressions triggered.

### Residue 2 (OPEN, matches an ALREADY-DOCUMENTED sibling stall): the missing 3-instruction "always-true" check

**Symptom.** Immediately before the loop's own setup (right after the
`D_8006D608 >= 2` "searching..." print), retail has:

```
ori   v0, zero, 1
beqz  v0, <loop's own exit label>
nop
```

— a check that is PROVABLY always false (never taken) from everything
visible in the surrounding code, yet retail keeps it. This function's
compiled body is otherwise the correct length; omitting this specific
3-instruction sequence is the WHOLE of the 3-word shortfall.

**This is the identical class already investigated and left OPEN in
`docs/match-reports/func_8002B94C.md`** (`libcd_bios`, this function's
own closest sibling — both are `CD_*` directory-scan functions built from
the same `func_8002BFA8`/`D_8008CFF0` plumbing). That report's own words:
*"GCC's own dead-code elimination at -O2 will remove a directly-reproduced
`if (constant)` check regardless of barriers... If retail keeps such a
check, the original source expression must not have been immediately
foldable to a literal by this compiler."*

**Tried here, all eliminated by DCE exactly as that report predicts:**
- A bare `while (1) { ... }` wrapping the loop setup — GCC removed the
  check entirely (no `li`/`beqz` at all), rather than keeping a
  false-but-present one.
- `ok = func_8002BFA8(...); if (ok != 1) return ...; ...; if (ok == 1) { ... }`
  — reusing the call's own return value as the tested variable — also
  fully eliminated (ok is a local not touched by the intervening debug-print
  call, so GCC proves it is still 1 and drops the branch).
- `while (ok == 1) { ... }` (loop-carried, re-tested every iteration
  instead of once) DOES keep a check, but pays for it with `ok` becoming
  an 8th persistent callee-saved register — worse than the target, and
  structurally a per-ITERATION test, not retail's single pre-loop test.

**No attempt reproduced a ONE-SHOT, DCE-surviving check.** Per the sibling
report's own conclusion, this needs a source expression that is NOT a
plain `if (constant-or-locally-known-value)` — something not staticly
resolvable from the two adjacent lines the check sits between. That
condition was not found in the time available here either.

**Round 27 follow-up: the head's two suggested non-foldable sources
(global, or call-result) tried and structurally ruled out.**

- **Global-sourced condition, tested:** `if (D_8006D608 >= -1) { <loop> }`
  (reading the already-in-scope verbosity global again, compared against a
  constant GCC cannot fold since it does not know the global's runtime
  value). This DOES survive — confirming the mechanism the sibling report
  proposes genuinely prevents DCE — but it compiles to a REAL `lw` +
  `slti` + `bnez` sequence, nothing like retail's `li`/`beqz`/`nop`. Reverted
  (does not help; recorded as a checked negative).
- **Call-sourced condition:** not tried, because there is no room for it.
  Retail's own instructions between the "searching..." print and the loop
  setup are EXACTLY three: `ori $v0,$zero,1` / `beqz $v0,<exit>` / `nop` —
  no `lw`, no `lbu`/`lhu`, and no `jal` appears anywhere in that span. Any
  source expression that reads a global or calls a function would need to
  emit a LOAD or a CALL instruction somewhere in this exact 3-instruction
  window, and none exists in retail's disassembly. **This rules out both
  suggested levers structurally, not just empirically** — whatever retail's
  source expression is, its compiled form contains no memory access and no
  call, meaning the value tested is provably a compile-time literal to
  RETAIL's OWN compiler too. The puzzle is not "what unfoldable expression
  produces this" (nothing unfoldable fits in 3 instructions with no load);
  it is "why did retail's compiler run its dead-branch-elimination pass at
  a different point/not at all here", which per `func_8002B94C`'s own
  report is a toolchain-behavior question, not a source-reshaping one.

**Common structural feature of both confirmed instances (asked for by the
head):** both `func_8002BCEC`'s and `func_8002B94C`'s dead checks sit
immediately after a debug-print CALL that is ITSELF conditionally
executed (`if (D_8006D608 >= N) print(...)`), i.e., right at a CONTROL-FLOW
JOIN where two predecessor paths (print-taken, print-skipped) merge before
falling into a loop setup. Neither instance's dead check is reachable from
straight-line code alone — both are the first statement AFTER a branch
merge point. That may be the actual trigger: this compiler's dead-branch
elimination might not look back across a JOIN the way it does within a
single straight-line block, independent of what the condition itself
computes.

## Register-identity residue found and fixed along the way (worth recording)

Two variables' declaration/initialization ORDER matters more than expected:

- Moving `count = 0;` to BEFORE the `D_8006D608 >= 2` print (matching
  retail's own instruction order, where `count`'s zeroing sits in the
  delay slot of the EARLIER debug-branch, not after it) fixed a `count`
  register-identity mismatch (`s1` vs `s2`) that persisted through the
  whole rest of the function.
- `slot = D_8008B3F0; name = (u8 *)slot + 8;` (deriving `name` FROM `slot`)
  reproduces retail's `s4`/`s5` register roles more closely than the
  reverse order (`name` first, `slot` derived from it), though this one
  remains imperfectly resolved — see the diff for the residual `s4`/`s5`
  swap right at the loop's own setup block, which is downstream of and
  possibly entangled with Residue 2's missing instructions.

## Attempts

Roughly 17 build/pipeline iterations across two sessions (well under the
30-attempt cap). Axes varied: loop-guard spelling (bare `while(1)`,
`if(ok==1)`, `while(ok==1)`, `for(;;)`, a global-sourced `if
(D_8006D608>=-1)`), the size-write's pointer-vs-field spelling and
association order (6 variants, see the table above — struct-base vs.
field-pointer, new local vs. reused-existing `slot`, with and without an
UNCHANGING `base` variable separate from the incrementing `slot`), the
debug-print's `+4` read decoupled into its own extern symbol
(`D_8008B3F4`) to prevent an unwanted CSE with the write, and
declaration/initialization order of `count`/`slot`/`name`. Both residues
are now VERY thoroughly isolated (six independent measurements on residue
1 alone) and cheaply reproducible; a third pass should look for a genuinely
new axis — see each residue's own "corrected verdict" / "common structural
feature" for the concrete next things to try — rather than re-deriving the
algorithm (which is solid) or re-trying a spelling already in the tables
above.

### Proposed learning

**A "one-shot true-looking check GCC eliminates" is not unique to
`func_8002B94C`** — it recurs in a second, closely-related `CD_*`
directory-scan function in a DIFFERENT unit, built from the same
`func_8002BFA8` plumbing. That raises this from "one function's odd
residue" to a candidate SHARED IDIOM across this whole family of CD
directory-cache functions, worth checking again on the next one
encountered (`func_8002B640`/`CdSearchFile`, per `func_8002B94C`'s own
report, is a third caller of the same plumbing and has not yet been
attempted).

**A struct-field write whose address can be encoded two equally-valid
ways (fold the small offset into the ADD vs into the STORE immediate)
is a genuine register/encoding-identity residue, not something source
reshaping alone resolves** — worth adding to `DECOMPILATION_LEARNINGS.md`'s
"reading a residue" list as its own bullet if a second instance turns up.

## Head broadcast check (round 27, "parameter survives call" lever)

Checked explicitly: `id` is the only incoming parameter that survives a
call in this function, and it already gets retail's own callee-saved-register
treatment (`s6`, matching exactly, no extra save/restore pair). **The
broadcast's signature (compiled LONG by a callee-saved save/restore pair
retail lacks, around an incoming PARAMETER) does NOT apply here** — this
function's own length/register residues are both around DERIVED local
values (`off`+4, and the derivation-order registers above), not around an
incoming parameter's own storage. Negative result, reported as requested.

## Round 31 update (runner echo) — figures rebuilt, a stale symbol name found and fixed, one more variant tried

Rebuilt the preserved body by splicing it into `src/PlacementGridVabSound.c` (via
`#if 0`/`#endif`) and reproduced the title's figures exactly: `funcdiff.py`
reports **49/175** raw word-match with the drift warning firing (~299428
bytes outside range this round vs. the ~299450 quoted before — the small
difference is consistent with unrelated SDK-linking work landing in between
rounds, not a regression here). `tools/asm-differ/diff.py` confirms the
SAME two residues at the SAME positions: the missing `li v0,1 / beqz
v0,<exit> / nop` (residue 2) immediately before the loop setup, and the
`swl v1,7(v0)`/`swr v1,4(v0)` vs retail's `swl v1,3(v0)`/`swr v1,0(v0)`
size-field-store immediates (residue 1, the "7 registers, wrong immediates"
row of the six-variant table). No new residue found; the report's own
characterization stands.

**Found and fixed: the preserved body's `func_80012C20` declaration is a
STALE SYMBOL NAME that no longer links.** Splicing the report's C exactly
as written now fails at link time —
`undefined reference to 'func_80012C20'` — because the Psy-Q `printf`
object was linked in the interim (rounds 29-30's SDK-objects work; see
`include/code_8220.h`'s own note that the old `func_80012C20` declarations
"moved into src/app/BMemPMgr.c when the SDK objects were linked"). Every other
unit that calls this function now declares it as `printf` directly (see
`src/libcd_bios.c`, `src/CdDriver.c`, `src/TitleMenuTaskObjF.c`,
`src/ScreenWidgets.c`, each with the argument shape their own call site
needs — per-unit local views, not a shared header, matching this project's
convention). Fixed in `src/PlacementGridVabSound.c` by declaring
`extern void printf(const char *fmt, ...);` and renaming all four call
sites in the preserved body from `func_80012C20(...)` to `printf(...)` —
this is a pure symbol-name fix, not a codegen change, and the rebuilt score
is otherwise identical. **The C block below is updated with this fix; any
future attempt splicing this report's body should use `printf`, not
`func_80012C20`, or it will not link.**

**One new residue-1 variant tried, WORSE than the six already in the
table:** indexing the destination via `EntryB3F0 *entry = &D_8008B3F0[count];`
(array-subscript by the already-live `count` variable, distinct from the
byte-offset `off` every other variant used) instead of
`(EntryB3F0 *)((u8 *)D_8008B3F0 + off)`. This compiles clean but drops the
raw word-match to **14/175** (from 49/175) — worse than every row in the
existing table, including the worst-performing "8 registers" variants.
Reverted immediately; not worth a table row of its own beyond this note,
since it did not isolate anything new about the register-count/immediate
trade-off — it just picked a worse overall allocation. The six-variant
table's own conclusion (no variant reaches retail's "fresh `addiu`,
7-register" combination) stands unchanged.

**Verdict: still an open STALL, 49/175 raw match under a 3-word drift —
unchanged in substance, with the stale-symbol fix now folded into the
preserved body for the next attempt.** `INCLUDE_ASM` restored; build green.
