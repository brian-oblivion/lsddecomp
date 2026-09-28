# CdDriver__Seek -- MATCHED (round 47, alpha)

> Renamed from `Class6D4E8__Seek` on 2026-09-26 (tools/rename.py). Address 0x80027528.

> Renamed from `func_80027528` on 2026-09-25 (tools/rename.py). Address 0x80027528.

104/104 words, byte-exact, file 0x17D28-0x17EC8. Cold ground.

## Source

```c
extern u8 gCdSeekLoc[8];
extern void *gCdSeekParam;
extern s32 gCdTickStep;

extern s32 GetCdFileSize(Obj80027480 *self);
extern s32 CdPosToInt(void *pos);
extern void CdIntToPos(s32 i, void *pos);
extern void CdControl(s32 arg0, void *buf, s32 arg2);
extern s32 CdSync(s32 mode, void *result);

s32 CdDriver__Seek(Obj80027480 *self, u32 arg1, s32 arg2) {
    s32 v0;
    u32 s0tmp;

    if (gCdAsyncEnabled == 0 && gCdSyncQueueMode == 0) {
        return GetCdFileSize(self);
    }
    LockCd();
    if (self->unk28 != 0) {
        if (gCdBusy == 0 && self->unk0C != 0) {
            StartCdOperation(2, 1);
            s0tmp = arg1 >> 11;
            if ((arg1 & 0x7FF) != 0) {
                s0tmp = s0tmp + 1;
            }
            v0 = CdPosToInt(self->unk18);
            CdIntToPos(v0 + s0tmp, gCdSeekLoc);
            if (arg2 == 0) {
                if (gCdAsyncEnabled != 0) {
                    gCdSeekParam = gCdSeekLoc - 0x14;
                    gCdTickStep = 1;
                } else {
                    do {
                        CdControl(2, gCdSeekLoc, 0);
                        do {
                            v0 = CdSync(0, 0);
                        } while (v0 == 0);
                    } while (v0 == 5);
                    ResetCdStateMachine();
                }
            } else {
                ResetCdStateMachine();
                UnlockCd();
                if ((self->unk1C & 0x7FF) != 0) {
                    return ((self->unk1C >> 11) + 1) << 11;
                }
                return self->unk1C;
            }
        }
    } else {
        EnqueueCdRequest(self, 0, 4, (s32)arg1, arg2);
    }
    UnlockCd();
    return 0;
}
```

`Obj80027480`'s local view was EXTENDED for this function (offsets 0x18/0x1C
were previously padding, now named `unk18`/`unk1C`) -- see the struct note
below.

## What it took, in order

1. **`if (arg2 != 0) { ...; return X; } if (gCdAsyncEnabled...) {...} else {...}`
   vs `if (arg2 == 0) {...} else { ...; return X; }`** -- the same
   nested-if/else-if-vs-nested-if POLARITY lever as `CdDriver__Close` and
   `CdDriver__Read`, on a THIRD shape this time (an early-return `if` next to
   unconditional fall-through code, not two sibling `if`s). The fall-through
   arm (immediately after the `bnez`/`beqz` test) must be the `arg2==0`
   branch (the CdControl retry / gp_rel-store code); the branch TARGET,
   positioned later in the function, is the `arg2!=0` branch (the
   `ResetCdStateMachine`/`UnlockCd`/rounded-return code). Getting this
   backwards didn't just reorder blocks -- it also produced 1 EXTRA word (a
   duplicated `ResetCdStateMachine`/`UnlockCd` call pair), so it's worth
   reading as ITS OWN diagnostic signature: an early-return `if` whose body
   duplicates a call pair that also appears on the fall-through path is this
   same polarity bug, not two independent residues.

