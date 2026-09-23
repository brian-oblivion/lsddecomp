/*
 * class_3ac78 -- the front half of Class866E8, the class whose method table is
 * D_800866E8 (80 slots, header 0x114; tools/classtable.py D_800866E8). It
 * derives from Class6B5CC (code_d294) through the intermediate base whose
 * table func_800428E4 returns (D_8006EFAC), and the game builds exactly one,
 * at boot, in class_39e08's func_80049684 via New_Class866E8(0, 1).
 *
 * What it manages is a GRID. The object owns seven elements (elems[7]), each
 * pairing a target object, a list, a parent node, and a 0x668-byte heap block
 * holding that element's grid of cell objects; the constructor seeds every
 * cell with a world position on a 0x800 lattice. Indexing the grid uses a row
 * stride of 20 cells -- the same 20 that gDefaultGridSpan >> 11 produces
 * (0xA000 / 0x800, see Class866E8__SetGridSpan) and the same stride
 * class_3bb8c_b's byte-matched func_8004CE24 walks.
 *
 * Work reaches the cells through a rectangle list (rects[4]/rectCount): a
 * notification arrives at Class866E8__OnNotify or Class866E8__OnCommand,
 * Class866E8__ForwardAcceptedCommand filters the sender against acceptedTags,
 * Class866E8__ApplyToSenderFootprint turns the sender's position into one
 * rectangle, and Class866E8__DispatchToRectCells re-notifies every cell in it
 * and every cell chained behind it. The queries that build those rectangles,
 * and an element's resource and GPU sides, live in class_3bb8c*, which keeps
 * its own independent view of the same object (Obj866E8 / Elem /
 * GridSlot866E8 in include/class_3bb8c.h).
 *
 * Every function in the unit is matched C; the last three stalls
 * (Class866E8__ResetAllElements, Class866E8__SetFootprintRect and
 * Class866E8__DispatchToRectCells) were matched in round 71. func_8004B324 keeps its placeholder name
 * deliberately -- it is an empty vtable stub with no established purpose, the
 * same case as func_8001D33C in code_d294_b.
 */
#include "common.h"
#include "class_3ac78.h"

void Class86668__SetChildFlag8(Class86668 *self, s32 value)
{
    Class866E8 *sub = self->unk34;

    if (sub != NULL) {
        sub->methods->setFlag8(sub, value, 0x7F, 0x7F);
    }
}

Class86668Methods *GetClass86668Methods(void)
{
    return &gClass86668Methods;
}

Class866E8 *New_Class866E8(s32 arg1, s32 arg2)
{
    Class866E8 *self;

    self = func_80017B34(0x1E8);
    if (self != NULL) {
        func_8004D244()->ctor(self, arg1, arg2);
        return self;
    }
    return NULL;
}

/*
 * Class866E8__Class866E8's own helpers -- all still-uncarved elsewhere, typed
 * purely from this call site's own register usage.
 */
typedef struct BaseCtorTable_3ac78 BaseCtorTable_3ac78;
struct BaseCtorTable_3ac78 {
    u8 pad0[0x8];
    void (*ctor)(void *self); /* +0x008, standard "further-base ctor first" slot */
    void (*dtor)(void *self); /* +0x00C, Class866E8__Finalize: standard "further-base dtor" slot, mirroring ctor */
};

extern BaseCtorTable_3ac78 *func_800428E4(void);
extern UnkSlotChildObj_3ac78 *func_80048894(void);
extern UnkSlotListObj_3ac78 *new_class_6d940(s32 arg1);
extern GenericObject *New_Class86AA0(void);
extern s32 func_80020C5C(void);
extern void func_80017CFC(void *arg1);
extern Vec3_3ac78 gDefaultOrigin;

