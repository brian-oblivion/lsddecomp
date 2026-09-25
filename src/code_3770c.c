/*
 * code_3770c -- GAME code carved from psyq_3770c on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x3770C..0x38110 (vram 0x80046F0C..0x80047910). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: the 18 methods of
 * gCdStreamObjMethods.
 *
 * Round 82 matched all 18 methods (the unit has no INCLUDE_ASM left).
 */
#include "common.h"
#include "BasicClass.h"

/* Local view of the gCdStreamObjMethods class: a CD streaming object (StSetRing /
 * StFreeRing / CdSync over libcd). Only the fields this unit's matched
 * methods touch are named. */
typedef struct CdStreamObj CdStreamObj;
typedef struct CdStreamObjMethods CdStreamObjMethods;

struct CdStreamObjMethods {
    BASICCLASS_SLOTS(CdStreamObj, (CdStreamObj *self, s32 arg1, s32 arg2, s32 arg3));
    /* +0x040 */ void (*setRing)(CdStreamObj *self, u32 *ring, u32 size); /* CdStreamObj__SetRing */
    /* +0x044 */ void *slot44;                                            /* CdStreamObj__Open */
    /* +0x048 */ void (*slot48)(CdStreamObj *self);                       /* CdStreamObj__Close */
    /* +0x04C */ void (*seek)(CdStreamObj *self, u8 *loc);              /* CdStreamObj__Seek */
    /* +0x050 */ void *slot50;                                            /* CdStreamObj__StartRead */
    /* +0x054 */ void (*slot54)(CdStreamObj *self);                       /* CdStreamObj__Stop */
    /* +0x058 */ void (*slot58)(CdStreamObj *self);                       /* CdStreamObj__Restart */
    /* +0x05C */ void (*slot5C)(CdStreamObj *self);                       /* CdStreamObj__func_800475C8, empty */
    /* +0x060 */ void (*slot60)(CdStreamObj *self);                       /* CdStreamObj__func_800475D0, empty */
    /* +0x064 */ void (*mute)(CdStreamObj *self);                         /* CdStreamObj__Mute */
    /* +0x068 */ void (*demute)(CdStreamObj *self);                       /* CdStreamObj__Demute */
    /* +0x06C */ void *slot6C;                                            /* CdStreamObj__GetNextFrame */
    /* +0x070 */ u32 (*freeRing)(CdStreamObj *self, u32 *base);           /* CdStreamObj__FreeRing */
    /* +0x074 */ void (*unsetRing)(CdStreamObj *self);                    /* CdStreamObj__UnsetRing */
    /* +0x078 */ void (*clearRing)(CdStreamObj *self);                    /* CdStreamObj__ClearRing */
    /* +0x07C */ void (*slot7C)(CdStreamObj *self);                       /* CdStreamObj__func_800478F8, empty */
};

struct CdStreamObj {
    BASICCLASS_FIELDS(CdStreamObjMethods);
    /* +0x00C */ u8 loc[0x24 - 0x0C];  /* passed to seek (CdStreamObj__Restart) */
    /* +0x024 */ u8 cdResult[8];      /* CdSync result buffer (CdStreamObj__Sync) */
    /* +0x02C */ s32 unk2C;           /* state: 0 idle, 1 seeking, 2, 4 */
    /* +0x030 */ s32 muted;           /* CdStreamObj__Mute / CdStreamObj__Demute */
    /* +0x034 */ s32 unk34;           /* < 4 selects read mode 0x1C0, else 0x140 */
    /* +0x038 */ s32 unk38;           /* ctor: (speed / arg2 / 2) * 2054 */
    /* +0x03C */ s32 unk3C;
    /* +0x040 */ s32 unk40;
    /* +0x044 */ void *cbArg;
    /* +0x048 */ void (*cb48)(void *arg);
    /* +0x04C */ void (*cb4C)(void *arg);
    /* +0x050 */ u32 *ring;           /* StSetRing's ring_addr (CdStreamObj__SetRing) */
    /* +0x054 */ void (*cb54)(void *arg);
    /* +0x058 */ s32 unk58;
};

extern CdStreamObjMethods gCdStreamObjMethods;  /* the class's method table */
CdStreamObjMethods *Get_vtable_CdStreamObj(void);
void OnCdSeekComplete(u8 status, u8 *result);
void CdStreamObj__ReleaseFrame(CdStreamObj *self, u32 *base, u32 frame);
void CdStreamObj__OnStreamEnd(CdStreamObj *self);
s32 SetupCdStreamAudio(CdStreamObj *self);

