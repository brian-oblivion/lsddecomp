# FINISHING-PLAN tracks 3, 4 and 5, as they stood at revision 26

Moved here verbatim on 2026-09-26 (revision 27) when all three were done and
phase 2 (tracks 6 to 9: the code reads like a game's source) needed the room
in the plan's word budget. They are the rules to follow if `plan.py` ever
reopens one of these tracks (an unpassed unit, a stray class view, a global
with two types, an unticked item); the section numbers below are revision
26's. The naming rules and the class model were NOT moved: phase 2 uses
them, and they live on in FINISHING-PLAN §3.

### Track 3: readability, one unit at a time

**Goal.** A reader can tell what each function, field, slot and global in the
unit does without opening the disassembly.

**Eligibility.** Every unit is eligible now; remaining `INCLUDE_ASM`s are
documented stalls. **Order:** `plan.py`'s centrality ranking (units whose
definitions other units reference most), because a name in a base class
propagates everywhere. **One pass per unit, no second pass** (revision 19):
the `func_`, `unk` and `slotNN` a marked unit still holds are named class by
class in track 4 (step 6 names slots and their occupants), and what track 4
leaves stays tier C with what is known written down.

**The pass, per unit.**
1. Read every function in the unit, its callers (`grep -rn` over `src/`),
   its class table (`tools/classtable.py`), and the unit's existing header
   comment and reports.
2. Name functions with `tools/rename.py` (one command: symbols file, sources,
   report file, extract, verify). Never by hand.
3. Name struct fields and vtable slots. (Functions and globals are NOT
   subject to this rule: `rename.py` renames a symbol tree-wide whoever calls
   it, because a symbol name is unique. Only FIELDS and SLOTS share names
   across structs.) Ownership is decided per FIELD by the COMPILER, never by
   grep: `slotNN` and `unkNN` names recur across unrelated structs, so a
   textual search over-counts (round 57: seven textual hits, one real
   accessor). Rename the field in the struct DEFINITION only and rebuild; the
   error list is the exact accessor set AMONG CODE THE DEFAULT BUILD
   COMPILES. `#ifdef NON_MATCHING` bodies are outside it by construction, so
   `tools/check-nonmatching.sh` is the second half of the list and is required
   after every field or slot rename (round 67 shipped a regression there).
   - every error, in both, is in your unit: fix them, both oracles, done;
   - any error is in another unit: revert the definition and do NOT rename. Put the proposed name, its
     tier and its evidence under `## Proposed field names` in the report of
     the function that established it, and post it to the broadcast. The HEAD
     applies it at merge time by TYPE SCOPE, never by whole-tree replace:
     rename the field in the struct DEFINITION only, rebuild, and the
     compiler lists every accessor of that struct as an error; fix exactly
     those, rebuild, oracle. Same-named fields in other structs are untouched
     because their definitions did not change. (Revision 2 claimed a
     whole-tree replace was safe because a mis-hit fails to compile; round 52
     measured `unk10` in 347 places across 50 files, where replacing
     definitions and uses together compiles clean and mislabels ~49 structs.)
   Never move an offset or a size in a shared header (CLAUDE.md, the
   shared-struct hazard). A return-type or parameter-type CORRECTION to a slot
   or prototype is allowed when the oracle stays green and the report lists
   every caller you checked (CLAUDE.md: a tail-call wrapper's byte match says
   nothing about its return type, so the callers are the evidence).
4. Name globals with `tools/rename.py`. Replace magic constants with named
   constants or enums where the meaning is established.
5. Write the unit's header comment: what the unit IS (which class or
   subsystem), in five to fifteen lines. Delete stale blocker banners.
6. Do the same to any `NON_MATCHING` bodies in the unit.
7. In each function's report add `## Naming`: the name, its tier, and the
   evidence (call sites, data touched, strings, table slot).
8. `./build-and-verify.sh` green after every step. A rename changes zero
   bytes; red means the rename is wrong.

**Naming rules.**
- **Name what the code does, never what you guess it is for.**
  `DreamSys__ResetLinkState` is evidence-based even when nobody knows why;
  `PlayNightmareSound` is a guess when all you see is a sound call.