void Class866E8__Class866E8(Class866E8 *self, Vec3_3ac78 *arg1, s32 arg2)
{
    s32 i;
    UnkSlotEntry_3ac78 *entry;
    GenericObject *obj;
    Class866E8 **cellp;
    u8 *p;
    u8 *end;
    s32 buf[3];

    func_800428E4()->ctor(self);
    self->methods = func_8004D244();

    if (arg1 != NULL) {
        self->origin = *arg1;
    } else {
        self->origin = gDefaultOrigin;
    }

    self->unk1B0 = 0;
    self->unk1B4 = 0;
    self->unk1B8 = 0;
    self->enabled = 0;
    self->unk6C = 0;
    self->acceptedTags = 0;
    self->unk1E0 = 0;

    for (i = 0; i < 7; i++) {
        entry = &self->elems[i];

        entry->target = func_80048894();
        entry->target->unk20 = (entry->target->unk10 != 0);
        entry->target->key = i;
        entry->target->methods->slot88(entry->target, arg2);

        entry->heldObj = NULL;
        entry->unk18 = 0;
        entry->key = i;
        entry->flag = 0;

        entry->list = new_class_6d940(0);
        entry->cellParent = New_Class86AA0();
        entry->cellParent->methods->slot4C(entry->cellParent, self, &self->origin);

        entry->cells = (Class866E8 **)func_80017B34(0x668);
        if (entry->cells == NULL) {
            return;
        }

        buf[0] = 0x400;
        buf[1] = 0;
        buf[2] = 0x400;

        cellp = entry->cells;
        end = (u8 *)cellp + 0x668;
        p = (u8 *)cellp;
        while (p < end) {
            obj = New_Class86AA0();
            *(GenericObject **)p = obj;
            obj->methods->slot4C(obj, entry->cellParent, buf);

            buf[0] += 0x800;
            if (buf[0] > 0xA400) {
                buf[0] = 0x400;
                buf[2] += 0x800;
            }

            obj = *(GenericObject **)p;
            obj->methods->slot70(obj, 1);
            obj = *(GenericObject **)p;
            p += 4;
            obj->unk10 |= 0x80000000;
        }
    }

    self->methods->addChild(self, func_80020C5C());
    self->methods->reset(self);
}

void Class866E8__Finalize(Class866E8 *self)
{
    s32 i;
    UnkSlotEntry_3ac78 *entry;
    GenericObject *obj;
    Class866E8 **cellp;
    u8 *p;
    u8 *end;

    self->methods->removeChild(self, (void *)func_80020C5C());

    for (i = 0; i < 7; i++) {
        entry = &self->elems[i];
        self->methods->onElementEvent(self, 6, entry, i);

        if (entry->target != NULL) {
            entry->target->methods->release(entry->target);
        }

        if (entry->list != NULL) {
            if (entry->list->unk2C != NULL) {
                entry->list->unk2C->methods->release(entry->list->unk2C);
            }
            entry->list = (UnkSlotListObj_3ac78 *)entry->list->methods->release(entry->list);
        }

        if (entry->cellParent != NULL) {
            entry->cellParent->methods->release(entry->cellParent);
        }

        cellp = entry->cells;
        end = (u8 *)cellp + 0x668;
        p = (u8 *)cellp;
        while (p < end) {
            obj = *(GenericObject **)p;
            if (obj != NULL) {
                obj->methods->release(obj);
            }
            p += 4;
        }

        func_80017CFC(entry->cells);
    }

    func_800428E4()->dtor(self);
}

/* MEASURED, round 9: GetClass6B5CCMethods TAKES NO ARGUMENTS -- its body is
 * `lui/addiu %hi/%lo(D_8006B5CC); jr $ra` and it reads neither $a0 nor $a1
 * (asm/code_d294.s). The two args below are what THIS call site passes, not
 * the callee's signature; include/class_3bb8c.h passes ONE to the same symbol
 * and is equally byte-exact. Retail's source called one zero-argument getter
 * with different argument counts from different files, which is what C89 does
 * with no prototype in scope. Do not reconcile the two declarations. */
/* ROUND 59 (extern review), refining the paragraph above rather than
 * overturning it. Round 9's own conclusion is "no prototype in scope", and
 * the faithful spelling of that is an UNSPECIFIED parameter list, not a
 * fabricated two-parameter prototype -- which is what the line below now is.
 * Measured before changing it: the two arguments this unit passes cost ZERO
 * bytes. Retail's `jal 8001e57c` in Class866E8__OnNotify (0x8004A9A0) and
 * Class866E8__OnElementEvent (0x8004AA88) both carry `move s2,a2` in the delay slot -- a
 * callee-save spill, NOT argument setup -- and $a0/$a1 still hold each
 * caller's own incoming arguments. So unlike the other round-59 findings,
 * nothing here is byte-load-bearing, and the arity claim was simply false
 * against the definition (src/code_d294_b.c:736, `(void)`). The call sites
 * below are UNCHANGED; only this declaration is. */
extern void *GetClass6B5CCMethods();

void Class866E8__OnNotify(Class866E8 *self, GenericObject *sender, s32 command)
{
    void (*fn)(Class866E8 *self, GenericObject *sender, s32 command);

    fn = *(void (**)(Class866E8 *, GenericObject *, s32))
        ((u8 *)GetClass6B5CCMethods(self, (s32)sender) + 0x38);
    fn(self, sender, command);

    if ((sender->methods->header & 0xF) == 1) {
        self->methods->slot100(self, sender, command);
    }
}

extern s32 gDefaultGridSpan;

