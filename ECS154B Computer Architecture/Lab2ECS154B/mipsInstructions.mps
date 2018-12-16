	addi	$1, $0, 8	# $1 = 8
	ori	$2, $1, 5	# $2 = 13
	andi	$3, $2, -1	# $3 = 13
	add	$4, $3, $1	# $4 = 8 + 13 = 21
	sub	$5, $4, $3	# $5 = 21 - 13 = 8
	and	$6, $1, $2	# $6 = 8
	xor	$7, $6, $3	# $7 = 5
	sw	$4, 100($5)	# memory[100 + 8] = 21
	lw	$8, 100($5)	# $8 = memory[100 + 8] = 21
	slt	$9, $1, $2	# $9 = $1 < $2 = 8 < 13 = 1
	or	$10, $9, $6	# $10 = 9
	addi	$11, $10, -10	# $11 = -1
	sltu	$12, $8, $11	# $12 = 1 (-1 = 0xFFFFFFFF)
	jal	begin	# $31 = 56
	addi	$13, $0, 1	# should not execute
begin:	addi	$31, $31, 24	# $31 = 80
middle:	beq	$6, $0, end	# not taken first time
	sub	$6, $5, $1	# $6 = 0
	j	middle
	addi	$14, $0, 1	# should not execute
end:	jr	$31	# infinite loop
	addi	$15, $0, 1	# should not execute
	addi	$16, $0, 1	# should not execute