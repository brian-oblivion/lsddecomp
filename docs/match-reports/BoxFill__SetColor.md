# BoxFill__SetColor — MATCHED (9/9 words)

> Renamed from `Obj6EAC0__SetColor` on 2026-09-25 (tools/rename.py). Address 0x8004076c.

Unit: `src/ScreenWidgets.c`. No prior report on file (oversight -- this is
a genuine one-line wrapper, not a splat-generated trivial body).

```c
void BoxFill__SetColor(Obj6EAC0 *self, s32 overwrite, u8 *src) {
    BoxFill__ApplyColor(self, self->color, src, overwrite);
}
```

Base occupant of `slotB8`: a thin forwarder into `BoxFill__ApplyColor`
(this unit), always passing `self->color` as the destination.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_8004076C` | `BoxFill__SetColor` | A |

**Evidence.** Mechanics are the whole purpose (tier A): the ENTIRE body
is a single call into `BoxFill__ApplyColor` with `self->color` as the
fixed destination -- "set (or blend) this object's colour" is exactly
what the code does, nothing more to guess at.

## Track 4 (2026-09-25, round 85, charlie)

Class 0x64 (was D_8006EAC0) is unified as BoxFill in include/BoxFill.h: Viewport__DrawNode draws a node whose class-id low byte is 0x64 with GsSortBoxFill over the GsBOXF at +0x058 (pri +0x044, `relative` +0x048, x/y +0x050/+0x054). The body now takes `BoxFill *`; zero bytes changed. Renamed from `Obj6EAC0__SetColor`: the +0x0B8 occupant, BoxFill's own slot `setColor`. It copies (or, with `overwrite` 0, adds) three bytes into +0x064, the GsBOXF r,g,b DrawNode sorts (tier A). Callers: Reset (the ctor's colour), GraphRoom__TickHighlight (1, &gGraphPointHighlightColor), FadeBox's fades (1, a table entry).
