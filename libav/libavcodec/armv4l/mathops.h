/*
 * simple math operations
 * Copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at> et al
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef FFMPEG_ARMV4L_MATHOPS_H
#define FFMPEG_ARMV4L_MATHOPS_H

#if defined( __GNUC__ )

#ifndef __CC_ARM

#ifdef FRAC_BITS
#   define MULL(a, b) \
        ({  int lo, hi;\
         asm("smull %0, %1, %2, %3     \n\t"\
             "mov   %0, %0,     lsr %4\n\t"\
             "add   %1, %0, %1, lsl %5\n\t"\
             : "=&r"(lo), "=&r"(hi)\
             : "r"(b), "r"(a), "i"(FRAC_BITS), "i"(32-FRAC_BITS));\
         hi; })
#endif

#define MULH(a, b) \
    ({ int lo, hi;\
     asm ("smull %0, %1, %2, %3" : "=&r"(lo), "=&r"(hi) : "r"(b), "r"(a));\
     hi; })

#else

/*
#ifdef FRAC_BITS
__inline int arm_mull( register int a, register int b )
	{
		register int lo, hi;
		__asm
		{
			SMULL lo, hi, b, a
			MOV   lo, lo, lsr #FRAC_BITS
			ADD   hi, lo, hi, lsr #32-FRAC_BITS
		}
		return hi;
	}
#define MULL(a, b) arm_mull( a, b )
#endif
*/

__inline int arm_mulh(register int a, register int b)
	{
		register int lo, hi;
		__asm
		{
			SMULL lo, hi, b, a
		}
		return hi;
	}

#define MULH( a, b ) arm_mulh( a, b)

#endif

#endif

#if 0 //defined(HAVE_ARMV5TE)

/* signed 16x16 -> 32 multiply add accumulate */
#   define MAC16(rt, ra, rb) \
        asm ("smlabb %0, %2, %3, %0" : "=r" (rt) : "0" (rt), "r" (ra), "r" (rb));
/* signed 16x16 -> 32 multiply */
#   define MUL16(ra, rb)                                                \
        ({ int __rt;                                                    \
         asm ("smulbb %0, %1, %2" : "=r" (__rt) : "r" (ra), "r" (rb));  \
         __rt; })

#endif

#endif /* FFMPEG_ARMV4L_MATHOPS_H */
