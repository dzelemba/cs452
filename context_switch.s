	.file	"context_switch.c"
	.text
	.align	2
	.global	k_enter
	.type	k_enter, %function
k_enter:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0

  # Grab pc & SPSR
  mov r1, lr
  mrs r2, spsr

  # Switch to system mode.
  mrs r2, cpsr
  bic r2, r2, #0x1f
  orr r2, r2, #0x1f
  msr cpsr_c, r2

  # Save user's registers.
  # r1 holds user's pc, r2 holds user's SPSR.
  stmfd sp!, {r1, r2, r4, r5, r6, r7, r8, sb, sl, fp, ip, lr}

  # Grab user sp into r3
  mov r3, sp

  # Back to supervisor mode.
  mrs r2, cpsr
  bic r2, r2, #0x1f
  orr r2, r2, #0x13
  msr cpsr_c, r2

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
  mrs r2, cpsr
  bic r2, r2, #0x1f
  orr r2, r2, #0x1f
  msr cpsr_c, r2

  # Restore user's stack ptr
  mov sp, r3

  # Restore task's registers.
  # r1 holds task's pc, r3 holds tasks spsr.
  ldmfd sp!, {r1, r3, r4, r5, r6, r7, r8, sb, sl, fp, ip, lr}

  # Back to supervisor mode.
  mrs r2, cpsr
  bic r2, r2, #0x1f
  orr r2, r2, #0x13
  msr cpsr_c, r2

  # Restore user SPSR
  msr spsr, r3

  # Jump to user task (this will also move the svc SPSR into users CPSR).
  movs pc, r1

	.size	k_exit, .-k_exit
	.ident	"GCC: (GNU) 4.0.2"
