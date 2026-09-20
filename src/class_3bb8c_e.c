#include "common.h"

/*
 * class_3bb8c_e (round 14): 19 functions carved from the same 305-function
 * class_3bb8c remainder segment as class_3bb8c_b/_c/_d/_f, but they operate
 * on a class UNRELATED to Obj866E8/D_800866E8 (include/class_3bb8c.h) --
 * none of these 19 functions read or write anything typed there, and no
 * function in _b/_c/_d/_f calls into this unit (checked: the only cross-unit
 * calls are FROM this unit INTO class_3bb8c_f's BuildMemcardPath /
 * TaskObjF__EnableEvents / TaskObjF__DisableEvents / TaskObjF__TestEvents /
 * TaskObjF__ForEachEvent / TaskObjF__WaitForReadyEvent, never the reverse).
 * These declarations are kept local to this .c rather than moved into
 * include/class_3bb8c.h, per the project's cross-unit-prototype rule.
 *
 * ROUND 60 CORRECTIONS to this comment. (1) Those six callees were described
 * here as "still-INCLUDE_ASM"; they are MATCHED C and have been for several
 * rounds -- the round-14 wording outlived the fact. (2) The original reason
 * given for keeping them local was "zero collision risk with the other two
 * runners editing that shared header this round", which froze one round's
 * staffing into a source comment; the durable reason is the rule, not who
 * happened to be running. (3) The "UNRELATED class" claim above concerns
 * Obj866E8 and still stands, but round 60 found concrete evidence that this
 * unit's Node3bb8cE and class_3bb8c_f's TaskObjF may be ONE class seen
 * through two independent local views: func_8004E5E4 fills Node3bb8cE's
 * threads[4] at +0x014 and hands the same pointer to TaskObjF__EnableEvents.
 * A track-4 lead; see docs/match-reports/TaskObjF__EnableEvents.md.
 *
 * The object derives from the same BasicClass framework documented in
 * include/code_8220.h (base vtable fetched via a no-argument getter,
 * Get_vtable_BasicClass(), with finalize/addChild/removeChild/removeAllChildren at
 * +0x00C/+0x010/+0x014/+0x018) -- this unit's own independent local view of
 * that same getter, per the project's established
 * multiple-independent-local-views convention (docs/DECOMPILATION_LEARNINGS.md).
 */
typedef struct BaseMethods3bb8cE BaseMethods3bb8cE;
struct BaseMethods3bb8cE {
    u8 pad000[0x00C];
    void (*finalize)(void *self);                 /* +0x00C, func_8004E40C */
    void (*addChild)(void *self, void *child);    /* +0x010, func_8004E444 */
    void (*removeChild)(void *self, void *child); /* +0x014, func_8004E4E8 */
    void (*removeAllChildren)(void *self);        /* +0x018, func_8004E588 */
};
extern BaseMethods3bb8cE *Get_vtable_BasicClass(void);

/*
 * A typed child resource attached to a Node3bb8cE. Only the resource's own
 * header/type-tag word (methods->header, whose low byte(s) this unit tests
 * against literals 2/5/0x10/0x20) is read here -- func_8004E444/func_8004E4E8
 * never dereference the resource beyond that one word.
 */
typedef struct ResHeader3bb8cE ResHeader3bb8cE;
struct ResHeader3bb8cE {
    s32 header;   /* +0x000, low byte(s): type tag (2, 5, 0x10 or 0x20) */
};
typedef struct Res3bb8cE Res3bb8cE;
struct Res3bb8cE {
    ResHeader3bb8cE *methods;   /* +0x000 */
};

/*
 * The class itself. Offsets established purely from this unit's own 19
 * functions (see each field's comment). unk60/unk64/unk78/unk7C are four
 * typed resource slots, one per tag value func_8004E444/func_8004E4E8
 * dispatch on; unk68 is zeroed alongside them by func_8004E3F4/func_8004E588
 * but has no setter anywhere in this unit, so its pointee type is unproven.
 */
