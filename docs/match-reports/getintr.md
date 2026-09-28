# getintr -- MATCHED (337/337, byte-exact, round 70)

> Renamed from `func_80029478` on 2026-09-23 (tools/rename.py). Address 0x80029478.

REVISITED, round 70: MATCHED (337/337, whole-image SHA1 green); names/types not relevant.

Unit: `src/psyq/libcd_bios.c`. 337 words. Owns `jtbl_800109F8` (rodata sub-slot
`[0x11F8, .rodata, libcd_bios]`), which the `switch` lowering generates and
which matches with no hand-authoring.

**Provenance.** This is libcd's `bios.c` `getintr` (build 1.71, December
1995 -- the libcd build that is on no SDK disc, per
`docs/research/psyq-sdk-objects.md`, so it cannot be linked as an object).
The strings (`"CDROM: unknown intr"`, `"DiskError: "`,
`"com=%s,code=(%02x:%02x)\n"`) are the libcd ones. It was compiled by the same
GCC 2.6.3, so it is ordinary C matching work; `sdkstalls.py` does not flag it
because no placed object covers it.

## Round 70: the inherited body, rebuilt as given

The round-21 title said "219/337, register-identity residue, size matches
retail exactly". Rebuilt verbatim (plus the two externs it used but did not
declare, `CD_debug` and `CD_comstr`), it measured:

- **159/337 raw, `.text` 0x54c (2 words LONG)**, whole image drifted;
- `insertions 71 / deletions 71`, positional skeleton diffs 141.

So neither the length claim nor the register-identity verdict held. The
residue decomposed into five separate defects, fixed in this order
(builds in brackets, each score from a fresh build):

| # | defect (asm-differ) | fix | result |
| --- | --- | --- | --- |
| 1 | `bnez` where mine had `beqz` after `CD_status & 0x10` | **logic**: `!(CD_status & 0x10) && (resp[0] & 0x10)` | -- |
| 2 | cases 1/2: `li v0,2`/`li v0,5` swapped around the `beqz s0` | **logic**: retail stores 5 when `flags != 0` (`flags ? 5 : 2`, `flags ? 5 : 1`). Case 2 still needs the if/else-into-a-local form; the ternary unfolds its store | ins/del 71/71, 0x54c |
| 3 | cases 4/5: mirror store folded (`$at`) where retail unfolds (`lui/addiu/sb 0`) | `*(volatile u8 *)&D_8006D8D9 = D_8006D8DA;` and `*(volatile u8 *)D_8006D8D8 = D_8006D8D9;` (same cast case 3 already used) | 67/67, still 0x54c |
| 4 | cases 4/5: copy's src pointer computed after the null check, a `nop` in the `beqz` delay slot | initialise the copy's `src` BEFORE `if (dst != NULL)` | **0x544 exact**, 253/337, 27/27 |
| 5 | all 10 copy sites: dst and counter registers swapped (`$a0`/`$v1`), src right | **the copy as a `static __inline__` function** instead of a `do{}while(0)` macro | **333/337, 0/0** |
| 6 | one missing `andi v0,v0,0xff` after `lbu v0,0x18(sp)`; mine loaded `resp[0]` twice instead | `CD_status = *(volatile u8 *)&resp[0]; CD_status1 = resp[1]; flags = CD_status & 0x1D;` | **337/337, OK: build matches retail** |

About 30 builds in all. No permuter search was spent; Gate 3 was never
reached.

### What did not work (defect 6, measured, all with the inline copy in place)

- `flags = CD_status & 0x1D` alone: one load, but no `andi 0xff` (0x540, one short).
- A `u8 st = resp[0]` local used for both the store and `flags`, at function
  scope, block scope, before the counter test and before the whole `if`;
  `flags` as `u8` as well as `s32`; reordering the three statements; `CD_status
  = st = resp[0]`; `flags = (st = resp[0]) & 0x1D`: all either one short
  (combine folds the zero-extension into the `lbu`) or worse.
