.data
_prompt: .asciiz "Enter an integer:"
_ret: .asciiz "\n"
.globl main
.text
read:
  li $v0, 4
  la $a0, _prompt
  syscall
  li $v0, 5
  syscall
  jr $ra

write:
  li $v0, 1
  syscall
  li $v0, 4
  la $a0, _ret
  syscall
  move $v0, $0
  jr $ra

main:
   addi $sp, $sp, -16
   sw $ra, 12($sp)
   sw $fp, 8($sp)
   addi $fp, $sp, 16
   jal read
   move $t0, $v0
   sw $t0, -12($fp)
   lw $t0, -16($fp)
   lw $t1, -12($fp)
   move $t0, $t1
   sw $t0, -16($fp)
   lw $t0, -16($fp)
   li $t1, 0
   bgt $t0, $t1, label0
   j label1
label0:
   li $t0, 1
   move $a0, $t0
   jal write
   j label2
label1:
   lw $t0, -16($fp)
   li $t1, 0
   blt $t0, $t1, label3
   j label4
label3:
   li $t0, -1
   move $a0, $t0
   jal write
   j label5
label4:
   li $t0, 0
   move $a0, $t0
   jal write
label5:
label2:
   li $t0, 0
   move $v0, $t0
   move $sp, $fp
   lw $ra, -4($fp)
   lw $fp, -8($fp)
   jr $ra
