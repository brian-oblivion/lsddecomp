# GraphRoom__PopulateGraphPoints — MATCH (108/108 words, whole-image SHA1 confirmed)

> Renamed from `GraphRoomObj__PopulateGraphPoints` on 2026-09-26 (tools/rename.py). Address 0x80058404.

> Renamed from `func_80058404` on 2026-09-24 (tools/rename.py). Address 0x80058404.

Unit `ObjMStyleActor`. Byte-exact. Genuinely fresh ground — no prior
attempt, no inherited verdict, no preserved body (this round's fourth
pass incorrectly named `GraphRoom__BuildGraphPoints` as the fresh one; see that
function's own report for the correction — this is the one that actually
was).

## What it does

Called on `self` (a `D_80087AACObj`) with an opaque `arg1` forwarded to
several places:

1. Calls the shared base class's `+0xE0` slot (a new slot, this unit's
   first call through it) with `(self, arg1)`.
2. Fetches `result = self->unk_0xA4->methods->slot1B0(self->unk_0xA4, 0)`
   (same already-documented call `GraphRoom__Update` also makes).
3. Calls `GraphRoom__ScoreDayLog` (still `addiu_at`-blocked, this unit) with
   `(self, result)`, storing its return into `self->unk_0x238`.
4. Computes a vertex count: 100 if `result->unk_0x4 != 0`, otherwise
   `result->unk_0x8` clamped to 100.
5. Walks a circular table of 2-byte `{dx, dy}` entries backward from
   `result->unk_0x8 - 1`, wrapping to `0x16C` when the index goes
   negative, for `count` iterations. Each entry's two signed bytes
   (`+0x18`/`+0x19` relative to the entry) are transformed into a 2-word
   `{x, y}` point (`x = dx*10-5`, `y = -dy*10-5`) and dispatched to
   `self->unk_0xA8[i]->methods->slotC4(entry, arg1, &point, 0)` — except
   for `i == 0`, whose point is SAVED rather than dispatched immediately.
6. After the loop, if any iteration ran (`i==0` was reached), dispatches
   the SAVED point to `self->unk_0xA8[0]` — i.e. entry 0's dispatch is
   deferred to the very end, after every other entry has already fired.

## Final body

```c
/* +0x0E0, called by this unit's own GraphRoom__PopulateGraphPoints as (self, arg1) -- the
 * FIRST thing that function does, before touching anything else. */
/* (added to D_8006E730Methods, see include comment in src/world/ObjMStyleActor.c) */

/* +0x0C4, called by this unit's own GraphRoom__PopulateGraphPoints as (self, arg1, &point,
 * 0), where `point` is a 2-word {x, y}-shaped local. */
/* (added to D_80087AACEntryMethods) */

extern s32 GraphRoom__ScoreDayLog(D_80087AACObj *self, D_80087AACUnkA4Result *arg1);

typedef struct Point2 {
    s32 x, y;
} Point2;

void GraphRoom__PopulateGraphPoints(D_80087AACObj *self, void *arg1) {
    D_80087AACUnkA4Result *result;
    s32 count;
    s32 i;
    s32 idx;
    s32 flag;
    Point2 point;
    Point2 firstPoint;

    GetTaskCoreMethods()->slotE0(self, arg1);
    result = self->unk_0xA4->methods->slot1B0(self->unk_0xA4, 0);
    self->unk_0x238 = GraphRoom__ScoreDayLog(self, result);

    flag = 0;
    if (result->unk_0x4 != 0) {
        count = 100;
    } else {
        count = result->unk_0x8;
        if (count >= 0x65) {
            count = 100;
        }
    }

    idx = result->unk_0x8 - 1;
    for (i = 0; i < count; i++, idx--) {
        s8 *p;
        s8 dx, dy;
        s32 ndy;

        if (idx < 0) {
            idx = 0x16C;
        }
        p = (s8 *)((u8 *)result + idx * 2);
        dx = p[0x18];
        point.x = dx * 10 - 5;
        dy = p[0x19];
        ndy = -dy;
        point.y = ndy * 10 - 5;

        if (i == 0) {
            firstPoint = point;
            flag = 1;
        } else {
            self->unk_0xA8[i]->methods->slotC4(self->unk_0xA8[i], arg1, (s32 *)&point, 0);
        }
    }

    if (flag) {
        self->unk_0xA8[0]->methods->slotC4(self->unk_0xA8[0], arg1, (s32 *)&firstPoint, 0);
    }
}
```

## New struct knowledge

- `D_8006E730Methods` (the shared base class table) gained slot `+0xE0`,
  appended immediately after the previously-highest slot `+0xDC` — a pure
  append, no padding recount needed.
