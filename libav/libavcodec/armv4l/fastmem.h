/* fastmem-arm9-inline.h - a small set of memset/memcpy function
   variants implemented in gcc inline assembler for ARM926EJ-S and
   probably all others cores from ARM9 family.

   Copyright (C) 2006  Siarhei Siamashka <ssvb@users.sourceforge.net>

   This software is provided 'as-is', without any express or implied
   warranty.  In no event will the authors be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
      claim that you wrote the original software. If you use this software
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.
   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original software.
   3. This notice may not be removed or altered from any source distribution.
*/
/*
   Please ignore gcc 'register range not in ascending order' warning, register
   order does not affect correctness as order of registers is the same anyway
   in LDM/STM instruction pairs
 */

#ifndef __FASTMEM_ARM9_H__
#define __FASTMEM_ARM9_H__

#if defined(__GNUC__) && defined(__arm__)

#include <stdint.h>

/*
* Function similar to standard memset function, but works with 32-bit data.
* @param _dst   - pointer to data buffer (32-bit aligned)
* @param _c     - filler 32-bit value
* @param _count - number of filler 32-bit values to be saved into buffer
*/
#define memset32(_dst, _c, _count) \
({ uint32_t *dst = (_dst); uint32_t c = (_c); int count = (_count); uint32_t dummy0, dummy1, dummy2; \
    __asm__ __volatile__ ( \
        "      cmp   %[count], #4\n" \
        "      blt   2f\n" \
        /* Alignment is known to be at least 32-bit */ \
        "      mov   %[dummy0], %[c]\n" \
        "      tst   %[dst], #4\n" \
        "      strne %[c], [%[dst]], #4\n" \
        "      subne %[count], %[count], #1\n" \
        "      tst   %[dst], #8\n" \
        "      stmneia %[dst]!, {%[dummy0], %[c]}\n" \
        "      subne %[count], %[count], #2\n" \
        /* Now we are 128-bit aligned */ \
        "      mov   %[dummy1], %[c]\n" \
        "      mov   %[dummy2], %[c]\n" \
        "1:\n" /* Copy 4 32-bit values per loop iteration */ \
        "      subs  %[count], %[count], #4\n" \
        "      stmgeia %[dst]!, {%[dummy0], %[dummy1], %[dummy2], %[c]}\n" \
        "      bge   1b\n" \
        "      add   %[count], %[count], #4\n" \
        "2:\n" /* Copy up to 3 remaining 32-bit values */ \
        "      subs  %[count], %[count], #1\n" \
        "      strge %[c], [%[dst]], #4\n" \
        "      subs  %[count], %[count], #1\n" \
        "      strge %[c], [%[dst]], #4\n" \
        "      subs  %[count], %[count], #1\n" \
        "      strge %[c], [%[dst]], #4\n" \
        "\n" \
        : [dst] "+&r" (dst), [count] "+&r" (count), [dummy0] "=&r" (dummy0), [dummy1] "=&r" (dummy1), [dummy2] "=&r" (dummy2), [c] "+&r" (c) \
        : \
        : "cc", "memory" \
    ); _dst; \
})

