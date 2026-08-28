# Match reports

**One file per function anyone touches — matched ones included, not just
stalls.** `tools/progress.py` decides whether a queued function is a documented
STALL or untouched FRESH ground purely by whether `docs/match-reports/<func>.md`
exists. A stall with no report is counted as unworked, and the next round staffs
someone straight back onto it to re-derive what you already established.

A report is a FILE. Not a comment in the `.c`, not a final chat message — those
die with the session, and the derivation is the expensive part.

## Shape

```markdown
# func_8005ABCD

**Unit:** DreamSys · **Size:** 47 instructions · **Status:** MATCHED | STALLED 44/47

## What it does
One paragraph, in terms of the game, not the registers.

## Derivation
What the signature is and how you know. Which struct fields you established,
at what offsets, and what evidence fixed each one.

## Residue  (stalls only)
What differs, and the class. Register identity / ordering / instruction count /
maspsx expansion — see docs/MATCHING-GUIDE.md "Reading a residue".

## Preserved body  (stalls only)
The best body you reached, as LITERAL SOURCE, with every declaration it needs,
positioned where it would compile. Not a path — a path into a gitignored
directory exists in exactly one checkout and travels nowhere. Test it before
you restore the INCLUDE_ASM: splice this text back in and build once.

## Proposed learning
One line, if there is something generalizable. The head promotes these into
docs/DECOMPILATION_LEARNINGS.md after merging; runners never edit that file
directly.
```
