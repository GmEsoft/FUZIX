#include "kernel-armm0.def"
.syntax unified
.cpu cortex-m0plus
.thumb
.bss
.balign 8
abi_swap_stack:
    .space 4096
.global abi_swap_stack_top
abi_swap_stack_top:
.text

.macro entry name, mode
.global \name
.thumb_func
\name:
    movs r0, #\mode
    b abi_run
.endm
entry abi_run_switch, 0
entry abi_run_fork, 1
entry abi_run_svc, 2
entry abi_run_exec, 3

.thumb_func
abi_run:
    push {r4, r5, r6, r7, lr}
    ldr r1, =abi_mode
    str r0, [r1]
    mov r0, r8
    mov r1, r9
    mov r2, r10
    mov r3, r11
    push {r0, r1, r2, r3}
    ldr r1, =abi_saved_sp
    mov r2, sp
    str r2, [r1]
    ldr r1, =(progbase + 8192)
    mov sp, r1
    ldr r4, =0x44444444
    ldr r5, =0x55555555
    ldr r6, =0x66666666
    ldr r7, =0x77777777
    ldr r1, =0x88888888
    mov r8, r1
    ldr r1, =0x99999999
    mov r9, r1
    ldr r1, =0xaaaaaaaa
    mov r10, r1
    ldr r1, =0xbbbbbbbb
    mov r11, r1
    ldr r1, =0xcccccccc
    mov r12, r1
    ldr r0, =abi_mode
    ldr r0, [r0]
    cmp r0, #0
    beq 1f
    cmp r0, #1
    beq 2f
    cmp r0, #2
    beq 3f
    ldr r0, =abi_exec_entry
    bl doexec
1:
    bl plt_switchout
    b abi_capture
2:
    ldr r0, =abi_ptab
    bl dofork
    b abi_capture
3:
    movs r1, #3
    movs r2, #4
    movs r3, #5
    svc #0xaa
    b abi_capture

.thumb_func
abi_exec_entry:
    ldr r0, =abi_exec_sp
    mov r1, sp
    str r1, [r0]
    b abi_restore

.thumb_func
abi_capture:
    ldr r2, =abi_result
    str r0, [r2, #0]
    str r1, [r2, #4]
    str r4, [r2, #8]
    str r5, [r2, #12]
    str r6, [r2, #16]
    str r7, [r2, #20]
    mov r3, r8
    str r3, [r2, #24]
    mov r3, r9
    str r3, [r2, #28]
    mov r3, r10
    str r3, [r2, #32]
    mov r3, r11
    str r3, [r2, #36]
    mov r3, r12
    str r3, [r2, #40]
abi_restore:
    ldr r0, =abi_saved_sp
    ldr r0, [r0]
    mov sp, r0
    pop {r0, r1, r2, r3}
    mov r8, r0
    mov r9, r1
    mov r10, r2
    mov r11, r3
    pop {r4, r5, r6, r7, pc}

/* Capture SP before a C prologue can change it. */
.macro capture name, variable, tail=
.global \name
.thumb_func
\name:
    mov r2, sp
    ldr r3, =\variable
    str r2, [r3]
.ifnb \tail
    b \tail
.else
    bx lr
.endif
.endm
capture contextswitch, abi_switch_sp
capture clonecurrentprocess, abi_clone_sp
capture makeproc, abi_makeproc_sp
capture syscall_handler, abi_kernel_sp, abi_platform_syscall_handler
capture deliver_signals, abi_signal_sp, abi_platform_deliver_signals
capture abi_signal_callback, abi_callback_sp

.global getproc
.thumb_func
getproc:
    ldr r0, =abi_ptab
    bx lr
