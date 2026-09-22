#include "common.h"
#include "Class6D3C8.h"

/* The `New_X` allocator for the class whose method table is D_8006D3C8:
 * allocates a 0x2C-byte instance and, on success, runs the class's own
 * constructor through slot +0x008 of the table GetClass6D3C8Methods() returns.
 *
 * The null path deliberately falls off the end rather than returning a
 * value. That is not an oversight in the transcription -- it is what
 * retail does, and it is the ONLY form that matches. `func_80017B34`
 * already left the null in $v0, so the original source never had to
 * restate it; every spelling that returns explicitly on that path
 * (`return 0`, `return self`, an early return, a goto to a shared exit)
 * costs an extra instruction that retail does not have. GCC 2.6.3 warns
 * "control reaches end of non-void function" here, and the warning is
 * correct about the C -- the bytes are what say the original had it too.
 * See docs/match-reports/new_class_6d3c8.md for the full derivation. */
Class6D3C8 *new_class_6d3c8(Class6D3C8CtorArgs *arg) {
    Class6D3C8 *self = func_80017B34(0x2C);

    if (self != 0) {
        ((Class6D3C8Methods *)GetClass6D3C8Methods())->ctor(self, arg);
        return self;
    }
}

/* Constructs a Class6D3C8 instance: runs the intermediate base class's own
 * constructor (through its ctor slot), installs this class's own vtable,
 * stores the ctor argument, loads the "ETC\DREAME5.TMD" model, builds this
 * object's owned DreamSys from it, dispatches one DreamSys init call, then
 * runs this class's own slot40 (func_800260A4) once. */
void func_80025FDC(Class6D3C8 *self, Class6D3C8CtorArgs *arg) {
    LoadModelRequest req;

    func_8003B20C()->ctor(self, arg->unk00);
    self->methods = GetClass6D3C8Methods();
    self->arg = arg;
    func_800270AC(func_80048CF0());
    req.type = 0;
    req.path = D_800107A4;
    self->dreamSys = New_DreamSys(func_80043840(&req), 0, 0);
    self->unk24 = 0;
    self->dreamSys->vt->func_228(self->dreamSys, arg->unk14);
    self->methods->slot40(self);
}

extern void func_80048CFC(s32 day, s32 unused);

/* Advances the day cursor: reads the running tick count kept in scratchpad
 * (0x1F800000, the PS-X data-cache-as-RAM region) and reduces it mod 365. */
void func_800260A4(void) {
    func_80048CFC(*(s32 *)0x1F800000 % 365, 0);
}

/* Defers to the base class's own implementation of this slot when this
 * object hasn't been given an override (unk18 == 0). */
void func_80026108(Class6D3C8 *self, void *a1, void *a2) {
    if (self->unk18 == 0) {
        func_8003B20C()->slot44(self, a1, a2, 0);
    }
}

/* Optional stream-load block, gated by self->arg->unk0C: registers a
 * "loader" task for "ETC\ASMKLOGO.TIM" (func_80026254), then a separate
 * "stream" task for whatever type code func_800490F4 hands back
 * ("ETC\ASMK.STR"), then a second loader task for "ETC\OSDLOGO.TIM". */
void func_80026170(Class6D3C8 *self) {
    const char *streamName;
    s32 typeCode;
    s32 typeLookup;
    StreamTask *task;

    if (self->arg->unk0C != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        func_80026254(self, D_800107B4);
        task = func_8003B854(0, 0, 0, 0);
        streamName = func_800490F4(&typeCode);
        typeLookup = func_800493C8(typeCode);
        task->methods->slot44(task, self->unk1C, streamName, typeLookup, 1);
        task->methods->slot4(task);
        func_80026254(self, D_800107C8);
    }
}

/* Registers a "loader" task for the given resource path: allocates the
 * task, gives it a completion callback (func_80026328) and context
 * (self), then sets its remaining parameters (path, self->unk1C) and
 * starts it. */
void func_80026254(Class6D3C8 *self, const char *path) {
    LoaderTask *task = func_8003BE94(0, 0, 0);

    task->methods->slot98(task, func_80026328, self);
    task->methods->slot6C(task, 0);
    task->methods->slotD4(task, path, 0);
    task->methods->slot44(task, self->unk1C, 0);
    task->methods->slot4(task);
}