/*
* Faster replacement for standard memset function.
* @param _dst   - pointer to data buffer
* @param _c     - filler 8-bit value
* @param _count - number of filler 8-bit values to be saved into buffer
*/
#define memset8(_dst, _c, _count) \
({ uint8_t *dst = (_dst); uint8_t c = (_c); int count = (_count); uint32_t dummy0, dummy1, dummy2; \
__asm__ __volatile__ ( \
        "      cmp   %[count], #4\n" \
        "      blt   3f\n" \
        /* Alignment is unknown */ \
        "      tst   %[dst], #1\n" \
        "      strneb %[c], [%[dst]], #1\n" \
        "      subne  %[count], %[count], #1\n" \
        /* Now we are 16-bit aligned (need to upgrade 'c' to 16-bit) */ \
        "      orr    %[c], %[c], %[c], asl #8\n" \
        "      tst   %[dst], #2\n" \
        "      strneh %[c], [%[dst]], #2\n" \
        "      subne  %[count], %[count], #2\n" \
        /* Now we are 32-bit aligned (need to upgrade 'c' to 32-bit) */ \
        "      orr    %[c], %[c], %[c], asl #16\n" \
        "      mov   %[dummy0], %[c]\n" \
        "      cmp   %[count], #16\n" \
        "      blt   2f\n" \
        "      tst   %[dst], #4\n" \
        "      strne %[c], [%[dst]], #4\n" \
        "      subne %[count], %[count], #4\n" \
        "      tst   %[dst], #8\n" \
        "      stmneia %[dst]!, {%[dummy0], %[c]}\n" \
        "      subne %[count], %[count], #8\n" \
        /* Now we are 128-bit aligned */ \
        "      mov   %[dummy1], %[c]\n" \
        "      mov   %[dummy2], %[c]\n" \
        "1:\n" /* Copy 4 32-bit values per loop iteration */ \
        "      subs  %[count], %[count], #16\n" \
        "      stmgeia %[dst]!, {%[dummy0], %[dummy1], %[dummy2], %[c]}\n" \
        "      bge   1b\n" \
        "      add   %[count], %[count], #16\n" \
        "2:\n" /* Copy up to 3 remaining 32-bit values */ \
        "      tst   %[count], #8\n" \
        "      stmneia %[dst]!, {%[dummy0], %[c]}\n" \
        "      tst   %[count], #4\n" \
        "      strne %[c], [%[dst]], #4\n" \
        "      and  %[count], %[count], #3\n" \
        "3:\n" /* Copy up to 3 remaining bytes */ \
        "      subs  %[count], %[count], #1\n" \
        "      strgeb %[c], [%[dst]], #1\n" \
        "      subs  %[count], %[count], #1\n" \
        "      strgeb %[c], [%[dst]], #1\n" \
        "      subs  %[count], %[count], #1\n" \
        "      strgeb %[c], [%[dst]], #1\n" \
        "\n" \
        : [dst] "+&r" (dst), [count] "+&r" (count), [dummy0] "=&r" (dummy0), [dummy1] "=&r" (dummy1), [dummy2] "=&r" (dummy2), [c] "+&r" (c) \
        : \
        : "cc", "memory" \
    ); _dst; \
})

/*
* Function similar to standard memcpy function, but works with 32-bit data.
* @param _dst   - pointer to destination buffer (32-bit aligned)
* @param _src   - pointer to source buffer
* @param _count - number of 32-bit values to be copied
*/
#define memcpy32(_dst, _src, _count) \
({ uint32_t *dst = (_dst); uint32_t *src = (_src); int count = (_count); \
    __asm__ __volatile__ ( \
        "      cmp   %[count], #4\n" \
        "      blt   2f\n" \
        /* Alignment is known to be at least 32-bit */ \
        "      tst   %[dst], #4\n" \
        "      ldrne r4, [%[src]], #4\n" \
        "      strne r4, [%[dst]], #4\n" \
        "      subne %[count], %[count], #1\n" \
        "      tst   %[dst], #8\n" \
        "      ldmneia %[src]!, {r4-r5}\n" \
        "      stmneia %[dst]!, {r4-r5}\n" \
        "      subne %[count], %[count], #2\n" \
        /* Now destination address is 128-bit aligned */ \
        "1:\n" /* Copy 4 32-bit values per loop iteration */ \
        "      subs  %[count], %[count], #4\n" \
        "      ldmgeia %[src]!, {r4-r7}\n" \
        "      stmgeia %[dst]!, {r4-r7}\n" \
        "      bge   1b\n" \
        "      add   %[count], %[count], #4\n" \
        "2:\n" /* Copy up to 3 remaining 32-bit values */ \
        "      tst   %[count], #2\n" \
        "      ldmneia %[src]!, {r4-r5}\n" \
        "      stmneia %[dst]!, {r4-r5}\n" \
        "      tst   %[count], #1\n" \
        "      ldrne r4, [%[src]], #4\n" \
        "      strne r4, [%[dst]], #4\n" \
        "\n" \
        : [dst] "+&r" (dst),  [src] "+&r" (src), [count] "+&r" (count) \
        : \
        : "r4", "r5", "r6", "r7", "cc", "memory" \
    ); _dst; \
})

