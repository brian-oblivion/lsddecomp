# func_80063CAC

**Unit:** Entity_f · **Size:** 7 words · **Status:** MATCHED (7/7 words)

## What it does

Sibling of `func_80063C84`: another one-parameter `out`-only setter,
`void func_80063CAC(EntityMoodHandlerArg *out)`: `out->unk1C=0x12;
out->unk10=0; out->unk30=3; out->unk44=3;`.

## Derivation

First transcription wrote `out->unk44 = 0x12` (matching the value used for
`unk1C` a few lines above, an easy visual slip) instead of the actual
stored value, `3` (the SAME register value already computed for
`out->unk30` two lines earlier, reused rather than reloaded — confirmed
against the raw hex, `sw $v0, 0x44($a0)` with `$v0` unchanged since the
`ori $v0, $zero, 0x3` two instructions prior). One-word fix.

## Proposed learning

**A tiny multi-field setter reusing one register across several stores is
exactly the shape where "the value looks like the one above it" produces
a plausible-but-wrong guess** — verify each `sw`'s SOURCE register's most
recent assignment individually rather than pattern-matching against a
neighboring store's literal.
