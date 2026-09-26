# TextEntry__TickState -- MATCH (24/24 words)

> Renamed from `Obj86ED0__TickState` on 2026-09-26 (tools/rename.py). Address 0x80051370.

> Renamed from `func_80051370` on 2026-09-24 (tools/rename.py). Address 0x80051370.

Unit `class_3bb8c_i`. Gated `self->unk30` increment: only proceeds when
`self->unk2C` is 2 or 3, then increments `self->unk30` and dispatches
`slot54(self, 4)` only when the OLD value of `self->unk30` was nonzero.

```c
void TextEntry__TickState(Obj86ED0 *self)
{
    s32 tag;
    s32 old;

    tag = self->unk2C;
    if (tag >= 4) {
        return;
    }
    if (tag < 2) {
        return;
    }
    old = self->unk30;
    self->unk30 = old + 1;
    if (old != 0) {
        self->methods->slot54(self, 4);
    }
}
```

Straightforward match, no header changes, no residue. The two early-return
checks are written in the same order as retail's two `slti` comparisons
(`tag >= 4` first, matching `slti $v0,$v1,4; beqz`, then `tag < 2`, matching
`slti $v0,$v1,2; bnez`) rather than folded into one `tag < 2 || tag >= 4`
expression -- writing the fold produced a different (and, untested, possibly
also-matching) instruction sequence, but there was no need to explore that
once the direct transliteration matched first try.

## Naming

- `TextEntry__TickState` -- tier B. gTextEntryMethods +0x058 (tickState slot, classtable.py), dispatched by TextEntry__OnNotify's tag==5 case. Per-notify advance while closeState is in [2,4): increments closeTickCount, and once it was already nonzero, calls setState(self,4) -- a one-tick-delayed close-to-finish handoff. Purpose (why a delay) not established.
