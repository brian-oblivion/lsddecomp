# SsUtGetVabHdr -- CONVERTED to a linked SDK object (round 34). NOT game code, NOT a stall.

> **ROUND 34 (2026-09-12), runner alpha. THIS FUNCTION IS NOW LINKED FROM
> SONY'S OWN OBJECT `libsnd/ut_gvh.o` (Psy-Q 3.3).** The name this report
> already used was a hypothesis read off `include/psyq/libsnd.h`; the object
> CONFIRMS it at the same address.
>
> It was the first of ELEVEN functions in the `code_179d8_i` PREFIX run
> (`0x2397C..0x24490`), ten `libsnd` objects that tile the range exactly. The
> unit's `c` line simply moved to `0x24490`. Whole-image SHA1 green. Nothing
> here is assignable.
>
> **The C this report derives was MATCHED and is now DELETED from
> `src/code_179d8_i.c`.** That is the correction CLAUDE.md asks for, not a
> regression. Do not write C for it again.
>
> This report is the one place in the round where the prose had ALREADY
> reached the right conclusion -- it argued from the SDK header that this
> "IS the SDK utility, not a coincidentally-named local" -- and no tool could
> read it. That is the `sdkstalls.py` lesson in its original form.
>
> Previous title: `SsUtGetVabHdr — MATCHED 57/57`
>
> **Everything below is kept as the derivation it was, not as live guidance.**

Unit: `code_179d8_i`. Blocker screens clean.

## This IS the Psy-Q SDK function, not a coincidental name

`include/psyq/libsnd.h:229` declares `extern short SsUtGetVabHdr (short,
VabHdr*);` — the game unit's function has the exact SDK name, and its
disassembly copies exactly the fields `VabHdr` (LIBSND.H:65-81) declares,
at exactly their declared offsets, skipping only the three `reserved`/`fsize`
fields the SDK marks "system reserved". That is strong confirming evidence,
per this task's own framing that a name alone is only a hypothesis.

**Not `#include`-d directly.** `LIBSND.H` itself does `#include <sys/types.h>`,
and that does not resolve under the pinned include path
(`-Iinclude -Iinclude/psyq`) on this case-sensitive filesystem — only
`include/psyq/sys/types.h` exists (uppercase). Confirmed in isolation with
the pinned `cpp`:

```
$ tools/gcc263/cpp -Iinclude -Iinclude/psyq -undef -lang-c -nostdinc \
    -Dmips -D__GNUC__=2 t.c
include/psyq/libsnd.h:18: sys/types.h: No such file or directory
```
(exit 33). So a local copy of the same 0x20-byte layout is used instead, per
this project's established multiple-independent-local-view convention —
this is a *header-inclusion* limitation, not a doubt about the field layout,
which is copied verbatim from the SDK's own comments.

## Signature

```c
short SsUtGetVabHdr(short vabId, VabHdr *hdr)
```

## Body (matched verbatim, first attempt)

```c
typedef struct {
    s32 form;
    s32 ver;
    s32 id;
    u32 fsize;
    u16 reserved0;
    u16 ps;
    u16 ts;
    u16 vs;
    u8 mvol;
    u8 pan;
    u8 attr1;
    u8 attr2;
    u32 reserved1;
} VabHdr;

extern u8 _svm_vab_used[];
extern VabHdr *D_8008E80C[];
extern VabHdr *_svm_vh;

short SsUtGetVabHdr(short vabId, VabHdr *hdr)
{
    VabHdr *vab;

    if (_svm_vab_used[vabId] != 1) {
        return -1;
    }
    vab = D_8008E80C[vabId];
    hdr->form = vab->form;
    hdr->id = vab->id;
    hdr->ver = vab->ver;
    hdr->ps = vab->ps;
    hdr->ts = vab->ts;
    _svm_vh = vab;
    hdr->vs = vab->vs;
    hdr->mvol = vab->mvol;
    hdr->pan = _svm_vh->pan;
    hdr->attr1 = _svm_vh->attr1;
    hdr->attr2 = _svm_vh->attr2;
    return 0;
}
```

## Reading the disassembly

`D_8008E80C` is an array of `VabHdr*` (each slot 4 bytes, indexed
`vabId*4`) — a per-slot loaded-VAB-header table shared with the sibling
functions in this unit that also index `_svm_vab_used`/`D_80090BD4`-style
tables by the same small id. `_svm_vh` is a one-deep "last-fetched VAB
header" cache: retail assigns it mid-copy (right after fetching `vs`) and
then reads `pan`/`attr1`/`attr2` back **through the global** rather than
keeping `vab` live in a register for those three fields, even though `vab`
is still live one field earlier (`mvol`). This reproduced byte-exact by
simply writing the source in that literal order — no barrier or reshaping
needed; GCC's own liveness/rematerialization choice, not a scheduling
artifact to fight.

### Proposed learning

**A CRLF-broken header's OWN `#include` can fail before any macro-expansion
issue is even reached, on a case-sensitive filesystem — check `sys/types.h`
resolution before blaming CRLF splicing.** `docs/research/psyq-header-crlf-blocker.md`
documents the multi-line-macro CRLF landmine in the Psy-Q headers, but this
is a *different* failure that would bite any first attempt to `#include`
`LIBSND.H` (and likely other Psy-Q headers with the same
`#include <sys/types.h>` line) regardless of the macro issue: the literal
path does not exist under this include search path because the SDK's own
`SYS/TYPES.H` directory is uppercase. Worth a one-line note next to the CRLF
finding so the next runner doesn't spend an attempt diagnosing the wrong
failure mode.
