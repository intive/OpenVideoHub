@.arch armv5te

#ifdef __ARM_EABI__
#define DWORD_ALIGNED_STACK 1
#endif

        .macro idct_stackalign_armv5te
#ifndef DWORD_ALIGNED_STACK
        tst    sp, #4
        strne  lr, [sp, #-4]!
        adrne  lr, unaligned_return_thunk_armv5te
#endif
        .endm

	.balign	16
xC1S7:	.long	64277
xC2S6:	.long	60547
xC3S5:	.long	54491
xC4S4:	.long	46341
xC5S3:	.long	36410
xC6S2:	.long	25080
xC7S1:	.long	12785
CM:	.word	ff_cropTbl+1024

/****************************************/

	.balign 32
	.global ff_vp3_idct_c
	.type ff_vp3_idct_c, %function
	.func ff_vp3_idct_c
ff_vp3_idct_c:

	idct_stackalign_armv5te

	bx	lr

	.endfunc

/****************************************/

	.balign	32
	.global	ff_vp3_idct_put_c
	.type	ff_vp3_idct_put_c, %function
	.func	ff_vp3_idct_put_c
ff_vp3_idct_put_c:

	idct_stackalign_armv5te

	stmfd	sp!, {v1, v2, v3, v4, v5, v6, v7, v8, lr}
	stmfd	sp!, {a1, a2}

	mov	ip, #8
	mov	lr, a3

p_rows:
	ldmia	lr!, {a1-a4}
	orr	v1, r0, r1
	orr	v1, v1, r2
	orrs	v1, v1, r3
	beq	p_rows_end

					@	a1	a2	a3	a4	v1	v2	v3	v4	v5	v6	v7	v8
					@	I01	I23	I45	I67
	ldr	v5, xC1S7		@									C1S7
	ldr	v6, xC7S1		@										C7S1
	rsb	v7, v5, #0		@											-C1S7
	smulwt	v1, v5, a1
	smlawt	v1, v6, a4, v1		@					A
	smulwt	v2, v6, a1
	smlawt	v2, v7, a4, v2		@						B
	ldr	v6, xC5S3		@										C5S3
	ldr	v5, xC3S5		@									C3S5
	rsb	v7, v6, #0		@											-C5S3
	smulwt	v3, v5, a2
	smlawt	v3, v6, a3, v3		@							C
	smulwt	v4, v5, a3
	smlawt	v4, v7, a2, v4		@								D

	sub	v5, v1, v3		@									A-C
	ldr	v7, xC4S4		@											C4S4
	sub	v6, v2, v4		@										B-D
	rsb	v8, v7, #0		@												-C4S4
	smulwb	v5, v7, v5		@									Ad
	smulwb	v6, v7, v6		@										Bd

	add	v1, v1, v3		@					A+C=Cd
	add	v2, v2, v4		@						B+D=Dd

	smulwb	v3, v7, a1
	smlawb	v4, v8, a3, v3		@								F
	smlawb	v3, v7, a3, v3		@							E

	ldr	a1, xC2S6		@	C2S6
	ldr	a3, xC6S2		@			C6S2
	smulwb	v7, a1, a2
	smlawb	v7, a3, a4, v7		@											G
	rsb	a1, a1, #0		@	-C2S6
	smulwb	v8, a3, a2
	smlawb	v8, a1, a4, v8		@												H

	sub	a1, v3, v7		@	E-G=Ed
	add	a2, v3, v7		@		E+G=Gd
	add	a3, v4, v5		@			F+Ad=Add
	sub	a4, v6, v8		@				Bd-H=Bdd
	sub	v7, v4, v5		@											F-Ad=Fd
	add	v8, v6, v8		@												Bd+H=Hd

	add	v3, a2, v1		@							Gd+Cd
	add	v4, a3, v8		@								Add+Hd
	sub	v5, a3, v8		@									Add-Hd
	add	v6, a1, v2		@										Ed+Dd
	strh	v3, [lr, #-16]
	strh	v4, [lr, #-14]
	strh	v5, [lr, #-12]
	strh	v6, [lr, #-10]

	sub	v3, a1, v2		@							Ed-Dd
	add	v4, v7, a4		@								Fd+Bdd
	sub	v5, v7, a4		@									Fd-Bdd
	sub	v6, a2, v1		@										Gd-Cd
	strh	v3, [lr, #-8]
	strh	v4, [lr, #-6]
	strh	v5, [lr, #-4]
	strh	v6, [lr, #-2]

p_rows_end:
	subs	ip, ip, #1
	bne	p_rows

	sub	lr, lr, #128

	mov	ip, #8

p_cols:
	ldrh	a1, [lr, #16]
	ldrh	a2, [lr, #48]
	ldrh	a3, [lr, #80]
	ldrh	a4, [lr, #112]
	orr	v5, a1, a2
	orr	v5, v5, a3
	orrs	v5, v5, a4
	bne	p_cols_commit

	ldrh	a2, [lr, #32]
	ldrh	a3, [lr, #64]
	ldrh	a4, [lr, #96]
	orr	v5, v5, a2
	orr	v5, v5, a3
	orrs	v5, v5, a4
	beq	p_cols_null

p_cols_even:

	ldr	v7, xC4S4		@											C4S4
	ldrh	a1, [lr]
	ldr	v3, =2056
	rsb	v8, v7, #0		@												-C4S4

	smlawb	v3, v7, a1, v3
	smlawb	v4, v8, a3, v3		@								F
	smlawb	v3, v7, a3, v3		@							E

	ldr	a1, xC2S6		@	C2S6
	ldr	a3, xC6S2		@			C6S2
	smulwb	v7, a1, a2
	smlawb	v7, a3, a4, v7		@											G
	rsb	a1, a1, #0		@	-C2S6
	smulwb	v8, a3, a2
	smlawb	v8, a1, a4, v8		@												H

	ldr	v5, [sp]		@									d
	ldr	v6, [sp, #4]		@										stride

	add	v1, v3, v7		@					E+G
	sub	v2, v3, v7		@						E-G
	add	v3, v4, v8		@							F+H
	sub	v4, v4, v8		@								F-H

	ldr	v7, CM			@											CM
	mov	v8, v5
	
	ldrb	v1, [v7, v1, asr #4]
	ldrb	v3, [v7, v3, asr #4]
	ldrb	v4, [v7, v4, asr #4]
	ldrb	v2, [v7, v2, asr #4]
	strb	v1, [v5], v6
	strb	v3, [v5], v6
	strb	v4, [v5], v6
	strb	v2, [v5], v6
	strb	v2, [v5], v6
	strb	v4, [v5], v6
	strb	v3, [v5], v6
	strb	v1, [v5], v6

	add	lr, lr, #2
	subs	ip, ip, #1
	beq	p_cols_end

	add	v8, v8, #1
	str	v8, [sp]
	b	p_cols

p_cols_commit:

					@	a1	a2	a3	a4	v1	v2	v3	v4	v5	v6	v7	v8
					@	I1	I3	I5	I7
	ldr	v5, xC1S7		@									C1S7
	ldr	v6, xC7S1		@										C7S1
	rsb	v7, v5, #0		@											-C1S7
	smulwb	v1, v5, a1
	smlawb	v1, v6, a4, v1		@					A
	smulwb	v2, v6, a1
	smlawb	v2, v7, a4, v2		@						B
	ldr	v6, xC5S3		@										C5S3
	ldr	v5, xC3S5		@									C3S5
	rsb	v7, v6, #0		@											-C5S3
	smulwb	v3, v5, a2
	smlawb	v3, v6, a3, v3		@							C
	smulwb	v4, v5, a3
	smlawb	v4, v7, a2, v4		@								D

	sub	v5, v1, v3		@									A-C
	ldr	v7, xC4S4		@											C4S4
	sub	v6, v2, v4		@										B-D
	rsb	v8, v7, #0		@												-C4S4
	smulwb	v5, v7, v5		@									Ad
	smulwb	v6, v7, v6		@										Bd

	add	v1, v1, v3		@					A+C=Cd
	add	v2, v2, v4		@						B+D=Dd

	ldr	v3, =2056

	ldrh	a1, [lr]		@	I0
	ldrh	a2, [lr, #32]		@		I2
	ldrh	a3, [lr, #64]		@			I4
	ldrh	a4, [lr, #96]		@				I6

	smlawb	v3, v7, a1, v3
	smlawb	v4, v8, a3, v3		@								F
	smlawb	v3, v7, a3, v3		@							E

	ldr	a1, xC2S6		@	C2S6
	ldr	a3, xC6S2		@			C6S2
	smulwb	v7, a1, a2
	smlawb	v7, a3, a4, v7		@											G
	rsb	a1, a1, #0		@	-C2S6
	smulwb	v8, a3, a2
	smlawb	v8, a1, a4, v8		@												H

	sub	a1, v3, v7		@	E-G=Ed
	add	a2, v3, v7		@		E+G=Gd
	add	a3, v4, v5		@			F+Ad=Add
	sub	a4, v6, v8		@				Bd-H=Bdd
	sub	v7, v4, v5		@											F-Ad=Fd
	add	v8, v6, v8		@												Bd+H=Hd

	add	v3, a2, v1		@							Gd+Cd
	sub	v4, a2, v1		@								Gd-Cd
	add	v5, a3, v8		@									Add+Hd
	sub	v6, a3, v8		@										Add-Hd
	add	a2, a1, v2		@		Ed+Dd
	sub	a3, a1, v2		@			Ed-Dd
	add	a1, v7, a4		@	Fd+Bdd
	sub	v1, v7, a4		@					Fd-Bdd

	ldr	a4, CM			@				CM
	ldr	v2, [sp]		@						d=dest
	ldr	v7, [sp, #4]		@											stride
	mov	v8, v2			@												dest

	ldrb	v3, [a4, v3, asr #4]
	ldrb	v5, [a4, v5, asr #4]
	ldrb	v6, [a4, v6, asr #4]
	ldrb	a2, [a4, a2, asr #4]
	ldrb	a3, [a4, a3, asr #4]
	ldrb	a1, [a4, a1, asr #4]
	ldrb	v1, [a4, v1, asr #4]
	ldrb	v4, [a4, v4, asr #4]

	strb	v3, [v2], v7
	strb	v5, [v2], v7
	strb	v6, [v2], v7
	strb	a2, [v2], v7
	strb	a3, [v2], v7
	strb	a1, [v2], v7
	strb	v1, [v2], v7
	strb	v4, [v2]

	add	lr, lr, #2
	subs	ip, ip, #1
	beq	p_cols_end

	add	v8, v8, #1
	str	v8, [sp]
	b	p_cols

p_cols_null:

	ldrh	a3, [lr]
	ldr	v2, xC4S4
	mov	v1, #8
	smlawb	v1, v2, a3, v1
	ldr	a1, [sp]		@ dest
	ldr	a2, [sp, #4]		@ stride
	mov	v8, a1
	ldr	v2, CM
	mov	v1, v1, asr #4
	add	v1, v1, #128
	ldrb	v1, [v2, v1]

	add	v8, v8, #1
	add	lr, lr, #2

	strb	v1, [a1], a2
	strb	v1, [a1], a2
	strb	v1, [a1], a2
	strb	v1, [a1], a2
	strb	v1, [a1], a2
	strb	v1, [a1], a2
	strb	v1, [a1], a2
	strb	v1, [a1]

	str	v8, [sp]

	subs	ip, ip, #1
	bne	p_cols

p_cols_end:

	mov	a1, #0
	mov	a2, #0
	mov	a3, #0
	mov	a4, #0
	sub	lr, lr, #16
	stmia	lr!, {a1-a4}
	stmia	lr!, {a1-a4}
	stmia	lr!, {a1-a4}
	stmia	lr!, {a1-a4}
	stmia	lr!, {a1-a4}
	stmia	lr!, {a1-a4}
	stmia	lr!, {a1-a4}
	stmia	lr!, {a1-a4}

	add	sp, #8
	ldmfd	sp!, {v1, v2, v3, v4, v5, v6, v7, v8, pc}

	.endfunc

/****************************************/

	.balign	32
	.global	ff_vp3_idct_add_c
	.type	ff_vp3_idct_add_c, %function
	.func	ff_vp3_idct_add_c
ff_vp3_idct_add_c:

	idct_stackalign_armv5te

	stmfd	sp!, {v1, v2, v3, v4, v5, v6, v7, v8, lr}
	stmfd	sp!, {a1, a2}

	mov	ip, #8
	mov	lr, a3

a_rows:
	ldmia	lr!, {a1-a4}
	orr	v1, r0, r1
	orr	v1, v1, r2
	orrs	v1, v1, r3
	beq	a_rows_end

					@	a1	a2	a3	a4	v1	v2	v3	v4	v5	v6	v7	v8
					@	I01	I23	I45	I67
	ldr	v5, xC1S7		@									C1S7
	ldr	v6, xC7S1		@										C7S1
	rsb	v7, v5, #0		@											-C1S7
	smulwt	v1, v5, a1
	smlawt	v1, v6, a4, v1		@					A
	smulwt	v2, v6, a1
	smlawt	v2, v7, a4, v2		@						B
	ldr	v6, xC5S3		@										C5S3
	ldr	v5, xC3S5		@									C3S5
	rsb	v7, v6, #0		@											-C5S3
	smulwt	v3, v5, a2
	smlawt	v3, v6, a3, v3		@							C
	smulwt	v4, v5, a3
	smlawt	v4, v7, a2, v4		@								D

	sub	v5, v1, v3		@									A-C
	ldr	v7, xC4S4		@											C4S4
	sub	v6, v2, v4		@										B-D
	rsb	v8, v7, #0		@												-C4S4
	smulwb	v5, v7, v5		@									Ad
	smulwb	v6, v7, v6		@										Bd

	add	v1, v1, v3		@					A+C=Cd
	add	v2, v2, v4		@						B+D=Dd

	smulwb	v3, v7, a1
	smlawb	v4, v8, a3, v3		@								F
	smlawb	v3, v7, a3, v3		@							E

	ldr	a1, xC2S6		@	C2S6
	ldr	a3, xC6S2		@			C6S2
	smulwb	v7, a1, a2
	smlawb	v7, a3, a4, v7		@											G
	rsb	a1, a1, #0		@	-C2S6
	smulwb	v8, a3, a2
	smlawb	v8, a1, a4, v8		@												H

	sub	a1, v3, v7		@	E-G=Ed
	add	a2, v3, v7		@		E+G=Gd
	add	a3, v4, v5		@			F+Ad=Add
	sub	a4, v6, v8		@				Bd-H=Bdd
	sub	v7, v4, v5		@											F-Ad=Fd
	add	v8, v6, v8		@												Bd+H=Hd

	add	v3, a2, v1		@							Gd+Cd
	add	v4, a3, v8		@								Add+Hd
	sub	v5, a3, v8		@									Add-Hd
	add	v6, a1, v2		@										Ed+Dd
	strh	v3, [lr, #-16]
	strh	v4, [lr, #-14]
	strh	v5, [lr, #-12]
	strh	v6, [lr, #-10]

	sub	v3, a1, v2		@							Ed-Dd
	add	v4, v7, a4		@								Fd+Bdd
	sub	v5, v7, a4		@									Fd-Bdd
	sub	v6, a2, v1		@										Gd-Cd
	strh	v3, [lr, #-8]
	strh	v4, [lr, #-6]
	strh	v5, [lr, #-4]
	strh	v6, [lr, #-2]

a_rows_end:
	subs	ip, ip, #1
	bne	a_rows

	sub	lr, lr, #128

	mov	ip, #8

a_cols:
	ldrh	a1, [lr, #16]
	ldrh	a2, [lr, #48]
	ldrh	a3, [lr, #80]
	ldrh	a4, [lr, #112]
	orr	v5, a1, a2
	orr	v5, v5, a3
	orrs	v5, v5, a4
	bne	a_cols_commit

	ldrh	a2, [lr, #32]
	ldrh	a3, [lr, #64]
	ldrh	a4, [lr, #96]
	orr	v5, v5, a2
	orr	v5, v5, a3
	orrs	v5, v5, a4
	beq	a_cols_null

a_cols_even:

	ldr	v7, xC4S4		@											C4S4
	ldrh	a1, [lr]
	mov	v3, #8
	rsb	v8, v7, #0		@												-C4S4

	smlawb	v3, v7, a1, v3
	smlawb	v4, v8, a3, v3		@								F
	smlawb	v3, v7, a3, v3		@							E

	ldr	a1, xC2S6		@	C2S6
	ldr	a3, xC6S2		@			C6S2
	smulwb	v7, a1, a2
	smlawb	v7, a3, a4, v7		@											G
	rsb	a1, a1, #0		@	-C2S6
	smulwb	v8, a3, a2
	smlawb	v8, a1, a4, v8		@												H

	ldr	v5, [sp]		@									d
	ldr	v6, [sp, #4]		@										stride

	add	v1, v3, v7		@					E+G
	sub	v2, v3, v7		@						E-G
	add	v3, v4, v8		@							F+H
	sub	v4, v4, v8		@								F-H

	ldr	v7, CM			@											CM
	mov	v8, v5

	ldrb	a1, [v5], v6		@	d[0]
	ldrb	a2, [v5], v6		@		d[1]
	ldrb	a3, [v5], v6		@			d[2]
	ldrb	a4, [v5], v6		@				d[3]
	add	a1, a1, v1, asr #4
	add	a2, a2, v3, asr #4
	add	a3, a3, v4, asr #4
	add	a4, a4, v2, asr #4
	ldrb	a1, [v7, a1]
	ldrb	a2, [v7, a2]
	ldrb	a3, [v7, a3]
	ldrb	a4, [v7, a4]
	strb	a1, [v8], v6
	strb	a2, [v8], v6
	strb	a3, [v8], v6
	strb	a4, [v8], v6

	ldrb	a1, [v5], v6		@	d[4]
	ldrb	a2, [v5], v6		@		d[5]
	ldrb	a3, [v5], v6		@			d[6]
	ldrb	a4, [v5], v6		@				d[7]
	add	a1, a1, v2, asr #4
	add	a2, a2, v4, asr #4
	add	a3, a3, v3, asr #4
	add	a4, a4, v1, asr #4
	ldrb	a1, [v7, a1]
	ldrb	a2, [v7, a2]
	ldrb	a3, [v7, a3]
	ldrb	a4, [v7, a4]
	strb	a1, [v8], v6
	strb	a2, [v8], v6
	strb	a3, [v8], v6
	strb	a4, [v8], v6

	add	lr, lr, #2
	subs	ip, ip, #1
	beq	a_cols_end

	sub	v5, v5, v6, lsl #3
	add	v5, v5, #1
	str	v5, [sp]
	b	a_cols

a_cols_commit:

					@	a1	a2	a3	a4	v1	v2	v3	v4	v5	v6	v7	v8
					@	I1	I3	I5	I7
	ldr	v5, xC1S7		@									C1S7
	ldr	v6, xC7S1		@										C7S1
	rsb	v7, v5, #0		@											-C1S7
	smulwb	v1, v5, a1
	smlawb	v1, v6, a4, v1		@					A
	smulwb	v2, v6, a1
	smlawb	v2, v7, a4, v2		@						B
	ldr	v6, xC5S3		@										C5S3
	ldr	v5, xC3S5		@									C3S5
	rsb	v7, v6, #0		@											-C5S3
	smulwb	v3, v5, a2
	smlawb	v3, v6, a3, v3		@							C
	smulwb	v4, v5, a3
	smlawb	v4, v7, a2, v4		@								D

	sub	v5, v1, v3		@									A-C
	ldr	v7, xC4S4		@											C4S4
	sub	v6, v2, v4		@										B-D
	rsb	v8, v7, #0		@												-C4S4
	smulwb	v5, v7, v5		@									Ad
	smulwb	v6, v7, v6		@										Bd

	add	v1, v1, v3		@					A+C=Cd
	add	v2, v2, v4		@						B+D=Dd

	mov	v3, #8

	ldrh	a1, [lr]		@	I0
	ldrh	a2, [lr, #32]		@		I2
	ldrh	a3, [lr, #64]		@			I4
	ldrh	a4, [lr, #96]		@				I6

	smlawb	v3, v7, a1, v3
	smlawb	v4, v8, a3, v3		@								F
	smlawb	v3, v7, a3, v3		@							E

	ldr	a1, xC2S6		@	C2S6
	ldr	a3, xC6S2		@			C6S2
	smulwb	v7, a1, a2
	smlawb	v7, a3, a4, v7		@											G
	rsb	a1, a1, #0		@	-C2S6
	smulwb	v8, a3, a2
	smlawb	v8, a1, a4, v8		@												H

	sub	a1, v3, v7		@	E-G=Ed
	add	a2, v3, v7		@		E+G=Gd
	add	a3, v4, v5		@			F+Ad=Add
	sub	a4, v6, v8		@				Bd-H=Bdd
	sub	v7, v4, v5		@											F-Ad=Fd
	add	v8, v6, v8		@												Bd+H=Hd

	ldr	v5, [sp]		@									d
	ldr	v6, [sp, #4]		@										stride
	add	v3, a2, v1		@							Gd+Cd
	ldrb	v4, [v5], v6		@								d[0]
	sub	a2, a2, v1		@		Gd-Cd
	ldrb	v1, [v5], v6		@					d[1]
	add	v3, v4, v3, asr #4	@							sum
	add	v4, a3, v8		@								Add+Hd
	sub	a3, a3, v8		@			Add-Hd
	ldrb	v8, [v5], v6		@												d[2]
	add	v4, v1, v4, asr #4	@								sum
	ldrb	v1, [v5], v6		@					d[3]
	add	a3, v8, a3, asr #4	@			sum
	add	v8, a1, v2		@												Ed+Dd
	sub	a1, a1, v2		@	Ed-Dd
	ldrb	v2, [v5], v6		@						d[4]
	add	v8, v1, v8, asr #4	@												sum
	add	v1, v7, a4		@					Fd+Bdd
	add	a1, v2, a1, asr #4	@	sum
	ldrb	v2, [v5], v6		@						d[5]
	sub	v7, v7, a4		@											Fd-Bdd
	ldrb	a4, [v5], v6		@				d[6]
	add	v1, v2, v1, asr #4	@					sum
	ldrb	v2, [v5], v6		@						d[7]
	add	v7, a4, v7, asr #4	@											sum
	ldr	a4, CM			@				CM
	add	a2, v2, a2, asr #4	@		sum
					@	Ed-Dd	Gd-Cd	Add-Hd	CM	Fd+Bdd		Gd+Cd	Add+Hd		stride	Fd-Bdd	Ed+Dd
	ldr	v2, [sp]		@						d=dest

	ldrb	v3, [a4, v3]
	ldrb	v4, [a4, v4]
	ldrb	a3, [a4, a3]
	ldrb	v8, [a4, v8]
	ldrb	a1, [a4, a1]
	ldrb	v1, [a4, v1]
	ldrb	v7, [a4, v7]
	ldrb	a2, [a4, a2]

	mov	v5, v2			@									dest

	strb	v3, [v2], v6
	strb	v4, [v2], v6
	strb	a3, [v2], v6
	strb	v8, [v2], v6
	strb	a1, [v2], v6
	strb	v1, [v2], v6
	strb	v7, [v2], v6
	strb	a2, [v2], v6

	add	lr, lr, #2
	subs	ip, ip, #1
	beq	a_cols_end

	add	v5, v5, #1
	str	v5, [sp]
	b	a_cols

a_cols_null:

	ldrh	a3, [lr]		@			I0
	ldr	v2, xC4S4		@						C4S4
	mov	v1, #8
	smlawb	v1, v2, a3, v1		@					V
	ldr	v2, [sp]		@						dL
	ldr	a2, [sp, #4]		@ 		stride
	ldr	a1, CM			@	CM
	mov	a3, v2			@			dest
	mov	v3, v2			@							dS
	mov	v1, v1, asr #4

	ldrb	v5, [v2], a2		@									d[0]
	ldrb	v6, [v2], a2		@										d[1]
	ldrb	v7, [v2], a2		@											d[2]
	ldrb	v8, [v2], a2		@												d[3]
	add	v5, v5, v1		@									d[0]+v
	add	v6, v6, v1		@										d[1]+v
	add	v7, v7, v1		@											d[2]+v
	add	v8, v8, v1		@												d[3]+v
	ldrb	v5, [a1, v5]
	ldrb	v6, [a1, v6]
	ldrb	v7, [a1, v7]
	ldrb	v8, [a1, v8]
	strb	v5, [v3], a2
	strb	v6, [v3], a2
	strb	v7, [v3], a2
	strb	v8, [v3], a2

	ldrb	v5, [v2], a2		@									d[4]
	ldrb	v6, [v2], a2		@										d[5]
	ldrb	v7, [v2], a2		@											d[6]
	ldrb	v8, [v2], a2		@												d[7]
	add	v5, v5, v1		@									d[4]+v
	add	v6, v6, v1		@										d[5]+v
	add	v7, v7, v1		@											d[6]+v
	add	v8, v8, v1		@												d[7]+v
	ldrb	v5, [a1, v5]
	ldrb	v6, [a1, v6]
	ldrb	v7, [a1, v7]
	ldrb	v8, [a1, v8]
	strb	v5, [v3], a2
	strb	v6, [v3], a2
	strb	v7, [v3], a2
	strb	v8, [v3], a2

	add	a3, a3, #1
	add	lr, lr, #2
	str	a3, [sp]

	subs	ip, ip, #1
	bne	a_cols

a_cols_end:

	mov	a1, #0
	mov	a2, #0
	mov	a3, #0
	mov	a4, #0
	sub	lr, lr, #16
	stmia	lr!, {a1-a4}
	stmia	lr!, {a1-a4}
	stmia	lr!, {a1-a4}
	stmia	lr!, {a1-a4}
	stmia	lr!, {a1-a4}
	stmia	lr!, {a1-a4}
	stmia	lr!, {a1-a4}
	stmia	lr!, {a1-a4}

	add	sp, #8
	ldmfd	sp!, {v1, v2, v3, v4, v5, v6, v7, v8, pc}

	.endfunc

/****************************************/

	.balign 32
unaligned_return_thunk_armv5te:
        ldr    pc, [sp], #4
