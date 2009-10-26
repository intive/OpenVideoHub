
        .align 8
        .global clear_block_arm
clear_block_arm:
        mov r1, #0
        mov r2, #0
        mov r3, #0
        mov r12, #0

        stmia r0!, { r1, r2, r3, r12 }
        stmia r0!, { r1, r2, r3, r12 }
        stmia r0!, { r1, r2, r3, r12 }
        stmia r0!, { r1, r2, r3, r12 }
        stmia r0!, { r1, r2, r3, r12 }
        stmia r0!, { r1, r2, r3, r12 }
        stmia r0!, { r1, r2, r3, r12 }
        stmia r0!, { r1, r2, r3, r12 }
        bx lr
