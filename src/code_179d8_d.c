/*
 * code_179d8_d -- window [100..119] of the original 274-function code_179d8
 * monolith, 0x1C440..0x1CC08 (vram 0x8002BC40..0x8002C408).
 *
 * Carved MID-round 16, to re-staff a runner whose own unit was exhausted.
 * Screened 17/20 clean on the three-grep blocker census -- the highest of
 * any window in this monolith -- but that number overstates its value: 9 of
 * the 20 are 4-6 word leaves, and splat matched five of those itself as
 * empty `jr $ra; nop` bodies (func_8002C3C0/C3C8/C3F0/C3F8/C400 below).
 * Those five count as `matched` in tools/progress.py without having been
 * work, which is exactly the caveat CLAUDE.md attaches to that column.
 *
 * NEITHER OF THIS UNIT'S TWO "BLOCKED" FUNCTIONS IS BLOCKED.  Both were
 * filed as `addiu_at`, and `addiu_at` was RESOLVED in round 21 (maspsx
 * `--addiu-at`; docs/research/addiu-at-blocker.md).  Re-screened with
 * `python3 tools/nearmiss.py` on 2026-09-08 (round 24):
 *   func_8002BC40 (43w)   MATCHED round 24, 43/43, first attempt.
 *   func_8002BCEC (175w)  blocker-clean. ROUND 32 CORRECTION: no longer
 *                         cold -- attempted since, 3 words short at
 *                         172/175, full worked report on file. Read it.
 * Their stub reports are gone.  The previous version of this comment listed
 * both as "blocked, have stub reports", which by round 24 was a stale
 * DIRECTIVE over free ground -- the fourth unit in two rounds to carry one.
 * func_8002C278 was originally screened as a third (nop_mflo_mfhi) but
 * that screen was inverted (checked mult/div BEFORE mflo/mfhi instead of
 * after) -- the head corrected it mid-round and deleted the stub report.
 * It is fresh ground; the mult/mfhi pair in its body is retail's signed-
 * divide-by-constant idiom, not the blocked mflo/mfhi-then-mult direction.
 *
 * Unlike its siblings code_179d8_b and code_179d8_c, this slice owns NO
 * jump table -- all seven jtbl blocks in the 0xFD8 rodata slot fall outside
 * 0x8002BC40..0x8002C408 -- so no rodata sub-slot is attached to it.
 *
 * Sibling-slice finding worth having up front (established in code_179d8_b,
 * round 16): this region is NOT class-framework code. tools/classtable.py
 * --scan has no hit anywhere near these globals, and the neighbouring
 * functions read as a low-level serial/link driver poking raw control words
 * into a block of globals that look like hardware/SIO register staging. Do
 * not expect vtables here; do not go looking for a `this` pointer.
 *
 * Declarations: keep anything that encodes THIS unit's reading of the region
 * next to the code, in this file. Do NOT create a shared code_179d8*.h --
 * the sibling slices are staffed independently and a shared header is what
 * makes their merges collide.
 */
#include "common.h"

/*
 * D_8006D940: a function-pointer table this unit's own `new_class_6d940`/
 * `func_8002C18C` dispatch through. Named/typed as a plain local struct,
 * NOT claimed to be a class-framework vtable -- per this unit's header
 * comment (sibling-slice finding: no classtable.py hit anywhere near this
 * region). Only the two slots this unit's own functions reach are typed;
 * the rest stays opaque padding. Kept LOCAL to this file, not a shared
 * header, per this round's rule for code_179d8 slices.
 */
typedef struct Table6D940 Table6D940;
struct Table6D940 {
    u8 pad000[0x008];
    /* +0x008, new_class_6d940's own dispatch -- this IS func_8002C18C
     * itself (same 2-arg (self, arg1) shape). */
    void (*slot08)(void *self, s32 arg1);
    u8 pad00C[0x06C - 0x00C];
    /* +0x06C, func_8002C18C's own conditional dispatch. */
    void (*slot6C)(void *self, s32 arg1);
};
extern Table6D940 D_8006D940;

/* The 0x34-byte object new_class_6d940 allocates. Only the fields
 * func_8002C18C itself touches are named. */
typedef struct Obj6D940 Obj6D940;
struct Obj6D940 {
    Table6D940 *methods; /* +0x000, func_8002C18C */
    u8 pad004[0x02C - 0x004];
    s32 unk2C;            /* +0x02C, func_8002C18C: zeroed */
    s32 unk30;             /* +0x030, func_8002C18C: zeroed */
};

/*
 * A second, DIFFERENT function-pointer table, reached only via the
 * uncarved accessor `func_80026CAC()` (not this unit's function to
 * define). Only the three slots this unit's functions dispatch through
 * are typed.
 */
