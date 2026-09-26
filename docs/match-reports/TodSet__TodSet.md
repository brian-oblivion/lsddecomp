# TodSet__TodSet -- MATCHED (33/33 words)

> Renamed from `func_80045228` on 2026-09-25 (tools/rename.py). Address 0x80045228.

Round 82, runner echo (code_33808 session, echo #8), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 33/33 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Constructor: the parent gTodMethods's ctor (through GetTodMethods's table, same argument), install D_8006F590; if the argument's first word is nonzero, call its own +0x064 (TodSet__BuildTods) and return NULL on a nonzero result; otherwise return self.

Table slot (`tools/classtable.py`): D_8006F590 +0x008.

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808` and `CountedBuf33808` sit at the top of
`src/code_33808.c`.

```c
/* D_8006F590 +0x008: constructor -- the parent gTodMethods's, then this
 * table; when the argument's first word is set, its own +0x064 runs, and a
 * nonzero result fails the construction (NULL). */
void *TodSet__TodSet(DataSrc33808 *self, s32 *arg) {
    ((Ctor33808 *)GetTodMethods())->ctor(self, arg);
    self->methods = GetTodSetMethods();
    if (*arg != 0) {
        if (((s32 (*)())self->methods->setFlag)(self)) {
            return NULL;
        }
    }
    return self;
}
```

## Notes

First build. Slot +0x064 is `void setFlag(Self *)` in the unified macro, but this class's override returns a status: cast at the call site (`((s32 (*)())self->methods->setFlag)(self)`), shared slot not retyped. The parent ctor goes through the unprototyped Ctor33808 view.

## Naming

- **TodSet__TodSet**, tier A. Constructor: the parent Tod's ctor, then this table; runs Load when the argument's first word is set.

## Track 4 (2026-09-26, round 88, delta)

Now `void *TodSet__TodSet(TodSet *self, struct Src6F240 *src)` (include/TodSet.h). The parent call goes through the typed base table, `GetTodMethods()->ctor((Tod *)self, src)`, instead of Ctor33808 (Tod__Tod returns nothing, so the void slot fits it). `*arg != 0` is now `src->buffer != NULL`: it is the same first word, the buffer Tod__Tod adopts. The +0x064 call keeps its cast: the inherited slot is `void setFlag`, the occupant TodSet__BuildTods returns s32. Bytes unchanged.
