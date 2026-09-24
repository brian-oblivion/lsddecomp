# Class86B60__OnTagBValue -- MATCH

> Renamed from `func_8004E230` on 2026-09-24 (tools/rename.py). Address 0x8004e230.

Unit `class_3bb8c_d`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py Class86B60__OnTagBValue`: 40/40 words match.

## Source

```c
void Class86B60__OnTagBValue(Class86B60 *self, s32 arg1, s32 value)
{
    if (value < 0x18) {
        if (value >= 0x16) {
            self->methods->slot12C(self);
            if (value == 0x16) {
                self->unkA4->methods->slot1A8(self->unkA4);
                self->methods->slot124(self, 0x16);
            }
        }
    }
}
```

`arg1` (the function's own 2nd parameter, `$a1`) is never read anywhere in
the body -- confirmed genuinely unused, not a derivation gap, per the
already-documented "an unused parameter in the callee shows up as a
genuinely uninitialized local in the caller" idiom (the mirror case: an
unused parameter in the CALLEE just needs declaring, not using).

## One near-miss: `&&` versus nested `if`

**First attempt (8/40, and everything past the guard shifted):** wrote the
two range checks as one combined `if (value < 0x18 && value >= 0x16)`.
GCC 2.6.3 collapsed this into a shorter instruction sequence than retail's
two separate `slti`/branch pairs -- the function came out the WRONG
LENGTH, which is why the diff showed drift outside the range and a
funcdiff warning rather than a clean few-word residue. Retail's actual
shape is two literal, separately-compiled range tests (`value < 0x18`,
then nested, `value >= 0x16`), which is what a straight source-order
transcription with nested `if`s (rather than a boolean `&&`) reproduces.
Rewriting as `if (value < 0x18) { if (value >= 0x16) { ... } }` matched on
the very next build with no other change.

## Struct changes (additive, `include/class_3bb8c.h`)

- `Class86B60Methods::slot12C` -- new slot, `void (*)(Class86B60 *self)`.
- `DreamSysViewMethods_3bb8c_c::slot1A8` -- new slot (declared ahead of
  this function while deriving `Class86B60__ShowTitleIcon`'s neighbourhood; this is
  the function that exercises it), `void (*)(DreamSysView_3bb8c_c *self)`.
- Fixed a self-inflicted duplicate-member bug introduced while adding
  `slot12C`: an earlier edit accidentally left two identical `slot130`
  declarations in `Class86B60Methods`, caught immediately by the compile
  error (`duplicate member 'slot130'`) before any score was read.

### Proposed learning

**A combined `&&`/`||` boolean test and the equivalent nested `if` are
NOT interchangeable at `-O2` even when both are logically identical and
the "one test, then a second nested test" shape is what retail has.**
Here the combined form actually compiled SHORTER than the nested one,
which is the opposite direction from most of this project's documented
"simplifying away a guard costs instructions" entries -- worth keeping as
its own data point rather than merged into that list, since the direction
of the effect is reversed. The reliable move once a `&&`/`||` residue
shows outside-range drift (not just a few different words) is to try the
nested-`if` spelling before anything else.

## Naming (round 77, naming runner delta)

Renamed `func_8004E230` -> `Class86B60__OnTagBValue`. **Tier B**: The exclusive dispatch target of `Class86B60__ForwardIfTagB`'s `slot138` forward (called only when `arg1`'s header nibble == 0xB), gating further work on ranges of its own `value` parameter (>= 0x16, < 0x18, == 0x16). Named for its role as the tag-0xB handler; the value ranges' meaning is not established.
