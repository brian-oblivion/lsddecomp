# FadeBox__PopPosition -- MATCH (9/9 words, first attempt)

> Renamed from `Class6E99C__PopPosition` on 2026-09-26 (tools/rename.py). Address 0x80040490.

> Renamed from `func_80040490` on 2026-09-20 (tools/rename.py). Address 0x80040490.

Unit `code_2cc8c_e`, carved round 14. `FadeBoxMethods::popPosition` (`+0x0EC`).

```c
void FadeBox__PopPosition(FadeBoxObj *self) {
    s32 t0, t1;

    t0 = self->unk90;
    t1 = self->unk94;
    self->unk50 = t0;
    self->unk54 = t1;
    __asm__("" ::: "memory");
    self->unk60 = self->unk88;
    self->unk62 = self->unk8C;
}
```

Plain 2-word + 2-halfword field copy, direction confirmed by the raw
`lw`/`sw` operand order (source registers loaded from `+0x90`/`+0x94`/
`+0x88`/`+0x8C`, stored to `+0x50`/`+0x54`/`+0x60`/`+0x62`).

**Correction: needs explicit temps and a `memory`-clobbering `__asm__("")`
barrier between the two field pairs.** An earlier version of this report
used the bare 4-line form (no temps, no barrier) and claimed 9/9 -- that
reading was taken during the stale-build window described in
`New_FadeBox.md`. Once genuinely rebuilt, the bare form scores 0/9: GCC
2.6.3 hoists BOTH independent load pairs (`unk90`/`unk94` AND `unk88`/
`unk8C`) ahead of any store, where retail interleaves each pair's load
with its own store before starting the next. The barrier is what stops
the hoist; a bare `__asm__("")` with no clobber list did NOT work when
tried first (memory clobber specifically is required). Also involved a
genuine field-width fix on `unk88`/`unk8C` (`s16` -> `s32`, see
`FadeBox__PushPosition.md`'s own report for the full writeup) that this function's
own `lhu`-into-`s16`-locals read is compatible with either way, but which
was necessary to get the SIBLING function (`FadeBox__PushPosition`, which WRITES
these fields as full words) correct.

## Naming (round 61, track 3)

**`FadeBox__PopPosition`** -- tier B. `FadeBoxMethods::popPosition`
(`+0x0EC`). Exact inverse of `FadeBox__PushPosition` (see that report's
naming note): copies `unk90`/`unk94` back into `unk50`/`unk54` and
`unk88`/`unk8C` back into `unk60`/`unk62`, with no gate of its own (the
caller is expected to know a push is outstanding). Named as the matching
"Pop" to `PushPosition`'s "Push".

## Track 4 (2026-09-26, round 87, echo)

Fields under their unified names: unkC -> parent, unk50/54 -> posX/posY,
unk60/62 -> boxW/boxH (BoxFill's), unk88/8C -> savedW/savedH, unk90/94 ->
savedPosX/savedPosY (FadeBox's). Image byte-identical.

## asm sites

Round 89 (runner delta, track 5 `asm-sites`): the barrier after
`self->posY = t1;` is **justified**, now commented at the site, and
simplified. Measured by deleting it alone: the image went red (12 bytes, same
length), `funcdiff` 3/9, and asm-differ shows `lhu a1,0x88(a0)` /
`lhu a2,0x8c(a0)` (`savedW`, `savedH`) hoisted above `sw v0,0x50(a0)` /
`sw v1,0x54(a0)` (`posX`, `posY`). Retail stores first, then loads.
Instruction order. It was spelled `__asm__("" ::: "memory")`; the bare
`__asm__("")` builds byte-identical (whole image green), so the clobber was
not what forced the order and the site now uses the bare form.

## Track 6 (2026-09-26, round 93, charlie)

Renamed with the class: `Class6E99C` is now `FadeBox`
(`python3 tools/renametype.py Class6E99C FadeBox`, table
`python3 tools/rename.py D_8006E99C gFadeBoxMethods`), tier A; the evidence
is in New_FadeBox.md's Track 6 section. The method's own name was kept: it
already says what the body does. renametype.py rewrote the old class name in
this report's earlier history too (known, pending an operator decision).