typedef struct BaseTable6D940 BaseTable6D940;
struct BaseTable6D940 {
    u8 pad000[0x008];
    void (*slot08)(void *self); /* +0x008, func_8002C18C's own base-chain call */
    /* +0x00C, func_8002C200's own dispatch -- that function's whole body
     * is this one call with nothing after it, so its own return type is
     * genuinely ambiguous (a void wrapper around an s32 tail call is
     * byte-identical); typed s32 here per CLAUDE.md's rule to default to
     * `return callee(...)` absent positive void evidence. */
    s32 (*slot0C)(void *self);
    u8 pad010[0x064 - 0x010];
    /* +0x064, func_8002C238's own dispatch -- same tail-call ambiguity as
     * slot0C above. */
    s32 (*slot64)(void *self);
};
extern BaseTable6D940 *func_80026CAC(void);

/* Pool allocator, already established elsewhere (e.g.
 * include/class_16334.h, include/code_8220.h) -- declared LOCAL here since
 * this unit does not include either header. */
extern void *func_80017B34(s32 size);

/* The 0x2C-stride table at D_8008B9F4, this unit's local view.  Both symbols
 * the disassembly names are ONE array: D_8008B9FC is D_8008B9F4 + 8, and both
 * advance by 0x2C per iteration in func_8002BC40 -- GCC builds one induction
 * variable per accessed member, which is why the `.id` access re-materialises
 * the base every iteration (indexed-global form, no live register needed)
 * while the `.name` address is strength-reduced into $s2 (it has to be a real
 * value, it gets passed as an argument).  Field at +4 is not read here. */
typedef struct Entry8008B9F4 {
    /* 0x00 */ s32 id;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ char name[0x24];
} Entry8008B9F4; /* size 0x2C */

extern Entry8008B9F4 D_8008B9F4[0x80];

s32 func_8002BC40(s32 id, char *name)
{
    s32 i;

    for (i = 0; i < 0x80; i++) {
        if (D_8008B9F4[i].id == 0) {
            break;
        }
        if (D_8008B9F4[i].id == id) {
            if (func_8002C048(name, D_8008B9F4[i].name) == 0) {
                return i + 1;
            }
        }
    }
    return -1;
}

/*
 * A 4-byte, alignment-1 view used only to force the unaligned lwl/lwr +
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
extern void printf(const char *fmt, ...); /* Psy-Q printf wrapper */

/* STALL -- see docs/match-reports/func_8002BCEC.md.  Best-derived body
 * compiles to 172/175 words (3 SHORT); raw word-match 49/175 under that
 * drift; first real diff at vram 0x8002BDB8 (file 0x1C5B8).  Two residues:
 * (1) the size-field write's own address-computation encoding (register
 * count vs. store-immediate trade-off, six variants tried, none matches),
 * (2) a missing 3-instruction "always-true" check GCC's own dead-code
 * elimination removes from every C form tried.  Preserved for the next
 * attempt. */
