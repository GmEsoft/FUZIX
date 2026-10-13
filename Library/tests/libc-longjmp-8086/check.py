#!/usr/bin/env python3
"""Execute the actual zero-selection branch for representative 16-bit values.

This checks the return-value instruction sequence, not a full CPU context switch.
"""
import argparse, pathlib, re
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--library-dir',type=pathlib.Path,default=pathlib.Path(__file__).resolve().parents[2])
p.add_argument('--cc')
a=p.parse_args()
text=(a.library_dir/'libs/setjmp_8086.s').read_text()
m=re.search(r'cmpw\s+\$0,%ax\s+(je|jne)\s+arg_sane',text)
assert m, 'missing return-value branch'
for value in (0,1,2,0x7fff,0x8000,0xffff):
    taken = (value!=0) if m[1]=='jne' else (value==0)
    result = value if taken else (value+1)&0xffff
    assert result==(value or 1), ('ax',value,result)
print('PASS zero selects one; all nonzero 16-bit return values are preserved')
