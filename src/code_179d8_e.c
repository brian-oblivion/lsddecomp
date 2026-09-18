/*
 * ROUND 43 UPDATE (2026-09-15): the round-17 `gp_rel` cluster named below is
 * now FULLY MATCHED.  All eight functions (`func_8002C448`, `func_8002C468`,
 * `func_8002C4E0`, `func_8002C638`, `func_8002C6FC`, `func_8002C890`,
 * `func_8002CC1C`, `func_8002CC28`) closed byte-exact once round 42's
 * `--gp-symbols`/`--no-nop-mflo-mfhi` maspsx flags resolved the blocker --
 * see each function's own `docs/match-reports/<func>.md` for the derivation.
 * `func_8002CA3C` is this unit's one remaining `INCLUDE_ASM` as of round 43
 * -- it was never part of the retracted gp_rel cluster and was out of this
 * round's assigned work list.
 *
 * The retracted "BLOCKED, do NOT spend attempts" directive that used to sit
 * here is gone; keeping it around after every function it named was matched
 * would only mislead the next reader.  The classification and provenance
 * notes below (carve history, class-framework correction) still stand.
 *
 * code_179d8_e -- functions 120..148 of the original 274-function code_179d8
 * monolith, 0x1CC08..0x1D508 (vram 0x8002C408..0x8002CD08).  Carved round 17
 * (2026-09-04) out of what the yaml called `code_179d8_mid_b`; the remainder
 * behind it is now `code_179d8_mid_c`.
 *
 * Owns NO switch jump table -- zero `jtbl_` references anywhere in the slice
 * -- so no rodata sub-slot is attached to this unit.
 *
 * CORRECTION (runner echo, round 17): the assignment inherited a blanket "not
 * class-framework code" note from code_179d8_b/_d's sibling findings, but
 * that finding does NOT hold for THIS slice.  `tools/classtable.py --scan`
 * hits both D_8006D9BC (29 slots, header 0x00000023) and D_8006DA34 (39
 * slots, header 0x00000A03) as real class tables.  func_8002C824's and
 * func_8002CC84's self objects load `*self` (offset 0, the methods pointer)
 * and dereference table slots at exactly the offsets `classtable.py D_8006DA34`
 * prints for func_8002C890 (+0x7C) and func_8002CB18 (+0x84) -- direct
 * confirmation this is genuine self->methods->slotN(self, ...) dispatch, not
 * driver code.  D_8006D9BC and D_8006DA34 share the same BasicClass tail
 * (slots +0x10..+0x38), so they are two related classes off the same base --
 * confirmed round 43 as a PS1 SPU/VAB sound-streaming object: `func_8002C4E0`
 * is its constructor, `func_8002C638` its close, `func_8002C6FC` its
 * per-frame poll (header/body transfer state machine), and `func_8002C890`
 * its post-load VAB attribute-table fetch.  `D_8006DA34`'s whole vtable was
 * read straight out of `asm/data/5E140.data.s` while matching this cluster
 * -- see `docs/match-reports/func_8002C4E0.md` for the full slot table,
 * including two slots (+0x58, +0x6C) that are null in retail's own data.
 */
#include "common.h"

/* ------------------------------------------------------------------------
 * SoundObj class framework (D_8006D9BC / D_8006DA34).  Only the slots and
 * fields THIS unit's functions actually touch are named; everything else is
 * opaque padding, per this project's local-reading convention (see
 * code_179d8_d.c's Table6D940/Obj6D940 for the same idiom applied to a
 * neighbouring class).  Kept LOCAL to this file -- no shared header, so a
 * sibling slice's independent reading of the same tables cannot collide with
 * this one on merge.
 * ------------------------------------------------------------------------ */
typedef struct ObjDA34 ObjDA34;

/* D_8006D9BC's own methods table.  Only +0x054 is a slot this unit defines. */
typedef struct TableD9BC {
    u8 pad000[0x054];
    s32 (*slot54)(void); /* func_8002C408 */
    u8 pad058[0x078 - 0x058];
} TableD9BC;
extern TableD9BC D_8006D9BC;

/* D_8006DA34's own methods table -- the class ObjDA34 below dispatches
 * through.  Only the slots this unit's own functions call or are assigned to
 * are named.  round 43 added slot58/slot5C/slot6C/slot9C once func_8002C4E0,
 * func_8002C6FC and func_8002C890 (all round-17 gp_rel stalls, resolved
 * round 42) were actually worked. */
