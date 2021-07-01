	.text
.syntax	unified

.align 4
.global add_int_assembly
.arm

/*
	r0 : address of pa
	r1 : address of pb
	r2 : n
	r3 : x	
	r4 : i	
	r5 : iterator of pa
	r6 : iterator of pb
	
	r4 : address of pa
	r5 : address of pb
	r6 : n
	r12 : x
*/

add_int_assembly:
	push {r4, r5, r6, r12, lr}
	bics             r12, r2, #3
	beq              label2
	vdup.32          q1,r3
	lsrs	 		 r2,r2,#2
	beq				 label2
label1:
	VLD1.32 {d0, d1}, [r0]!
	vadd.i32 q0, q0, q1
	subs r2,r2,#1
	vst1.32 {d0,d1}, [r1]!
	bne label1
label2:
	pop {r4,r5,r6,r12,lr}
	bx lr
