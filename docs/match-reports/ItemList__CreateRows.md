# ItemList__CreateRows — MATCHED (round 45, 82/82 words)

> Renamed from `Class86F88__CreateRows` on 2026-09-26 (tools/rename.py). Address 0x80052644.

> Renamed from `func_80052644` on 2026-09-24 (tools/rename.py). Address 0x80052644.

**Unit:** class_3bb8c_k · **Size:** 82 words (0x148 bytes)

Filed as a `gp_rel`-blocked stub in round 15. That blocker was RESOLVED in
round 42 (`--gp-symbols`, pinned in the Makefile). The unit header comment
had been left saying "STILL BLOCKED, stub report stands" (from a round-27
correction that only meant to clear a DIFFERENT function's false-positive
`addiu_at` tag) — that line is retracted by the round-42 banner at the top
of the same file; this function is ordinary matching work.

## Derivation

```c
extern char *ItemList__FormatRowText(ItemList *self, char *dest, s32 arg3, s32 arg4, char *base);
extern void ItemList__SetView(ItemList *self, s32 a1, s32 a2, s32 a3, s32 a4);

extern s32 gItemListRowOriginX;
extern s32 gItemListRowOriginY;

typedef struct {
    s32 a;
    s32 b;
} Elem4CArg_3bb8c_k;

void ItemList__CreateRows(ItemList *self, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    char buf[0x20];
    Elem4CArg_3bb8c_k local;
    ItemListElem **p;
    s32 count;
    s32 i;

    if (!self->unk50) {
        return;
    }

    local.a = gItemListRowOriginX;
    local.b = gItemListRowOriginY;
    count = self->unk10;
    p = &self->unk40[0];
    if (count >= 5) {
        count = 4;
    }

    for (i = 0; i < count; i++) {
        ItemList__FormatRowText(self, buf, i, arg3, (char *)arg4);
        *p = (ItemListElem *)New_TextRow((void *)arg2, 0x1A, buf);
        (*p)->methods->slot4C(*p, arg1, &local);
        (*p)->methods->slotB8(*p, &gItemListRowColor);
        local.b += 0xA;
        p++;
    }

    ItemList__SetView(self, arg3, arg4, arg5, 1);
}
```

**A close sibling of the already-matched `ItemList__RefreshRows`** (same unit, same
`self->unk40[]` element array, same `count = self->unk10; if (count >= 5)
count = 4;` clamp, same `ItemList__FormatRowText` text-formatting call inside the
loop): the two differ in that this function ALSO calls each freshly-created
element's own `slot4C` (with a 2-word stack-local argument seeded from
`gItemListRowOriginX`/`gItemListRowOriginY`, the second word accumulating by `0xA` per
iteration) and `slotB8` before `ItemList__RefreshRows`'s sibling code reaches its
own `slotCC`, and this one always passes `1` (not a caller flag) as the
final `ItemList__SetView` argument. `ItemListElemMethods::slot4C` (offset
0x04C) is a new additive header field, typed `void *arg2` there (the
concrete 2-word `Elem4CArg_3bb8c_k` struct is kept unit-local, since it's
this one call site's own reading) — its offset and neighbor (`release` at
0x004, `slotB8` at 0x0B8) line up exactly with a DIFFERENT unit's own
`FieldM7CMethods` (`include/class_3bb8c.h`, established from
`ObjM__AdvancePauseSetup`), which is suggestive but not something this round needed
to resolve.

`New_TextRow` already has a SHARED declaration in `include/class_3bb8c.h`
(`extern FieldM7C *New_TextRow(void *ctx, s32 len, char *name);`, a
different unit's own independent view of the same uncarved external
function) — this unit's `self->unk40[]` is typed `ItemListElem *`, a
different local name for what the header comment already notes might be
the same real object, so the return value is cast explicitly rather than
adding a second, conflicting declaration.

`ItemList__FormatRowText` and `ItemList__SetView` are defined LATER in this same file
(ROM order), so both needed forward `extern` declarations above
`ItemList__CreateRows`, matching their real definitions exactly — same convention
already used for `DayTask__StartObjM`/`TextEntry__ClearChildRefs` elsewhere this round.

**One register-identity trap, closed by reordering two local
declarations — no logic change.** With `Elem4CArg_3bb8c_k local;` declared
before `char buf[0x20];`, GCC decided `buf`'s address (used at two call
sites, `ItemList__FormatRowText` and `New_TextRow`) was worth caching in a
callee-saved register across both uses, rather than recomputing the cheap
`sp`-relative address each time — retail does NOT cache it (two separate
`addiu $a1/$a2, $sp, 0x18` computations). That one cached register pushed
every other `s`-register up by one (`s3`→`s4`, `s7`→`s8`, …) and, since
`buf` also moved to a different stack offset as a result, shifted the
whole function's addresses on top of it (2/82 words matching). Simply
declaring `buf` FIRST — otherwise identical C — made the compiler treat
its address as cheap-to-recompute again and matched byte-exact.

### Proposed learning

**A local array's DECLARATION ORDER relative to other locals can change
whether the compiler caches its address in a register across multiple
uses, even with no change to which statements reference it.** If a
function's register file is off by exactly one consistent callee-saved
register the whole way through (every `sN` shifted up or down by one, not
a scattered mismatch), and the function takes the address of a local array
at more than one call site, try reordering that array's declaration
relative to other locals before suspecting anything else — this is now the
SECOND such attribution trap this round (`StampSaveTitleFileLetter`'s report has the
sibling case: a function-WIDE shared local, not a declaration-order issue,
but the same family of "an unrelated-looking local variable decision
shifts register allocation across the whole function").

## Naming

Round 75 (bravo, track 3). `func_80052644` -> `ItemList__CreateRows`, **tier A**.

Slot +0x08C (`tools/classtable.py gItemListMethods`). Called by ItemList__LoadResources (class_3bb8c_i) as (self, parent, FONTICON handle, topIndex, column, cursorIndex). Builds min(itemCount, 4) row text objects with New_TextRow(font, 26, text), lays each out at (gItemListRowOriginX, gItemListRowOriginY + 0xA*i), colours it gItemListRowColor, then SetView(..., highlight=1).

ItemList, per the round-75 pass, is a scrolling list selector: up to 4 visible rows of 26-character item text, a highlighted cursor row, a horizontal column offset (see the unit header comment of `src/class_3bb8c_k.c`).

## Round 99 (delta, track 7)

Local `p` -> `row`. `0x20` -> 32 (the buffer, decimal as a size), `0x1A` -> `ITEMLIST_ROW_CHARS`, the clamp `>= 5 ... = 4` -> `ARRAY_COUNT(self->rows)`, the row step `0xA` -> `ITEMLIST_ROW_SPACING` (10, unit-local define: only this function uses it). `buf` declared first keeps a `MATCHING:` line.
