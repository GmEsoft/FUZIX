#!/usr/bin/env python3
"""Compile actual driver functions with deterministic host hardware stubs."""
import argparse, os, re, subprocess, tempfile
from pathlib import Path

def function(text,name):
    match=re.search(r'(?m)^[\w *\t]+\b'+re.escape(name)+r'\s*\([^;{}]*\)\s*\{',text)
    if not match: raise ValueError('missing function '+name)
    masked=re.sub(r'/\*[\s\S]*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',lambda m:' '*len(m.group()),text)
    depth=0
    for i in range(text.index('{',match.start()),len(text)):
        if masked[i]=='{':depth+=1
        elif masked[i]=='}':
            depth-=1
            if not depth:return text[match.start():i+1]
    raise ValueError('unterminated '+name)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--kernel-dir',type=Path,default=Path(__file__).resolve().parents[2])
    p.add_argument('--cc',default=os.environ.get('CC','cc'))
    a=p.parse_args();kernel=a.kernel_dir.resolve()
    fixture=Path(__file__).with_name('check.c').read_text()
    def expand(m):
        kind,file,name=m.groups(); text=(kernel/file).read_text()
        if kind=='HEADER':return re.sub(r'(?m)^#include[^\n]*\n','',text)
        if kind=='ARRAY':return re.search(r'static const uint16_t '+name+r'\[\]\s*=\s*\{[\s\S]*?\};',text).group()
        return function(text,name)
    source=re.sub(r'/\* (FUNCTION|HEADER|ARRAY) ([\w./-]+)(?: (\w+))? \*/',expand,fixture)
    with tempfile.TemporaryDirectory(prefix='fuzix-dev-check-') as td:
        c=Path(td)/'check.c';binary=Path(td)/'check';c.write_text(source)
        subprocess.run([a.cc,'-std=c99','-g','-Wall','-Wextra','-Wno-unused-function','-Wno-unused-variable','-Wno-unused-parameter','-Wno-sign-compare','-Wno-unused-value','-fsanitize=address,undefined',str(c),'-o',str(binary)],check=True)
        subprocess.run([str(binary)],check=True,timeout=10,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
    print('PASS '+Path(__file__).parent.name)
if __name__=='__main__':main()
