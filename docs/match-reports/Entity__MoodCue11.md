# Entity__MoodCue11

> Renamed from `func_8005E7F8` on 2026-09-23 (tools/rename.py). Address 0x8005e7f8.

**Unit:** Entity_b · **Size:** 167 words · **Status:** MATCHED (167/167
words, whole-image build verified byte-exact) — the unit's largest function
and the last of this queue; the head predicted a residue would survive here,
but none did.

## What it does

`(Entity *this, EntityMoodHandlerArg *out) -> void`. The most elaborate
mood-dispatch handler in this unit, combining nearly every idiom the unit's
smaller handlers established individually:

1. `this->unk48 = -0x14;` then the usual `out->unk10 = slot148(this);`
   opener. `this->unk48` is a NEW field — a `s16` (`sh`/`lh`, not the `sw`/
   `lw` every other Entity field here uses).
2. `row = NULL;` then `if (out->unk4 % (this->unk80 / 2) == 0) { out->unk1C
   = 0xA; out->unk20 = 1; }` (same div-by-`unk80/2` idiom as
   `Entity__MoodCue09`).
3. Three-way dispatch on `this->unk44`:
   - `== 0xB`: THREE INDEPENDENT (not `else if`-chained) `unkFC` literal
     checks that each unconditionally overwrite `row` when matched (see
     below), then a range-checked call through `this->unk94`'s OWN vtable
     (`Unk94Methods::slot100`) that can reset `this->unkFC`/`this->unk44`.
   - `== 0xC`: single `unkFC == 0x7BC` check, sets `row`.
   - `== 0xD`: `this->unk48 = -0x78;` + `Class6B5CC__FaceTarget(...)` +
     `slot48(this, 1, SCALE_HALF)` (return discarded) + a `slot144`
     threshold check (the SAME 2-argument `slot144` established in
     `Entity__IsTargetInRange`/`Entity__MoodCue12`) gating a `slot30(this, 0xB)` call.
