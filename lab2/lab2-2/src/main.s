 .syntax unified
 .cpu cortex-m4
 .thumb

.data
	result: .word 0
	max_size: .word 0
.text
.global main
	m: .word 0x27
	n: .word 0x41

main:
	// r0 = m, r1 = n
	mov r1, #12
	ldr r4, =m
	ldr r5, =n
	ldr r1, [r4]
	ldr r2, [r5]
	mov r3, #1 // r3 = ans
	mov r9, #0 // r9 = stack size

 	BL GCD
 	ldr r7, = result
	str r3, [r7]
	ldr r8, = max_size
	str r9, [r8]
	b L
 	// get return val and store into result

GCD:
	//TODO: Implement your GCD function
	push {lr}
	add r9, r9, #4

	cmp r1, #0
	bne t1
	// a == 0
	mul r3, r3, r2
	pop {pc}
	bx lr
t1:
	cmp r2, #0
	bne t2
	// b == 0
	mul r3, r3, r1
	pop {pc}
	bx lr
t2:
	and r6, r1, #1		// a % 2
	and r7, r2, #1		// b % 2

	cmp r6, #0
	beq t3
	// a % 2 != 0; b?
	cmp r7, #0
	beq t5
	//// a % 2 != 0; b % 2 != 0
	//// else
	sub r8, r1, r2
	cmp r8, #0
	bgt pos
	mov r11, #-1
	mul r8, r8, r11
pos:
	cmp r1, r2
	blt asmall
	mov r1, r8
	bl GCD
	pop {pc}
	bx lr
asmall:
	mov r2, r1
	mov r1, r8
	bl GCD
	pop {pc}
	bx lr
t3:
	// a % 2 == 0; b?
	cmp r7, #0
	bne t4
	//// a % 2 == 0; b % 2 == 0
	movs r1, r1, lsr#1
	movs r2, r2, lsr#1
	mov r11, #2
	mul r3, r3, r11
	bl GCD
	pop {pc}
	bx lr
t4:
	//// a % 2 == 0; b % 2 != 0
	//// else if (a % 2 == 0)
	movs r1, r1, lsr#1
	bl GCD
	pop {pc}
	bx lr
t5:
	//// a % 2 != 0; b % 2 == 0
	//// else if (b % 2 == 0)
	movs r2, r2, lsr#1
	bl GCD
	pop {pc}
	bx lr

L:
	nop
	b L;
