# CdDriver__LoadFile -- MATCHED (round 47, alpha)

> Renamed from `Class6D4E8__LoadFile` on 2026-09-26 (tools/rename.py). Address 0x80027800.

> Renamed from `func_80027800` on 2026-09-25 (tools/rename.py). Address 0x80027800.

137/137 words, byte-exact, file 0x18000-0x18224. Cold ground, the largest of
the six assigned before `CdDriver__RunRequestQueue`.

## Source

```c
/* This class's own methods table -- only the two slots CdDriver__LoadFile reads
 * through are named (offsets 0x48/0x64). Added to the unit's shared
 * Obj80027480/Methods80027480 view; re-verified the four sibling functions
 * that already matched against the earlier, narrower Obj80027480 still
 * match after this extension. */
extern void FileResource__LoadFile(void);
extern void *sCdSavedSeekParam;

/* generic doubly-linked-list node, 0x24 bytes (src/cd/CdDriver.c's own
 * reading); only offset 0x0 is touched here -- declared LOCAL, per the
 * project's multiple-local-views convention. */
typedef struct Node8008A894 {
    s32 unk0;
} Node8008A894;

extern Node8008A894 *sCdRequestQueue;
extern void *BMemPMgrAlloc(s32 size);

void CdDriver__LoadFile(Obj80027480 *self, char *arg1) {
    Rec80028448 *rec;
    s32 sectorCount;
    s32 pos;
    void *ret;
    s32 v1;

    if (sCdAsyncEnabled == 0 && gCdSyncQueueMode == 0) {
        FileResource__LoadFile();
        self->unk24 |= 0x200;
        self->methods->slot64(self);
        return;
    }
    LockCd();
    if (self->unk28 != 0) {
        if (sCdBusy == 0 && (self->unk10 == NULL || self->unk20 != 0)) {
            StartCdOperation(4, 1);
            sCdSavedSeekParam = gCdSeekParam;
            rec = FindCdFileEntry(arg1);
            gCdSeekParam = rec;
            if (rec == NULL) {
                return;
            }
            {
                sectorCount = rec->unk18 >> 11;
                sCdReadSectorCount = sectorCount;
                if ((rec->unk18 & 0x7FF) != 0) {
                    sCdReadSectorCount = sectorCount + 1;
                }
                pos = sCdReadSectorCount << 11;
                if (self->unk10 == NULL) {
                    ret = BMemPMgrAlloc(pos);
                    if (ret == NULL) {
                        self->methods->slot48(self);
                        return;
                    }
                    sCdReadBuffer = ret;
                    self->unk10 = ret;
                } else {
                    sCdReadBuffer = self->unk10;
                }
                if (sCdAsyncEnabled != 0) {
                    self->unk14 = pos;
                    gCdTickStep = 2;
                } else {
                retry:
                    do {
                        CdControl(2, (u8 *)gCdSeekParam + 0x14, 0);
                        do {
                            v1 = CdSync(0, 0);
                        } while (v1 == 0);
                    } while (v1 == 5);
                    CdRead(sCdReadSectorCount, self->unk10, 0x80);
                    do {
                        v1 = CdReadSync(0, 0);
                    } while (v1 > 0);
                    if (v1 == -1) {
                        goto retry;
                    }
                    self->unk14 = pos;
                    sCdRequestQueue->unk0 = 1;
                    ResetCdStateMachine();
                }
            }
        }
    } else {
        EnqueueCdRequest(self, FindCdFileIndex(arg1), 7, 0, 0);
    }
    UnlockCd();
}
```

