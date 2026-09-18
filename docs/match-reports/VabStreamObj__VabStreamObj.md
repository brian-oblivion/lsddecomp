> Renamed from `func_8002C4E0` on 2026-09-18 (tools/rename.py). Address 0x8002c4e0.

# VabStreamObj__VabStreamObj -- MATCHED 86/86 (round 43)

Unit `code_179d8_e`. Previously filed as a `gp_rel` stall (round 17, never
attempted); reopened round 42 once `--gp-symbols`/`--no-nop-mflo-mfhi`
resolved that blocker project-wide. This is the class's own
`new_class_da34` dispatch target -- `D_8006DA34`'s own slot +0x08, confirmed
against the real rodata (`asm/data/5E140.data.s`), not just inferred from the
header comment.

## Derivation

Cross-referencing `asm/data/5E140.data.s`'s `D_8006DA34` table (not just the
existing header-comment guess) against every call site in this unit pins down
the WHOLE table, not just this one slot:

| offset | target | called from |
| --- | --- | --- |
| +0x08 | `VabStreamObj__VabStreamObj` | `New_VabStreamObj` (`new_class_da34`) |
| +0x0C | `VabStreamObj__Close` | data only (a subclass's own "close" chain) |
| +0x58 | **null in retail** | `VabStreamObj__Update`'s own case-1 dispatch |
| +0x5C | `func_80026C20` (uncarved) | `VabStreamObj__LoadVagAttrs`'s own opening call |
| +0x6C | **null in retail** | `VabStreamObj__VabStreamObj`'s own final dispatch |
| +0x78 | `VabStreamObj__OnBodyReady` | already matched |
| +0x7C | `VabStreamObj__LoadVagAttrs` | already matched (as data) |
| +0x84 | `VabStreamObj__StopVoice` | already matched |
| +0x9C | `VabStreamObj__SetPitchOffset` | already matched |

The two null slots (+0x58, +0x6C) are retail's own data, not a derivation
error: `VabStreamObj__VabStreamObj` unconditionally builds a ".VH" path and dispatches
through `methods->slot6C` whenever its `arg1` (base filename) is non-NULL,
and that slot is genuinely `0x00000000` in the shipped table this class
uses. Whatever calls `VabStreamObj__VabStreamObj` with a non-NULL name in practice must
either never happen or crash; either way, the C only has to reproduce the
dispatch, not explain why retail left the target empty.

The body, once the field layout was pinned (see "Proposed learning" below):

```c
void VabStreamObj__VabStreamObj(ObjDA34 *self, char *arg1) {
    void *buf;
    char path[0x20];

    func_80026CAC()->slot08(self);
    self->methods = GetVabStreamObjMethods();
    self->unk4C = NULL;
    self->unk50 = NULL;
    self->unk54 = 0;
    self->unk56 = 0;
    self->methods->slot9C(self, 0);
    self->unk58 = 0;
    self->unk5A = 0;
    self->unk5C = NULL;
    if (D_8008A8B8 == 0) {
        func_80032368();
        D_8008A8B8 = 1;
        SsSetTableSize(func_8003A068(), 2, 1);
    }
    if (D_8008A8BC == 0) {
        D_8008A8CC = 0x3C;
        func_80032588(1);
        D_8008A8BC = 1;
    }
    D_8008A8C4++;
    if (arg1 != NULL) {
        buf = func_80017B34(strlen(arg1) + 1);
        if (buf != NULL) {
            self->unk5C = buf;
            strcpy(buf, arg1);
            func_800270C4(path, buf, NULL, D_8008A8D0);
            self->unk2A = 1;
            self->methods->slot6C(self, path);
        }
    }
}
```

`D_8008A8D0` is retail's own `.sdata` string `".VH"` -- referenced, not
retyped, per the duplicated-string-shift lesson in CLAUDE.md.

## Result

First full-image-correct build (after fixing `unk2A`'s signedness -- see
below): byte-exact.

```
VabStreamObj__VabStreamObj: 86/86 words match (file 0x1CCE0-0x1CE38)
```

`./build-and-verify.sh`: `OK: build matches retail SLPS_015.56`.

### Proposed learning

**`ObjDA34::unk2A`'s real type is `u16`, not `s16`, and the tell is which
half-word load instruction the switch statement compiles to, not the field's
"looks signed" small-state-machine usage.** A minimal reproducer of the
switch shape (case 0/1/6, `slti` boundary check) compiles a `s16` field to
`lh` under the pinned toolchain -- so when retail's own disassembly showed
`lhu` at the identical instruction, that was conclusive: the field must be
declared `u16`. Retail still emits a *signed* `slti` for the `< 2` guard even
though the load is unsigned -- GCC 2.6.3's switch lowering mixes a
zero-extending load with a signed range comparison when every case label is
non-negative, which is internally inconsistent for a value that could
actually be negative but is completely invisible in ordinary testing since
the real state machine never produces one. Anyone hitting an `lh`/`lhu`
mismatch on a switch-controlling field should suspect the field's
signedness before suspecting the switch's case structure.

**The explicit empty `case 0:` mattered for byte-exactness, not just
documentation.** Without it, GCC's switch lowering drops the `slti v0,v1,2 /
bnez v0,exit` guard entirely and jumps straight from the `case 1` equality
test to the `case 6` one -- one word short and structurally different from
retail. Adding `case 0: break;` (even though its body is empty and
behaviourally identical to `default`) is what makes GCC treat 0 as a
distinct low case needing its own range exclusion, matching retail's shape
exactly.

**`D_8006DA34`'s vtable can be walked function-by-function straight out of
`asm/data/5E140.data.s`** rather than guessed from which functions happen to
be in this unit. Two of the slots this unit's own functions dispatch through
(+0x58, +0x6C) are null in the shipped table -- worth knowing before
spending an attempt trying to explain a "why does this crash" question that
the C doesn't need to answer.