typedef struct TableDA34 {
    u8 pad000[0x008];
    void (*slot08)(void *self, char *arg1); /* func_8002C4E0 -- this unit's own new_class_da34 dispatch; arg1 is a base filename, not a plain s32 */
    s32 (*slot0C)(ObjDA34 *self);           /* func_8002C638, confirmed against D_8006DA34's own rodata (+0x0C) */
    u8 pad010[0x058 - 0x010];
    /* +0x058 and +0x06C are BOTH null in retail's own D_8006DA34 (confirmed
     * against asm/data/5E140.data.s) -- func_8002C6FC and func_8002C4E0 each
     * dispatch through one of them anyway, on a path that is apparently
     * never actually taken for an object built with this exact base table.
     * That is retail's own behaviour, not a derivation error: the dispatch
     * still has to compile, whatever sits at the address at runtime. */
    void (*slot58)(void *self, char *path); /* func_8002C6FC's own dispatch -- begins the VAB body transfer once the ".VB" path is built; null in retail */
    void (*slot5C)(void *self);             /* Class6D430__FreeBuffer (uncarved, cross-unit) -- func_8002C890's own dispatch, called before it re-fetches the VAB header */
    u8 pad060[0x06C - 0x060];
    void (*slot6C)(void *self, char *path); /* func_8002C4E0's own dispatch -- begins the VAB header transfer for the ".VH" path; null in retail */
    u8 pad070[0x078 - 0x070];
    s32 (*slot78)(ObjDA34 *self, s32 arg1); /* func_8002C824, and func_8002C6FC's own body-complete notify */
    s32 (*slot7C)(ObjDA34 *self);           /* func_8002C890 */
    u8 pad080[0x084 - 0x080];
    s32 (*slot84)(ObjDA34 *self, s32 arg1); /* func_8002CB18, arg1 is s16-truncated by the callee */
    u8 pad088[0x09C - 0x088];
    void (*slot9C)(void *self, s32 arg1);   /* func_8002CBF4 -- func_8002C4E0's own dispatch, called with arg1 == 0 right after construction */
} TableDA34;
extern TableDA34 D_8006DA34;

/* One "chunk" entry inside a self->unk50 sub-array, stride 0x20.  This is
 * Sony's own `VagAtr` (include/psyq/LIBSND.H, 32 bytes) -- unk4/unk5 land
 * exactly on VagAtr's `center`/`shift` bytes -- but kept under this unit's
 * own local name per the project's independent-local-view convention rather
 * than pulling in the real header (see code_179d8_k.c for the same choice).
 * Only the two bytes func_8002CA3C itself reads are named. */
typedef struct Chunk179D8E {
    u8 pad0[0x4];
    u8 unk4;
    u8 unk5;
    u8 pad6[0x20 - 0x6];
} Chunk179D8E;

/* This unit's own reduced view of Sony's `VabHdr` (include/psyq/LIBSND.H,
 * 32 bytes) -- only the two fields func_8002C890 itself reads are named:
 * `ts` (program count) and `vs` (vag count), at the real struct's own
 * offsets +0x12/+0x14. */
typedef struct VabHdr179D8E {
    u8 pad0[0x12];
    u16 ts;  /* +0x12, program count */
    u16 vs;  /* +0x14, vag count */
    u8 pad16[0x20 - 0x16];
} VabHdr179D8E;

/* self for the D_8006DA34-dispatched methods in this unit.  Only fields this
 * unit's own functions touch are named -- see the header-comment correction
 * above for how this was established. */
struct ObjDA34 {
    TableDA34 *methods;    /* +0x000 */
    u8 pad004[0x010 - 0x004];
    u8 *unk10;              /* +0x010, streaming file buffer -- passed to SsVabOpenHead/SsVabTransBody */
    u8 pad014[0x024 - 0x014];
    u32 unk24;               /* +0x024, flag word; bit 0x200 gates the header/body transfer steps */
    u8 pad028[0x02A - 0x028];
    u16 unk2A;                 /* +0x02A, load state: 0 idle, 1 header pending, 6 body pending -- unsigned (retail loads it lhu) */
    VabHdr179D8E unk2C;          /* +0x02C, filled by SsUtGetVabHdr; 32 bytes, ends exactly at +0x04C */
    Chunk179D8E *unk4C;            /* +0x04C, VagAtr pool, unk2C.vs entries */
    Chunk179D8E **unk50;             /* +0x050, array of unk2C.ts pointers into unk4C's pool */
    s16 unk54;                        /* +0x054 */
    s16 unk56;                         /* +0x056, boolean-ish flag */
    u16 unk58;                          /* +0x058, unsigned (retail loads it lhu in func_8002C890) */
    u16 unk5A;                           /* +0x05A */
    void *unk5C;                          /* +0x05C, malloc'd copy of the base filename */
    s32 unk60;                             /* +0x060 */
};

