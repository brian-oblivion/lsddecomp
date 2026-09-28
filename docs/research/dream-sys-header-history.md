# Dream system headers: process text moved out by the API pass

Track 12 (item `area-dream-sys`, round 106, runner charlie) rewrote these
headers as Doxygen API documentation:

    include/dream_sys.h include/stage_grid.h include/tod_actor.h

and the comments of their units, src/world/dream_sys.c,
src/world/stage_grid.c and src/world/tod_actor.c. Text that justified a C
spelling or recorded a derivation for one function went to that function's
match report, under "History: track 12". This note holds what belongs to no
single function: pointers at the project's tools, and remarks about the
toolchain or retail that the class banners and type comments carried.
`python3 tools/apidoc.py --item area-dream-sys` measures what is left.

## History: the passages as they stood before the pass, verbatim

The source of every excerpt is commit `2aa001d93`.

### include/tod_actor.h

The slot list's comment, a tool pointer:

```c
/* Occupants in gTodActorMethods named at each slot; `tools/classtable.py
 * gEntityMethods --vs gTodActorMethods` lists Entity's overrides. The
```

The class banner's note on the attachToParent cast:

```c
 * it to TodActorAttachToParentFn (a function-pointer cast emits no code).
```

### src/world/tod_actor.c

```c
/* Header word of gModelDataMethods (tools/classtable.py), the class New_ModelData
 * allocates and TodActor.modelData points at. */
```

### include/stage_grid.h

The header carried an unused tag, removed by the pass (no reference in
src/ or include/; the build stayed byte-exact):

```c
struct simplePair {
    s8 x;
    s8 y;
};
```

Its prototype comments were plain `/* @brief */` lines, which Doxygen
skips; the pass rewrote them as `/** */` blocks, checked against the bodies.

### src/world/stage_grid.c

```c
/* The two per-stage tables (splat data), read only here. */
```

### include/dream_sys.h

The method table's comment, a tool pointer (the rest of the comment, on the
two cast slots, is in DreamSys__DreamSys's report):

```c
/* Occupants in gDreamSysMethods named at each slot (`tools/classtable.py
 * gDreamSysMethods --vs gActorMethods`). The inherited slots keep Actor's
```

The flashback rotation's field remarks, replaced by what each axis is:

```c
typedef struct FlashbackRotation {
    struct Angle {
        s16 angle;
        s16 one;
    } pitch; /* Does not do what you think it does */
    struct Angle heading;
    struct Angle roll; /* Ditto */
} FlashbackRotation;
```

The prototype comments were plain `/* @brief */` lines, which Doxygen
skips; the pass rewrote every one as a `/** */` block checked against the
body. Comments naming a link test's call shape (`bltz`, `beqz`) went to
each function's report.

### src/world/dream_sys.c

The unit banner, split by the pass: the per-tick chain went to the DreamSys
class doc in include/dream_sys.h, and the banner keeps what the file holds.

```c
/* DreamSys -- the class that runs a dream in progress (include/dream_sys.h
 * has the class as a whole). Every DreamSys method is here, in four groups,
 * followed by the free functions the link tests are built from.
 *
 * 1. Construction and reset: New_DreamSys, DreamSys__DreamSys,
 *    ResetSessionState (no tick callbacks, sound cue set freed, staircase
 *    walk cleared, base orientation applied), ResetLinkState, SpawnAtLink.
 *
 * 2. The per-tick chain. TimerTick advances the dream clock (Actor's `tick`)
 *    and, at dreamTimeLimit, loads the next flashback or ends the dream;
 *    below the limit it runs UpdateTickState and RunTickCallbacks, which call
 *    the look and move callbacks SelectLookCallback/SelectMoveCallback install. The look
 *    callback is StepLook: two spring-back accumulators, StepLookOffset (the
 *    view height) and StepLookYaw (45 degrees a tick, up to 181, and back).
 *    The move callback is TickDrift or TickMove, the movement state machine
 *    (free, forced or held, by moveOverride) around AdvanceMoveCycle's
 *    four-tick step: a voice through the VabStreamObj in soundObj
 *    (StartVoice/StopVoice), a view bob, and ApplyMoveCommand, which tries
 *    the link tests before it lets the step happen.
 *
 * 3. Day, mood and flashback bookkeeping: StartDay/EndDay, the mood graph
 *    (two MoodGraphContributor accumulators that UpdateDreamChart averages
 *    and CalcDreamColor turns into a DreamColors value),
 *    AddFlashback/FlashbackSaving, CalcUnlockScore, and GetSaveBlock, which
 *    hands out the DREAMSYS_SAVE_SIZE bytes from saveMagic that InitNewGame
 *    initializes.
 *
 * 4. Linking: WallLink/DynamicLink and the Try...Link family, over the free
 *    TestFor.../GetStaticSpawn testers and the stage spawn tables. ExecuteLink
 *    records the link (enum DreamSysLinkCode) in Actor's `state` and tells
 *    the parents. */
```
