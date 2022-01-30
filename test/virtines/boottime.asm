%macro trial 1
	rdtscp
	mov edi, (0 + (%1 * 8))
	mov [edi], eax
	mov [edi+4], edx
%endmacro

;# CR0_PAGING equ 0x80000000
;# CR4_PAE equ 0x20
;# CR4_PSE equ 0x10
;# MSR_EFER equ 0xC0000080
;# EFER_LM equ 0x100
;# EFER_NX equ 0x800
;# EFER_SCE equ 0x001


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



[bits 16]
[section .text]
[org 0x8000]


global _start
_start:
	;# baseline measurement. Measured twice to prime instruction cache.
	;# This is needed after resets, as the tsc is not reset by the resetter
	trial 0
	trial 0
	trial 1 ;# First instruction

	mov eax, gdtr32
	lgdt [eax]

	trial 2 ;# Load 32-bit GDT

	mov eax, cr0
	or al, 1
	mov cr0, eax

	trial 3 ;# Protected Mode Enable

	jmp 08h:main32

[bits 32]
[section .text]
main32:

	trial 4 ;# Protected Transition
	mov esp, 0x8000

	push ebp
  mov ebp, esp

	; start setting up paging
	; PML4[0] -> PDPT
	mov eax, pdpt
	or  eax, 0x3      ; entry is present, rw
	mov ebx, pml4
	mov [ebx], eax

	; PDPT[0] -> PDT
	mov eax, pd
	or  eax, 0x3
	mov ebx, pdpt
	mov [ebx], eax

	; Identity map the first GB with 2MB pages
	mov ecx, 512
	mov edx, pd
	mov eax, 0x83 ; set PS bit also (PDE -> 2MB page)
.write_pde:
	mov [edx], eax
	add eax, 0x200000
	add edx, 0x8
	loop .write_pde

	; put pml4 address in cr3
	mov eax, pml4
	mov cr3, eax


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
	trial 5 ;# Paging Identity Mapping

  ; leave compatibility mode and enter long mode

	mov eax, gdtr64
	lgdt [eax]

	trial 6 ;# Load 64-bit GDT
	jmp 0x08:main64

[bits 64]
main64:

	mov rax, 0

	trial 7 ;# Jump to Long Mode
	hlt



[section .data]
align 8
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


align 4096
pml4: times 4096 db 0
pdpt: times 4096 db 0
pd: times 4096 db 0





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