typedef struct Node3bb8cE Node3bb8cE;

/* The object's OWN vtable, at offset 0 -- distinct from the separately
 * fetched base-class table (`Get_vtable_BasicClass()`, `BaseMethods3bb8cE` above).
 * Only the one slot this unit's functions reach is typed. */
typedef struct SelfMethods3bb8cE SelfMethods3bb8cE;
struct SelfMethods3bb8cE {
    u8 pad000[0x054];
    /* func_8004EADC: called (self, 0, buf) per candidate string; 0 return
     * means "match" (buf is returned as the winning string). */
    s32 (*slot54)(Node3bb8cE *self, s32 arg1, char *arg2); /* +0x054 */
};

struct Node3bb8cE {
    SelfMethods3bb8cE *methods;   /* +0x000, func_8004EADC */
    u8 pad04[0x00C - 0x004];
    s32 unkC;              /* +0x00C, func_8004E5D4 sets it (caller value); func_8004E940 nonzero-tests it; func_8004EA38/func_8004ECCC forward it as BuildMemcardPath's arg1 */
    s32 unk10;             /* +0x010, func_8004E5D4: unkC << 4; func_8004E7D0/func_8004E890: a resource handle passed to _card_info/_card_load/func_80050B28 */
    s32 threads[4];        /* +0x014..+0x020, func_8004E5E4: 4 OpenTh-style thread handles, one per D_80086E78[] entry */
    u8 pad24[0x060 - 0x024];
    Res3bb8cE *unk60;      /* +0x060, tag 2 */
    Res3bb8cE *unk64;      /* +0x064, tag 5 */
    Res3bb8cE *unk68;      /* +0x068, zeroed only -- no setter in this unit */
    u8 pad6C[0x078 - 0x06C];
    Res3bb8cE *unk78;      /* +0x078, tag 0x10 */
    Res3bb8cE *unk7C;      /* +0x07C, tag 0x20 */
};

/* Uncarved helpers this unit calls into, all still INCLUDE_ASM in
 * class_3bb8c_f.c (extern for a function OUTSIDE this unit). Declared with
 * unspecified argument lists (K&R style, no prototype) where this unit's own
 * call sites disagree on arity -- same idiom already established for
 * strcpy/strcat in include/psyq/STRINGS.H -- rather than forcing one
 * prototype to fit every call site. */
extern void *BuildMemcardPath(); /* arity-ok: definition is 3-parameter and the callee reads $a2 (`suffix`), but this unit's two call sites disagree on arity and BOTH are byte-load-bearing -- func_8004EA38 emits no $a2 at all (0x8004EA58) while func_8004ECCC emits `lui a2`/`addiu a2` (0x8004ECE8) */
extern void TaskObjF__EnableEvents(void *self);
extern void *TaskObjF__DisableEvents(void *self);
extern void TaskObjF__TestEvents(void *self);
extern void TaskObjF__ForEachEvent(void *self, void (*fn)(void), s32 arg2);
extern s32 TaskObjF__WaitForReadyEvent(void *self);

/* PSX thread-table constant walked by func_8004E5E4 (4 entries, one per
 * OpenTh-style thread it starts). Address-only-derived walk (lui/addiu then
 * plain lw at increasing offsets), never gp-relative, so unaffected by the
 * project's gp_rel blocker. */
