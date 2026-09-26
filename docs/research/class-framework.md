# The class framework: it is plain C, not C++

**Settled 2026-08-28.** This was the project's top open question, because
`class_39e08` alone is 415 functions and writing them the wrong way is
expensive to discover late.

**Answer: the game is plain C with a hand-rolled class framework.** No C++, no
`cc1plus`, no name mangling, no compiler-generated vtables. The existing build
pipeline is correct and needs no change.

The rest of this file is the evidence, because a conclusion this structural
should be checkable rather than believed.

## Why the symbol names prove nothing

`New_DreamSys`, `DreamSys__DreamSys`, `Get_vtable_DreamSys`, `BasicClass__*` —
all of these are **FirecatFG's hypotheses**, inherited from lsddecomp along with
the rest of the symbol file. There has never been a symbol leak for this game.
Reasoning "the names look like C++, therefore it is C++" is circular: the names
look like C++ because someone who suspected C++ chose them.

Everything below is from the bytes.

## What the code actually does

`New_DreamSys` (0x80058774), the allocation-and-construction sequence:

```
ori   $a0, $zero, 0x928        ; size
jal   BMemPMgrAlloc            ; allocator
addu  $s0, $v0, $zero
beqz  $s0, .L800587D0          ; NULL CHECK
jal   Get_vtable_DreamSys      ; -> &DREAMSYS_METHODS
lw    $v0, 0x8($v0)            ; slot +0x008
addu  $a0, $s0, $zero          ; this
jalr  $v0                      ; CONSTRUCTOR, CALLED INDIRECTLY
addu  $v0, $s0, $zero          ; return the object
```

`DreamSys__DreamSys` (0x800587F0), the constructor itself:

```
jal   GetActorMethods            ; -> &gActorMethods (the BASE class table)
lw    $v0, 0x8($v0)            ; slot +0x008 again
jalr  $v0, $a0 = this          ; BASE CONSTRUCTOR, ALSO INDIRECT
jal   Get_vtable_DreamSys
sw    $v0, 0x0($s0)            ; store table pointer at object offset 0
sw    $s2, 0x58($s0)           ; ... then ordinary field init
```

And a method call, the shape that appears everywhere:

```
lw    $v0, 0x0($s1)            ; obj->methods
lw    $v0, 0x80($v0)           ; slot +0x80
jalr  $v0
```

## What GCC 2.6.3's own C++ emits, for comparison

Not from memory — compiled with **this repo's `tools/gcc263/cc1plus`**, the
same toolchain that reproduces retail. The reproducer is at the bottom of this
file. Its output:

```
_vt.4Base:
        .half   0               ; delta
        .half   0               ; index
        .word   3               ; entry count
        .half   0
        .half   0
        .word   a__4Base
        ...                     ; 8 BYTES PER ENTRY
```

```
makeDerived__Fv:
        jal     __builtin_new
         li     $4, 0x0000000c  ; sizeof, no null check afterwards
        jal     __7Derived      ; CONSTRUCTOR CALLED DIRECTLY, BY NAME
         move   $4, $2
```

```
__7Derived:
        jal     __4Base         ; BASE CONSTRUCTOR, ALSO DIRECT
         move   $16, $4
        la      $3, _vt.7Derived
        sw      $3, 4($2)       ; vptr at offset 4 — AFTER the base's data
```

## The six discriminators

| | GCC 2.6.3 C++ | this game |
| --- | --- | --- |
| vtable entry size | **8 bytes** `{delta, index, pfn}` | **4 bytes**, flat pointer |
| table header | entry count, in an 8-byte slot | one non-pointer word, varying per class (`0x1F34`, `0x34`, `0x1130`, `0x11144`…) |
| constructor | called **directly by name** | called **through slot +0x008** |
| base constructor | called **directly by name** | called **through the base table's slot +0x008** |
| after allocation | no null check | **null check** |
| vptr offset | after base data members (4 here) | **0** |

**Virtual construction is the decisive one.** No C++ compiler can call a
constructor through a vtable — the object has no vtable pointer until the
constructor stores it, which is why the language forbids virtual constructors.
This game does it twice in two functions. That behaviour can only be written by
hand.

Supporting: the tables contain **null slots mid-table** (both examples above
have one at +0x03C). g++ fills an unimplemented virtual with `__pure_virtual`,
never with zero, and never leaves a hole.

## The falsification test, and its result

If any translation unit in the game were real C++, its vtables would be in the
data with the 8-byte `{0, 0, text-pointer}` stride proven above. Scanning the
entire 505856-byte executable for that pattern (3+ consecutive entries):

```
compiler-generated (8-byte-stride) C++ vtables found: 0
flat 4-byte function-pointer tables (>=8 slots):    128
```

**Zero, against 128.** Nothing in this binary was built by a C++ front end.

## What this means for the decomp

- **Keep the pipeline.** `cc1`, not `cc1plus`. Units stay `.c`.
- **Methods are ordinary C functions with an explicit `this` first parameter.**
  Write `void DreamSys_TimerTick(DreamSys *self, ...)`.
- **The method tables are DATA**, decompiled like any other data slot: a
  `static` struct-of-function-pointers initializer. A derived class's table is
  written out in full, as a copy of its base's with slots replaced — that is
  what the bytes show, so that is what the source did.
