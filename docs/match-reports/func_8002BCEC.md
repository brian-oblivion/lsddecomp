# func_8002BCEC — STALL (best compiled 172/175, 3 words SHORT; raw word-match 49/175 under that drift; first real diff at vram 0x8002BDB8 / file 0x1C5B8)

`code_179d8_d`, vram `0x8002BCEC`, file offset `0x1C4EC`, 175 instructions
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
   iteration, up to 0x40 entries or until the cursor reaches `D_8008D7F0`
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
 * idiom code_179d8_g.c's own `UWord` type uses for the identical purpose
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

extern u8 D_8008CFF0[]; /* PVD/dir-listing buffer -- code_179d8_g.c's own
                         * comment on this symbol */
extern u8 D_8008D7F0[]; /* upper-bound sentinel on the scan cursor --
                         * address-only use, per code_179d8_g.c's comment */

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
extern void func_80012C20(const char *fmt, ...); /* Psy-Q printf wrapper */

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
            func_80012C20(D_80010C58);
        }
        return -1;
    }

    count = 0;
    if (D_8006D608 >= 2) {
        func_80012C20(D_80010C78);
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
            func_80012C20(D_80010C9C, *((u8 *)D_8008B3F0 + off),
                          *((u8 *)D_8008B3F0 + off + 1),
                          *((u8 *)D_8008B3F0 + off + 2),
                          *(s32 *)((u8 *)D_8008B3F4 + off), (char *)name);
        }

        name += 0x18;
        count++;
        rec = (IsoDirRecord *)((u8 *)rec + rec->len);
        off += 0x18;
        if (count >= 0x40 || (u8 *)rec >= D_8008D7F0) {
            break;
        }
    }

    D_8006D938 = id;
    if (count < 0x40) {
        D_8008B3F0[count].name[0] = 0;
    }
    if (D_8006D608 >= 2) {
        func_80012C20(D_80010CB8, count);
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
immediates). Every combination tried (association order `off+4` vs `4+off`,
raw pointer arithmetic vs struct field access, with and without the
block-scoping) reproduces exactly one of these two outcomes and never
both-correct-at-once. This is a genuine two-word residue, register-identity
class, not reachable by further reshaping in the time available — CLAUDE.md
files this as a legitimate stall condition (same instructions/values,
different encoding choice the compiler makes internally).

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
`docs/match-reports/func_8002B94C.md`** (`code_179d8_g`, this function's
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

Roughly 10 build/pipeline iterations (well under the 30-attempt cap).
Axes varied: loop-guard spelling (bare `while(1)`, `if(ok==1)`,
`while(ok==1)`, `for(;;)`), the size-write's pointer-vs-field spelling and
association order, the debug-print's `+4` read decoupled into its own
extern symbol (`D_8008B3F4`) to prevent an unwanted CSE with the write,
and declaration/initialization order of `count`/`slot`/`name`. Both
residues are now well-isolated and cheaply reproducible; a second pass
should start from the two named residues above rather than re-deriving the
algorithm (which is solid).

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
