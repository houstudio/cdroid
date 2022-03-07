/* Copyright (c) 2010-2011, Linaro Limited
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

      * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

      * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

      * Neither the name of Linaro Limited nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   Written by Dave Gilbert <david.gilbert@linaro.org>

   This memcpy routine is optimised on a Cortex-A9 and should work on
   all ARMv7 processors with NEON. */

/* Modified:
 * Change preload offset to 192.
 */
#define PRELOAD_OFFSET 192

@ 2011-09-01 david.gilbert@linaro.org
@    Extracted from local git 2f11b436

	.syntax unified
	.arch armv7-a
	.fpu neon

@ this lets us check a flag in a 00/ff byte easily in either endianness
#ifdef __ARMEB__
#define CHARTSTMASK(c) 1<<(31-(c*8))
#else
#define CHARTSTMASK(c) 1<<(c*8)
#endif
	.text
	.thumb

@ ---------------------------------------------------------------------------
	.thumb_func
	.align 2
	.p2align 4,,15
	.global memcpy_hybrid
	.type memcpy_hybrid,%function
memcpy_hybrid:
	@ r0 = dest
	@ r1 = source
	@ r2 = count
	@ returns dest in r0
	@ Overlaps of source/dest not allowed according to spec
	@ Note this routine relies on v7 misaligned loads/stores
	pld	[r1]
	mov	r12, r0		@ stash original r0
	cmp	r2,#32
	blt	10f		@ take the small copy case separately

	@ test for either source or destination being misaligned
	@ (We only rely on word align)
	tst	r0,#3
	it	eq
	tsteq	r1,#3
	bne	30f		@ misaligned case

4:
	@ at this point we are word (or better) aligned and have at least
	@ 32 bytes to play with

	@ If it's a huge copy,  try Neon
	cmp	r2, #128*1024
	bge	35f		@ Sharing general non-aligned case here, aligned could be faster
	
	push	{r3,r4,r5,r6,r7,r8,r10,r11}
5:
	ldmia	r1!,{r3,r4,r5,r6,r7,r8,r10,r11}
	sub	r2,r2,#32
	pld	[r1,#PRELOAD_OFFSET]
	cmp	r2,#32
	stmia	r0!,{r3,r4,r5,r6,r7,r8,r10,r11}
	bge	5b

	pop	{r3,r4,r5,r6,r7,r8,r10,r11}
	@ We are now down to less than 32 bytes
	cbz	r2,15f		@ quick exit for the case where we copied a multiple of 32

10:  @ small copies (not necessarily aligned - note might be slightly more than 32bytes)
	cmp	r2,#4
	blt	12f
11:
	sub	r2,r2,#4
	cmp	r2,#4
	ldr	r3, [r1],#4
	str	r3, [r0],#4
	bge	11b
12:
	tst	r2,#2
	itt	ne
	ldrhne	r3, [r1],#2
	strhne	r3, [r0],#2
	
	tst	r2,#1
	itt	ne
	ldrbne	r3, [r1],#1
	strbne	r3, [r0],#1
	
15:  @ exit
	mov	r0,r12		@ restore r0
	bx	lr

	.align 2
	.p2align 4,,15
30:  @ non-aligned - at least 32 bytes to play with
	@ Test for co-misalignment
	eor	r3, r0, r1
	tst	r3,#3
	beq	50f

	@ Use Neon for misaligned
35:
	vld1.8	{d0,d1,d2,d3}, [r1]!
	sub	r2,r2,#32
	cmp	r2,#32
	pld	[r1,#PRELOAD_OFFSET]
	vst1.8	{d0,d1,d2,d3}, [r0]!
	bge	35b
	b	10b		@ TODO: Probably a bad idea to switch to ARM at this point

	.align 2
	.p2align 4,,15
50: @ Co-misaligned
	@ At this point we've got at least 32 bytes
51:
	ldrb	r3,[r1],#1
	sub	r2,r2,#1
	strb	r3,[r0],#1
	tst	r0,#7
	bne	51b

	cmp	r2,#32
	blt	10b
	b	4b
