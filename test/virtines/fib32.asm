[bits 16]
[org 0x8000]
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

	mov eax, 0
	mov ds, ax

	mov esp, 0x8000

	push ebp
  mov ebp, esp


	mov     esi, 20
	call    fib
	hlt


fib:
	cmp esi, 2
	jge .fib_sum
	mov eax, esi
	ret

.fib_sum:

	;# stack: [  acc    ] [  arg    ]
	;#        esp + 0     esp+4

	sub esp, 8
	mov dword [esp + 4], esi
	mov dword [esp + 0], 0

	sub esi, 2
	call fib
	add dword [esp + 0], eax

	mov esi, dword [esp + 4]
	sub esi, 1
	call fib
	add dword [esp + 0], eax
	mov eax, dword [esp + 0]
	add esp, 8
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

