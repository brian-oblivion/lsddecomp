# New_ObjM

> Renamed from `func_80052B70` on 2026-09-24 (tools/rename.py). Address 0x80052b70.

**Unit:** ObjMStyleActor · **Size:** 40 instructions (0xA0 bytes) ·
**Status: MATCHED 40/40**, whole-image SHA1 green.

## Role

`New_X`-style allocator for the class whose method table is `gObjMMethods`
(the same table `GetObjMMethods` returns). Allocates a 0x88-byte instance
via `BMemPMgrAlloc`; if allocation succeeds, dispatches through the
returned vtable's `+0x008` (ctor) slot with the 5 forwarded arguments and
returns the new instance; returns `NULL` on allocation failure.

`ObjM__ObjM` (this unit, matched in the same round) IS the ctor slot
this function dispatches to.

## Signature

The return type and first-argument type are NOT free choices here: this
function already has a pre-existing prototype in `include/class_39e08.h`
(established from `DayTask__StartObjM`'s call site, which stores the result
into `Obj865C8::unk4C`):

```c
extern Obj4C *New_ObjM(SubObjB *a0, s32 a1, s32 a2, s32 a3, s32 a4);
```

`ObjMStyleActor.c` includes `class_39e08.h`, so this function's definition
must match that prototype exactly (return type and arg0 type) or the two
conflict at compile time. `Obj4C` is `class_39e08.h`'s own narrow opaque
view of the object this function constructs (methods pointer only) --
deliberately narrower than this unit's own view (see below), per the
project's multiple-independent-local-views convention.

## The residue that mattered: two return statements, not one

The body is a plain "if allocation succeeded, construct" -- the interesting
part is entirely in how the C is *shaped* around the two exits, because the
three shapes tried compile to three different instruction counts/layouts
despite being semantically identical.

Final, byte-exact form:

```c
Obj4C *New_ObjM(SubObjB *a0, s32 a1, s32 a2, s32 a3, s32 a4)
{
    Obj4C *self;
    Class87034Methods_3bb8c_k *methods;

    self = BMemPMgrAlloc(0x88);
    if (self != NULL) {
        methods = GetObjMMethods();
        methods->ctor(self, a0, a1, a2, a3, a4);
        return self;
    }
    return NULL;
}
```

Two shapes that looked equally plausible and did NOT match, kept here
because the failure mode is worth knowing about before re-deriving it:

1. **Single variable, single trailing `return self;`:**
   ```c
   self = BMemPMgrAlloc(0x88);
   if (self != NULL) {
       methods = GetObjMMethods();
       methods->ctor(self, a0, a1, a2, a3, a4);
   }
   return self;
   ```
   39/40 words. The ONE word that differs is the branch's delay slot:
   retail materializes `v0 = 0` there (an immediate, `addu $v0,$zero,$zero`),
   this shape materializes `v0 = s0` (`move v0,s0`) instead -- functionally
   identical (`s0` IS `0` on the taken path), but different bytes. GCC
   apparently only picks the immediate-zero form when there are textually
   TWO DISTINCT `return` statements to materialize independently (see the
   winning form above); with one shared `return self`, it reuses whatever
   register already holds the value.

2. **Two variables (`self` + `result`, `result` preset to `NULL` before the
   `if`, only overwritten inside it):**
   ```c
   self = BMemPMgrAlloc(0x88);
   result = NULL;
   if (self != NULL) {
       ...
       result = self;
   }
   return result;
   ```
   This is what `m2c` seeded (`temp_v0`/`var_v0`). It reproduces the two
   independent value-materializations correctly, but `result` is live
   across the whole `if` block (including the `ctor` call, which clobbers
   the usual temporaries), so it needs its OWN callee-saved register
   distinct from `self`'s -- a 7th saved register (`$s6`) where retail uses
   only 6 (`$s0`-`$s5`). That extra `sw`/`lw` pair grows the function by
   2 words, which shifts every subsequent function's linked address (funcdiff
   reported ~162KB of outside-range difference -- the whole-image tell for
   "this function's length itself is wrong", not a register-identity
   residue).

3. **Early-return guard (`if (self == NULL) { return NULL; } ...; return
   self;`)** -- logically the same two-exit shape as the winning form, just
   with the two `if`-arms swapped (guard-clause style instead of
   guard-the-happy-path style). This does NOT reorder for free: GCC laid the
   `return NULL;` block out AFTER the ctor body (not merged into the
   branch's delay slot), so the ctor-body's own `return self;` needed an
   extra explicit `j` to skip over it and reach the shared epilogue. Net:
   +2 words (one wasted delay-slot `nop` at the early branch, one extra `j`
   at the end of the ctor body), same "outside-range" symptom as #2.

**Takeaway for future `alloc; if (ok) { construct; return obj; } return
NULL;` bodies in this codebase:** write the two `return`s with the
*success* path as the `if`-body (ending in its own `return`) and the
*failure* path as the unconditional trailing `return NULL;` — not the
early-return-guard ordering, and not a single shared-variable return. This
is the one layout GCC 2.6.3 -O2 folded the failure return entirely into the
branch's delay slot for, with no extra block or jump.

### Proposed learning

For a "try to construct, return NULL on failure" body with exactly two
exits, write the SUCCESS path's `return` inside the `if`-body and the
FAILURE path's `return NULL;` as the trailing unconditional statement (not
an early-return guard clause, and not a single variable shared across both
paths) -- this is the layout GCC 2.6.3 -O2 folds into a single branch with
the failure value in the delay slot and no extra jump. Confirmed once, on
`New_ObjM`; worth checking against future instances of this same
project-wide `New_X` allocator shape before assuming it generalizes.

## Naming

Round 75 (bravo, track 3). `func_80052B70` -> `New_ObjM`, **tier A**.

New_X allocator: BMemPMgrAlloc(0x88), then GetObjMMethods()->ctor (+0x008 of gObjMMethods, `tools/classtable.py 0x80087034`) with the 5 forwarded arguments; returns the object or NULL. Caller: DayTask__StartObjM (class_39e08). The class's type name is `ObjM` (include/class_3bb8c.h).


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the ObjMStyleActor/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and class_39e08.h's Obj4C/SubObjB/EventArg are gone. Byte-identical. Signature now `ObjM *New_ObjM(BasicClass *sound, WBgm *bgm, TimImage *etcTim, LinkResource *dreamerTmd, s32 stage)`: the arguments are DayTask's `sound`, `bgm`, `etcTim`, `dreamerTmd` (include/DayTask.h), which StartObjM now passes uncast.

## Round 99 (delta, track 7)

`BMemPMgrAlloc(0x88)` -> `BMemPMgrAlloc(sizeof(ObjM))` (ObjM is 0x88 bytes, include/ObjM.h; byte-identical). The two returns keep a `MATCHING:` line.
