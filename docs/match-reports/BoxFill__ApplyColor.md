# BoxFill__ApplyColor — MATCHED (26/26), round 19

> Renamed from `Obj6EAC0__ApplyColor` on 2026-09-25 (tools/rename.py). Address 0x80040790.

> Renamed from `func_80040790` on 2026-09-18 (tools/rename.py). Address 0x80040790.

Unit: `src/ui/screen_widgets.c`. Blocker screen clean (no `gp_rel`, no
`addiu $at,$at,%lo`, no jump table).

## Round 19: closed with the whole-struct-assignment axis

Same lever that closed `TaskCore__SetColors` (this round) and `FadeBox__PushPosition`/
`BoxFill__SetPosition` (round 18/19): the "copy arm" of this function was a
3-byte scalar-by-scalar assignment (`d[0]=r; d[1]=g; d[2]=b;`), and
rewriting it as one whole-struct assignment through a local 3-byte
struct type removed the residue -- both the redundant-move class
described below AND the register choice for the loads.

```c
typedef struct { s8 r, g, b; } RGB80040790;

void BoxFill__ApplyColor(Obj6EAC0 *self, u8 *dst, u8 *src, s32 overwrite) {
    u8 *d;
    d = dst;
    if (overwrite) {
        *(RGB80040790 *)d = *(RGB80040790 *)src;
    } else {
        d[0] += src[0];
        d[1] += src[1];
        d[2] += src[2];
    }
}
```

Full oracle confirms: `build-and-verify.sh` exits 0, `OK: build matches
retail SLPS_015.56` -- genuinely whole-image byte-exact, not just this
function's own 26/26 window.

The `else` (accumulate) arm needed no change at all from the original
report's body -- it is not a copy of contiguous bytes (each byte is
independently read-modify-written), so there is no aggregate to fold it
into, and it was never part of the residue.

## Round-2 history (superseded, kept for the record)

Original 7 attempts established: correct length needs named `r,g,b`
temps in the copy arm (attempt 1, kept); the residue was `dst` failing to
get cached into a second register (`$t0`) the way retail's first branch's
delay slot does, and every "keep the pointer in two names" trick (plain
alias, alias+barrier, two-statement alias, "mention twice", `volatile`)
either did nothing (copy-propagated away) or actively regressed
(`volatile` added a real stack slot). A permuter run was done but against
a length-WRONG seed, so its 845-base score was never trustworthy evidence
either way.

None of those 7 attempts touched the axis that closed it: replacing the
scalar-by-scalar copy-arm assignment with one whole-struct assignment.
This is the SAME shape as `TaskCore__SetColors`'s stall (a `sX[0]=aY[0];
sX[1]=aY[1]; sX[2]=aY[2];` byte triple) and the same fix applies.

### Proposed learning

A fourth confirmation of the whole-struct-assignment lever (after
`FadeBox__PushPosition`, `BoxFill__SetPosition`, `TaskCore__SetColors`), and the first
instance paired with a "redundant move" framing rather than an
insertion/deletion framing: **the "retail caches an argument into an
extra unconditional register" residue and the "scalar-vs-aggregate
field copy" residue are, empirically, very often the SAME defect wearing
different funcdiff signatures** (missing instruction here, wrong
register there, wrong opcode in `TaskCore__SetColors`). Whenever a stalled
function's body contains ANY multi-field/multi-byte copy written as
consecutive scalar assignments, try folding it into one aggregate
assignment before spending further attempts on the register/scheduling
symptom directly -- it has now closed 4 different apparent residue
shapes in this same unit's neighborhood alone.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040790` | `BoxFill__ApplyColor` | A |

**Evidence.** Mechanics are the whole purpose (tier A): given `overwrite`,
either copies a 3-byte RGB buffer wholesale (`*(RGB *)d = *(RGB *)src`) or
adds each of the 3 bytes into the destination in place -- a "set or blend
a colour" operation, with `dst` always `self->color` at its one call site
(`BoxFill__SetColor`, this unit).

## Track 4 (2026-09-25, round 85, charlie)

Class 0x64 (was D_8006EAC0) is unified as BoxFill in include/box_fill.h: Viewport__DrawNode draws a node whose class-id low byte is 0x64 with GsSortBoxFill over the GsBOXF at +0x058 (pri +0x044, `relative` +0x048, x/y +0x050/+0x054). The body now takes `BoxFill *`; zero bytes changed. Renamed from `Obj6EAC0__ApplyColor`: BoxFill__SetColor's only callee, not a slot (tier A: copy or add three bytes).

## Track 6 (2026-09-27, round 98, delta): `RGB80040790` -> `BoxFillRgb`

- **`BoxFillRgb`** (tier A, include/box_fill.h): the local copy type, named for this function's address, is BoxFill's own colour record by use. `src` is setColor's `rgb` argument (BoxFill__SetColor forwards it with `dst = self->color`, the GsBOXF r, g, b at +0x064), and the same three bytes arrive through the ctor/Reset colour argument (New_BoxFill). GraphRoom's dot colours (dream_scene, round 97's `GraphPointColor`) are those arguments too, so the one type is defined once in box_fill.h and both local views are retired onto it. Same shape and convention as `BgLayerRgb`/`ViewportRgb`: signed, three bytes, whole-struct copy = three lb/sb pairs.
- Byte-identical: whole image green, 0 new typeviews warnings, nonmatching green.
- **Proposed, not applied:** `BOXFILL_FIELDS`' `u8 color[3]` -> `BoxFillRgb color`, setColor's `void *rgb` and New_BoxFill/Reset's `void *color` -> `BoxFillRgb *`, and this function's `u8 *dst, u8 *src` -> `BoxFillRgb *`. The accumulate arm reads `src[0..2]` as bytes (`lbu` + add), so retyping `src` needs the arm rewritten through `.r/.g/.b` with an s8/u8 load check; callers outside this unit (TaskCore's listView, dream_scene/_n, FadeBox, task) pass their own buffer types and would each need a cast. Not measured this round.

## Track 7 (round 99, bravo)

The `d = dst` copy is gone; the body writes through `dst`. Byte-exact (26/26).

## Track 10 (2026-09-28, round 104, echo)

The six per-class aliases of `ColorRgb` (include/draw_system.h) -- BgLayerRgb, BoxFillRgb, FlatLightColor, LightRigRgb, ViewportRgb, TimBlockSrcColor -- are deleted and every use is spelled `ColorRgb`. Byte-identical. Measured for the MATCHING line in TaskCore__SetColors: the whole-struct copy is three `lb` then three `sb`, and rewriting one of the copies byte by byte loads each byte with `lbu` and interleaves the stores (asm-differ on the experiment), so the struct copy stays; the old line's "signed bytes" was wrong (ColorRgb's channels are u8; the lb comes from the block copy, not the type), and the same claim in GraphRoom__BuildGraphPoints' colour comment is corrected.

## History (source comments moved in track 12, round 106)

From `include/box_fill.h`:

> On the box's colour as a ColorRgb: "A whole-struct copy is three lb/sb pairs
> (BoxFill__ApplyColor, GraphRoom__BuildGraphPoints). The field and the slot
> parameter stay `u8 color[3]` and `void *rgb`."
