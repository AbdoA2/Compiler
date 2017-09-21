	li $i1, 100
main:
	;loading main args
	ld $i2, $sp[4] ; argc
	ld $i3, $sp[8] ; argv

	li $a1, 95
	mov $i4, $a1

	li $a1, 30
	mov $i5, $a1

	li $i6, 0
	li $a1, 0
	mov $i6, $a1

lbl1:
	li $a3, 50
	slt $a1, $i6, $a3
	beq $i1, $0, lbl2
	li $a1, 13
	mov $i7, $a1

	addi $sp, $sp, 4

	li $a3, 1
	add $a1, $i6, $a3
	mov $i6, $a1

	j lbl1

lbl2:
	addi, $sp, $sp, 4
	ret