/*
* Function similar to standard memset function, but works with 16-bit data.
* @param _dst   - pointer to data buffer (16-bit aligned)
* @param _c     - filler 16-bit value
* @param _count - number of filler 16-bit values to be saved into buffer
*/
#define memset16(_dst, _c, _count) \
({ uint16_t *dst = (_dst); uint16_t c = (_c); int count = (_count); uint32_t dummy0, dummy1, dummy2; \
    __asm__ __volatile__ ( \
        "      cmp   %[count], #2\n" \
        "      blt   3f\n" \
        /* Alignment is known to be at least 16-bit */ \
        "      tst   %[dst], #2\n" \
        "      strneh %[c], [%[dst]], #2\n" \
        "      subne  %[count], %[count], #1\n" \
        /* Now we are 32-bit aligned (need to upgrade 'c' to 32-bit )*/ \
        "      orr    %[c], %[c], %[c], asl #16\n" \
        "      mov   %[dummy0], %[c]\n" \
        "      cmp   %[count], #8\n" \
        "      blt   2f\n" \
        "      tst   %[dst], #4\n" \
        "      strne %[c], [%[dst]], #4\n" \
        "      subne %[count], %[count], #2\n" \
        "      tst   %[dst], #8\n" \
        "      stmneia %[dst]!, {%[dummy0], %[c]}\n" \
        "      subne %[count], %[count], #4\n" \
        /* Now we are 128-bit aligned */ \
        "      mov   %[dummy1], %[c]\n" \
        "      mov   %[dummy2], %[c]\n" \
        "1:\n" /* Copy 4 32-bit values per loop iteration */ \
        "      subs  %[count], %[count], #8\n" \
        "      stmgeia %[dst]!, {%[dummy0], %[dummy1], %[dummy2], %[c]}\n" \
        "      bge   1b\n" \
        "      add   %[count], %[count], #8\n" \
        "2:\n" /* Copy up to 3 remaining 32-bit values */ \
        "      tst   %[count], #4\n" \
        "      stmneia %[dst]!, {%[dummy0], %[c]}\n" \
        "      tst   %[count], #2\n" \
        "      strne %[c], [%[dst]], #4\n" \
        "      and  %[count], %[count], #1\n" \
        "3:\n" /* Copy up to 1 remaining 16-bit value */ \
        "      subs  %[count], %[count], #1\n" \
        "      strgeh %[c], [%[dst]], #2\n" \
        "\n" \
        : [dst] "+&r" (dst), [count] "+&r" (count), [dummy0] "=&r" (dummy0), [dummy1] "=&r" (dummy1), [dummy2] "=&r" (dummy2), [c] "+&r" (c) \
        : \
        : "cc", "memory" \
    ); _dst;\
})

