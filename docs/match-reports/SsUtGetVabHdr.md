# SsUtGetVabHdr — MATCHED 57/57

Unit: `code_179d8_i`. Blocker screens clean.

## This IS the Psy-Q SDK function, not a coincidental name

`include/psyq/LIBSND.H:229` declares `extern short SsUtGetVabHdr (short,
VabHdr*);` — the game unit's function has the exact SDK name, and its
disassembly copies exactly the fields `VabHdr` (LIBSND.H:65-81) declares,
at exactly their declared offsets, skipping only the three `reserved`/`fsize`
fields the SDK marks "system reserved". That is strong confirming evidence,
per this task's own framing that a name alone is only a hypothesis.

**Not `#include`-d directly.** `LIBSND.H` itself does `#include <sys/types.h>`,
and that does not resolve under the pinned include path
(`-Iinclude -Iinclude/psyq`) on this case-sensitive filesystem — only
`include/psyq/SYS/TYPES.H` exists (uppercase). Confirmed in isolation with
the pinned `cpp`:

```
$ tools/gcc263/cpp -Iinclude -Iinclude/psyq -undef -lang-c -nostdinc \
    -Dmips -D__GNUC__=2 t.c
include/psyq/LIBSND.H:18: sys/types.h: No such file or directory
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

extern u8 D_8008EA2C[];
extern VabHdr *D_8008E80C[];
extern VabHdr *D_8008E970;

short SsUtGetVabHdr(short vabId, VabHdr *hdr)
{
    VabHdr *vab;

    if (D_8008EA2C[vabId] != 1) {
        return -1;
    }
    vab = D_8008E80C[vabId];
    hdr->form = vab->form;
    hdr->id = vab->id;
    hdr->ver = vab->ver;
    hdr->ps = vab->ps;
    hdr->ts = vab->ts;
    D_8008E970 = vab;
    hdr->vs = vab->vs;
    hdr->mvol = vab->mvol;
    hdr->pan = D_8008E970->pan;
    hdr->attr1 = D_8008E970->attr1;
    hdr->attr2 = D_8008E970->attr2;
    return 0;
}
```

## Reading the disassembly

`D_8008E80C` is an array of `VabHdr*` (each slot 4 bytes, indexed
`vabId*4`) — a per-slot loaded-VAB-header table shared with the sibling
functions in this unit that also index `D_8008EA2C`/`D_80090BD4`-style
tables by the same small id. `D_8008E970` is a one-deep "last-fetched VAB
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
