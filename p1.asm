	li $i1, 100
do_input:
	;loading do_input args
	ld $i2, $sp[4] ; x
	ld.f $f1, $sp[8] ; y

	add.f $d1, $i2, $f1
	mov $re.d, $d1
	addi, $sp, $sp, 0
	ret

	ret

main:
	;loading main args
	ld $i3, $sp[4] ; argc
	ld $i4, $sp[8] ; argv

	li $a1, 12
	st $a1, $sp[-12]
	li $a1, 13
	cvt.i.f $d1, $a1
	st $d1, $sp[-8]
	addi $sp, $sp, -12
	call do_input
	addi $sp, $sp, 8
	mov $a1, $re.i
	mov $i5, $a1

	addi, $sp, $sp, 4
	ret