extern s32 D_80086E78[4];
extern s32 OpenEvent(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Two format-string-like globals selected by func_8004E940 on self->unkC's
 * truth value; passed opaquely (never dereferenced in this unit). */
extern s32 D_8008AA9C;
extern s32 D_8008AAA4;
/* Third such constant, passed as BuildMemcardPath's 3rd argument by
 * func_8004ECCC only. */
extern s32 D_8008AAAC;

extern void *func_80017B34(s32 size);
extern void *func_80017CFC(void *ptr);
extern char *strcpy(char *dest, char *src);
extern char *strcat(char *dest, char *src);

void func_8004E3F4(Node3bb8cE *self)
{
    self->unk60 = NULL;
    self->unk64 = NULL;
    self->unk68 = NULL;
    self->unk78 = NULL;
    self->unk7C = NULL;
}

void func_8004E40C(Node3bb8cE *self)
{
    Get_vtable_BasicClass()->finalize(self);
}

void func_8004E444(Node3bb8cE *self, Res3bb8cE *res)
{
    s32 tag;

    if (res == NULL) {
        return;
    }
    Get_vtable_BasicClass()->addChild(self, res);
    tag = res->methods->header;
    if ((tag & 0xF) == 2) {
        self->unk60 = res;
        return;
    }
    if ((tag & 0xF) == 5) {
        self->unk64 = res;
        return;
    }
    if ((tag & 0xFF) == 0x10) {
        self->unk78 = res;
        return;
    }
    if ((tag & 0xFF) == 0x20) {
        self->unk7C = res;
    }
}

void func_8004E4E8(Node3bb8cE *self, Res3bb8cE *res)
{
    s32 tag;

    if (res == NULL) {
        return;
    }
    tag = res->methods->header;
    if ((tag & 0xF) == 2) {
        self->unk60 = NULL;
    } else if ((tag & 0xF) == 5) {
        self->unk64 = NULL;
    } else if ((tag & 0xFF) == 0x10) {
        self->unk78 = NULL;
    } else if ((tag & 0xFF) == 0x20) {
        self->unk7C = NULL;
    }
    Get_vtable_BasicClass()->removeChild(self, res);
}

void func_8004E588(Node3bb8cE *self)
{
    self->unk60 = NULL;
    self->unk64 = NULL;
    self->unk68 = NULL;
    self->unk78 = NULL;
    self->unk7C = NULL;
    Get_vtable_BasicClass()->removeAllChildren(self);
}

void func_8004E5D4(Node3bb8cE *self, s32 val)
{
    self->unkC = val;
    self->unk10 = val << 4;
}

extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);

s32 func_8004E5E4(Node3bb8cE *self)
{
    s32 i;
    Node3bb8cE *cur;

    EnterCriticalSection();
    i = 0;
    cur = self;
    do {
        cur->threads[0] = OpenEvent(0xF4000001, D_80086E78[i], 0x2000, 0);
        i++;
        cur = (Node3bb8cE *)((u8 *)cur + 4);
    } while (i < 4);
    ExitCriticalSection();
    TaskObjF__EnableEvents(self);
    return 1;
}

extern void CloseEvent(void);

s32 func_8004E678(Node3bb8cE *self)
{
    TaskObjF__DisableEvents(self);
    TaskObjF__ForEachEvent(self, CloseEvent, 1);
    return 1;
}

extern s32 func_8004E77C(Node3bb8cE *self, s32 *p1, s32 *p2, s32 *p3);

/* STALL -- see docs/match-reports/func_8004E6B8.md. Best reached: correct
 * CONTROL FLOW and correct VALUES (confirmed via objdump against the exact
 * disassembly), but the compiled body is one word short with different
 * callee-saved register numbering and a different call-argument delay-slot
 * split. Restored to INCLUDE_ASM so the correct-length placeholder doesn't
 * cascade drift into func_8004E77C and everything after it in this unit.
 *
 * ROUND 37 (delta): rebuilt this EXACT preserved body against today's
 * pinned toolchain before trusting its recorded score, per this round's
 * own directive -- and it does NOT reproduce the claim above. Confirmed
 * with an isolated cpp|cc1|maspsx|as reproducer (no project headers, just
 * this function and a stub `Node3bb8cE`/`func_8004E77C` declaration) that
 * this exact source, unchanged, compiles with the two `func_8004E77C`
 * call sites CROSS-JUMP MERGED into a single shared `jal` -- the same
 * pathology the report's own attempts 1/2/3 hit and attempt 4 (this body)
 * was written up as having eliminated ("the cross-jump merge disappeared
 * entirely"). It has not: only one `jal` appears in the compiled object,
 * reached by two different paths that set up its argument registers
 * differently beforehand, exactly like the merge the report describes
 * for the REJECTED attempts. This is not a toolchain drift -- the
 * addiu_at flag (round 21) that landed after this stall was written only
 * touches maspsx, never cc1, and the isolated repro used today's cc1 in
 * total isolation from the rest of this file. The report's claim was
 * simply never true of this exact source, or stopped being tested before
 * being written down. See the match report for the corrected residue and
 * this round's re-measurement. */
