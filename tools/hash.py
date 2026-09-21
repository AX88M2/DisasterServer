#!/usr/bin/env python3
# By FakerNull0 (aka MiniExe)
import sys

def java_hash(s: str) -> int:
    h = 0
    for ch in s:
        h = (h * 31 + ord(ch)) & 0xFFFFFFFF
    return h if h < 0x80000000 else h - 0x100000000

def cmd_hash(name: str) -> int:
    if not name.startswith('.'):
        name = '.' + name
    return java_hash(name)

if len(sys.argv) < 2:
    print("Usage: python hash.py <command>")
    print("Example: python hash.py .cmd")
    sys.exit(1)

name = sys.argv[1]
clean = name.lstrip('.').upper()
print(f"#define CMD_{clean} {cmd_hash(name)}")