[bits 16]
[section .text]
extern __virtine_main
extern __snapshot
extern __libc_init_array
extern __libc_fini_array
extern atexit


MSR_EFER equ 0xC0000080
EFER_LM equ 0x100
EFER_NX equ 0x800
EFER_SCE equ 0x001
CR0_PAGING equ 0x80000000
CR4_PAE equ 0x20
CR4_PSE equ 0x10
PG_PRESENT  equ 0x1
PG_WRITABLE equ 0x2
PG_USER     equ 0x4
PG_BIG      equ 0x80
PG_NO_EXEC  equ 0x8000000000000000
LOG_TABLE_SIZE equ 9
LOG_PAGE_SIZE  equ 12
PAGE_SIZE  equ (1 << LOG_PAGE_SIZE)
TABLE_SIZE equ (1 << LOG_TABLE_SIZE)


extern __stack_base

global _start
_start:
	cli
	mov eax, gdtr32
	lgdt [eax]
	mov eax, cr0
	or al, 1
	mov cr0, eax
	jmp 08h:.trampoline

[bits 32]
.trampoline:

	mov esp, __stack_base - 16
	mov ebp, esp


  ; enable PAE and PSE
  mov eax, cr4
  or eax, (CR4_PAE + CR4_PSE)
  mov cr4, eax

	; enable long mode and the NX bit
  mov ecx, MSR_EFER
  rdmsr
  or eax, (EFER_LM | EFER_NX | EFER_SCE)
  wrmsr

  ; set cr3 to a pointer to pml4
  mov eax, boot_p4
  mov cr3, eax

  ; enable paging
  mov eax, cr0
  or eax, CR0_PAGING
  mov cr0, eax

  ; leave compatibility mode and enter long mode
  lgdt [gdtr64]
  mov ax, 0x10
  mov ss, ax
  mov ax, 0x0
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  jmp 0x08:.trampoline64

[bits 64]
.trampoline64:

	mov rsp, __stack_base - 16
	mov rbp, rsp

	call enable_sse

	;; call the libc stuff
	call __libc_init_array
	mov rdi, __libc_fini_array
	call atexit

	call __snapshot

	mov rdi, 0
	call __virtine_main
	out 0xFA, ax ;; call the exit hypercall




enable_sse:
	push rax

	;; setup sse
	mov rax, cr0
	and ax, 0xFFFB		;clear coprocessor emulation CR0.EM
	or ax, 0x2			;set coprocessor monitoring  CR0.MP
	mov cr0, rax

	mov rax, cr4
	or ax, 3 << 9		;set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
	mov cr4, rax

	pop rax
	ret



global __hypercall
__hypercall:
	out 0xFF, eax
	ret


global setjmp
global _setjmp
global __setjmp
global longjmp
global _longjmp
global __longjmp

__setjmp:
_setjmp:
setjmp:
	mov [rdi], rbx ; rdi is jmp_buf, move registers onto it
	mov [rdi + 8], rbp
	mov [rdi + 16], r12
	mov [rdi + 24], r13
	mov [rdi + 32], r14
	mov [rdi + 40], r15
	lea rdx, [rsp + 8] ; this is our rsp WITHOUT current ret addr
	mov [rdi + 48], rdx
	mov rdx, [rsp]    ; save return addr ptr for new rip
	mov [rdi + 56], rdx
	xor rax, rax      ; always return 0
	ret



_longjmp:
__longjmp:
longjmp:
	mov rax, rsi    ; val will be longjmp return
	test rax, rax
	jnz .continue
	inc rax         ; if val==0, val=1 per longjmp semantics
.continue:
	mov rbx, [rdi]  ; rdi is the jmp_buf, restore regs from it
	mov rbp, [rdi + 8]
	mov r12, [rdi + 16]
	mov r13, [rdi + 24]
	mov r14, [rdi + 32]
	mov r15, [rdi + 40]
	mov rdx, [rdi + 48] ; this ends up being the stack pointer
	mov rsp, rdx
	mov rdx, [rdi + 56] ; this is the instruction pointer
	jmp rdx


[section .data]
gdt32:
  	dq 0x0000000000000000
  	dq 0x00cf9a000000ffff
  	dq 0x00cf92000000ffff
gdtr32:
	dw 23
	dd gdt32



; the global descriptor table
gdt64:
    dq 0
    dq 0x00AF98000000FFFF
    dq 0x00CF92000000FFFF
gdt64_end:
  dq 0 ; some extra padding so the gdtr is 16-byte aligned
gdtr64:
  dw gdt64_end - gdt64 - 1
  dq gdt64



; paging structures
[section .paging]
align PAGE_SIZE
[global boot_p4]
boot_p4:
	dq (boot_p3 + PG_PRESENT + PG_WRITABLE)
	times 511 dq 0

boot_p3:
	dq (boot_p2 + PG_PRESENT + PG_WRITABLE)
	times 511 dq 0

boot_p2:
	%assign pg 0
  %rep 512
    dq (pg + PG_PRESENT + PG_WRITABLE + PG_BIG)
    %assign pg pg+(PAGE_SIZE * 512)
  %endrep




