> Renamed from `func_8005DAFC` on 2026-09-19 (tools/rename.py). Address 0x8005dafc.

# Entity__StartSoundCue -- MATCHED (36/36 words)

**Unit:** Entity · Runner: charlie, round 23.

## What it does

Lazily-flavoured "start" call: initializes a slot table via `InitSoundCueSet`
(matched in `code_179d8_e.c`), passing that unit's own `this->unk58`, the
address of `this->unk9C` as the object to init, `this->moodIndex + 1` as
`arg2`, `this` itself as `arg3`, and the current mood row's dispatch-handler
function pointer (`D_80089EB0[this->moodIndex].handler`, first word of the
16-byte `D_80089EB0` row) as `arg4`. Then calls two more self-only vtable
slots and resets a pair of counters.

## Final C

```c
typedef struct EntityMoodHandlerRow EntityMoodHandlerRow;
struct EntityMoodHandlerRow {
    void *handler; /* +0x00 */
    u8 pad04[0x10 - 0x04];
};
extern EntityMoodHandlerRow D_80089EB0[];
extern void InitSoundCueSet(s32 arg0, void *arg1, s32 arg2, Entity *arg3, void *arg4);

void Entity__StartSoundCue(Entity *this) {
    InitSoundCueSet(this->unk58, &this->unk9C, this->moodIndex + 1, this,
                  D_80089EB0[this->moodIndex].handler);
    this->methods->slot12C(this);
    this->methods->slot110(this);
    this->unkFC = 0;
    this->unkF8 = 1;
}
```

Byte-exact on the first attempt, whole-image build verified.

## Notes

`InitSoundCueSet`'s own match report (in `code_179d8_e.c`) already established
its real signature; DreamSys.c's own extern (`InitSoundCueSet(s32, void *,
s32, DreamSys *, void *)`) is the same function called with a different
unit's own local `arg3` type -- per project convention, this file's own
extern types `arg3` as `Entity *` instead, matching the "multiple
independent local views" rule for cross-unit prototypes (kept local to this
`.c`, not added to `Entity.h`, since `InitSoundCueSet` is defined in a
different unit).

`D_80089EB0` is the mood-index-selected event-dispatch table (16-byte rows:
handler fn ptr + 3 data words) already referenced by name in several match
reports for `Entity_b`'s handler functions (e.g. `func_8005E480`), but this
is the first place any unit indexes the RAW TABLE itself in C rather than
just being one of its handler bodies. Declared a minimal
`EntityMoodHandlerRow` (only the first word named) directly in `Entity.c`,
not `Entity.h` -- the only field this function needs is the handler pointer,
treated opaquely (passed straight through to `InitSoundCueSet` as a `void *`,
never called here). Whoever carves `func_8005DE18` (the actual table
dispatcher, still in the uncarved `Entity_b`) should check whether this
minimal row type is enough or needs the data words added, and should
probably promote it to `Entity.h` at that point since it would then have
two real users.

Two new `EntityMethods` vtable slots added, both self-only `void`:
`slot110` (this function's second call) and `slot12C` was already
documented. `slot110`'s only known caller is this function.

### Proposed learning

None beyond what's already documented -- this one was a clean, mechanical
translation with no residue. The round-23 head broadcast's three levers
(s16-local widening, sltiu-implies-unsigned-counter, subscript-vs-pointer
addressing) do not apply here: no `s16` locals, no loop, no `&arr[i+j]`
shape.
