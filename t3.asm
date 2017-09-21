main:
	;loading main args
	ld $i1, $sp[4] ; argc
	ld $i2, $sp[8] ; argv

	li $i3, 0
lbl1:
	li $a3, 100
	slt $a1, $i3, $a3
	beq $a1, $0, lbl2
	li $a1, 15
	mov $i4, $a1

	li $a3, 1
	add $a1, $i3, $a3
	mov $i3, $a1

	j lbl1

lbl2:
	li $a1, 0
	mov $i3, $a1

lbl3:
	li $a3, 100
	slt $a1, $i3, $a3
	beq $a1, $0, lbl4
	li $a1, 15
	mov $i4, $a1

	li $a3, 1
	add $a1, $i3, $a3
	mov $i3, $a1

	j lbl3

lbl4:
	li $a1, 0
	mov $i3, $a1

lbl5:
	li $a1, 15
	mov $i4, $a1

	li $a3, 1
	add $a1, $i3, $a3
	mov $i3, $a1

	li $a3, 100
	seq $a1, $i4, $a3
	beq $a1, $0, lbl5
lbl6:
	addi, $sp, $sp, 12
	ret

