#include "common.h"

/* This class's own +0x058 slot. The sibling class D_8006D430 (see
 * include/code_171e0.h's UnkFlagsObjMethods_171e0) has the identical slot
 * unnamed as "func_80026B08's own slot, unused here" -- kept as an
 * independent local view here, per the project's multiple-local-views
 * convention, rather than editing that shared header (code_179d8_h.c and
 * code_171e0.c also include it this round). */
typedef struct SelfC80Methods SelfC80Methods;
struct SelfC80Methods {
    u8 pad00[0x58];
    /* +0x58 */ void (*slot58)(void *self, char *arg1);
};

typedef struct SelfC80 SelfC80;
struct SelfC80 {
    /* +0x00 */ SelfC80Methods *methods;
    u8 pad04[0x22 - 0x04];
    /* +0x22 */ u16 unk22;
    /* +0x24 */ s32 unk24;
};

/* +0x04 slot of whatever object a still-uninitialized local $s2 points at
 * on this path -- see the func_80027C80 report for why that local is never
 * assigned; only the one field this store touches is typed. */
typedef struct UnkC80 UnkC80;
struct UnkC80 {
    u8 pad00[0x04];
    /* +0x04 */ s32 unk04;
};

struct Self800282AC;
extern void func_800282AC(struct Self800282AC *arg0, s32 arg1, s32 arg2,
                           s32 arg3, s32 arg4);
extern s32 func_800284C4(char *arg0); /* code_179d8_r */
extern s32 D_8008A85C;

void func_80027C80(SelfC80 *self, char *arg1)
{
    UnkC80 *s2;
    s32 idx;

    func_800280D0();

    if (arg1 != NULL) {
        if (D_8008A85C != 0) {
            s2->unk04 = 1;
            idx = func_800284C4(arg1);
            func_800282AC((struct Self800282AC *)self, idx, 7, 0, 0);
        } else {
            self->methods->slot58(self, arg1);

            if (self->unk22 == 0) {
                self->unk24 |= 4;
            }
        }
    }

    func_800280E0();
}

extern void func_800280D0(void);
extern void func_800280E0(void);
extern void func_80028218(void);

void func_80027D40(void)
{
    func_800280D0();
    func_80028218();
    func_800280E0();
}

/* The pending-list node type func_8002832C (code_179d8_r) allocates and
 * func_800283C4 (code_179d8_r) unlinks/frees -- only the fields this call
 * site itself reads are typed here. */
typedef struct QueueEntryD70 QueueEntryD70;
struct QueueEntryD70 {
    /* +0x00 */ s32 unk00;
    u8 pad04[0x0C - 0x04];
    /* +0x0C */ s32 owner;
    u8 pad10[0x20 - 0x10];
    /* +0x20 */ QueueEntryD70 *next;
};

typedef struct SelfD70 SelfD70;
struct SelfD70 {
    u8 pad00[0x22];
    /* +0x22 */ u16 unk22;
    /* +0x24 */ s32 unk24;
};

extern s32 D_8008A894;
extern s32 D_8008A870;
extern s32 D_8008A888;
extern s32 D_8008A87C;
extern void CdFlush(void);
extern void func_80028864(void); /* code_179d8_r */
extern void func_800283C4(QueueEntryD70 *arg0); /* code_179d8_r */

void func_80027D70(SelfD70 *self)
{
    QueueEntryD70 *entry;
    QueueEntryD70 *node;
    QueueEntryD70 *next;
    s32 saved;

    func_800280D0();

    entry = (QueueEntryD70 *)D_8008A894;

    if (entry != NULL && self->unk22 != 0) {
        self->unk24 = 0;

        if (entry->owner == (s32)self && entry->unk00 != 0 && D_8008A870 == 0) {
            CdFlush();
            func_80028864();
            saved = D_8008A888;
            D_8008A888 = 0;
            D_8008A87C = saved;
        }

        for (node = (QueueEntryD70 *)D_8008A894; node != NULL; node = next) {
            next = node->next;
            if (node->owner == (s32)self) {
                func_800283C4(node);
                self->unk22--;
            }
        }
    }

    func_800280E0();
}

