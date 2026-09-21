> Renamed from `func_8005C9DC` on 2026-09-21 (tools/rename.py). Address 0x8005c9dc.

# FireDreamAuxTriggerEntries

**Unit:** code_4cd08 · **Size:** 54 words · **Status:** MATCHED round 43
(54/54, byte-exact whole-image build).

## History

Filed BLOCKED in round 2026-08-30-a on one `%gp_rel` reference (to
`gDreamAuxStage`) plus one `addiu_at` indexed-load. Round 42 resolved both.
Never actually attempted -- the stub carried no derivation. Round 43 derived
and matched it.

## What it does

Construct a `TriggerWorld` via `func_80044A0C`; if construction succeeds,
walk 3 candidate bytes (`a1[3..5]`, terminated early by a `-1` sentinel) and
fire `ProcessDreamAuxTriggerRecord` once per non-sentinel byte against the SAME
`gDreamAuxGroupRecords`/`gDreamAuxStage` parallel-group table `InitDreamAux` clears
(8-byte stride, confirmed there and reused here identically); the loop's
`ProcessDreamAuxTriggerRecord` results are discarded (called for side effects only). The
return value is just whether construction succeeded:

```c
extern TriggerWorld *func_80044A0C(s32 *ctx);
bool ProcessDreamAuxTriggerRecord(s32 value, void *ctx, TriggerRecord *record, TriggerWorld *world);

s32 FireDreamAuxTriggerEntries(s32 a0, s8 *a1, s32 a2)
{
    s32 ctxArg[4];
    TriggerWorld *world;

    ctxArg[0] = a2;
    world = func_80044A0C(ctxArg);

    if (world != NULL) {
        DreamAuxGroupRecord *base = gDreamAuxGroupRecords[gDreamAuxStage];
        s8 *p = a1 + 3;
        s8 *end = a1 + 6;

        while (p < end) {
            s8 entry = *p;

            if (entry == -1) {
                break;
            }
            ProcessDreamAuxTriggerRecord(a0, a1, (TriggerRecord *)((u8 *)base + entry * 8), world);
            p++;
        }
        return (s32)world;
    }
    return 0;
}
```

`ProcessDreamAuxTriggerRecord`'s `record` parameter here is a `DreamAuxGroupRecord *`
(8-byte stride, `gDreamAuxGroupRecords`) reinterpret-cast to `TriggerRecord *` (0x38-byte
stride, `ProcessDreamAuxTriggerRecord`'s own already-matched typed view). Both sizes are
independently confirmed correct for their own already-matched call sites
(`InitDreamAux`'s pointer scaling for the first, `ProcessDreamAuxTriggerRecord`'s own
`record + 1` recursion for the second) -- this call site legitimately hands
a small 8-byte slot to a function that privately treats it as a much larger
struct, which is safe here only because the recursive (`kind == 2`) arm of
`ProcessDreamAuxTriggerRecord` is never taken for these particular records. This is the
same kind of cross-type reinterpretation CLAUDE.md documents for
`CheckDreamAuxWorldState`/`AdjustDreamAuxTriggerOffset`'s shared `gDreamAuxWorld` global, just at a
struct-pointer level instead of a scalar.

`func_80044A0C` is a new symbol, not owned by this unit and not previously
declared anywhere in the tree; declared here with a minimal local prototype
(`TriggerWorld *func_80044A0C(s32 *ctx)`).

## Derivation notes

Two attempts short of byte-exact, both frame-shape mismatches rather than
logic errors -- the CALL sequence and branch structure were right from the
first build, but the STACK FRAME size was wrong twice:

1. **First pass (7/54, one word of frame missing, 81797 bytes of drift):**
   used `for (i = 3; i < 6; i++)` with `a1[i]` (integer indexing over two
   LITERAL bounds). GCC can statically prove `3 < 6` and unconditionally
   enters the loop, generating a do-while with no guard sltu -- retail's own
   asm has an explicit `sltu $v1, $s0, $s1` BEFORE the loop even starts. That
   only happens when the bound is a runtime POINTER COMPARISON the compiler
   cannot fold, i.e. retail's C walks `s8 *p`/`s8 *end` pointers, not integer
   indices, the same idiom `ProcessDreamAuxTriggerRecord` (already matched, same unit) uses
   for its own `entries` scan. Switching to `s8 *p = a1+3; s8 *end = a1+6;
   while (p < end) { ...; p++; }` reproduced the guard (36/54, no more
   out-of-range drift).
2. **Second pass (36/54, frame still 8 bytes/2 words short):** with
   `s32 ctxArg = a2;` passed as `&ctxArg` to `func_80044A0C`, every
   instruction inside the function matched except the FRAME SIZE itself
   (`addiu $sp,$sp,-0x38` vs retail's `-0x40`) and the consequent save-slot
   offsets. Retail reserves 0x10 bytes (4 words) at the bottom of its frame
   for this local, not 4 bytes for one word -- the same "caller hands the
   callee a stack buffer wider than what gets explicitly written" shape as
   `SetDreamAuxWorld`'s `New_Entity` call (a 4-word buffer with only the last
   word set) documented earlier this round. Widening `ctxArg` from a scalar
   to `s32 ctxArg[4]` (only `ctxArg[0]` ever written) matched the frame size
   exactly and reached byte-exact.

## Proposed learning

Two entries, both reinforcing patterns already on file rather than new
mechanisms:

- Confirms the round-43 `DespawnDreamAuxEntity`/`SetDreamAuxWorld` observation that a
  scratch buffer handed to an external call can be WIDER than what the
  caller itself writes -- a THIRD instance in this same unit
  (`SetDreamAuxWorld`'s `New_Entity` buffer, `DespawnDreamAuxEntity`'s implicit
  `localPos`, now `FireDreamAuxTriggerEntries`'s `ctxArg[4]`). When a local's frame
  footprint comes up short by a clean multiple of 4 bytes with every
  instruction otherwise matching, suspect an under-sized scratch buffer
  before anything else.
- Confirms (does not add) the existing "pointer-walk over two runtime
  bounds needs actual pointers, not integer indices over literal bounds" --
  already implicit in `ProcessDreamAuxTriggerRecord`'s own body, now independently
  reproduced by matching a second, unrelated function against the same
  idiom.
