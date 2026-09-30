# tools/research

Tools no current work in this tree uses. Each one answered a question the
project had at some point, and most would answer the same question for
another PSX decomp. They're kept runnable (run them from the repository
root, as `python3 tools/research/<tool>.py`), but nothing in the build, lint
or CI calls them.

| tool | what it answers |
| --- | --- |
| `nearmiss.py` | which still-`INCLUDE_ASM` functions are closest to matching, ranked |
| `uncarved.py` | how much game code is still inside monolithic `asm` segments |
| `tuboundary.py` | where the original source files began and ended, read from the rodata |
| `sonyheaders.py` | which game headers re-declare a Sony name their own way |
| `externcheck.py` | `extern` function declarations whose arity disagrees with the definition |
| `stalesyms.py` | match reports whose preserved body names a since-renamed symbol; the reports are on `archive/process`, so it needs `--reports DIR` pointed at a checkout of that branch |

The round machinery (the plan tracker `plan.py`, its ledger, the runner
broadcast channel, rename replay across merges) is on the `archive/process`
branch rather than here: it only makes sense with the plan it served.