The shared `Obj80027480` struct (top of unit) was extended again: a
`Methods80027480 *methods` field at offset 0 (the class's own method table,
two slots named: `slot48`, `slot64`), plus `unk10` (`void *`, a CdRead
target buffer), `unk14` (`u32`), `unk20` (`u16`), and `unk24` (`s32`, a flags
word, `|=`'d with `0x200`). All purely additive naming of previously-plain
padding; re-verified the four already-matched sibling functions
(`CdDriver__Close`, `CdDriver__Read`, `CdDriver__Seek`, `CdDriver__Open`) still
match after the extension (whole-image SHA1 passed with all five in the
unit).

## What it took, in order

Four attempts, each closing a distinct residue -- listed in the order found,
not necessarily the order that matters most:

1. **The nested-if/else-if-vs-nested-if polarity lever, a FOURTH instance in
   this unit**, on the outermost `self->unk28` split: `if (self->unk28 != 0)
   { if (sCdBusy==0 && ...) {...} } else { EnqueueCdRequest(...); }`, not
   the flattened else-if form.

2. **The SAME polarity lever again, a fifth instance, on `self->unk10 ==
   NULL` vs `!= NULL`**: the fall-through (immediately after the test) must
   be the LONGER continuation (`BMemPMgrAlloc` call and its own nested
   checks), with the trivial one-line store (`sCdReadBuffer = self->unk10;`)
   at the branch target. This is now the clearest pattern in the unit: with
   five confirmed instances across four functions, whichever arm is
   textually longer needs to be read off the `.s` as the fall-through, not
   assumed from either the `if` or the `else` keyword.

3. **A field read TWICE in one statement must be written twice in SOURCE
   TEXT, and pre-loading it into two SEPARATELY NAMED variables is NOT
   enough to stop GCC's CSE** (round 44's lever, but its own wording --
   "caching it in a variable makes GCC compute it once" -- undersells the
   fix here). Retail reads `rec->unk18` twice, once for the `>>11` and once
   for the `&0x7FF` test, into two DIFFERENT registers. Writing
   `fieldA = rec->unk18; fieldB = rec->unk18;` still let GCC prove the two
   loads identical and collapse them to one (1 word short). Only writing
   the field access INLINE at each point of use --
   `sectorCount = rec->unk18 >> 11;` then later `if ((rec->unk18 & 0x7FF)
   != 0)` -- reproduced retail's two separate loads. **The lever is about
   where the EXPRESSION TEXT appears, not about how many named locals
   receive the value.**

