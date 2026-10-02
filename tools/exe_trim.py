#!/usr/bin/env python3
"""Cut a linked PS-X EXE to the length its own header gives.

`objcopy -O binary` writes every loaded byte, and GCC 2.6.3 emits an
initialized `.sbss` as data, so the image runs on to the end of the last
`.sbss` object. Retail's file stops at 0x800 + its header's text size
(0x7B800, a 2 KB sector boundary): its `.sbss` was never in the file, and
the zeros up to that boundary are sector padding. TmdRenderer's
sDivPolygon4 crosses it.

Only zeros may be cut. A non-zero byte past the end means real data was
placed there, and the build stops rather than hide it.

Usage: exe_trim.py <exe>
"""
import struct
import sys

HEADER_SIZE = 0x800
T_SIZE_OFFSET = 0x1C  # the header's .text size word


def main():
    path = sys.argv[1]
    with open(path, 'rb') as f:
        data = f.read()
    end = HEADER_SIZE + struct.unpack_from('<I', data, T_SIZE_OFFSET)[0]
    if len(data) <= end:
        return 0
    tail = data[end:]
    nonzero = next(i for i, b in enumerate(tail + b'\1') if b)
    if nonzero < len(tail):
        sys.exit(f'{path}: non-zero byte at file offset 0x{end + nonzero:X}, past the '
                 f'header\'s end 0x{end:X}: data was linked beyond the image')
    with open(path, 'wb') as f:
        f.write(data[:end])
    return 0


if __name__ == '__main__':
    sys.exit(main())
