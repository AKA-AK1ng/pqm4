.syntax unified
.thumb
.text
.align 2

/* void viper_asm_unpack12_centered(int16_t *out, const uint8_t *in)
 *
 * Decode two little-endian packed 12-bit coefficients from every three input
 * bytes.  SBFX performs the q=4096 centering for free by sign-extending bit 11.
 */
.global viper_asm_unpack12_centered
.type viper_asm_unpack12_centered, %function
viper_asm_unpack12_centered:
    push    {r4-r6, lr}
    movs    r2, #128
.Lunpack12_loop:
    ldrb    r3, [r1], #1
    ldrb    r4, [r1], #1
    ldrb    r5, [r1], #1
    orr     r3, r3, r4, lsl #8
    sbfx    r6, r3, #0, #12
    lsrs    r4, r4, #4
    orr     r4, r4, r5, lsl #4
    sbfx    r4, r4, #0, #12
    strh    r6, [r0], #2
    strh    r4, [r0], #2
    subs    r2, r2, #1
    bne     .Lunpack12_loop
    pop     {r4-r6, pc}
.size viper_asm_unpack12_centered, .-viper_asm_unpack12_centered

.macro recon_q word, qbit, dbit
    ubfx    r7, \word, #\qbit, #9
    ubfx    r8, r6, #\dbit, #3
    sub     r7, r8, r7, lsl #3
    rsb     r7, r7, #0
    sbfx    r7, r7, #0, #12
    strh    r7, [r0], #2
.endm

/* Decode one 9-bit U polynomial and its packed 3-bit dither directly into
 * centered int16 coefficients.  Each iteration consumes 9 + 3 bytes and
 * emits eight coefficients, matching the natural LWQ packing block. */
.global viper_asm_unpack_reconstruct_t9_d3_centered
.type viper_asm_unpack_reconstruct_t9_d3_centered, %function
viper_asm_unpack_reconstruct_t9_d3_centered:
    push    {r4-r10, lr}
    movs    r3, #32