void Class866E8__Reset(Class866E8 *self)
{
    self->config = NULL;
    self->acceptedTags = 0;
    self->rectCount = 0;
    self->methods->setGridSpan(self, gDefaultGridSpan);
    self->unk1CC = -1;
    self->unk1D0 = -1;
    self->unk1D4 = -1;
    self->unk1D8 = -1;
}

void Class866E8__OnElementEvent(Class866E8 *self, s32 command, UnkSlotEntry_3ac78 *elem)
{
    void (*fn)(Class866E8 *self, s32 command);

    fn = *(void (**)(Class866E8 *, s32))((u8 *)GetClass6B5CCMethods(self, command) + 0x88);
    fn(self, command);

    if (command == 6)
        goto handle6;
    if (command == 7)
        goto merge;
    return;

handle6:
    if (elem->heldObj != NULL) {
        elem->heldObj = elem->heldObj->methods->release(elem->heldObj);
    }

merge:
    self->lastEventElem = elem;
    self->methods->notifyParents(self, command);
}

void Class866E8__UpdateIfEnabled(Class866E8 *self)
{
    if (self->enabled) {
        self->methods->slotF4(self);
        self->methods->slot13C(self);
    }
}

void Class866E8__OnCommand(Class866E8 *self, GenericObject *sender, s32 command)
{
    if ((u8)sender->methods->header == 0x34) {
        self->methods->forwardAcceptedCommand(self, sender, command);
    }
}

/* Reset every one of the seven grid elements, then the two counters.
 * Matched round 71: `&self->elems[i]` is what produces retail's
 * base + running-offset walk (GCC's strength reduction), not a hand-rolled
 * byte offset. */
void Class866E8__ResetAllElements(Class866E8 *self)
{
    s32 i;
    UnkSlotEntry_3ac78 *entry;
    UnkSlotListObj_3ac78 *list;

    for (i = 0; i < 7; i++) {
        entry = &self->elems[i];
        entry->target->methods->slot74(entry->target);
        entry->flag = 0;
        self->methods->slot108(self, entry);
        list = entry->list;
        if (list->unk2C != NULL) {
            list->unk2C = list->unk2C->methods->release(list->unk2C);
        }
        self->methods->onElementEvent(self, 6, entry, i);
        entry->target->methods->slot84(entry->target);
    }

    self->unk1B8 = 0;
    self->unk1B4 = 0;
    self->methods->slot140(self);
}

void Class866E8__SetChildParams(Class866E8 *self, s32 count, s32 arg2, s32 arg3)
{
    s32 i;
    UnkChildObj_3ac78 *child;

    for (i = 0; i < count; i++) {
        child = self->methods->getChild(self, i);
        child->methods->slot44(child, 1, arg3);
        arg3 += 3;
        child->methods->slot48(child, 1, arg2);
        arg2 += 6;
    }
}

void Class866E8__SetCallback(Class866E8 *self, s32 fn, s32 ctx)
{
    self->valueFn = fn;
    self->valueFnCtx = ctx;
}

void Class866E8__SetAcceptedTags(Class866E8 *self, s32 tags)
{
    self->acceptedTags = tags;
}

void Class866E8__ForwardAcceptedCommand(Class866E8 *self, void *sender, s32 command)
{
    s32 *p;
    u8 unused[24];

    switch (command) {
    case 2:
    case 3:
    case 5:
    case 6:
    case 7:
    case 8:
        break;
    default:
        return;
    }

    p = (s32 *)self->acceptedTags;
    if (p == NULL)
        return;
    if (*p == 0)
        return;

    do {
        if (*p == ((GenericObject *)sender)->methods->header) {
            self->methods->applyToSenderFootprint(self, sender, command);
        }
        p++;
    } while (*p != 0);
}

extern void Class866E8__SetFootprintFromCell(Class866E8 *self, UnkArgObj_3ac78 *desc, s32 span);
extern void Class866E8__SetFootprintRect(Class866E8 *self, UnkArgObj_3ac78 *desc, s32 span);
extern void Class866E8__DispatchToRectCells(Class866E8 *self, UnkListObj_3ac78 *sender, s32 command);

void Class866E8__ApplyToSenderFootprint(Class866E8 *self, UnkListObj_3ac78 *sender, s32 command)
{
    s32 gateArg;
    GridRectList_3ac78 saved;
    UnkArgObj_3ac78 buf;
    s32 savedRectCount;

    if (sender->unk0C != 0) {
        gateArg = (s32)((u8 *)sender->unk14 + 0x38);
    } else {
        gateArg = 0;
    }

    if (self->methods->slot110(self, &buf, gateArg) != 0) {
        return;
    }

    savedRectCount = self->rectCount;
    saved = self->rects;

    if (self->config->unk4 == 0) {
        Class866E8__SetFootprintFromCell(self, &buf, 3);
    } else {
        Class866E8__SetFootprintRect(self, &buf, 3);
    }

    Class866E8__DispatchToRectCells(self, sender, command);

    self->rectCount = savedRectCount;
    self->rects = saved;
}

