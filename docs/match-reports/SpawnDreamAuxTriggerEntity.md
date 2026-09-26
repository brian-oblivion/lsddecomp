# SpawnDreamAuxTriggerEntity

> Renamed from `func_8005CDF8` on 2026-09-21 (tools/rename.py). Address 0x8005cdf8.

**Unit:** code_4cd08 · **Size:** 79 words · **Status:** MATCHED round 43
(79/79, byte-exact whole-image build). This was the LAST of the eight
functions assigned to runner charlie this round -- `code_4cd08.c` now has
zero `INCLUDE_ASM` lines.

## History

Filed BLOCKED in round 2026-08-30-a on five `%gp_rel` references (the first
to `D_8008AC04`) plus three `addiu_at` indexed loads. Round 42 resolved both
mechanisms. Never actually attempted -- the stub carried no derivation.
Round 43 derived and matched it, the biggest function in the unit.

## What it does

Spawn an `Entity` for a trigger `entry`; on success, fill a small local
coordinate buffer from two lookup tables and dispatch it through three
vtable calls (two through the new entity, one through a second global
object); on `New_Entity` failure, return `true` immediately instead:

```c
typedef struct {
    u16 val0;
    s8 val2;
    s8 posIndex;
} DreamAuxSpawnInfo;

extern DreamAuxSpawnInfo gDreamAuxSpawnInfo[];

typedef struct {
    s16 x;
    s16 y;
} DreamAuxPosXY;

typedef struct {
    DreamAuxPosXY xy;
    s16 z;
} DreamAuxPos6;

extern DreamAuxPos6 gDreamAuxPosTable[];
extern u8 D_80088F18[];

typedef void (*DreamAuxObjFn11)(DreamAuxObj *self, s32 arg1, void *arg2);
typedef void (*DreamAuxObjFn3A)(DreamAuxObj *self, void *arg1, void *arg2);

bool SpawnDreamAuxTriggerEntity(s32 kind, void *out, void *ctx, s32 entry)
{
    DreamAuxObj *entity = (DreamAuxObj *)New_Entity((void *)kind, out, (void *)D_8008AC04);

    if (entity != NULL) {
        DreamAuxSpawnInfo *rec;
        struct {
            u16 ctxVal;
            u16 recordVal0;
            DreamAuxPos6 pos;
        } coords;
        s32 outBuf[4];
        DreamAuxObj *obj;

        coords.ctxVal = *(u16 *)ctx;
        rec = &gDreamAuxSpawnInfo[entry];
        coords.recordVal0 = rec->val0;
        coords.pos = gDreamAuxPosTable[rec->posIndex];

        obj = (DreamAuxObj *)D_8008ABFC;
        ((DreamAuxObjFn3A)obj->vtable[0x3A])(obj, outBuf, &coords);
        ((DreamAuxObjFn11)entity->vtable[0x11])(entity, 1, D_80088F18 + rec->val2 * 12);
        ((DreamAuxObjFn13)entity->vtable[0x13])(entity, gDreamAuxWorld, D_8008AC08, (void *)D_8008ABFC, outBuf);
        return false;
    }
    return true;
}
```