.Lu_reconstruct_loop:
    ldr     r4, [r1]
    ldr     r5, [r1, #4]
    ldrb    r9, [r1, #8]
    add     r1, r1, #9

    ldrb    r6, [r2]
    ldrb    r8, [r2, #1]
    orr     r6, r6, r8, lsl #8
    ldrb    r8, [r2, #2]
    orr     r6, r6, r8, lsl #16
    add     r2, r2, #3

    recon_q r4, 0, 0
    recon_q r4, 9, 3
    recon_q r4, 18, 6

    lsrs    r7, r4, #27
    orr     r7, r7, r5, lsl #5
    ubfx    r8, r6, #9, #3
    sub     r7, r8, r7, lsl #3
    rsb     r7, r7, #0
    sbfx    r7, r7, #0, #12
    strh    r7, [r0], #2

    recon_q r5, 4, 12
    recon_q r5, 13, 15
    recon_q r5, 22, 18

    lsrs    r7, r5, #31
    orr     r7, r7, r9, lsl #1
    ubfx    r8, r6, #21, #3
    sub     r7, r8, r7, lsl #3
    rsb     r7, r7, #0
    sbfx    r7, r7, #0, #12
    strh    r7, [r0], #2

    subs    r3, r3, #1
    bne     .Lu_reconstruct_loop
    pop     {r4-r10, pc}
.size viper_asm_unpack_reconstruct_t9_d3_centered, .-viper_asm_unpack_reconstruct_t9_d3_centered

.macro quant_u_q poff, dbit
    ldrh    r7, [r1, #\poff]
    ubfx    r12, r4, #\dbit, #3
    add     r7, r7, r12
    ubfx    r7, r7, #0, #12
    add     r7, r7, #4
    ubfx    r7, r7, #3, #9
.endm

.macro load_u_dither
    ldrb    r4, [r2]
    ldrb    r8, [r2, #1]
    orr     r4, r4, r8, lsl #8
    ldrb    r8, [r2, #2]
    orr     r4, r4, r8, lsl #16
    add     r2, r2, #3
.endm

.macro make_u_words
    quant_u_q 0, 0
    mov     r5, r7
    quant_u_q 2, 3
    orr     r5, r5, r7, lsl #9
    quant_u_q 4, 6
    orr     r5, r5, r7, lsl #18
    quant_u_q 6, 9
    orr     r5, r5, r7, lsl #27
    lsrs    r6, r7, #5
    quant_u_q 8, 12
    orr     r6, r6, r7, lsl #4
    quant_u_q 10, 15
    orr     r6, r6, r7, lsl #13
    quant_u_q 12, 18
    orr     r6, r6, r7, lsl #22
    quant_u_q 14, 21
    orr     r6, r6, r7, lsl #31
.endm

.global viper_asm_quantize_pack_t9_d3
.type viper_asm_quantize_pack_t9_d3, %function
viper_asm_quantize_pack_t9_d3:
    push    {r4-r10, lr}
    movs    r3, #32
.Lu_pack_loop:
    load_u_dither
    make_u_words
    str     r5, [r0]
    str     r6, [r0, #4]
    lsrs    r7, r7, #1
    strb    r7, [r0, #8]
    add     r0, r0, #9
    add     r1, r1, #16
    subs    r3, r3, #1
    bne     .Lu_pack_loop
    pop     {r4-r10, pc}
.size viper_asm_quantize_pack_t9_d3, .-viper_asm_quantize_pack_t9_d3

.global viper_asm_quantize_pack_cmp_t9_d3
.type viper_asm_quantize_pack_cmp_t9_d3, %function
viper_asm_quantize_pack_cmp_t9_d3:
    push    {r4-r11, lr}
    mov     r10, r0
    movs    r11, #0
    movs    r3, #32
.Lu_compare_loop:
    load_u_dither
    make_u_words
    ldr     r8, [r10]
    eor     r8, r8, r5
    orr     r11, r11, r8
    ldr     r8, [r10, #4]
    eor     r8, r8, r6
    orr     r11, r11, r8
    ldrb    r8, [r10, #8]
    eor     r8, r8, r7, lsr #1
    orr     r11, r11, r8
    add     r10, r10, #9
    add     r1, r1, #16
    subs    r3, r3, #1
    bne     .Lu_compare_loop
    mov     r0, r11
    pop     {r4-r11, pc}
.size viper_asm_quantize_pack_cmp_t9_d3, .-viper_asm_quantize_pack_cmp_t9_d3

.macro quant_pk_q poff, dbit
    ldrh    r7, [r1, #\poff]
    ubfx    r12, r4, #\dbit, #2
    add     r7, r7, r12
    ubfx    r7, r7, #0, #12
    add     r7, r7, #4
    ubfx    r7, r7, #3, #9
.endm

.global viper_asm_quantize_pack_pk_t9_d2
.type viper_asm_quantize_pack_pk_t9_d2, %function
viper_asm_quantize_pack_pk_t9_d2:
    push    {r4-r10, lr}
    movs    r3, #32
.Lpk_pack_loop:
    ldrb    r4, [r2]
    ldrb    r8, [r2, #1]
    orr     r4, r4, r8, lsl #8
    add     r2, r2, #2

    quant_pk_q 0, 0
    mov     r5, r7
    quant_pk_q 2, 2
    orr     r5, r5, r7, lsl #9
    quant_pk_q 4, 4
    orr     r5, r5, r7, lsl #18
    quant_pk_q 6, 6
    orr     r5, r5, r7, lsl #27
    lsrs    r6, r7, #5
    quant_pk_q 8, 8
    orr     r6, r6, r7, lsl #4
    quant_pk_q 10, 10
    orr     r6, r6, r7, lsl #13
    quant_pk_q 12, 12
    orr     r6, r6, r7, lsl #22
    quant_pk_q 14, 14
    orr     r6, r6, r7, lsl #31

    str     r5, [r0]
    str     r6, [r0, #4]
    lsrs    r7, r7, #1
    strb    r7, [r0, #8]
    add     r0, r0, #9
    add     r1, r1, #16
    subs    r3, r3, #1
    bne     .Lpk_pack_loop
    pop     {r4-r10, pc}
.size viper_asm_quantize_pack_pk_t9_d2, .-viper_asm_quantize_pack_pk_t9_d2

.macro recon_pk_q word, qbit, dbit
    ubfx    r7, \word, #\qbit, #9
    ubfx    r8, r6, #\dbit, #2
    sub     r7, r8, r7, lsl #3
    rsb     r7, r7, #0
    sbfx    r7, r7, #0, #12
    strh    r7, [r0], #2
.endm

.global viper_asm_unpack_reconstruct_pk_centered_t9_d2
.type viper_asm_unpack_reconstruct_pk_centered_t9_d2, %function
viper_asm_unpack_reconstruct_pk_centered_t9_d2:
    push    {r4-r10, lr}
    movs    r3, #32
.Lpk_reconstruct_loop:
    ldr     r4, [r1]
    ldr     r5, [r1, #4]
    ldrb    r9, [r1, #8]
    add     r1, r1, #9
    ldrb    r6, [r2]
    ldrb    r8, [r2, #1]
    orr     r6, r6, r8, lsl #8
    add     r2, r2, #2

    recon_pk_q r4, 0, 0
    recon_pk_q r4, 9, 2
    recon_pk_q r4, 18, 4
    lsrs    r7, r4, #27
    orr     r7, r7, r5, lsl #5
    ubfx    r8, r6, #6, #2
    sub     r7, r8, r7, lsl #3
    rsb     r7, r7, #0
    sbfx    r7, r7, #0, #12
    strh    r7, [r0], #2
    recon_pk_q r5, 4, 8
    recon_pk_q r5, 13, 10
    recon_pk_q r5, 22, 12
    lsrs    r7, r5, #31
    orr     r7, r7, r9, lsl #1
    ubfx    r8, r6, #14, #2
    sub     r7, r8, r7, lsl #3
    rsb     r7, r7, #0
    sbfx    r7, r7, #0, #12
    strh    r7, [r0], #2

    subs    r3, r3, #1
    bne     .Lpk_reconstruct_loop
    pop     {r4-r10, pc}
.size viper_asm_unpack_reconstruct_pk_centered_t9_d2, .-viper_asm_unpack_reconstruct_pk_centered_t9_d2

.macro eta_centered bit0, bit1
    ubfx    r4, r3, #\bit0, #1
    ubfx    r5, r3, #\bit1, #1
    sub     r4, r4, r5
    strh    r4, [r0], #2
.endm

.global viper_asm_eta1_decode_centered
.type viper_asm_eta1_decode_centered, %function
viper_asm_eta1_decode_centered:
    push    {r4-r6, lr}
    movs    r2, #64
.Leta_centered_loop:
    ldr     r3, [r1], #4
    eta_centered 0, 1
    eta_centered 8, 9
    eta_centered 16, 17
    eta_centered 24, 25
    subs    r2, r2, #1
    bne     .Leta_centered_loop
    pop     {r4-r6, pc}
.size viper_asm_eta1_decode_centered, .-viper_asm_eta1_decode_centered

.macro eta_modq bit0, bit1
    ubfx    r4, r3, #\bit0, #1
    ubfx    r5, r3, #\bit1, #1
    sub     r4, r4, r5
    ubfx    r4, r4, #0, #12
    strh    r4, [r0], #2
.endm

.global viper_asm_eta1_decode_modq
.type viper_asm_eta1_decode_modq, %function
viper_asm_eta1_decode_modq:
    push    {r4-r6, lr}
    movs    r2, #64
.Leta_modq_loop:
    ldr     r3, [r1], #4
    eta_modq 0, 1
    eta_modq 8, 9
    eta_modq 16, 17
    eta_modq 24, 25
    subs    r2, r2, #1
    bne     .Leta_modq_loop
    pop     {r4-r6, pc}
.size viper_asm_eta1_decode_modq, .-viper_asm_eta1_decode_modq

.global viper_asm_pack_secret12
.type viper_asm_pack_secret12, %function
viper_asm_pack_secret12:
    push    {r4-r6, lr}
    movs    r2, #128
.Lsecret_pack_loop:
    ldrh    r3, [r1], #2
    ldrh    r4, [r1], #2
    ubfx    r3, r3, #0, #12
    ubfx    r4, r4, #0, #12
    orr     r3, r3, r4, lsl #12
    strh    r3, [r0], #2
    lsrs    r3, r3, #16
    strb    r3, [r0], #1
    subs    r2, r2, #1
    bne     .Lsecret_pack_loop
    pop     {r4-r6, pc}
.size viper_asm_pack_secret12, .-viper_asm_pack_secret12
