/*
 * Copyright © 2006-2008, 2013 Siarhei Siamashka <siarhei.siamashka@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Copyright 2013 Harm Hanemaaijer <fgenfb@yahoo.com>
 *
 * 1. Add ".type <function_name>, function" to function definition macro, which
 *    was required for correct linkage on my platform.
 * 2. Add non-overfetching memcpy version with a plethora of optimizations and variants using
 *    macros.
 *    To do: -- More complete implementation of write_align == 64 for unaligned case.
 *
 * On the RPi platform, a good choice is armv5te_no_overfetch_align_16_block_write_16_preload_early_128,
 * closely followed by armv5te_no_overfetch_align_16_block_write_16_preload_early_96. For
 * CPU-cache based work loads armv5te_no_overfetch_align_16_block_write_16_preload_96 might be
 * a little faster.
 *
 * On the Allwinner A10 platform, with the reworked version a variant with cache line size of 64,
 * memcpy_armv5te_no_overfetch_line_64_align_32_block_write_32_preload_early_192, seems to be the
 * best performer.
 *
 * On the Allwinner platform, the optimized memcpy is faster; on the RPi libcofi does relatively well
 * and the optimal memcpy depends on the application.
 */

/* Prevent the stack from becoming executable */
#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif

#ifdef __arm__

.text
.syntax unified
.fpu neon
.arch armv7a
.object_arch armv4
.arm
.altmacro
.p2align 2

/******************************************************************************/

.macro asm_function function_name
    .global \function_name
.func \function_name
.type \function_name, function
.p2align 5
\function_name:
.endm

/******************************************************************************/

#if !defined(MEMCPY_REPLACEMENT_SUNXI) && !defined(MEMCPY_REPLACEMENT_RPI)

/*
 * Helper macro for memcpy function, it can copy data from source (r1) to 
 * destination (r0) buffers fixing alignment in the process. Destination
 * buffer should be aligned already (4 bytes alignment is required.
 * Size of the block to copy is in r2 register
 */
