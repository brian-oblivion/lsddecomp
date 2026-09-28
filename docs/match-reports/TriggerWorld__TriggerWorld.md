# TriggerWorld__TriggerWorld -- MATCHED (34/34 words)

> Renamed from `func_80044A7C` on 2026-09-25 (tools/rename.py). Address 0x80044a7c.

Round 82, runner echo (GraphicsResources session, echo #8), 2026-09-25. Unit `GraphicsResources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 34/34 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Constructor: the parent gModelDataMethods's ctor (through GetModelDataMethods's table, with (self, arg, 0)), install gTriggerWorldMethods; if the argument's first word is nonzero, call its own +0x064 (TriggerWorld__Load) and return NULL on a nonzero result; otherwise return self.

Table slot (`tools/classtable.py`): gTriggerWorldMethods +0x008.

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable` and `SubBlockTable` sit at the top of
`src/GraphicsResources.c`.

```c
/* gTriggerWorldMethods +0x008: constructor -- the parent gModelDataMethods's (third argument
 * 0), then this table; when the argument's first word is set, its own
 * +0x064 runs, and a nonzero result fails the construction (NULL). */
void *TriggerWorld__TriggerWorld(DataSrc33808 *self, s32 *arg) {
    ((UnprototypedCtorTable *)GetModelDataMethods())->ctor(self, arg, 0);
    self->methods = GetTriggerWorldMethods();
    if (*arg != 0) {
        if (((s32 (*)())self->methods->setFlag)(self)) {
            return NULL;
        }
    }
    return self;
}
```

## Notes

First build; the same shape as TodSet__TodSet (gTodSetMethods's ctor). The allocator New_ModelData passes 1 as the parent's third argument; this subclass passes 0. setFlag is cast at the call site to return s32.

## Naming

- **TriggerWorld__TriggerWorld**, tier B (head review, round 83: was A). Constructor: the parent ModelData's ctor (owns=0), then this table; runs its own Load when the argument's first word is set.
  Head review, round 83: the class name rests on one caller's local view type, named by an earlier runner (DayTaskStageMap.c round 20 for LinkResource; DreamAux.c round 43 for TriggerWorld), not on this body. The body shows mechanics only, so tier B; track 4 may sharpen it.

## Track 4 (2026-09-26, round 88, bravo)

Now `void *TriggerWorld__TriggerWorld(TriggerWorld *self, struct ResourceSource *src)`: the `s32 *arg` was ModelData's descriptor, so `*arg != 0` reads `src->buffer != NULL`. The parent ctor is still called through UnprototypedCtorTable (it takes a third argument, 0, that the void-typed slot has no room for) and +0x064 through an `s32 (*)()` cast (setFlag is void; TriggerWorld__Load returns nothing, but the ctor tests $v0, as retail does). Bytes unchanged.

## History (moved from include/TriggerWorld.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
 * The name is round 83's (DreamAux.c had declared New_TriggerWorld's
 * result `TriggerWorld *`), kept as the only name any view gave the class.
 * What its own methods do:
```