/*
* Function similar to standard memcpy function, but works with 16-bit data.
* @param _dst   - pointer to destination buffer (16-bit aligned)
* @param _src   - pointer to source buffer
* @param _count - number of 16-bit values to be copied
*/
#define memcpy16(_dst, _src, _count) \
({ uint16_t *dst = (_dst); uint16_t *src = (_src); int count = (_count); uint32_t dummy0; \
    __asm__ __volatile__ ( \
        "      cmp   %[count], #2\n" \
        "      blt   6f\n" \
        /* Alignment is known to be at least 16-bit */ \
        "      tst   %[dst], #2\n" \
        "      ldrneh r4, [%[src]], #2\n" \
        "      strneh r4, [%[dst]], #2\n" \
        "      subne  %[count], %[count], #1\n" \
        /* Now destination address is 32-bit aligned, still need to check whether */ \
        /* source is 32-bit aligned or not */ \
        "      tst   %[src], #2\n" \
        "      bne   3f\n" \
        /* Both destination and source are 32-bit aligned */ \
        "      cmp   %[count], #8\n" \
        "      blt   2f\n" \
        "      tst   %[dst], #4\n" \
        "      ldrne r4, [%[src]], #4\n" \
        "      strne r4, [%[dst]], #4\n" \
        "      subne %[count], %[count], #2\n" \
        "      tst   %[dst], #8\n" \
        "      ldmneia %[src]!, {r4-r5}\n" \
        "      stmneia %[dst]!, {r4-r5}\n" \
        "      subne %[count], %[count], #4\n" \
        /* Destination address is 128-bit aligned, source address is 32-bit aligned */ \
        "1:    subs  %[count], %[count], #8\n" \
        "      ldmgeia %[src]!, {r4-r7}\n" \
        "      stmgeia %[dst]!, {r4-r7}\n" \
        "      bge   1b\n" \
        "      add   %[count], %[count], #8\n" \
        /* Copy up to 3 remaining aligned 32-bit values */ \
        "2:    tst   %[count], #4\n" \
        "      ldmneia %[src]!, {r4-r5}\n" \
        "      stmneia %[dst]!, {r4-r5}\n" \
        "      tst   %[count], #2\n" \
        "      ldrne r4, [%[src]], #4\n" \
        "      strne r4, [%[dst]], #4\n" \
        "      and  %[count], %[count], #1\n" \
        "      b      6f\n" \
        /* Destination is 32-bit aligned, but source is only 16-bit aligned */ \
        "3:    cmp   %[count], #8\n" \
        "      blt   5f\n" \
        "      tst   %[dst], #4\n" \
        "      ldrneh r4, [%[src]], #2\n" \
        "      ldrneh r5, [%[src]], #2\n" \
        "      orrne  r4, r4, r5, asl #16\n" \
        "      strne r4, [%[dst]], #4\n" \
        "      subne %[count], %[count], #2\n" \
        "      tst   %[dst], #8\n" \
        "      ldrneh r4, [%[src]], #2\n" \
        "      ldrne  r5, [%[src]], #4\n" \
        "      ldrneh r6, [%[src]], #2\n" \
        "      orrne  r4, r4, r5, asl #16\n" \
        "      movne  r5, r5, lsr #16\n" \
        "      orrne  r5, r5, r6, asl #16\n" \
        "      stmneia %[dst]!, {r4-r5}\n" \
        "      subne %[count], %[count], #4\n" \
        /* Destination is 128-bit aligned, but source is only 16-bit aligned */ \
        "4:    subs  %[count], %[count], #8\n" \
        "      ldrgeh r4, [%[src]], #2\n" \
        "      ldmgeia %[src]!, {r5-r7}\n" \
        "      ldrgeh %[dummy0], [%[src]], #2\n" \
        "      orrge r4, r4, r5, asl #16\n" \
        "      movge r5, r5, lsr #16\n" \
        "      orrge r5, r5, r6, asl #16\n" \
        "      movge r6, r6, lsr #16\n" \
        "      orrge r6, r6, r7, asl #16\n" \
        "      movge r7, r7, lsr #16\n" \
        "      orrge r7, r7, %[dummy0], asl #16\n" \
        "      stmgeia %[dst]!, {r4-r7}\n" \
        "      bge    4b\n" \
        "      add    %[count], %[count], #8\n" \
        /* Copy up to 6 remaining 16-bit values (to 32-bit aligned destination) */ \
        "5:    subs   %[count], %[count], #2\n" \
        "      ldrgeh r4, [%[src]], #2\n" \
        "      ldrgeh r5, [%[src]], #2\n" \
        "      orrge  r4, r4, r5, asl #16\n" \
        "      strge  r4, [%[dst]], #4\n" \
        "      bge    5b\n" \
        "      add    %[count], %[count], #2\n" \
        /* Copy the last remaining 16-bit value if any */ \
        "6:    subs   %[count], %[count], #1\n" \
        "      ldrgeh r4, [%[src]], #2\n" \
        "      strgeh r4, [%[dst]], #2\n" \
        "\n" \
        : [dst] "+&r" (dst),  [src] "+&r" (src), [count] "+&r" (count), [dummy0] "=&r" (dummy0) \
        : \
        : "r4", "r5", "r6", "r7", "cc", "memory" \
    ); _dst; \
})
#endif

#endif
