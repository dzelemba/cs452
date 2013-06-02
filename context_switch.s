	.file	"context_switch.c"
	.text
	.align	2
	.global hwi_enter
	.type	hwi_enter, %function
hwi_enter:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0

@  mov r0, #1
@  mrs r1, cpsr
@  bl bwputr

  # Switch to system mode.
  msr cpsr_c, #159

    # Save scratch registers.
    stmfd sp!, {r0, r1, r2, r3}

  # Back to IRQ mode.
  msr cpsr_c, #146

    # Grab PC & CPSR
    mov r1, lr
    mrs r2, spsr

  # Switch to system mode.
  msr cpsr_c, #159

    # Save PC & CPSR registers.
    stmfd sp!, {r1, r2}

  # Switch to supervisor mode.
  msr cpsr_c, #147

    # Now we can put our info in svc's so it will jump back to us.
    msr spsr_c, #146
    mov r0, #0

    bl k_enter

  #### Back from k_exit ####

  # Switch to system mode.
  msr cpsr_c, #159

    # Grab user's CPSR & PC
    ldmfd sp!, {r1, r2}

  # Back to IRQ mode.
  msr cpsr_c, #146

    # Grab PC & CPSR
    mov lr, r1
    msr spsr, r2

  # Switch to system mode.
  msr cpsr_c, #159

    # Restor user's scratch registers
    ldmfd sp!, {r0, r1, r2, r3}

  # Back to IRQ mode.
  msr cpsr_c, #146

    # Jump to user task (this will also move the svc SPSR into users CPSR).
    subs pc, lr, #4

	.align	2
	.global	k_enter
	.type	k_enter, %function
k_enter:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
  # Do NOT touch r0! It holds the pointer to the request.

  # Grab pc & SPSR
  mov r1, lr
  mrs r2, spsr

  # Switch to system mode.
  msr cpsr_c, #159

  # Save user's registers.
  # r1 holds user's pc, r2 holds user's SPSR.
  stmfd sp!, {r1, r2, r4, r5, r6, r7, r8, sb, sl, fp, ip, lr}

  # Grab user sp into r3
  mov r3, sp

  # Back to supervisor mode.
  msr cpsr_c, #147

  # Restore kernel registers.
  # The kernel's pointer to the user's stack ptr is put into r1
  ldmfd sp!, {r1, r4, r5, r6, r7, r8, sb, sl, fp, ip}

  # Update kernel's pointer to user's stack ptr.
  str r3, [r1]

  # Jump back to whoever called k_exit
  ldmfd sp!, {pc}

	.size	k_enter, .-k_enter
	.align	2
	.global	k_exit
	.type	k_exit, %function
k_exit:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
  # Don't touch r0! It holds the user's return value.

  # Derefernce pointer to stack pointer.
  ldr r3, [r1]

  # Save kernel's registers.
  # r1 holds kernel's pointer to user's stack ptr, so we can update it later.
  stmfd sp!, {r1, r4, r5, r6, r7, r8, sb, sl, fp, ip, lr}

  # Switch to system mode.
  msr cpsr_c, #159

  # Restore user's stack ptr
  mov sp, r3

  # Restore task's registers.
  # r1 holds task's pc, r3 holds tasks spsr.
  ldmfd sp!, {r1, r3, r4, r5, r6, r7, r8, sb, sl, fp, ip, lr}

  # Back to supervisor mode.
  msr cpsr_c, #147

  # Restore user SPSR
  msr spsr, r3

  # Jump to user task (this will also move the svc SPSR into users CPSR).
  movs pc, r1

	.size	k_exit, .-k_exit
	.ident	"GCC: (GNU) 4.0.2"