extern s32 func_8004A070(s32 a0);

s32 func_80026328(void) {
    return func_8004A070(0);
}

/* Optional stream-task init block, gated by self->arg->unk08 (the same
 * shape as func_80026170's self->arg->unk0C gate, minus the two
 * func_80026254 loader-task calls, and using func_8004913C instead of
 * func_800490F4 to derive the type code). */
void func_80026348(Class6D3C8 *self) {
    s32 derivedValue;
    s32 typeCode;
    s32 typeLookup;
    StreamTask *task;

    if (self->arg->unk08 != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        task = func_8003B854(0, 0, 0, 0);
        derivedValue = func_8004913C(&typeCode, 0);
        typeLookup = func_800493C8(typeCode);
        task->methods->slot44(task, self->unk1C, derivedValue, typeLookup, 1);
        task->methods->slot4(task);
    }
}

/* Gated by self->arg->unk10. Checks the DreamSys's own status slot
 * (+0x1A0); if it isn't already "1" and self->unk24 hasn't latched, kicks
 * off one PollTask (func_80057F68) and, if THAT reports "2", runs
 * func_8002658C. Then polls a second PollTask (New_Class86B60) in a loop,
 * restarting the first PollTask each time it reports "2", until it
 * reports anything else; clears self->unk24 and returns 0 or 2 depending
 * on whether that final status was below 1. */
s32 func_80026410(Class6D3C8 *self) {
    s32 status;
    s32 pollDone;

    if (self->arg->unk10 != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);

        status = self->dreamSys->vt->DreamSys__GetCurrentDayAndYear(self->dreamSys, 0);
        if (status != 1) {
            if (self->unk24 == 0) {
                status = func_80026518(func_80057F68, self->dreamSys, self->unk1C);
                if (status == 2) {
                    func_8002658C(self);
                }
            }
        }

        pollDone = 2;
    retry:
        status = func_80026518(New_Class86B60, self->dreamSys, self->unk1C);
        if (status == pollDone) {
            func_80026518(func_80057F68, self->dreamSys, self->unk1C);
            goto retry;
        }

        self->unk24 = 0;
        return ((u32)status < 1) << 1;
    }
    return 2;
}

/* Constructs a PollTask via the caller-supplied `ctor`, dispatches
 * slot44(task, extra, 0) and slot4(task) on it (fire-and-forget), and
 * returns slot44's result. */
s32 func_80026518(PollTaskCtor ctor, void *dreamSys, s32 extra) {
    PollTask *task = ctor(dreamSys);
    s32 result = task->methods->slot44(task, extra, 0);

    task->methods->slot4(task);
    return result;
}

/* Called by func_80026410 when its first PollTask reports "2". Gated by
 * self->arg->unk08 (same gate as func_80026348). Builds a StreamTask,
 * derives a count via func_800493E4, initializes the task with that
 * count's quotient-by-9 and a fixed sub-slot, then a 5-argument slot44
 * call (a3 = -1, unlike the other slot44 call sites), then starts it. */
void func_8002658C(Class6D3C8 *self) {
    StreamTask *task;
    struct {
        u32 unk00;
        u32 unk04;
        u32 count;
    } buf;
    s32 extra;

    if (self->arg->unk08 != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        task = func_8003B854(0, 0, 0, 0);
        extra = func_800493E4(&buf.count, 0, 10);
        task->methods->slot6C(task, buf.count / 15);
        task->methods->slot12C(task, 0);
        task->methods->slot44(task, self->unk1C, extra, -1, 1);
        task->methods->slot4(task);
    }
}

void func_80026690(void) {
}

/* Builds a StatusObj, dispatches slot44(obj) then reads slot4(obj)'s
 * return as a status code: 2 runs func_8002677C, 3 latches self->unk24.
 * Then queries the DreamSys status slot again (as func_80026410 does),
 * this time passing an out-param, and derives a 0/1 result from both the
 * call's return and the out-param. */