/* Cross-unit calls into the still-uncarved code_179d8_tail monolith --
 * declared LOCAL to this unit, per-call-site typed, since none of them have
 * an established prototype anywhere yet. */
extern void *func_80017B34(s32 size);
extern s16 func_80030E90(s16 a0, s16 hi, s16 lo, s16 a3, s32 b5, s32 argA, s32 argB);
extern void func_80031E94(s16 a0, s16 a1, s16 a2, s32 a3);
extern void func_80031890(s16 index);
extern void func_80031F3C(s32 arg0);
/* Sony's `SsVabTransCompleted` (`libsnd/vs_vtc`) and `SsSetMute`
 * (`libsnd/scsmute`), linked from the SDK objects since round 34.  The two
 * signatures are this call site's own reading and disagree with the sibling
 * reading in code_179d8_i.c about the return types -- that is the project's
 * independent-local-view convention, and it is exactly why a Psy-Q prototype
 * must never go into a header these units share. */
extern void SsVabTransCompleted(s32 arg0);
extern s32 SsSetMute(s32 arg0);

/* func_8002CC34's own "obj" (its `arg1`) -- a small slot-table object,
 * unrelated to ObjDA34 (this function is NOT a D_8006DA34 vtable slot; its
 * only callers pass a plain heap/stack struct pointer).  Only the fields
 * this unit's own function touches are named. */
typedef struct Slot179D8ECC34 {
    s32 unk0; /* -1 = free/sentinel after init */
    u8 pad4[0x14 - 0x4];
} Slot179D8ECC34;

typedef struct ObjCC34 {
    s32 unk0;   /* +0x00, guard: 0 = uninitialized */
    s32 unk4;   /* +0x04 */
    void *unk8; /* +0x08 */
    s32 unkC;   /* +0x0C */
    u8 pad10[0x14 - 0x10];
    s32 unk14;  /* +0x14 */
    Slot179D8ECC34 arr[3]; /* +0x18 */
} ObjCC34;

/* Forward declaration: func_8002CC0C is defined later in this file (ROM
 * order), but func_8002C480 (earlier in ROM order) calls it. */
TableDA34 *func_8002CC0C(void);

s32 func_8002C408(void) {
    return 0;
}

void func_8002C410(void) {
}

void func_8002C418(void) {
}

void func_8002C420(void) {
}

void func_8002C428(void) {
}

void func_8002C430(void) {
}

TableD9BC *func_8002C438(void) {
    return &D_8006D9BC;
}

extern s32 D_8008A8B0;
extern s32 D_8008A8B4;

s32 func_8002C448(s32 *arg0) {
    if (arg0 != NULL) {
        *arg0 = D_8008A8B4;
    }
    return D_8008A8B0;
}

s32 func_8002C468(s32 a, s32 b)
{
	D_8008A8B0 = a;
	D_8008A8B4 = b;
	return 1;
}

s32 func_8002C478(void) {
    return 0;
}

void *func_8002C480(s32 arg0) {
    void *self;

    self = func_80017B34(0x64);
    if (self != NULL) {
        func_8002CC0C()->slot08(self, arg0);
        return self;
    }
    return NULL;
}

/* Base-class table reached via the uncarved accessor `GetActiveDataSourceMethods()` --
 * not this unit's function to define.  Only the two slots this unit's own
 * functions dispatch through are typed, per the same convention as
 * code_179d8_d.c's own independent reading of the same physical table. */
typedef struct BaseTable179D8E {
    u8 pad000[0x008];
    void (*slot08)(void *self);  /* func_8002C4E0's own base-chain call */
    /* func_8002C638's own base-chain call -- its own return is likewise a
     * bare tail call with nothing after it, so per CLAUDE.md's rule this
     * defaults to s32 absent positive void evidence. */
    s32 (*slot0C)(void *self);
} BaseTable179D8E;
extern BaseTable179D8E *GetActiveDataSourceMethods(void);

/* Sony's own VAB streaming calls (include/psyq/LIBSND.H), declared locally
 * per this project's convention of not sharing Psy-Q prototypes across
 * units (see the SsVabTransCompleted/SsSetMute comment above). */
