	li $i1, 100
	li $i2, 10
main:
	;loading main args
	ld $i3, $sp[4] ; argc
	ld $i4, $sp[8] ; argv

	li $a1, 20
	mov $i5, $a1

	li $a5, 10
	add $a3, $i5, $a5
	li $a6, 5
	mul $a4, $i2, $a6
	and $a2, $a3, $a4
	or $a1, $a2, $i1
	beq $a1, $0, lbl1
	li $a1, 50
	mov $i5, $a1

	j lbl2

lbl1:
	li $a1, 60
	mov $i5, $a1


lbl2:
	ret

