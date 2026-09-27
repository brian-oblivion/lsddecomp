# TextRow__Finalize — MATCHED (22/22 words)

> Renamed from `Obj6EAC0__Destruct` on 2026-09-26 (tools/rename.py). Address 0x80040a30.

> Renamed from `func_80040A30` on 2026-09-18 (tools/rename.py). Address 0x80040a30.

Unit: `src/code_2cc8c_f.c`. First attempt.

```c
void TextRow__Finalize(Obj6EAC0 *self) {
    ReleaseBasicClassArray(self->unkB4, self->unkA9);
    self->unkB4 = BMemPMgrFree(self->unkB4);
    GetCharSpriteMethods()->finalize((CharSprite *)self);
}
```

Derived-table occupant of `Obj6EAC0Methods::slot0C`. Retail's own
delay slot after the `GetCharSpriteMethods()` call stores the PRECEDING
call's return (`BMemPMgrFree`'s), not `GetCharSpriteMethods`'s own -- read
per the established "a delay slot after a `jal` carries the value from
the PRECEDING call's return" idiom, and it is what motivated retyping
`BMemPMgrFree`'s shared declaration (see below).

## Existing declaration retyped

`include/code_2cc8c.h`'s `BMemPMgrFree` was declared `void
BMemPMgrFree(void *ptr)`, following `code_171e0.h`/`Entity.h`'s
typing. Its own (still-`INCLUDE_ASM`) disassembly
(`asm/nonmatchings/BMemPMgr/BMemPMgrFree.s`) ends with an explicit
`addu $v0, $zero, $zero` -- it genuinely returns `NULL`, and this
function is the first in this unit to actually USE that return value
(`self->unkB4 = BMemPMgrFree(self->unkB4)`). Retyped to
`void *BMemPMgrFree(void *ptr)`. Every other call site in this unit
(`code_2cc8c_b.c`, `code_2cc8c_d.c`) discards the result as a bare
statement, so the retype changes no compiled bytes there; confirmed by
a full green `build-and-verify.sh`.

### Proposed learning

Another instance of "a discarded return value is never evidence of
void" -- this one caught BEFORE it caused a stall, because the callee's
own disassembly was checked directly rather than trusted from another
unit's caller-side typing.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040A30` | `TextRow__Finalize` | B |

**Evidence.** Derived occupant of `slot0C`: releases the whole
`children` array (`ReleaseBasicClassArray(self->children,
self->totalChildCount)`), frees the array pointer itself
(`BMemPMgrFree`), then chains to the third sibling table's own `slot0C`
-- the mirror-image teardown of `TextRow__TextRow`'s own setup. Tier B
for the same reason as `Construct`: the destructor role is certain from
the mechanics, the class's broader purpose is not independently
confirmed.

## Track 4

2026-09-26, round 86 (bravo): the parent class 0x1144 is unified as CharSprite (`include/CharSprite.h`, formerly D_8006EC74). The base call is `GetCharSpriteMethods()->finalize((CharSprite *)self)` (was `->slot0C(self)`): an upcast, no code. Image byte-identical.

## Track 4 (2026-09-26, round 88, charlie)

2026-09-26, round 88 (charlie): class 0x11144 unified as TextRow in `include/TextRow.h` (a row of CharSprite cells: the ctor makes `count` New_CharSprite cells, setText hands each the next byte of a string, the layout slots step `cellPitch` along x). The view `Obj6EAC0` (named after BoxFill's old table address) is gone; its +0x00C `hasChildren` is SceneNode's `parent` (--merge CONFLICT s32 vs pointer: only tested against 0, bytes unchanged), `children` (+0x0B4) is `CharSprite **cells`, and the per-cell calls go through CharSprite's slots by name. Renamed from `Obj6EAC0__Destruct`: the +0x00C finalize occupant. Releases `cellCount` cells (ReleaseBasicClassArray), frees `cells`, then CharSprite's finalize. Image byte-identical; the current source is src/code_2cc8c_f.c.
