# func_8003FB1C -- STALL (addiu-$at jump-table blocker, not attempted)

Unit `code_2cc8c_e`, carved round 14. **Not attempted.**

## Classification

```sh
grep -n 'addiu *\$at, *\$at, *%lo' asm/nonmatchings/code_2cc8c_e/func_8003FB1C.s
```

Hit, on `%lo(jtbl_80011108)`. **A `%lo(jtbl_*)` hit counts** -- a dense
`switch` is blocked exactly like an indexed global, because the fold happens in
maspsx below cc1 and maspsx cannot tell a code address from a data one. Retail
has the UNFOLDED four instructions; the pinned pipeline emits the FOLDED three.
See `docs/research/addiu-at-blocker.md`, and CLAUDE.md's screen section for why
this is the one place the grep looks like it is over-reporting when it is not.

## This function is why the carve needed a rodata split

`func_8003FB1C` owns `jtbl_80011108`, which lived in the standalone rodata slot
at `0x1908`. Carving `code_2cc8c_e` broke the link with
`undefined reference to '.L8003FB98'` -- the table's words are `.L` labels
local to this function's own `.s` file, which a standalone rodata object cannot
see.

**Two things about that are worth knowing before the next carve:**

- Attaching the slot is required even though this function stays
  `INCLUDE_ASM`. Being `INCLUDE_ASM` assembles the text; it does not embed the
  table.
- Attaching the WHOLE slot then broke `D_800111B4`/`D_800111C8`, which belong
  to `asm/psyq_memset.s` -- a different segment. With
  `migrate_rodata_to_functions`, an attached slot's symbols migrate into the
  owning unit's functions, and a symbol referenced only from elsewhere has
  nowhere to go and drops out of the link. The slot had to be SPLIT at the
  ownership boundary (`0x1908` to this unit, `0x19B4` left standalone).

No C was written and no score was measured.