/* LIBCD.H */
extern void StSetRing(u32 *ring_addr, u32 ring_size);
extern void StClearRing(void);
extern void StUnSetRing(void);
extern u32 StFreeRing(u32 *base);
extern int CdSync(int mode, u8 *result);
extern void *CdSyncCallback(void (*func)(u8 status, u8 *result));
extern int CdControl(u8 com, u8 *param, u8 *result);
extern int CdControlF(u8 com, u8 *param);
extern int CdRead2(u32 mode);
extern int StGetNext(u32 **addr, u32 **header);
extern void *CdSearchFile(void *fp, char *name);

/* libc2 (Sony's, linked) */
extern char *strcpy(char *dest, char *src);
extern char *strcat(char *dest, char *src);

void *func_800270B8(void);   /* code_171e0.c: the data directory string */
extern s32 D_8008A94C;
extern char D_8008A954[];    /* ";1" */
extern void StSetStream(u32 mode, u32 start_frame, u32 end_frame, void (*func1)(), void (*func2)());

extern void *BMemPMgrAlloc(s32 size);
extern CdStreamObj *gActiveCdStreamObj;  /* the active stream object */

/* LIBSPU.H */
typedef struct {
    s16 left;
    s16 right;
} SpuVolume;

typedef struct {
    SpuVolume volume;
    s32 reverb;
    s32 mix;
} SpuExtAttr;

typedef struct {
    u32 mask;
    SpuVolume mvol;
    SpuVolume mvolmode;
    SpuVolume mvolx;
    SpuExtAttr cd;
    SpuExtAttr ext;
} SpuCommonAttr;

extern void SpuSetCommonAttr(SpuCommonAttr *attr);

CdStreamObj *New_CdStreamObj(s32 arg1, s32 arg2, s32 arg3) {
    CdStreamObj *obj = BMemPMgrAlloc(0x5C);

    if (obj != NULL) {
        Get_vtable_CdStreamObj()->ctor(obj, arg1, arg2, arg3);
        return obj;
    }
    return NULL;
}
void CdStreamObj__CdStreamObj(CdStreamObj *self, u32 arg1, s32 arg2, s32 arg3) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_CdStreamObj();
    self->unk34 = arg1;
    self->muted = 0;
    self->unk38 = (((arg1 < 4) ? 300 : 150) / arg2 / 2) * 2054;
    self->unk3C = arg3;
    self->ring = NULL;
    self->cb4C = NULL;
    self->cb48 = NULL;
    self->cb54 = NULL;
    self->unk2C = 0;
}
void CdStreamObj__Finalize(CdStreamObj *self) {
    self->methods->slot48(self);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}
