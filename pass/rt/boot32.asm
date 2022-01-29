[bits 16]
[section .text]
extern __virtine_main

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

	mov esp, 0x1000
	mov ebp, esp

	push dword 0
	call __virtine_main
	out 0xFA, ax ;; call the exit hypercall


global hcall
hcall:
	push ebp
	mov ebp, esp
	mov eax, [ebp + 8]
	mov ebx, [ebp + 12]
	out 0xFF, eax,
	pop ebp
	ret


[section .data]
gdt32:
  	dq 0x0000000000000000
  	dq 0x00cf9a000000ffff
  	dq 0x00cf92000000ffff
gdtr32:
	dw 23
	dd gdt32
