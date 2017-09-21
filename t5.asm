	li $i1, 0
main:
	;loading main args
	ld $i2, $sp[4] ; argc
	ld $i3, $sp[8] ; argv

	li $a1, 3
	mov $i4, $a1

	li $a1, 5
	mov $i5, $a1

	addi $sp, $sp, 4

	li.f $f1, 12.50000
	cvt.f.i $a1, $d1
	mov $i4, $a1

	addi, $sp, $sp, 8
	ret

