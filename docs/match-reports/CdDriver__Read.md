# CdDriver__Read -- MATCHED (round 47, alpha)

> Renamed from `Class6D4E8__Read` on 2026-09-26 (tools/rename.py). Address 0x800276d0.

> Renamed from `func_800276D0` on 2026-09-25 (tools/rename.py). Address 0x800276d0.

76/76 words, byte-exact, file 0x17ED0-0x18000. Cold ground.

## Source

```c
extern s32 gCdReadSectorCount; /* CdRead sector count */
extern void *sCdReadBuffer; /* CdRead target buffer */
extern s32 gCdTickStep;

extern void ReadCdFile(Obj80027480 *self, void *arg1, s32 arg2);
extern s32 CdRead(s32 sectors, void *buf, s32 mode);
extern s32 CdReadSync(s32 mode, s32 result);
extern void ResetCdStateMachine(void);

s32 CdDriver__Read(Obj80027480 *self, void *buf, u32 size) {
    s32 v1;

    if (sCdAsyncEnabled == 0 && gCdSyncQueueMode == 0) {
        ReadCdFile(self, buf, size);
        return 0;
    }
    LockCd();
    if (self->unk28 != 0) {
        if (sCdBusy == 0 && self->unk0C != 0) {
            StartCdOperation(3, 7);
            if (sCdAsyncEnabled != 0) {
                gCdReadSectorCount = size >> 11;
                sCdReadBuffer = buf;
                gCdTickStep = 1;
            } else {
            retry:
                CdRead(size >> 11, buf, 0x80);
                do {
                    v1 = CdReadSync(0, 0);
                } while (v1 > 0);
                if (v1 == -1) {
                    goto retry;
                }
                ResetCdStateMachine();
            }
        }
    } else {
        EnqueueCdRequest(self, 0, 5, (s32)buf, size);
    }
    UnlockCd();
    return 0;
}
```

(`Obj80027480`, `sCdAsyncEnabled`, `gCdSyncQueueMode`, `sCdBusy`, `LockCd`,
`StartCdOperation`, `EnqueueCdRequest`, `UnlockCd` are all declared earlier in
the unit, ahead of `CdDriver__Close`.)

## What it took, in order

1. **The nested-if-vs-if/else-if polarity lever from `CdDriver__Close` applied
   again unchanged**: `if (self->unk28 != 0) { if (sCdBusy==0 && ...) {...}
   } else { EnqueueCdRequest(...); }`, not the flattened else-if form. Same
   reasoning as that report.
2. **`size >> 11` must be an UNSIGNED shift.** Retail's `srl` (logical) vs an
   initial `s32 size` parameter, which produces `sra` (arithmetic) for a
   right-shift of a negative-capable type. Declaring the parameter `u32 size`
   fixed all three `>>11` sites at once (`gCdReadSectorCount` store, the two `CdRead`
   calls).
3. **The real find: GCC 2.6.3's loop-invariant code motion hoists a
   loop-carried literal comparison (`v1 == -1`) out of a `do { ... } while
   (v1 == -1);` retry loop into a spare callee-saved register** (here it
   picked `$s0`, since `self` is dead on this path after the last
   `self->unk0C` read) **-- 1 word SHORTER than retail, which recomputes
   `li v0,-1` fresh every outer iteration and keeps `CdReadSync`'s return in
   an explicit second register (`move v1,v0`) instead of reading `$v0`
   in place.** Isolated through the pinned pipeline (four standalone variants
   in `/tmp/.../t1.c` through `t4.c`, all four reproducing the identical
   hoist regardless of register pressure -- adding back the full real
   function's register load via `self`/`buf`/`size` and the sibling
   `EnqueueCdRequest` branch changed nothing).

   **The fix: write the OUTER retry as an explicit `label:` + `if (...)
   goto label;` instead of a `do { } while (...)`.** GCC 2.6.3's classic
   loop optimizer (`loop.c`) recognises invariant-hoisting candidates from
   the loop notes the front end emits for syntactic `for`/`while`/`do`
   constructs; a hand-written `goto` back-edge carries no such note, so the
   pass that hoists the constant never fires, even though the CFG (and the
   resulting *correctness*) is identical. Confirmed in isolation
   (`/tmp/.../t5.c`): swapping only the outer loop's spelling, nothing else,
   reproduces retail's `move v1,v0` / fresh `li v0,-1` / no-hoist shape
   exactly. The INNER `do { v1 = CdReadSync(0,0); } while (v1 > 0);` stayed a
   real `do`-loop throughout -- it has no loop-carried literal to hoist, so
   there was nothing to defeat there, and changing it to `goto` was never
   tried/needed.

### Proposed learning

**GCC 2.6.3's loop-invariant code motion is SYNTAX-GATED, not CFG-gated, and
this is a second, independent instance of the same fact CLAUDE.md already
records for `break`-past-a-fall-through-tail (`ResolveFileEntries`, round 45).**
When a `do`/`while`/`for` loop hoists a loop-carried CONSTANT comparison into
a spare register and retail does not, don't hunt for a register-pressure or
liveness difference (four isolated variants here proved pressure is not the
knob) -- try replacing the loop with an equivalent hand-written `label:` /
`goto` back-edge first. It is legal C89, it pins no register and names no
operand (nowhere near HARD RULE 6), and it defeats exactly the one pass
(`loop.c`'s invariant motion) that a syntactic loop invites and a `goto`
does not. Discriminator for when to reach for it: the diff shows an EXTRA
`li <saved-reg>,<const>` outside the loop body paired with a MISSING `move`
that copies a call's return value into its own register inside the loop --
that pairing is the hoist's signature, not a generic register-identity
residue.

## Naming

Round 79 (charlie), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800276D0` | `CdDriver__Read` | A |

**Evidence.** `(self, buf, size)`. Sync mode forwards to `ReadCdFile`;
otherwise it enqueues op 5 with `buf`/`size`, or inside a queue dispatch on
an open file reads `size >> 11` sectors into `buf` (CdRead + CdReadSync
retry loop, or hands `gCdReadSectorCount`/`sCdReadBuffer` to the state
machine). `FileResource__LoadFile` calls this slot with the buffer it just
allocated and its size, between the rewind and the close.

**Class prefix.** `Class6D4E8` is the placeholder token for the method
table `gCdDriverMethods` (the convention `CdDriver__RequestLoadFile` and its two
siblings already use); `tools/classtable.py gCdDriverMethods` lists this function
at slot `+0x054`. The prefix names the table, not the developers' class.


Track 4, 2026-09-26 (round 88). The class of gCdDriverMethods (was D_8006D4E8, id 0x13 = DATASOURCE_CD) is CdDriver, in include/CdDriver.h: its ctor calls InitCdDrive, its slots enqueue CD_OP_* requests and drive the CD read state machine, and it is NullDriver's sibling. The object views this function was typed against are replaced by CdDriver, whose fields are all FileResource's (the driver runs on its clients' objects; FileResource's +0x018/+0x01C were named pos/size for it). Byte-identical. `Class6D4E8__Read` -> `CdDriver__Read` by rename.py.