extern void SsVabClose(s16 vabId);
extern s16 SsVabOpenHead(u8 *addr, s16 arg1);
extern s16 SsVabTransBody(u8 *addr, s16 vabId);
extern s16 SsUtGetVabHdr(s16 vabId, void *out);
extern s16 SsUtGetProgAtr(s16 vabId, s16 prog, void *out);
extern s16 SsUtGetVagAtr(s16 vabId, s16 prog, s16 tone, void *out);
extern void SsSetMVol(s16 a0, s16 a1);
extern void SsSetTableSize(char *a0, s16 a1, s16 a2);

/* Uncarved code_179d8_tail helpers this cluster calls. */
extern s32 func_80032368(void);
extern char *func_8003A068(void);
extern s32 func_8003A05C(void);
extern void func_800329D8(void);
extern void func_80032A7C(void);
extern void func_80032588(s32 a0);
extern void func_80032998(void);
extern void *func_80017CFC(void *ptr);
extern char *BuildFileName(char *dest, char *arg1, char *arg2, char *arg3);
extern s32 strlen(char *s);
extern char *strcpy(char *dest, char *src);

/* ".VH"/".VB" -- already emitted by splat in .sdata, referenced not
 * retyped (a literal here would duplicate the bytes and shift the image). */
extern const char D_8008A8D0[];
extern const char D_8008A8D4[];

extern s32 D_8008A8B8;
extern s32 D_8008A8BC;
extern s32 D_8008A8C0;
extern s32 D_8008A8C4;
extern s32 D_8008A8CC;
extern void *D_8008A8C8;

void func_8002C4E0(ObjDA34 *self, char *arg1) {
    void *buf;
    char path[0x20];

    GetActiveDataSourceMethods()->slot08(self);
    self->methods = func_8002CC0C();
    self->unk4C = NULL;
    self->unk50 = NULL;
    self->unk54 = 0;
    self->unk56 = 0;
    self->methods->slot9C(self, 0);
    self->unk58 = 0;
    self->unk5A = 0;
    self->unk5C = NULL;
    if (D_8008A8B8 == 0) {
        func_80032368();
        D_8008A8B8 = 1;
        SsSetTableSize(func_8003A068(), 2, 1);
    }
    if (D_8008A8BC == 0) {
        D_8008A8CC = 0x3C;
        func_80032588(1);
        D_8008A8BC = 1;
    }
    D_8008A8C4++;
    if (arg1 != NULL) {
        buf = func_80017B34(strlen(arg1) + 1);
        if (buf != NULL) {
            self->unk5C = buf;
            strcpy(buf, arg1);
            BuildFileName(path, buf, NULL, D_8008A8D0);
            self->unk2A = 1;
            self->methods->slot6C(self, path);
        }
    }
}

s32 func_8002C638(ObjDA34 *self) {
    SsVabClose(self->unk54);
    if (--D_8008A8C4 < 0) {
        D_8008A8C4 = 0;
    }
    if (D_8008A8C4 == 0 && func_8003A05C() == 0) {
        D_8008A8B8 = 0;
        D_8008A8C0 = 0;
        D_8008A8BC = 0;
        func_800329D8();
        func_80032A7C();
    }
    func_80017CFC(self->unk4C);
    func_80017CFC(self->unk50);
    func_80017CFC(self->unk5C);
    return GetActiveDataSourceMethods()->slot0C(self);
}

void func_8002C6FC(ObjDA34 *self) {
    char path[0x20];

    switch (self->unk2A) {
    case 0:
        break;
    case 1:
        if (self->unk24 & 0x200) {
            self->unk54 = SsVabOpenHead(self->unk10, -1);
            BuildFileName(path, self->unk5C, NULL, D_8008A8D4);
            D_8008A8C8 = self->unk10;
            self->unk2A = 6;
            self->unk10 = NULL;
            self->methods->slot58(self, path);
            if (self->unk5C != NULL) {
                func_80017CFC(self->unk5C);
                self->unk5C = NULL;
            }
        }
        break;
    case 6:
        if (self->unk24 & 0x200) {
            self->unk54 = SsVabTransBody(self->unk10, self->unk54);
            if (self->unk54 != -1) {
                self->unk5A = 1;
                self->methods->slot78(self, 1);
            }
        }
        break;
    default:
        break;
    }
}

s32 func_8002C824(ObjDA34 *self, s32 arg1) {
    s32 result;

    result = 0;
    if (self->unk5A != 0) {
        if (arg1 != 0) {
            SsVabTransCompleted(1);
            self->unk5A = 0;
            self->unk58 = 1;
            self->methods->slot7C(self);
            result = 1;
        }
    }
    return result;
}

