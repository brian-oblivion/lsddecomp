# SDK-object conversion runs — who does what, and the prompts to paste

Companion to `docs/PARALLEL-RUNS.md` for a different kind of round: replacing
the Psy-Q library **disassembly** with Sony's own **linked objects**. Read
this whole file before pasting either prompt. The recipe a runner follows is
`docs/SDK-OBJECTS-GUIDE.md`; the measurements behind it are
`docs/research/psyq-sdk-objects.md`.

## What this work is, in one paragraph

It is not decompilation and it is not matching. The SDK discs ship the
libraries as object files **with their symbol tables**, and a placed object's
bytes already equal retail's, so nothing is written in C. A conversion edits
configuration only: the splat yaml (an `o` segment per object, plus lines
for its data sections), the manifest `config/psyq-objects.txt`, the generated
fragment `config/psyq-objects.ld`, the symbols file, and the `func_XXXXXXXX`
callers in `src/`/`include/` that take Sony's names. In that sense it is a
carve — the same kind of edit as Gate 2 — whose payload is a prebuilt object
instead of a C stub, and whose reward is every SDK function and variable
getting its real name. The oracle is unchanged: `./build-and-verify.sh` must
still say the image matches retail after every step.

## Who does what

| | head — **Fable** | runner — **Opus** |
| --- | --- | --- |
| queue | derives it (`psyq_sdk.py runs`), decides order, assigns segments | works the assigned segment's runs in address order |
| pure `psyq_*` runs | — | **all of them**: text-only and data-bearing alike, per the guide |
| runs inside GAME units (`c` segments) | splits the C unit around the object (Gate 2 carve), or assigns it to a runner who owns that unit anyway | only when the head assigned that unit |
| `PARTIAL OVERLAP` in `runs` output | decides which object is real (the segment's glabels) | reports it, skips the run |
| `relocations DISAGREE` from `place` | decides (edit the object, or leave the run as asm) | reports, skips |
| `NOTE:` lines from `ldfrag` (an extern resolved at two addresses) | decides | reports, skips |
| a symbol name already taken in the symbols file, or a `SUSPECT` placement in `runs` (a masked-call body that placed on the wrong function; `libsnd/ssinit_c`) | decides the name / rejects the placement | reports, uses `func_*` meanwhile, skips the run |
| merge, regenerate `config/psyq-objects.ld` after merge, verify `main`, update `docs/PROGRESS.md` and the research doc | yes | — |

Everything in the runner column is mechanical once the head has handed over a
segment; everything in the head column is a decision that changes what other
work sees.

## Collision rules for THIS kind of round

The files this work edits are **shared by every conversion**, which is the
opposite of a matching round's one-unit-per-runner partition:

- `config/splat.slps01556.lsdde.yaml` — edited in the text list, the rodata
  slots, the data/sdata slots. Two runners on two *different* `psyq_*`
  segments edit different hunks; git merges them. Two runners on the same
  segment conflict on every commit.
- `config/psyq-objects.txt` — every runner appends. Trivial conflicts; the
  head resolves by keeping both blocks.
- `config/psyq-objects.ld` — GENERATED. Never hand-merge it: after merging,
  the head runs `psyq_sdk.py ldfrag` and commits the result.
- `config/symbols.slps01556.lsdde.txt` — additive lines; merges cleanly if
  each runner inserts its block in address order rather than at the end.
- `src/`, `include/` — only `func_XXXXXXXX -> SonyName` renames of callers,
  which may touch ANY unit. This is the one place this work collides with a
  concurrent matching round: a matching runner's unit may get a rename under
  it. Harmless on merge (a rename is a one-word hunk) but the matching runner
  must not be told the function is theirs.

So: **at most one runner per `psyq_*` segment, at most two segments in flight,
and no matching runner on a unit whose SDK objects are being carved out that
round.** Pure `psyq_*` conversions can run alongside a matching round; the
game-unit splits cannot share the unit with a matching runner.

Every run is one commit on the runner's branch, made right after the oracle
passes — a conversion that is half-applied is a broken build for whoever
merges next.

## Order, and how long

Measured queue (`tools/psyq_sdk.py runs`, 2026-09-11, after round 29's
containment rule dropped the 25 finer-grained 3.5/3.6 pieces): 165 objects
in 44 runs (alternates with identical bytes counted once), no overlaps.
110 objects / 23 runs are in pure `psyq_*` segments (`psyq_2258`,
`psyq_GsLinkObject4`, `psyq_SpuSetMute`, `psyq_memset`, `psyq_rand`);
55 objects / 21 runs sit inside game units and need the unit split. Re-derive
these with `runs` rather than trusting them. After round 30 the queue is
58 objects / 24 runs: the three `gs_00x` runs and the game-unit objects.

The two blocks already converted took one iteration (text-only) and two
(data + bss) once the tooling existed. A runner that has the guide open should
manage a data-bearing run in well under half an hour and a text-only one in a
few minutes, so **ten to fifteen runs per runner session** is a fair planning
figure:

| phase | what | who | rounds |
| --- | --- | --- | --- |
| 1 | the pure `psyq_*` segments — **DONE 2026-09-11 (rounds 29–30)** except `libgs/gs_001 gs_002 gs_003`, held for the operator | four Opus runners, one at a time | 2 |
| 2 | the 60-odd objects inside game units | head (Fable) carving, or a runner who is assigned that unit for the round | 3–4, interleaved with matching rounds |
| 3 | naming pass for the 426 functions no disc places (name from the shape-matched module, bytes stay asm) | one cheap runner with `psyq_sdk.py symbols` | 1 |

**Do phase 1 before the next big matching round**, or alongside a small one.
It is independent of the game code, it touches no unit a matching runner
needs, and it retires names and stalls that game work would otherwise keep
tripping over. Phase 2 is best folded into the head's normal consolidation
carving, one or two units per round, because that is when a unit has no
owner. Phase 3 is whenever — it only adds names.

## Runner prompt (head fills in `<>`; paste to an Opus agent)

> You are an SDK-object conversion runner for the LSD: Dream Emulator (PSX)
> decomp. Work ONLY in the worktree `<path>` (cd there first) on branch
> `runner/<name>`. Read CLAUDE.md, then docs/SDK-OBJECTS-GUIDE.md end to end,
> then docs/research/psyq-sdk-objects.md. Your assignment is the `psyq_*`
> segment `<segment>` (`0x<start>..0x<end>` in file offsets) and nothing else.
>
> **What you are doing.** Replacing Sony library disassembly with Sony's own
> objects from the SDK discs, one contiguous run of placed objects at a time,
> so the build links the objects exactly where the game did. You write no C.
> You edit `config/splat.slps01556.lsdde.yaml`, append to
> `config/psyq-objects.txt`, regenerate `config/psyq-objects.ld` with
> `.venv/bin/python3 tools/psyq_sdk.py ldfrag`, add Sony's names to
> `config/symbols.slps01556.lsdde.txt` (inserted in address order, not at the
> end), and rename `func_XXXXXXXX` callers in `src/` and `include/` to those
> names. Never touch `asm/` by hand except to DELETE the stale top-level
> `asm/<segment>.s` after `make extract`, which the guide tells you to do.
>
> **The queue.** `.venv/bin/python3 tools/psyq_sdk.py runs` and read only your
> segment's block. Work its runs in address order. A run whose objects tile
> their range exactly (each `fileoff + text` equals the next `fileoff`) is
> yours to convert. Entries written `a|b` are two objects with identical bytes:
> link either, prefer the 3.3 disc. A `PARTIAL OVERLAP` line means one of the
> two placements is a false positive — **do not guess; skip the run and put it
> in your report.** A `SUSPECT` line means the object placed on the wrong
> function (its calls resolve away from another object's definition): skip
> that run too, and report it. The footer lists finer-grained placements
> `runs` dropped in favour of a coarser object; link the coarse one, never the
> pieces. Functions of the segment that no run covers stay as asm:
> keep a smaller `asm` segment for them under a new `psyq_*` name and do NOT
> try to match them as C. Convert a run of more than about ten objects in
> chunks at object boundaries, one commit each, with the unconverted tail as
> a temporary `psyq_<fileoff>` asm segment.
>
> **Per run, in this order — the guide has the details and the exact
> commands.** (1) `psyq_sdk.py place <objects>` for the data sections; take
> the lines whose byte search agrees, stop on `relocations DISAGREE`.
> (2) append the manifest lines, `psyq_sdk.py install`. (3) yaml: text `o`
> lines, rodata/data/sdata slot splits; never place `.bss`/`.sbss`. Do the
> `start + size` arithmetic for every data section first: the script's
> `SUBALIGN(2)` means a 2- or 3-byte hole before the next object needs a
> `- [0xOFF, pad]` line (guide, step 4) or the next section lands early.
> (4) `psyq_sdk.py ldfrag`. (5) symbol names from `psyq_sdk.py symbols`
> (`:def` lines in your range) into the symbols file; rename callers
> (`grep -rn func_XXXXXXXX src include`). Weak duplicates such as `memclr`
> keep their `func_*` name. (6) `make extract`, delete the stale
> `asm/<old segment>.s`, then the oracle:
> `./build-and-verify.sh > /tmp/<name>_b.log 2>&1; echo "build exit=$?"; grep -nE 'undefined reference|\*\*\* \[[^]]*\.o\]|multiple definition' /tmp/<name>_b.log | head; tail -1 /tmp/<name>_b.log`
> The log path MUST carry your runner name; `/tmp` is shared between
> worktrees. (7) `.venv/bin/python3 tools/psyq_sdk.py check`. (8) `git commit`
> — one commit per run, only after `OK: build matches retail`.
>
> **If the oracle fails** read the linker's message before anything else: an
> `undefined reference` from a `lib/` object means the fragment is stale or
> an extern is unresolvable (the `ldfrag` output will carry a `NOTE:`); an
> SHA1 mismatch with a clean link means a data section landed in the wrong
> place — re-check `place`'s byte-search offsets against the yaml, and check
> that you split the enclosing data slot at exactly the object boundaries.
> Never edit `check.sha1`, `build.sha1`, or anything under `tools/gcc263`,
> `tools/binutils`, `tools/maspsx`. If a run does not verify after a
> reasonable effort, `git checkout` your changes for that run, record it in
> the report, and move on to the next run.
>
> **Hard rules you inherit from CLAUDE.md**: nothing under `sdk/` or `lib/`
> is ever committed; `build-and-verify.sh` is the only build; `asm/` is
> generated.
>
> **Your final message is a report**, in this shape: the runs you converted
> (range, objects, commit hash each); the runs you skipped and WHY, quoting
> the `PARTIAL OVERLAP` / `DISAGREE` / `NOTE:` line verbatim; any symbol name
> you could not apply because the name was taken or the game seems to
> override it (say which); and the `psyq_sdk.py runs` block for your segment
> as it stands at the end. Leave your worktree with `git status --porcelain`
> empty.

## Head prompt (operator pastes this to Fable)

> Act as the head for an SDK-object conversion round per
> docs/SDK-OBJECTS-RUNS.md. First measure: `python3 tools/progress.py`,
> `.venv/bin/python3 tools/psyq_sdk.py runs`, `psyq_sdk.py check`. Decide which
> `psyq_*` segments go to runners this round (at most two, disjoint) and which
> game-unit splits, if any, you will carve yourself during consolidation.
> Provision one worktree per runner (`tools/setup-worktree.sh <name>`), fill
> in and paste the runner prompt from the RUNS doc, run them in the background.
> While they run, resolve any `PARTIAL OVERLAP`, `DISAGREE` or `NOTE:` items
> already known from `runs`/`ldfrag` output and, if scheduled, carve one
> game-unit split per the guide's last section. As runners finish: merge each
> branch with `--no-ff`, confirm no `MERGE_HEAD` remains, run
> `psyq_sdk.py ldfrag` and commit the regenerated fragment if it changed, run
> `./build-and-verify.sh` and `psyq_sdk.py check`, then `python3
> tools/progress.py`. Consolidate: append the round to `docs/PROGRESS.md`,
> add any new finding to `docs/research/psyq-sdk-objects.md` and any new
> pitfall to `docs/SDK-OBJECTS-GUIDE.md`, tear down the worktrees, and report
> what converted, what is blocked on a decision only the operator can make,
> and whether the next round should be more conversion, game-unit splits, or
> a matching round.
