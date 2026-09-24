# New_Obj86ED0 -- MATCHED (27/27 words)

> Renamed from `func_80050BA8` on 2026-09-24 (tools/rename.py). Address 0x80050ba8.

Unit `class_3bb8c_i`, carved round 14.

`New_X`-shaped factory for `Obj86ED0` (see `include/class_3bb8c.h`, vtable
`D_80086ED0`, resolved with `tools/classtable.py D_80086ED0`): allocates the
0x4C-byte instance and dispatches its own ctor (slot 0x008, `Obj86ED0__Obj86ED0`,
itself a STALLED gp_rel-blocked function in this same unit -- see its own
report). `Get_vtable_Obj86ED0` (`class_3bb8c_j`, still `INCLUDE_ASM`) is this
class's own table getter, mirroring `Get_vtable_BasicClass`'s no-argument shape;
its return type only needed naming here (`Obj86ED0Methods *`), not a body.

```c
void *New_Obj86ED0(s32 arg0, s32 arg1)
{
    Obj86ED0 *self;

    self = BMemPMgrAlloc(0x4C);
    if (self != NULL) {
        Get_vtable_Obj86ED0()->ctor(self, arg0, arg1);
        return self;
    }
    return NULL;
}
```

## Residue and how it was closed

First attempt used the single-exit shape (`if (self != NULL) { ctor(...); }
return self;`), which is what `funcdiff` initially showed as 25/27: GCC 2.6.3
hoisted `move v0,s0` to run unconditionally on BOTH paths instead of retail's
`move v0,zero` on the null path. A second attempt used two explicit `return`
statements (early `return NULL;`, later `return self;`) hoping the compiler
would tail-merge the two epilogues the way retail's single shared label
suggests -- it did NOT: GCC 2.6.3 emitted a genuinely duplicated epilogue
plus a stray `j` into the middle of it, growing the function by 8 bytes and
shifting every later address in the unit.

The proven idiom already used elsewhere in this project's `New_X` functions
(`src/class_3bb8c_c.c`: `self = BMemPMgrAlloc(sz); if (self != NULL) {
ctor(self); return self; } return NULL;`) is what actually matches --
returning INSIDE the `if` block, with the final `return NULL;` falling
through from outside it. Applying that exact shape closed it to 27/27.

### Proposed learning

For a `New_X` allocator, do not improvise the null-check shape -- use the
already-proven idiom (`if (self != NULL) { ctor(self, ...); return self; }
return NULL;`) verbatim. Two visually-reasonable alternatives (`return self;`
after an un-early-returning `if`, or two separate top-level `return`
statements) both compile to something OTHER than retail's single shared
epilogue under this pinned GCC 2.6.3, one by hoisting the return value
computation above the branch, the other by failing to tail-merge the two
exits at all.
