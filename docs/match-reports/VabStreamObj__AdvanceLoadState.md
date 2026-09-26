# VabStreamObj__AdvanceLoadState -- MATCHED 74/74 (round 43)

> Renamed from `VabStreamObj__Update` on 2026-09-26 (tools/rename.py). Address 0x8002c6fc.

> Renamed from `func_8002C6FC` on 2026-09-18 (tools/rename.py). Address 0x8002c6fc.

Unit `code_179d8_e`. Previously filed as a `gp_rel` stall (round 17, never
attempted); reopened round 42 once `--gp-symbols`/`--no-nop-mflo-mfhi`
resolved that blocker project-wide. Confirmed against `gVabStreamObjMethods`'s own
rodata as that table's `+0x64` slot -- the class's own per-frame poll/update
method: it switches on the object's own load state (`self->unk2A`) and drives
the VAB header/body streaming state machine.

## Derivation

```c
void VabStreamObj__AdvanceLoadState(VabStreamObj *self) {
    char path[0x20];

    switch (self->loadState) {
    case 0:
        break;
    case 1:
        if (self->flags & 0x200) {
            self->vabId = SsVabOpenHead(self->streamBuffer, -1);
            func_800270C4(path, self->baseFilename, NULL, gVabBodySuffix);
            gPendingVabBuffer = self->streamBuffer;
            self->loadState = 6;
            self->streamBuffer = NULL;
            self->methods->slot58(self, path);
            if (self->baseFilename != NULL) {
                BMemPMgrFree(self->baseFilename);
                self->baseFilename = NULL;
            }
        }
        break;
    case 6:
        if (self->flags & 0x200) {
            self->vabId = SsVabTransBody(self->streamBuffer, self->vabId);
            if (self->vabId != -1) {
                self->bodyTransferPending = 1;
                self->methods->slot78(self, 1);
            }
        }
        break;
    default:
        break;
    }
}
```

(Field names updated to round 52's renames -- `ObjDA34::unk2A/unk24/unk54/
unk10/unk5C/unk5A` are now `loadState`/`flags`/`vabId`/`streamBuffer`/
`baseFilename`/`bodyTransferPending`; bytes unchanged.)

State 1 ("header pending", set by `VabStreamObj__VabStreamObj`): if the object's flag
word has bit 0x200 set, open the VAB header (`SsVabOpenHead`), build a
".VB" path from the same base filename, hand the old streaming buffer
pointer off to the shared `gPendingVabBuffer` global, transition to state 6, and
dispatch the body transfer through `methods->slot58` (null in retail's own
`gVabStreamObjMethods` -- see `VabStreamObj__VabStreamObj.md`); then free the filename copy if one
was allocated. State 6 ("body pending"): if the same flag is set, continue
the body transfer (`SsVabTransBody`), and on success mark the object ready
(`bodyTransferPending = 1`) and notify via `methods->slot78` (==
`VabStreamObj__OnBodyReady`).

`gVabBodySuffix` is retail's own `.sdata` string `".VB"`, referenced not
retyped, same as `gVabHeaderSuffix` in the sibling function.

## Result

First full-image-correct build: byte-exact.

```
VabStreamObj__AdvanceLoadState: 74/74 words match (file 0x1CEFC-0x1D024)
```

`./build-and-verify.sh`: `OK: build matches retail SLPS_015.56`.

### Proposed learning

**This is where the `unk2A` signedness bug was actually found** -- see
`VabStreamObj__VabStreamObj.md` for the full writeup. The short version: this function's
own `switch (self->unk2A)` is what surfaced the `lh`-vs-`lhu` mismatch
against a minimal reproducer, which pinned the field's real type as `u16`
rather than the `s16` the header comment had assumed before either function
was attempted. Fixing the one field byte-matched all three functions in this
cluster (`VabStreamObj__VabStreamObj`, `VabStreamObj__Finalize`, `VabStreamObj__AdvanceLoadState`) simultaneously,
since they all touch the same struct.

**The empty `case 0:` label is load-bearing, not decorative** -- without it
GCC drops the `slti`-based range guard retail has and the switch compiles
one word short. See `VabStreamObj__VabStreamObj.md`'s proposed learning for the full
mechanism.

## Naming

Renamed `func_8002C6FC` -> `VabStreamObj__AdvanceLoadState`, tier B. Confirmed as
`gVabStreamObjMethods`'s own +0x64 slot. Mechanics are concrete (a
state-machine switch driving the header-load then body-load steps of VAB
streaming), matching this unit's header-comment description of it as the
class's "per-frame poll" -- that specific cadence claim (once per frame,
rather than on some other trigger) isn't independently re-derived here
(this unit has no visibility into slot +0x64's own caller), so kept as
tier B rather than A.

## Track 4 (2026-09-26, round 87)

Renamed `VabStreamObj__Update` -> `VabStreamObj__AdvanceLoadState` with
`rename.py`. The slot is +0x064, Class6D430's `setFlag`. It is not a
per-frame poll. The CD driver calls `self->methods->setFlag(self)` when a
request completes (`src/code_179d8_s.c`: `CdDriver__LoadFile` and the
request-queue completion, each right after it ORs a `CD_FLAG_*_DONE` bit
into `flags`). The 0x200 this body tests is `CD_FLAG_LOAD_FILE_DONE`. So the
state machine moves forward once per completed file load. First the `.VH`
header, requested by the ctor through `requestLoadFile` (+0x06C). Then the
`.VB` body, requested here through `loadFile` (+0x058). The name follows
TimBlockSrc's override of the same slot, `TimBlockSrc__AdvanceLoadState`.

The report above says +0x058 and +0x06C are "null in retail". That is only
true of the static table. `SetActiveDataSource` copies the active driver's
interface slots into every table `gDataSourceClientGetters` lists
(`include/Class6D430.h`), and `GetVabStreamObjMethods` is on that list
(D_8006D430 +0x098). At run time these calls reach the driver.
