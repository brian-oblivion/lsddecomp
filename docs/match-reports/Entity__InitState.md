> Renamed from `func_8005D278` on 2026-09-19 (tools/rename.py). Address 0x8005d278.

# Entity__InitState -- MATCHED (39/39 words)

**Unit:** Entity · Runner: charlie, round 23.

## What it does

Reads `D_80089EA6[this->moodIndex*0x10]` (the same "GetUnlockEffect" table
`Entity__GetUnlockEffect` reads, unsigned byte load here vs. that stalled
function's signed one), and if `kind - 1` is unsigned-less-than 9 (i.e.
`kind` in `[1..9]`) calls a new self-only vtable slot with `arg1==1`. Then
unconditionally calls two more vtable slots.

## Final C

```c
void Entity__InitState(Entity *this) {
    s32 kind;

    kind = ((u8 *)D_80089EA6)[this->moodIndex * 0x10];
    if ((u32)(kind - 1) < 9) {
        this->methods->slot70(this, 1);
    }
    this->methods->slot10C(this, 0x42);
    this->methods->slot160(this);
}
```

## Attempt log

First attempt wrote the range check as `(u32)((kind + 9) & 0xFF) < 9`,
copying the `+9`/`&0xFF`/byte-sign-extend pattern from `Entity__IsNearTarget`'s
similar-looking table read by mistake -- that function's residue really
does need that shape, but this one's own disassembly is simpler:
`addiu $v0,$v0,-0x1` / `sltiu $v0,$v0,0x9`, no `andi` mask anywhere. The
wrong condition compiled to extra/different instructions and shifted the
whole image (127668 bytes outside-range, funcdiff's drift warning fired).
Fixed to plain `(u32)(kind - 1) < 9` and it matched immediately.

Two new `EntityMethods` vtable slots: `slot70` (`self, s32 arg1`, called
here with `arg1==1`) and `slot10C` (`self, s32 arg1`, called here with
`arg1==0x42`, unconditionally). `slot160` was already documented.

`kind` is read via a cast to `u8 *` (`((u8 *)D_80089EA6)[...]`) rather than
through the header's own `extern s8 D_80089EA6[]` declaration, because this
site's load is `lbu` (unsigned) while `Entity__GetUnlockEffect`'s stalled
body reads the same table with `lb` (signed) -- same table, two different
element interpretations at two different call sites. Casting locally avoids
redeclaring `D_80089EA6` with a conflicting type in this same translation
unit (which would be a silent fatal `conflicting types` error, per
CLAUDE.md's build-log guidance).

### Proposed learning

**A table read that "looks like" another function's near-identical-looking
table read is not evidence of the same expression shape -- check the actual
instructions before pattern-matching from memory.** Here `Entity__IsNearTarget`
(read moments earlier while scoping the whole batch) uses `D_80089EA6`-style
byte reads with a `+9`/`&0xFF`/sign-extend-by-shift idiom for an unrelated
purpose (widening a byte to a signed multiplier); this function reads the
*same table symbol* for a plain unsigned range check and has neither the
`+9` nor the mask. Carrying the first function's idiom into the second
produced a 127668-byte whole-image drift on the first attempt -- caught
immediately by the drift warning, not by a plausible-looking wrong score.

Also: none of the round-23 head broadcast's three levers apply here -- no
`s16` locals, no loop counter, no `&arr[i+j]` pointer arithmetic.
