# SceneNode__NoOpSlot5C

> Renamed from `SceneNode__func_1d33c` on 2026-09-26 (tools/rename.py). Address 0x8001d33c.

> Renamed from `Class6B5CC__func_1d33c` on 2026-09-26 (tools/rename.py). Address 0x8001d33c.

> Renamed from `func_8001D33C` on 2026-09-23 (tools/rename.py). Address 0x8001d33c.

**Unit:** code_d294 · **Size:** 2 words · **Status:** MATCHED (an empty `void f(void) {}` body; no matching work was needed).

## Naming

Round 71 (alpha). `func_8001D33C` -> `SceneNode__NoOpSlot5C`, **tier C**. Table slot +0x05C: an empty body (`jr $ra; nop`). Finalize calls the slot with (self, 0). Of the 17 tables checked (the tag-4 family plus two others), only gNodeGuardedViewportMethods overrides +0x05C, with another empty body (Viewport__NoOpSlot5C, code_2cc8c). Nothing establishes what the slot is for, so it keeps the tier-C `Class__func_xxxxx` form.

## Track 6 (round 91, echo): named `SceneNode__NoOpSlot5C`, tier C

Empty. Slot +0x05C's only caller is SceneNode__Finalize, as `(self, 0)`, and no subclass overrides it, so nothing shows what the slot is for. `NoOpSlotNN` follows MoviePlayer__NoOpSlot5C. Was `SceneNode__func_1d33c`. The class was renamed Class6B5CC -> SceneNode in the same pass (include/SceneNode.h's banner has the evidence).
