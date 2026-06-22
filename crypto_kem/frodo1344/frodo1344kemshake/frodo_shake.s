.syntax unified
.cpu cortex-m4
.thumb

.equ PARAMS_N_BYTES, 2688


//extern void sa(uint16_t *out,  const uint16_t *s, const int16_t *a)   matrix_shake
.global sa
.type sa, %function
.align 2 
sa:
    push {r4-r11, lr}
    movw r11, #PARAMS_N_BYTES
    // Load a
    ldrh r12, [r2, #0]
    add r2, r2, r11
    ldrh r5, [r2, #0]
    bfi r12, r5, #16, #16
    add r2, r2, r11
    ldrh r14, [r2, #0]
    add r2, r2, r11
    ldrh r5, [r2, #0]
    bfi r14, r5, #16, #16
    add r2, r2, r11
    ldrh r3, [r2, #0]
    add r2, r2, r11
    ldrh r5, [r2, #0]
    bfi r3, r5, #16, #16
    add r2, r2, r11
    ldrh r4, [r2, #0]
    add r2, r2, r11
    ldrh r5, [r2, #0]
    bfi r4, r5, #16, #16
    // First row
    ldmia r1, {r5,r6,r7,r8}
    ldrh r10, [r0, #0]
    smlad r10, r12, r5, r10
    smlad r10, r14, r6, r10
    smlad r10, r3, r7, r10
    smlad r10, r4, r8, r10
    strh r10, [r0, #0]
    // Second row
    add r1, r1, r11
    add r0, r0, r11
    ldmia r1, {r5,r6,r7,r8}
    ldrh r10, [r0, #0]
    smlad r10, r12, r5, r10
    smlad r10, r14, r6, r10
    smlad r10, r3, r7, r10
    smlad r10, r4, r8, r10
    strh r10, [r0, #0]
    // Third row
    add r1, r1, r11
    add r0, r0, r11
    ldmia r1, {r5,r6,r7,r8}
    ldrh r10, [r0, #0]
    smlad r10, r12, r5, r10
    smlad r10, r14, r6, r10
    smlad r10, r3, r7, r10
    smlad r10, r4, r8, r10
    strh r10, [r0, #0]
    // Fourth row
    add r1, r1, r11
    add r0, r0, r11
    ldmia r1, {r5,r6,r7,r8}
    ldrh r10, [r0, #0]
    smlad r10, r12, r5, r10
    smlad r10, r14, r6, r10
    smlad r10, r3, r7, r10
    smlad r10, r4, r8, r10
    strh r10, [r0, #0]
    // Fifth row
    add r1, r1, r11
    add r0, r0, r11
    ldmia r1, {r5,r6,r7,r8}
    ldrh r10, [r0, #0]
    smlad r10, r12, r5, r10
    smlad r10, r14, r6, r10
    smlad r10, r3, r7, r10
    smlad r10, r4, r8, r10
    strh r10, [r0, #0]
    // Sixth row
    add r1, r1, r11
    add r0, r0, r11
    ldmia r1, {r5,r6,r7,r8}
    ldrh r10, [r0, #0]
    smlad r10, r12, r5, r10
    smlad r10, r14, r6, r10
    smlad r10, r3, r7, r10
    smlad r10, r4, r8, r10
    strh r10, [r0, #0]
    // Seventh row
    add r1, r1, r11
    add r0, r0, r11
    ldmia r1, {r5,r6,r7,r8}
    ldrh r10, [r0, #0]
    smlad r10, r12, r5, r10
    smlad r10, r14, r6, r10
    smlad r10, r3, r7, r10
    smlad r10, r4, r8, r10
    strh r10, [r0, #0]
    // Eight row
    add r1, r1, r11
    add r0, r0, r11
    ldmia r1, {r5,r6,r7,r8}
    ldrh r10, [r0, #0]
    smlad r10, r12, r5, r10
    smlad r10, r14, r6, r10
    smlad r10, r3, r7, r10
    smlad r10, r4, r8, r10
    strh r10, [r0, #0]

    pop {r4-r11, pc}
