# getintr -- MATCHED (337/337, byte-exact, round 70)

> Renamed from `func_80029478` on 2026-09-23 (tools/rename.py). Address 0x80029478.

REVISITED, round 70: MATCHED (337/337, whole-image SHA1 green); names/types not relevant.

Unit: `src/code_179d8_b.c`. 337 words. Owns `jtbl_800109F8` (rodata sub-slot
`[0x11F8, .rodata, code_179d8_b]`), which the `switch` lowering generates and
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
declare, `D_8006D608` and `D_8006D620`), it measured:

- **159/337 raw, `.text` 0x54c (2 words LONG)**, whole image drifted;
- `insertions 71 / deletions 71`, positional skeleton diffs 141.

So neither the length claim nor the register-identity verdict held. The
residue decomposed into five separate defects, fixed in this order
(builds in brackets, each score from a fresh build):

| # | defect (asm-differ) | fix | result |
| --- | --- | --- | --- |
| 1 | `bnez` where mine had `beqz` after `D_8006D60C & 0x10` | **logic**: `!(D_8006D60C & 0x10) && (resp[0] & 0x10)` | -- |
| 2 | cases 1/2: `li v0,2`/`li v0,5` swapped around the `beqz s0` | **logic**: retail stores 5 when `flags != 0` (`flags ? 5 : 2`, `flags ? 5 : 1`). Case 2 still needs the if/else-into-a-local form; the ternary unfolds its store | ins/del 71/71, 0x54c |
| 3 | cases 4/5: mirror store folded (`$at`) where retail unfolds (`lui/addiu/sb 0`) | `*(volatile u8 *)&D_8006D8D9 = D_8006D8DA;` and `*(volatile u8 *)D_8006D8D8 = D_8006D8D9;` (same cast case 3 already used) | 67/67, still 0x54c |
| 4 | cases 4/5: copy's src pointer computed after the null check, a `nop` in the `beqz` delay slot | initialise the copy's `src` BEFORE `if (dst != NULL)` | **0x544 exact**, 253/337, 27/27 |
| 5 | all 10 copy sites: dst and counter registers swapped (`$a0`/`$v1`), src right | **the copy as a `static __inline__` function** instead of a `do{}while(0)` macro | **333/337, 0/0** |
| 6 | one missing `andi v0,v0,0xff` after `lbu v0,0x18(sp)`; mine loaded `resp[0]` twice instead | `D_8006D60C = *(volatile u8 *)&resp[0]; D_8006D610 = resp[1]; flags = D_8006D60C & 0x1D;` | **337/337, OK: build matches retail** |

About 30 builds in all. No permuter search was spent; Gate 3 was never
reached.

### What did not work (defect 6, measured, all with the inline copy in place)

- `flags = D_8006D60C & 0x1D` alone: one load, but no `andi 0xff` (0x540, one short).
- A `u8 st = resp[0]` local used for both the store and `flags`, at function
  scope, block scope, before the counter test and before the whole `if`;
  `flags` as `u8` as well as `s32`; reordering the three statements; `D_8006D60C
  = st = resp[0]`; `flags = (st = resp[0]) & 0x1D`: all either one short
  (combine folds the zero-extension into the `lbu`) or worse.
- `flags = (u8)D_8006D60C & 0x1D` or `D_8006D60C = resp[0] & 0xFF`: these reload
  the global's low byte (`lbu` of `D_8006D60C`), 2 words long.
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
  up from `D_8006D620` (the same string table `func_80028CF8`, matched
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
4. Unless cause==3 with a false `D_8006D7C0[D_8006D61D]` lookup (a per-mode
   flag table, same selector family as `D_8006D620`/`D_6006D6A0`), update
   an error counter (`D_8006D614`) when a flag bit turns on across the
   read, latch the two response bytes into `D_8006D60C`/`D_8006D610`, and
   compute a `flags` value (`resp[0] & 0x1D`) used by cases 1-3 below.
   (Round 70: the counter increments when bit 0x10 turns ON, i.e. the old
   `D_8006D60C` bit is CLEAR and the new `resp[0]` bit is set.)
5. On cause==5, log the diagnostic strings above.
6. Dispatch on cause (1-5, via `jtbl_800109F8`; 6/7/out-of-range and the
   post-mask 0 case fall to a "CDROM: unknown intr (%d)\n" default),
   writing a small status byte (`D_8006D8D8[0]` and/or `D_8006D8D9`,
   depending on the case and mirrored between them in cases 4/5) and
   copying the 8-byte response into one or two of three contiguous 8-byte
   mailboxes (`D_8008B3CC`, `D_8008B3D4`, `D_8008B3DC`), returning a small
   bit-flag-shaped result (0, 1, 2, 4, 6, or -1) -- confirmed against
   caller sites in `asm/code_179d8_mid.s`, which `andi` the result against
   `0x2` and `0x4`, consistent with a flag word.

`s32 getintr(void)` -- confirmed against three call sites
(`asm/code_179d8_mid.s`, `asm/nonmatchings/code_179d8_g/func_8002B3F4.s`,
`asm/nonmatchings/code_179d8_g/func_8002AEE0.s`), all `jal` with a `nop`
delay slot and no argument setup, and against `code_179d8_g.c`'s own
existing forward declaration (`extern s32 getintr(void);`).


## The source as matched

It is live in `src/code_179d8_b.c`. The load-bearing shapes:

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
        D_8006D60C = *(volatile u8 *)&resp[0];
        D_8006D610 = resp[1];
        flags = D_8006D60C & 0x1D;
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

None. The round-21 report's `extern u8 D_8006D60C` conflict note is moot
(the unit holds only this function).