#if 0
s32 func_8004E6B8(Node3bb8cE *self, s32 *p1, s32 *p2, s32 *p3)
{
    s32 retries;
    s32 localFlag;
    s32 result;

    retries = 10;
    *p2 = 0;
    result = func_8004E77C(self, p1, &localFlag, p3);
    goto check;
retry:
    if (retries == 0) {
        goto done;
    }
    retries--;
    result = func_8004E77C(self, p1, p2, p3);
check:
    if (result == 0) {
        goto retry;
    }
    if (*p1 != 0) {
        goto retry;
    }
    if (*p3 == 0) {
        goto retry;
    }
done:
    *p2 = *p2 | localFlag;
    return result;
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_e", func_8004E6B8);

extern s32 func_8004E7D0(Node3bb8cE *self, s32 *p1, s32 *p2);
extern s32 func_8004E890(Node3bb8cE *self, s32 *p1, s32 *p2);

s32 func_8004E77C(Node3bb8cE *self, s32 *p1, s32 *p2, s32 *p3)
{
    if (func_8004E7D0(self, p1, p2) != 0) {
        func_8004E890(self, p1, p3);
    }
}

extern s32 _card_info(s32 arg0);
extern s32 func_80050B28(s32 arg0);

s32 func_8004E7D0(Node3bb8cE *self, s32 *p1, s32 *p2)
{
    s32 status;
    s32 code;

    status = 1;
    *p2 = *p1 = 0;
    TaskObjF__TestEvents(self);
    while (_card_info(self->unk10) == 0)
        ;
    code = TaskObjF__WaitForReadyEvent(self);
    if (code == 0x100) {
        status = 0;
    } else if (code == 0x8000) {
        status = 0;
        *p1 = 1;
    } else if (code == 0x2000) {
        *p2 = 1;
        func_80050B28(self->unk10);
    }
    return status;
}

extern s32 _card_load(s32 arg0);

s32 func_8004E890(Node3bb8cE *self, s32 *p1, s32 *p2)
{
    s32 status;
    s32 code;

    status = 1;
    *p2 = (*p1 = 0, status);
    TaskObjF__TestEvents(self);
    while (_card_load(self->unk10) == 0)
        ;
    code = TaskObjF__WaitForReadyEvent(self);
    if (code == 0x100) {
        status = 0;
    } else if (code == 0x8000) {
        status = 0;
        *p1 = 1;
    } else if (code == 0x2000) {
        *p2 = 0;
    }
    return status;
}

extern s32 format(s32 *arg0);

s32 func_8004E940(Node3bb8cE *self)
{
    s32 retries;
    s32 result;
    s32 *path;

    retries = 10;
    do {
        path = self->unkC != 0 ? &D_8008AA9C : &D_8008AAA4;
        result = format(path);
    } while (result == 0 && retries-- != 0);
    return result;
}

/* func_8004EA38's 3rd parameter (`filterName` here) is forwarded verbatim
 * by func_8004E9AC but never read by func_8004EA38's own body -- the same
 * "unused parameter invisible from the callee's own disassembly" shape
 * already established elsewhere in this project (only the CALLER's setup
 * proves it's a real parameter). */
extern s32 func_8004EA38(Node3bb8cE *self, u8 *destBuf, u8 *filterName);

s32 func_8004E9AC(Node3bb8cE *self, u8 *destBuf, u8 *filterName)
{
    s32 retries;
    s32 result;

    retries = 0;
    if (filterName == NULL || *filterName == 0) {
        return 0;
    }
    do {
        result = func_8004EA38(self, destBuf, filterName);
    } while (result == 0 && retries-- != 0);
    return result;
}

extern void *BuildMemcardPath(); /* arity-ok: second copy of the declaration above, same reason -- the 2-argument call at func_8004EA38 and the 3-argument call at func_8004ECCC cannot share one prototype */
extern s32 open(void *arg0, s32 arg1);
extern s32 read(s32 arg0, void *arg1, s32 arg2);
extern s32 close(s32 arg0);

/* STALL -- see docs/match-reports/func_8004EA38.md. Best reached: 1/41
 * words in-range, but with ZERO instruction-count drift (40 vs retail's
 * 41 -- confirmed via objdump on the compiled .o) caused by one missing
 * redundant register move; every semantic value and branch is right.
 * Preserved body below, restored to INCLUDE_ASM so this correct-length
 * placeholder doesn't cascade drift into every function after it in this
 * unit.
 *
 * ROUND 37 (delta): rebuilt this EXACT preserved body before trusting its
 * score -- reproduces 1/41 in-range, 40 vs 41 words (missing redundant
 * move), exactly as recorded. Lowest priority on this round's list (an
 * already-permuter-searched function, twice: round 14 ~4600 iterations
 * and round 19 ~130,167 iterations, both converging on the same floor of
 * 5 with no zero); not re-searched this round in favour of the two
 * never-searched functions this round's own thesis prioritized. */
#if 0
s32 func_8004EA38(Node3bb8cE *self, u8 *destBuf, u8 *filterName)
{
    s32 pathBuf[8];
    void *path;
    s32 handle;
    void *buf;

    path = BuildMemcardPath(pathBuf, self->unkC);
    handle = open(path, 1);
    if (handle == -1) {
        return 0;
    }
    if (destBuf != NULL) {
        buf = func_80017B34(0x80);
        read(handle, buf, 0x80);
        strcpy((char *)destBuf, (char *)buf + 4);
        func_80017CFC(buf);
    }
    close(handle);
    return 1;
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_e", func_8004EA38);

char *func_8004EADC(Node3bb8cE *self, char *buf, char *middle, char **entries)
{
    while (*entries != NULL) {
        strcpy(buf, middle);
        strcat(buf, *entries);
        if (self->methods->slot54(self, 0, buf) == 0) {
            return buf;
        }
        entries++;
    }
    return NULL;
}

s32 func_8004EB88(Node3bb8cE *self, s32 *values, char **outArr, char *middle, char **entries)
{
    s32 count;
    char buf[0x20];

    count = 0;
    while (*entries != NULL) {
        strcpy(buf, middle);
        strcat(buf, *entries);
        if (self->methods->slot54(self, *values, buf) != 0) {
            count++;
            *outArr = *entries;
            values++;
            outArr++;
        }
        entries++;
    }
    return count;
}

extern s32 func_8004ECCC(Node3bb8cE *self, u8 id, s32 sizeArg);

s32 func_8004EC5C(Node3bb8cE *self, u8 id, s32 sizeArg)
{
    s32 retries;
    s32 result;

    retries = 10;
    do {
        result = func_8004ECCC(self, id, sizeArg);
    } while (result == 0 && retries-- != 0);
    return result;
}

extern s32 delete(void *arg0);

s32 func_8004ECCC(Node3bb8cE *self, u8 id, s32 sizeArg)
{
    s32 pathBuf[8];
    void *path;
    s32 handle;
    s32 sectors;

    sectors = (u32)(sizeArg + 0x21FF) >> 13;
    path = BuildMemcardPath(pathBuf, self->unkC, &D_8008AAAC);
    handle = open(path, (sectors << 16) | 0x200);
    if (handle == -1) {
        return 0;
    }
    close(handle);
    delete(pathBuf);
    return 1;
}