#if 0
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
        if (count >= 0x40 || (u8 *)rec >= D_8008D7F0) {
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
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_d", func_8002BCEC);

/* Cross-unit calls into code_179d8_b -- declared LOCAL to this unit,
 * per-call-site typed, since none of them have an established prototype
 * anywhere yet.  (This comment used to name which runner held code_179d8_b
 * "this round"; a round-specific staffing fact does not belong in a durable
 * file, because it is false from the next round onward.) */
extern void func_800292F4(void *arg0, s32 *outBuf);
extern void func_80028DF0(s32 arg0, s32 *buf, s32 arg2);
extern void func_80029274(void *arg0, void *arg1, s32 arg2);
extern s32 func_80029254(s32 arg0, s32 arg1);

s32 func_8002BFA8(void *p0, void *p1, void *p2)
{
    s32 buf[2];

    func_800292F4(p1, buf);
    func_80028DF0(2, buf, 0);
    func_80029274(p0, p2, 0x80);
    return (u32)func_80029254(0, 0) < 1;
}

void func_8002C014(char *dest, char *src, s32 count)
{
    s32 i;

    for (i = count - 1; i != -1; i--) {
        *dest++ = *src++;
    }
}

s32 func_8002C048(char *s1, char *s2)
{
    char c1;
    char c2;
    s32 eq;

    if (s1 == NULL) {
        goto check_eq;
    }
    if (s2 != NULL) {
        goto loop_start;
    }
check_eq:
    if (s1 != s2) {
        goto not_equal;
    }
return_zero:
    return 0;
not_equal:
    if (s1 == NULL) {
        return -1;
    }
    return 1;

loop_check:
    if (c1 == 0) {
        goto return_zero;
    }
    s1++;
loop_start:
    c1 = *s1;
    c2 = *s2;
    eq = c1 == c2;
    s2++;
    if (eq) {
        goto loop_check;
    }
    return *s1 - *(s2 - 1);
}

s32 func_8002C0AC(char *s1, char *s2, s32 n)
{
    char c1;
    char c2;
    s32 mismatch_flag;

    if (s1 == NULL) {
        goto check_eq;
    }
    if (s2 != NULL) {
        goto loop_entry;
    }
check_eq:
    if (s1 != s2) {
        goto not_equal;
    }
    goto return_zero;
not_equal:
    if (s1 == NULL) {
        return -1;
    }
    return 1;

loop_entry:
    n--;
    if (n < 0) {
        return 0;
    }
loop_top:
    c1 = *s1;
    c2 = *s2;
    mismatch_flag = c1 != c2;
    s2++;
    if (mismatch_flag) {
        goto mismatch;
    }
    if (c1 == 0) {
        goto return_zero;
    }
    s1++;
    n--;
    __asm__("");
    if (n >= 0) {
        goto loop_top;
    }
mismatch:
    if (n < 0) {
        goto return_zero;
    }
    return *s1 - *(s2 - 1);
return_zero:
    return 0;
}

void *new_class_6d940(s32 arg1)
{
    void *self;
    Table6D940 *table;

    self = func_80017B34(0x34);
    if (self != NULL) {
        table = func_8002C3A8();
        table->slot08(self, arg1);
        return self;
    }
    return NULL;
}

void func_8002C18C(Obj6D940 *self, s32 arg1)
{
    func_80026CAC()->slot08(self);
    self->methods = func_8002C3A8();
    self->unk2C = 0;
    self->unk30 = 0;
    if (arg1 != 0) {
        self->methods->slot6C(self, arg1);
    }
}

s32 func_8002C200(void *self)
{
    return func_80026CAC()->slot0C(self);
}

s32 func_8002C238(s32 *self)
{
    self[0xC] = 1;
    return func_80026CAC()->slot64(self);
}

/*
 * func_8002C278's own "descriptor" pointer, resolved either from a cached
 * byte offset (Obj278::unk34) or freshly from `index*12+8` into
 * Ctx278::unk10's byte array. Field meaning unestablished beyond
 * offset/width -- this region reads as raw hardware/SIO register staging
 * (no classtable.py hit anywhere nearby), not class-framework data.
 */
typedef struct Entry278 {
    u8 unk0;   /* +0x0, tag/kind: zero means "not present", tested first */
    u8 unk1;   /* +0x1 */
    u16 unk2;  /* +0x2 */
    u8 unk4;   /* +0x4 */
    u8 unk5;   /* +0x5 */
    s16 unk6;  /* +0x6 */
    s32 unk8;  /* +0x8 */
} Entry278;

/* func_8002C278's own object (its own `arg1`). Only the fields this
 * function itself touches are named. */
typedef struct Obj278 {
    u8 pad0[0xC];
    s32 unkC;   /* +0xC */
    s32 unk10;  /* +0x10 */
    s32 unk14;  /* +0x14 */
    u8 pad18[0x1A - 0x18];
    s16 unk1A;  /* +0x1A */
    u8 pad1C[0x2C - 0x1C];
    s16 unk2C;  /* +0x2C */
    s16 unk2E;  /* +0x2E */
    s32 unk30;  /* +0x30, cache-hit flag: 1 if unk34 was reused, 0 if freshly computed */
    s32 unk34;  /* +0x34, BEFORE resolution: a cached byte offset into Ctx278::unk10; AFTER: Entry278::unk8 */
    s32 unk38;  /* +0x38 */
} Obj278;

/* Opaque target of Ctx278::unk2C -- only the one dispatched slot named. */
typedef struct Ctx278SubMethods Ctx278SubMethods;
typedef struct Ctx278Sub Ctx278Sub;
struct Ctx278SubMethods {
    u8 pad000[0x080];
    s32 (*slot80)(Ctx278Sub *self);
};
struct Ctx278Sub {
    Ctx278SubMethods *methods;
};

/* func_8002C278's own `arg0`. Only the fields this function itself
 * touches are named. */
typedef struct Ctx278 {
    u8 pad0[0x10];
    u8 *unk10;       /* +0x10, byte-addressed base for the Entry278 table */
    u8 pad14[0x2C - 0x14];
    Ctx278Sub *unk2C; /* +0x2C */
} Ctx278;

INCLUDE_ASM("asm/nonmatchings/code_179d8_d", func_8002C278);

Table6D940 *func_8002C3A8(void)
{
    return &D_8006D940;
}

s32 func_8002C3B8(void)
{
    return 0;
}

void func_8002C3C0(void) {
}

void func_8002C3C8(void) {
}

void func_8002C3D0(void)
{
    char buf[0x40];
}

void func_8002C3E0(void)
{
    char buf[0x40];
}

void func_8002C3F0(void) {
}

void func_8002C3F8(void) {
}

void func_8002C400(void) {
}
