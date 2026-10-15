#!/usr/bin/env python3
"""Compile and run a focused check against the actual V7 ed source.

The fixture adapts the environment to the host; it does not replace the
function under test. Use --repo-root to check another revision.
"""
import argparse, os, pathlib, re, shlex, subprocess, tempfile

def function(text, name):
    pattern = r'(?m)^[\w *\t]+\b' + re.escape(name) + r'\s*\([^;{}]*\)\s*\{'
    match = re.search(pattern, text)
    if not match:
        raise ValueError('missing function ' + name)
    masked = re.sub(r'/\*[\s\S]*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', lambda m: ' '*len(m.group()), text)
    depth = 0
    for pos in range(text.index('{', match.start()), len(text)):
        if masked[pos] == '{': depth += 1
        if masked[pos] == '}':
            depth -= 1
            if depth == 0: return text[match.start():pos+1]
    raise ValueError('unterminated function ' + name)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo-root', type=pathlib.Path, default=pathlib.Path(__file__).resolve().parents[4])
    parser.add_argument('--cc', default=os.environ.get('CC', 'cc'))
    args = parser.parse_args()
    root = args.repo_root.resolve()
    fixture = pathlib.Path(__file__).with_name('check.c').read_text()
    def source(m):
        text = (root/m[1]).read_text()
        return re.sub(r'^\s*#\s*include[^\n]*', '', text, flags=re.M)
    fixture = re.sub(r'/\* SOURCE ([\w./-]+) \*/', source, fixture)
    fixture = re.sub(r'/\* FUNCTION ([\w./-]+) (\w+) \*/', lambda m: function((root/m[1]).read_text(),m[2]), fixture)
    with tempfile.TemporaryDirectory(prefix='fuzix-v7-check-') as tmp:
        tmp = pathlib.Path(tmp)
        cfile, binary = tmp/'check.c', tmp/'check'
        cfile.write_text(fixture)
        cc = shlex.split(args.cc)
        flags = ['-std=c99','-D_DEFAULT_SOURCE','-g','-O1','-Wall','-Wextra','-Wno-unused-parameter','-Wno-unused-variable','-Wno-unused-function','-Wno-sign-compare','-Wno-missing-field-initializers','-fno-builtin','-fsanitize=address,undefined','-fno-sanitize-recover=all']
        subprocess.run(cc+flags+[str(cfile),'-o',str(binary)],check=True)
        env = dict(os.environ,ASAN_OPTIONS='detect_leaks=0')
        return subprocess.run([str(binary)],timeout=10,env=env).returncode
if __name__ == '__main__':
    raise SystemExit(main())