- **Tiers, recorded in the report:** A, purpose known: evident from the body
  alone, or from two or more callers that agree, and a pure leaf whose
  mechanics ARE its purpose (a getter, a clamp, a list push) is tier A by
  definition; B, mechanics described but purpose in the game not established;
  C, placeholder kept, with what IS known written down. The tier-C form for a
  method whose CLASS is known is `Class__func_xxxxx` (e.g. `Foo__func_12345`;
  examples here are synthetic so `rename.py` can never rewrite them), and bare
  `func_800xxxxx` otherwise;
  `plan.py` counts both as unnamed. A wrong tier-A name is worse than a
  placeholder. A tier-B name is expected to be sharpened later; renames are
  cheap now.
- **Conventions, from the code as it stands:** methods `Class__Method`
  (`DreamSys__AdvanceDay`), where `Class` is the struct's type name;
  free functions `VerbNoun` (`CalcDreamColor`); constructors `New_Class` and
  `Class__Class`; types `PascalCase`; fields `camelCase`; globals `gName`;
  unit-static data `sName`; constants `UPPER_SNAKE`; vtable slots named like
  the method they dispatch to. Do not invent a new style.
- **Every inherited name (FirecatFG's, `CREDITS.md`) is a tier-B hypothesis.**
  Confirm it with evidence or rename it; either way, record it.
- **Do not rename a Sony symbol, and give no game name to anything Sony
  owns**: a function `progress.py` counts as library, a variable
  `psyq-objects.ld` pins or an address inside one (`rename.py` refuses both),
  or data only Sony functions read, which an object that never placed leaves
  unpinned (round 86: 29 libsnd/libapi variables; `rename.py` refuses a
  `gName`/`sName` for it, `tools/sonydata.py` lists it with Sony's name as a
  lead to anchor, and a static no disc names goes back to its placeholder,
  `rename.py OLD D_<own address>`). All are track 2's.

**Head at merge.** Apply the runner's proposed cross-unit field names one at
a time by type scope (definition first, compiler lists the accessors, fix
those, oracle), then `make extract` (the symbols file changed) and the
oracle.

**Head review before `mark-unit`.** Sample five names per unit against their
evidence. A name the head has to change because it claims more than its
evidence shows, at ANY tier (round 77's "UnlessOverridden", "Wobble"), or a
class prefix `classtable.py` contradicts (round 73), sends the unit back.
Convention fixes (`g`/`s`, spelling) do not, and neither does a Sony
function no tool flagged: that gap is the tool's. Two clean Opus units in a row and
the naming runner becomes Sonnet (`plan.py set-model --role naming_runner
--model sonnet`); one Sonnet unit sent back and it goes back to Opus. Then:

```sh
python3 tools/plan.py mark-unit --unit <unit>
```

**Runner** prompt in §4.2. **Park rule:** a unit whose class cannot be
identified after a full pass is marked with what was learned and a
`NAMING PARKED: <reason>` line in its header comment; the head still marks it
so the plan moves on, and track 4 picks it up with the class.

### Track 4: types and data

**Opens** when 80% of units have passed track 3 (`plan.py` says).

**Goal.** 4a: one definition per class, in a header named for it, with every
unit-local view of the class merged into it. 4b: one type per global. Then
state values as enums, once their meanings are named.

**Measure, never grep.** `python3 tools/plan.py classes` prints every class in
tree order with its state (UNIFIED, PARKED, ready, waiting, `no C`), how many
of its own methods are C, and where its views live. `tools/typeviews.py`
compiles every unit through the pinned cpp and cc1 with `-gstabs` and reads
the layouts cc1 itself computed: `--census` (per class: own methods, object
views, table views, with files), `--merge V...` (the union of named layouts,
`CONFLICT` where two disagree in size, signedness or a slot's return type),
`--tree`, `--globals`, `--warnings` (every unit's `-Wall` output against
`config/typeviews-warnings.txt`: a slot's PARAMETER types are not in stabs,
and a disagreeing call site is a new warning), `--upcast`.

**The class model.** `include/BasicClass.h`'s banner is the worked example;
`include/Class6D430.h` is the first class with subclasses. One header per
class at `include/<Class>.h` (top level: the Makefile's header list and
`rename.py` read `include/*.h` only) holding the object struct, the method
table struct, the table's extern and getter, and the class's own method
prototypes. A class WITH subclasses also defines `<CLASS>_FIELDS(Methods)` and
`<CLASS>_SLOTS(Self, CtorParams)`; each class expands its parent's macro
first, so accessors stay flat at any depth and nothing is re-pathed (GCC
2.6.3 has no anonymous struct members, and embedding the base would turn
Entity's accessors into `this->base.base.base.x`). The table word +0x000 is a
nibble-path class id (the parent of 0x1F234 is 0xF234, 0x234, 0x34, 0x4, 0x0),
a first guess at the tree: a ctor calls its parent's ctor first, and
`typeviews.py` moves a class whose ctor makes the SAME first call as its id
parent's up beside it (round 83: Tod and ModelData, id children of
TimBlockSrc, chain to the active data source as TimBlockSrc does, so they are
its siblings; `plan.py classes` marks such a class "parent by CTOR CHAIN").
Classes are unified root first: `plan.py`
offers a class only when its ancestors are unified, parked, `no C` or `no C
yet`. A `no C` class (no own method C, no view anywhere) is not a job; its
subclasses expand the nearest unified ancestor's macros and list its slots
flat. `no C yet` is the same while its methods are carved INCLUDE_ASM game
code (revision 18: every round-80 `no C` class was one); it becomes an
ordinary class when track 1 matches one.

**Procedure, one class per commit.**
1. `typeviews.py --census`: the class's own methods and every object and
   table view. Grep the table symbol, its getter and its method names too: a
   unit's local `extern` of any of them is a view.
2. Names: `<Class>` from the existing object view (drop an `Obj`/unit
   suffix) or the table's stem, `<Class>Methods`, `include/<Class>.h`. A
   name for what the class's own methods DO is allowed when its header
   banner states that evidence (Actor: translation setters, local-axis
   moves, link search; round 82); never one for what it is guessed to be.
3. Union: `--merge` the object views, then the table views. Every CONFLICT
   is settled by the accessors' bytes; a slot's return type is its
   occupant's unless a caller's bytes need otherwise (a void call cannot
   cross-jump with a value-returning one: `include/Entity.h` slotC4).
4. Boundary: the class's own fields run from its parent's size to its own,
   the size from an allocation (`New_<Class>`) or, with none, from any
   subclass's first own field. A field a subclass view names inside that
   range is this class's. Pad what is unknown; never move an offset.
5. **Check who calls a function before you trust the fields it touches.**
   Layout is measured; meaning is not. Class6D430's `unk40..unk74` were
   method-table SLOTS: the function copying them is passed tables (round 80).
6. Slots: expand the parent's SLOTS with this class's type and ctor
   parameter list, then its own slots named for their occupants
   (`classtable.py <table> --vs <parent>`). An inherited slot keeps the
   parent's name at every accessor (Pad's `onButtonEvent` was BasicClass's
   `notifyParents`). An override is named for its slot with `rename.py`
   (`Class__Finalize`, `Class__Release`) unless its report shows the body
   does more than the slot name says. An override whose parameter list differs
   from the slot's keeps the inherited slot type; the caller that forwards
   the extra arguments casts to a typedef of the override (no code), and
   the header comment names it (round 85: Class65650 +0x04C).
7. Write the header; delete every other view and local `extern` of the
   table, getter and methods; include the class header where needed. Fix
   what the compiler lists: accessor renames, base-table calls upcast
   (`typeviews.py --upcast <getter> <Base> <files>`; a pointer cast emits no
   code), slot renames. Retype the globals that hold this class's objects
   (assigned from `New_<Class>()`, or passed as a method's `self`): one
   consistent wrong type is invisible to `--globals` (round 85:
   gStyleDecorObj/gStyleDecorSlots held BoxFills as `s32`/`void *`).
8. After every step: `./build-and-verify.sh` byte-identical,
   `typeviews.py --warnings` 0 new (a GONE warning is progress: rewrite the
   baseline with `--baseline` in that commit and say so),
   `tools/check-nonmatching.sh` green.
9. A renamed function's report gets a dated `Track 4` paragraph with the
   evidence. `rename.py` rewrites the old name in prose too, including
   sentences it turns into nonsense: read the diff.

**Rules.** A type change to a field or slot is allowed when the oracle stays
green and the commit names the accessors that settle it. A subclass's own
views are the subclass's job: leave them compiling and report any
contradiction with the parent. Once a class is unified, CLAUDE.md's
independent-local-views convention ends for it: no unit declares its own
view again, and `plan.py classes` lists any that appears as a STRAY VIEW.

**Staffing.** Opus runners, one class each. A class merge touches every
unit that sees the class, so `plan.py` gives each ready class a measured
FOOTPRINT (units naming its table, getter, own methods or view types, plus
every unit including a header that does, plus the units calling any function
whose body calls the getter, i.e. its allocator; a unified ancestor's type is
not a view: revision 25, round 88) and lists every ready class, most
classes below it first; one whose footprint shares a unit with a class
above it is DEFERRED (a class renames only what its footprint holds, so the
call-graph test applies to other jobs only; revision 23: revision 20's
strict sequence, one runner at a time, was that test unmeasured). Tell each
runner the others' classes and to post before editing outside its footprint.
Prompt §4.6. Head
review before `python3 tools/plan.py mark-class --table <sym> --class
<Class>`: `plan.py classes` shows no stray view, the three oracles are green,
and three sampled slot or field names agree with their occupants or
accessors. **Park rule:** two views that disagree on a field's type at the
same offset, with both readings confirmed by their accessors, stay split
with a comment naming both (`mark-class --park "<reason>"`).

**Track 4b: one type per global.** `typeviews.py --globals` lists every
game global declared `extern` with more than one type. It leaves out a global
whose every accessor (read from the built objects' relocations) is library
code: that data is Sony's and track 2's (revision 21; the 0x34-byte table
this section once called the largest case is libsnd's `_svm_voice`). It also
leaves out a not-yet-unified class's own table, which is a view for that
class's job. Recipe, one global or one family per commit (round 85:
`include/CdDriver.h`, `gMcDevicePath0/1`, `D_8008ACA4..AC`):
1. Read every accessor: the type is what the accessors need (walked at a
   record stride, dereferenced for a field, passed as a pointer), never the
   widest declaration.
2. Home: the header that already owns the global's type, or a new
   `include/<Subsystem>.h` for a family several units share, holding the
   record types and value sets they need. A class is referred to there by
   tag only (`struct Class6D4E8 *`). With no owning header both units
   include, each declares the same type and names the other.
3. Delete every other declaration, fix the accessors (C warns, not errs, on
   pointer/integer mixes, so `typeviews.py --warnings` is the accessor list),
   and run the three oracles. A report gets a dated `Track 4b` paragraph.
A job is ready when `plan.py` lists it; runners take it with §4.6's prompt,
"one class" read as "one global family".

### Track 5: close-out

Opens when tracks 3 and 4 are done. Items, ticked with `plan.py check --item`:

| item | done when |
| --- | --- |
| `readme` | a reader-facing README.md: what the game's code is, how it is organised (classes, subsystems, units), how to build, where the SDK comes from |
| `credits` | CREDITS.md names every inherited name, tool and reference |
| `asm-sites` | every live `__asm__` justified at the site or retired: each bare barrier says what order it forces, each GTE block names its macro (`TransformAndCullPoly` is C since 2026-09-14) |
| `docs-budget` | every doc within its `plan.py` budget |
| `nonmatching-clean` | `tools/check-nonmatching.sh` green; every stall has a `NON_MATCHING` body or a written reason |

An item already true when measured is ticked by the head with the
measurement in PROGRESS.md. The rest are Opus runner jobs, one item each,
prompt §4.7; the head reviews the diff and ticks with `plan.py check --item`.
README and CREDITS jobs edit only their file; asm-sites edits comments at
the sites and retires a barrier only when the oracle stays green without it.


## Prompts of tracks 3 to 5 (revision 26 numbering)

### 4.2 Naming runner prompt (track 3; head fills in `<>`)

> You are a naming runner for the LSD: Dream Emulator (PSX) decomp, round
> `<N>`. Work ONLY in worktree `<path>` on branch `runner/<name>`, on
> `src/<unit>.c`, the header(s) it owns `<list>`, and the match reports of its
> functions. Read CLAUDE.md, `docs/FINISHING-PLAN.md` §3 track 3 and
> `docs/PARALLEL-RUNS.md` §2 and §5 first.
>
> Do the pass in §3 track 3 in order, for every function in the unit including
> `NON_MATCHING` bodies. Use `python3 tools/rename.py OLD NEW` for every
> function and global rename; never rename by hand. Fields and slots follow
> step 3's ownership rule: rename what only your unit accesses; PROPOSE the
> rest in the report under `## Proposed field names` and on the broadcast, and
> leave the header alone. Read the broadcast before each function. After every
> rename and every header edit:
>
> ```sh
> ./build-and-verify.sh > /tmp/<name>_b.log 2>&1; echo "build exit=$?"; \
> grep -nE 'error:|parse error|undefined reference|\*\*\* \[[^]]*\.o\]' /tmp/<name>_b.log | head -8
> tools/check-nonmatching.sh   # after a FIELD or SLOT rename: the accessors the default build cannot see
> ```
>
> Exit 0, no hits, and check-nonmatching green, or the rename is wrong; revert it.
>
> Naming rules: name what the code does, not what you guess it is for; tiers
> A/B/C recorded per name in the report's `## Naming` section with the
> evidence; conventions as §3 track 3 lists them; inherited names are
> hypotheses to confirm or replace; Sony symbols are never renamed; a wrong
> tier-A name is worse than `func_`. Before calling a body unnameable, grep
> `src/` for its placeholder-named callees: one matched and named elsewhere
> often names the caller (round 66). When unsure, keep `func_` and write down
> what you know.
>
> Commit per logical step: one commit per `rename.py` run or per batch of
> renames in this unit, one commit per header edit, one for the unit header
> comment. `git status --porcelain` empty when you report. Never push, never
> edit shared docs or the splat yaml, never edit the symbols file except
> through `rename.py`.
>
> Final summary: a table of every function in the unit: old name, new name,
> tier, one-line evidence; the header comment you wrote; every header edit
> you made and every field name you PROPOSED for the head to apply; anything
> you could not name and why.

### 4.6 Types runner prompt (track 4; Opus; head fills in `<>`)

> You are a types runner for the LSD: Dream Emulator (PSX) decomp, round
> `<N>`. Work ONLY in worktree `<path>` on branch `runner/<name>`. Your job is
> ONE class: `<table symbol>`, as `plan.py`'s job line names it. Read CLAUDE.md,
> `docs/FINISHING-PLAN.md` §3 track 4, `include/BasicClass.h`'s banner and
> `include/Class6D430.h` (the worked example of a class with subclasses), then
> do track 4's procedure for your class, in order. The edit is non-local: you
> may edit every file `python3 tools/typeviews.py --census` lists for your
> class, the units whose accessors the compiler then flags, and their match
> reports; you may not unify, rename or retype any other class. After every
> edit:
>
> ```sh
> ./build-and-verify.sh > /tmp/<name>_b.log 2>&1; echo "build exit=$?"; \
> grep -nE 'error:|parse error|undefined reference|has no member|conflicting|\*\*\* \[[^]]*\.o\]' /tmp/<name>_b.log | head -8
> python3 tools/typeviews.py --warnings | tail -3
> tools/check-nonmatching.sh
> ```
>
> `OK: build matches retail`, 0 new warnings and nonmatching green, or undo
> the last edit. Functions and globals are renamed only with
> `python3 tools/rename.py OLD NEW`; types, fields and slots by hand, the
> compiler listing the accessors. Before you name or retype a field, read the
> CALLERS of the functions that touch it (step 5). Commit per step (renames,
> the new header, each view replaced), with the message in a file
> (`git commit -F`): backticks in a double-quoted `-m` run as commands.
> `git status --porcelain` empty when you report; never push; never edit
> shared docs, the ledger, or the symbols file except through `rename.py`.
>
> Final summary: the header you wrote; every slot and field you named or
> renamed with the occupant or accessor that shows it; every view and local
> `extern` you deleted; every function you renamed and why; every `--merge`
> CONFLICT and how the bytes settled it; anything parked or unsettled; any
> contradiction you saw in a subclass's views.