/* D_8006D4E8's own method table, 29 slots per tools/classtable.py (header
 * 0x13 at +0x000, func_800269F0 at +0x004/own-slot, func_80027228 at
 * +0x008/ctor, func_80027274 at +0x00C/dtor, the 13 inherited BasicClass
 * slots at +0x010..+0x038, then own slots at +0x040..+0x074 -- func_80027C80
 * (+0x06C), func_80027D40 (+0x070) and func_80027D70 (+0x074), all three
 * queued later in this unit, are among them). This function is this class's
 * "get my own method table" accessor, the same convention func_800269E0
 * uses for D_8006D3C8 and func_80026C9C uses for D_8006D430 (see
 * include/code_171e0.h) -- just an address-of, not gp_rel since D_8006D4E8
 * lives in .data, not .sdata. */
extern s32 D_8006D4E8[];

s32 *func_80027E68(void)
{
    return D_8006D4E8;
}

/* libcd/sys entry points (lib/libcd/sys.o, linked since round 34) --
 * per-call-site typed for this unit, per the code_179d8_h.c convention. */
extern s32 CdSetDebug(s32 arg0);
extern s32 CdControlB(u_char com, void *param, void *result);

extern s32 D_8008A858;

void func_80027E78(void)
{
    u8 mode;

    if (D_8008A858 != 0) {
        return;
    }

    CdSetDebug(0);
    mode = 0x80;
    while (CdControlB(0xE, &mode, 0) == 0) {
    }
    D_8008A858 = 1;
}

extern s32 D_8008A864;

s32 func_80027EC8(void)
{
    return D_8008A864;
}

extern s32 D_8008A870;

s32 func_80027ED4(void)
{
    return D_8008A870;
}

extern s32 D_8008A874;

s32 func_80027EE0(void)
{
    return D_8008A874;
}

extern s32 D_8008A878;

s32 func_80027EEC(void)
{
    return D_8008A878;
}

extern s32 D_8008A860;
extern s32 D_8008A85C;

s32 func_80027EF8(s32 *a0)
{
    if (a0 != NULL) {
        *a0 = D_8008A860;
    }
    return D_8008A85C;
}

extern s32 func_80020C5C(void); /* class_3ac78, returns a pointer cast to s32 */
extern s32 func_800280EC(void);
extern s32 D_8008A864;
extern s32 D_8008A85C;
extern s32 D_8008A860;
extern s32 D_8008A8A4;

/* Object returned by func_80020C5C; only the slot this call site dispatches
 * (+0x84 of its method table) is typed here. */
typedef struct ObjF18Methods ObjF18Methods;
struct ObjF18Methods {
    u8 pad00[0x84];
    void (*slot84)(void *self, void *arg);
};

typedef struct ObjF18 ObjF18;
struct ObjF18 {
    ObjF18Methods *methods;
};

s32 func_80027F18(s32 arg0, s32 arg1, s32 arg2)
{
    ObjF18 *obj;

    if (D_8008A864 == 0) {
        if (arg2 == 0) {
            obj = (ObjF18 *)func_80020C5C();

            if (D_8008A85C == 0) {
                if (arg0 != 0) {
                    obj->methods->slot84(obj, (void *)func_800280EC);
                }
            } else {
                if (arg0 == 0) {
                    obj->methods->slot84(obj, 0);
                }
            }
        }

        D_8008A8A4 = arg2;
        D_8008A85C = arg0;
        D_8008A860 = arg1;

        return 1;
    }

    return 0;
}

extern s32 D_8008A868;

void func_80027FD8(s32 a0)
{
    D_8008A868 = a0;
}

extern s32 D_8008A86C;

void func_80027FE4(s32 a0)
{
    D_8008A86C = a0;
}

extern s32 D_8008A86C;

s32 func_80027FF0(void)
{
    return D_8008A86C;
}

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
 * D_8008A868 (this unit's own func_80027FD8/set) and D_8008A86C (func_8002
 * 7FE4/FF0) are this array's base pointer and element count -- func_800284C4
 * (code_179d8_r) walks the identical 0x1C stride over D_8008A868 doing
 * strstr() against `name`, confirming the layout independently. */
typedef struct FileEntryQ FileEntryQ;
struct FileEntryQ {
    /* +0x00 */ char name[0x14];
    /* +0x14 */ Pair16Q unk14;
    /* +0x18 */ u32 unk18;
};

extern const char D_800107D8[]; /* "File not found. file = %s\n" */
extern s32 CdSearchFile(CdStatBufQ *statBuf, char *path); /* lib/libcd/iso9660.o */
extern void printf(const char *fmt, void *arg1);
extern char *func_800289CC(char *dest, char *suffix);
extern void func_80027E78(void);

