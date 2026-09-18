> Renamed from `func_800283C4` on 2026-09-18 (tools/rename.py). Address 0x800283c4.

# FreeCdRequestNode

**Unit:** code_179d8_r · **Size:** 33 words · **Status:** MATCHED (33/33 words) · **Round 45**

## What it does

The remove/free counterpart to `AllocCdRequestNode`: unlinks a node from the
doubly-linked list rooted at `D_8008A894` (fixing up `prev->next`,
`next->prev`, or the list head as appropriate) and frees it via
`func_80017CFC`. Void return. Never-attempted cold ground.

## The C

```c
void FreeCdRequestNode(Node8008A894 *node)
{
    Node8008A894 *prev;
    Node8008A894 *next;

    LockCd();
    if (node != NULL) {
        prev = node->prev;
        if (prev != NULL) {
            prev->next = node->next;
        } else {
            D_8008A894 = node->next;
        }
        next = node->next;
        if (next != NULL) {
            next->prev = node->prev;
        }
        func_80017CFC(node);
    }
    UnlockCd();
}
```

Closed on the first attempt (uses the `Node8008A894` struct from
`AllocCdRequestNode`, declared once at the top of the unit).
