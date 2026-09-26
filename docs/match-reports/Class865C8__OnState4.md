# Class865C8__OnState4 -- MATCHED (splat-generated, empty body)

> Renamed from `Obj865C8__Noop7C` on 2026-09-26 (tools/rename.py). Address 0x80049ea4.

`Class865C8Methods` slot +0x07C. Entire function body:

```c
void Class865C8__OnState4(Obj865C8 *self) {
}
```

An empty override, byte-exact by construction (`jr $ra` / `nop`). Never had its own report before this round's rename; created here because CLAUDE.md requires one file per touched function, matched ones included.


## Naming

`Class865C8__OnState4` -- tier A. Empty body (`{}`), occupies the class's own +0x07C override slot (already named `noop7C` in the header before this round). An empty body is direct evidence of a no-op leaf.

## Track 4 (2026-09-26, round 88, Class865C8)

The class (table D_800865C8, id 0x1F230, TimedTask's subclass) is unified as Class865C8 in include/Class865C8.h; the Obj865C8/Class865C8Methods views in class_39e08.h are gone. Renamed from Obj865C8__Noop7C: occupant of TimedTask's +0x07C onState4 (setState(4) calls it), empty.
