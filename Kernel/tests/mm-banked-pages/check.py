#!/usr/bin/env python3
"""Compile actual mm sources with host fixtures, then run under ASan/UBSan."""
import argparse, os, re, subprocess, tempfile
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--kernel-dir',type=Path,default=Path(__file__).resolve().parents[2])
a=p.parse_args()
failed=False
with tempfile.TemporaryDirectory(prefix='fuzix-mm-') as tmp:
 for fixture in sorted(Path(__file__).parent.glob('*.c')):
  source=fixture.read_text()
  def expand(m):
   text=(a.kernel_dir/m[1]).read_text()
   return re.sub(r'^#include[^\n]*','',text,flags=re.M)
  source=re.sub(r'/\* SOURCE ([\w./]+) \*/',expand,source)
  c=Path(tmp)/(fixture.stem+'.c');c.write_text(source)
  binary=c.with_suffix('')
  result=subprocess.run([os.environ.get('CC','cc'),'-std=gnu99','-g','-Wall','-Wextra','-Wno-unused-parameter','-Wno-unused-variable','-Wno-unused-function','-Wno-sign-compare','-fsanitize=address,undefined',str(c),'-o',str(binary)],capture_output=True,text=True)
  if result.returncode:
   print('FAIL compile',fixture.name); print(result.stderr);failed=True;continue
  try:
   result=subprocess.run([str(binary)],capture_output=True,text=True,timeout=10)
   print(('PASS' if result.returncode==0 else 'FAIL'),fixture.name)
   if result.returncode:print(result.stdout+result.stderr);failed=True
  except subprocess.TimeoutExpired:print('FAIL timeout',fixture.name);failed=True
raise SystemExit(int(failed))