`New_Entity`'s three arguments here are the exact same shape as
`SetDreamAuxWorld`'s call: `(kind, out, D_8008AC04)`, where `out` is THIS
function's own `void *out` parameter, itself a 4-word caller-provided
scratch buffer per the header's existing comment on this function's
signature (`ProcessDreamAuxTriggerRecord`'s `scratch[4]`). `gDreamAuxSpawnInfo`/`gDreamAuxPosTable` are two
more small unit-owned lookup tables (a 4-byte "spawn info" record indexed by
`entry`, and a 6-byte position record indexed by that record's `posIndex`
field, named round 63). `entity->vtable[0x13]` is the SAME slot `DespawnDreamAuxEntity` (matched
earlier this round) dispatches through, reusing `DreamAuxObjFn13`.
`D_8008ABFC`'s vtable slot 0x3A (byte offset 0xE8) and `entity`'s slot 0x11
(byte offset 0x44) are new, function-local typedefs.

## Derivation notes

Four attempts to byte-exact, three distinct mechanisms:

1. **First pass (3/79, one word too long, 90339 bytes of drift):** wrote
   `kind`/`entry` as `u8` (matching the ALREADY-CORRECT prototype this
   function's caller, `ProcessDreamAuxTriggerRecord`, already used) and cast `kind` through
   an explicit `(void *)(s32)kind`. This produced spurious `andi $x, $y,
   0xff` masks at the FUNCTION ENTRY for both parameters, which retail does
   not have. This is exactly the round-10-era learning already on file in
   `docs/DECOMPILATION_LEARNINGS.md`: **"Passing a `u8` lvalue straight to an
   `int`/`s32` parameter costs a redundant `andi`... Read the callee's body
   for the width it actually uses, not the callers for the width they happen
   to pass."** The callee's own body treats both `kind` and `entry` as full
   words (array index, opaque `void *` argument) with no narrowing -- so the
   fix was to WIDEN THE CALLEE'S OWN PARAMETER TYPES to `s32` in both the
   definition and (necessarily, to avoid a silent `conflicting types` error)
   the shared header prototype. Rebuilding confirmed `ProcessDreamAuxTriggerRecord` (the
   only caller, already matched) is UNAFFECTED by this change -- its own
   69/69 score held exactly, because its call-site arguments are already
   memory loads (`record->kind`, a cast byte read) that zero-extend for free
   regardless of the callee's declared width.
2. **Second pass (28/79, 122901 bytes of drift, function now too SHORT):**
   with the parameters widened, the masks were gone, but the two
   lookup-table field reads (`rec->val3`, used once for the `xy`/`z` copy)
   were split across two separate C statements (`coords.xy = ...; coords.z =
   ...;`), each independently re-reading `rec->val3` and recomputing its
   `* 6` address. Retail computes that index and its `* 6` scaling ONCE and
   derives all three addresses (the `lwl` symbol, the `lwr` symbol, and the
   `z` half-word's symbol) from the same register before storing anything.
3. **Third pass, the fix:** merged the destination fields into a single
   embedded `DreamAuxPos6 pos;` member and did ONE whole-struct assignment,
   `coords.pos = gDreamAuxPosTable[rec->val3];`, instead of two separate sub-field
   assignments. This is the SAME "all-s8/s16 struct, alignment 2, whole-
   struct assignment compiles to `lwl`/`lwr` + `swl`/`swr`" idiom CLAUDE.md
   already documents (confirmed there three times previously) -- but this is
   the first instance in this unit where getting the WHOLE-STRUCT COPY
   shape right (one assignment of the embedded struct, not per-field
   assignments of its two halves) was the difference between computing the
   shared index once versus twice. Byte-exact immediately after (79/79).

The `gDreamAuxPosTable`/`D_80088D3F` symbol pair (used respectively for the `lwr`
and `lwl` halves of the unaligned load, 3 bytes apart) is not referenced by
name anywhere in this unit's C -- only `gDreamAuxPosTable` appears in the source,
scaled by `sizeof(DreamAuxPos6)` (6) through ordinary array indexing.
`D_80088D3F` is retail's own separately-named symbol for `gDreamAuxPosTable+3`
(the byte address `lwl` needs); since both resolve to the same linked
address, the compiler's own `+3` computation over `gDreamAuxPosTable` produces
identical final bytes to referencing `D_80088D3F` directly. Nothing needed
to be added to `config/` for this.

## Proposed learning

Add to the corpus, as a refinement of the existing "whole-struct assignment"
idiom entry: **when a struct-with-alignment-2 field is embedded inside a
LARGER local (here, `coords`), the source MUST be copied as one whole-struct
assignment of the embedded member, not as separate assignments of its own
sub-fields** -- splitting it (`dst.sub1 = src.sub1; dst.sub2 = src.sub2;`)
independently recomputes the shared index/address for each half, which is
both extra instructions AND, if the index expression has any side effect or
even just a register-pressure cost, a different schedule entirely. The tell
here was the function coming out too SHORT with a large out-of-range drift
(`122901 bytes`) -- the opposite signature from the round-10 entry's
"redundant `andi`" (which makes code longer), a useful reminder that both
directions of length mismatch can come from the same family of "narrow type
handled at the wrong granularity" issue.

## Naming

**SpawnDreamAuxTriggerEntity** — tier B. Spawns an `Entity` via `New_Entity`
for a trigger `entry`; on success, fills a local coordinate buffer from
`gDreamAuxSpawnInfo`/`gDreamAuxPosTable` and dispatches it through three
vtable calls (two through the new entity, one through `D_8008ABFC`); on
`New_Entity` failure returns `true` (treated as "handled" by callers) rather
than `false`. Named for the mechanic that dominates the body (spawn +
attach); tier B since the exact game meaning of the coordinate/dispatch
sequence is not established from this unit alone. Renamed
`DreamAuxSpawnInfo.val3` to `posIndex` in this pass (definition-only
rename, confirmed confined to this unit by rebuild).

## Track 4 (2026-09-26, round 88, echo)

The entity is now `Entity *` and its raw `vtable[0x11]`/`vtable[0x13]` calls are the typed slots updateRotation and attachToParent (through TodActorAttachToParentFn: peer gDreamAuxWorld, companion D_8008AC08, parent D_8008ABFC, offset outBuf), same bytes. DreamAuxObjFn11/13 deleted.

Byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
