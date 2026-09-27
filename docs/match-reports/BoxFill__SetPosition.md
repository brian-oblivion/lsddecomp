# BoxFill__SetPosition — MATCHED (11/11), round 19

> Renamed from `Obj6EAC0__SetPosition` on 2026-09-25 (tools/rename.py). Address 0x800407f8.

> Renamed from `func_800407F8` on 2026-09-18 (tools/rename.py). Address 0x800407f8.

Unit: `src/code_2cc8c_f.c`. Blocker screen clean. Retail's own
callee-saved count is 0 -- not the saturated-register-file class.

**Round 19 correction: the "missing unconditional cache" framing below
(round-18 verdict) was the wrong lever, not an unfixable residue.** The
same whole-struct-assignment idiom that closed `FadeBox__PushPosition` (see that
report) closes this one too, and for the SAME reason: `self`'s own extra
unconditional cache (`addu $a2,$a0,$zero`) and `a1`'s cache into the
branch's delay slot (`addu $a3,$a1,$zero`) are not something the source
needs to ask for directly -- they fall out of GCC 2.6.3's register
allocation once the final field-copy is expressed as ONE aggregate copy
instead of two scalar assignments, exactly as `FadeBox__PushPosition` documented.

## Final body (byte-exact, 11/11, no drift)

```c
void BoxFill__SetPosition(Obj6EAC0 *self, BoxFillPos *a1) {
    if (self->unkC != 0) {
        *(BoxFillPos *)&self->unk50 = *a1;
    }
}
```

`BoxFillPos` (`include/code_2cc8c.h`) is the existing two-`s32`-record
type introduced for `FadeBox__PushPosition`'s own `a2` argument; `self->unk50`/
`unk54` are already documented there as "first/second word of a 2-word
struct copied from their own `a1` argument", so this is the same shape,
not a new one. `Obj6EAC0Methods::slotBC`'s prototype was retyped from
`void *a1` to `BoxFillPos *a1` to match (the only call site,
`BoxFill__AttachToParent`'s `self->methods->slotBC(self, a2)` in this same unit,
compiles unchanged since `a2` there is still `void *` and C89 converts
either direction implicitly).

Verified via `cmp -l build/SLPS_015.56 disk/SLPS_015.56`: with this
function's own C in place, the only mismatched bytes anywhere in the
whole image all decode (via `vram = (N-1)-0x800+0x80010000`) into
`func_8003FCFC`'s own still-open range (`0x8003fcfc`-`0x8003fd4c`), a
completely separate, already-known stall in this same unit's sibling
file -- i.e. zero drift attributable to this function.

## Round-18 history (superseded by the fix above, kept for the record)

Original 4 attempts (direct body -> 12 words; named temps -> true
bare-minimum 10 words; aliased `q`/`p` locals -> still 10, copy-propagated
away; combined -> still 10) all varied HOW the two scalar stores were
written, never whether they should be one aggregate store instead. That is
exactly `docs/MATCHING-GUIDE.md`'s "a long attempt list is not a broad
one" pattern: four attempts, one untested axis.

The round-18 report reasoned by analogy to `BoxFill__ApplyColor` (a genuinely
different function, still stalled, see that report) and declined to run
the permuter because that sibling's permuter run "made no real progress."
That comparison doesn't hold up: `BoxFill__ApplyColor`'s residue is a plain
caller-saved temp-register CHOICE with no adjacent struct-shaped field
pair to fold into an aggregate copy, so the same lever doesn't apply
there -- their surface symptoms (unconditional cache into a fresh
register) looked identical, but the underlying mechanisms differ.

### Proposed learning

Confirms and generalizes `FadeBox__PushPosition`'s own learning with a THIRD
instance in a different function/unit: **retail's "cache an argument
into a fresh unconditional register before/inside a branch" residue,
when the argument's only use is a field-pair store, is frequently a
SIDE EFFECT of a two-scalar-assignment shape rather than an intrinsic
register-identity fact.** Rewriting the final field-pair as one
aggregate (whole-struct) assignment removes the two-statement shape
that was inducing GCC to interleave separately, which coincidentally
flips the surrounding register allocation to match retail's own
caching choices for BOTH the object pointer and the source pointer --
not just the struct fields themselves. Before accepting a
"missing/extra cache" verdict on a function whose only body is a
field-pair copy, try the whole-struct-assignment idiom BEFORE reasoning
about register allocation at all.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800407F8` | `BoxFill__SetPosition` | B |

**Evidence.** Base occupant of `slotBC`: `if (self->hasChildren != 0) {
*(BoxFillPos *)&self->posX = *a1; }` -- stores an incoming 2-word pair
into `posX`/`posY` (renamed from `unk50`/`unk54`), but only when the
object has children. The derived occupant of the SAME slot
(`TextRow__SetPosition`, this unit) treats the equivalent pair as a
running CURSOR it advances per child by `childPitch`, which is the
evidence `posX`/`posY` really is a position rather than an arbitrary
2-word pair -- but nothing independently confirms which axis is X vs Y
(or that these are screen coordinates at all), hence tier B.

## Track 4 (2026-09-25, round 85, charlie)

Class 0x64 (was D_8006EAC0) is unified as BoxFill in include/BoxFill.h: Viewport__DrawNode draws a node whose class-id low byte is 0x64 with GsSortBoxFill over the GsBOXF at +0x058 (pri +0x044, `relative` +0x048, x/y +0x050/+0x054). The body now takes `BoxFill *`; zero bytes changed. Renamed from `Obj6EAC0__SetPosition`: the +0x0BC occupant, BoxFill's own `setPosition` (tier A): while attached (`parent`, +0x00C, non-NULL) it copies the pair into posX/posY, which DrawNode turns into the GsBOXF x/y.

## Track 6 (2026-09-27, round 96, echo)

The position record, `Pair32E99C` (include/BoxFill.h), is now `BoxFillPos`
(`python3 tools/renametype.py Pair32E99C BoxFillPos`), fields `a`/`b` now
`x`/`y`. Tier A: it is the argument type of this method, of
`BoxFill__AttachToParent` and `BoxFill__AttachAbsolute`, and of
`FadeBox__PushPosition`, and every one of them copies it whole into
`posX`/`posY` (Viewport__DrawNode's source for the GsBOXF x/y). The field
rename had no accessor anywhere (the build and `check-nonmatching.sh` stayed
green with no errors): every use is a whole-record copy. The fields stay
`s32` rather than becoming padding because the 4-byte alignment is what
makes those copies `lw`/`sw` pairs.

Not unified with ScreenSprite's `ScreenSpritePos` (include/ScreenSprite.h),
which has the same layout: the old banner's "gTextRowMethods's layout loops
use the same record" was already stale (TextRow's methods take
ScreenSpritePos since round 86), and the two do not mean the same thing.
A ScreenSpritePos is always a percentage of half the screen from the centre;
a BoxFillPos is that only while `relative` is set, and pixels after
`attachAbsolute` (GraphRoom's dots, class_3bb8c_t, pass
`dx * 10 - 5`-style pixel offsets). TaskCore__RefreshSlotView
(code_2cc8c.c) passes one local SlotPos to both a BoxFill and its
TextRows, so a single `ScreenPos` in SceneNode.h is a reasonable proposal
for the head; it cannot be done through `renametype.py` (the new name
already exists) and would touch ScreenSprite.h and SceneNode.h, outside
this job.

Image byte-identical.
