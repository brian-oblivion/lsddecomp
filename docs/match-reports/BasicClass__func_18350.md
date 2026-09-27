# BasicClass__func_18350 — MATCHED (2/2 words), NAME DELIBERATELY NOT CHANGED

**Unit:** `TmdRenderer` · **Status:** matched since round 12; splat generated
the body itself (`jr $ra; nop`). This report was created in round 51 by the
naming pass, because a function that was looked at and left with a
placeholder name needs the reason written down as much as a renamed one
does.

## What it is

`BasicClassMethods` vtable slot `+0x038 - 0x4` = `+0x034`, resolved with
`tools/classtable.py gBasicClassMethods`. The body is empty.

```c
void BasicClass__func_18350(void) {
}
```

## Naming (round 51, bravo)

**Kept as `BasicClass__func_18350`. Tier C.** The class is known, so the
name carries the tier-C form the project already uses for that case
(`Class__func_xxxxx`); the method half stays a placeholder.

**What is known, measured rather than reasoned.** I ran
`tools/classtable.py` over all 60 method tables it finds and read slot
`+0x034` out of each:

| slot +0x034 holds | tables |
| --- | --- |
| `BasicClass__func_18350` (this function) | 58 |
| `func_80023368` | 1 (`D_8006C0F8`) |
| `StyleCue12` | 1 (`gStyleCueCallbacks`) |

Neither of the two exceptions is a BasicClass override:

- `D_8006C0F8`'s header word is `0x800103D4`, a text-looking pointer where
  every real table in this corpus has a small id/flags value, and the
  function it names lives in `asm/psyq_10ee0.s` (SDK). That is a
  mis-detected table start, not a class.
- `gStyleCueCallbacks` is a wholly independent 14-slot class — `ParamObj` in
  `src/class_3bb8c_r.c` — that overrides **every** slot including `+0x004`,
  and whose slots do not correspond to BasicClass's semantically at all
  (its `+0x030`, `+0x034` and `+0x038` are three sibling per-kind parameter
  tweaks, `StyleCue11`/`StyleCue12`/`StyleCue13`, not a
  notify/hook/handler trio). It shares BasicClass's slot COUNT and nothing
  else.

**So nothing in the game overrides this slot, and nothing calls it.** With
no override and no call site there is no evidence for a name, and an empty
body supplies none by itself. A tier-A or even tier-B name here would be
pure invention.

There is a second consequence worth recording: the slot's declared
signature in `include/BMemPMgr.h` is `void (*slot34)(void)`, which is what
an empty base body PERMITS, not what anything was observed to pass. If a
caller is ever found, expect that declaration to be wrong rather than
confirmed. `grep -rn -- '->slot34\b\|\.slot34\b' src/` returns nothing, so
the field is also the one field in `BasicClassMethods` with no accessor
anywhere and could in principle be renamed by a single runner — there is
just nothing to rename it to.

## Round 91 polish (delta, track 7)

Name kept, tier C. The source comment was cut to the conclusion; the table
census it quoted is this report's table above.
