.syntax unified
.cpu cortex-m4
.thumb


//extern void sa(uint16_t *out,  const uint16_t *s, const int16_t *a)   matrix_shake
.global sa
.type sa, %function
.align 2 
sa:
    push {r4-r11, lr}
    // Load a
    ldrh r12, [r2, #0]
    ldrh r5, [r2, #1024]
    bfi r12, r5, #16, #16
    ldrh r14, [r2, #2048]
    ldrh r5, [r2, #3072]
    bfi r14, r5, #16, #16
    add r2, r2, #4096
    ldrh r3, [r2, #0]
    ldrh r5, [r2, #1024]
    bfi r3, r5, #16, #16
    ldrh r4, [r2, #2048]
    ldrh r5, [r2, #3072]
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
    add r1, r1, #1024
    ldmia r1, {r5,r6,r7,r8}
    ldrh r10, [r0, #1024]
    smlad r10, r12, r5, r10
    smlad r10, r14, r6, r10
    smlad r10, r3, r7, r10
    smlad r10, r4, r8, r10
    strh r10, [r0, #1024]
    // Third row
    add r1, r1, #1024
    ldmia r1, {r5,r6,r7,r8}
    ldrh r10, [r0, #2048]
    smlad r10, r12, r5, r10
    smlad r10, r14, r6, r10
    smlad r10, r3, r7, r10
    smlad r10, r4, r8, r10
    strh r10, [r0, #2048]
    // Fourth row
    add r1, r1, #1024
    ldmia r1, {r5,r6,r7,r8}
    ldrh r10, [r0, #3072]
    smlad r10, r12, r5, r10
    smlad r10, r14, r6, r10
    smlad r10, r3, r7, r10
    smlad r10, r4, r8, r10
    strh r10, [r0, #3072]
    add r0, r0, #4096
    // Fifth row
    add r1, r1, #1024
    ldmia r1, {r5,r6,r7,r8}
    ldrh r10, [r0, #0]
    smlad r10, r12, r5, r10
    smlad r10, r14, r6, r10
    smlad r10, r3, r7, r10
    smlad r10, r4, r8, r10
    strh r10, [r0, #0]
    // Sixth row
    add r1, r1, #1024
    ldmia r1, {r5,r6,r7,r8}
    ldrh r10, [r0, #1024]
    smlad r10, r12, r5, r10
    smlad r10, r14, r6, r10
    smlad r10, r3, r7, r10
    smlad r10, r4, r8, r10
    strh r10, [r0, #1024]
    // Seventh row
    add r1, r1, #1024
    ldmia r1, {r5,r6,r7,r8}
    ldrh r10, [r0, #2048]
    smlad r10, r12, r5, r10
    smlad r10, r14, r6, r10
    smlad r10, r3, r7, r10
    smlad r10, r4, r8, r10
    strh r10, [r0, #2048]
    // Eight row
    add r1, r1, #1024
    ldmia r1, {r5,r6,r7,r8}
    ldrh r10, [r0, #3072]
    smlad r10, r12, r5, r10
    smlad r10, r14, r6, r10
    smlad r10, r3, r7, r10
    smlad r10, r4, r8, r10
    strh r10, [r0, #3072]

    pop {r4-r11, pc}
