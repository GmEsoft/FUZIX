;
;	For simple banked systems there is a standard implementation. The
;	only reason to do otherwise is for speed. A custom bank logic aware
;	bank to bank copier will give vastly better fork() performance.
;
#include "kernelu.def"
#include "../../cpu-z80u/kernel-z80.def"
; For the moment to get it up and running
#include "../../lib/z80ufixedbank.S"