.macro  UNALIGNED_MEMCPY shift
    sub     r1, #(\shift)
    ldr     ip, [r1], #4

    tst     r0, #4
    movne   r3, ip, lsr #(\shift * 8)
    ldrne   ip, [r1], #4
    subne   r2, r2, #4
    orrne   r3, r3, ip, asl #(32 - \shift * 8)
    strne   r3, [r0], #4

    tst     r0, #8
    movne   r3, ip, lsr #(\shift * 8)
    ldmiane r1!, {r4, ip}
    subne   r2, r2, #8
    orrne   r3, r3, r4, asl #(32 - \shift * 8)
    movne   r4, r4, lsr #(\shift * 8)
    orrne   r4, r4, ip, asl #(32 - \shift * 8)
    stmiane r0!, {r3-r4}
    cmp     r2, #32
    blt     3f
    pld     [r1, #48]
    stmfd   sp!, {r7, r8, r9, r10, r11}
    add     r3, r1, #128
    bic     r3, r3, #31
    sub     r9, r3, r1
1:
    pld     [r1, r9]
    subs    r2, r2, #32
    movge   r3, ip, lsr #(\shift * 8)
    ldmiage r1!, {r4-r6, r7, r8, r10, r11, ip}
    orrge   r3, r3, r4, asl #(32 - \shift * 8)
    movge   r4, r4, lsr #(\shift * 8)
    orrge   r4, r4, r5, asl #(32 - \shift * 8)
    movge   r5, r5, lsr #(\shift * 8)
    orrge   r5, r5, r6, asl #(32 - \shift * 8)
    movge   r6, r6, lsr #(\shift * 8)
    orrge   r6, r6, r7, asl #(32 - \shift * 8)
    stmiage r0!, {r3-r6}
    movge   r7, r7, lsr #(\shift * 8)
    orrge   r7, r7, r8, asl #(32 - \shift * 8)
    movge   r8, r8, lsr #(\shift * 8)
    orrge   r8, r8, r10, asl #(32 - \shift * 8)
    movge   r10, r10, lsr #(\shift * 8)
    orrge   r10, r10, r11, asl #(32 - \shift * 8)
    movge   r11, r11, lsr #(\shift * 8)
    orrge   r11, r11, ip, asl #(32 - \shift * 8)
    stmiage r0!, {r7, r8, r10, r11}
    bgt     1b
2:
    ldmfd   sp!, {r7, r8, r9, r10, r11}
3:  /* copy remaining data */
    tst     r2, #16
    movne   r3, ip, lsr #(\shift * 8)
    ldmiane r1!, {r4-r6, ip}
    orrne   r3, r3, r4, asl #(32 - \shift * 8)
    movne   r4, r4, lsr #(\shift * 8)
    orrne   r4, r4, r5, asl #(32 - \shift * 8)
    movge   r5, r5, lsr #(\shift * 8)
    orrge   r5, r5, r6, asl #(32 - \shift * 8)
    movge   r6, r6, lsr #(\shift * 8)
    orrge   r6, r6, ip, asl #(32 - \shift * 8)
    stmiane r0!, {r3-r6}

    tst     r2, #8
    movne   r3, ip, lsr #(\shift * 8)
    ldmiane r1!, {r4, ip}
    orrne   r3, r3, r4, asl #(32 - \shift * 8)
    movne   r4, r4, lsr #(\shift * 8)
    orrne   r4, r4, ip, asl #(32 - \shift * 8)
    stmiane r0!, {r3-r4}

    tst     r2, #4
    movne   r3, ip, lsr #(\shift * 8)
    ldrne   ip, [r1], #4
    sub     r1, r1, #(4 - \shift)
    orrne   r3, r3, ip, asl #(32 - \shift * 8)
    strne   r3, [r0], #4

    tst     r2, #2
    ldrbne  r3, [r1], #1
    ldrbne  r4, [r1], #1
    ldr     r5, [sp], #4
    strbne  r3, [r0], #1
    strbne  r4, [r0], #1

    tst     r2, #1
    ldrbne  r3, [r1], #1
    ldr     r6, [sp], #4
    strbne  r3, [r0], #1

    pop     {r0, r4}

    bx      lr
.endm

/*
 * Memcpy function with Raspberry Pi specific aligned prefetch, based on
 * https://garage.maemo.org/plugins/scmsvn/viewcvs.php/mplayer/trunk/fastmem-arm9/fastmem-arm9.S
 */
asm_function memcpy_armv5te
    cmp     r2, #20
    blt     9f
    /* copy data until destination address is 4 bytes aligned */
    tst     r0, #1
    ldrbne  r3, [r1], #1
    stmfd   sp!, {r0, r4}
    subne   r2, r2, #1
    strbne  r3, [r0], #1
    tst     r0, #2
    ldrbne  r3, [r1], #1
    ldrbne  r4, [r1], #1
    stmfd   sp!, {r5, r6}
    subne   r2, r2, #2
    orrne   r3, r3, r4, asl #8
    strhne  r3, [r0], #2
    /* destination address is 4 bytes aligned */
    /* now we should handle 4 cases of source address alignment */
    tst     r1, #1
    bne     6f
    tst     r1, #2
    bne     7f

    /* both source and destination are 4 bytes aligned */
    stmfd   sp!, {r7, r8, r9, r10, r11}
    tst     r0, #4
    ldrne   r4, [r1], #4
    subne   r2, r2, #4
    strne   r4, [r0], #4
    tst     r0, #8
    ldmiane r1!, {r3-r4}
    add     r9, r1, #96
    subne   r2, r2, #8
    bic     r9, r9, #31
    stmiane r0!, {r3-r4}
    sub     r9, r9, r1
1:
    subs    r2, r2, #32
    ldmiage r1!, {r3-r6, r7, r8, r10, r11}
    pld     [r1, r9]
    stmiage r0!, {r3-r6}
    stmiage r0!, {r7, r8, r10, r11}
    bgt     1b
2:
    ldmfd   sp!, {r7, r8, r9, r10, r11}
    tst     r2, #16
    ldmiane r1!, {r3-r6}
    stmiane r0!, {r3-r6}
    tst     r2, #8
    ldmiane r1!, {r3-r4}
    stmiane r0!, {r3-r4}
    tst     r2, #4
    ldrne   r3, [r1], #4
    mov     ip, r0
    strne   r3, [ip], #4
    tst     r2, #2
    ldrhne  r3, [r1], #2
    ldmfd   sp!, {r5, r6}
    strhne  r3, [ip], #2
    tst     r2, #1
    ldrbne  r3, [r1], #1
    ldmfd   sp!, {r0, r4}
    strbne  r3, [ip], #1

    bx      lr

6:
    tst    r1, #2
    bne    8f
    UNALIGNED_MEMCPY 1
7:
    UNALIGNED_MEMCPY 2
8:
    UNALIGNED_MEMCPY 3
9:
    stmfd  sp!, {r0, r4}
1:  subs   r2, r2, #3
    ldrbge ip, [r0]
    ldrbge r3, [r1], #1
    ldrbge r4, [r1], #1
    ldrbge ip, [r1], #1
    strbge r3, [r0], #1
    strbge r4, [r0], #1
    strbge ip, [r0], #1
    bge    1b
    adds   r2, r2, #2
    ldrbge r3, [r1], #1
    mov    ip, r0
    ldr    r0, [sp], #4
    strbge r3, [ip], #1
    ldrbgt r3, [r1], #1
    ldr    r4, [sp], #4
    strbgt r3, [ip], #1
    bx     lr
.endfunc

#endif

/*
 * PRELOAD_CATCH_UP enables catching up the early preload offset with the preload offset in
 * the main loop.
 */

#define PRELOAD_CATCH_UP

/*
 * CHECK_EARLY_PRELOADS enables checks to avoid overfetching beyond the source region when
 * doing early preloads. This is currently only implemented for the unaligned case.
 * Due to the overhead it adds this option may not improve performance.
 */
// #define CHECK_EARLY_PRELOADS

/*
 * Allow unaligned memory access.
 */

#define UNALIGNED_ACCESS

/*
 * Helper macro for non-overfetching version.
 *
 * If preload_early == 1,
 * r6 is the address of the 32-byte aligned region containing the last source byte.
 * r3 is the address of the 32-byte aligned region where the first preload occurred, preloads
 * have occurred up to [r3 + line_size].
 *
 * Registers up to r7 have been saved on the stack.
 */

.macro  UNALIGNED_MEMCPY_VARIANT granularity, shift, line_size, write_align, block_write_size, preload_offset, preload_early, overfetch
    sub     r1, #(\shift)
.if \preload_early == 1
    add     r7, r3, #(\line_size * 2)
.endif
    ldr     ip, [r1], #4
.if \preload_early == 1
#ifdef CHECK_EARLY_PRELOADS
.if \overfetch == 0
    cmp     r6, r7
    /* Only preload if the source region extends into it. */
    blt     5f
.endif
#endif
    pld     [r7]
5:
.endif

    tst     r0, #4
    movne   r3, ip, lsr #(\shift * 8)
    ldrne   ip, [r1], #4
    subne   r2, r2, #4
    orrne   r3, r3, ip, asl #(32 - \shift * 8)
    strne   r3, [r0], #4

    tst     r0, #8
    movne   r3, ip, lsr #(\shift * 8)
    ldmiane r1!, {r4, ip}
    subne   r2, r2, #8
    orrne   r3, r3, r4, asl #(32 - \shift * 8)
    movne   r4, r4, lsr #(\shift * 8)
    orrne   r4, r4, ip, asl #(32 - \shift * 8)
    stmiane r0!, {r3-r4}

.if \write_align >= 32
    tst     r0, #16
    movne   r3, ip, lsr #(\shift * 8)
    beq     5f
    ldmia   r1!, {r4-r6, ip}
    sub     r2, r2, #16
    orr     r3, r3, r4, asl #(32 - \shift * 8)
    mov     r4, r4, lsr #(\shift * 8)
.if \write_align == 32
    cmp     r2, #32
.endif
    orr     r4, r4, r5, asl #(32 - \shift * 8)
    mov     r5, r5, lsr #(\shift * 8)
    orr     r5, r5, r6, asl #(32 - \shift * 8)
    mov     r6, r6, lsr #(\shift * 8)
    orr     r6, r6, ip, asl #(32 - \shift * 8)
    stmia   r0!, {r3-r6}
.if \write_align == 32
    blt     3f
    b       1f
.endif
5:
.endif

.if \write_align == 64
    tst     r0, #32
    movne   r3, ip, lsr #(\shift * 8)
    beq     5f
    ldmia   r1!, {r4-r6, ip}
    sub     r2, r2, #32
    orr     r3, r3, r4, asl #(32 - \shift * 8)
    mov     r4, r4, lsr #(\shift * 8)
    cmp     r2, #32
    orr     r4, r4, r5, asl #(32 - \shift * 8)
    mov     r5, r5, lsr #(\shift * 8)
    orr     r5, r5, r6, asl #(32 - \shift * 8)
    mov     r6, r6, lsr #(\shift * 8)
    orr     r6, r6, ip, asl #(32 - \shift * 8)
    stmia   r0!, {r3-r6}
    mov     r3, ip, lsr #(\shift * 8)
    ldmia   r1!, {r4-r6, ip}
    orr     r3, r3, r4, asl #(32 - \shift * 8)
    mov     r4, r4, lsr #(\shift * 8)
    orr     r4, r4, r5, asl #(32 - \shift * 8)
    mov     r5, r5, lsr #(\shift * 8)
    orr     r5, r5, r6, asl #(32 - \shift * 8)
    mov     r6, r6, lsr #(\shift * 8)
    orr     r6, r6, ip, asl #(32 - \shift * 8)
    stmia   r0!, {r3-r6}
    blt     3f
    b       1f
5:
.endif

    cmp     r2, #32
    blt     3f
1:
.if \preload_offset != 0
.if \overfetch == 1
    cmp     r2, #64
.else
    cmp     r2, #\preload_offset
.endif
.endif
    stmfd   sp!, {r8, r9, r10, r11}
.if \preload_offset != 0
    add     r10, r1, #\preload_offset
#ifdef PRELOAD_CATCH_UP
.if \preload_early == 1 && \preload_offset >= 64 && \block_write_size >= 16
    add     r7, r7, #(\line_size * 2)
.endif
#endif
    bic     r10, r10, #(\line_size - 1)
    sub     r9, r10, r1
.if \overfetch == 0
    /* If there are <= preload_offset bytes to go, skip the main loop. */
    ble     4f
.else
    blt     1f
.endif
.if \preload_early == 1 && \preload_offset >= 64 && \block_write_size >= 16
    /*
     * At this point, if overfetch is 0, there are at least preload_offset
     * bytes left, so when CHECK_EARLY_PRELOAD is set, we only need to
     * perform a check if it is possible that the preload overfetches,
     * given that the upcoming early preload is the 4th one (making a
     * total of line_size * 4 byte preloaded from the 32-byte aligned
     * start address).
     */
#ifdef PRELOAD_CATCH_UP
#ifdef CHECK_EARLY_PRELOADS
.if \preload_offset < (\line_size * 4)
    add     r3, r1, r2
    mov     r11, r7
    sub     r3, r3, #1
    sub     r7, #\line_size
    bic     r3, r3, #(\line_size - 1)
    cmp     r7, r3
    add     r7, #\line_size
    bgt     5f
    pld     [r7]
5:
.else
    mov     r11, r7
    pld     [r7, #-\line_size]
.endif
#else
    mov     r11, r7
    pld     [r7, #-\line_size]
#endif
#else
#ifdef CHECK_EARLY_PRELOADS
.if \preload_offset < (\line_size * 4)
    add     r3, r1, r2
    add     r7, #\line_size
    sub     r3, r3, #1
    bic     r3, r3, #(\line_size - 1)
    cmp     r7, r3
    bgt     5f
    pld     [r7]
5:
.else
    pld     [r7, #\line_size]
.endif
#else
    pld     [r7, #\line_size]
#endif
#endif
#ifdef PRELOAD_CATCH_UP
    /*
     * The last preload already done is at [r11 - line_size].
     * The next preload in the main loop will happen at [r10].
     * If r11 < r10, we want to do an extra preload at [r11].
     * Note if write alignment is 64, it may become unaligned.
     */
18:
    cmp     r11, r10
    movlt   r3, ip, lsr #(\shift * 8)
    ldmialt r1!, {r4-r6, r7}
    add     r11, #64
    orrlt   r3, r3, r4, asl #(32 - \shift * 8)
    movlt   r4, r4, lsr #(\shift * 8)
    bge     1f
    cmp     r2, #(\preload_offset + 32)
    pld     [r11, #-64]
    sub     r2, r2, #32
    orr     r4, r4, r5, asl #(32 - \shift * 8)
    mov     r5, r5, lsr #(\shift * 8)
    orr     r5, r5, r6, asl #(32 - \shift * 8)
    mov     r6, r6, lsr #(\shift * 8)
    orr     r6, r6, r7, asl #(32 - \shift * 8)
    mov     r7, r7, lsr #(\shift * 8)
    stmia   r0!, {r3-r6}
    mov     r3, r7
    ldmia   r1!, {r4, r5, r6, ip}
    orr     r3, r3, r4, asl #(32 - \shift * 8)
    add     r10, r1, r9
    mov     r4, r4, lsr #(\shift * 8)
    orr     r4, r4, r5, asl #(32 - \shift * 8)
    mov     r5, r5, lsr #(\shift * 8)
.if \line_size == 32
    pld     [r11, #-32]
.endif
    orr     r5, r5, r6, asl #(32 - \shift * 8)
    mov     r6, r6, lsr #(\shift * 8)
    orr     r6, r6, ip, asl #(32 - \shift * 8)
    stmia   r0!, {r3, r4, r5, r6}
    bgt     18b
.if \overfetch == 0
    b       4f
.endif
#endif
.endif
1:
.if \line_size == 64 || \write_align == 64
    /* Process 64 bytes at a time. */
.if \overfetch == 1
    cmp     r2, #(64 + 64)
.else
    cmp     r2, #(\preload_offset + 64)
.endif
    pld     [r1, r9]
    mov     r3, ip, lsr #(\shift * 8)
    ldmia   r1!, {r4-r6, r7, r8, r10, r11, ip}
    orr     r3, r3, r4, asl #(32 - \shift * 8)
    mov     r4, r4, lsr #(\shift * 8)
    sub     r2, r2, #32
    orr     r4, r4, r5, asl #(32 - \shift * 8)
    mov     r5, r5, lsr #(\shift * 8)
    orr     r5, r5, r6, asl #(32 - \shift * 8)
    mov     r6, r6, lsr #(\shift * 8)
    orr     r6, r6, r7, asl #(32 - \shift * 8)
    mov     r7, r7, lsr #(\shift * 8)
.if \block_write_size == 16
    stmia   r0!, {r3-r6}
.endif
    orr     r7, r7, r8, asl #(32 - \shift * 8)
    mov     r8, r8, lsr #(\shift * 8)
    orr     r8, r8, r10, asl #(32 - \shift * 8)
    mov     r10, r10, lsr #(\shift * 8)
.if \block_write_size == 8
    stmia   r0!, {r7-r8}
.endif
    orr     r10, r10, r11, asl #(32 - \shift * 8)
    mov     r11, r11, lsr #(\shift * 8)
    orr     r11, r11, ip, asl #(32 - \shift * 8)
.if \block_write_size == 32
    stmia   r0!, {r3-r6, r7, r8, r10, r11}
.endif
.if \block_write_size == 16
    stmia   r0!, {r7, r8, r10, r11}
.endif
.if \line_size == 32
    pld     [r1, r9]
.endif
    mov     r3, ip, lsr #(\shift * 8)
    ldmia   r1!, {r4-r6, r7, r8, r10, r11, ip}
    orr     r3, r3, r4, asl #(32 - \shift * 8)
    mov     r4, r4, lsr #(\shift * 8)
    sub     r2, r2, #32
    orr     r4, r4, r5, asl #(32 - \shift * 8)
    mov     r5, r5, lsr #(\shift * 8)
    orr     r5, r5, r6, asl #(32 - \shift * 8)
    mov     r6, r6, lsr #(\shift * 8)
    orr     r6, r6, r7, asl #(32 - \shift * 8)
    mov     r7, r7, lsr #(\shift * 8)
.if \block_write_size == 16
    stmia   r0!, {r3-r6}
.endif
    orr     r7, r7, r8, asl #(32 - \shift * 8)
    mov     r8, r8, lsr #(\shift * 8)
    orr     r8, r8, r10, asl #(32 - \shift * 8)
    mov     r10, r10, lsr #(\shift * 8)
.if \block_write_size == 8
    stmia   r0!, {r7-r8}
.endif
    orr     r10, r10, r11, asl #(32 - \shift * 8)
    mov     r11, r11, lsr #(\shift * 8)
    orr     r11, r11, ip, asl #(32 - \shift * 8)
.if \block_write_size == 32
    stmia   r0!, {r3-r6, r7, r8, r10, r11}
.endif
.if \block_write_size == 16
    stmia   r0!, {r7, r8, r10, r11}
.endif
.else
    /* Process 32 bytes at a time. */
.if \overfetch == 1
    cmp     r2, #(32 + 32)
.else
    cmp     r2, #(\preload_offset + 32)
.endif
    pld     [r1, r9]
    mov     r3, ip, lsr #(\shift * 8)
    ldmia   r1!, {r4-r6, r7, r8, r10, r11, ip}
    orr     r3, r3, r4, asl #(32 - \shift * 8)
    mov     r4, r4, lsr #(\shift * 8)
    sub     r2, r2, #32
    orr     r4, r4, r5, asl #(32 - \shift * 8)
    mov     r5, r5, lsr #(\shift * 8)
.if \block_write_size == 8
    stmia   r0!, {r3-r4}
.endif
    orr     r5, r5, r6, asl #(32 - \shift * 8)
    mov     r6, r6, lsr #(\shift * 8)
    orr     r6, r6, r7, asl #(32 - \shift * 8)
    mov     r7, r7, lsr #(\shift * 8)
.if \block_write_size == 16
    stmia   r0!, {r3-r6}
.endif
.if \block_write_size == 8
    stmia   r0!, {r5-r6}
.endif
    orr     r7, r7, r8, asl #(32 - \shift * 8)
    mov     r8, r8, lsr #(\shift * 8)
    orr     r8, r8, r10, asl #(32 - \shift * 8)
    mov     r10, r10, lsr #(\shift * 8)
.if \block_write_size == 8
    stmia   r0!, {r7-r8}
.endif
    orr     r10, r10, r11, asl #(32 - \shift * 8)
    mov     r11, r11, lsr #(\shift * 8)
    orr     r11, r11, ip, asl #(32 - \shift * 8)
.if \block_write_size == 32
    stmia   r0!, {r3-r6, r7, r8, r10, r11}
.endif
.if \block_write_size == 16
    stmia   r0!, {r7, r8, r10, r11}
.endif
.if \block_write_size == 8
    stmia   r0!, {r10-r11}
.endif
.endif
    bge     1b
.endif /* preload_offset != 0 */
.if \overfetch == 0
4:
    cmp     r2, #(32 + 32)
    mov     r3, ip, lsr #(\shift * 8)
    ldmia   r1!, {r4-r6, r7, r8, r10, r11, ip}
    orr     r3, r3, r4, asl #(32 - \shift * 8)
    sub     r2, r2, #32
    mov     r4, r4, lsr #(\shift * 8)
    orr     r4, r4, r5, asl #(32 - \shift * 8)
    mov     r5, r5, lsr #(\shift * 8)
.if \block_write_size == 8
    stmia   r0!, {r3-r4}
.endif
    orr     r5, r5, r6, asl #(32 - \shift * 8)
    mov     r6, r6, lsr #(\shift * 8)
    orr     r6, r6, r7, asl #(32 - \shift * 8)
    mov     r7, r7, lsr #(\shift * 8)
.if \block_write_size == 16
    stmia   r0!, {r3-r6}
.endif
.if \block_write_size == 8
    stmia   r0!, {r5-r6}
.endif
    orr     r7, r7, r8, asl #(32 - \shift * 8)
    mov     r8, r8, lsr #(\shift * 8)
    orr     r8, r8, r10, asl #(32 - \shift * 8)
    mov     r10, r10, lsr #(\shift * 8)
.if \block_write_size == 8
    stmia   r0!, {r7-r8}
.endif
    orr     r10, r10, r11, asl #(32 - \shift * 8)
    mov     r11, r11, lsr #(\shift * 8)
    orr     r11, r11, ip, asl #(32 - \shift * 8)
.if \block_write_size == 32
    stmia   r0!, {r3-r6, r7, r8, r10, r11}
.endif
.if \block_write_size == 16
    stmia   r0!, {r7, r8, r10, r11}
.endif
.if \block_write_size == 8
    stmia   r0!, {r10-r11}
.endif
    bge     4b
.endif /* overfetch == 0 */
21:
    ldmfd   sp!, {r8, r9, r10, r11}
3:  /* copy remaining data */
    tst     r2, #16
    ldmfd   sp!, {r7}
    mov     r3, ip, lsr #(\shift * 8)
    beq     1f
    ldmia   r1!, {r4-r6, ip}
    orr     r3, r3, r4, asl #(32 - \shift * 8)
    mov     r4, r4, lsr #(\shift * 8)
    orr     r4, r4, r5, asl #(32 - \shift * 8)
    mov     r5, r5, lsr #(\shift * 8)
    orr     r5, r5, r6, asl #(32 - \shift * 8)
    mov     r6, r6, lsr #(\shift * 8)
    orr     r6, r6, ip, asl #(32 - \shift * 8)
    stmia   r0!, {r3-r6}
1:
    tst     r2, #8
    movne   r3, ip, lsr #(\shift * 8)
    ldmiane r1!, {r4, ip}
    orrne   r3, r3, r4, asl #(32 - \shift * 8)
    movne   r4, r4, lsr #(\shift * 8)
    orrne   r4, r4, ip, asl #(32 - \shift * 8)
    stmiane r0!, {r3-r4}

    tst     r2, #4
    movne   r3, ip, lsr #(\shift * 8)
    ldrne   ip, [r1], #4
    sub     r1, r1, #(4 - \shift)
    orrne   r3, r3, ip, asl #(32 - \shift * 8)
    strne   r3, [r0], #4

.if \granularity <= 2
    tst     r2, #2
    ldrbne  r3, [r1], #1
    ldrbne  r4, [r1], #1
.endif
    ldr     r5, [sp], #4
.if \granularity <= 2
    strbne  r3, [r0], #1
    strbne  r4, [r0], #1
.endif

.if \granularity == 1
    tst     r2, #1
    ldrbne  r3, [r1], #1
.endif
    ldr     r6, [sp], #4
.if \granularity == 1
    strbne  r3, [r0], #1
.endif

    pop     {r0, r4}

    bx      lr
.endm


/*
 * Macro that defines the main body of a memcpy version with optional no over-fetching
 * beyond the source memory region.
 *
 * granularity must be 1, 2 or 4. This value is 1 for normal memcpy, 2 for operations on half-word
 * aligned regions such as 16bpp framebuffers/images, and 4 for operations on word aligned regions
 * such as 32bpp framebuffers\images.
 * line_size must be 32 or 64.
 * write_align must be 32 or 16, or 64.
 * block_write_size must be 32, 16 or 8.
 * preload_offset must be a multiple of 32, 96 was the default setting. When preload_offset is 0,
 * no preload instructions will be generated at all.
 * preload_early must be 0 or 1.
 * overfetch must be 0 or 1.
 *
 * If line_size is 64, write_align must be 32 or 64, block_write_size must be 32, and preload_offset
 * must be a multiple of 64.
 *
 * If line_size is 64 or write_align is 64, overfetch must be 0.
 */

.macro MEMCPY_VARIANT granularity, line_size, write_align, block_write_size, preload_offset, preload_early, overfetch
    cmp     r2, #52
    bic     r3, r1, #(\line_size - 1)
.if \preload_early == 1
    pld     [r3]
.endif
    /* Jump if we have a large size. */
    bge     1f

.if \granularity <= 2
    /*
     * Small sizes. Test whether both source and destination are word aligned.
     */
    tst    r0, #3
    andseq r3, r1, #3
    /* If not, jump to the unaligned code for small sizes */
    mov    ip, r0
    bne    9f
.else
    mov    ip, r0
.endif

    /* Copy words. Fast path for small sizes with word aligned src and dest. */
    /* ip must be equal to the original r0. */
29:
22:
    cmp    r2, #8
    ldrge  r3, [r1], #4
    strge  r3, [r0], #4
    ldrge  r3, [r1], #4
    sub    r2, r2, #8
    strge  r3, [r0], #4
    bgt    22b
    moveq  r0,ip
    bxeq   lr
    tst    r2, #4
    ldrne  r3, [r1], #4
    strne  r3, [r0], #4
    tst    r2, #3
    moveq  r0, ip
    bxeq   lr
.if \granularity <= 2
    tst    r2, #2
    ldrhne r3, [r1], #2
    strhne r3, [r0], #2
.endif
.if \granularity == 1
    tst    r2, #1
    ldrbne r3, [r1]
    strbne r3, [r0]
.endif
    mov    r0, ip
    bx     lr

1:
    /*
     * Larger sizes. Copy data until destination address is 4 bytes aligned.
     * Optimize the common case in which both source and destination are
     * are already word-aligned.
     */
.if \granularity == 1
    tst     r0, #3
    stmfd   sp!, {r0, r4}
    andseq  r3, r1, #3
    stmfd   sp!, {r5, r6}
#ifdef CHECK_EARLY_PRELOADS
.if \preload_early == 1
    /* Determine the 32-byte aligned address of the last byte. */
    addeq   r6, r1, r2
.endif
#endif
    beq 2f
.else
    stmfd   sp!, {r0, r4}
    stmfd   sp!, {r5, r6}
.endif

.if \granularity == 1
    tst     r0, #1
    ldrbne  r4, [r1], #1
    subne   r2, r2, #1
    strbne  r4, [r0], #1
.endif

.if \granularity <= 2
    tst     r0, #2
    ldrbne  r4, [r1], #1
.endif
.if \granularity <= 2
    ldrbne  r5, [r1], #1
    subne   r2, r2, #2
    orrne   r4, r4, r5, asl #8
.endif
#ifdef CHECK_EARLY_PRELOADS
.if \preload_early == 1
    /* Determine the 32-byte aligned address of the last byte. */
    add     r6, r1, r2
.endif
#endif
.if \granularity <= 2
    strhne  r4, [r0], #2
.endif
    /* destination address is 4 bytes aligned */

.if \granularity == 1
    tst     r1, #1
.endif
#ifdef CHECK_EARLY_PRELOADS
.if \preload_early == 1
    sub     r6, r6, #1
.endif
#endif
#ifdef CHECK_EARLY_PRELOADS
.if \preload_early == 1
    bic     r6, r6, #(\line_size - 1)
.endif
#endif

    /* now we should handle 4 cases of source address alignment */
.if \granularity == 1
    bne     6f
.endif
.if \granularity <= 2
    tst     r1, #2
.endif
    stmfd   sp!, {r7}
.if \granularity <= 2
    bne     7f
.endif
    tst     r0, #4
    b       3f

2:
    /* Further optimize for the 16-byte aligned case. */
    tst     r0, #12
#ifdef CHECK_EARLY_PRELOADS
.if \preload_early == 1
    sub     r6, r6, #1
.endif
#endif
.if \preload_early == 1
    pld     [r3, #\line_size]
.endif
#ifdef CHECK_EARLY_PRELOADS
.if \preload_early == 1
    bic     r6, r6, #(\line_size - 1)
.endif
#endif
    stmfd   sp!, {r7}
    beq     1f
    tst     r0, #4

3:
    /* both source and destination are 4 bytes aligned */
#ifdef CHECK_EARLY_PRELOADS
.if \preload_early == 1
    mov     ip, r6
.endif
#endif
    ldrne   r5, [r1], #4
    subne   r2, r2, #4
    strne   r5, [r0], #4
    tst     r0, #8
    ldmiane r1!, {r4, r5}
    subne   r2, r2, #8
    stmiane r0!, {r4, r5}
1:
.if \write_align >= 32
    tst     r0, #16
    ldmiane r1!, {r4-r7}
    subne   r2, r2, #16
    stmiane r0!, {r4-r7}
.endif
.if \write_align == 64
    tst     r0, #32
    ldmiane r1!, {r4-r7}
    subne   r2, r2, #32
    stmiane r0!, {r4-r7}
    ldmiane r1!, {r4-r7}
    stmiane r0!, {r4-r7}
.endif
    /* Source is now write_align bytes aligned. */

    /*
     * The chunk size is defined is 64 if write_align == 64 or line_size = 64;
     * otherwise, it is equal to write_align.
     * If the number of bytes left is smaller than the chunk size, skip all loops.
     * If the number of bytes left is <= (preload_offset + chunk_size), skip the 
     * loop with preload and jump to the loop without preload.
     * Also calculate the preload offset in r9 and the address of the next main loop preload
     * in r5 if early preload is enabled and PRELOAD_CATCH_UP is set.
     * If preload is enabled, r3 is updated to hold the address of the next early preload.
     */
.if \preload_offset == 0
    cmp     r2, #32
    blt     14f
    stmfd   sp!, {r8, r9, r10, r11}
.elseif \write_align == 64 || \line_size == 64
    cmp     r2, #64
.if \line_size == 64 && \write_align == 32
    add     r5, r1, #\preload_offset
.endif
.if \preload_early == 1
    pld     [r3, #(\line_size * 2)]
#ifdef PRELOAD_CATCH_UP
    add     r3, #(\line_size * 3)
#endif
.endif
.if \line_size == 64 && \write_align == 32
    bic     r5, r5, #63
.else
#ifdef PRELOAD_CATCH_UP
.if \preload_early == 1
    add     r5, r1, #\preload_offset
.endif
#endif
.endif
    blt     2f
    cmp     r2, #(\preload_offset + 64)
    stmfd   sp!, {r8, r9, r10, r11}
.if \line_size == 64 && \write_align == 32
    sub     r9, r5, r1
.else
    mov     r9, #\preload_offset
.endif
.if \overfetch == 1
    ble     1f
.else
    ble     10f
.endif
.elseif \write_align == 32
    /* In the case of line_size == 32 and write_align == 32 r9 will be equal to preload_offset. */
    cmp     r2, #32
.if \preload_early == 1
    pld     [r3, #(\line_size * 2)]
#ifdef PRELOAD_CATCH_UP
    add     r3, #(\line_size * 3)
#endif
.endif
#ifdef PRELOAD_CATCH_UP
.if \preload_early == 1
    add     r5, r1, #\preload_offset
.endif
#endif
    blt     14f
    cmp     r2, #(\preload_offset + 32)
    stmfd   sp!, {r8, r9, r10, r11}
    mov     r9, #\preload_offset
.if \overfetch == 1
    ble     1f
.else
    ble     10f
.endif
.else // write_align == 16
    cmp     r2, #32
    add     r5, r1, #\preload_offset
.if \preload_early == 1
    pld     [r3, #(\line_size * 2)]
#ifdef PRELOAD_CATCH_UP
    add     r3, #(\line_size * 3)
#endif
.endif
    bic     r5, r5, #31
    /* If there are less than 32 bytes to go, skip all loops. */
    blt     14f
    cmp     r2, #(\preload_offset + 32)
    stmfd   sp!, {r8, r9, r10, r11}
    sub     r9, r5, r1
    /* If there are <= (preload_offset + 32) bytes to go, skip the main loop. */
.if \overfetch == 1
    ble     1f
.else
    ble     10f
.endif
.endif

.if \preload_offset != 0
.if \preload_early == 1
#ifndef PRELOAD_CATCH_UP
    pld     [r3, #(\line_size * 3)]
#else
.if \block_write_size >= 16 && \preload_offset >= 96
    /*
     * The last preload already done is at [r3 - line_size].
     * The next preload in the main loop will happen at [r5 + line_size].
     * If there are line-sized chunks in between that we have not yet preloaded,
     * we want to do preloads for them.
     */
    cmp     r3, r5
    bge     1f
#if 0
    /* Implement catch-up using a simple loop. */
    add     r3, r3, #\line_size
13:
    cmp     r3, r5
    pld     [r3, #-\line_size]
    add     r3, r3, #\line_size
    blt     13b
#else
    /*
     * Implement catch-up while processing chunks. block_write_size of 32
     * uses 16-byte writes because of a lack of registers.
     * Note: if line_size is 64 and write alignment is 64, we have to be 
     * careful that write alignment remains 64 bytes.
     */
    pld     [r3]
    add     r3, r3, #\line_size
13:
    cmp     r3, r5
    ldmialt r1!, {r7, r8, r10, r11}
    addlt   r5, r5, #32
    bge     1f
.if \line_size == 64 || \write_align == 64
    cmp     r2, #(\preload_offset + 64 + 32)
.else
    cmp     r2, #(\preload_offset + 32 + 32)
.endif
    stmia   r0!, {r7, r8, r10, r11}
    pld     [r3]
    add     r3, r3, #64
    ldmia   r1!, {r7, r8, r10, r11}
    sub     r2, r2, #32
    stmia   r0!, {r7, r8, r10, r11}
.if \line_size == 32
    pld     [r3, #-\line_size]
.endif
.if \line_size != 64 || \write_align != 64
    bgt     13b
.if \overfetch == 0
    b       10f
.endif
.else
    /*
     * If line_size is 64 and write_align is 64, make sure
     * the write alignment of 64 maintained.
     *
     * Jump if we don't need to do preloads anymore; 64-byte write
     * alignment is not important in this case.
     */
    add     r5, r5, #32
    ble     10f
    cmp     r3, r5
    ldmia   r1!, {r7, r8, r10, r11}
    /* In case of a jump, we will be doing more preloads so we */
    /* have to ensure 64 bytes write alignment. */
    bge     5f
    cmp     r2, #(\preload_offset + 64 + 32)
    stmia   r0!, {r7, r8, r10, r11}
    pld     [r3]
    add     r3, r3, #64
    ldmia   r1!, {r7, r8, r10, r11}
    sub     r2, r2, #32
    stmia   r0!, {r7, r8, r10, r11}
    bgt     13b
.if \overfetch == 0
    b       10f
.endif
.endif /* line_size == 64 write_alignment == 64 */
#endif
.else
    pld     [r3]
.endif
#endif
.endif /* preload_early == 1 */
1:
.if \line_size == 64 || \write_align == 64
.if \overfetch == 1
    cmp     r2, #(64 + 64)
.else
    cmp     r2, #(\preload_offset + 64 + 64)
.endif
    ldmia   r1!, {r3-r6, r7, r8, r10, r11}
    stmia   r0!, {r3-r6, r7, r8, r10, r11}
.if \line_size == 32
    pld     [r1, r9]
.endif
    ldmia   r1!, {r3-r6, r7, r8, r10, r11}
    sub     r2, r2, #64
    stmia   r0!, {r3-r6, r7, r8, r10, r11}
    pld     [r1, r9]
.else
.if \overfetch == 1
    cmp     r2, #(32 + 32)
.else
    cmp     r2, #(\preload_offset + 32 + 32)
.endif
.if \block_write_size == 32
    ldmia   r1!, {r3-r6, r7, r8, r10, r11}
    sub     r2, r2, #32
    stmia   r0!, {r3-r6, r7, r8, r10, r11}
    pld     [r1, r9]
.endif
.if \block_write_size == 16
    ldmia   r1!, {r3-r6, r7, r8, r10, r11}
    sub     r2, r2, #32
    stmia   r0!, {r3-r6}
    pld     [r1, r9]
    stmia   r0!, {r7, r8, r10, r11}
.endif
.if \block_write_size == 8
    ldmia   r1!, {r3-r6, r7, r8, r10, r11}
    sub     r2, r2, #32
    stmia   r0!, {r3-r4}
    stmia   r0!, {r5-r6}
    pld     [r1, r9]
    stmia   r0!, {r7-r8}
    stmia   r0!, {r10-r11}
.endif
.endif /* line_size ==  64 */
    bge     1b
.endif /* preload_offset != 0 */
.if \overfetch == 0
10:
.if \line_size == 64 || \write_align == 64
    cmp     r2, #(64 + 64)
    ldmia   r1!, {r3-r6, r7, r8, r10, r11}
    sub     r2, r2, #64
    stmia   r0!, {r3-r6, r7, r8, r10, r11}
    ldmia   r1!, {r3-r6, r7, r8, r10, r11}
    stmia   r0!, {r3-r6, r7, r8, r10, r11}
.else
.if \block_write_size == 32
    cmp     r2, #(32 + 32)
    ldmia   r1!, {r3-r6, r7, r8, r10, r11}
    sub     r2, r2, #32
    stmia   r0!, {r3-r6, r7, r8, r10, r11}
.endif
.if \block_write_size == 16
    cmp     r2, #(32 + 32)
    ldmia   r1!, {r3-r6, r7, r8, r10, r11}
    sub     r2, r2, #32
    stmia   r0!, {r3-r6}
    stmia   r0!, {r7, r8, r10, r11}
.endif
.if \block_write_size == 8
    cmp     r2, #(32 + 32)
    ldmia   r1!, {r3-r6, r7, r8, r10, r11}
    sub     r2, r2, #32
    stmia   r0!, {r3-r4}
    stmia   r0!, {r5-r6}
    stmia   r0!, {r7-r8}
    stmia   r0!, {r10-r11}
.endif
.endif /* line_size == 64 || write_align == 64  */
    bge     10b
.endif /* overfetch == 0 */
    ldmfd   sp!, {r8, r9, r10, r11}
2:
.if \line_size == 64 || \write_align == 64
    tst     r2, #32
    ldmiane r1!, {r3-r6}
    stmiane r0!, {r3-r6}
    ldmiane r1!, {r3-r6}
    stmiane r0!, {r3-r6}
.endif
14:
    tst     r2, #16
    ldmfd   sp!, {r7}
    ldmiane r1!, {r3-r6}
    stmiane r0!, {r3-r6}
3:
    tst     r2, #8
    ldmiane r1!, {r3-r4}
    stmiane r0!, {r3-r4}
    tst     r2, #4
    ldrne   r3, [r1], #4
    mov     ip, r0
    strne   r3, [ip], #4
.if \granularity == 1
    /* Optimize for the word-sized case. */
    tst     r2, #3
    ldmfdeq sp!, {r5, r6}
    ldmfdeq sp!, {r0, r4}
    bxeq    lr
.endif
.if \granularity <= 2
    tst     r2, #2
    ldrhne  r3, [r1], #2
.endif
    ldmfd   sp!, {r5, r6}
.if \granularity <= 2
    strhne  r3, [ip], #2
.endif
.if \granularity == 1
    tst     r2, #1
    ldrbne  r3, [r1]
.endif
    ldmfd   sp!, {r0, r4}
.if \granularity == 1
    strbne  r3, [ip]
.endif
    bx      lr
5:
    /* We get here in case we need to fix write alignment to 64 bytes. */
    stmia   r0!, {r7, r8, r10, r11}
    ldmia   r1!, {r7, r8, r10, r11}
    sub     r2, r2, #32
    stmia   r0!, {r7, r8, r10, r11}
    b       1b
.if \granularity == 1
6:
    tst    r1, #2
    stmfd   sp!, {r7}
    bne    8f
    UNALIGNED_MEMCPY_VARIANT \granularity, 1, \line_size, \write_align, \block_write_size, \preload_offset, \preload_early, \overfetch
.endif
7:
    UNALIGNED_MEMCPY_VARIANT \granularity, 2, \line_size, \write_align, \block_write_size, \preload_offset, \preload_early, \overfetch
.if \granularity == 1
8:
    UNALIGNED_MEMCPY_VARIANT \granularity, 3, \line_size, \write_align, \block_write_size, \preload_offset, \preload_early, \overfetch
.endif

    .p2align 4
.if \granularity <= 2
9:
    cmp     r2, #8
    push    {r0}
    blt     1f               /* Jump to special case for really small sizes. */
    
    /* copy data until destination address is 4 bytes aligned */
.if \granularity == 1
    tst     r0, #1
    ldrbne  r3, [r1], #1
    subne   r2, r2, #1
    strbne  r3, [r0], #1
.endif

    tst     r0, #2
    ldrbne  r3, [r1], #1
    ldrbne  ip, [r1], #1
    subne   r2, r2, #2
    orrne   r3, r3, ip, asl #8
    strhne  r3, [r0], #2
    /* destination address is 4 bytes aligned */

    /* now we should handle four cases of source address alignment */
.if \granularity == 1
    tst     r1, #1
    bne     25f
.endif
    tst     r1, #2
    popeq   {ip}
    beq     29b               /* Jump if the source is word aligned. */

    /* shift 2 */
//    sub     r1, r1, #2
//    ldr     ip, [r1], #4
    ldr     ip, [r1, #-2]
    add     r1, r1, #2
23:
    subs    r2, r2, #4
    movge   r3, ip, lsr #(2 * 8)
    ldrge   ip, [r1], #4
    orrge   r3, r3, ip, asl #(32 - 2 * 8)
    strge   r3, [r0], #4
    bge     23b

    sub     r1, r1, #2
    tst     r2, #2
    ldrbne  r3, [r1], #1
    ldrbne  ip, [r1], #1
    strbne  r3, [r0], #1
    strbne  ip, [r0], #1

.if \granularity == 1
    tst     r2, #1
    mov     ip, r0
    ldrbne  r3, [r1]
    ldr     r0, [sp], #4
    strbne  r3, [ip]
.else
    pop     {r0}
.endif
    bx      lr

    /* Handle sizes < 8 */
1:
.if \granularity == 2
    tst     r2, #4
    ldrhne  r3, [r1], #2
    ldrhne  ip, [r1], #2
    strhne  r3, [r0], #2
    strhne  ip, [r0], #2
    test    r2, #2
    mov     ip, r0
    ldrhne  r3, [r1]
    pop     {r0}
    strhne  r3, [ip]
.else
    tst     r2, #4
    ldrbne  r3, [r1], #1
    beq     2f
    ldrb    ip, [r1], #1
    strb    r3, [r0], #1
    strb    ip, [r0], #1
    ldrb    r3, [r1], #1
    ldrb    ip, [r1], #1
    strb    r3, [r0], #1
    strb    ip, [r0], #1
2:
    tst     r2, #2
    ldrbne  r3, [r1], #1
    ldrbne  ip, [r1], #1
    strbne  r3, [r0], #1
    strbne  ip, [r0], #1
    tst     r2, #1
    mov     ip, r0
    ldrbne  r3, [r1]
    pop     {r0}
    strbne  r3, [ip]
.endif
    bx      lr

.if \granularity == 1
24:
    /* shift 1 */
//    sub     r1, r1, #1
//    ldr     ip, [r1], #4
    ldr     ip, [r1, #-1]
    add     r1, r1, #3
27:
    subs    r2, r2, #4
    movge   r3, ip, lsr #(1 * 8)
    ldrge   ip, [r1], #4
    orrge   r3, r3, ip, asl #(32 - 1 * 8)
    strge   r3, [r0], #4
    bge     27b

    sub     r1, r1, #3
    tst     r2, #2
    ldrbne  r3, [r1], #1
    ldrbne  ip, [r1], #1
    strbne  r3, [r0], #1
    strbne  ip, [r0], #1

    tst     r2, #1
    mov     ip, r0
    ldrbne  r3, [r1]
    ldr     r0, [sp], #4
    strbne  r3, [ip]
    bx      lr

25:
    tst     r1, #2
    beq     24b          /* shift 1 */

    /* shift 3 */
26:
//    sub     r1, r1, #3
//    ldr     ip, [r1], #4
    ldr     ip, [r1, #-3]
    add     r1, r1, #1
28:
    subs    r2, r2, #4
    movge   r3, ip, lsr #(3 * 8)
    ldrge   ip, [r1], #4
    orrge   r3, r3, ip, asl #(32 - 3 * 8)
    strge   r3, [r0], #4
    bge     28b

    sub     r1, r1, #1
    tst     r2, #2
    ldrbne  r3, [r1], #1
    ldrbne  ip, [r1], #1
    strbne  r3, [r0], #1
    strbne  ip, [r0], #1

    tst     r2, #1
    mov     ip, r0
    ldrbne  r3, [r1]
    ldr     r0, [sp], #4
    strbne  r3, [ip]
    bx      lr
.endif /* granularity == 1 */
.endif /* granularity <= 2 */

.endm

/*
 * The following macros implement a simpler memcpy that is optimized with a fast path
 * for common cases and may use unaligned access for small sizes.
 *
 * line_size of 64 or 32 is supported, write_align must be 32 or 16, block_write_size
 * must be 32 or 16, early_preload and overfetch are enabled.
 */

.macro MEMCPY_VARIANT_SIMPLE_WRITE_ALIGN write_align
    /* Align to a 16-byte or 32-byte boundary. */
    tst     r0, #4
    ldrne   r3, [r1], #4
    subne   r2, r2, #4
    strne   r3, [r0], #4
    tst     r0, #8
    ldrne   r3, [r1], #4
    ldrne   ip, [r1], #4
    subne   r2, r2, #8
    strne   r3, [r0], #4
    strne   ip, [r0], #4
.if \write_align >= 32
    tst     r0, #16
    ldrne   r3, [r1]
    beq     31f
    ldr     ip, [r1, #4]
    str     r3, [r0]
    sub     r2, r2, #16
    str     ip, [r0, #4]
    ldr     r3, [r1, #8]
    ldr     ip, [r1, #12]
    add     r1, #16
    str     r3, [r0, #8]
    str     ip, [r0, #12]
    add     r0, #16
31:
.endif
.if \write_align == 64
    tst     r0, #32
    ldmiane r1!, {r3, ip}
    beq     32f
    stmia   r0!, {r3, ip}
    ldmia   r1!, {r3, ip}
    stmia   r0!, {r3, ip}
    ldmia   r1!, {r3, ip}
    stmia   r0!, {r3, ip}
    ldmia   r1!, {r3, ip}
    sub     r2, r2, #32
    stmia   r0!, {r3, ip}
32:
.endif
.endm

.macro MEMCPY_VARIANT_SIMPLE_WRITE_ALIGN_CUSTOM
    /*
     * Align destination to a 16-byte or 32-byte boundary,
     * depending on whether the 32-byte alignment of the
     * source is optimal.
     */
    tst     r0, #4
    ldrne   r3, [r1], #4
    subne   r2, r2, #4
    strne   r3, [r0], #4
    tst     r0, #8
    ldrne   r3, [r1], #4
    ldrne   ip, [r1], #4
    subne   r2, r2, #8
    strne   r3, [r0], #4
    strne   ip, [r0], #4
    /*
     * If (source & 16) is zero, allow write aligning to 32 bytes.
     * This improves performance.
     */
    eor     r3, r1, r0
    tst     r0, #16
    tstne   r3, #16
    ldrne   r3, [r1]
    beq     31f
    ldr     ip, [r1, #4]
    str     r3, [r0]
    sub     r2, r2, #16
    str     ip, [r0, #4]
    ldr     r3, [r1, #8]
    ldr     ip, [r1, #12]
    add     r1, #16
    str     r3, [r0, #8]
    str     ip, [r0, #12]
    add     r0, #16
31:
.endm

.macro MEMCPY_VARIANT_SIMPLE_UNALIGNED_MAIN_PART shift, line_size, write_align, block_write_size, preload_offset, custom_write_align
    ldr     ip, [r1, #(-\shift)]
    add     r1, r1, #(4 - \shift)

    tst     r0, #4
    push    {r4-r6}
    movne   r3, ip, lsr #(\shift * 8)
    ldrne   ip, [r1], #4
    subne   r2, r2, #4
    orrne   r3, r3, ip, asl #(32 - \shift * 8)
    strne   r3, [r0], #4

    tst     r0, #8
    movne   r3, ip, lsr #(\shift * 8)
    ldmiane r1!, {r4, ip}
    subne   r2, r2, #8
    orrne   r3, r3, r4, asl #(32 - \shift * 8)
    movne   r4, r4, lsr #(\shift * 8)
    orrne   r4, r4, ip, asl #(32 - \shift * 8)
    stmiane r0!, {r3-r4}

.if \write_align == 32
.if \custom_write_align == 1
    eor     r3, r1, r0
    tst     r0, #16
    tstne   r3, #16
.else
    tst     r0, #16
.endif
    movne   r3, ip, lsr #(\shift * 8)
    beq     25f
    ldmia   r1!, {r4-r6, ip}
    sub     r2, r2, #16
    orr     r3, r3, r4, asl #(32 - \shift * 8)
    mov     r4, r4, lsr #(\shift * 8)
.if (68 - (\write_align - 1)) < 32
    cmp     r2, #32
.endif
    orr     r4, r4, r5, asl #(32 - \shift * 8)
    mov     r5, r5, lsr #(\shift * 8)
    orr     r5, r5, r6, asl #(32 - \shift * 8)
    mov     r6, r6, lsr #(\shift * 8)
    orr     r6, r6, ip, asl #(32 - \shift * 8)
    stmia   r0!, {r3-r6}
.if (68 - (\write_align - 1)) < 32
    blt     22f
    b       26f
.endif
25:
.endif

    /*
     * We don't need a check if the number of bytes left is guaranteed to
     * be >= 32.
     */
.if (68 - (\write_align - 1)) < 32
    cmp     r2, #32
    blt     22f
.endif
26:
.if \write_align == \line_size && 0
    push    {r7-r11}
    mov     r9, #\preload_offset
    sub     r2, r2, #32
.else
    add     r3, r1, #\preload_offset
    push    {r7-r11}
    bic     r3, r3, #(\line_size - 1)
    sub     r2, r2, #32
    sub     r9, r3, r1
.endif
    /*
     * Main loop for unaligned copy. Process 32 bytes at a time.
     */
21:
    pld     [r1, r9]
    mov     r3, ip, lsr #(\shift * 8)
    ldmia   r1!, {r4-r6, r7, r8, r10, r11, ip}
    orr     r3, r3, r4, asl #(32 - \shift * 8)
    mov     r4, r4, lsr #(\shift * 8)
    subs    r2, r2, #32
    orr     r4, r4, r5, asl #(32 - \shift * 8)
    mov     r5, r5, lsr #(\shift * 8)
    orr     r5, r5, r6, asl #(32 - \shift * 8)
    mov     r6, r6, lsr #(\shift * 8)
    orr     r6, r6, r7, asl #(32 - \shift * 8)
    mov     r7, r7, lsr #(\shift * 8)
.if \block_write_size == 16
    stmia   r0!, {r3-r6}
.endif
    orr     r7, r7, r8, asl #(32 - \shift * 8)
    mov     r8, r8, lsr #(\shift * 8)
    orr     r8, r8, r10, asl #(32 - \shift * 8)
    mov     r10, r10, lsr #(\shift * 8)
    orr     r10, r10, r11, asl #(32 - \shift * 8)
    mov     r11, r11, lsr #(\shift * 8)
    orr     r11, r11, ip, asl #(32 - \shift * 8)
.if \block_write_size == 32
    stmia   r0!, {r3-r6, r7, r8, r10, r11}
.endif
.if \block_write_size == 16
    stmia   r0!, {r7, r8, r10, r11}
.endif
    bge     21b
    adds    r2, r2, #32
    pop     {r7-r11}
    popeq   {r4-r6}
    popeq   {r0}
    bxeq    lr
22:
    pop     {r4-r6}
23:
    subs    r2, r2, #4
    movge   r3, ip, lsr #(\shift * 8)
    ldrge   ip, [r1], #4
    orrge   r3, r3, ip, asl #(32 - \shift * 8)
    strge   r3, [r0], #4
    bgt     23b

24:
    sub     r1, r1, #(4 - \shift)
.endm


.macro MEMCPY_VARIANT_SIMPLE granularity, line_size, write_align, block_write_size, \
preload_offset, preload_catch_up, preload_early, overfetch, custom_write_align, \
check_small_size_alignment
    cmp     r2, #68
.if \preload_early == 1
    bic     r3, r1, #(\line_size - 1)
.endif
    mov     ip, r0
.if \preload_early == 1
    pld     [r3]
.endif
    bge     1f

    /*
     * Path for sizes < 68 bytes; don't care about unaligned access
     * except if both the source and destination are unaligned and
     * the number of bytes is > 32. This checks costs a few percent
     * performance for the common word aligned-case.
     */
.if \check_small_size_alignment == 1
.if \granularity <= 2
    /* This assumes lt flag is set. */
    tst     r0, #3
    tstne   r1, #3
    cmpne   r2, #32
    bgt     2f
.endif
.endif
3:
    tst    r2, #4
    ldrne  r3, [r1], #4
    subne  r2, r2, #4
    strne  r3, [r0], #4
4:
    cmp    r2, #8
    ldrge  r3, [r1], #4
    strge  r3, [r0], #4
    ldrge  r3, [r1], #4
    subge  r2, r2, #8
    strge  r3, [r0], #4
    bgt    4b
.if \granularity <= 2
    tstne  r2, #3
    moveq  r0, ip
    bxeq   lr
.endif
.if \granularity <= 2
    tst    r2, #2
    ldrhne r3, [r1], #2
    strhne r3, [r0], #2
.endif
.if \granularity == 1
    tst    r2, #1
    ldrbne r3, [r1]
    strbne r3, [r0]
.endif
    mov    r0, ip
    bx     lr

.if \check_small_size_alignment == 1
.if \granularity <= 2
2:
    /* Align the destination. */
.if \granularity == 1
    tst     r0, #1
.if \preload_early == 1 && \line_size == 32
    pld     [r3, #32]
.endif
    ldrbne  r3, [r1], #1
    subne   r2, r2, #1
    strbne  r3, [r0], #1
.endif

    tst     r0, #2
.if \granularity == 2 && \preload_early == 1 && \line_size == 32
    pld     [r3, #32]
.endif
    ldrbne  r3, [r1], #1
    ldrbne  ip, [r1], #1
    subne   r2, r2, #2
    orrne   r3, r3, ip, asl #8
    strhne  r3, [r0], #2
    b       3b
.endif
.endif

    /* Aligning this branch target to a 16-byte boundary helps performance a bit. */
.p2align 4
1:
    /* Check that both destination and source are word aligned. */
.if \granularity <= 2
    tst     r0, #3
.endif
    push    {r0}
.if \granularity == 1
    tsteq   r1, #3
.endif
.if \preload_early == 1
    pld     [r3, #\line_size]
.endif
    push    {r3}
.if \granularity <= 2
    bne     3f
.endif

    /* Larger sizes with word aligned source and destination. */
2:
.if \custom_write_align == 1
    MEMCPY_VARIANT_SIMPLE_WRITE_ALIGN_CUSTOM
.else
    MEMCPY_VARIANT_SIMPLE_WRITE_ALIGN \write_align
.endif
    /*
     * We don't need a check if the number of bytes left is guaranteed to
     * be >= line_size.
     */
.if (68 - (\write_align - 1)) >= \line_size
    pop     {r3}
.if \write_align == \line_size && 0
    mov     ip, #\preload_offset
.if \preload_early == 1
    pld     [r3, #(\line_size * 2)]
.endif
    push    {r4-r11}
.else
    add     ip, r1, #\preload_offset
.if \preload_early == 1
    pld     [r3, #(\line_size * 2)]
.endif
    push    {r4-r11}
    bic     ip, ip, #(\line_size - 1)
    sub     ip, ip, r1
.endif
.else
    cmp     r2, #\line_size
.if \write_align == \line_size && 0
    pop     {r3}
    mov     ip, #\preload_offset
.if \preload_early == 1
    pld     [r3, #(\line_size * 2)]
.endif
    pushge  {r4-r11}
    blt     6f
.else
    pop     {r3}
    addge   ip, r1, #\preload_offset
.if \preload_early == 1
    pld     [r3, #(\line_size * 2)]
.endif
    pushge  {r4-r11}
    blt     6f
    bic     ip, ip, #(\line_size - 1)
    sub     ip, ip, r1
.endif
.endif

.if \line_size == 32
.if \preload_early == 1 && \preload_offset >= 96
.if \preload_catch_up == 1
    add     r4, r1, ip
    add     r3, r3, #(\line_size * 3)
    cmp     r3, r4
    addlt   r3, r3, #\line_size
    bge     12f
11:
    cmp     r3, r4
    pld     [r3, #-\line_size]
    add     r3, r3, #\line_size
    blt     11b
12:
.else
    pld     [r3, #(\line_size * 3)]
.endif
.endif
    sub     r2, r2, #32
5:
    /*
     * The main loop for large sizes. Copy 32 bytes at a time
     * using ldmia/stmia while prefetching a 32-byte aligned
     * address.
     */
    pld     [r1, ip]
.if \block_write_size == 32
    ldmia   r1!, {r4-r11}
    subs    r2, r2, #32
    stmia   r0!, {r4-r11}
.else
    ldmia   r1!, {r4-r7}
    subs    r2, r2, #32
    ldmia   r1!, {r8-r11}
    stmia   r0!, {r4-r7}
    stmia   r0!, {r8-r11}
.endif
    bge     5b
    adds    r2, r2, #32
    pop     {r4-r11}
    popeq   {r0}
    bxeq    lr
.endif

.if \line_size == 64
.if \preload_early == 1 && \preload_offset >= 128
.if \preload_catch_up == 1
    add     r4, r1, ip
    add     r3, r3, #(\line_size * 3)
    cmp     r3, r4
    addlt   r3, r3, #\line_size
    bge     12f
11:
    cmp     r3, r4
    pld     [r3, #-\line_size]
    add     r3, r3, #\line_size
    blt     11b
12:
.else
    pld     [r3, #(\line_size * 3)]
.endif
.endif
    sub     r2, r2, #64
    /* Aligning the main loop branch target seems to help performance a bit. */
    b       5f
.p2align 4
5:
    /*
     * The main loop for large sizes. Copy 64 bytes at a time
     * using ldmia/stmia while prefetching a 64-byte aligned
     * address.
     */
    pld     [r1, ip]
    ldmia   r1!, {r4-r11}
    subs    r2, r2, #64
    stmia   r0!, {r4-r11}
    ldmia   r1!, {r4-r11}
    stmia   r0!, {r4-r11}
    bge     5b
    adds    r2, r2, #64
    pop     {r4-r11}
    popeq   {r0}
    bxeq    lr
.endif

6:
.if \line_size == 64
    cmp     r2, #32
    ldmiage r1!, {r3, ip}
    blt     10f
    stmia   r0!, {r3, ip}
    ldmia   r1!, {r3, ip}
    stmia   r0!, {r3, ip}
    ldmia   r1!, {r3, ip}
    stmia   r0!, {r3, ip}
    ldmia   r1!, {r3, ip}
    sub     r2, r2, #32
    stmia   r0!, {r3, ip}
    popeq   {r0}
    bxeq    lr
10:
.endif

    cmp     r2, #16
    ldrge   r3, [r1]
    ldrge   ip, [r1, #4]
    blt     7f
    sub     r2, r2, #16
    str     r3, [r0]
    str     ip, [r0, #4]
    ldr     r3, [r1, #8]
    ldr     ip, [r1, #12]
    add     r1, r1, #16
    str     r3, [r0, #8]
    str     ip, [r0, #12]
    popeq   {r0}
    bxeq    lr
    add     r0, r0, #16
7:
    cmp     r2, #8
    ldrge   ip, [r1]
    ldrge   r3, [r1, #4]
    strge   ip, [r0], #4
    pop     {ip}
    strge   r3, [r0], #4
    moveq   r0, ip
    bxeq    lr
    addge   r1, r1, #8

    tst     r2, #4
    ldrne   r3, [r1], #4
    strne   r3, [r0], #4
    tst     r2, #3
    moveq   r0, ip
    bxeq    lr
.if \granularity <= 2
    tst     r2, #2
    ldrhne  r3, [r1], #2
    strhne  r3, [r0], #2
.endif
.if \granularity == 1
    tst     r2, #1
    ldrbne  r3, [r1]
    strbne  r3, [r0]
.endif
    mov     r0, ip
    bx      lr

.if \granularity <= 2
3:
    /*
     * Copy data until destination address is 4 bytes aligned.
     */
.if \granularity == 1
    tst     r0, #1
    ldrbne  r3, [r1], #1
    subne   r2, r2, #1
    strbne  r3, [r0], #1
.endif

    tst     r0, #2
    ldrbne  r3, [r1], #1
    ldrbne  ip, [r1], #1
    subne   r2, r2, #2
    orrne   r3, r3, ip, asl #8
    strhne  r3, [r0], #2
    /* destination address is 4 bytes aligned */

    tst     r1, #3
    popne   {r3}
    beq     2b

    /* Unaligned copy. */
.if \granularity == 1
    tst     r1, #1
.endif
.if \preload_early == 1
    pld     [r3, #(\line_size * 2)]
.endif
.if \granularity == 1
    bne     2f
.endif

    MEMCPY_VARIANT_SIMPLE_UNALIGNED_MAIN_PART 2, line_size, write_align, block_write_size, preload_offset, custom_write_align
4:
    tst     r2, #2
    ldrbne  r3, [r1], #1
    ldrbne  ip, [r1], #1
    strbne  r3, [r0], #1
    strbne  ip, [r0], #1

.if \granularity == 1
    tst     r2, #1
    mov     ip, r0
    ldrbne  r3, [r1]
    ldr     r0, [sp], #4
    strbne  r3, [ip]
.else
    pop     {r0}
.endif
    bx      lr

.if \granularity == 1
3:
    MEMCPY_VARIANT_SIMPLE_UNALIGNED_MAIN_PART 3, line_size, write_align, block_write_size, preload_offset, custom_write_align
    b       4b

2:
    tst     r1, #2
    bne     3b

    MEMCPY_VARIANT_SIMPLE_UNALIGNED_MAIN_PART 1, line_size, write_align, block_write_size, preload_offset, custom_write_align
    b       4b
.endif

.endif

.endm

/*
 * Settings for the MEMCPY_VARIANT_SIMPLE macro
 *
 * granularity
 *     Must be 1, 2 or 4. This value is 1 for normal memcpy, 2 for operations on half-word
 *     aligned regions such as 16bpp framebuffers/images, and 4 for operations on word aligned
 *     regions such as 32bpp framebuffers\images.
 * line_size
 *     Must be 32 or 64. Defines the cache line size used for preloads. Preloads are only done
 *     at line_size aligned addresses. When early preload is enabled, the current implementation
 *     results in more aggressive early preload in the case of a line size of 64.
 * write_align
 *     Must be 16, 32, or 64. Defines the write alignment that is applied just before the main loop
 *     for larger sizes. The main loop processes chunks of line_size bytes at a time.
 * block_write_size
 *     Must must be 32, 16. Defines the size of multiple-register load and store instructions that
 *     are used in the main loop for larger sizes.
 * preload_offset
 *     Must be a multiple of line_size. Defines the offset from the current source address at which
 *     preloads are performed (look-ahead) in the main loop. The real applied offset is derived before
 *     the start of the main loop by adding the preload offset to the source address and rounding
 *     down the result to a line_size boundary, and then substracting the source address.
 * preload_catch_up
 *     Must be 0 or 1. When early preload is enabled, this enables code just before the main loop
 *     that performs a series of preloads from just beyond the last early preload to just before
 *     the first preload in the main loop, filling in the gap.
 * preload_early
 *     Must be 0 or 1. When enabled, preload instructions are enabled early in the memcpy function
 *     to preload the initial part of the source memory region. Early preloads start at the source
 *     address aligned to a line_size boundary and end at that address + line_size * 2 (three
 *     early preloads in total).
 * overfetch
 *     Must be 1.
 * custom_write_align
 *     Must be 0 or 1. Enables RPi-specific write alignment whereby 32-byte alignment is only applied
 *     if the source address will be located after alignment in the second half of a 32-byte aligned
 *     chunk; if not, write alignment remains at 16 bytes.
 * check_small_size_alignment
 *     Must be 0 or 1. For small sizes less than 68 bytes, unaligned memory access is used to reduce
 *     overhead in improve performance. However, when both source and destination are unaligned
 *     this induce a performance penalty. When this option is enabled, beyond a certain size threshold
 *     (currently set at 32 bytes), the destination is aligned to a word boundary. This may speed up
 *     unaligned copies in the range of 33 to 67 bytes.
 *
 * Restrictions:
 *     If line_size is 64, write_align must be 32 or 64, block_write_size must be 32, preload_offset
 *     must be a multiple of 64.
 *     If preload_catch_up is 1 then preload_early must be 1.
 */


#if defined(MEMCPY_REPLACEMENT_SUNXI) || defined(MEMCPY_REPLACEMENT_RPI)

#ifdef MEMCPY_REPLACEMENT_SUNXI

/* memcpy replacement for the Allwinner platform. */

asm_function memcpy
    MEMCPY_VARIANT_SIMPLE 1, 64, 32, 32, 192, 0, 1, 1, 0, 0
.endfunc

#endif

#ifdef MEMCPY_REPLACEMENT_RPI

/* memcpy replacement for the RPi platform. */

asm_function memcpy
    MEMCPY_VARIANT_SIMPLE 1, 32, 32, 16, 128, 1, 1, 1, 1, 1
.endfunc

#endif

#else

#ifdef RPI_BEST_MEMCPY_ONLY

/* Optimized memcpy variants for the RPi platform . */

asm_function memcpy_armv5te_no_overfetch
    MEMCPY_VARIANT 1, 32, 16, 16, 96, 1, 0
.endfunc

asm_function memcpy_armv5te_overfetch
    MEMCPY_VARIANT 1, 32, 16, 16, 128, 1, 1
.endfunc

asm_function memcpy_halfwords_armv5te_no_overfetch
    MEMCPY_VARIANT 2, 32, 16, 16, 96, 1, 0
.endfunc

asm_function memcpy_halfwords_armv5te_overfetch
    MEMCPY_VARIANT 2, 32, 16, 16, 128, 1, 1
.endfunc

asm_function memcpy_words_armv5te_no_overfetch
    MEMCPY_VARIANT 4, 32, 16, 16, 96, 1, 0
.endfunc

asm_function memcpy_words_armv5te_overfetch
    MEMCPY_VARIANT 4, 32, 16, 16, 128, 1, 1
.endfunc

#else

/* A large set of memcpy variants, used in the benchmark program */

asm_function memcpy_armv5te_no_overfetch_align_16_block_write_8_preload_96
    MEMCPY_VARIANT 1, 32, 16, 8, 96, 0, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_16_block_write_16_preload_96
    MEMCPY_VARIANT 1, 32, 16, 16, 96, 0, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_16_block_write_16_preload_early_96
    MEMCPY_VARIANT 1, 32, 16, 16, 96, 1, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_16_block_write_16_preload_early_128
    MEMCPY_VARIANT 1, 32, 16, 16, 128, 1, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_8_preload_96
    MEMCPY_VARIANT 1, 32, 32, 8, 96, 0, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_16_preload_64
    MEMCPY_VARIANT 1, 32, 32, 16, 64, 0, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_16_preload_96
    MEMCPY_VARIANT 1, 32, 32, 16, 96, 0, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_16_preload_128
    MEMCPY_VARIANT 1, 32, 32, 16, 128, 0, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_16_preload_160
    MEMCPY_VARIANT 1, 32, 32, 16, 160, 0, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_16_preload_192
    MEMCPY_VARIANT 1, 32, 32, 16, 192, 0, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_16_preload_256
    MEMCPY_VARIANT 1, 32, 32, 16, 256, 0, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_32_preload_64
    MEMCPY_VARIANT 1, 32, 32, 32, 64, 0, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_32_preload_96
    MEMCPY_VARIANT 1, 32, 32, 32, 96, 0, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_32_preload_128
    MEMCPY_VARIANT 1, 32, 32, 32, 128, 0, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_32_preload_160
    MEMCPY_VARIANT 1, 32, 32, 32, 160, 0, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_32_preload_192
    MEMCPY_VARIANT 1, 32, 32, 32, 192, 0, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_32_preload_256
    MEMCPY_VARIANT 1, 32, 32, 32, 256, 0, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_16_preload_early_96
    MEMCPY_VARIANT 1, 32, 32, 16, 96, 1, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_16_preload_early_128
    MEMCPY_VARIANT 1, 32, 32, 16, 128, 1, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_16_preload_early_192
    MEMCPY_VARIANT 1, 32, 32, 16, 192, 1, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_16_preload_early_256
    MEMCPY_VARIANT 1, 32, 32, 16, 256, 1, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_32_preload_early_128
    MEMCPY_VARIANT 1, 32, 32, 32, 128, 1, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_32_preload_early_192
    MEMCPY_VARIANT 1, 32, 32, 32, 192, 1, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_32_preload_early_256
    MEMCPY_VARIANT 1, 32, 32, 32, 256, 1, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_16_no_preload
    MEMCPY_VARIANT 1, 32, 32, 16, 0, 0, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_align_32_block_write_32_no_preload
    MEMCPY_VARIANT 1, 32, 32, 32, 0, 0, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_line_64_align_32_block_write_32_preload_early_128
    MEMCPY_VARIANT 1, 64, 32, 32, 128, 1, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_line_64_align_32_block_write_32_preload_early_192
    MEMCPY_VARIANT 1, 64, 32, 32, 192, 1, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_line_64_align_32_block_write_32_preload_early_256
    MEMCPY_VARIANT 1, 64, 32, 32, 256, 1, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_line_64_align_32_block_write_32_preload_early_320
    MEMCPY_VARIANT 1, 64, 32, 32, 320, 1, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_line_64_align_64_block_write_32_preload_early_192
    MEMCPY_VARIANT 1, 64, 64, 32, 192, 1, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_line_64_align_64_block_write_32_preload_early_256
    MEMCPY_VARIANT 1, 64, 64, 32, 256, 1, 0
.endfunc

asm_function memcpy_armv5te_no_overfetch_line_64_align_64_block_write_32_preload_early_320
    MEMCPY_VARIANT 1, 64, 64, 32, 320, 1, 0
.endfunc

/* Overfetching versions. */

asm_function memcpy_armv5te_overfetch_align_16_block_write_16_preload_early_128
    MEMCPY_VARIANT 1, 32, 16, 16, 128, 1, 1
.endfunc

asm_function memcpy_armv5te_overfetch_align_32_block_write_32_preload_early_192
    MEMCPY_VARIANT 1, 32, 32, 32, 192, 1, 1
.endfunc

asm_function memcpy_simple_sunxi_preload_early_192
    MEMCPY_VARIANT_SIMPLE 1, 64, 32, 32, 192, 1, 1, 1, 0, 0
.endfunc

asm_function memcpy_simple_sunxi_preload_early_192_no_catch_up
    MEMCPY_VARIANT_SIMPLE 1, 64, 32, 32, 192, 0, 1, 1, 0, 0
.endfunc

asm_function memcpy_simple_sunxi_preload_early_192_no_catch_up_check_small_size_alignment
    MEMCPY_VARIANT_SIMPLE 1, 64, 32, 32, 192, 0, 1, 1, 0, 1
.endfunc

asm_function memcpy_simple_sunxi_preload_early_256
    MEMCPY_VARIANT_SIMPLE 1, 64, 32, 32, 256, 1, 1, 1, 0, 0
.endfunc

asm_function memcpy_simple_sunxi_preload_early_256_no_catch_up
    MEMCPY_VARIANT_SIMPLE 1, 64, 32, 32, 256, 0, 1, 1, 0, 0
.endfunc

asm_function memcpy_simple_rpi_preload_early_96
    MEMCPY_VARIANT_SIMPLE 1, 32, 32, 16, 96, 1, 1, 1, 1, 1
.endfunc

asm_function memcpy_simple_rpi_preload_early_96_no_catch_up
    MEMCPY_VARIANT_SIMPLE 1, 32, 32, 16, 96, 0, 1, 1, 1, 0
.endfunc

asm_function memcpy_simple_rpi_preload_early_96_no_catch_up_check_small_size_alignment
    MEMCPY_VARIANT_SIMPLE 1, 32, 32, 16, 96, 0, 1, 1, 1, 1
.endfunc

asm_function memcpy_simple_rpi_preload_early_128
    MEMCPY_VARIANT_SIMPLE 1, 32, 32, 16, 128, 1, 1, 1, 1, 1
.endfunc

asm_function memcpy_simple_rpi_preload_early_128_no_catch_up
    MEMCPY_VARIANT_SIMPLE 1, 32, 32, 16, 128, 0, 1, 1, 1, 1
.endfunc

#endif

#endif

#endif
