# InitDreamAux -- MATCHED (56/56), round 24

> Renamed from `func_8005C508` on 2026-09-21 (tools/rename.py). Address 0x8005c508.

> **VERDICT CORRECTED, round 24 (2026-09-08), runner delta.** Everything
> below this note was written against the `addiu_at` jump-table/global-index
> folding blocker, resolved in round 21 (maspsx `--addiu-at`;
> `docs/research/addiu-at-blocker.md`). **This report was never re-screened
> after that fix** -- the same way `CheckDreamAuxTriggerCondition`'s report was missed by
> round 22's sweep because it reads as a considered plateau, not a stub.
>
> The residue this report called unclosable (`%lo(gDreamAuxSlots)` folded into
> the store's own displacement, instead of retail's full `lui`+`addiu`
> materialize-then-`addu`) is **the exact same construct** as the switch
> jump-table fold that blocked `CheckDreamAuxTriggerCondition` -- an indexed-global address
> computation, which is precisely the class CLAUDE.md's blocker screen
> already generalizes ("a jump-table dispatch loads a CODE address and jumps
> ... an indexed global loads a DATA value ... they are ONE construct at the
> layer that decides"). With `--addiu-at` live, GCC now emits retail's full
> 4-instruction unfolded form for this indexing too, with **no source change
> at all** -- the near-miss body below, restored VERBATIM (attempt (3)'s
> kept version, unchanged), compiles **byte-for-byte identical to retail**:
>
> ```sh
> ./build-and-verify.sh   # exit 0, "OK: build matches retail SLPS_015.56"
> .venv/bin/python3 tools/funcdiff.py InitDreamAux
> # InitDreamAux: 56/56 words match (file 0x4CD08-0x4CDE8)
> ```
>
> Measured with `CheckDreamAuxTriggerCondition` held at `INCLUDE_ASM` (its own case-6/7 fix
> is a separate, still-open residue -- see that function's own report) so
> this result is isolated from any other function's drift. The whole-image
> oracle passed outright, which is the strongest possible confirmation: no
> per-function window, no drift caveat, just a green `build-and-verify.sh`.
>
> **This is now real C in `src/DreamAux.c`, committed.** The rodata
> ownership trap documented below (defining `gMomPathSymSpy`/`gMomPathSymDog` as real
> string data ahead of the function) was exactly as described and is now
> permanent, not a note for a future attempt.
>
> **Proposed learning, promoted:** any report attributing a residue to
> "retail materializes a full address, mine folds `%lo` into the load/store
> displacement" for an INDEXED GLOBAL (not just a jump table) should be
> re-screened against `addiu_at` before being trusted -- it is the same
> maspsx mechanism CLAUDE.md already documents for jump tables, just not yet
> generalized in writing to plain indexed-global stores/loads. Grep the
> function's own `.s` for `%lo(jtbl` OR a `lui`/`addiu`/`addu` triplet
> immediately preceding a `lw`/`sw` at the SAME symbol the source indexes --
> either is the same construct.

---

## Original report (pre-round-24, kept for history)

Unit `code_4cd08` ("DreamAux"). Best reached: **40/56 words** (file
`0x4CD08`-`0x4CDE8`), everything outside that range then drifts because the
function compiles 4 bytes (one instruction) short of retail's 0xE0/56 words.
No `gp_rel` in this function's asm (`grep -l gp_rel` does not list it) -- this
is not the gp-relative blocker, it is a residue class of its own. Restored to
`INCLUDE_ASM` before committing; no C left in `src/`.

## What it does

Two independent passes over unrelated tables, then a load of the "DreamAux"
audio-stream-request object:

1. For `i` in `0..13`: `gDreamAuxGroupRecords[i]` is a pointer to an array of
   `gDreamAuxGroupCounts[i]` (signed count) 8-byte records; clear byte 0 (offset `0x0`,
   named `flag`) of each.
2. Build a request (`ResourceRequest__Set`, already matched elsewhere in
   `GameApplicationFileResource.c` as a plain 3-word field setter) with `flag=0`,
   `name="ETC\\SYMSPY.MOM"`, `mode=1`.
3. A `for (i = 0; i < 1; i++)` loop (see the "loop that only runs once" note
   in `ReleaseDreamAuxModels`'s report -- same confirmed idiom) that calls
   `New_ModelData((struct ResourceSource *)&req)` and stores the result into `gDreamAuxSlots[0].obj`,
   then overwrites `req.name` with `"ETC\\SYMDOG.MOM"`. Because the loop
   only runs once, that second name write is dead in THIS retail build --
   likely a leftover of an original 2-iteration loop (SYMSPY then SYMDOG)
   that got cut down to 1 without removing the "prepare next name" tail.
   Not confirmed; flagged as a curiosity, not load-bearing for the match.

## Near-miss body (compiles, builds green, 40/56)

```c
#include "common.h"
#include "DreamAux.h"

/* const char gMomPathSymSpy[] = "ETC\\SYMSPY.MOM"; */
/* const char gMomPathSymDog[] = "ETC\\SYMDOG.MOM"; */
/* ^ These must be defined in this .c file (not just declared extern) when
 * this function is not INCLUDE_ASM'd -- see "Rodata ownership" below. */

void InitDreamAux(void)
{
    DreamAuxLoadReq req;
    u32 i;
    s32 j;

    for (i = 0; i < 14; i++) {
        for (j = 0; j < gDreamAuxGroupCounts[i]; j++) {
            gDreamAuxGroupRecords[i][j].flag = 0;
        }
    }

    ResourceRequest__Set(&req, 0, gMomPathSymSpy, 1);

    for (i = 0; i < 1; i++) {
        gDreamAuxSlots[i].obj = New_ModelData((struct ResourceSource *)&req);
        req.name = gMomPathSymDog;
    }
}
```

Needs (from `include/DreamAux.h`, added this round):
`DreamAuxLoadReq`, `DreamAuxGroupRecord`, `gDreamAuxGroupCounts`, `gDreamAuxGroupRecords`,
`DreamAuxSlot`, `gDreamAuxSlots`, `ResourceRequest__Set`, `New_ModelData`.

## The residue, precisely

Retail, storing the call result into `gDreamAuxSlots[i].obj`:

```
lui   at, %hi(gDreamAuxSlots)
addiu at, at, %lo(gDreamAuxSlots)   ; <-- retail materializes the FULL address
addu  at, at, s0                 ;     (s0 = running byte offset, i*0x14)
sw    v0, 0(at)
```

Every source shape tried here compiles to:

```
lui   at, %hi(gDreamAuxSlots)
addu  at, at, s0
sw    v0, %lo(gDreamAuxSlots)(at)   ; <-- %lo folded into the store's own
                                 ;     displacement instead
```

Both compute the identical target address; mine is one instruction shorter.
This is the ONLY residue in the function -- the two preceding loops, the
`ResourceRequest__Set` call, and the loop-control shape of the final loop (see
`ReleaseDreamAuxModels`'s report for why `for (i=0;i<1;i++)` is the right shape, not
decompiler noise) are all byte-exact already (confirmed via `asm-differ`,
which shows the first ~46 instructions matching before this one diverges).

## What was tried (all rejected, in order)

1. Manually hoisted `s8 *counts` / `DreamAuxGroupRecord **groups` pointers
   incremented by hand in loop 1, instead of indexing `gDreamAuxGroupCounts[i]` /
   `gDreamAuxGroupRecords[i][j]` directly -- **worse** (5/56). Switching to direct
   array indexing and letting GCC do its own induction-variable strength
   reduction (per the head's "let GCC hoist its own invariants" broadcast)
   got loop 1 to match byte-for-byte; this part of the lever generalizes.
2. Separate `s32 i, j, k` (three distinct loop variables) -- outer loop's
   counter landed in a caller-saved temp (`t0`/`a3` depending on variant)
   instead of retail's `$s1`. **Register identity**, not touched via
   `register asm` (banned) -- instead tried reshaping.
3. Reusing the SAME `i` for both the first (0..13) and the third (0..0)
   loop, typed `u32` -- fixed the register identity (both loops now land in
   `$s1`, matching retail exactly) AND fixed a second residue
   (`sltiu` vs `slti` on the loop-1 bound, needing `i` unsigned). Jumped
   32/56 -> 40/56. This is the version kept as the near-miss.
4. Reintroducing a separate `u32 k` for the third loop (declared, not
   reused) -- regressed to 32/56. Confirms (3) needs the reuse, not just
   the type.
5. For the address-fold residue specifically, all of the following compile
   clean but do not add the missing instruction (most made other things
   *worse*, none improved past 40/56): reordering `req.name = ...` before
   vs. after the store; splitting the call result into a named temporary
   before storing it; declaring `gDreamAuxSlots` with an explicit bound (`[14]`)
   vs. incomplete (`[]`) -- no difference either way; replacing the
   `u8 unk4[0x10]` padding with four named `s32` fields -- no difference;
   `DreamAuxSlot *entry = &gDreamAuxSlots[i]; entry->obj = ...;` -- worse
   (36/56); explicit byte-pointer arithmetic
   (`*(DreamAuxObj **)((u8 *)gDreamAuxSlots + i * sizeof(DreamAuxSlot))`) --
   worse (36/56 and 28/56 depending on exact form); a `volatile` cast on the
   array -- worse (36/56); a bare `__asm__("")` as the function's first
   statement -- worse (33/56), and per CLAUDE.md rule 6 this would need
   checking whether it changes register identity before it could even be
   considered, moot since it regressed the score anyway; `do { ... } while
   (i == 0)` instead of `for (i = 0; i < 1; i++)` -- identical to the `for`
   form (still 40/56, confirming the loop CONTROL shape is not the issue).

## Rodata ownership (a real trap, worth flagging even though this stalled)

`config/splat.slps01556.lsdde.yaml` marks the `0x206C` rodata segment
`.rodata, DreamAux` (dot-prefixed) for `CheckDreamAuxTriggerCondition`'s jump tables, but
`gMomPathSymSpy`/`gMomPathSymDog` (the two MOM filenames) live in the same run and
are consumed only by `InitDreamAux`. While this function is `INCLUDE_ASM`,
its own `.s` file carries these two strings as raw (`nonmatching`) asm
blocks and the build is green. The MOMENT this function is de-`INCLUDE_ASM`'d,
that `.s` file is no longer pulled in by anything (it lives under
`asm/nonmatchings/`, which per the Makefile is *only* assembled via
`INCLUDE_ASM`), and both symbols go undefined at link time. Fix: define them
as real C string data (`const char gMomPathSymSpy[] = "ETC\\SYMSPY.MOM";` etc.)
directly in `DreamAux.c`, ahead of the function -- this is "flip to the
dot form in the same commit that writes the C" from
`docs/DECOMPILATION_LEARNINGS.md`, just for a rodata slot that happens to
hold strings rather than a table. Whoever re-attempts this function needs
that definition back (commented out above, in the near-miss body) or the
build will fail at link with `undefined reference to gMomPathSymSpy`.

## Proposed learning

- **A `%lo(sym)` fold into a store/load's own displacement, vs. retail
  materializing the full absolute address first (`lui`+`addiu`) before
  adding a running byte offset, does not appear to be controllable from
  plain C source in this one case** -- reordering statements, splitting the
  call result into a temp, changing the array's declared bound, changing
  the struct's field shape, pointer-vs-index access style, `volatile`, and a
  scheduling barrier were all tried and none closed it (several made it
  worse by changing unrelated register allocation too). Flagging this as
  its own residue category distinct from "instruction order only" and
  "register identity" in MATCHING-GUIDE.md's list -- next attempt should
  probably reach for the permuter rather than more manual reshaping.

## Naming

**InitDreamAux** — tier B. Called exactly once, as the first DreamAux-specific
call inside `DayTask__DayTask` (a constructor: `GetTimedTaskMethods()->ctor(self,...);
self->methods = GetDayTaskMethods(); InitDreamAux(); ...`), before the rest of
that object's own fields are set up. Clears every `gDreamAuxGroupRecord`'s
`flag` across all 14 groups and loads the initial MOM audio-stream object
into `gDreamAuxSlots[0].obj`. "Init" fits the one-shot, construction-time
call site; the broader game reason (why THIS unit's state resets alongside
that particular object's construction) is not established from this unit
alone, hence tier B rather than A.

## Track 4

2026-09-25, round 84 (delta): ModelData (gModelDataMethods) is unified in `include/ModelData.h`. DreamAux includes it, and include/DreamAux.h's local `extern void *New_ModelData(DreamAuxLoadReq *req)` is deleted. The call reads `New_ModelData((struct ResourceSource *)&req)`, a pointer cast with no code. DreamAuxLoadReq {flag, name, mode} has the descriptor's own shape: word 0 is the buffer to adopt, and it is 0 here, so ModelData__ModelData requests the MOM file named in word 1. Image byte-identical.

### Track 6 (round 97, alpha)

The request local (was `DreamAuxLoadReq {flag, name, mode}`; word 0 is the buffer, NULL here) is now include/FileResource.h's `ResourceRequest`
(`{ ResourceSource src; s32 mode; }`), and ResourceRequest__Set's prototype
comes from that header. The unit's own view of the record and its local
extern are gone. `req.buffer`/`req.name` become `req.src.buffer`/`req.src.name`, and
`(ResourceSource *)&req` becomes `&req.src`. Byte-identical.
The two MOM path names are `const char[]` and are passed as `(char *)`, since ResourceSource.name is `char *`.

## Round 100 (alpha): track 7, moved from src/DreamAux.c and include/DreamAux.h

The pass rewrote the unit's banner (now at the top of src/DreamAux.c) and
every declaration comment in include/DreamAux.h. The comments that carried
history or superseded readings are kept here verbatim, as they stood before
the pass (names as they were at round 99).

Findings of the pass that change what those comments said: DreamAuxGroupRecord
is TriggerRecord (the same 8-byte records; `flag` is `triggered`), so the view
is gone and the record is TriggerRecord.triggered; the per-stage tables are 14
pointers each (DREAM_AUX_STAGE_COUNT, from the label spacing) and the slot
arrays hold ONE slot (gDreamAuxPosTable starts 0x14 after gDreamAuxSlots), not
14; a slot's first word is the ModelData New_ModelData returns (`model`), and
the "tick" at method slot +0x004 is BasicClass's release. The MOM files are
ModelData files (a TMD and a TodSet, include/ModelData.h), not audio. With one
slot, SYMDOG.MOM is never requested. Locals: `j` -> `record`; the 14 and 1
loop bounds are ARRAY_COUNT of the tables they walk. Byte-identical.

The header banner:

```c
/* This unit is lsddecomp's "DreamAux". Fully matched, round 43 (0
 * INCLUDE_ASM); track 3 naming pass round 63. It owns the 0x206C rodata
 * slot (its switch jump tables) and manages a small "trigger record" system:
 * a table of 8-byte TriggerRecord entries, each gating on a caller-supplied
 * `value` (CheckDreamAuxTriggerCondition) and a coordinate parity
 * (CheckTriggerParity), that on success spawns or despawns an Entity into
 * one of two 14-slot object-tracking families (SpawnDreamAuxTriggerEntity /
 * DespawnDreamAuxEntity, backed by gDreamAuxSlots / gDreamAuxSlots2) and can
 * gate the game's teleport flag (EnableTeleportsForKind, SetTeleportsEnabled
 * in DreamSys.c). InitDreamAux/TickDreamAuxSlots/TickDreamAuxSlots2 are the
 * construct/tick/destruct hooks a caller in class_39e08.c and
 * ObjMStyleActor.c drives this subsystem through. `gDreamAuxStage`,
 * `gDreamAuxWorld` and three sibling globals SetDreamAuxWorld installs are
 * the shared context every other function in the unit reads.
 */
```

The slot and its object view (DreamAuxObj / DreamAuxTickFn, deleted):

```c
/* An object whose method table pointer sits at offset 0 (every object in
 * this game's class framework, per CLAUDE.md's "Writing a class method").
 * Only slot 1 (offset 0x4 in the table) is known here: a "tick" method that
 * takes the object and returns a (possibly new/updated) object pointer. */

/* A slot in the 0x80088D28 / 0x80088D2C families: one live-object pointer
 * (ticked once per call by calling obj->vtable[1](obj) and storing the
 * result back into the same slot); an Entity at +0x4 (include/Entity.h)
 * that SetDreamAuxWorld makes with New_Entity and DespawnDreamAuxEntity
 * detaches and re-attaches (detachFromParent, attachToParent with the
 * player gDreamAuxWorld as the peer);
 * and a 3-word position vector at +0x8 that DespawnDreamAuxEntity passes as
 * `SceneNode__LocalOffsetToWorldPos`'s `src` (that function's own signature, `code_d294.h`,
 * takes `s32 *src` and treats it as a 3-word vector). Stride is 0x14,
 * confirmed by SetDreamAuxWorld's walk over gDreamAuxSlots. */
```

The group-record view (deleted; TriggerRecord):

```c
/* A tiny fixed-size record family read by InitDreamAux: 14 (0xE) parallel
 * groups, gDreamAuxGroupCounts[i] a signed count and gDreamAuxGroupRecords[i] a pointer to an
 * array of count 8-byte records whose first byte InitDreamAux clears. The
 * record's remaining 7 bytes are not accessed here. */
```

The MOM paths:

```c
/* "ETC\\SYMSPY.MOM" / "ETC\\SYMDOG.MOM" -- MOM = this game's audio-stream
 * format (per lsddecomp naming elsewhere in the project). Defined in
 * DreamAux.c, right before InitDreamAux which is their only reader. */
```
