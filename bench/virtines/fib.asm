[bits 16]
[org 0x8000]
[section .text]


PG_PRESENT  equ 0x1
PG_WRITABLE equ 0x2
PG_USER     equ 0x4
PG_BIG      equ 0x80
PG_NO_EXEC  equ 0x8000000000000000
; CR0 bitmasks
CR0_PAGING equ 0x80000000

; CR4 bitmasks
CR4_PAE equ 0x20
CR4_PSE equ 0x10

; MSR numbers
MSR_EFER equ 0xC0000080
EFER_SCE equ 0x001

; EFER bitmasks
EFER_LM equ 0x100
EFER_NX equ 0x800


[bits 16]
[section .text]

extern stack_start

global _start
_start:
	mov eax, gdtr32
	lgdt [eax]
	mov eax, cr0
	or al, 1
	mov cr0, eax

	jmp 08h:main32

[bits 32]
[section .text]
main32:

	mov esp, 0x8000
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

	mov rax, 0
	;# load the argument from address zero
	mov rsi, [0]
	call fib
	hlt


fib:
	cmp esi, 2
	jge .fib_sum
	mov eax, esi
	ret

.fib_sum:

	sub rsp, 8
	mov dword [rsp + 4], esi
	mov dword [rsp + 0], 0

	sub esi, 2
	call fib
	add dword [rsp + 0], eax

	mov esi, dword [rsp + 4]
	sub esi, 1
	call fib
	add dword [rsp + 0], eax
	mov eax, dword [rsp + 0]
	add rsp, 8
	ret



[section .data]
align 8
gdt32:
  	dq 0x0000000000000000
  	dq 0x00cf9a000000ffff
  	dq 0x00cf92000000ffff
align 8
gdtr32:
	dw 23
	dd gdt32


align 8
gdt64:
    dq 0x0000000000000000 ; null
    dq 0x00af9a000000ffff ; code (note lme bit)
    dq 0x00af92000000ffff ; data (most entries don't matter)

align 8
gdtr64:
    dw 23
    dq gdt64



; paging structures
[section .paging]
align 4096
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
    %assign pg pg+(4096 * 512)
  %endrep