- **Every object has its table pointer at offset 0.** A `lw $v0, 0x0($reg)`
  followed by `lw $v0, <off>($v0)` and a `jalr` is a method call, and `<off>`
  identifies it. Resolve it with:

  ```sh
  .venv/bin/python3 tools/classtable.py DREAMSYS_METHODS
  .venv/bin/python3 tools/classtable.py DREAMSYS_METHODS --vs 0x800878D4
  ```

  The `--vs` form is the useful one: a derived table is its base's with some
  slots replaced, so the diff *is* the subclass's behaviour. DreamSys inherits
  48 slots from `gActorMethods` and adds 90, overriding 8.

- **58 classes (round 80's count; 60 tables), ~1425 method slots.** This framework is the game's backbone,
  not a corner of it. `class_39e08`'s 415 functions are almost certainly its
  implementation plus a large class hierarchy.

## The header word is a hierarchical class id (2026-09-25, round 80)

Word +0x000 of every table is the class id, read as a path of nibbles from
the low end: each nibble above the lowest non-zero one is one more level of
derivation, so the parent of `0x1F234` (Entity) is `0xF234` (no table of its
own), then `0x234` (Class65650), `0x34`, `0x4` and `0x0` (BasicClass). Three
independent checks agree:

- **Slot sharing.** For all 57 derived tables, the table sharing the most
  slots with it is an id-relative (parent, child, sibling, or in two cases a
  grandchild), and `classtable.py <child> --vs <parent>` reads as the parent's
  table with slots replaced (the CD and SPU drivers replace 14 of
  FileResource's 30) and, in 54 of 57, slots appended.
- **The game's own tests.** `(methods->header & 0xFFF) == id`,
  `(header & 0xFFFFF) == 0x1F234` and `(header & 0xF) == 4` are
  is-kind-of tests: a prefix match on the path, at the width of the level
  being asked about.
- **BasicClass's fifteenth slot.** Every one of the 57 derived tables has
  NULL at +0x03C, which `classtable.py` trims from the root table's end:
  BasicClass's table is 15 slots, not 14.

`python3 tools/typeviews.py --tree` prints the tree; it finds 58 classes.
`classtable.py --scan`'s 60 tables include `gStyleCueCallbacks` (a callback
array sharing no slot with any class) and `D_8006C0F8` (its first word is a
code pointer). `D_8006D430`'s scan also reads on into the next symbol,
`gDataSourceClientGetters`, a NULL-terminated list of table getters.

**Class methods in `psyq_*` segments.** 26 classes keep some or all of their
own methods in segments named `psyq_*` (`psyq_33808` holds 76, `psyq_322b4`
38, `psyq_10ee0` 19; `plan.py classes` marks a class with no C method `no C`).
Those tables derive from BasicClass through the id tree and keep its
inherited slots, which no Psy-Q library does. Round 70 left the same
segments for the operator on weaker evidence; whether they are game code is
the operator's decision and nothing has been re-segmented.

## Carve implication

splat derives symbols from `jal`/`j` references, and a method reached only
through a table has none — so the obvious prediction is that splat cannot see
those entry points and will under-split, gluing several real functions into one
symbol.

**That prediction was tested and is FALSE for this game.** spimdisasm also
scans DATA for pointers into `.text` and creates a symbol for each. Of the
**894** distinct function addresses referenced by class tables, **894 have a
glabel**; the only three without one are already matched as C, so splat
generates no `.s` for them. A corpus-wide prologue census over every uncarved
segment returns **zero** functions with more than one `addiu $sp, $sp, -`.

The per-function check is still worth knowing, because it costs nothing:

```sh
grep -c 'addiu *\$sp, *\$sp, *-' asm/nonmatchings/<unit>/<func>.s
```

A function allocates its frame exactly once, so any count above 1 is
conclusive. But do not budget time for hunting; treat a hit as surprising.

`tools/classtable.py --scan` remains useful for carve boundaries — the table
addresses are real function entry points, so they are known-safe places to
split a segment.

## Reproducer

```cpp
/* cpp -undef -lang-c++ -nostdinc -Dmips -D__GNUC__=2 vt.cpp > vt.ii
 * cc1plus -mips1 -mcpu=3000 -quiet -G0 -O2 vt.ii -o vt.s   */
class Base {
public:
    Base();
    virtual void a();
    virtual void b();
    virtual int  c(int x);
    int field;
};
class Derived : public Base {
public:
    Derived();
    virtual void b();
    int extra;
};
Base::Base()        { field = 1; }
void Base::a()      { field = 2; }
void Base::b()      { field = 3; }
int  Base::c(int x) { return field + x; }
Derived::Derived()  { extra = 4; }
void Derived::b()   { extra = 5; }

Base    *makeBase()    { return new Base(); }
Derived *makeDerived() { return new Derived(); }
```

`tools/gcc263/cc1plus` ships with the compiler tarball `tools/setup.sh`
fetches, so this stays runnable without adding anything to the toolchain.

## What would overturn this

An 8-byte-stride vtable appearing anywhere in the data, or a constructor called
directly by name with a `_vt.` pointer stored inline. Neither exists today. The
scan above is the check; it takes a second and the binary never changes, so
there is nothing here that needs periodic re-evaluation — only re-running if
someone doubts it.