- `D_80087AACEntryMethods` gained slot `+0xC4` (`void
  (*slotC4)(D_80087AACEntry *self, void *arg1, s32 *point, s32 arg3)`),
  with an 8-byte pad inserted between the existing `+0xB8` slot and the
  new one (`0xB8 + 4 = 0xBC`, gap to `0xC4` is 8 bytes) — additive, total
  preserved, verified by the full oracle passing.

## Three genuine decompilation levers, none of them a register-identity
## workaround

This was cold ground (no prior attempt to correct), so the levers below
are worth recording precisely since each one closed a real, measured gap
rather than reshaping around an unreachable residue.

**1. Two independent byte reads followed by arithmetic: read-and-fully-
consume the FIRST value as a complete statement before reading the
SECOND, rather than reading both up front.** Reading `dx` and `dy`
together (both loads adjacent, then both `*10-5` transforms after) makes
GCC 2.6.3 emit `lb`+`lbu`+manual-sign-extend (`sll`/`sra` by 24) for the
SECOND byte specifically, even though it is declared `s8` and read
through an `s8*` — reproduced in isolation with a 12-line standalone `cc1`
probe through the pinned pipeline (three variants: reading both up front
in either order both regress to the lbu+extend pattern for whichever load
is scheduled second; reading `dx`, fully computing and storing `dx*10-5`,
THEN reading `dy` and negating it immediately reproduces retail's plain
`lb`/`lb` pair exactly). The type of the pointer (`u8*` vs `s8*`) made NO
difference in isolation — only the STATEMENT-LEVEL separation between the
two reads did. Worth checking whenever two adjacent byte-sized reads
compile with one plain `lb` and one unexplained `lbu`+sign-extend pair:
suspect the SECOND read is being scheduled too early relative to the
first read's own arithmetic, not a type mismatch.

**2. Anonymous structs with identical members are NOT compatible types in
C, and a struct assignment between two of them can silently compile to
something other than a copy.** Declaring `point`/`firstPoint` as two
SEPARATE anonymous `struct { s32 x, y; }` locals and writing `firstPoint =
point;` compiled clean with no warning surfaced by this build, but
produced a build that DROPPED the assignment's observable effect
entirely — the compiler treated the two variables as unrelated types with
no valid conversion between them and the assignment statement effectively
evaporated (confirmed via `objdump`: no store to `firstPoint`'s stack slot
appeared anywhere). Giving both locals the SAME named struct tag
(`Point2`) reproduced retail's actual RELOAD-from-memory pattern
(`lw`/`lw` from `point`'s stack slots, `sw`/`sw` to `firstPoint`'s) —
which is itself informative: retail's own source treats this as a genuine
struct-to-struct copy (memory-to-memory), not a value already sitting in
a register being reused, even though the value was JUST computed two
instructions earlier. **This is worth flagging past this one function: an
anonymous-struct type mismatch is not something `-Wall` reliably surfaces
in this pinned toolchain, and it does not always fail loudly — check the
actual object code, not just a clean compile, whenever two locals of the
same "shape" need to interact.**

