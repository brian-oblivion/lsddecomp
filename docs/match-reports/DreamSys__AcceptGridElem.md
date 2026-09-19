> Renamed from `func_80057B54` on 2026-09-19 (tools/rename.py). Address 0x80057b54.

# DreamSys__AcceptGridElem -- MATCHED (15/15)

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys` family (called only from
this unit's own `DreamSys__ScanGridWindow`, still queued at time of writing, twice, as
`DreamSys__AcceptGridElem(elemOrHead, arg1, arg2)`). Not itself a vtable slot in any
of the class tables reachable from this unit's addresses.

## Signature

```c
void *DreamSys__AcceptGridElem(void *arg0, void *arg1, void *arg2);
```

`arg1`/`arg2` are never read by the body -- present only to match the
caller's 3-argument calling convention.

## Body

```c
void *DreamSys__AcceptGridElem(void *arg0, void *arg1, void *arg2) {
    if (arg0 != NULL) {
        if (func_8001E7BC() != 0) {
            return arg0;
        }
    }
    return NULL;
}
```

`func_8001E7BC` is declared locally (`extern s32 func_8001E7BC(void);`) --
it is matched/queued in a different unit (`src/code_d294_c.c`), so its
prototype belongs here, not in a shared header.

## Shape note

Written as NESTED `if`s rather than `if (arg0 != NULL && func_8001E7BC())`.
The combined-condition form compiles to an extra `move v1,v0` before the
`bnez` and shifts the whole function 4 bytes long (a spurious second
`return NULL;` merge point reached via a different control path than
retail's, which falls through to the shared `move v0,zero` from the INNER
check only). Confirmed by direct comparison: combined form gives 6/15
words with the size drift warning; nested form gives 15/15 exactly.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py DreamSys__AcceptGridElem   # 15/15
```

### Proposed learning

**A guard clause folded into `&&` with the following check is not always
equivalent to writing it as a nested `if`, even when both are logically a
single early return.** `if (a && b) return x; return y;` and
`if (a) { if (b) return x; } return y;` can compile to different merge
points for the "no" path -- one funnels through an extra register copy to
reach a single shared zero-setter, the other lets the inner check fall
through directly. When a combined condition scores short by exactly a
handful of words with everything downstream shifted, try splitting it into
nested `if`s before suspecting anything else.
