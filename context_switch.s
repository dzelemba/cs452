	.file	"context_switch.c"
	.text
	.align	2
	.global	k_enter
	.type	k_enter, %function
k_enter:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0

  # Restore kernel registers.
  ldmfd sp, {r4, r5, r6, r7, r8, sb, sl, fp, ip, pc}

	.size	k_enter, .-k_enter
	.align	2
	.global	k_exit
	.type	k_exit, %function
k_exit:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0

  # Get args.
	str	r0, [sp, #-20]
	str	r1, [sp, #-24]

  # Save kernel's registers.
	stmfd	sp!, {r4, r5, r6, r7, r8, sb, sl, fp, ip, lr}

  # Switch to system mode.
  mrs r2, cpsr
  bic r2, r2, #0x1f
  orr r2, r2, #0x1f
  msr cpsr_c, r2

  # Restore task's registers (just stack ptr for now).
  mov sp, r0

  # Back to supervisor mode.
  mrs r2, cpsr
  bic r2, r2, #0x1f
  orr r2, r2, #0x13
  msr cpsr_c, r2

  # Create a user SPSR
  mrs r2, cpsr
  bic r2, r2, #0x1f
  orr r2, r2, #0x10
  msr spsr, r2

  # Jump to user task (this will also move the svc SPSR into users CPSR).
  movs pc, r1

	.size	k_exit, .-k_exit
	.ident	"GCC: (GNU) 4.0.2"
