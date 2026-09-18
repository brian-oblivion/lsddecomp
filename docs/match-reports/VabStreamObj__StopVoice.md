> Renamed from `func_8002CB18` on 2026-09-18 (tools/rename.py). Address 0x8002cb18.

# VabStreamObj__StopVoice -- MATCHED (16/16 words)

Unit: `code_179d8_e`. Runner: echo, round 17.

## Result

```c
s32 VabStreamObj__StopVoice(ObjDA34 *self, s32 index) {
    if (index < 0x18) {
        func_80031890(index);
    } else {
        func_80031F3C(0);
    }
    return -1;
}
```

with `extern void func_80031890(s16 index);` declared at the top of the unit.

Byte-exact, 16/16 words.

## Notes

`gVabStreamObjMethods`'s vtable slot +0x084. `self` is unused in the body -- confirmed
by the actual instructions only ever reading `$a1` (the second argument
register), never `$a0`. Its only caller is the vtable slot itself
(`FlushSoundCueSet`, this unit), which passes `(self, arr[i].field0)` -- two
args -- so the unused-`self` parameter has to stay in the C signature or
`index` would land in `$a0` instead of `$a1` and every access would be
wrong.

Both call targets' (`func_80031890`, `func_80031F3C`) return values are
discarded -- retail unconditionally sets `$v0 = -1` after either branch --
so the function always returns `-1` regardless of which path is taken.

**First attempt (11/16) mis-typed `func_80031890`'s parameter as `s32`.**
Retail computes the argument with an explicit sign-extending truncation
(`sll $a0,$a1,0x10` / `sra $a0,$a0,0x10`) immediately before the `jal`, not a
plain register move, and does the move only inside the taken branch. With
the parameter declared `s32`, GCC instead moved `index` from `$a1` into
`$a0` unconditionally at function entry (a plain `move`, no truncation) --
consistent with the callee expecting a full 32-bit value, so no narrowing
was needed and the compiler had no reason to delay the move past the
`slti` test. Declaring the callee's parameter `s16` forced the same
truncate-immediately-before-the-call shape retail has, and the premature
`move` disappeared along with the type mismatch -- both symptoms had the
same one-line cause.

### Proposed learning

When a near-miss shows a register move or truncation happening EARLIER (or
in a plainer form) than retail's, check whether the CALLEE's parameter type
is narrower than what you declared -- a truncating (`sll`/`sra`) argument
setup right before a `jal` is retail's tell that the callee's parameter is
`s16`, not `s32`, even though the call site's own value is a full 32-bit
local. Getting the callee's declared width right can also fix an unrelated-
looking early-function register-allocation difference, not just the
instructions at the call site itself.
