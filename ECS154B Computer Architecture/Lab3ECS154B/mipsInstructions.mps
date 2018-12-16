	addi	$1, $0, 5	# $1 = 5
	ori	$2, $1, 3	# $2 = 7
	andi	$3, $2, -1	# $3 = 7
	add	$4, $3, $1	# $4 = 5 + 7 = 12
	sub	$5, $2, $3	# $5 = 7 - 7 = 0
	and	$6, $1, $4	# $6 = 12 & 5 = 4 
	sw	$4, 100($5)	# memory[100] = 12
	lw	$7, 100($0)	# $7 = memory[100] = 12
	addi	$8, $7, -13	# $8 = 12 - 13 = -1
	sltu	$9, $7, $8	# $9 = $7 < $8 = 12 < 0xFFFFFFFF = 1
	sll	$10, $6, 10	# $10 = 4096
	srl	$11, $10, 5	# $11 = 128
back:	beq	$9, $0, forward	# branch not taken the first time but taken the second
	slt	$9, $2, $1	# $9 = $2 < $1 = 7 < 5 = 0
	or	$12, $1, $4	# $12 = 5 | 12 = 13
	xor	$13, $3, $7	# $13 = 11
	beq	$9, $0, back	# jump to back
forward:	jal	close	# $31 = 72
	addi	$14, $0, 1	# should not execute
done:	j	done	# infinite loop to mark that we are done
close:	addi	$31, $31, 4	# $31 = 76
	jr	$31	# jump to "done"
	addi	$15, $0, 1	# should not execute
bad:	j	bad	# should not execute