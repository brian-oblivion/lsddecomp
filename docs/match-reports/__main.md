# __main (round 79)

> Renamed from `func_80011994` on 2026-09-25 (tools/rename.py). Address 0x80011994.

**Unit:** main · **Size:** 2 instructions · **Status:** MATCHED (part of the
already-matched `main` unit; this function's own body is `jr $ra; nop`)

## What it does

An empty function (`void __main(void) {}`), called as literally the
first statement inside `GameMain` (`src/main.c`, formerly `func_800118DC`,
see `docs/match-reports/main.md`). It reads and writes nothing.

## Naming (round 79): NOT renamed -- likely Sony's/the toolchain's own `__main`

**No game name given.** Two independent pieces of evidence point at this
being GCC 2.6.3's own ritual `__main` stub (called from a `main`-named
function to run static-constructor tables) rather than authored game code:

1. **Corpus fingerprint (weak on its own).** `tools/sdkname.py __main`
   returns only a TINY (2-word) EXACT match, shared by a dozen unrelated
   library functions across libgpu, libsnd, libcd and the GTE macros, with
   no disambiguating position evidence -- both functions in this unit get
   the same generic "before `_obj/malloc`" note. This is exactly the
   shape `docs/PROGRESS.md` round 78 already swept for, project-wide, and
   found nothing among 2-word stubs ("Adjacency leads ... find all five of
   round 78's hand finds plus SsStart/SsStart2, and no 2-word stub"). So
   this fingerprint ALONE is not identification, per the runner brief's own
   screen, and is not sufficient by itself.

2. **Build-level signal (new this round, independent of the corpus).**
   Renaming the caller (`func_800118DC`) to literally `main` -- otherwise a
   well-evidenced, tier-A name for that function -- compiles clean but
   fails to LINK: `undefined reference to '__main'`. GCC 2.6.3
   unconditionally inserts a `jal __main` as the first compiled statement
   of any function spelled exactly `main` (`expand_main_function`, gated
   only by `-ffreestanding`, which this project's pinned flags do not
   pass). That auto-inserted call would land at exactly the position this
   function already occupies -- the very first call inside `main`.
   Reproducer:

   ```sh
   # in a scratch worktree
   python3 tools/rename.py func_800118DC main
   # compiles; ld fails:
   #   undefined reference to `__main'
   ```

   The parsimonious reading: retail's own source named the entry function
   `main` and never wrote an explicit call to this stub at all -- GCC
   inserted it. This project's C reproduces the identical bytes with an
   explicit call instead (because renaming to literal `main` here would
   ALSO insert a second, redundant `__main` call on top of the explicit
   one, which is what actually broke the link) -- consistent with, not
   contrary to, `func_80011994` being the definition that resolved `__main`
   in retail's own link.

**Per CLAUDE.md's Sony-ownership rule** ("Never write C for a function a
Sony object owns" / "give no game name to anything Sony owns") and the
runner brief, this function is left as `__main`, unrenamed, and is
NOT counted toward this unit's naming debt. Reclassifying it (moving it out
of `src/main.c`, adding it to `config/sdk-in-game.txt`, or literally
spelling it `__main` and restructuring `main` to rely on the compiler's
own auto-insertion) is a bigger structural change than a naming pass should
make unilaterally and is left for the head/operator to decide. Posted to
`tools/broadcast.sh`.

## Proposed learning

See `docs/match-reports/main.md`'s `### Proposed learning`: a
`main`-rename experiment that demands an undefined `__main` at a specific
call site is independent, build-level evidence that whatever already
occupies that call site is that symbol -- useful beyond this one function
wherever a tiny, ambiguous corpus fingerprint sits at the very start of a
plausible game `main`.

## Head note, round 79: the entry point is `main`, the stub is `__main`

Alpha found that spelling the entry point `main` makes cc1 insert
`jal __main`, which failed to link. The head measured the conclusion in a
worktree. With `func_80011994` renamed `__main`, `GameMain` renamed `main`,
and `main`'s explicit first-statement call to the stub DELETED, the image is
byte-identical: retail's first call is the one cc1 inserts itself, so the
original source spelled it `main` and never wrote the call. `__main` is
Sony's empty `_obj/none` stub: an EXACT (TINY) fingerprint, placed directly
before the `_obj/malloc` object, and the call only cc1 emits. Both oracles
green. `GameMain` is withdrawn: the name was a workaround for the link
failure, not a claim about the code.

## History (moved from src/main.c, round 101)

The comment above the definition, verbatim as it stood at `57fb90b6`:

```c
/* Sony's _obj/none (round 79); the call to it is cc1's, inside main. */
```
