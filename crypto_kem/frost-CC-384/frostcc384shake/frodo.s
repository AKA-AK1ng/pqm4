.syntax unified
.cpu cortex-m4
.thumb

.global xs
.type xs, %function
.align 2
xs:
    push {r4-r11, lr}
    mov r12, r0
    mov r0, #0
    movw r11, #241
1:
    ldmia r12!, {r2, r3, r4, r5}
    ldmia r1!, {r7, r8, r9, r10}
    smlad r0, r2, r7, r0
    smlad r0, r3, r8, r0
    smlad r0, r4, r9, r0
    smlad r0, r5, r10, r0
    sub.w r11, r11, #1
    cmp r11, #0
    bne 1b
    pop {r4-r11, pc}
