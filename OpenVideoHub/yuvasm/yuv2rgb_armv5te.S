/*
 * Copyright (c) 2007-2009 BLStream Oy.
 *
 * This file is part of OpenVideoHub.
 *
 * OpenVideoHub is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * OpenVideoHub is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpenVideoHub; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

@.arch armv5te

#ifdef __ARM_EABI__
#define DWORD_ALIGNED_STACK 1
#endif

        .macro yuv2rgb_stackalign_armv5te
#ifndef DWORD_ALIGNED_STACK
        tst    sp, #4
        strne  lr, [sp, #-4]!
        adrne  lr, unaligned_return_thunk_armv5te
#endif
        .endm

	.text

	.balign	16
cGU:	.long	-24759
cBU:	.long	132798
cGV:	.long	-53109
cRV:	.long	104448

/****************************************/

	.balign 32
	.global YUV2RGB_Convert
	.type YUV2RGB_Convert, %function
	.func YUV2RGB_Convert
YUV2RGB_Convert:

	yuv2rgb_stackalign_armv5te

	stmfd	sp!, {v1, v2, v3, v4, v5, v6, v7, v8, lr}
	stmfd	sp!, {a2, a3, a4}

	ldr	a2, [a1], #4		@ Y*
	ldr	a3, [a1], #4		@ U*
	ldr	a4, [a1], #4		@ V*

	ldr	v1, [sp, #8]		@ srcStride
	ldr	v2, [sp, #4]		@ srcHeight

	stmfd	sp!, {a2, a3, a4}

	mla	v3, v1, v2, a2

	ldr	v1, [sp, #0x48]		@ dstStride

	ldr	a1, [sp, #0x4c]		@ dst*
	ldr	ip, [sp, #0x0c]		@ srcWidth
	add	ip, ip, a2		@ srcEndRow

	str	v3, [sp, #0x10]		@ srcY + srcHeight * srcStride

	mov	v1, v1, lsl #2
	str	v1, [sp, #0x48]

loop_Convert:
	
	ldrb	v5, [a3], #1		@ U
	ldrb	v6, [a4], #1		@ V

	ldr	v2, cGU
	ldr	v4, cBU
	ldr	v3, cGV
	ldr	v1, cRV

	sub	v5, v5, #128
	sub	v6, v6, #128

	smulwb	v7, v2, v5		@ GU
	smulwb	v5, v4, v5		@ BU
	smlawb	v7, v3, v6, v7		@ GUV
	smulwb	v6, v1, v6		@ RV

	ldr	v4, CM
	ldr	v8, [sp, #0x14]		@ srcStride
	ldr	lr, [sp, #0x48]		@ dstStride

	@	px00

	ldrb	v1, [a2]		@ RY
	ldrb	v2, [a2]		@ GY
	ldrb	v3, [a2], #1		@ BY

	add	v1, v1, v6
	add	v2, v2, v7
	add	v3, v3, v5

	ldrb	v1, [v4, v1]
	ldrb	v2, [v4, v2]
	ldrb	v3, [v4, v3]

	strh	v1, [a1, #2]
	strb	v2, [a1, #1]
	strb	v3, [a1], #4

	@	px10

	ldrb	v1, [a2]		@ RY
	ldrb	v2, [a2]		@ GY
	ldrb	v3, [a2], v8		@ BY, a2 - next line

	add	v1, v1, v6
	add	v2, v2, v7
	add	v3, v3, v5

	ldrb	v1, [v4, v1]
	ldrb	v2, [v4, v2]
	ldrb	v3, [v4, v3]

	strh	v1, [a1, #2]
	strb	v2, [a1, #1]
	strb	v3, [a1], lr

	@	px11

	ldrb	v1, [a2]		@ RY
	ldrb	v2, [a2]		@ GY
	ldrb	v3, [a2], #-1		@ BY

	add	v1, v1, v6
	add	v2, v2, v7
	add	v3, v3, v5

	ldrb	v1, [v4, v1]
	ldrb	v2, [v4, v2]
	ldrb	v3, [v4, v3]

	strh	v1, [a1, #2]
	strb	v2, [a1, #1]
	strb	v3, [a1], #-4

	@	px01

	ldrb	v1, [a2]		@ RY
	ldrb	v2, [a2]		@ GY
	ldrb	v3, [a2], #2		@ BY, a2 - next 4pix
	sub	a2, a2, v8

	add	v1, v1, v6
	add	v2, v2, v7
	add	v3, v3, v5

	ldrb	v1, [v4, v1]
	ldrb	v2, [v4, v2]
	ldrb	v3, [v4, v3]

	strh	v1, [a1, #2]
	strb	v2, [a1, #1]
	strb	v3, [a1], #8
	sub	a1, a1, lr

	cmp	a2, ip
	blt	loop_Convert

	ldr	a1, [sp, #0x4c]
	ldr	v7, [sp, #0x3c]		@ strideUV
	ldr	v6, [sp, #0x10]		@ srcEndTotal

	ldmfd	sp!, {a2, a3, a4}

	add	a1, a1, lr, lsl #1
	add	ip, ip, v8, lsl #1
	add	a2, a2, v8, lsl #1
	add	a3, a3, v7
	add	a4, a4, v7

	stmfd	sp!, {a2, a3, a4}
	str	a1, [sp, #0x4c]

	cmp	a2, v6
	blt	loop_Convert

	add	sp, sp, #24
	ldmfd	sp!, {v1, v2, v3, v4, v5, v6, v7, v8, pc}

	.endfunc

/****************************************/

	.balign 32
	.global YUV2RGB_ConvertRotate
	.type YUV2RGB_ConvertRotate, %function
	.func YUV2RGB_ConvertRotate
YUV2RGB_ConvertRotate:

	yuv2rgb_stackalign_armv5te

	stmfd	sp!, {v1, v2, v3, v4, v5, v6, v7, v8, lr}
	stmfd	sp!, {a2, a3, a4}

	ldr	a2, [a1], #4		@ Y*
	ldr	a3, [a1], #4		@ U*
	ldr	a4, [a1], #4		@ V*

	ldr	v1, [sp, #8]		@ srcStride
	ldr	v2, [sp, #4]		@ srcHeight

	stmfd	sp!, {a2, a3, a4}

	mla	v3, v1, v2, a2

	ldr	v1, [sp, #0x48]		@ dstStride
	ldr	v2, [sp, #0x40]		@ dstWidth

	ldr	a1, [sp, #0x4c]		@ dst*
	ldr	ip, [sp, #0x0c]		@ srcWidth
	add	ip, ip, a2		@ srcEndRow

	str	v3, [sp, #0x10]		@ srcY + srcHeight * srcStride

	mov	v1, v1, lsl #2
	str	v1, [sp, #0x48]

	sub	v2, v2, #1
	add	a1, a1, v2, lsl #2
	str	a1, [sp, #0x4c]

loop_ConvertRotate:
	
	ldrb	v5, [a3], #1		@ U
	ldrb	v6, [a4], #1		@ V

	ldr	v2, cGU
	ldr	v4, cBU
	ldr	v3, cGV
	ldr	v1, cRV

	sub	v5, v5, #128
	sub	v6, v6, #128

	smulwb	v7, v2, v5		@ GU
	smulwb	v5, v4, v5		@ BU
	smlawb	v7, v3, v6, v7		@ GUV
	smulwb	v6, v1, v6		@ RV

	ldr	v4, CM
	ldr	v8, [sp, #0x14]		@ srcStride
	ldr	lr, [sp, #0x48]		@ dstStride

	@	px00

	ldrb	v1, [a2]		@ RY
	ldrb	v2, [a2]		@ GY
	ldrb	v3, [a2], #1		@ BY

	add	v1, v1, v6
	add	v2, v2, v7
	add	v3, v3, v5

	ldrb	v1, [v4, v1]
	ldrb	v2, [v4, v2]
	ldrb	v3, [v4, v3]

	strh	v1, [a1, #2]
	strb	v2, [a1, #1]
	strb	v3, [a1], lr

	@	px10

	ldrb	v1, [a2]		@ RY
	ldrb	v2, [a2]		@ GY
	ldrb	v3, [a2], v8		@ BY, a2 - next line

	add	v1, v1, v6
	add	v2, v2, v7
	add	v3, v3, v5

	ldrb	v1, [v4, v1]
	ldrb	v2, [v4, v2]
	ldrb	v3, [v4, v3]

	strh	v1, [a1, #2]
	strb	v2, [a1, #1]
	strb	v3, [a1], #-4

	@	px11

	ldrb	v1, [a2]		@ RY
	ldrb	v2, [a2]		@ GY
	ldrb	v3, [a2], #-1		@ BY

	add	v1, v1, v6
	add	v2, v2, v7
	add	v3, v3, v5

	ldrb	v1, [v4, v1]
	ldrb	v2, [v4, v2]
	ldrb	v3, [v4, v3]

	strh	v1, [a1, #2]
	strb	v2, [a1, #1]
	strb	v3, [a1]
	sub	a1, a1, lr

	@	px01

	ldrb	v1, [a2]		@ RY
	ldrb	v2, [a2]		@ GY
	ldrb	v3, [a2], #2		@ BY, a2 - next 4pix
	sub	a2, a2, v8

	add	v1, v1, v6
	add	v2, v2, v7
	add	v3, v3, v5

	ldrb	v1, [v4, v1]
	ldrb	v2, [v4, v2]
	ldrb	v3, [v4, v3]

	strh	v1, [a1, #2]
	strb	v2, [a1, #1]
	strb	v3, [a1], #4
	add	a1, a1, lr, lsl #1

	cmp	a2, ip
	blt	loop_ConvertRotate

	ldr	a1, [sp, #0x4c]
	ldr	v7, [sp, #0x3c]		@ strideUV
	ldr	v6, [sp, #0x10]		@ srcEndTotal

	ldmfd	sp!, {a2, a3, a4}

	sub	a1, a1, #8
	add	ip, ip, v8, lsl #1
	add	a2, a2, v8, lsl #1
	add	a3, a3, v7
	add	a4, a4, v7

	stmfd	sp!, {a2, a3, a4}
	str	a1, [sp, #0x4c]

	cmp	a2, v6
	blt	loop_ConvertRotate

	add	sp, sp, #24
	ldmfd	sp!, {v1, v2, v3, v4, v5, v6, v7, v8, pc}

	.endfunc

/****************************************/

	.balign 32
	.global YUV2RGB_ConvertScaleYUV
	.type YUV2RGB_ConvertScaleYUV, %function
	.func YUV2RGB_ConvertScaleYUV
YUV2RGB_ConvertScaleYUV:

	yuv2rgb_stackalign_armv5te

	stmfd	sp!, {v1, v2, v3, v4, v5, v6, v7, v8, lr}
	stmfd	sp!, {a2, a3, a4}

	ldr	a2, [a1], #4		@ Y*
	ldr	a3, [a1], #4		@ U*
	ldr	a4, [a1], #4		@ V*

	stmfd	sp!, {a2, a3, a4}

	ldr	a2, [sp, #0x0c]		@ srcWidth
	ldr	a1, [sp, #0x40]		@ dstWidth
	mov	a2, a2, lsl #16
	bl	YUV2RGB_GetFactor
	mov	v1, a1			@ dx

	ldr	a2, [sp, #0x10]		@ srcHeight
	ldr	a1, [sp, #0x44]		@ dstHeight
	mov	a2, a2, lsl #16
	bl	YUV2RGB_GetFactor
	mov	lr, a1			@ dy

	mov	ip, v1

	ldr	v3, [sp, #0x48]		@ dstStride
	ldr	v4, [sp, #0x14]		@ srcStride

	ldr	v6, [sp, #0x44]		@ dstHeight

	ldr	a1, [sp, #0x00]		@ Y*
	ldr	a2, [sp, #0x00]		@ Y*
	ldr	a3, [sp, #0x04]		@ U*
	ldr	a4, [sp, #0x08]		@ V*
	ldr	v2, [sp, #0x4c]		@ dst*
	ldr	v1, [sp, #0x4c]		@ dst*
	add	v2, v2, v3, lsl #2	@ 2nd line
	smlatb	a2, lr, v4, a2		@ 2nd line

	mov	v3, v3, lsl #2		@ byte unit

	mla	v6, v3, v6, v1
	str	v6, [sp, #0x44]		@ dstEndImage

	ldr	v6, [sp, #0x40]		@ dstWidth
	mov	v5, #0			@ lineIndexer

	add	v6, v1, v6, lsl #2	@ dstEndRow

	mov	v8, #0

loop_ConvertScaleYUVScale:

	mov	v7, v8, asr #17

	ldrb	v3, [a3, v7]		@ U
	ldrb	v4, [a4, v7]		@ V

	sub	v3, v3, #128
	sub	v4, v4, #128

	strh	v3, [v1], #2
	strh	v4, [v1], #6

	mov	v7, v8, asr #16
	ldrb	v3, [a1, v7]		@ Y00
	ldrb	v4, [a2, v7]		@ Y10
	add	v8, v8, ip

	strb	v3, [v2], #2
	strb	v4, [v2], #-1

	mov	v7, v8, asr #16
	ldrb	v3, [a1, v7]		@ Y01
	ldrb	v4, [a2, v7]		@ Y11
	add	v8, v8, ip

	strb	v3, [v2], #2
	strb	v4, [v2], #5

	cmp	v1, v6
	blt	loop_ConvertScaleYUVScale


	ldr	v8, [sp, #0x3c]		@ srcStrideUV

	ldr	a3, [sp, #0x04]		@ U*
	ldr	a4, [sp, #0x08]		@ V*

	add	v5, v5, lr
	mov	a1, v5, asr #1
	smlatb	a3, a1, v8, a3
	smlatb	a4, a1, v8, a4

	ldr	v8, [sp, #0x14]		@ srcStride

	ldr	a1, [sp, #0x00]		@ Y*
	ldr	a2, [sp, #0x00]		@ Y*

	smlatb	a1, v5, v8, a1

	add	v5, v5, lr
	smlatb	a2, v5, v8, a2

	ldr	v7, [sp, #0x40]

	ldr	v8, [sp, #0x44]

	sub	v1, v1, v7, lsl #2
	sub	v2, v2, v7, lsl #2

	ldr	v7, [sp, #0x48]		@ dstStride

	cmp	v1, v8

	add	v6, v6, v7, lsl #3
	add	v1, v1, v7, lsl #3
	add	v2, v2, v7, lsl #3

	mov	v8, #0

	blt	loop_ConvertScaleYUVScale

	@@@@	Actual Convertion

	ldr	a1, [sp, #0x4c]		@ dst* , UV*
	ldr	v8, [sp, #0x48]		@ aDstStride
	ldr	v7, [sp, #0x40]		@ aDstWidth
	add	a2, a1, v8, lsl #2	@ dst* 2nd line , Y*
	add	ip, a1, v7, lsl #2	@ endRow

	ldr	v2, cGU
	ldr	v4, cBU
	ldr	v3, cGV
	ldr	v1, cRV

	ldr	lr, CM

loop_ConvertScaleYUVConvert:

	ldr	v5, [a1]		@ UV

	ldrb	a3, [a2]		@ RY
	ldrb	a4, [a2]		@ GY
	ldrb	v8, [a2]		@ BY

	smulwb	v6, v2, v5		@ GU
	smulwb	v7, v4, v5		@ BU
	smlawt	v6, v3, v5, v6		@ GUV
	smulwt	v5, v1, v5		@ RV

	@	px00

	add	a3, a3, v5
	add	a4, a4, v6
	add	v8, v8, v7

	ldrb	a3, [lr, a3]
	ldrb	a4, [lr, a4]
	ldrb	v8, [lr, v8]

	strh	a3, [a1, #2]
	strb	a4, [a1, #1]
	strb	v8, [a1], #4

	@	px10

	ldrb	a3, [a2, #1]		@ RY
	ldrb	a4, [a2, #1]		@ GY
	ldrb	v8, [a2, #1]		@ BY

	add	a3, a3, v5
	add	a4, a4, v6
	add	v8, v8, v7

	ldrb	a3, [lr, a3]
	ldrb	a4, [lr, a4]
	ldrb	v8, [lr, v8]

	strh	a3, [a1, #2]
	strb	a4, [a1, #1]
	strb	v8, [a1], #4

	@	px11

	ldrb	a3, [a2, #3]		@ RY
	ldrb	a4, [a2, #3]		@ GY
	ldrb	v8, [a2, #3]		@ BY

	add	a3, a3, v5
	add	a4, a4, v6
	add	v8, v8, v7

	ldrb	a3, [lr, a3]
	ldrb	a4, [lr, a4]
	ldrb	v8, [lr, v8]

	strh	a3, [a2, #6]
	strb	a4, [a2, #5]
	strb	v8, [a2, #4]

	@	px01

	ldrb	a3, [a2, #2]		@ RY
	ldrb	a4, [a2, #2]		@ GY
	ldrb	v8, [a2, #2]		@ BY

	add	a3, a3, v5
	add	a4, a4, v6
	add	v8, v8, v7

	ldrb	a3, [lr, a3]
	ldrb	a4, [lr, a4]
	ldrb	v8, [lr, v8]

	strh	a3, [a2, #2]
	strb	a4, [a2, #1]
	strb	v8, [a2], #8

	cmp	a1, ip
	blt	loop_ConvertScaleYUVConvert

	ldr	a1, [sp, #0x4c]		@ dst*
	ldr	v8, [sp, #0x48]		@ aDstStride

	ldr	lr, [sp, #0x44]		@ endImage

	add	a1, a1, v8, lsl #3
	add	a2, a1, v8, lsl #2
	add	ip, ip, v8, lsl #3
	str	a1, [sp, #0x4c]

	cmp	a1, lr
	ldr	lr, CM
	blt	loop_ConvertScaleYUVConvert

	add	sp, sp, #24
	ldmfd	sp!, {v1, v2, v3, v4, v5, v6, v7, v8, pc}

	.endfunc

/****************************************/

	.balign 32
	.global YUV2RGB_ConvertRotateScaleYUV
	.type YUV2RGB_ConvertRotateScaleYUV, %function
	.func YUV2RGB_ConvertRotateScaleYUV
YUV2RGB_ConvertRotateScaleYUV:

	yuv2rgb_stackalign_armv5te

	stmfd	sp!, {v1, v2, v3, v4, v5, v6, v7, v8, lr}
	stmfd	sp!, {a2, a3, a4}

	ldr	a2, [a1], #4		@ Y*
	ldr	a3, [a1], #4		@ U*
	ldr	a4, [a1], #4		@ V*

	stmfd	sp!, {a2, a3, a4}

	ldr	a2, [sp, #0x10]		@ srcHeight
	ldr	a1, [sp, #0x40]		@ dstWidth
	mov	a2, a2, lsl #16
	bl	YUV2RGB_GetFactor
	mov	v1, a1			@ dx

	ldr	a2, [sp, #0x0c]		@ srcWidth
	ldr	a1, [sp, #0x44]		@ dstHeight
	mov	a2, a2, lsl #16
	bl	YUV2RGB_GetFactor
	mov	lr, a1			@ dy

	mov	ip, v1

	ldr	v3, [sp, #0x48]		@ dstStride
	ldr	v4, [sp, #0x14]		@ srcStride

	ldr	v6, [sp, #0x44]		@ dstHeight

	ldr	a1, [sp, #0x3c]		@ strideUV
	ldr	a2, [sp, #0x14]		@ srcStride

	ldr	a3, [sp, #0x04]		@ U*
	ldr	a4, [sp, #0x08]		@ V*
	orr	a2, a2, a1, lsl #16	@ srcStride, srcStrideUV
	ldr	a1, [sp, #0x00]		@ Y*
	ldr	v2, [sp, #0x4c]		@ dst*
	ldr	v1, [sp, #0x4c]		@ dst*
	add	v2, v2, v3, lsl #2	@ 2nd line

	mov	v3, v3, lsl #2		@ byte unit

	mla	v6, v3, v6, v1
	str	v6, [sp, #0x44]		@ dstEndImage

	ldr	v6, [sp, #0x40]		@ dstWidth
	ldr	v8, [sp, #0x10]		@ srcHeight
	mov	v5, #0			@ lineIndexer

	add	v6, v1, v6, lsl #2	@ dstEndRow

	mov	v8, v8, lsl #16

loop_ConvertRotateScaleYUVScale:

	sub	v8, v8, ip
	smultb	v7, v8, a2
	ldrb	v3, [a1, v7]		@ Y00
	add	v7, v7, lr, asr #16
	ldrb	v4, [a1, v7]		@ Y10
	strb	v3, [v2], #2
	strb	v4, [v2], #-1

	sub	v8, v8, ip
	smultb	v7, v8, a2
	ldrb	v3, [a1, v7]		@ Y01
	add	v7, v7, lr, asr #16
	ldrb	v4, [a1, v7]		@ Y11
	strb	v3, [v2], #2
	strb	v4, [v2], #5

	mov	v7, v8, asr #1
	smultt	v7, v7, a2
	ldrb	v3, [a3, v7]		@ U
	ldrb	v4, [a4, v7]		@ V
	sub	v3, v3, #128
	sub	v4, v4, #128
	strh	v3, [v1], #2
	strh	v4, [v1], #6

	cmp	v1, v6
	blt	loop_ConvertRotateScaleYUVScale

	ldr	a3, [sp, #0x04]		@ U*
	ldr	a4, [sp, #0x08]		@ V*

	add	v5, v5, lr
	add	a3, a3, v5, asr #16
	add	a4, a4, v5, asr #16

	ldr	a1, [sp, #0x00]		@ Y*

	add	a1, a1, v5, asr #15

	ldr	v7, [sp, #0x40]		@ dstWidth

	ldr	v8, [sp, #0x44]		@ endImage

	sub	v1, v1, v7, lsl #2
	sub	v2, v2, v7, lsl #2

	ldr	v7, [sp, #0x48]		@ dstStride

	cmp	v1, v8

	ldr	v8, [sp, #0x10]		@ srcHeight

	add	v6, v6, v7, lsl #3
	add	v1, v1, v7, lsl #3
	add	v2, v2, v7, lsl #3

	mov	v8, v8, lsl #16

	blt	loop_ConvertRotateScaleYUVScale

	@@@@	Actual Convertion

	ldr	a1, [sp, #0x4c]		@ dst* , UV*
	ldr	v8, [sp, #0x48]		@ aDstStride
	ldr	v7, [sp, #0x40]		@ aDstWidth
	add	a2, a1, v8, lsl #2	@ dst* 2nd line , Y*
	add	ip, a1, v7, lsl #2	@ endRow

	ldr	v2, cGU
	ldr	v4, cBU
	ldr	v3, cGV
	ldr	v1, cRV

	ldr	lr, CM

loop_ConvertRotateScaleYUVConvert:

	ldr	v5, [a1]		@ UV

	ldrb	a3, [a2]		@ RY
	ldrb	a4, [a2]		@ GY
	ldrb	v8, [a2]		@ BY

	smulwb	v6, v2, v5		@ GU
	smulwb	v7, v4, v5		@ BU
	smlawt	v6, v3, v5, v6		@ GUV
	smulwt	v5, v1, v5		@ RV

	@	px00

	add	a3, a3, v5
	add	a4, a4, v6
	add	v8, v8, v7

	ldrb	a3, [lr, a3]
	ldrb	a4, [lr, a4]
	ldrb	v8, [lr, v8]

	strh	a3, [a1, #2]
	strb	a4, [a1, #1]
	strb	v8, [a1], #4

	@	px10

	ldrb	a3, [a2, #1]		@ RY
	ldrb	a4, [a2, #1]		@ GY
	ldrb	v8, [a2, #1]		@ BY

	add	a3, a3, v5
	add	a4, a4, v6
	add	v8, v8, v7

	ldrb	a3, [lr, a3]
	ldrb	a4, [lr, a4]
	ldrb	v8, [lr, v8]

	strh	a3, [a1, #2]
	strb	a4, [a1, #1]
	strb	v8, [a1], #4

	@	px11

	ldrb	a3, [a2, #3]		@ RY
	ldrb	a4, [a2, #3]		@ GY
	ldrb	v8, [a2, #3]		@ BY

	add	a3, a3, v5
	add	a4, a4, v6
	add	v8, v8, v7

	ldrb	a3, [lr, a3]
	ldrb	a4, [lr, a4]
	ldrb	v8, [lr, v8]

	strh	a3, [a2, #6]
	strb	a4, [a2, #5]
	strb	v8, [a2, #4]

	@	px01

	ldrb	a3, [a2, #2]		@ RY
	ldrb	a4, [a2, #2]		@ GY
	ldrb	v8, [a2, #2]		@ BY

	add	a3, a3, v5
	add	a4, a4, v6
	add	v8, v8, v7

	ldrb	a3, [lr, a3]
	ldrb	a4, [lr, a4]
	ldrb	v8, [lr, v8]

	strh	a3, [a2, #2]
	strb	a4, [a2, #1]
	strb	v8, [a2], #8

	cmp	a1, ip
	blt	loop_ConvertRotateScaleYUVConvert

	ldr	a1, [sp, #0x4c]		@ dst*
	ldr	v8, [sp, #0x48]		@ aDstStride

	ldr	lr, [sp, #0x44]		@ endImage

	add	a1, a1, v8, lsl #3
	add	a2, a1, v8, lsl #2
	add	ip, ip, v8, lsl #3
	str	a1, [sp, #0x4c]

	cmp	a1, lr
	ldr	lr, CM
	blt	loop_ConvertRotateScaleYUVConvert

	add	sp, sp, #24
	ldmfd	sp!, {v1, v2, v3, v4, v5, v6, v7, v8, pc}

	.endfunc

/****************************************/

	.balign 32
	.global YUV2RGB_ScaleUpRGB
	.type YUV2RGB_ScaleUpRGB, %function
	.func YUV2RGB_ScaleUpRGB
YUV2RGB_ScaleUpRGB:

	yuv2rgb_stackalign_armv5te

	stmfd	sp!, {v1, v2, v3, v4, v5, v6, v7, v8, lr}

	ldr	v7, [sp, #0x24]		@ aDstWidth
	ldr	v8, [sp, #0x28]		@ aDstStride
	ldr	lr, [sp, #0x2c]		@ aDstSize

	sub	v4, v8, v7		@ dstMod

	add	lr, a1, lr, lsl #2	@ endImage

	add	ip, a1, v7, lsl #2
	mov	v3, a2
	mov	v5, #0
	mov	v6, #0

loop_ScaleUpRGB:

	mov	v2, v5, asr #16
	bic	v2, v2, #3
	ldr	v1, [a2, v2]
	add	v5, a3, lsl #2
	str	v1, [a1], #4

	cmp	a1, ip
	blt	loop_ScaleUpRGB

	add	v6, v6, a4
	smultb	v5, v6, v8

	add	a1, a1, v4, lsl #2
	add	ip, ip, v8, lsl #2

	add	a2, v3, v5, lsl #2
	mov	v5, #0

	cmp	a1, lr
	blt	loop_ScaleUpRGB

	ldmfd	sp!, {v1, v2, v3, v4, v5, v6, v7, v8, pc}

	.endfunc

/****************************************/

@ unsigned 32/32-bit divide by Newton-Raphson

q	.req	a1	@ input denominator d, output quotient q
r	.req	a2	@ input numerator n, output remainder r
s	.req	a3	@ scratch
m	.req	a4	@ scratch
a	.req	ip	@ scratch

	.balign 32
	.global YUV2RGB_GetFactor
	.type YUV2RGB_GetFactor, %function
	.func YUV2RGB_GetFactor
YUV2RGB_GetFactor:

	clz	s, q			@ find normalizing shift
	movs	a, q, lsl s		@ perform a lookup on the
	add	a, pc, a, lsr #25	@ most significant 7 bits
	ldrneb	a, [a, #t32-b32-64]	@ of divisor
b32:	subs	s, s, #7		@ correct shift
	rsb	m, q, #0		@ m = -d
	movpl	q, a, lsl s		@ q approx (1<<32)/d
	@ 1st Newton iteration follows
	mulpl	a, q, m			@ a = -q*d
	bmi	GF_large_d		@ large d trap
	smlawt	q, q, a, q		@ q approx q-(q*q*d>>32)
	teq	m, m, asr #1		@ check for d=0 or d=1
	@ 2nd Newton iteration follows
	mulne	a, q, m			@ a = -q*d
	movne	s, #0			@ s = 0
	smlalne	s, q, a, q		@ q = q-(q*q*d>>32)
	beq	GF_by_0_or_1		@ trap d=0 or d=1
	@ now accurate enough for a remainder r, 0<=r<3*d
	umull	s, q, r, q		@ q = (r*q)>>32
	add	r, r, m			@ r = n-d
	mla	r, q, m, r		@ r = n-(q+1)*d
	@ since 0 <= n-q*d < 3*d, thus -d <= r < 2*d
	cmn	r, m			@ t = r-d
	subcs	r, r, m			@ if (t<-d || t>=0) r=r+d
	addcc	q, q, #1		@ if (-d<=t && t<0) q=q+1
	addpl	r, r, #1		@ if (t>=0) { r=r-2*d
	addpl	q, q, #2		@             q=q+2 }
	bx	lr			@ return {q, r}
GF_large_d:
	@ at this point we know 2 >= 2^(31-6)=2^25
	sub	a, a, #4		@ set q to be an
	rsb	s, s, #0		@ underestimate of
	mov	q, a, lsr s		@ (1<<32)/d
	umull	s, q, r, q		@ q = (n*q)>>32
	mla	r, q, m, r		@ r = n-q*d
	@ now accurate enough for a remainder r, 0<=r<4*d
	cmn	m, r, lsr #1		@ if (r/2 >= d)
	addcs	r, r, m, lsr #1		@ { r=r-2*d;
	addcs	q, q, #2		@   q=q+2; }
	cmn	m, r			@ if (r >= d)
	addcs	r, r, m			@ { r=r-d;
	addcs	q, q, #1		@   q=q+1; }
	bx	lr			@ return {q, r}
GF_by_0_or_1:
	@ carry set if d=1, carry clear if d=0
	movcs	q, r			@ if (d==1) { q=n;
	movcs	r, #0			@             r=0; }
	movcc	q, #-1			@ if (d==0) { q=-1;
	movcc	r, #-1			@             r=-1; }
	bx	lr			@ return {q, r}

	@ table for 32 by 32 bit Newton Raphson divisions
	@ table[0] = 255
	@ table[i] = (1<<14)/(64+i) for i=0,1,2,...,63
t32:	.byte	0xff, 0xfc, 0xf8, 0xf4, 0xf0, 0xed, 0xea, 0xe6
	.byte	0xe3, 0xe0, 0xdd, 0xda, 0xd7, 0xd4, 0xd2, 0xcf
	.byte	0xcc, 0xca, 0xc7, 0xc5, 0xc3, 0xc0, 0xbe, 0xbc
	.byte	0xba, 0xb8, 0xb6, 0xb4, 0xb2, 0xb0, 0xae, 0xac
	.byte	0xaa, 0xa8, 0xa7, 0xa5, 0xa3, 0xa2, 0xa0, 0x9f
	.byte	0x9d, 0x9c, 0x9a, 0x99, 0x97, 0x96, 0x94, 0x93
	.byte	0x92, 0x90, 0x8f, 0x8e, 0x8d, 0x8c, 0x8a, 0x89
	.byte	0x88, 0x87, 0x86, 0x85, 0x84, 0x83, 0x82, 0x81

	.endfunc

/****************************************/

	.long	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
	.long	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
	.long	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
	.long	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
	.long	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
	.long	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
	.long	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
	.long	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
	.long	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
	.long	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
	.long	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
	.long	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
CMX:	.long	0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c, 0x13121110, 0x17161514, 0x1b1a1918, 0x1f1e1d1c
	.long	0x23222120, 0x27262524, 0x2b2a2928, 0x2f2e2d2c, 0x33323130, 0x37363534, 0x3b3a3938, 0x3f3e3d3c
	.long	0x43424140, 0x47464544, 0x4b4a4948, 0x4f4e4d4c, 0x53525150, 0x57565554, 0x5b5a5958, 0x5f5e5d5c
	.long	0x63626160, 0x67666564, 0x6b6a6968, 0x6f6e6d6c, 0x73727170, 0x77767574, 0x7b7a7978, 0x7f7e7d7c
	.long	0x83828180, 0x87868584, 0x8b8a8988, 0x8f8e8d8c, 0x93929190, 0x97969594, 0x9b9a9998, 0x9f9e9d9c
	.long	0xa3a2a1a0, 0xa7a6a5a4, 0xabaaa9a8, 0xafaeadac, 0xb3b2b1b0, 0xb7b6b5b4, 0xbbbab9b8, 0xbfbebdbc
	.long	0xc3c2c1c0, 0xc7c6c5c4, 0xcbcac9c8, 0xcfcecdcc, 0xd3d2d1d0, 0xd7d6d5d4, 0xdbdad9d8, 0xdfdedddc
	.long	0xe3e2e1e0, 0xe7e6e5e4, 0xebeae9e8, 0xefeeedec, 0xf3f2f1f0, 0xf7f6f5f4, 0xfbfaf9f8, 0xfffefdfc
	.long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
	.long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
	.long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
	.long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
	.long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
	.long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
	.long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
	.long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
	.long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
	.long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
	.long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
	.long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff

CM:	.long	CMX

/****************************************/

	.balign 32
unaligned_return_thunk_armv5te:
        ldr    pc, [sp], #4