void CdStreamObj__SetRing(CdStreamObj *self, u32 *ring, u32 size) {
    if (self->unk2C == 0) {
        StSetRing(ring, size >> 11);
        self->ring = ring;
    }
}
s32 CdStreamObj__Open(CdStreamObj *self, char *name, s32 tries) {
    char path[0x20];
    s32 n;

    n = tries;
    if (self->unk2C == 0) {
        if (self->ring == NULL) {
            return 1;
        }
        if (gActiveCdStreamObj != NULL) {
            return 0;
        }
        path[0] = '\\';
        strcpy(&path[1], func_800270B8());
        strcat(path, name);
        strcat(path, D_8008A954);
        while (CdSearchFile(self->loc, path) == 0) {
            if (n >= 0 && --tries < 0) {
                return 1;
            }
        }
        self->unk40 = *(u32 *)&self->loc[4] / self->unk38;
        D_8008A94C = SetupCdStreamAudio(self);
        gActiveCdStreamObj = self;
        self->methods->seek(self, self->loc);
        return 0;
    }
    return 1;
}
s32 SetupCdStreamAudio(CdStreamObj *self) {
    SpuCommonAttr attr;

    attr.mask = 0x2C3;
    attr.mvol.left = 0x3FFF;
    attr.mvol.right = 0x3FFF;
    attr.cd.volume.left = 0x7FFF;
    attr.cd.volume.right = 0x7FFF;
    attr.cd.mix = 1;
    SpuSetCommonAttr(&attr);
    return 1;
}
void CdStreamObj__Close(CdStreamObj *self) {
    CdStreamObj *cur;

    if (self->unk2C != 0) {
        cur = gActiveCdStreamObj;
        if (cur == self) {
            cur->methods->slot54(cur);
            cur->unk2C = 0;
            gActiveCdStreamObj = NULL;
        }
    }
}
void CdStreamObj__Seek(CdStreamObj *self, u8 *loc) {
    if (self->unk2C != 2 && gActiveCdStreamObj == self) {
        if (self->cb54 != NULL) {
            CdSyncCallback(OnCdSeekComplete);
            CdControlF(0x15, loc);
        } else {
            while (CdControl(0x15, loc, 0) == 0) {
            }
        }
        self->unk2C = 1;
    }
}
void OnCdSeekComplete(u8 status, u8 *result) {
    if (gActiveCdStreamObj != NULL && status == 2) {
        CdSyncCallback(NULL);
        if (gActiveCdStreamObj->cb54 != NULL) {
            gActiveCdStreamObj->cb54(gActiveCdStreamObj->cbArg);
        }
    }
}
void CdStreamObj__StartRead(CdStreamObj *self, u32 startFrame, s32 arg2) {
    u32 mode;

    if (self->unk2C == 1 && gActiveCdStreamObj == self) {
        mode = 0x140;
        if (self->unk34 < 4) {
            mode = 0x1C0;
        }
        if (arg2 != 0) {
            self->unk40 = arg2;
        }
        self->unk58 = 0;
        StSetStream(0, startFrame, -1, 0, 0);
        self->methods->mute(self);
        while (CdControl(2, self->loc, 0) == 0 || CdRead2(mode) == 0) {
        }
        self->methods->demute(self);
        self->unk2C = 2;
    }
}
void CdStreamObj__Stop(CdStreamObj *self) {
    if (self->unk2C == 2 && gActiveCdStreamObj == self) {
        self->methods->mute(self);
        self->methods->clearRing(self);
        self->methods->unsetRing(self);
        while (CdControl(9, 0, 0) == 0) {
        }
        self->unk2C = 4;
    }
}
void CdStreamObj__Restart(CdStreamObj *self) {
    CdStreamObj *cur;

    if (self->unk2C == 4) {
        cur = gActiveCdStreamObj;
        if (cur == self) {
            cur->unk2C = 0;
            cur->methods->seek(cur, cur->loc);
        }
    }
}
void CdStreamObj__func_800475C8(CdStreamObj *self) {
}
void CdStreamObj__func_800475D0(CdStreamObj *self) {
}
void CdStreamObj__Mute(CdStreamObj *self) {
    if (self->muted == 0 && gActiveCdStreamObj == self) {
        while (CdControl(0xB, 0, 0) == 0) {
        }
        self->muted = 1;
    }
}
void CdStreamObj__Demute(CdStreamObj *self) {
    if (self->muted != 0 && gActiveCdStreamObj == self) {
        while (CdControl(0xC, 0, 0) == 0) {
        }
        self->muted = 0;
    }
}
s32 CdStreamObj__GetNextFrame(CdStreamObj *self, u32 **addr, u32 *frame, s32 tries) {
    u32 *header;
    u32 n;

    if (tries < 0) {
        tries = 0x800000;
    }
    while (StGetNext(addr, &header) != 0) {
        if (--tries < 0) {
            self->methods->freeRing(self, (u32 *)addr);
            return 0;
        }
    }
    n = header[2];
    *frame = n;
    if (self->unk40 > 0) {
        if (n >= self->unk40 || n < self->unk58) {
            if (n < self->unk58) {
                *frame = 0;
            }
            CdStreamObj__ReleaseFrame(self, *addr, *frame);
            CdStreamObj__OnStreamEnd(self);
            return -1;
        }
        self->unk58 = n;
    }
    CdStreamObj__ReleaseFrame(self, *addr, *frame);
    return 1;
}
void CdStreamObj__ReleaseFrame(CdStreamObj *self, u32 *base, u32 frame) {
    if (self->cb48 != NULL) {
        self->cb48(self->cbArg);
        self->methods->freeRing(self, base);
    }
}
void CdStreamObj__OnStreamEnd(CdStreamObj *self) {
    if (self->cb4C != NULL) {
        self->cb48(self->cbArg);
        self->methods->slot48(self);
    }
}
u32 CdStreamObj__FreeRing(CdStreamObj *self, u32 *base) {
    return StFreeRing(base);
}
void CdStreamObj__UnsetRing(CdStreamObj *self) {
    StUnSetRing();
}
void CdStreamObj__ClearRing(CdStreamObj *self) {
    StClearRing();
}
int CdStreamObj__Sync(CdStreamObj *self, int mode) {
    return CdSync(mode, self->cdResult);
}
void CdStreamObj__func_800478F8(CdStreamObj *self) {
}
CdStreamObjMethods *Get_vtable_CdStreamObj(void) {
    return &gCdStreamObjMethods;
}
