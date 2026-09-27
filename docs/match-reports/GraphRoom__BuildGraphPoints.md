# GraphRoom__BuildGraphPoints — MATCH (56/56 words, whole-image SHA1 confirmed)

> Renamed from `GraphRoomObj__BuildGraphPoints` on 2026-09-26 (tools/rename.py). Address 0x80058228.

> Renamed from `func_80058228` on 2026-09-24 (tools/rename.py). Address 0x80058228.

Unit `class_3bb8c_t`. Byte-exact. History below in arrival order.

## Correction to this round's staffing: this was NOT fresh ground

Round 19's fourth-pass assignment described this function as "FRESH — no
prior attempt, no inherited verdict, no preserved body," treating
`GraphRoom__PopulateGraphPoints` as the documented stall. **That is backwards.** This
report already existed (committed in `4358700`, "Salvage two in-flight
bodies...") with a near-complete 52/56 mid-attempt snapshot from runner
alpha (round 17); `GraphRoom__PopulateGraphPoints` is the one with no report file at all
(`ls docs/match-reports/` confirms only `GraphRoom__BuildGraphPoints.md` and
`GraphRoom__ScoreDayLog.md` exist for this unit's three queued functions).
`tools/progress.py`'s own `fresh` count for `class_3bb8c_t` (1) is
correct — it just points at `GraphRoom__PopulateGraphPoints`, not this function. Given
this report's near-miss was extremely close and cheap to re-attempt, it
was worth closing before moving to the genuinely cold function; see this
round's final summary for the disposition of `GraphRoom__PopulateGraphPoints`.

## What it does

Builds a 100-entry array of allocated `D_80087AACEntry` objects
(`self->unk_0xA8[0..99]`), each constructed via `New_BoxFill` (already
matched, `code_2cc8c_e.c`) with a colour-like 3-byte argument: entry 0
gets the constant `gGraphPointNewestColor` directly; entries 1-99 get successive
values of a mutable local copy of `gGraphPointBaseColor` (also 3 bytes), decremented
by 0x14 for the first 6 loop iterations and by 1 thereafter. Finally
allocates a 4-byte scratch buffer into `self->unk_0x240`.

## Final body (landed in `src/class_3bb8c_t.c`)

```c
/* A 3-byte colour-ish triple, read/written strictly byte-for-byte in
 * DECLARATION order (round 19, verified against retail byte-for-byte --
 * the natural sequential order is what matches, no reordering needed).
 * Field names are a plausible RGB reading of a colour-cycling table
 * builder, not confirmed evidence. */
typedef struct GraphPointColor {
    s8 r;
    s8 g;
    s8 b;
} GraphPointColor;
extern u8 gGraphPointSize;
extern u8 gGraphPointNewestColor;
extern GraphPointColor gGraphPointBaseColor;
extern D_80087AACEntry *New_BoxFill(void *a0, void *a1, s32 a2);

void GraphRoom__BuildGraphPoints(D_80087AACObj *self) {
    GraphPointColor rgb;
    s32 i;

    self->unk_0xA8[0] = New_BoxFill(&gGraphPointSize, &gGraphPointNewestColor, 0);
    rgb = gGraphPointBaseColor;
    for (i = 1; i < 100; i++) {
        s32 dec;

        self->unk_0xA8[i] = New_BoxFill(&gGraphPointSize, &rgb, 0);
        dec = 1;
        if (i < 7) {
            dec = 0x14;
        }
        rgb.r -= dec;
        rgb.g -= dec;
        rgb.b -= dec;
    }
    self->unk_0x240 = BMemPMgrAlloc(4);
}
```

`New_BoxFill` is called with `&gGraphPointSize` (its own address, opaque,
never dereferenced by this function) as the first argument -- the
salvaged snapshot below passed `gGraphPointSize` bare (without `&`), which
would only compile/link correctly if `gGraphPointSize` were itself already a
pointer-typed global; declaring it as a plain byte and taking its address
explicitly is the safer, self-consistent reading and is what was used
here.

## What closed it: alpha's near-miss had the field DECREMENT order wrong, not the STRUCT'S field order

Alpha's snapshot (52/56, preserved below for provenance) already had the
entire control flow, both `New_BoxFill` calls, the loop bound, and the
6-vs-1 decrement-size branch byte-exact -- the ONLY residue was two
swapped store offsets (`+0x11`/`+0x12`), confined to the tail of the
loop's decrement sequence. Alpha's own body decremented in the order
`r, b, g` (explicitly NOT the struct's declared field order, per its own
report's note), on the theory that retail's actual per-field write order
must be r, b, g rather than r, g, b.

**That theory was the wrong half to vary.** Retracing retail's own
disassembly instruction-by-instruction (not by reasoning about which
order "looks more plausible") shows the compiled instruction sequence is:
load field0 and field2 together, decrement-and-store field0, THEN load
field1, decrement-and-store field2 (using the register loaded earlier),
decrement-and-store field1 -- an interleaved load/store pattern that
looks superficially like a "0, 2, 1" write order but is actually GCC's
own scheduling of three INDEPENDENT byte operations, not a literal
transcription of source statement order. Writing the decrements in the
struct's own natural declared order (`rgb.r -= dec; rgb.g -= dec; rgb.b
-= dec;`, i.e. what alpha's report explicitly says it deliberately did
NOT try, believing the "not r/g/b" order was required) reproduces this
exact interleaved schedule byte-for-byte. Verified directly: swapping
back to `r, b, g` order (matching alpha's snapshot exactly) reproduces
the identical 52/56 residue at the identical two words; `r, g, b`
order is byte-exact.

### Proposed learning

**When three independent byte-sized field writes show a residue that
looks like two swapped store offsets, do not assume the fix is to
reorder the SOURCE statements to match the apparent physical write
order.** GCC 2.6.3's scheduler can interleave loads and stores of
independent bytes into a sequence that only superficially resembles a
different statement order -- here the plain, natural declared-field-order
source (`r, g, b`) was the one that reproduced retail's own scheduling
choice, while the "matches the apparent write order" source (`r, b, g`)
did not. Test the boring, natural ordering FIRST and empirically, rather
than reasoning from the compiled instruction sequence's surface
appearance to a source order.

Also: a rewrite that changes the local's TYPE (a `s8[3]` array instead of
a 3-field `s8` struct, and/or a ternary instead of an `if`/no-`else` for
the decrement-size branch) is not a safe intermediate step even when it
looks equivalent -- an early attempt at this function using an array
local and a ternary regressed to 10/56 with severe address drift, despite
seemingly encoding the identical logic. Change ONE axis at a time against
an already-close near-miss; a struct-shaped local with an `if`, matching
alpha's original snapshot's shape exactly except for the one line that
needed to change, is what reached byte-exact.

## History: alpha's snapshot (52/56, superseded above)

**This was a mid-attempt snapshot salvaged by the head, not a considered
plateau.** Runner alpha died to an infrastructure failure (weekly API
limit) with this body uncommitted in its worktree. No author ever applied
a stop rule to it. Salvaged under `docs/PARALLEL-RUNS.md` §4c.

Measured by the head, by copying alpha's worktree file into `main`,
running the full oracle, and restoring:

```
GraphRoom__BuildGraphPoints: 52/56 words match (file 0x48A28-0x48B08)
```

### The residue as originally described (superseded — see above)

```
off=0x048AC8 vram=0x800582C8 DIFF retail=1200a3a3 built=1100a3a3
off=0x048AD0 vram=0x800582D0 DIFF retail=1100a2a3 built=1200a2a3
```

Both differing words are `sb` stores, and the two immediates are swapped:
retail stores at `+0x12` where the build stores at `+0x11`, and at `+0x11`
where the build stores at `+0x12`.

### Body, as salvaged (superseded — do not use; the decrement order below is wrong)

```c
#if 0
void GraphRoom__BuildGraphPoints(D_80087AACObj *self) {
    GraphPointColor rgb;
    s32 i;

    self->unk_0xA8[0] = New_BoxFill(gGraphPointSize, &gGraphPointNewestColor, 0);
    rgb = gGraphPointBaseColor;
    for (i = 1; i < 100; i++) {
        s32 dec;

        self->unk_0xA8[i] = New_BoxFill(gGraphPointSize, &rgb, 0);
        dec = 1;
        if (i < 7) {
            dec = 0x14;
        }
        rgb.r -= dec;
        rgb.b -= dec;
        rgb.g -= dec;
    }
    self->unk_0x240 = BMemPMgrAlloc(4);
}
#endif
```

## Provenance

Round 17, runner alpha (killed mid-attempt, recovered by the head per
PARALLEL-RUNS §4c). Round 19 fourth pass, runner delta: corrected the
staffing mislabeling, fixed the decrement-order bug, MATCHED 56/56.

## Naming (round 75, track 3)

**`GraphRoom__BuildGraphPoints`** -- tier B. Own vtable slot +0x0D8.
Builds the 100-entry `points` array of small coloured `New_BoxFill`
objects that the graph plots dream-history onto (see class header
comment); also allocates the 4-byte `matchedDayIndices` scratch buffer
`GraphRoom__ScoreDayLog` later fills in.

## Track 4 (2026-09-25, round 85, charlie)

The points are New_BoxFill boxes (include/BoxFill.h): size &gGraphPointSize, colour &gGraphPointNewestColor then the fading `rgb`, priority 0. Zero bytes.

## Track 4 (2026-09-26, round 87, alpha): renamed `GraphRoomObj__BuildGraphPoints` -> `GraphRoom__BuildGraphPoints`

The class is unified in `include/GraphRoom.h` (class id 0x2F130, table `gGraphRoomMethods`, parent TaskCore; `tools/classtable.py gGraphRoomMethods --vs gTaskCoreMethods`). The class name drops round 75's `Obj` suffix (FINISHING-PLAN track 4 step 2); every function prefix moved with it. The unit's GraphRoomObj/GraphRoomMethods views are deleted and `self` is `GraphRoom *`; inherited fields and slots carry TaskCore's and IntermediateBase's names. Zero bytes changed (build-and-verify OK, typeviews --warnings 0 new). Prefix only. Occupies +0x0D8, TaskCore's `setTarget`, and keeps its own name (step 6: the body sets no target, it builds the 100 BoxFill dots and matchedDayIndices). The ctor reaches it as setTarget(self, NULL).

## Track 6 (2026-09-27, round 97, delta): `D_8008ABB8Color` -> `GraphPointColor`, `D_8008ABB8` -> `gGraphPointBaseColor`

- **`GraphPointColor`** (tier A for what it is): the colour argument this function hands `New_BoxFill`; `BoxFill__SetColor` copies its three bytes into the box's `GsBOXF` r, g, b (`include/BoxFill.h`, `color[3]`), which confirms round 19's "plausible RGB reading" of the field names. It is the only user of the type (`grep -rn` over `src/`, no other view).
- **Not Sony's `CVECTOR`**: `CVECTOR` is `u_char r, g, b, cd`, four bytes. Retail copies the global into the local with three `lb`/`sb` pairs (signed, three bytes), so a four-byte unsigned struct would not compile to it. Kept as its own `s8` triple, like `BgLayerRgb`/`ViewportRgb`.
- **`gGraphPointBaseColor`** (tier A): 4 bytes of sdata `FF FF FF 00`, white. This function is its only reader: points 1..99 start from a copy of it and darken by 0x14 per point for the first six, then by 1. (Point 0 takes `gGraphPointNewestColor`, `FF 00 00`, red -- left unnamed, outside the job.)
- The type's comment moved round 19's history here and says what the type is; zero bytes changed.

## Track 7 (2026-09-27, round 97, delta)

- **Naming: `D_8008ABAC` -> `gGraphPointSize`** (tier A): two sdata words `{10, 10}`, New_BoxFill's size argument (BoxFill reads the low halfwords into boxW/boxH, include/BoxFill.h `SkipShort2`) for all 100 dots. Its only user. Now declared `s32 gGraphPointSize[2]` and passed bare instead of `&` of a `u8`: zero bytes changed.
- **Naming: `D_8008ABB4` -> `gGraphPointNewestColor`** (tier A): sdata `FF 00 00`, red, the colour of `points[0]`, which PopulateGraphPoints plots from the newest logged day (`currentDay - 1`) and Update blinks. Its only user. Declared `GraphPointColor` instead of `u8`: zero bytes changed.
- Local `dec` -> `step`; the loop bound is `ARRAY_COUNT(self->points)`; the darkening step 0x14 is decimal 20; `matchedDayIndices` is `BMemPMgrAlloc(GRAPH_SCORE_MOOD_COUNT * sizeof(s8))`, one byte per gGraphScoreMoods entry.
