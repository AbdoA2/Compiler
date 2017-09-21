	li $i1, 10
main:
	;loading main args
	ld $i2, $sp[4] ; argc
	ld $i3, $sp[8] ; argv

	li $a1, 12
	mov $i4, $a1

	li $a2, 12
	bne $a2, $i4, lbl2
	li $a2, 9
	mul $a1, $a2, $i1
	mov $i1, $a1

	j lbl1
lbl2:
	li $a2, 17
	bne $a2, $i4, lbl3
	li $a3, 4
	div $a1, $i1, $a3
	mov $i1, $a1

	j lbl1
lbl3:
	li $a1, 200
	mov $i1, $a1

lbl1:
	addi, $sp, $sp, 8
	ret