/* This unit's own reduced view of Sony's `ProgAtr` (include/psyq/LIBSND.H,
 * 16 bytes) -- only the one field func_8002C890 itself reads is named,
 * per the same local-struct convention used for VabHdr179D8E above (and
 * matching code_179d8_k.c's own reduced `ProgAtr` reading). */
typedef struct ProgAtr179D8E {
    u8 tones; /* +0x00, program's tone count, written by SsUtGetProgAtr */
    u8 pad1[0x10 - 0x1];
} ProgAtr179D8E;

/* Class6D430__FreeBuffer -- uncarved, cross-unit; reached only through
 * D_8006DA34's own +0x5C slot (declared above as `slot5C`), never called
 * directly by name here. */

void func_8002C890(ObjDA34 *self)
{
    ProgAtr179D8E prog;
    Chunk179D8E *pool;
    s32 i;
    s32 j;
    s16 result;

    if (self->unk58 == 0) {
        return;
    }
    self->methods->slot5C(self);
    self->unk10 = D_8008A8C8;
    result = SsUtGetVabHdr(self->unk54, &self->unk2C);
    if (result == -1) {
        return;
    }
    self->unk4C = func_80017B34(self->unk2C.vs << 5);
    if (self->unk4C == NULL) {
        return;
    }
    self->unk50 = func_80017B34(self->unk2C.ts << 2);
    if (self->unk50 == NULL) {
        return;
    }
    pool = self->unk4C;
    for (i = 0; i < self->unk2C.ts; i++) {
        self->unk50[i] = pool;
        result = SsUtGetProgAtr(self->unk54, i, &prog);
        if (result == -1) {
            return;
        }
        for (j = 0; j < prog.tones; j++) {
            result = SsUtGetVagAtr(self->unk54, i, j, pool);
            if (result == -1) {
                return;
            }
            pool++;
        }
    }
    if (D_8008A8C0 == 0) {
        func_80032998();
        SsSetMVol(0x78, 0x78);
        D_8008A8C0 = 1;
    }
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_e", func_8002CA3C);

s32 func_8002CB18(ObjDA34 *self, s32 index) {
    if (index < 0x18) {
        func_80031890(index);
    } else {
        func_80031F3C(0);
    }
    return -1;
}

s32 func_8002CB58(ObjDA34 *self) {
    s32 flag;

    flag = self->unk56;
    if (flag == 0) {
        SsSetMute(1);
        flag = 1;
        self->unk56 = flag;
    }
    return flag;
}

s32 func_8002CB9C(ObjDA34 *self) {
    s32 flag;

    flag = self->unk56;
    if (flag != 0) {
        flag = SsSetMute(0);
        self->unk56 = 0;
    }
    return flag;
}

void func_8002CBDC(void) {
}

void func_8002CBE4(void) {
}

void func_8002CBEC(void) {
}

void func_8002CBF4(ObjDA34 *self, s32 arg1) {
    self->unk60 = arg1 * 12 - 0x18;
}

TableDA34 *func_8002CC0C(void) {
    return &D_8006DA34;
}

extern s32 D_8008A8C4;

s32 func_8002CC1C(void) {
    return D_8008A8C4;
}

extern s32 D_8008A8CC;

s32 func_8002CC28(void) {
    return D_8008A8CC;
}

s32 func_8002CC34(void *unused, ObjCC34 *obj, s32 arg2, void *arg3, s32 arg4) {
    Slot179D8ECC34 *slot;
    s32 count;
    s32 sentinel;

    if (obj->unk0 != 0) {
        return 0;
    }
    slot = obj->arr;
    sentinel = -1;
    count = 2;
    obj->unk0 = arg2;
    obj->unk8 = arg3;
    obj->unkC = arg4;
    do {
        slot->unk0 = sentinel;
        count--;
        slot++;
    } while (count >= 0);
    obj->unk4 = 0;
    obj->unk14 = 10;
    return 1;
}

void func_8002CC84(ObjDA34 *self, ObjCC34 *obj) {
    s32 i;
    Slot179D8ECC34 *slot;

    slot = obj->arr;
    for (i = 0; i < 3; i++) {
        if (slot->unk0 >= 0) {
            slot->unk0 = self->methods->slot84(self, slot->unk0);
        }
        slot++;
    }
    obj->unk0 = 0;
}