- `flags = (u8)CD_status & 0x1D` or `CD_status = resp[0] & 0xFF`: these reload
  the global's low byte (`lbu` of `CD_status`), 2 words long.
- The whole `resp` array `volatile`: 1 word long, and every `resp[1]` read gets
  masked too.
- `volatile` read of `resp[0]` with `flags = resp[0] & 0x1D` (not from the
  global): 333, because `flags` then reloads `resp[0]`.

Why the volatile read works: a volatile `mem:QI` cannot be merged into a
`zero_extend` load by combine, so the value stays a QImode register and its
widening shows up as a separate `andi 0xff`. That is exactly how this same
function's `volatile u8 cause` compares compile (`lbu; andi 0xff; bne`).
`flags` then comes from CSE of the value just stored. Whether Sony wrote a
volatile access there (say through a `volatile` result pointer) or something
else that lowers the same way cannot be recovered from the bytes.

## What it is (fully derived, high confidence)

A PSX CD-ROM controller interrupt-cause dispatcher. Confirmed via the
debug/log strings it references (all verified against `asm/data/FD8.rodata.s`
before use, per the project's string-literal rule):

- `D_800109D8` = `"CDROM: unknown intr"` -- the default-case message.
- `D_800109B0` = `"DiskError: "`, `D_800109BC` = `"com=%s,code=(%02x:%02x)\n"`
  -- the cause==5 (error) diagnostic, printing a command-name string looked
  up from `CD_comstr` (the same string table `func_80028CF8`, matched
  earlier this round, indexes) plus two raw status bytes.
- `D_800109EC` = `"(%d)\n"` -- appended to the unknown-cause message.

Control flow, fully reconstructed and verified structurally byte-exact
(every branch, loop, and case boundary lines up 1:1 with retail -- the
residue described below is a pure *register-identity* difference, not a
missed condition or wrong constant anywhere):

1. Write 1 to a command/latch port (`D_8006D8C0`, a `volatile u8 *`), read
   the status/cause port (`D_8006D8CC`, masked to 3 bits) with a debounce
   loop (read until two consecutive reads agree), returning 0 immediately
   if the debounced cause is 0.
2. Read up to 8 response bytes from a data port (`D_8006D8C4`) while a
   "data ready" flag (bit `0x20`) is set on `D_8006D8C0`, zero-filling the
   rest of an 8-byte stack buffer if fewer than 8 arrived.
3. Re-arm the ports (write 1/7/7 to `D_8006D8C0`/`CC`/`C8`).
4. Unless cause==3 with a false `D_8006D7C0[CD_com]` lookup (a per-mode
   flag table, same selector family as `CD_comstr`/`D_6006D6A0`), update
   an error counter (`CD_nopen`) when a flag bit turns on across the
   read, latch the two response bytes into `CD_status`/`CD_status1`, and
   compute a `flags` value (`resp[0] & 0x1D`) used by cases 1-3 below.
   (Round 70: the counter increments when bit 0x10 turns ON, i.e. the old
   `CD_status` bit is CLEAR and the new `resp[0]` bit is set.)
5. On cause==5, log the diagnostic strings above.
6. Dispatch on cause (1-5, via `jtbl_800109F8`; 6/7/out-of-range and the
   post-mask 0 case fall to a "CDROM: unknown intr (%d)\n" default),
   writing a small status byte (`D_8006D8D8[0]` and/or `D_8006D8D9`,
   depending on the case and mirrored between them in cases 4/5) and
   copying the 8-byte response into one or two of three contiguous 8-byte
   mailboxes (`Result`, `D_8008B3D4`, `D_8008B3DC`), returning a small
   bit-flag-shaped result (0, 1, 2, 4, 6, or -1) -- confirmed against
   caller sites in `asm/code_179d8_mid.s`, which `andi` the result against
   `0x2` and `0x4`, consistent with a flag word.

`s32 getintr(void)` -- confirmed against three call sites
(`asm/code_179d8_mid.s`, `asm/nonmatchings/libcd_bios/callback.s`,
`asm/nonmatchings/libcd_bios/CD_readsync.s`), all `jal` with a `nop`
delay slot and no argument setup, and against `libcd_bios.c`'s own
existing forward declaration (`extern s32 getintr(void);`).


## The source as matched

It is live in `src/psyq/libcd_bios.c`. The load-bearing shapes:

```c
static __inline__ void copy8(u8 *d, const u8 *s)
{
    s32 i;
    if (d != NULL) {
        for (i = 7; i != -1; i--) {
            *d++ = *s++;
        }
    }
}
/* ... */
        CD_status = *(volatile u8 *)&resp[0];
        CD_status1 = resp[1];
        flags = CD_status & 0x1D;
/* ... */
    case 4:
        D_8006D8DA = 4;
        *(volatile u8 *)&D_8006D8D9 = D_8006D8DA;
        copy8(D_8008B3DC, resp);
        copy8(D_8008B3D4, resp);
        return 4;
```

Other round-21 findings still hold: cases are written in retail's PHYSICAL
order (3, 2, 1, 4, 5, default), the copy counter is `for (i = 7; i != -1; i--)`,
the four strings are rodata symbols, `D_8006D8D9`/`D_8006D8DA` are
`volatile`-declared so the mirror store reloads, and case 3's stores are
`*(volatile u8 *)D_8006D8D8 = N` (unfolded) while case 2's is a plain folded
store of an if/else-merged local.

### Proposed learning

- **A repeated null-guarded copy that is register-identity as a macro can be
  byte-exact as a `static __inline__` function.** The two emit the same
  instructions. Only which register holds dst and which holds the counter
  differs, at all 10 sites. Inline parameters are fresh pseudos assigned at
  the call, which changes allocation order. This is a source-shape lever for
  3d ("any new name is a new allocno"). Try it whenever a block repeated
  N times has the same register permutation at every copy. SDK-derived code
  (libcd here) is a likely place for real inline helpers. Not a register pin:
  no register is named anywhere.
- **Re-read branch polarity before accepting a "register identity" title.**
  Two of this function's round-21 defects were inverted logic: a flipped
  `if` test and swapped ternary arms. Both showed up as "`li v0,2`/`li v0,5`
  swapped" and "`bnez` vs `beqz`" in asm-differ. The report attributed them to
  allocation because it read the diff before the length was right and never
  after. Round 70's first `insertions 71 / deletions 71` read was enough to
  reject the verdict.
- **A missing `andi rX,rX,0xff` right after an `lbu` is a volatile (QImode)
  read that combine could not fold.** The discriminator: the same function's
  volatile `u8` locals compile the same way.

## Anomalous

None. The round-21 report's `extern u8 CD_status` conflict note is moot
(the unit holds only this function).

## File history

Round 90 (track 8) merged the carve units code_179d8_b (this function),
code_179d8_n (CD_sync, CD_ready, CD_cw) and code_179d8_g (CD_vol .. cb_read)
into `src/libcd_bios.c`, on `tools/tuboundary.py`'s rodata proof that they
were one file. The three unit banners are moved here verbatim, as they stood
before the merge (unit names in them are the carve names).

### code_179d8_b banner

```c
/*
 * ROUND 42 CORRECTION (2026-09-15) -- READ BEFORE ANY "BLOCKED" LINE BELOW:
 * every claim in this comment that a function is BLOCKED by `gp_rel`,
 * `nop_mflo_mfhi` or `addiu_at` is STALE.  All three constructs are RESOLVED
 * by pinned maspsx flags (CLAUDE.md, "Open toolchain blockers");
 * `tools/nearmiss.py` reports them tagged (RESOLVED-not-a-blocker) and counts
 * none of them.  Any "do NOT spend attempts on these" directive below is
 * therefore RETRACTED: those functions are ordinary matching work, and most
 * carry a mechanism-correct partial derivation already.  The rest of this
 * comment still stands -- only the blocker verdicts are withdrawn.
 * Screen: `python3 tools/nearmiss.py`, round 43 (2026-09-15).
 *
 * code_179d8_b -- window [60..79] of the original 274-function code_179d8
 * monolith, originally 0x194E0..0x1A1BC (vram 0x80028CE0..0x800299BC).
 *
 * ROUND 34 (head): NINETEEN of the twenty functions left this unit.  Everything
 * from func_80028CE0 (CdSetDebug) through func_800293F8 (CdPosToInt) is
 * `libcd/sys.o` (Psy-Q 3.3), which starts four functions earlier in
 * code_179d8_h and is now linked from the object.  Sixteen of them had been
 * matched as C and three -- CdControl (func_80028DF0), CdControlF
 * (func_80028F38), CdControlB (func_80029074) -- were INCLUDE_ASM stalls with
 * about 1200 lines of derivation between them that could never have closed.
 * The C is gone because Sony's object owns those bytes now (CLAUDE.md: never
 * write C for a function a Sony object owns); the reports are kept, retitled
 * CONVERTED.  The unit is now 0x19C78..0x1A1BC and holds ONE function,
 * getintr, which still owns jtbl_800109F8 and so the 0x11F8 rodata
 * attach.  The "low-level serial/link driver" reading below was written
 * about the whole window and is now mostly a reading of libcd itself.
 *
 * Carved round 16 by blocker DENSITY, not by "next": code_179d8 is 44%
 * blocked in aggregate but the blockers CLUSTER, so the aggregate says
 * nothing about any particular window. This one screened 16/20 clean.
 * NONE OF THE THREE "BLOCKED" FUNCTIONS IS BLOCKED ANY MORE.  All three
 * were blocked on `addiu_at` ALONE, and `addiu_at` was RESOLVED in round 21
 * (maspsx `--addiu-at`; docs/research/addiu-at-blocker.md).  Re-screened
 * with `python3 tools/nearmiss.py` on 2026-09-08 (round 24):
 *   func_80028CF8  MATCHED    func_80028D30  MATCHED
 *   getintr  MATCHED round 70 (docs/match-reports/getintr.md)
 * The previous version of this comment said all three "are already stubbed
 * as match reports", which by round 24 was a stale DIRECTIVE over free
 * ground; their stubs are gone.
 * func_800292F4 was misclassified nop_mflo_mfhi by an inverted screen
 * (round 16 head correction) -- it is fresh ground, not blocked. It
 * contains mult->mfhi (the hazard-slot direction, not a blocker), retail's
 * signed-divide-by-constant idiom.
 *
 * getintr owns jtbl_800109F8, whose sub-slot of the 0xFD8 rodata
 * region is ATTACHED to this unit in the splat yaml. Leave that alone.
 *
 * This slice was cut at ROM-address boundaries, so it has no reason to
 * align with class boundaries -- expect it to span more than one class,
 * and identify each with tools/classtable.py rather than assuming one.
 *
 * Declarations: keep anything that encodes THIS unit's reading of a class
 * next to the code, in this file. Do not create a shared code_179d8*.h --
 * the sibling slices are staffed independently and a shared header is what
 * makes their merges collide.
 */
```

### code_179d8_n banner

```c
/*
 * ROUND 42 CORRECTION (2026-09-15) -- READ BEFORE ANY "BLOCKED" LINE BELOW:
 * every claim in this comment that a function is BLOCKED by `gp_rel`,
 * `nop_mflo_mfhi` or `addiu_at` is STALE.  All three constructs are RESOLVED
 * by pinned maspsx flags (CLAUDE.md, "Open toolchain blockers");
 * `tools/nearmiss.py` reports them tagged (RESOLVED-not-a-blocker) and counts
 * none of them.  Any "do NOT spend attempts on these" directive below is
 * therefore RETRACTED: those functions are ordinary matching work, and most
 * carry a mechanism-correct partial derivation already.  The rest of this
 * comment still stands -- only the blocker verdicts are withdrawn.
 * Screen: `python3 tools/nearmiss.py`, round 43 (2026-09-15).
 *
 * code_179d8_n -- functions 80..82 of the original 274-function code_179d8
 * monolith, 0x1A1BC..0x1AB78 (vram 0x800299BC..0x8002A378).  Carved round 26
 * (2026-09-09) out of what had been the `code_179d8_mid` uncarved remainder;
 * renamed on carve because "mid" named a REMAINDER and this is no longer one
 * -- the whole remainder was consumed, so no `code_179d8_mid` segment exists
 * any more.  Do not look for one.
 *
 * Blocker census at carve time, four screens per function (gp_rel,
 * forward nop_mflo_mfhi, `jr $t2` trampoline, jtbl):
 *   CD_sync (161w)  CLEAN
 *   CD_ready (180w)  CLEAN
 *   CD_cw (282w)  CLEAN
 * 3 of 3 clean, zero trivial `jr $ra` leaves.  These are BIG bodies, so this
 * unit is far larger in work than 3 suggests -- budget accordingly.
 *
 * Owns NO switch jump table (zero `jtbl_` references in the slice), so no
 * rodata sub-slot is attached to this unit and the 0x120C slot in the splat
 * yaml stays standalone.
 *
 * It DOES reference plain rodata SYMBOLS: D_80010984, D_80010994,
 * D_80010A0C, D_80010A14, D_80010A20, D_80010A28, D_80010A38.  Several of
 * those are ASCII.  They are SYMBOLS to reference (`extern const char
 * D_XXXXXXXX[];`), never strings to re-type as C literals -- splat has
 * already emitted those bytes and a literal emits a second copy, which
 * shifts the whole image (see CLAUDE.md, the duplicated-rodata-string
 * trap).
 *
 * This unit's own extern declarations are kept LOCAL to this file per the
 * project's multiple-independent-local-views convention.  It shares no
 * project header with any other unit.
 *
 * Round 26 (echo): all three functions were worked to near-misses and
 * STALLED -- see docs/match-reports/CD_sync.md (162/161, 1 word LONG),
 * CD_ready.md (178/180, 2 words short) and CD_cw.md (278/282,
 * 4 words short).  Every residue is an already-characterized GCC 2.6.3
 * quirk (a hoisted-constant register choice, dead-code-eliminated redundant
 * masks, and delay-slot/addressing-mode scheduling) documented in
 * docs/DECOMPILATION_LEARNINGS.md as not fixable by hand C restructuring --
 * read the three reports before re-attempting; they carry the full
 * near-miss bodies and the exact levers already tried.
 *
 * Round 35 (echo): re-screened round 26's stalled bodies against the
 * current tree before re-attempting -- per CLAUDE.md's "BUILD any inherited
 * body ONCE" discipline. Four of the six `extern func_XXXXXXXX` helper
 * declarations round 26's preserved bodies used are now STALE placeholder
 * names -- SDK-object rounds since renamed them to their real Sony symbols
 * (`func_80025900` -> `VSync`, `func_80025AE4` -> `puts`,
 * `func_80024E64` -> `CheckCallback`, `func_80012C20` -> `printf`; see
 * `src/code_179d8_g.c`'s own already-updated local declarations for the
 * same globals). Round 26's bodies, spliced verbatim, would not have LINKED
 * under today's tree -- the "a preserved body's `jal` targets can go STALE
 * across an SDK-object round" hazard from round 31 (DECOMPILATION_LEARNINGS.md).
 * `CD_flush` and `getintr` were NOT renamed (still real game
 * code, still INCLUDE_ASM/matched under those names in sibling units).
 *
 * All three bodies were rebuilt this round with the four names corrected,
 * plus a couple of quick untried levers per rounds 31/33's newer findings
 * (routing the CD_sync hoisted-constant "2" through a separate named
 * local; reading CD_ready's two flag bytes through a `volatile u8 *`
 * cast). All three REPRODUCED their round-26 recorded scores exactly
 * (162/161, 178/180, 278/282) and neither new lever moved anything --
 * consistent with round 33's finding that a GCSE/value-availability hoist
 * (CD_sync's case) and a genuinely-redundant-mask DCE (CD_ready's
 * case, independently re-confirmed here against the live `.s` rather than
 * inherited from a citation to the now-SDK-owned `func_8002B94C`) are both
 * immune to source-level rescue by construction, not by insufficient
 * effort. Re-filed as STALLS; see the three match reports for the updated
 * verdicts and the round-35 addenda.
 *
 * Round 37 (echo): re-confirmed CD_ready's and CD_cw's
 * round-35 bodies by rebuilding each live and checking funcdiff/lsdde.map
 * before trusting either score (both reproduce exactly), then preserved
 * both verbatim in `#if 0` blocks ahead of a permuter search -- neither had
 * been permuter-searched before this round. See the two match reports for
 * the search results.
 */
```

### code_179d8_g banner

```c
/*
 * ROUND 42 CORRECTION (2026-09-15) -- READ BEFORE ANY "BLOCKED" LINE BELOW:
 * every claim in this comment that a function is BLOCKED by `gp_rel`,
 * `nop_mflo_mfhi` or `addiu_at` is STALE.  All three constructs are RESOLVED
 * by pinned maspsx flags (CLAUDE.md, "Open toolchain blockers");
 * `tools/nearmiss.py` reports them tagged (RESOLVED-not-a-blocker) and counts
 * none of them.  Any "do NOT spend attempts on these" directive below is
 * therefore RETRACTED: those functions are ordinary matching work, and most
 * carry a mechanism-correct partial derivation already.  The rest of this
 * comment still stands -- only the blocker verdicts are withdrawn.
 * Screen: `python3 tools/nearmiss.py`, round 43 (2026-09-15).
 *
 * code_179d8_g -- functions 83..99 of the original 274-function code_179d8
 * monolith, 0x1AB78..0x1C440 (vram 0x8002A378..0x8002BC40).  Carved round 17
 * (2026-09-04) off the back of what was then `code_179d8_mid`, the three
 * functions in front of this slice. Those three were left as "all addiu-$at
 * blocked, so there is nothing left to staff there"; re-censused round 24
 * (2026-09-08) that is FALSE -- CD_sync (161w), CD_ready (180w)
 * and CD_cw (282w) are ALL THREE blocker-clean now that `addiu_at`
 * is resolved. Round 26 (2026-09-09) acted on that and CARVED them as the
 * C unit `code_179d8_n`; no `code_179d8_mid` segment exists any more.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 14 of the 17 clean, zero trivial leaves.  These are BIG bodies -- 196,
 * 223, 186 and 189 instructions among them -- so this unit is smaller in
 * count and considerably larger in work than 17 suggests.  Budget fewer
 * functions per pass here than in a leaf-heavy unit.
 *
 * NO LONGER BLOCKED -- all three of this unit's blocked functions were
 * blocked on `addiu_at` ALONE, and `addiu_at` was RESOLVED in round 21
 * (maspsx `--addiu-at`; docs/research/addiu-at-blocker.md). Re-screened with
 * `python3 tools/nearmiss.py` on 2026-09-08 (round 24):
 *   CD_readsync (174w)  func_8002B640 (186w)  func_8002B94C (189w)
 * The previous version of this comment read "BLOCKED, stub reports already
 * filed, do NOT spend attempts on these" -- a stale DIRECTIVE over free
 * ground.
 *
 * ROUND 32 (2026-09-12) CORRECTION -- that reopening WORKED, and the
 * "FRESH and assignable / their stub reports are already gone" wording it
 * left behind is now stale in the OPPOSITE direction. All three have since
 * been attempted and all three carry full worked stall reports (174/174
 * length-exact at 153 words; 3 words short; 2 words long respectively).
 * They are near-misses, NOT cold ground: read
 * docs/match-reports/<func>.md before spending an attempt, or you will
 * re-derive several hundred lines of someone else's derivation. Verified
 * by `tools/nearmiss.py` and by the presence of the report files, not by
 * reading this comment.
 *
 * AND A SECOND ROUND-32 CORRECTION, made the same day as the one above:
 * func_8002B640 and func_8002B94C are NOT GAME CODE AT ALL. Both lie fully
 * inside `libcd/iso9660.o` (Psy-Q 3.3), an object already placed in
 * config/psyq-objects.txt and verified against retail. No C matches them;
 * the correct disposition is conversion per docs/SDK-OBJECTS-GUIDE.md.
 * Only CD_readsync of the three is real game ground.
 *
 * ROUND 34 (head): CONVERTED. The unit's last three functions -- CdSearchFile
 * (func_8002B640), _cmp (func_8002B928, which had been matched as C) and
 * CD_newmedia (func_8002B94C) -- are linked from `lib/libcd/iso9660.o`, which
 * runs on into code_179d8_d (CD_searchdir, CD_cachefile, cd_read and a WEAK
 * memcpy). The unit is now 0x1AB78..0x1BE40 (vram 0x8002A378..0x8002B640),
 * 14 functions. The iso9660-only declarations that used to sit below (the
 * CD_* diagnostic strings, the directory-cache views, UWord) went with them.
 *
 * Note what happened here, because it is the reason this comment now
 * carries three verdicts: round 24 reopened all three as free ground and
 * round 32's first pass "corrected" that to near-misses -- both times
 * without asking whether Sony owned them. `python3 tools/sdkstalls.py`
 * answers that in one command and did not exist until round 32.
 *
 * Owns NO switch jump table -- zero `jtbl_` references in the slice -- so no
 * rodata sub-slot is attached to this unit.
 */
/* code_179d8_g -- this window's globals continue code_179d8_b's reading:
 * plain scalar/pointer driver state, not object fields (no classtable.py
 * hit near D_8006D5FC..D_8006D934). This unit's own extern declarations,
 * kept local per the project's multiple-independent-local-views convention
 * -- see code_179d8_b.c's header comment for why no shared header. */
```

## History (moved from src/libcd_bios.c, comments pass)

The prototype of getintr, in the declarations above CD_vol, carried:

> libcd_bios.c, MATCHED round 70
>                                                                    (libcd getintr by its strings)

The comment on copy8, getintr's inline helper, read:

> 8-byte response copy with a null guard on dst (2.6.3 does not fold
> `&array != NULL`).  It must be an INLINE FUNCTION, not a macro: as a
> do{}while(0) macro every site swapped the dst and counter registers
> (round 70); the inline's parameter pseudos give retail's allocation.

The file's banner carried its edge evidence:

> What decided its edges (python3 tools/tuboundary.py):
>   - start: the object in front, libcd/sys, ends here ("start edge
>     possible"), and getintr is bios.c's first function;
>   - CD_sync and CD_vol, once the first functions of their own carve units,
>     are proven to be the SAME file as what precedes them by the rodata
>     ("start edge IMPOSSIBLE", strings 0x800109F8 > 0x80010984 and
>     0x80010A38 > 0x80010984), so the three carve slices are merged here;
>   - end: the placed object libcd/iso9660 follows cb_read.
>
> The history of the three carve slices this file was merged from is in
> docs/match-reports/getintr.md, "File history".
