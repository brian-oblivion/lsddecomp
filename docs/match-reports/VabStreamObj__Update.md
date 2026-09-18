> Renamed from `func_8002C6FC` on 2026-09-18 (tools/rename.py). Address 0x8002c6fc.

# VabStreamObj__Update -- MATCHED 74/74 (round 43)

Unit `code_179d8_e`. Previously filed as a `gp_rel` stall (round 17, never
attempted); reopened round 42 once `--gp-symbols`/`--no-nop-mflo-mfhi`
resolved that blocker project-wide. Confirmed against `gVabStreamObjMethods`'s own
rodata as that table's `+0x64` slot -- the class's own per-frame poll/update
method: it switches on the object's own load state (`self->unk2A`) and drives
the VAB header/body streaming state machine.

## Derivation

```c
void VabStreamObj__Update(ObjDA34 *self) {
    char path[0x20];

    switch (self->unk2A) {
    case 0:
        break;
    case 1:
        if (self->unk24 & 0x200) {
            self->unk54 = SsVabOpenHead(self->unk10, -1);
            func_800270C4(path, self->unk5C, NULL, gVabBodySuffix);
            gPendingVabBuffer = self->unk10;
            self->unk2A = 6;
            self->unk10 = NULL;
            self->methods->slot58(self, path);
            if (self->unk5C != NULL) {
                func_80017CFC(self->unk5C);
                self->unk5C = NULL;
            }
        }
        break;
    case 6:
        if (self->unk24 & 0x200) {
            self->unk54 = SsVabTransBody(self->unk10, self->unk54);
            if (self->unk54 != -1) {
                self->unk5A = 1;
                self->methods->slot78(self, 1);
            }
        }
        break;
    default:
        break;
    }
}
```

State 1 ("header pending", set by `VabStreamObj__VabStreamObj`): if the object's flag
word has bit 0x200 set, open the VAB header (`SsVabOpenHead`), build a
".VB" path from the same base filename, hand the old streaming buffer
pointer off to the shared `gPendingVabBuffer` global, transition to state 6, and
dispatch the body transfer through `methods->slot58` (null in retail's own
`gVabStreamObjMethods` -- see `VabStreamObj__VabStreamObj.md`); then free the filename copy if one
was allocated. State 6 ("body pending"): if the same flag is set, continue
the body transfer (`SsVabTransBody`), and on success mark the object ready
(`unk5A = 1`) and notify via `methods->slot78` (== `VabStreamObj__OnBodyReady`).

`gVabBodySuffix` is retail's own `.sdata` string `".VB"`, referenced not
retyped, same as `gVabHeaderSuffix` in the sibling function.

## Result

First full-image-correct build: byte-exact.

```
VabStreamObj__Update: 74/74 words match (file 0x1CEFC-0x1D024)
```

`./build-and-verify.sh`: `OK: build matches retail SLPS_015.56`.

### Proposed learning

**This is where the `unk2A` signedness bug was actually found** -- see
`VabStreamObj__VabStreamObj.md` for the full writeup. The short version: this function's
own `switch (self->unk2A)` is what surfaced the `lh`-vs-`lhu` mismatch
against a minimal reproducer, which pinned the field's real type as `u16`
rather than the `s16` the header comment had assumed before either function
was attempted. Fixing the one field byte-matched all three functions in this
cluster (`VabStreamObj__VabStreamObj`, `VabStreamObj__Close`, `VabStreamObj__Update`) simultaneously,
since they all touch the same struct.

**The empty `case 0:` label is load-bearing, not decorative** -- without it
GCC drops the `slti`-based range guard retail has and the switch compiles
one word short. See `VabStreamObj__VabStreamObj.md`'s proposed learning for the full
mechanism.
