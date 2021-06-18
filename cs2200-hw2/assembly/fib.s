! ============================================================
! CS 2200 Homework 2 Part 2: Fibonacci
!
! Apart from initializing the stack,
! please do not edit main's functionality.
!============================================================

main:
    lea     $sp, stack          ! load ADDRESS of stack label into $sp
    lw      $sp, 0x00($sp)
    lea     $at, fib            ! load address of fib label into $at
    lea     $a0, Num            ! load ADDRESS of Num label into $a0
    lw $a0, 0x00($a0)
    sw      $t0, 0x00($sp)      ! Save registers 0-2
    addi $sp, $sp, -1
    sw      $t1, 0x00($sp)
    addi $sp, $sp, -1
    sw      $t2, 0x00($sp)      ! Increment stack pointer
    addi $sp, $sp, -1
    sw      $ra, 0x00($sp)      ! Save the return address pointer
    
    jalr    $ra, $at            ! jump to fib, set $ra to return addr


    
    lw      $ra, 0x00($sp) 
    addi    $sp, $sp, 1
    lw      $t2, 0x00($sp)      ! Save registers 0-2
    lw      $t1, 0x01($sp)
    lw      $t0, 0x02($sp)
    addi    $sp, $sp, 2
    halt                        ! when we return, just halt

Num: .word  2                   ! The number n used for fib(n)
                                ! You may want to change this number to different values
                                ! to test your algorithm

fib:
    
    addi $sp, $sp, -1
    sw      $fp, 0x00($sp)      ! Save the previous frame pointer
    
    addi $t1, $a0, -1
    ble $t1, $zero, base                             ! IF (a0 <= 1)
    ble $zero, $zero, else                        !    GOTO BASE
                                ! ELSE
                                !    GOTO ELSE

else:
    add     $t0, $a0, $zero ! TODO: Save the value of the $a0 into a saved register

    addi    $a0, $a0, -1        ! $a0 = $a0 - 1 (n - 1)

    addi    $sp, $sp, -1
    sw      $t0, 0x00($sp)      ! Save registers 0-2
    addi $sp, $sp, -1
    sw      $t1, 0x00($sp)
    addi $sp, $sp, -1
    sw      $t2, 0x00($sp)      ! Increment stack pointer
    addi $sp, $sp, -1
    sw      $ra, 0x00($sp)      ! Save the return address pointer
    addi $sp, $sp, -1
    sw      $fp, 0x00($sp)      ! Save the previous frame pointer
    jalr    $ra, $at ! TODO: Implement the recursive call to fib
                                ! You should not have to set any of the argument registers here.
    lw      $fp, 0x00($sp)
    addi    $sp, $sp, 1
    lw      $ra, 0x00($sp) 
    addi    $sp, $sp, 1
    lw      $t2, 0x00($sp)      ! Save registers 0-2
    lw      $t1, 0x01($sp)
    lw      $t0, 0x02($sp)
    addi    $sp, $sp, 3                       ! when we return, just halt 

    add     $t1, $v0, $zero ! TODO: Save the return value of the fib call into a register

    add     $a0, $t0, $zero ! TODO: Restore the old value of $a0 that we saved earlier

    addi    $a0, $a0, -2        ! $a0 = $a0 - 2 (n - 2)

    addi    $sp, $sp, -1
    sw      $t0, 0x00($sp)      ! Save registers 0-2
    addi $sp, $sp, -1
    sw      $t1, 0x00($sp)
    addi $sp, $sp, -1
    sw      $t2, 0x00($sp)      ! Increment stack pointer
    addi $sp, $sp, -1
    sw      $ra, 0x00($sp)      ! Save the return address pointer
    addi $sp, $sp, -1
    sw      $fp, 0x00($sp)      ! Save the previous frame pointer
    jalr    $ra, $at ! TODO: Implement the recursive call to fib
                                ! You should not have to set any of the argument registers here.
    lw      $fp, 0x00($sp)
    addi    $sp, $sp, 1
    lw      $ra, 0x00($sp) 
    addi    $sp, $sp, 1
    lw      $t2, 0x00($sp)      ! Save registers 0-2
    lw      $t1, 0x01($sp)
    lw      $t0, 0x02($sp)
    addi    $sp, $sp, 3 
    add     $v0, $v0, $t1 ! TODO: Compute fib(n - 1) [stored from earlier] + fib(n - 2) [just computed]
                                ! Place the sum of those two values into $v0
    bge     $zero, $zero, teardown ! return

base:
    ble     $a0, $zero, normalize
    addi    $v0, $a0, 0         ! return a
    bge     $zero, $zero, teardown ! teardown the stack
normalize:
    addi    $v0, $zero, 0
    bge     $zero, $zero, teardown ! teardown the stack
teardown:
    lw      $fp, 0x00($sp)
    addi    $sp, $sp, 1
    jalr    $zero, $ra          ! return to caller

stack: .word 0xFFFF             ! the stack begins here
