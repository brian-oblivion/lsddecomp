# GsSetNearClip -- MATCH (4/4 words, first attempt)

> **Head, round 78: SONY CODE, re-identified under FINISHING-PLAN track 2.** This is `GsSetNearClip` (libgs/gs_101, Psy-Q 3.3/3.5/3.6), not game code. Evidence: `sdkname.py` EXACT masked 1.00 (TINY, 4 words) against libgs/gs_101; its sole store target is Sony's `GsCLIP3near` (pinned in `config/psyq-objects.ld`); position inside the run of placed libgs objects; prototype from `include/psyq/LIBGS.H`. The runner's game name `SetClipNear` in the `## Naming` section below is SUPERSEDED (a game name on Sony code is the thing track 3 forbids); the recorded mechanics are right. The function now counts as library and is outside every game queue.

> Renamed from `SetClipNear` on 2026-09-24 (tools/rename.py). Address 0x8003fb0c.

> Renamed from `CachePtr` on 2026-09-24 (tools/rename.py). Address 0x8003fb0c.

> Renamed from `func_8003FB0C` on 2026-09-24 (tools/rename.py). Address 0x8003fb0c.

> **ROUND 34 (2026-09-12), runner bravo -- UNIT MOVE, nothing else.** This
> function is still game code and still MATCHED; it simply lives in a
> different file. Seven of `code_2cc8c_e`'s functions turned out to be Sony's
> and are now linked from SDK objects, which left this one wedged between
> `o` segments -- so it has its own one-function unit, **`code_2cc8c_e0`**
> (`src/code_2cc8c_e0.c`). The body below is unchanged and still compiles
> byte-exact. `include/code_2cc8c.h` still declares it for its one caller,
> but that new file does NOT include the header, so the two are no longer
> cross-checked by the compiler and must be kept in step by hand.


Unit `code_2cc8c_e`, carved round 14.

Plain global-pointer setter: `void GsSetNearClip(void *a0) { GsCLIP3near = a0;
}`. `GsCLIP3near` is otherwise unreferenced anywhere else decompiled so far;
declared `void *` since nothing dereferences it here.

> **ROUND 78 (2026-09-24), runner charlie -- track 3 naming pass.** Renamed
> `func_8003FB0C` -> `CachePtr` -> `GsSetNearClip`. The global went
> `D_800902E4` -> `GsCLIP3near`, and it was NOT a free choice: `tools/rename.py`
> refused any other spelling on the first try, because `0x800902e4` is
> Sony's own `GsCLIP3near`, pinned in `config/psyq-objects.ld` by fourteen
> `libgs` objects (`gs_103`, `gs_104`, `gs_105`, `gs_107`, `gs_108`,
> `gs_110`, `gs_111`, `gs_113`, `gs_119` through `gs_123`, `gs_127`) -- the
> sibling of `GsCLIP3far` (0x8008E9D8, same object set, same ld file). Per
> CLAUDE.md ("never rename a Sony symbol, and give no game name to anything
> Sony owns"), the only rename the tool allowed was to Sony's own name. This
> FUNCTION is still game code -- round 34 already established that; only
> the other function that used to share its old unit, `func_8003FB1C` /
> `Gssub_make_matrix`, turned out to be Sony's -- it is one line of game C
> that happens to write a value into a Sony-owned bss global.
>
> This turns the function's name from a guess into tier-A evidence: the
> body is exactly and only `GsCLIP3near = a0;`, so "sets Sony's GS
> near-clip-plane pointer" is not inferred from the one call site, it is
> what Sony's own name for the write target says the write target IS.
> `include/code_2cc8c.h`'s prototype and `src/code_2cc8c_d.c`'s one call
> site (`GsSetNearClip(self->unk4C);`, inside `Unk18Obj__Update`) were updated
> by `rename.py` tree-wide, automatically. No `Unk18Obj` field was touched
> or proposed by this pass: `self->unk4C` belongs to `code_2cc8c_d.c`'s own
> unit (PARALLEL-RUNS.md collision rule 1), and the value it holds is a
> plain `s32` reused three lines later in `Unk18Obj__Update` in an ordinary
> size/count calculation (`(self->unk50 - self->unk4C) / (1 <<
> self->unk3C) + 1`) -- consistent with "a clip-near bound", but that
> reading rests on the CALLER's arithmetic, not on anything this function's
> own body shows, so it is left as a note here rather than a rename
> proposal.

## Naming

| name | tier | evidence |
| --- | --- | --- |
| `GsSetNearClip` | A | body is exactly `GsCLIP3near = a0;`; the write target is Sony's own symbol name (`config/psyq-objects.ld`, pinned by 14 `libgs` objects), so the name states what the function does, not a guess at why. |
| `GsCLIP3near` (global) | Sony's -- not a game name | `tools/rename.py D_800902E4 GsCLIP3near` refused any other spelling: the address is Sony's bss symbol, linked from `libgs/gs_103` through `gs_123`/`gs_127`. |
