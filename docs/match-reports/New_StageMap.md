# New_StageMap

> Renamed from `New_Class866E8` on 2026-09-26 (tools/rename.py). Address 0x8004a4c8.

> Renamed from `func_8004A4C8` on 2026-09-22 (tools/rename.py). Address 0x8004a4c8.

**Unit:** class_3ac78 · **Size:** 27 words · **Status:** MATCHED (27/27 words)

This is the **canonical account of how the `New_X` epilogue-merge residue
class was closed.** Four other reports point here:
`New_DayTask`, `New_TimedTask` (class_39e08), `New_StreamTask`,
`New_TaskCore` (code_2c054).

## What it does

The `New_X` constructor wrapper for `StageMap`: allocate `0x1E8` bytes and,
if that succeeds, dispatch the class's ctor slot (`+0x008`) with the two
caller-supplied arguments. Returns the new object, or NULL.

## The match

```c
StageMap *New_StageMap(s32 arg1, s32 arg2)
{
    StageMap *self;

    self = BMemPMgrAlloc(0x1E8);
    if (self != NULL) {
        GetStageMapMethods()->ctor(self, arg1, arg2);
        return self;
    }
    return NULL;
}
```

## The rule

**`return NULL;` must come textually LAST, after the success return.**

That is the whole discriminator. Everything else about the function — the
allocator, the size constant, the accessor, the argument forwarding — was
already right in every previous attempt on both machines.

| source shape | result |
| --- | --- |
| one exit, `if (self) { ctor(...); } return self;` | **26/27** — delay slot gets `move $v0,$s0`, retail has `move $v0,$zero` |
| `if (self == NULL) { return NULL; }` **first**, body after | worse — GCC lays the NULL block out *after* the body and pays a `j` to skip it |
| `if/else` with `ret = NULL;` in the `else` | identical to the above, same extra `j` |
| separate `ret` local assigned `NULL` before the `if` | retail's **exact instruction sequence**, but `ret` lands in `$v1` plus a `move v0,v1` at the epilogue — assigning before the branch makes the live range span both calls, so GCC keeps it out of the call-clobbered `$v0` |
| **`return NULL;` last** (above) | **27/27** |

The fourth row is the one that shows the mechanism: the structure was always
reachable, and only register allocation stood in the way. Moving the NULL
return after the success return removes the live range entirely.

`goto fail;` with a trailing `fail: return NULL;` label is byte-identical and
was how this was first cracked (copied from `New_Pad` in
`class_16334`, matched rounds earlier). It is **not** load-bearing — two
runners reached the same bytes with a `goto`-free spelling. The rule is about
statement order, not `goto`.

## What this closes, and what it does not

`docs/research/epilogue-merge-residue.md` had this class at **24 instances
corpus-wide**, ~25 spent attempts, four independent confirmations, and a
stated conclusion that GCC 2.6.3 `-O2` "will not merge two exits carrying
different values into one epilogue" — filed as a permuter target. That
conclusion is **falsified**: the compiler does it readily, given the right
statement order. No permuter was needed.

Five instances closed in one pass, all byte-exact:

| function | unit | words |
| --- | --- | --- |
| `New_StageMap` | class_3ac78 | 27/27 |
| `New_TimedTask` | class_39e08 | 27/27 |
| `New_DayTask` | class_39e08 | 31/31 |
| `New_StreamTask` | code_2c054 | 36/36 |
| `New_TaskCore` | code_2c054 | 31/31 |

**It does NOT close `New_GameApplication`** (code_1677c, 23/24), and that negative
matters: the class has **two sub-shapes**, distinguished by what retail puts
in the `beqz` delay slot.

- **`move $v0, $zero`** — retail materializes the NULL return. Closed by this
  rule. All five above.
- **`nop`** — retail materializes nothing, relying on `$v0` still holding the
  allocator's own zero return. `New_GameApplication` is this shape, and both the
  rule and a `return self;`-on-the-null-path variant were measured against it
  and rejected. Still open.