4. `if (this->unkFC == 0x618) { ... coin flip sets unk44/row ... }`
   (unconditional, independent of step 3's outcome).
5. `if (row != NULL) { slot44(this, 0, row); }`, then unconditionally
   `slotD0(this, this->unk48, 0);`.
6. `if (this->unk44 != 0xC && this->unk28 != 0) { slotCC(this, -0xC8, 0);
   }`. `this->unk28` is a NEW field (plain `s32`).

## Two residue classes, both closed

**1. Three independent `if`s look identical to `else if` in the disassembly
until you check whether later checks still run after an earlier match.**
The `unk44==0xB` arm's three `unkFC` literal checks
(`0xA8C`/`0xC6C`/`0xE10`) are NOT mutually exclusive in the COMPILED code —
retail falls through and re-tests the second and third conditions even
after the first one matches and sets `row` (impossible to trigger twice in
practice since `unkFC` can't equal three different constants at once, but
the COMPILER doesn't know that, and it changes the codegen). Writing them
as `else if` produced a `beq`+forward-jump shape (short-circuiting on the
first match); retail's actual bytes are `bne`+fall-through-to-next-check
for each, independently. Three separate top-level `if` statements (no
`else`) matched.

**2. Statement order inside a two-way branch is not decided by data
dependency alone.** The `unkFC == 0x618` coin flip's two arms each set
BOTH `this->unk44` and `row`, and retail orders the two writes DIFFERENTLY
per arm — `row` first in the `!= 0` arm (with the `unk44` store landing in
the branch's own delay slot), but `row` first in the `== 0` (else) arm too,
once traced through correctly. (An intermediate attempt guessed `unk44`
first for the else arm from a misreading of the disassembly; re-checking
the actual byte order at that specific address run settled it.) The
generalizable point: this compiler does not reorder two independent,
side-effect-only assignments within a basic block — whichever order they
are WRITTEN in a source statement sequence is the order they compile in,
same finding as `Entity__MoodCue16.md`'s "textual order controls register
pressure," now confirmed for plain store ordering with no register
pressure question at all.

## New symbols

- `Unk94Methods::slot100`, `s32 (*)(Unk94Obj *self)` — a second far slot in
  the `Unk94Obj` vtable, below `slot130`/`slot200`.
- `EntityMethods::slotCC`, `void (*)(Entity *self, s32 arg1, s32 arg2)`.
- `Entity::unk28` (`s32`) and `Entity::unk48` (`s16` — the unit's first
  confirmed halfword-sized Entity field).

## Final C

```c
void Entity__MoodCue11(Entity *this, EntityMoodHandlerArg *out) {
    u8 *row;

    this->unk48 = -0x14;
    out->unk10 = this->methods->slot148(this);
    row = 0;
    if (out->unk4 % (this->unk80 / 2) == 0) {
        out->unk1C = 0xA;
        out->unk20 = 1;
    }
    if (this->unk44 == 0xB) {
        if (this->unkFC == 0xA8C) {
            row = ROTATION_YAW_MINUS90;
        }
        if (this->unkFC == 0xC6C) {
            row = ROTATION_YAW_PLUS90;
        }
        if (this->unkFC == 0xE10) {
            row = ROTATION_YAW_MINUS90;
        }
        if ((u32)(this->unkFC - 0xD5D) < 0x78) {
            if (this->unk94->methods->slot100(this->unk94) != 0) {
                this->unkFC = 0;
                this->unk44 = 0xD;
            }
        }
    } else if (this->unk44 == 0xC) {
        if (this->unkFC == 0x7BC) {
            row = ROTATION_YAW_MINUS90;
        }
    } else if (this->unk44 == 0xD) {
        this->unk48 = -0x78;
        Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
        this->methods->slot48(this, 1, SCALE_HALF);
        if (this->methods->slot144(this, this->unk94) < 0x400) {
            this->methods->slot30(this, 0xB);
        }
    }
    if (this->unkFC == 0x618) {
        if ((rand() & 1) != 0) {
            row = ROTATION_YAW_PLUS90;
            this->unk44 = 0xB;
        } else {
            row = ROTATION_YAW_MINUS90;
            this->unk44 = 0xC;
        }
    }
    if (row != 0) {
        this->methods->slot44(this, 0, row);
    }
    this->methods->slotD0(this, this->unk48, 0);
    if (this->unk44 != 0xC) {
        if (this->unk28 != 0) {
            this->methods->slotCC(this, -0xC8, 0);
        }
    }
}
```

The `(u32)(this->unkFC - 0xD5D) < 0x78` range check compiles directly to
retail's unsigned-subtraction idiom, same as `Entity__MoodCue00`'s `>= 0x105 &&
< 0x238`, but written as an explicit cast this time since it's a single
range test rather than a two-constant `&&`.

## Attempt log

Three attempts. First: 47/167, length correct only after fixing the
`else if` -> three-independent-`if` issue above (caught immediately by
comparing retail's `bne`-falls-through-to-next-check shape against the
built `beq`-skip-ahead shape at the very first mismatch). Second: 163/167,
length still correct, 4-word residue isolated to the `unk44 == 0xC` (else)
arm of the `unkFC == 0x618` coin flip — an intermediate guess reordered it
to `unk44` first, which was wrong; re-reading the raw bytes at that
specific address confirmed `row` first in BOTH arms. Third attempt (row
first in both arms) matched.

## Proposed learning

Two, both stated above as the generalizable versions:

- **A chain of mutually-exclusive-looking literal-equality checks writing
  the SAME variable is not necessarily `else if` in the source — verify by
  checking whether retail's bytes re-test the LATER conditions after an
  earlier one already matched.** `bne`-falls-through per check (all
  independently tested) means separate `if`s; `beq`-jumps-past-the-rest
  after the first match means `else if`. This project's units have used
  redundant/independent tests like this at least twice now (also
  `Entity__MoodCue00.md`'s literal re-test); the safe default when several
  checks target the same field with mutually-exclusive constants is to
  look at the bytes before assuming `else if`.
- **This compiler does not reorder two independent single-assignment
  statements within a basic block on its own** — confirms and generalizes
  `Entity__MoodCue16.md`'s finding beyond the "value needs a callee-saved
  register" case to plain store ordering with no register-pressure
  consequence at all. When two writes to different fields/locals show up
  swapped in a residue, try swapping their SOURCE statement order before
  reaching for anything more elaborate.
