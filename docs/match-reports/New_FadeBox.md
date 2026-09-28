# New_FadeBox -- MATCHED (31/31 words)

> Renamed from `New_Class6E99C` on 2026-09-26 (tools/rename.py). Address 0x8003fdb0.

> Renamed from `func_8003FDB0` on 2026-09-20 (tools/rename.py). Address 0x8003fdb0.

Unit `ScreenWidgets`, carved round 14.

> **UPDATE (targeted permuter pass, round 17).** MATCHED, no permuter
> needed. `docs/research/epilogue-merge-residue.md` (updated 2026-09-02,
> after this function's stall was filed) documents the real discriminator
> for this whole residue class: **`return NULL;` must come textually LAST,
> after the success return**, not as an early guard clause. This function's
> preserved body used a single merged `if (self != NULL) { ctor(...); }
> return self;` -- neither family the research doc describes. Splitting
> into two explicit returns with `self` first and `NULL` last closed it on
> the first try:
>
> ```c
> FadeBoxObj *New_FadeBox(void *a1, s32 a2, s32 a3) {
>     FadeBoxObj *self;
>
>     self = BMemPMgrAlloc(0xA0);
>     if (self != NULL) {
>         GetFadeBoxMethods()->ctor(self, a1, a2, a3);
>         return self;
>     }
>     return NULL;
> }
> ```
>
> Verified: `./build-and-verify.sh` exit 0 (full-image SHA1 match),
> `funcdiff.py` 31/31.

## Shape

`New_FadeBox`-shaped allocator, the checked-return-regardless variant.

## Residue (closed)

One word: on the allocation-failure path, retail materialises the return
value as `addu $v0, $zero, $zero` (a fresh zero); the earlier single-merged
`return self;` form produced `move $v0, $s0` instead (also zero-valued on
that path, since `self` is `NULL` there, but a different instruction). This
was the project's documented "New_X epilogue-merge residue" class -- see
`docs/research/epilogue-merge-residue.md` for the full discriminator
(textual order of the two `return`s, not "does GCC merge exits with
different values" as an earlier draft of that research doc claimed).

## Attempts (3, beyond the merged-return shape, all pre-dating the fix)

1. `if (self == NULL) { return NULL; }` early return instead of a merged
   `if (self != NULL) { ctor(...); }`: worse at the time -- changed the
   branch offset (+1 word) and produced `sw ...; sw ...` in a different
   order. (This is the "`return NULL;` FIRST" family the research doc later
   confirmed always fails -- consistent with what closed it being the
   OPPOSITE order.)
2. `if (self == NULL) { return self; }`: same regression as attempt 1.
3. Confirmed via `tools/asm-differ/diff.py` that the one-word residue was
   clean and isolated, not masked by anything else.

## What actually happened (kept for the record -- a real process lesson)

This function's retype (`SubHandleObj *` -> `FadeBoxObj *`) was
committed alongside a header change that used `ClassEAC0Obj` at a line
position ABOVE that type's own forward declaration -- a parse error in
`Task.c` (a DIFFERENT unit) that made `make` skip the final link
entirely, leaving `build/SLPS_015.56` stale. `funcdiff.py`'s own
STALE-BUILD guard did not catch it because the STALE object here was
`Task.c.o`, not `ScreenWidgets.c.o` -- this unit's own object kept
rebuilding fine, so nothing about the guard's usual signal (source newer
than binary) looked wrong; the binary just never reached the final link
step at all. Caught only by cross-checking EVERY function's LINKED
ADDRESS against `build/lsdde.map` after fixing the forward-declare bug.

### Proposed learning

**Checking a build's EXIT STATUS is not the same as checking that the
CORRECT files got compiled.** A parse error in a unit you do not own can
leave your own object rebuilding successfully every time while the FINAL
LINK silently never runs, so `build/SLPS_015.56` stays pinned at whatever
it was before your session started. The targeted check that catches it:
after ANY multi-function commit, grep `build/lsdde.map` for every touched
symbol's address and diff it against the expected retail address -- a
mismatch there means SOMETHING upstream is wrong even when `build exit=`
and funcdiff's own guards say nothing.

**Superseded by this round's fix:** the class is not "unfixable, retail
merges what GCC won't" -- it is a textual-order-of-returns sensitivity, and
once the research doc identified that, this function (and its sibling
`New_BoxFill`) matched on the very first correctly-shaped attempt.

## Naming (round 61, track 3)

**`New_FadeBox`** -- tier A. Standard `New_X`-shaped allocator: allocates
a fixed 0xA0 bytes (`FadeBoxObj`'s own size) and, on success, dispatches
its ctor through `GetFadeBoxMethods()->ctor(...)` before returning it;
matches the project's established `New_X` convention (`New_Entity`,
`New_DreamSys`, `New_TextRow`, etc.) exactly. Mechanics fully determine the
name; no game-purpose claim beyond "allocate and construct one".

## Track 6 (2026-09-26, round 93, charlie)

The class `Class6E99C` (table `D_8006E99C`, id 0x164) is now `FadeBox`
(`python3 tools/renametype.py Class6E99C FadeBox`, then
`python3 tools/rename.py D_8006E99C gFadeBoxMethods`), tier A: the body
alone shows it -- Update adds a step to the selected bytes of BoxFill's
colour once per call until a tick count runs out, StartFadeDown/StartFadeUp
set the start colour and the step's sign, and configure turns on additive
(a mask) or subtractive (mask 0) semi-transparency; its two users agree
(Viewport's full-screen 320x240 subHandle, which ObjM fades and whose
5/6 "fade down/up done" it handles, and Entity's on-demand `unk100`, which
the MoodCue handlers fade). The name claims a fading box and no more: which
game transitions use it is the callers' business. The header moved to
`include/FadeBox.h`, the family's types followed (`FadeBoxMethods`,
`FadeBoxResetFn`), and the own field `unk84` became `maskPerTick`
(configure's `BoxFill::mask / ticksLeft`; written, never read). renametype.py
also rewrote the old class name inside earlier sections' history prose in
this and sibling reports, including the retired `Class6E99CObj` view and
the round-20 working names (`FadeBoxObj`, `FadeBox__GetMethods`, ...): those
lines name types and functions that were then `Class6E99C...` (known,
pending an operator decision; not hand-reverted).

### The unit banner and function comments, moved from src/ui/ScreenWidgets.c

The banner now says what the file holds. Its history, and the long form of
three comments now reduced to one `MATCHING:` line each, verbatim (with
renametype's rewrite of the class name):

```
/* ROUND 34: THIS UNIT LOST ITS FIRST EIGHT FUNCTIONS -- six of them to Sony,
 * two to files of their own -- and now begins at 0x305B0 / New_FadeBox.
 * The segment it used to be is split three ways:
 *
 *   [c libgs_gs_101]   GsSetNearClip          Sony libgs/gs_101, C file
 *   [o libgs/gs_123]   Gssub_make_matrix      was func_8003FB1C, matched C
 *   [c libgs_gs_124]   GsSetWorkBase          Sony libgs/gs_124, C file
 *   [o libgs/gs_111]   GsDrawOt               was func_8003FBF4, matched C
 *   [o libgs/gs_113]   GsClearOt              was func_8003FC18, matched C
 *   [o libgs/gs_108]   GsSetLightMode         was func_8003FC70, matched C
 *   [o libgte/fgo_00]  TransposeMatrix        was func_8003FCFC, a 20w stall
 *   [o libgte/fog_01]  SetFogNear             was func_8003FD4C, matched C
 *   [c ScreenWidgets]   New_FadeBox onward   <- this file
 *
 * THIS FILE KEEPS THE NAME deliberately: it holds the unit's remaining
 * INCLUDE_ASM stubs and its class, so every
 * `INCLUDE_ASM("asm/nonmatchings/ScreenWidgets", ...)` path below and every
 * match report naming this unit stays valid. Only the two one-function heads
 * needed new names.
 *
 * NEITHER RODATA SLOT IS OURS ANY MORE. jtbl_80011108 (0x1908) went with
 * Gssub_make_matrix and D_80011194 (0x1994, "not supported light mode %d\n")
 * went with GsSetLightMode -- both are their own object's `.rdata` section
 * now. This unit needs no `.rodata` attach at all; if a future carve of it
 * hits Gate 2's `undefined reference to '.LXXXXXXXX'`, that is a NEW jump
 * table, not these.
 *
 * The six Sony bodies are gone from this file, not lost -- five matched C
 * bodies and one INCLUDE_ASM stub. Each one's match report is kept and
 * retitled CONVERTED, and carries its derivation verbatim.
 */

/*
 * WHAT THIS UNIT IS (round 61, track 3; revised rounds 85 and 87, track 4).
 * Its 17 functions are the bottom two links of `SceneNode -> BoxFill ->
 * FadeBox`: first FadeBox's (gFadeBoxMethods, 0x164, `New_FadeBox` to
 * `GetFadeBoxMethods`, include/FadeBox.h), then BoxFill's allocator,
 * ctor and Reset (0x64, include/BoxFill.h, a GsBOXF screen rectangle; the
 * rest of its methods open ScreenWidgets).
 *
 * FadeBox fades the box's colour: configure picks the channels (a
 * 4/2/1 = r/g/b mask) and a tick count, StartFadeDown/StartFadeUp set the
 * start colour and the step's sign, Update steps the selected channels once
 * per call until Stop, and PushPosition/PopPosition save and restore the
 * box's size and position (tier B; include/FadeBox.h's banner has the
 * evidence). See each function's own `## Naming` section.
 */
```

```
/* Both StartFade functions forward their own three arguments to configure
 * untouched (no argument register is set before that jalr), and spelling
 * the forward is load-bearing: a `(self)`-only call compiles to the same
 * instructions in a different order (29/35; round 73). */
/* Both s32 pairs are copied as whole structs. GCC 2.6.3's MIPS
 * `movstrsi_internal` clobbers $v0/$v1/$a0/$a1, so `self` and `size`, live
 * across the first copy, cannot stay in their incoming registers: that is
 * retail's entry `move $a3,$a0` / delay-slot `move $t0,$a1` (round 73). */
    /* Keeps the savedW/savedH loads below the posX/posY stores; without it
     * GCC hoists both lhu above the two sw. (A "memory" clobber is not needed.) */
```

## Proposed field names

Outside this job's edit set (the head applies them by type scope):

- `Entity::unk100` -> `fadeBox` (include/entity.h): a `FadeBox *` made by
  Entity__GetOrCreateFadeBox and faded by the MoodCue handlers; with it,
  `Entity__GetOrCreateUnk100` -> `Entity__GetOrCreateFadeBox` (rename.py;
  the body creates the FadeBox on first call, reattaches it and sets its
  step).
- Viewport's `subHandle` (+0x0B0, include/Viewport.h) -> `fadeBox`, and its
  getter slot `getSubHandle` -> `getFadeBox`: the ctor fills it with
  New_FadeBox, and both ObjM callers cast the result to `FadeBox *`.
  Viewport.h's banner still lists it as "not settled"; its type could
  become `FadeBox *` once Viewport.h can include FadeBox.h.
- `FadeBox::unk7C` stays: configure stores its third argument there and
  update skips stepping while it is 9, but every caller passes 0, so what
  9 means is not shown.
- `BoxFillPos` (include/BoxFill.h) is named after this class's old
  address; it is BoxFill's position record (setPosition, attachToParent,
  TextRow's layout) and belongs to BoxFill.h's job, not this one.


## History (moved from src/ScreenWidgets.c, comments pass)

The file's banner carried its edge evidence and the reason it is parked:

> What decided its edges (python3 tools/tuboundary.py): the placed object
> libgte/fog_01 precedes it ("start edge possible") and psyq_memset follows
> it; every edge inside is "boundary possible". The forced boundary the tool
> reports between jump tables 0x80011108 (Task.c) and 0x800111dc (Sprite.c)
> is met by those two object edges. Content decided: BoxFill's methods
> straddled the old carve edge between code_2cc8c_e and code_2cc8c_f
> (allocator, ctor and Reset before it), so the two were merged. PARKED: the
> content would put a file boundary before New_TextRow (and perhaps before
> the Shift-JIS helpers); a split is a new carve, so the file keeps them and
> is named for all it holds.
