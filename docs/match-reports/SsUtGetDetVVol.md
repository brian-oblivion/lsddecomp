# SsUtGetDetVVol -- MATCH (22/22 words, 2 rebuild attempts)

> Renamed from `func_80031C98` on 2026-09-23 (tools/rename.py). Address 0x80031c98.

Unit `code_179d8_j`, round 21 (2026-09-06). Not a class method
(`D_8006DAD4` is a plain global pointer variable, no `classtable.py`
hit). This is the "raw getter" half of a pair with `SsUtGetVVol`
(same table, same bounds, `SsUtGetVVol` divides each field by 129
before returning it -- see that function's stall report).

```c
/* Base pointer for a table of 0x10-byte entries, indexed by a 0..0x17
 * id.  Only the two leading s16 fields this unit's own accessors touch
 * are named. */
typedef struct EntryDAD4 {
    s16 unk0; /* +0x0 */
    s16 unk2; /* +0x2 */
    u8 pad4[0x10 - 0x4];
} EntryDAD4;
extern EntryDAD4 *D_8006DAD4;

s32 SsUtGetDetVVol(s16 idx, s16 *out1, s16 *out2)
{
    if ((u16) idx < 0x18) {
        *out1 = D_8006DAD4[idx].unk0;
        *out2 = D_8006DAD4[idx].unk2;
        return 0;
    }
    return -1;
}
```

**First attempt used the inverted guard-clause form** (`if (invalid)
return -1; ... body ...; return 0;`) and got the CONTROL FLOW backwards:
retail's `bnez $v0,<body>` / fallthrough `j <end>; li $v0,-1` shape is
what you get from `if (valid) { body; return 0; } return -1;` -- the
guard-clause form produces the opposite branch polarity (`beqz` to a
tail fail-block) for this specific shape. Second attempt (the if/else
form above) matched immediately.

Confirms retail re-loads the `D_8006DAD4` global pointer a second time
between the two field reads (there is an intervening store to `*out1`
that the compiler can't prove doesn't alias the global) -- writing the
two field copies as two independent indexing expressions reproduces
this without needing to suppress it.

### Proposed learning

**For this "bounds check then body, else return a sentinel" shape, the
guard-clause form (`if (invalid) return X;` followed by the body) is NOT
always what retail's branch polarity implies -- check whether the VALID
path is the one that branches AWAY (to a label) while the invalid path
falls straight through into an inline `j end; li v0,X`.** When it is (as
here, and in the two siblings `SsUtAutoVol`/`SsUtAutoPan`), the
`if (valid) { body; return 0; } return X;` form is what reproduces it.
This is the opposite of MATCHING-GUIDE's usual "write early exits as
guard clauses" advice for LARGE bodies -- for a body this small, retail
apparently kept the natural `if`/`else` and it is the SOURCE's actual
shape, not something GCC restructured.