/* Builds a StatusObj for this instance's current state, reads one status
 * code off it, tears it down, and reacts to two of the codes. Then asks the
 * owned DreamSys a question and reports whether its answer was 1.
 *
 * Two things here were long-standing misreadings, both worth keeping written
 * down (docs/match-reports/func_80026698.md):
 *
 *  - `case 3` stores 1, NOT 3. Retail's `li $v0, 0x1` sits in the delay slot
 *    of the case-3 branch, so it executes before the jump is taken and $v0
 *    holds 1 -- not the 3 it held for the comparison -- by the time the
 *    store runs. Reading the store as `unk24 = 3` (the discriminant) was
 *    what produced the old 53/57 and the "the compiler materialises an
 *    unused default-arm constant" theory attached to it. There is no unused
 *    constant: `li $v0, 0x1` is the value being stored, hoisted into a delay
 *    slot on the only path that needs it.
 *  - `result = (check == 1)` is the whole comparison. GCC 2.6.3 lowers an
 *    equality test against a small constant to `xori` + `sltiu`, which reads
 *    back out of the disassembly as `(u32)(check ^ 1) < 1`. That transcription
 *    is arithmetically right and cost two instructions; the plain `== 1` is
 *    what the source said. */
s32 func_80026698(Class6D3C8 *self) {
    s32 status;
    StatusObj *obj;
    s32 outVal;
    s32 check;
    s32 result;

    obj = func_80049608(self->unk1C, self->dreamSys, self->arg->unk04);
    status = obj->methods->slot44(obj);
    obj->methods->slot4(obj);

    switch (status) {
    case 2:
        func_8002677C(self);
        break;
    case 3:
        self->unk24 = 1;
        break;
    }

    check = self->dreamSys->vt->DreamSys__GetCurrentDayAndYear(self->dreamSys, &outVal);
    result = 0;
    if (outVal != 0) {
        result = (check == 1);
    }
    return result;
}

/* Reads DreamSys's current cinematic slot, resolves it to a channel index
 * (func_80049334); if that fails (-1), starts a LoaderTask on the fixed
 * "no cinematic" path; otherwise, if self->arg->unk08 gates it, starts a
 * StreamTask on the resolved channel. Either branch finishes by starting
 * whichever task it built; if neither branch runs, nothing happens. */
void func_8002677C(Class6D3C8 *self) {
    CinematicCall cc;
    struct {
        s32 chan;
        u32 unk04;
        u32 unk08;
    } chanBuf;
    s32 groupId;
    s32 lookup;
    LoaderTask *task;

    cc = self->dreamSys->vt->GetCinematic(self->dreamSys);
    groupId = func_80049334(&chanBuf.chan, (u16) cc.bank | ((u32) (u16) cc.entry << 16));
    SetActiveDataSourceDriverMode(0, 0, 0);

    if (chanBuf.chan != -1) {
        if (self->arg->unk08 != 0) {
            StreamTask *streamTask = func_8003B854(0, 0, 0, 0);

            streamTask->methods->slot12C(streamTask, 0);
            lookup = func_800493C8(chanBuf.chan);
            streamTask->methods->slot44(streamTask, self->unk1C, groupId, lookup, 1);
            streamTask->methods->slot4(streamTask);
        }
    } else {
        task = func_8003BE94(0, 0, 0);
        task->methods->slot6C(task, 10);
        task->methods->slotD4(task, groupId, 0);
        task->methods->slot44(task, self->unk1C, 0);
        task->methods->slot4(task);
    }
}

/* Class6D3C8Methods slot +0x064. Gated by self->arg->unk08 (same gate as
 * func_80026348/func_8002658C). Builds a StreamTask, runs its slot12C,
 * derives a type code via func_800491FC, looks it up via func_800493C8,
 * initializes the task with it, then starts it -- the same shape as
 * func_80026170/func_80026348, but with slot12C added and func_800491FC
 * in place of func_800490F4/func_8004913C. */
void func_80026900(Class6D3C8 *self) {
    StreamTask *task;
    s32 typeCode;
    s32 outerValue;
    s32 typeLookup;

    if (self->arg->unk08 != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        task = func_8003B854(0, 0, 0, 0);
        task->methods->slot12C(task, 0);
        outerValue = func_800491FC(&typeCode, 0);
        typeLookup = func_800493C8(typeCode);
        task->methods->slot44(task, self->unk1C, outerValue, typeLookup, 1);
        task->methods->slot4(task);
    }
}
