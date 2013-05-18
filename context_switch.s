	.file	"context_switch.c"
	.text
	.align	2
	.global	k_enter
	.type	k_enter, %function
k_enter:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0

# System MOde
  # TODO: Save user's registers.
# End SYstem Mode

  # Get status into r1
  # ldr r1, [R14, #-4]
  #bic r1, r1, #0xFF000000

  # Restore kernel registers. Put pc into r2
  ldmfd sp!, {r4, r5, r6, r7, r8, sb, sl, fp, ip, pc}

/*
  # Pass args[] into r0
  lbm r1, [r0, #16]
  str r1, [sp]

  ldm r1, [r0, #4]
  ldm r2, [r0, #8]
  ldm r3, [r0, #12]
  ldm r0, [r0]

  # Call appropriate method

  # bl Create..

  # ldmia sp!, {pc}
  mov pc, r2
*/
	.size	k_enter, .-k_enter
	.align	2
	.global	k_exit
	.type	k_exit, %function
k_exit:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0

  # Save kernel's registers.
	stmfd	sp!, {r4, r5, r6, r7, r8, sb, sl, fp, ip, lr}

  # Switch to system mode.
  mrs r2, cpsr
  bic r2, r2, #0x1f
  orr r2, r2, #0x1f
  msr cpsr_c, r2

  # Restore stack ptr
  mov sp, r0 

  # Grab values we'll need back in supervisor mode.
  ldr r0, [sp, #0]
  ldr r1, [sp, #-4]

  # Restore task's registers.
  sub sp, sp, #8
  ldmfd sp, {r4, r5, r6, r7, r8, sb, sl, fp, ip, lr}
  add sp, sp, #8

  # Back to supervisor mode.
  mrs r2, cpsr
  bic r2, r2, #0x1f
  orr r2, r2, #0x13
  msr cpsr_c, r2

  # Restore user SPSR 
  msr spsr, r1

  # Jump to user task (this will also move the svc SPSR into users CPSR).
  movs pc, r0

	.size	k_exit, .-k_exit
	.ident	"GCC: (GNU) 4.0.2"