4. **A fourth, new-to-this-unit finding: an outer retry loop that WRAPS a
   whole nested retry-loop-plus-a-different-retry-loop needs the
   goto-instead-of-do-while rewrite from `CdDriver__Read`, even in a unit
   where the SAME lever was already shown NOT to transfer to a
   structurally-simpler sibling loop (`CdDriver__Seek`'s report).** This
   function's outermost retry (`while (v1 == -1)`, wrapping a CdControl/
   CdSync retry loop AND a CdRead/CdReadSync retry loop back to back) is
   MORE complex than `CdDriver__Seek`'s CdControl/CdSync-only retry, which
   matched as a plain `do`/`while`. Written as a plain `do`/`while` here,
   GCC's scheduler filled the back-edge branch's delay slot with a
   RECOMPUTED `a0=2` (the next iteration's `CdControl` argument setup) that
   duplicates an already-present, real instruction at the retry target --
   1 word too long -- instead of retail's `li v0,1` (the constant later
   stored through `sCdRequestQueue->unk0`). Rewriting only this OUTERMOST loop
   as `retry: ...; if (v1 == -1) goto retry;` (keeping the two INNER retry
   loops as plain `do`/`while`, since their own hoisted-constant shapes
   already matched) fixed it immediately. **So the goto-vs-do-while choice
   is not just per-loop (round 47's earlier finding) but seemingly
   correlated with how much CODE the loop body contains** -- worth testing
   directly next time a retry loop is long/compound rather than a single
   pair of calls.

5. **The `rec == NULL` early exit must SKIP `UnlockCd()` entirely**,
   jumping straight to the function's true final exit -- not fall through to
   the shared tail. Missing this cost 2 words (a branch target 2 words too
   early) even though the OVERALL function length already matched by that
   point, which is itself worth noting: a single wrong branch TARGET, with
   no length drift anywhere else, can still hide inside an
   otherwise-clean-looking near-full-length diff.

## Struct/behavior notes

`self->unk0` (the class's own vtable pointer, offset 0) has two identified
slots: `slot48` and `slot64`, both `s32 (*)(Obj80027480 *self)` by inference
(neither call's return value is used by this function, so the byte match
does not itself certify the return type -- see the standing wrapper
caution). `rec` (`FindCdFileEntry`'s return) reuses `Rec80028448` from
`CdDriver__Open`'s report; its `unk18` field (`u32`) is read here in the same
shape as there. No caller of `CdDriver__LoadFile` exists yet in carved C, so its
own return type is likewise UNCERTIFIED by any call site; `void` was chosen
because every path either explicitly returns via a callee's forwarded value
(itself never captured anywhere) or falls off the end with `v0` left
unspecified, matching retail's own lack of any `v0`-reset before the shared
epilogue.

### Proposed learning

**A goto-vs-do-while choice for a retry loop is not fully explained by
"check this loop's own bytes" (round 47's earlier correction) -- loop BODY
SIZE/complexity is a second axis worth testing directly.** Two nearly
same-shaped CdControl/CdSync retry loops in this unit (`CdDriver__Seek`'s
simple one, `CdDriver__LoadFile`'s one nested inside a larger combined retry)
took OPPOSITE answers for the SAME inner shape, and the difference tracked
with how much additional code (a second nested retry, more surrounding
statements) the outer loop's body carried. When a retry loop's plain
`do`/`while` produces an extra word that looks like a duplicated
"next-iteration setup" instruction landing in a branch's delay slot instead
of the retail's own unrelated constant, try the goto rewrite before assuming
the earlier negative applies.

## Naming

Round 79 (charlie), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027800` | `CdDriver__LoadFile` | A |

**Evidence.** Overrides the base class's `+0x058` (`FileResource__LoadFile`,
which does open/size/alloc/rewind/read/close through the slots above). Sync
mode calls that base method, then ORs `CD_FLAG_LOAD_FILE_DONE` (0x200) and
calls `setFlag`. Otherwise it enqueues op 7 (`CD_OP_LOAD_FILE`, the name
CdDriver already gives it), or inside a queue dispatch: looks the file
up by name, rounds its size up to whole sectors, allocates `self->buffer`
with `BMemPMgrAlloc` if it has none (calling `close` if that fails), and
seeks + reads the whole file into it, recording the rounded size in
`self->bufferSize`. `CdDriver__RequestLoadFile` (CdDriver) dispatches
this slot as `loadFile`, so the slot name and the function name agree.

**Class prefix.** `Class6D4E8` is the placeholder token for the method
table `gCdDriverMethods` (the convention `CdDriver__RequestLoadFile` and its two
siblings already use); `tools/classtable.py gCdDriverMethods` lists this function
at slot `+0x058`. The prefix names the table, not the developers' class.

## Proposed field names

For the head to apply by type scope (out of unit). `FileResource__LoadFile`
(GameApplicationFileResource), the base method this function overrides and calls in sync
mode, drives these slots in the order open, size query, alloc, rewind, read,
close; the one class that fills them (`gCdDriverMethods`) fills them with the
methods named here.

| unit | type | field | proposed | tier | evidence |
| --- | --- | --- | --- | --- | --- |
| include/GameApplicationFileResource.h | `FileResourceMethods` | `configureBuffer` (+0x44) | `open` | A | called with the file name first; override is `CdDriver__Open` |
| include/GameApplicationFileResource.h | `FileResourceMethods` | `bufferControl` (+0x4C) | `seek` | B | called `(0, 2)` for the size and `(0, 0)` to rewind; override is `CdDriver__Seek` |
| include/GameApplicationFileResource.h | `FileResourceMethods` | `installBuffer` (+0x54) | `read` | A | called with the new buffer and its size; override is `CdDriver__Read` |
| include/GameApplicationFileResource.h | `FileResource` | `pendingGeneration` (+0x0C) | `isOpen` | B | same offset as `Class6D4E8::isOpen`; AllocBuffer zeroes it before calling `open` (which opens only when it is 0) and restores it after `close` |

Also noted for whoever names GameApplicationFileResource again: `FileResource__LoadFile`
opens, sizes, allocates for, reads and closes a named file, i.e. it is the
base-class LoadFile. Not renamed here (out of unit).


Track 4, 2026-09-26 (round 88). The class of gCdDriverMethods (was D_8006D4E8, id 0x13 = DATASOURCE_CD) is CdDriver, in include/CdDriver.h: its ctor calls InitCdDrive, its slots enqueue CD_OP_* requests and drive the CD read state machine, and it is NullDriver's sibling. The object views this function was typed against are replaced by CdDriver, whose fields are all FileResource's (the driver runs on its clients' objects; FileResource's +0x018/+0x01C were named pos/size for it). Byte-identical. `Class6D4E8__LoadFile` -> `CdDriver__LoadFile` by rename.py. The arg-less FileResource__LoadFile call goes through the LoadFileNoArgsFn cast (no code) now that FileResource.h's prototype is in scope.

## History (moved from code_179d8_s.c, round 100)

The comment on `LoadFileNoArgsFn` read, before track 7's pass:

> FileResource__LoadFile (include/FileResource.h) takes (self, name) and
> reads both, but CdDriver__LoadFile passes NEITHER -- retail's jal at
> 0x80027834 has a bare nop delay slot and leaves its own incoming $a0/$a1 in
> place. The call goes through this typedef (a cast of a function address, no
> code); it was a conflicting local `extern void FileResource__LoadFile(void)`
> until round 88.

The source now carries one `MATCHING:` line for it, and one each for the
`goto retry` (item 4 above) and the twice-spelled `entry->size` (item 3).
The inner `{ ... }` block that wrapped the body after the `entry == NULL`
return since round 47 was removed in round 100 (charlie): byte-identical.
The `(u8 *)gCdSeekParam + 0x14` CdlSetloc argument became
`(u_char *)&gCdSeekParam->pos`, Sony's `CdControl` parameter type.