extern void func_8004C93C(Class866E8 *self);

void Class866E8__SetFootprintFromCell(Class866E8 *self, UnkArgObj_3ac78 *desc, s32 span)
{
    s16 t;

    self->footprintCol = desc->unk2 - 1;
    t = desc->unk3 - 1;
    self->footprintW = span;
    self->footprintH = span;
    self->footprintRow = t;
    func_8004C93C(self);
}

/* Clamp a span x span footprint centred on desc's cell to the 20 x 20 grid:
 * a cell on the low edge (0) loses one row/column, one on the high edge
 * (0x13) loses one too. The edge tests read a COPY of each byte taken before
 * the decrement, and the height companion is `span` itself. Matched round 71. */
void Class866E8__SetFootprintRect(Class866E8 *self, UnkArgObj_3ac78 *desc, s32 span)
{
    s32 col;
    s32 row;
    s32 width;
    s32 origCol;
    s32 origRow;

    width = span;
    col = desc->unk2;
    row = desc->unk3;
    origCol = col;
    origRow = row;

    if (col == 0) {
        width = span - 1;
    } else {
        col--;
    }
    if (origCol == 0x13) {
        width--;
    }

    if (origRow == 0) {
        span--;
    } else {
        row--;
    }
    if (origRow == 0x13) {
        span--;
    }

    self->rectCount = 1;
    self->rects.e[0].elemIdx = self->methods->slot124(self, desc->unk28);
    self->rects.e[0].col = col;
    self->rects.e[0].row = row;
    self->rects.e[0].width = width;
    self->rects.e[0].height = span;
}

extern void NotifyGridCell(Class866E8 *cell, UnkListObj_3ac78 *sender, s32 command);

/* Notify every cell of every rectangle, and every object chained behind
 * each cell. Matched round 71: the ORDER of the comma-separated increments
 * is load-bearing in both loops (`entry++, i++` and `cell++, col++`); the
 * reverse order was the whole 95/117 residue. */
void Class866E8__DispatchToRectCells(Class866E8 *self, UnkListObj_3ac78 *sender, s32 command)
{
    s32 i;
    s32 row;
    s32 col;
    GridRect_3ac78 *entry;
    UnkSlotEntry_3ac78 *slot;
    Class866E8 **cell;
    Class866E8 *obj;

    entry = self->rects.e;
    for (i = 0; i < self->rectCount; entry++, i++) {
        slot = &self->elems[entry->elemIdx];
        if (slot->target->unk2C != 0) {
            cell = (slot->cells + entry->col) + entry->row * 20;
            for (row = 0; row < entry->height; row++) {
                for (col = 0; col < entry->width; cell++, col++) {
                    self->curCellTag = self->cellTag;
                    self->curCellCol = entry->col + col;
                    self->curCellRow = entry->row + row;
                    NotifyGridCell(*cell, sender, command);
                    for (obj = (*cell)->nextInCell; obj != NULL; obj = obj->nextInCell) {
                        NotifyGridCell(obj, sender, command);
                    }
                }
                cell += 20 - entry->width;
            }
        }
    }
}

/* Widened this round (Class866E8__DispatchToRectCells) from a single-param signature to
 * accept two more, unused, forwarded params: Class866E8__DispatchToRectCells's own call
 * sites explicitly set up $a1/$a2 before every call here (unlike
 * GetClass6B5CCMethods's "leftover, already-there" args -- these are real,
 * explicit `move` instructions), so the call itself needs a matching
 * 3-param prototype to compile. Confirmed harmless to THIS function's own
 * already-matched body: neither extra param is read, and GCC does not
 * reserve stack space for unused trailing integer/pointer args on this
 * target, so the definition's own bytes are unaffected (reverified
 * 18/18 after the widening). */
void NotifyGridCell(Class866E8 *cell, UnkListObj_3ac78 *sender, s32 command)
{
    if (cell != NULL && (cell->flags36 & 0x80)) {
        cell->methods->onNotify(cell);
    }
}

void *Class866E8__GetCurrentCellKey(Class866E8 *self)
{
    return &self->curCellTag;
}

void func_8004B324(void) {
}

void Class866E8__SetGridSpan(Class866E8 *self, s32 span)
{
    self->gridSpan = span;
    self->gridCells = (s16)(span >> 11);
    self->gridHalfCells = (s16)(span >> 12);
}

void Class866E8__SetConfig(Class866E8 *self, UnkPtr68Obj_3ac78 *config)
{
    self->methods->reset(self);
    self->config = config;
}
