# GsSetWorkBase -- MATCH (4/4 words, first attempt)

> **Head, round 78: SONY CODE, re-identified under FINISHING-PLAN track 2.** This is `GsSetWorkBase` (libgs/gs_124, Psy-Q 3.3/3.5/3.6), not game code. Evidence: `sdkname.py` EXACT masked 1.00 (TINY, 4 words) against libgs/gs_124; its sole store target is Sony's `GsOUT_PACKET_P` (pinned in `config/psyq-objects.ld`); position inside the run of placed libgs objects; prototype from `include/psyq/libgs.h`. The runner's game name `SetPacketBufCursor` in the `## Naming` section below is SUPERSEDED (a game name on Sony code is the thing track 3 forbids); the recorded mechanics are right. The function now counts as library and is outside every game queue.

> Renamed from `SetPacketBufCursor` on 2026-09-24 (tools/rename.py). Address 0x8003fbe4.

> Renamed from `func_8003FBE4` on 2026-09-24 (tools/rename.py). Address 0x8003fbe4.

> **ROUND 34 (2026-09-12), runner bravo -- UNIT MOVE, nothing else.** This
> function is still game code and still MATCHED; it simply lives in a
> different file. Seven of `screen_widgets`'s functions turned out to be Sony's
> and are now linked from SDK objects, which left this one wedged between
> `o` segments -- so it has its own one-function unit, **`libgs_gs_124`**
> (`src/psyq/libgs_gs_124.c`). The body below is unchanged and still compiles
> byte-exact. `include/task.h` still declares it for its one caller,
> but that new file does NOT include the header, so the two are no longer
> cross-checked by the compiler and must be kept in step by hand.


Unit `screen_widgets`, carved round 14.

Plain global-pointer setter, same shape as `func_8003FB0C`:
`void GsSetWorkBase(void *a0) { GsOUT_PACKET_P = a0; }`.

## Naming

**`GsSetWorkBase`, tier A.** The whole body is the store
`GsOUT_PACKET_P = a0`, and a setter's mechanics ARE its purpose (CLAUDE.md
naming rule: "a pure leaf whose mechanics ARE its purpose ... is tier A by
definition"). `GsOUT_PACKET_P` is the target -- Sony's own name, recovered by
`tools/rename.py` when it refused a game name for it (pinned in
`config/psyq-objects.ld`) -- and `SortTmdObject` (`src/graphics/TmdRenderer.c`)
confirms the mechanics from the reader side: its own comment calls this same
global "the packet-buffer write cursor, reloaded ... at the top of every
group and advanced by each submit wrapper's return value"
(`prim = (u8 *)GsOUT_PACKET_P; ...; GsOUT_PACKET_P = prim;`). The one caller,
`Viewport__Update`, passes `self->unk88[idx]` (a per-OT-slot work-buffer base)
right before calling `GsClearOt` on the same slot -- i.e. this resets the
cursor to the start of that slot's packet buffer for the frame about to
render into it, which is consistent with, and does not exceed, what the body
shows.

`GsOUT_PACKET_P` is Sony's global (pinned; not renamed -- CLAUDE.md/FINISHING-PLAN
track 3, "do not rename a Sony symbol"). Referenced from two units
(`libgs_gs_124.c` here, `TmdRenderer.c`), so it is a genuine cross-unit
global, not unit-static -- moot here since it already carries its Sony name.

### Sibling note

`func_8003FB0C` (`src/psyq/libgs_gs_101.c`, charlie's unit this round) is the
identical shape one function earlier in the same original segment
(`D_800902E4 = a0`), called from the same caller (`Viewport__Update`) two
lines above this one. `D_800902E4` is NOT referenced outside
`libgs_gs_101.c` (checked: `grep -rn D_800902E4 src/ include/`), so unlike
`GsOUT_PACKET_P` it does not carry a Sony pin as far as this unit can see --
that's charlie's call to make with `rename.py`, which will say either way.
Posted to the broadcast for parallel naming.

## File history (moved from the unit banner, round 90)

- Round 34 (2026-09-12): the functions on both sides became linked Sony
  objects (gs_123 `Gssub_make_matrix` in front, gs_111 `GsDrawOt` behind), so
  this function got its own one-function unit rather than putting an `o`
  segment inside a `c` one.
- Round 78 (head, track 2): identified as Sony's `GsSetWorkBase`
  (libgs/gs_124): `sdkname.py` EXACT (TINY, 4 words) on discs 3.3/3.5/3.6;
  its one store target 0x8008E794 is `GsOUT_PACKET_P` (pinned in
  `config/psyq-objects.ld`; LIBGS.H: "Work Base pointer"). `progress.py`
  counts it as library through the `identified` comment on its symbols entry.
- Round 90 (track 8): `tools/unitfile.py rename code_2cc8c_e1 libgs_gs_124`.
  `tuboundary.py --unit code_2cc8c_e1`: "(after sony:libgs/gs_123): start edge
  possible"; both neighbours are placed objects, so no merge was possible.

## History (moved from src/libgs_gs_124.c, comments pass)

The file's banner carried its edge evidence:

> Edges: both are placed Sony objects (libgs/gs_123 before, libgs/gs_111
> after), so this file cannot merge with a neighbour; its extent is the
> gs_124 slot of the archive's link order.
