# New_Class86F88 -- MATCHED (27/27 words)

> Renamed from `New_Class86F88_3bb8c_j` on 2026-09-24 (tools/rename.py). Address 0x80051a5c.

> Renamed from `func_80051A5C` on 2026-09-24 (tools/rename.py). Address 0x80051a5c.

Unit: `src/class_3bb8c_j.c`. `New_Class86F88` -- the allocator for
`Class86F88_3bb8c_j` (a small BasicClass-derived sibling class discovered this
round, alloc size 0x54, vtable D_80086F88 reached through `GetClass86F88Methods()`
(class_3bb8c_k) -- NOT `D_80086ED0`/`Get_vtable_Obj86ED0`, which is a
DIFFERENT, unrelated class (`Obj86ED0`, established by class_3bb8c_i) that
this function's own body never touches; ROUND 75 CORRECTION, see
`Get_vtable_Obj86ED0.md`).

This function ALREADY had an extern declaration in the shared
`include/class_3bb8c.h` (class_3bb8c_f's own screening, "Address-of only
in this unit's own screening"), with the correct signature
`void *New_Class86F88(void *arg0, s32 arg1)` -- confirms `arg1` is `s32`
(class_3bb8c_f's own call site passes a literal `1`), which fixed this
unit's own ctor-table `+0x008` slot signature to match.

## Body

```c
void *New_Class86F88(void *arg0, s32 arg1)
{
    Class86F88_3bb8c_j *self = BMemPMgrAlloc(0x54);

    if (self == NULL) {
        goto fail;
    }
    GetClass86F88Methods()->ctor(self, arg0, arg1);
    return self;
fail:
    return NULL;
}
```

## Residue and how it closed

First attempt (`if (self) { ctor(...); } return self;`, no explicit early
return) scored 26/27: retail's `beqz $s0,.L80051AAC` delay slot holds
`li $v0,0` (a literal), but this form let GCC fold "return self" into
that delay slot instead (`move v0,s0`), value-equivalent (s0==0 there) but
byte-different.

Second attempt used a plain `if (self == NULL) { return NULL; }` early
return -- this made it WORSE (28 words, one too many): GCC placed the
NULL-return as a small block AFTER the success path with the success path
needing an extra `j` to skip it, rather than folding the NULL case into
the branch's own delay slot. This is exactly
DECOMPILATION_LEARNINGS' "An early exit returning a DIFFERENT value from
the main path: try goto and return both" entry.

Switching to `goto fail; ... fail: return NULL;` fixed it immediately:
the exit value is stolen into the branch's delay slot and the branch
retargets straight to the shared epilogue, matching retail exactly.

### Proposed learning

Another confirmed instance of the already-documented
"goto vs return, different-value early exit" lever
(`func_80025B34` in DECOMPILATION_LEARNINGS) -- worth noting it applies
here too even though the "different value" is a pointer (`self` vs
`NULL`) rather than two integer constants, and the caller is a `New_X`
allocator wrapper (sub-shape "ignores the constructor's return, one early
exit"), reinforcing that this sub-shape specifically wants `goto`.

## Naming

- `New_Class86F88` -- tier A. BMemPMgrAlloc(0x54) + dispatch through GetClass86F88Methods()->ctor(...) -- the project's standard New_X allocator shape, named per the "constructors New_Class" convention. Class identity (0x54-byte Class86F88_3bb8c_j, table D_80086F88) from class_3bb8c.h's round-15 HEAD NOTE plus classtable.py D_80086F88.
