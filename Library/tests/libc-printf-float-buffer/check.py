#!/usr/bin/env python3
"""Compile and run a focused check against the actual library source.

The fixture adapts FUZIX types and syscalls to the host; it does not replace
function bodies. Use --library-dir to check another revision.
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
    parser.add_argument('--library-dir', type=pathlib.Path, default=pathlib.Path(__file__).resolve().parents[2])
    parser.add_argument('--cc', default=os.environ.get('CC', 'cc'))
    parser.add_argument('--binary32', action='store_true', help='check the single-precision target profile')
    args = parser.parse_args()
    library = args.library_dir.resolve()
    fixture = pathlib.Path(__file__).with_name('check.c').read_text()
    def source(m):
        text = (library/m[1]).read_text()
        return re.sub(r'^\s*#\s*include[^\n]*', '', text, flags=re.M)
    fixture = re.sub(r'/\* SOURCE ([\w./-]+) \*/', source, fixture)
    fixture = re.sub(r'/\* FUNCTION ([\w./-]+) (\w+) \*/', lambda m: function((library/m[1]).read_text(),m[2]), fixture)
    if args.binary32:
        # Host varargs promote floats to binary64 even when modeling a binary32 target.
        fixture = fixture.replace('va_arg(ap, double)', '(float)va_arg(ap, host_double)')
    if '/* STDIO */' in fixture:
        text = (library/'include/stdio.h').read_text()
        modes = '\n'.join(re.findall(r'^#define\s+(?:__MODE_\w+|_IOFBF|_IOLBF|_IONBF|BUFSIZ)\s+[^\n]+',text,re.M))
        struct = re.search(r'struct __stdio_file \{[\s\S]*?\n\};',text).group()
        fixture = fixture.replace('/* STDIO */', 'typedef unsigned char uchar;\n'+modes+'\n'+struct+'\ntypedef struct __stdio_file FuzixFILE;\n#define FILE FuzixFILE\n#define ferror(fp) (((fp)->mode & __MODE_ERR) != 0)\n')
    with tempfile.TemporaryDirectory(prefix='fuzix-lib-check-') as tmp:
        tmp = pathlib.Path(tmp)
        cfile, binary = tmp/'check.c', tmp/'check'
        cfile.write_text(fixture)
        cc = shlex.split(args.cc)
        flags = ['-std=c99','-D_DEFAULT_SOURCE','-g','-O1','-Wall','-Wextra','-Wno-unused-parameter','-Wno-unused-variable','-Wno-unused-function','-Wno-sign-compare','-Wno-missing-field-initializers','-fno-builtin','-fsanitize=address,undefined','-fno-sanitize-recover=all']
        if args.binary32:
            flags.append('-DTEST_BINARY32')
        subprocess.run(cc+flags+[str(cfile),'-o',str(binary)],check=True)
        env = dict(os.environ,ASAN_OPTIONS='detect_leaks=0')
        return subprocess.run([str(binary)],timeout=10,env=env).returncode
if __name__ == '__main__':
    raise SystemExit(main())