Screen a candidate before assuming the rule applies:

```sh
grep -A1 'beqz' asm/nonmatchings/<unit>/<func>.s | grep -E 'addu *\$v0, *\$zero, *\$zero|nop'
```

## Provenance

Worked independently on two machines from the same base commit (`0a544ff`),
both stalling at 26/27 and both filing it as a residue class — one as a
"genuine redundant move" instance, one as an epilogue-merge instance. Closed
by the head during divergence resolution, 2026-09-02.

The discriminator that unlocked it: the redundant-move class requires the two
instructions to be interchangeable *given the same source*. `move v0,zero` and
`move v0,s0` are not — they come from two different **return expressions**.
Retail materializing the return value twice, from two different operands, is
positive evidence for a source with two `return` statements. Once that is the
question, the search space is statement order, not barriers or temps.

### Proposed learning

**Two different source operands for the same value mean two different return
expressions — a source-shape question, not a toolchain one.** Reach for
statement order before reaching for `__asm__("")`, `volatile`, or a permuter.

**And when a residue class has a research doc with a stated impossibility,
check the doc's own discriminator against a fresh instance before trusting
it.** This one had four confirmations and ~25 attempts behind a claim that was
simply wrong, and the cost of that was five functions left unmatched plus a
recommendation to spend permuter time.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004A4C8` | `New_StageMap` | A | Body is the project's established `New_X` shape: allocate `0x1E8` via `BMemPMgrAlloc`, and on success dispatch the class's ctor slot `+0x008` with the caller's two arguments, else return NULL. `New_Class` is the convention named in FINISHING-PLAN.md track 3, and this report already used the phrase before the rename. |

The one call site is `src/class_39e08.c`'s `DayTask__DayTask`, the boot path:
`arg1->unkC = (SubObjG *)New_StageMap(0, 1);`. So exactly one instance of
this class exists, created at game start.

## Track 6 (2026-09-26, round 93, alpha)

The class `Class866E8` (table `gClass866E8Methods`, id 0x114, LightRig's
subclass) is now `StageMap` (`python3 tools/renametype.py Class866E8
StageMap`, tier B): it keeps seven slots loaded with map chunks of the
current stage (LbdFile, `STGnn\Mnnn.LBD`) around a tracked target, the
centre chunk and its six staggered neighbours (`sChunkNeighbourDeltas`), laid
out by the stage's `StageGridDimensions` (`setConfig`, from ObjM's
`GetStageGridDimensions(stage)`), each slot's placements linked into a 20 x
20 lattice of GridCells whose drawn window follows the target. Tier B: the
mechanics are established; "the stage's map" rests on the files it loads and
the per-stage config. Header now `include/StageMap.h`; evidence in its banner.

Member types, same pass: `Unk68Struct` is `StageGridDimensions`
(include/StageGrid.h), `Unk54Struct` is `LongVec3` (include/SceneNode.h),
`EntryDesc866E8` is `Ratio16[3]` (include/SceneNode.h), all by layout and
use; `Class866E8Elem` -> `ChunkSlot`, `QueryPos866E8` -> `SplitLongVec3`,
`SetupEntry866E8` -> `ChunkLoadEntry`, `SetupSub866E8` ->
`ChunkLoadEntryTail`, `TargetSpec866E8` -> `ChunkSlotSpec`, `GridSlot866E8`
-> `CellRect`, `GridSlotList866E8` -> `CellRectSet`, `Bounds866E8_3bb8c_b`
-> `CellBounds`, `Class866E8ValueFn` -> `ChunkFileFn`,
`Class866E8OnElementEventFn` -> `StageMapOnSlotEventFn`,
`Class866E8ElemFn` -> `ChunkSlotFn`, `Class866E8CellFn` -> `StageMapCellFn`;
new `ChunkNeighbourDelta` for `sChunkNeighbourDeltas` (was typed as the
3-word placeholder). renametype.py also rewrote the old names inside
earlier sections' history prose in this and sibling reports (known, pending
an operator decision; not hand-reverted).
