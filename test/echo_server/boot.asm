extern kmain ;; c entry point

global _start

extern boot_stack_end

section .boot

;; real mode entrypoint
;; TODO: just boot into real mode
[bits 16]
[section .boot]

extern stack_start
global _start
_start:
	rdtsc
	rdtsc
	mov [0], eax
	mov [4], edx

	mov esp, 0x4000
	mov ebp, esp

	mov eax, gdtr32
	lgdt [eax]
	mov eax, cr0
	or al, 1 ;# protected mode enable
	mov cr0, eax
	jmp 08h:main32

[bits 32]
[section .boot]
main32:

	call record_timestamp
	call kmain
	out 0xFA, eax


global record_timestamp
record_timestamp:
	;; read the time stamp counter ASAP
	push edi
	push eax
	push edx
	rdtsc

	mov edi, DWORD [timestamp_loc]

	mov [edi], eax
	mov [edi + 4], edx

	add edi, 8

	mov [timestamp_loc], edi
	mov dword [edi], 0
	mov dword [edi + 4], 0
	pop edx
	pop eax
	pop edi
	ret

timestamp_loc:
	dq 8


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