s32 func_80027FFC(FileEntryQ *arg0, s32 count)
{
    FileEntryQ *end;
    char path[0x40];
    CdStatBufQ buf;
    s32 tries;

    end = arg0 + count;

    func_80027E78();

    for (; arg0 < end; arg0++) {
        func_800289CC(path, arg0->name);

        for (tries = 0; tries < 0x65; tries++) {
            if (CdSearchFile(&buf, path) != 0) {
                goto found;
            }
        }

        printf(D_800107D8, path);

    found:
        arg0->unk14 = buf.unk0;
        arg0->unk18 = buf.unk4;
    }

    return 1;
}

/* Paired with func_800280E0 just below -- a 1/0 flag toggle on D_8008A88C,
 * called from func_80027C80 (this table's slot +0x06C) as the first thing
 * it does, and from func_80028280 which clears it right back. Reads as a
 * "some subsystem is active" latch; nothing in this unit's own bodies
 * dereferences D_8008A88C, so its consumer lives elsewhere. */
extern s32 D_8008A88C;

void func_800280D0(void)
{
    D_8008A88C = 1;
}

extern s32 D_8008A88C;

void func_800280E0(void)
{
    D_8008A88C = 0;
}

extern s32 GetBMemPMgrBusy(void); /* code_8220_b */
extern s32 D_8008A8A4;
extern s32 D_8008A898;
extern void func_8002858C(void); /* code_179d8_r */
extern void func_800286E4(void); /* code_179d8_r */
extern s32 D_8008A890;
extern void VSyncCallback(void (*cb)(void));

/* This unit's own slot at +0x068 of D_8006D4E8's table (see func_80027E68's
 * class-map comment above); only the one slot this call site dispatches is
 * typed here, following the pad-to-offset convention include/code_171e0.h
 * uses for D_8006D430's own table. */
typedef struct D_8006D4E8Methods D_8006D4E8Methods;
struct D_8006D4E8Methods {
    u8 pad00[0x68];
    void (*slot68)(void);
};

s32 func_800280EC(void)
{
    if (D_8008A88C != 0) {
        return 0;
    }

    if (GetBMemPMgrBusy() != 0) {
        return 0;
    }

    if (D_8008A8A4 != 0) {
        VSyncCallback(0);
    }

    if (D_8008A898 == 1) {
        func_8002858C();
    } else if (D_8008A898 == 2) {
        func_800286E4();
    }

    if (D_8008A890 != 0) {
        ((D_8006D4E8Methods *)func_80027E68())->slot68();
    }

    if (D_8008A8A4 != 0) {
        VSyncCallback((void (*)(void))func_800280EC);
    }

    return 0;
}

extern s32 D_8008A89C;

void func_800281B0(void)
{
    func_800280D0();

    if (D_8008A89C == 0) {
        if (D_8008A8A4 != 0) {
            VSyncCallback((void (*)(void))func_800280EC);
        }
        D_8008A89C = 1;
    }

    D_8008A890 = 1;
    func_800280E0();
}

extern s32 D_8008A898;
extern s32 D_8008A89C;
extern s32 D_8008A8A4;
extern s32 D_8008A890;
extern void VSyncCallback(void (*cb)(void));

void func_80028218(void)
{
    func_800280D0();

    if (D_8008A898 == 0 && D_8008A89C != 0) {
        if (D_8008A8A4 != 0) {
            VSyncCallback(0);
        }
        D_8008A89C = 0;
        D_8008A890 = 0;
    }

    func_800280E0();
}

extern s32 D_8008A890;

void func_80028280(void)
{
    func_800280D0();
    D_8008A890 = 0;
    func_800280E0();
}

/* func_8002832C (code_179d8_r) allocates and links a 0x24-byte list node;
 * only the fields this call site writes are typed here (padded to their
 * offsets, per this unit's convention). */
typedef struct Entry800282AC Entry800282AC;
struct Entry800282AC {
    u8 pad00[0x08];
    s32 unk08;
    s32 unk0C;
    s32 unk10;
    s32 unk14;
    s32 unk18;
};
extern Entry800282AC *func_8002832C(void); /* code_179d8_r */

typedef struct Self800282AC Self800282AC;
struct Self800282AC {
    u8 pad00[0x22];
    u16 unk22;
    s32 unk24;
};

void func_800282AC(Self800282AC *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    Entry800282AC *entry = func_8002832C();

    entry->unk08 = arg2;
    entry->unk14 = arg3;
    entry->unk0C = (s32)arg0;
    entry->unk10 = arg1;
    entry->unk18 = arg4;

    arg0->unk22++;
    arg0->unk24 = 0;
    func_800281B0();
}