2. **The `CdDriver__Read` goto-instead-of-do-while lever does NOT apply here,
   and this is the important negative.** This function has a SECOND
   CdControl/CdSync retry loop, structurally identical in shape to
   `CdDriver__Read`'s CdRead/CdReadSync loop (a `do { call; do { v = call2();
   } while (v cond); } while (v == CONST);` retry pattern). Reflexively
   applying the same `goto`-based rewrite (to defeat GCC's loop-invariant
   hoist of the retry constant) produced the WRONG shape here: retail's own
   asm for THIS loop explicitly hoists `5` into `$s0` BEFORE the retry label
   (`ori $s0,$zero,0x5` sits above `.L80027614:`, the `CdControl` call site) --
   the exact optimization the goto rewrite exists to DEFEAT. A plain
   `do { ... } while (v0 == 5);` matched immediately once the polarity lever
   (above) was also applied; the goto form was 1 word too long against this
   retail shape. **Whether retail hoists a retry loop's constant is a
   per-function fact, not a per-shape one** -- check the actual `.s` for
   the hoisted `li $sN,<const>` (or its absence) before choosing `do-while`
   vs `goto`, rather than carrying the previous function's answer forward.

3. Everything else (the `srl` vs `sra` unsigned-shift lever from
   `CdDriver__Read`, applied to `arg1 >> 11`/`self->unk1C >> 11`; the
   round-up-to-sector idiom `x>>11; if (x&0x7FF) x++;`; the
   `EnqueueCdRequest`/`StartCdOperation` call shapes) matched on the first
   build once 1 and 2 above were fixed.

## Struct note

`self->unk18` is a 4-byte position buffer (only its ADDRESS is taken here,
passed to `CdPosToInt`) and `self->unk1C` is a `u32` byte-length field
(rounded up to a 0x800-byte sector boundary, same formula as
`GetCdFileSize`'s own `((self->unk1C >> 11) + 1) << 11`, code_179d8_h.c) --
this is the SAME struct as `ObjA34_179D8H` there, and that unit already
names offset 0x1C the same way, independently. `gCdSeekLoc` is an 8-byte
zero-initialized buffer (`asm/data/5DB70.data.s`); this function only ever
takes its address, so it's declared as a plain byte array locally.
`gCdSeekParam = gCdSeekLoc - 0x14` matches `code_179d8_s.c`'s existing reads of
that global (`(u8 *)gCdSeekParam + 0x14`) -- the same pointer, offset the other
direction.

### Proposed learning

**Do not generalize a loop-shape lever across two structurally-identical
loops in the SAME function without checking each one's own retail bytes.**
`CdDriver__Seek` has two nearly-identical retry loops in spirit (both
"do a CD op, poll for completion, retry on a specific status code"), and
retail hoists the retry constant in ONE of them and not the other analog
found in the previous function. The `goto`-defeats-loop.c's-invariant-motion
lever from `CdDriver__Read` is real, but its APPLICATION is per-loop: read
whether retail's own `.s` shows a `li $sN,<const>` sitting above the retry
label before reaching for `goto` instead of `do`/`while`.

## Naming

Round 79 (charlie), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027528` | `CdDriver__Seek` | B |

**Evidence.** `(self, offset, mode)`. Inside a queue dispatch on an open
file it converts `self->pos` to a sector number (`CdPosToInt`), adds
`offset` rounded up to whole 0x800-byte sectors, and writes the result to
`gCdSeekLoc` (`CdIntToPos`). With `mode == 0` it then seeks there
(CdlSetloc, or queues the seek on the state machine) and returns 0; with
`mode != 0` it instead returns `self->size` rounded up to a whole sector.
Outside a dispatch it enqueues op 4 with both arguments. The base-class
caller agrees with that reading: `FileResource__LoadFile` calls this slot
as `(0, 2)` to get the size it allocates and `(0, 0)` to rewind before
reading, the shape of `lseek(fd, 0, SEEK_END)` / `lseek(fd, 0, SEEK_SET)`.

**Why tier B.** The sync-mode path does not seek at all: it forwards to
`GetCdFileSize` and ignores both arguments. And only `mode` 0 and "nonzero"
are distinguished, so the lseek analogy is a description of the two
callers, not an established `whence` enumeration. The name covers the
async body and the rewind use; the size query is the other half of it.

**Class prefix.** `Class6D4E8` is the placeholder token for the method
table `gCdDriverMethods` (the convention `CdDriver__RequestLoadFile` and its two
siblings already use); `tools/classtable.py gCdDriverMethods` lists this function
at slot `+0x04C`. The prefix names the table, not the developers' class.

## Track 4b (2026-09-25, round 85)

The CD driver's shared globals and records are now declared once, in
`include/CdDriver.h`, and this body uses that one reading: the fake seek entry is spelled `(CdFileEntry *)(gCdSeekLoc - 0x14)`, so the state machine's `&gCdSeekParam->pos` lands on the loc. The
global's type comes from its accessors (`gFileTable` is walked at the 0x1C
`CdFileEntry` stride; `gCdSeekParam` is read for `->size` and sought to at
`+0x14`, i.e. `pos`). Byte-identical; no new `-Wall` warning.


Track 4, 2026-09-26 (round 88). The class of gCdDriverMethods (was D_8006D4E8, id 0x13 = DATASOURCE_CD) is CdDriver, in include/CdDriver.h: its ctor calls InitCdDrive, its slots enqueue CD_OP_* requests and drive the CD read state machine, and it is VabDriver's sibling. The object views this function was typed against are replaced by CdDriver, whose fields are all FileResource's (the driver runs on its clients' objects; FileResource's +0x018/+0x01C were named pos/size for it). Byte-identical. `Class6D4E8__Seek` -> `CdDriver__Seek` by rename.py.
