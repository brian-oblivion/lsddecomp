# RegisterRecordTableFiles — MATCHED (round 45, 48/48 words)

> Renamed from `func_8004A070` on 2026-09-27 (tools/rename.py). Address 0x8004a070.

**Unit:** dream_day · **Size:** 48 words (0xC0 bytes)

Filed as a `gp_rel`-blocked stub before round 15. That blocker was RESOLVED
in round 42 (`--gp-symbols`, pinned in the Makefile).

## Derivation

```c
/* Sony's, from the still-uncarved psyq_39094 SDK segment
 * (asm/psyq_39094.s): `if (out != NULL) *out = 0x230; return &sRecordTable;`
 * -- an unconditional out-param write (the address passed here is always a
 * stack address, never NULL) plus a fixed .data address, unrelated to the
 * write. Declared locally per CLAUDE.md's rule against writing C for
 * SDK-owned code. */
extern void *GetRecordTable(s32 *out);
extern s32 RegisterFileTableEntries(void *arg0, s32 arg1);

extern s32 sRecordRegisterCalls;
extern s32 sRecordFirstBatchCount;

s32 RegisterRecordTableFiles(s32 arg0)
{
    s32 local;
    void *obj;
    s32 prev;
    s32 result;

    obj = GetRecordTable(&local);
    prev = sRecordRegisterCalls;
    sRecordRegisterCalls = prev + 1;

    switch (prev + 1) {
    case 1:
        if (arg0 != 0) {
            sRecordRegisterCalls = prev + 2;
        } else {
            local = local / 2;
            sRecordFirstBatchCount = local;
        }
        break;
    case 2:
        local = local - sRecordFirstBatchCount;
        break;
    default:
        local = 0;
        break;
    }

    while ((result = RegisterFileTableEntries(obj, local)) == 0) {
    }
    return result;
}
```

`GetRecordTable` is Sony's (still uncarved, `asm/psyq_39094.s`, the
`libetc`-style SDK block) — never given a C definition here, only an
`extern` declaration, per the project's rule against writing C for
SDK-owned functions.

The return type had to be `s32`, not `void`: `include/dream_day.h`
already carries `extern s32 RegisterRecordTableFiles(s32 arg1);` (this unit's own
prior local view, noting "already declared elsewhere as `extern s32
RegisterRecordTableFiles(s32 a0)`"), and the definition here must match it exactly or
`cc1` rejects it as `conflicting types`. Retail's own asm supports this: at
`jr $ra` the last thing written to `$v0` is the final (non-zero) return
value of the tail `RegisterFileTableEntries` call — genuinely live register content,
not leftover noise — so the C spells it out explicitly with a `result`
local rather than relying on it falling out of the loop by accident.

**One structural miss before it matched:** the natural-looking `if
(prev+1==1) {...} else if (prev+1==2) {...} else {...}` compiled to the
WRONG branch polarity — `bne`-skip-over-then blocks with extra `j`
instructions to rejoin, one word longer than retail and shifting the whole
image. Retail's actual shape is `beq v1,1,CASE1; beq v1,2,CASE2;
fallthrough DEFAULT` — direct equality branches straight to each case body,
no inverted skip-branches. Rewriting the same three-way dispatch as an
explicit C `switch (prev + 1) { case 1: ...; break; case 2: ...; break;
default: ...; break; }` reproduced that exactly: GCC 2.6.3's switch lowering
for this few, non-contiguous-enough-for-a-table case values emits
straight-to-case equality branches rather than an if/else-if chain's
inverted skip-branches, even though the two are logically equivalent C.

### Proposed learning

An if/else-if/else chain and a `switch` over the same small integer set are
NOT interchangeable at the instruction level on this compiler: retail's
`beq`-to-case-body branch shape (no skip-branches, no extra rejoin jumps)
is switch-statement lowering, not if-else lowering. If a 2-or-3-way integer
dispatch mismatches by exactly one word with extra unconditional jumps
appearing after each `if` block, try `switch` before suspecting anything
else.

## Naming

`RegisterRecordTableFiles` -- left unrenamed (round 73, alpha, track-3 pass). Called
once from `DayTask__DayTask`'s ctor as `RegisterRecordTableFiles(1)` (return
discarded) and once from `game_shell.c` as `RegisterRecordTableFiles(0)` (also
discarded). It is not a class method (no `self` parameter, not reachable
through any vtable slot in either the 33-slot or 28-slot table this unit
resolved), and its own body -- a two-call-deep counter over
`sRecordRegisterCalls`/`sRecordFirstBatchCount` feeding a loop on `RegisterFileTableEntries` --
does not establish what it is registering. Per CLAUDE.md, "a wrong tier-A
name is worse than `func_`"; this stays `RegisterRecordTableFiles` rather than assert
a guess.

## History moved from comments (track 7, round 99, charlie)

The unit's comment on `GetRecordTable` read: "Sony's, from the
still-uncarved psyq_39094 SDK segment (asm/psyq_39094.s): `if (out != NULL)
*out = 0x230; return &gRecordTable;` ... Declared locally per CLAUDE.md's
rule against writing C for SDK-owned code." That is no longer true:
GetRecordTable is game code, matched in `src/code_39094.c`, and the comment
now says so. (The derivation above quotes the old comment as it was.)

## Naming (track 7, round 99, charlie)

`func_8004A070` -> `RegisterRecordTableFiles` (tier B). The body, read with
its two callees: GetRecordTable returns sRecordTable (0x230 records of 0x1C
bytes, each a file path first; game_files.c's banner) and its count;
RegisterFileTableEntries (game_shell.c) appends `count` records to the CD
driver's file table and resolves them, returns 0 to be retried, and 1 when
the CD driver is not the active source. So the function registers the record
table's files with the CD driver: on its first call all of them when `all`
is set (and it then counts itself as two calls), else the first half; on its
second call the rest; later calls register nothing. The mechanics are
certain; why the table is registered in halves (DayTask's ctor passes 1,
GameApplication__RegisterFilesCallback 0) is not, hence B.

- `D_8008A978` -> `sRecordRegisterCalls` (tier A: a call counter, read and
  written only here).
- `D_8008A97C` -> `sRecordFirstBatchCount` (tier A: the first batch's record
  count, which the second call subtracts).
- Parameter `arg0` -> `all`; locals `local`/`obj` -> `count`/`table`.

## Track 10 (2026-09-28, round 104, echo)

game_files.h's `FilePathRecord` (an opaque `u8 data[0x1C]`) merged into cd_driver.h's `CdFileEntry` ({name[0x14], CdlLOC pos, u32 size}, 0x1C): sRecordTable goes GetRecordTable -> RegisterRecordTableFiles -> RegisterFileTableEntries -> SetFileTable, so its records are the CD driver's file-table entries. game_files.h includes cd_driver.h; sRecordTable is declared `CdFileEntry[]` and GetRecordTable returns `CdFileEntry *`, dropping the casts at its callers, and a record used as a path is spelled `record->name` instead of a `(char *)`/`(const char *)` cast. Byte-identical.
