
.global _test_timer_irq_flag_nop_0
.global _test_timer_irq_flag_nop_1
.global _test_timer_irq_flag_nop_2
.global _test_timer_irq_flag_nop_3
.global _test_timer_irq_flag_nop_4

.section .iwram
.arm

_test_timer_irq_flag_nop_0:
	@ r0 = TM0CNT_L
	ldr r0, =0x04000100
	
	@ stop timer0 and set reload value to 0xFFFF
	ldr r1, =#0x0000FFFF
	str r1, [r0]

	@ IE=0, IF=acknowledge all
	@ r2 points to IF after this block
	ldr r2, =0x04000200
	mvn r1, r1 @ 0x0000FFFF -> 0xFFFF0000
	str r1, [r2], #2

	@ start timer0 with IRQ enabled
	mov r1, #0xC0
	strb r1, [r0, #2]

	@ load IF register and return
	ldrh r0, [r2]
	bx lr

_test_timer_irq_flag_nop_1:
	@ r0 = TM0CNT_L
	ldr r0, =0x04000100
	
	@ stop timer0 and set reload value to 0xFFFF
	ldr r1, =#0x0000FFFF
	str r1, [r0]

	@ IE=0, IF=acknowledge all
	@ r2 points to IF after this block
	ldr r2, =0x04000200
	mvn r1, r1 @ 0x0000FFFF -> 0xFFFF0000
	str r1, [r2], #2

	@ start timer0 with IRQ enabled
	mov r1, #0xC0
	strb r1, [r0, #2]

	@ load IF register and return
	nop
	ldrh r0, [r2]
	bx lr

_test_timer_irq_flag_nop_2:
	@ r0 = TM0CNT_L
	ldr r0, =0x04000100
	
	@ stop timer0 and set reload value to 0xFFFF
	ldr r1, =#0x0000FFFF
	str r1, [r0]

	@ IE=0, IF=acknowledge all
	@ r2 points to IF after this block
	ldr r2, =0x04000200
	mvn r1, r1 @ 0x0000FFFF -> 0xFFFF0000
	str r1, [r2], #2

	@ start timer0 with IRQ enabled
	mov r1, #0xC0
	strb r1, [r0, #2]

	@ load IF register and return
	nop
	nop
	ldrh r0, [r2]
	bx lr

_test_timer_irq_flag_nop_3:
	@ r0 = TM0CNT_L
	ldr r0, =0x04000100
	
	@ stop timer0 and set reload value to 0xFFFF
	ldr r1, =#0x0000FFFF
	str r1, [r0]

	@ IE=0, IF=acknowledge all
	@ r2 points to IF after this block
	ldr r2, =0x04000200
	mvn r1, r1 @ 0x0000FFFF -> 0xFFFF0000
	str r1, [r2], #2

	@ start timer0 with IRQ enabled
	mov r1, #0xC0
	strb r1, [r0, #2]

	@ load IF register and return
	nop
	nop
	nop
	ldrh r0, [r2]
	bx lr

_test_timer_irq_flag_nop_4:
	@ r0 = TM0CNT_L
	ldr r0, =0x04000100
	
	@ stop timer0 and set reload value to 0xFFFF
	ldr r1, =#0x0000FFFF
	str r1, [r0]

	@ IE=0, IF=acknowledge all
	@ r2 points to IF after this block
	ldr r2, =0x04000200
	mvn r1, r1 @ 0x0000FFFF -> 0xFFFF0000
	str r1, [r2], #2

	@ start timer0 with IRQ enabled
	mov r1, #0xC0
	strb r1, [r0, #2]

	@ load IF register and return
	nop
	nop
	nop
	nop
	ldrh r0, [r2]
	bx lr