**3. A `for` loop's multi-variable increment clause has a load-bearing
order, extended past the array-name case DECOMPILATION_LEARNINGS already
documents.** Writing `idx--;` as the loop body's own last statement (with
only `i++` in the `for` clause) schedules the circular index's
decrement BEFORE the implicit array-pointer advance GCC performs for
`self->unk_0xA8[i]`'s own indexing. Moving `idx--` into the `for` clause
itself (`for (i = 0; i < count; i++, idx--)`) reordered the two
decrements to match retail exactly (array-pointer advance first, `i++`
second, `idx--` last, filling the loop branch's own delay slot) — this
closed the LAST two-word residue. The array-pointer advance here is
IMPLICIT (never spelled out in C at all — it exists only because GCC
walks `unk_0xA8[i]` via a stepped pointer under the hood), so this is a
new instance of the documented lever in a case where one of the two
"variables" being ordered isn't a C-level name at all.

### Proposed learning

Three narrow, concrete levers, all confirmed on cold ground (no inherited
verdict to correct):
- Two adjacent byte reads feeding independent arithmetic: fully compute
  and store the first value's result before reading the second byte, to
  avoid an unexplained `lbu`+manual-sign-extend on the second read.
- Two struct locals of the "same shape" declared as separate anonymous
  `struct { ... }` types are NOT the same type in C and a copy between
  them can silently no-op; give them one shared tag and verify the
  resulting object code, not just a clean compile.
- A `for` loop's increment clause ordering governs relative scheduling
  even against an INDEX GCC computes implicitly (an array-walk pointer
  with no C-level name) — the existing "multi-variable increment order"
  entry generalises past named locals.

## Provenance

Round 19 fourth pass, runner delta. Cold ground, no prior attempt.
MATCHED 108/108 on the first full derivation, after the three levers
above closed the residues found while verifying against the real oracle.

## Naming (round 75, track 3)

**`GraphRoom__PopulateGraphPoints`** -- tier B. Own vtable slot +0x0E0.
Fetches the day-log, scores it (`GraphRoom__ScoreDayLog`), then walks
the day ring backwards computing each `{x, y}` point and dispatching it to
the matching `points[i]`'s `setPosition` slot -- the function that
actually lays the graph's dots out on screen, matching the class's own
identity evidence.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `GetTaskCoreMethods()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-25, round 85, charlie)

points[] are BoxFills (include/BoxFill.h); the deleted `GraphRoomPoint` view's `setPosition` (+0x0C4) is attachAbsolute(parent, &point, 0): attach at a pixel position. Zero bytes.

## Track 4 (2026-09-26, round 87, alpha): renamed `GraphRoomObj__PopulateGraphPoints` -> `GraphRoom__PopulateGraphPoints`

The class is unified in `include/GraphRoom.h` (class id 0x2F130, table `gGraphRoomMethods`, parent TaskCore; `tools/classtable.py gGraphRoomMethods --vs gTaskCoreMethods`). The class name drops round 75's `Obj` suffix (FINISHING-PLAN track 4 step 2); every function prefix moved with it. The unit's GraphRoomObj/GraphRoomMethods views are deleted and `self` is `GraphRoom *`; inherited fields and slots carry TaskCore's and IntermediateBase's names. Zero bytes changed (build-and-verify OK, typeviews --warnings 0 new). Prefix only. Occupies +0x0E0, TaskCore's `updateSlotElements`, and keeps its own name (step 6: after the base call it scores the log and places one dot a day). The +0x1B0 call is DreamSys__GetSaveBlock through dream_sys.h; the record is DreamSaveBlock (fields currentYear/currentDay/moodPreviousDays, DreamSys's names at the same offsets from saveMagic).

## Track 7 (2026-09-27, round 97, delta)

- **The raw offsets are fields.** `p = (s8 *)((u8 *)result + idx * 2); dx = p[0x18]; dy = p[0x19];` is now `save->moodPreviousDays[day].axis.dynamic` / `.axis.upper`: DreamSaveBlock's ring is typed `MoodGraphPoint[DAYS_PER_YEAR]` (include/common.h's union, the type dream_sys.h declares the ring with). Zero bytes changed.
- **Measured: the pointer form does not match.** `MoodGraphPoint *p = &save->moodPreviousDays[day]; dx = p->axis.dynamic; dy = p->axis.upper;` makes cc1 add the array's +0x018 to the pointer first (an extra `addiu`, the loop shifted from word 50 on). Indexing the array twice is byte-exact, and carries a `MATCHING:` line.
- `Point2` deleted: it was `BoxFillPos` (include/BoxFill.h), the type attachAbsolute takes, and the two `(BoxFillPos *)` casts went with it.
- Constants: the 100s are `ARRAY_COUNT(self->points)` (`count >= 0x65` is `count > ARRAY_COUNT(...)`), 0x16C is `DAYS_PER_YEAR - 1`, and `* 10 - 5` is `* GRAPH_PIXELS_PER_MOOD - GRAPH_POINT_SIZE / 2`: sGraphPointSize is 10 square and BoxFill's position is the GsBOXF's top-left, so the -5 centres each dot on its mood times 10. Two names because the two 10s mean different things.
- Locals: `result` -> `save`, `idx` -> `day`, `flag` -> `haveNewest`.
- Moved from the source, verbatim, the DreamSaveBlock comment's history and the Point2 comment:

```c
/* What DreamSys__GetSaveBlock (the DreamSys's +0x1B0) returns: &saveMagic,
 * the 0x700-byte save block. This record reads it from there; the offsets
 * are DreamSys's own fields relative to saveMagic (DreamSys +0x178):
 * currentYear, currentDay, moodPreviousDays[365] (include/dream_sys.h). The
 * round-24 reading of this record ("a separate day-log object, not
 * DreamSys") predates knowing who passes the ctor's argument:
 * GameApplication__RunTitleMenu passes its dreamSys.
 *
 * +0x467 is DreamSys +0x5DF, the last byte of DreamSys's
 * unknown_values_0x5d8[8]: ScoreDayLog fails once it is set and sets it on
 * success, so the graph's highlight runs once per save. */
```
