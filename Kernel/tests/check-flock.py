#!/usr/bin/env python3
"""Exercise the production flock/deflock bodies with a small inode/OFT fixture."""
from pathlib import Path
import subprocess
import tempfile

kernel = Path(__file__).resolve().parent.parent

def function(path, signature):
    source = path.read_text()
    start = source.index(signature)
    brace = source.index('{', start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

fixture = r'''
#include <assert.h>
#include <stdint.h>
#include <errno.h>
#define staticfast static
#define LOCK_SH 0
#define LOCK_EX 1
#define LOCK_UN 2
#define LOCK_NB 4
#define O_FLOCK 8
#define CFLOCK 7
#define CFLEX 7
#define CFMAX 6
#define NULLINODE ((inoptr)0)
typedef int arg_t;
struct inode { uint8_t c_flags; } inode;
typedef struct inode *inoptr;
struct oft { inoptr o_inode; uint8_t o_access; } of_tab[2];
struct { unsigned u_argn, u_argn1; uint8_t u_files[2]; int u_error; } udata;
static inoptr getinode(unsigned fd) { return of_tab[udata.u_files[fd]].o_inode; }
static void wakeup(void *p) { (void)p; }
static int psleep_flags(void *p, unsigned flags) {
    (void)p; (void)flags; udata.u_error = EAGAIN; return -1;
}
'''
fixture += function(kernel / 'filesys.c', 'void deflock(')
fixture += '\n#define file ((uint16_t)udata.u_argn)\n#define lockop ((uint16_t)udata.u_argn1)\n'
fixture += function(kernel / 'syscall_fs3.c', 'arg_t _flock(')
fixture += '\n#undef file\n#undef lockop\n'
fixture += r'''
static int lock(unsigned fd, unsigned op) {
    udata.u_argn = fd; udata.u_argn1 = op; udata.u_error = 0;
    return _flock();
}
int main(void) {
    of_tab[0].o_inode = of_tab[1].o_inode = &inode;
    udata.u_files[0] = 0; udata.u_files[1] = 1;
    assert(lock(0, LOCK_EX | LOCK_NB) == 0);
    assert(lock(1, LOCK_EX | LOCK_NB) == -1);
    assert(lock(0, LOCK_UN) == 0);
    assert(!(inode.c_flags & CFLOCK) && !(of_tab[0].o_access & O_FLOCK));
    assert(lock(0, LOCK_UN) == 0); /* repeated unlock is harmless */
    assert(lock(1, LOCK_EX | LOCK_NB) == 0); /* another open file can acquire */
    deflock(&of_tab[1]); of_tab[1].o_access = 0; /* last close releases */
    assert(!(inode.c_flags & CFLOCK));
    assert(lock(0, LOCK_SH | LOCK_NB) == 0);
    assert(lock(1, LOCK_SH | LOCK_NB) == 0);
    assert((inode.c_flags & CFLOCK) == 2);
    assert(lock(0, LOCK_UN) == 0);
    assert((inode.c_flags & CFLOCK) == 1);
    assert(lock(1, LOCK_UN) == 0);
    assert(!(inode.c_flags & CFLOCK));
}
'''
with tempfile.TemporaryDirectory(prefix='fuzix-flock-') as directory:
    source = Path(directory) / 'flock.c'
    exe = Path(directory) / 'flock'
    source.write_text(fixture)
    subprocess.run(['cc', '-std=c99', '-Wall', '-Wextra', '-fsanitize=address,undefined',
                    str(source), '-o', str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
    # The same behavioral fixture must fail with the former unlock ordering.
    old = fixture.replace('deflock(o);\n\t\to->o_access &= ~O_FLOCK;',
                          'o->o_access &= ~O_FLOCK;\n\t\tdeflock(o);')
    assert old != fixture
    source.write_text(old)
    subprocess.run(['cc', '-std=c99', str(source), '-o', str(exe)], check=True)
    failed = subprocess.run([str(exe)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    assert failed.returncode != 0, 'former unlock sequence unexpectedly passed'
print('PASS: exclusive/shared unlock, repeated unlock, reacquire and last close')
